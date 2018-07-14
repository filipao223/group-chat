#ifndef STRUCTS_H
#define STRUCTS_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define SERVER_PORT 9000
#define MAX_BUFFER 1024
#define MAX_CLIENTS 10
#define MAX_NOME 50
#define MAX_MESSAGES 200
#define MAX_MESSAGE_BUFFER 500
#define MAX_MESSAGE_TEMP 500

//Lista ligada
typedef struct client_node{
  struct sockaddr_in client_addr;
  char nome[MAX_NOME];
  int fd;
  struct client_node* next;
}client;

typedef struct thread_args_accept{
  int fd;
  client* head;
}thread_args;

typedef struct thread_args_msg_timed{
  int sleep_time, messageID;
  struct history_node* head;
}thread_args_timed;

typedef struct block_list_node{
  char nome[MAX_NOME];
  struct block_list_node* next;
}block_list;

typedef struct thread_args_msg_write{
  int fd;
  char argv[MAX_NOME];
}thread_args_write;

typedef struct history_node{
  int messageNumber;
  char message[MAX_BUFFER+MAX_NOME];
  struct history_node* previous;
  struct history_node* next;
}history;

#endif //STRUCTS_H
