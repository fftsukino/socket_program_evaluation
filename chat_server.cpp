#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 5000
#define SOCK_MAX 5
#define UNUSED (-1)
#define MAX_EVENTS 64

class MultiEpoll{
    private :
        int epfd_read;
        int epfd_write;
        struct epoll_event ev_read;
        struct epoll_event ev_write;
        int ret;
    public :
        void prepare_socket();
        void make_socket();
        void bind();
        void listen();
        void prepare_poll();
        void polling();
        void close();
}

// epoll用に追加
int epfd, epfd_client;
struct epoll_event event;
struct epoll_event ev_write;
int ret;


int prepare_multi_socket(){

    // epfd for read
    epfd = epoll_create(10);
    if(epfd < 0){
        perror("epoll_create");
    }
    // epfd for write
    epfd_client = epoll_create(10);
    if(epfd_client < 0){
        perror("epoll_create");
    }

    return 0;
}

int make_socket(int s[]){
    // TCP 待ちうけようソケットを作成
    if ((s[0] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
}

int do_bind(struct sockaddr_in *saddr, int s[]){
    // bind用データ設定
    memset(saddr, 0, sizeof (*saddr) );

    saddr->sin_family = AF_INET;
    saddr->sin_addr.s_addr = INADDR_ANY;
    saddr->sin_port = htons(PORT);

    if ((bind(s[0], (struct sockaddr *)saddr, sizeof(*saddr))) == -1) {
        perror("bind");
        exit(1);
    }
}

int do_listen(int s[]){
    if ((listen(s[0], SOCK_MAX)) == -1) {
        perror("listen");
        exit(1);
    }
}

int set_multi_read(int s[]){
    memset(&event, 0, sizeof event);
    event.data.fd = s[0];
    event.events = EPOLLIN;

    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, s[0], &event);
    if(ret){
        perror("epoll_ctl");
    }
}

int main()
{
    int s[SOCK_MAX + 1];
    int n = 0;
    int len;
    fd_set readfds;
    int cllen;
    struct sockaddr_in saddr, caddr;
    char str[1024], buf[1024];
    int i, j, msglen;


    prepare_multi_socket();

    make_socket(s);

    do_bind(&saddr, s);

    do_listen(s);

    set_multi_read(s);

    struct epoll_event *events;
    int nr_events;
    char buffer[1024];
    events = malloc(sizeof(struct epoll_event) * MAX_EVENTS);
    if(!events){
        perror("malloc");
        return 1;
    }

    while(1){
        nr_events = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if(nr_events < 0){
            perror("epoll_wait");
            free(events);
            return 1;
        }

        for(i=0; i< nr_events; i++){
            if(events[i].data.fd == s[0]){
                // listenしているsocket処理
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof client_addr;

                int client = accept(s[0], (struct sockaddr *) &client_addr, &client_addr_len);
                if (client < 0) {
                    perror("accept");
                    continue;
                }

                //setnonblocking(client);
                memset(&event, 0, sizeof event);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client, &event);
                // To send message for client 
                memset(&ev_write, 0 , sizeof ev_write);
                ev_write.events = EPOLLOUT;
                ev_write.data.fd = client;
                epoll_ctl(epfd_client, EPOLL_CTL_ADD, client, &ev_write);

            }else{
                // clientとつながっているsocket処理
                int client = events[i].data.fd;
                int n = read(client, buffer, sizeof buffer);
                if (n < 0) {
                    perror("read");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client, &event);
                    close(client);
                } else if (n == 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, client, &event);
                    close(client);
                } else {
                    // clientを探して出力
                    int write_events;
                    struct epoll_event *events_write;
                    events_write = malloc(sizeof(struct epoll_event) * MAX_EVENTS);
                    write_events = epoll_wait(epfd_client,  events_write, MAX_EVENTS, -1);
                    if(write_events < 0){
                        perror("epoll_wait22");
                        free(events_write);
                        return ;
                    }

                    int j;
                    for(j=0; j< write_events; j++){
                        int client_write = events_write[j].data.fd;
                        if(client_write != s[0]){
                            int m;
                            m = write(client_write, buffer, n);
                            if(m <= 0){
                                epoll_ctl(epfd_client, EPOLL_CTL_DEL, client_write, &ev_write );
                                close(client_write);
                            }

                        }
                    }
                }
            }
        }
    }


    /*
       while (1) {
       FD_ZERO(&readfds);

       for (i = 0; i < SOCK_MAX; i++) {
       if (s[i] != UNUSED) {
       FD_SET(s[i], &readfds);
       }
       }

       if ((n = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) == -1) {
       perror("select");
       exit(1);
       }
       printf("select returns: %d\n", n);

       for (i = 1; i < SOCK_MAX; i++) {
       if (s[i] != UNUSED) {
       if (FD_ISSET(s[i], &readfds)) {

       printf("s[%d] ready for reading\n", i);
       bzero(str, sizeof(str));
       if ((msglen = read(s[i], str, sizeof(str))) == -1) {
       perror("read");
       } else if (msglen != 0) {
       printf("client[%d]: %s", i, str);
       bzero(buf,sizeof(buf));
       sprintf(buf, "client[%d];%s", i, str);
       for (j = 1; j < SOCK_MAX; j++) {
       if (s[j] != UNUSED) {
       write(s[j], buf, strlen(buf));
       }
       }
       } else {
       printf("client[%d]: connection closed.\n", i);
       close(s[i]);
       s[i] = UNUSED;
       }
       }
       }
       }

       if (FD_ISSET(s[0], &readfds) != 0) {
       printf("Accept New one.\n");
       len = sizeof(caddr);
       for(i = 1; i < SOCK_MAX + 1; i++) {
       if(s[i] == UNUSED) {
       s[i] = accept(s[0], (struct sockaddr *)&caddr, &len);
       printf("%d = accept()\n", s[i]);
       if (s[i] == -1){
       perror(NULL);
       exit(1);
       }
       if(i == SOCK_MAX ) {
       printf("refuse connection.\n");
       strcpy(str, "Server is too busy.\n");
       write(s[i], str, strlen(str));
       close(s[i]);
       s[i] =UNUSED;
       }
       i = SOCK_MAX + 1;
       }
       }
       }
       }
     */

}

