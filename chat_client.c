#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>

#define PORT 5000

main (int argc, char *argv[])
{
    struct sockaddr_in    addr;
    struct hostent *hp;
    int    fd, n;
    int    len;
    char   buf[1024];
    int    ret;
    fd_set readfds;
    fd_set writefds;

    if (argc != 2){
        printf("Usage: iclient SERVER_NAME\n");
        exit(1);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    bzero((char *)&addr, sizeof(addr));

    if ((hp = gethostbyname(argv[1])) == NULL) {
        perror("No such host");
        exit(1);
    }
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("connect");
        exit(1);
    }

    for(;;){
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      FD_SET(fd, &readfds);
      FD_SET(0, &readfds);
      if ((n = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) == -1) {
	perror("select");
	exit(1);
      }

      if (FD_ISSET(fd, &readfds)) {
	bzero(buf, sizeof(buf));
	ret = read(fd, buf, 1023);
	printf("%s",buf);
      }

      if(FD_ISSET(0, &readfds)){
	fgets(buf, 1024, stdin);
	write(fd, buf, 1024);
      }
    }
    close(fd);
    exit(0);
}

