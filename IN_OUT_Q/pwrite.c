#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <errno.h>  

#define HOST "127.0.0.1"
#define PORT 1111


int main (int argc, char **argv)
{
    char *buf;
    char *r_buf;
    int bufsiz;
    int sd, err;
    socklen_t len, r_len;
    int sndbufsiz, used, rcvbufsiz, r_used;
    int count = 0, r_count = 0;
    int i;
    struct sockaddr_in sa;
    int val;

    if(argc == 2) {
        bufsiz = atoi(argv[1]);
    } else {
        bufsiz = BUFSIZ;
    }
    printf("Allocating %d bytes for write buffer.\n", bufsiz);
    buf = malloc(bufsiz);
    r_buf = malloc(bufsiz);
    if(!buf) {
        perror("malloc");
        exit(1);
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(0);

    err = bind(sd, (struct sockaddr*)&sa, sizeof(sa));
    if(err < 0) {
        perror("bind");
        exit(1);
    }

    // set nonblocking
    //val = 1;
    //ioctl(sd, FIONBIO, &val);

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    if(!inet_aton(HOST, &sa.sin_addr)) {
        printf("bad address\n");
        exit(1);
    }

    err = connect(sd, (struct sockaddr *)&sa, sizeof(sa));
    if(err < 0) {
        perror("connect");
        exit(1);
    }

    // set nonblocking
    val = 1;
    ioctl(sd, FIONBIO, &val);

    //
    printf("NO\tSO_SNDBU\tSIOCOUTQ\tsnd_avail\tSO_RCVBUF\tSOICINQ\trcv_avail\n");

    // give the read socket time to collect buffer sizes
    // start hitting it with data.
    sleep(1);

    for(i=0;;i++) {
        len = sizeof(sndbufsiz);
        err = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &sndbufsiz, &len);
        if(err < 0) {
            perror("getsockopt write");
            exit(1);
        }
        r_len = sizeof(rcvbufsiz);
        err = getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &rcvbufsiz, &r_len);
        if(err <0){
            perror("getsockopt read");
            exit(1);
        }

        err = ioctl(sd, SIOCOUTQ, &used);
        if(err < 0) {
            perror("ioctl SIOCOUTQ");
            exit(1);
        }

        err = ioctl(sd, SIOCINQ, &r_used);
        if(err < 0){
            perror("ioctl SIOCINQ");
            exit(1);
        }

        len = write(sd, buf, bufsiz);
        if((int)len < 0) {
            perror("write");
            exit(0);
        }
        if((int)len <= 0) {
            exit(0);
        }
        r_len = read(sd, r_buf, bufsiz);
        if ((int)r_len < 1){
            if (errno == EAGAIN){
                r_len = 0;
            }else{
                perror("read");
                exit(1);
            }
        }else{
            // 読み込む用
            printf("%2d \n", r_len);
        }

        count += len;
        //printf("%2i: SO_SNDBUF=%6d, SIOCOUTQ=%6d: %6d avail. Wrote %d bytes, %d total.\n",
        //        i, sndbufsiz, used, sndbufsiz - used, len, count);
        r_count += r_len;
        //printf("%2i: SO_RCVBUF=%6d, SIOCINQ =%6d: %6d avail. Red   %d bytes, %d total.\n",
        //        i, rcvbufsiz, r_used, rcvbufsiz - r_used, r_len, r_count);
        printf("%2i\t%6d\t%6d\t%6d\t%6d\t%6d\t%6d\n", i, sndbufsiz, used, sndbufsiz -used, rcvbufsiz, r_used, rcvbufsiz - r_used);

        sleep(1);
    }
}

