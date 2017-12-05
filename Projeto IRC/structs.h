#ifndef STRUCTS_H
#define STRUCTS_H

#define SERVER_PORT 9000
#define MAX_BUFFER 1024
#define MAX_CLIENTS 10
#define MAX_NOME 256
#define MAX_MESSAGES 200

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

typedef struct thread_args_msg_delay{
  int fd, seconds;
  char str[MAX_BUFFER];
}thread_args_delay;

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
  struct history_node* next;
}history;

#endif //STRUCTS_H
