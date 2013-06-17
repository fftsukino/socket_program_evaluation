COPTS+=-g -Wall -Werror

all: pread pwrite

pread: pread.c
	$(CC) $(COPTS) pread.c -o pread

pwrite: pwrite.c
	$(CC) $(COPTS) pwrite.c -o pwrite

clean:
	rm -f pread pwrite


