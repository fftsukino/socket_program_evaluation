COPTS+=-g -Wall -Werror

all: pread pwrite

pread: pread.c
$(CC) $(COPTS) pread.c -o pread

pwrite: pwrite.c
$(CC) $(COPTS) pwrite.c -o pwrite

clean:
rm -f pread pwrite
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


