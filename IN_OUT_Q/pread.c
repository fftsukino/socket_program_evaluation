/* This program accepts a single connection on PORT but it doesn't read
 * anything. All the data piles up until the socket buffers are full.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>


#define PORT 1111


int rd;


void stats()
{
    int err;
    socklen_t len;
    int rcvbufsiz, used;

    len = sizeof(rcvbufsiz);
    err = getsockopt(rd, SOL_SOCKET, SO_RCVBUF, &rcvbufsiz, &len);
    if(err < 0) {
        perror("getsockopt");
        exit(1);
    }

    err = ioctl(rd, SIOCINQ, &used);
    if(err < 0) {
        perror("ioctl SIOCINQ");
        exit(1);
    }

    printf("Read buffer is %d bytes, with %d bytes used, %d byes availe\n",
            rcvbufsiz, used, rcvbufsiz - used);
}


void sigint()
{
    stats();
    exit(0);
}


int main(int argc, char **argv)
{
    int sd, err;
    struct sockaddr_in addr;
    int port = PORT;
    char *end_ptr;

    int ret_opt;
    while((ret_opt=getopt(argc, argv, "p:")) != -1){
        switch(ret_opt){
        case 'p':
//            fprintf(stdout, "port:%s", optarg);
            port = strtol(optarg, &end_ptr, 10);
            break;
        case ':':
            fprintf(stdout, "%c needs value\n", ret_opt);
            break;
        case '?':
            fprintf(stdout, "unknown options\n");
            break;
        }
        for(;optind<argc;optind++){
            fprintf(stdout, "not option arg %s\n", argv[optind]);
        }
    }


    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd<0) {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    err = bind(sd, (struct sockaddr*)&addr, sizeof(addr));
    if(err < 0) {
        perror("bind");
        exit(1);
    }

    err = listen(sd, 5);
    if(err < 0) {
        perror("listen");
        exit(1);
    }

    printf("waiting on port %d...\n", port);

    rd = accept(sd, NULL, NULL);
    if(rd < 0) {
        perror("accept");
        exit(1);
    }
    signal(SIGINT, sigint);

    while(1) {

        stats();
        sleep(1);

    }
}
