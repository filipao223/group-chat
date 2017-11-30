#ifndef STRUCTS_H
#define STRUCTS_H

#define SERVER_PORT 9000
#define MAX_BUFFER 1024
#define MAX_CLIENTS 10
#define MAX_NOME 256

//Lista ligada
typedef struct client_node{
  struct sockaddr_in client_addr;
  char nome[MAX_NOME];
  int fd;
  struct client_node* next;
}client;

typedef struct info{
  int fd;
  client* head;
}thread_args;

#endif //STRUCTS_H
