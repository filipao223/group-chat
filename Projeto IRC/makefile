CFLAGS = -g -Wall -D_REENTRANT -pthread -DDEBUG=0 -c
CFLAGSF = -g -Wall -D_REENTRANT -pthread -DDEBUG=0 -o
CC = gcc

server: server.o linkedList.o log.o
	$(CC) server.o linkedList.o log.o $(CFLAGSF) server
client: client.o linkedList.o
	$(CC) client.o linkedList.o $(CFLAGSF) client
client.o: client.c linkedList.h structs.h
	$(CC) client.c $(CFLAGS)
server.o: server.c linkedList.h structs.h
	$(CC) server.c $(CFLAGS)
linkedList.o: linkedList.c structs.h
	$(CC) linkedList.c $(CFLAGS)
log.o: log.c structs.h
	$(CC) log.c $(CFLAGS)
clean:
	rm *.o server client
