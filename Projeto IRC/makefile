CFLAGS = -g -Wall -D_REENTRANT -pthread -c
CFLAGSF = -g -Wall -D_REENTRANT -pthread -o
CC = gcc

server: server.o linkedList.o
	$(CC) server.o linkedList.o $(CFLAGSF) server
server.o: server.c linkedList.h structs.h
	$(CC) server.c $(CFLAGS)
linkedList.o: linkedList.c structs.h
	$(CC) linkedList.c $(CFLAGS)
clean:
	rm *.o server
