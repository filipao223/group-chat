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

#include "structs.h"
#include "linkedList.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* acceptClient(void*);
void erro(char *msg);

void cleanup(int signum){
  pthread_mutex_destroy(&mutex);
  exit(0);
}

int main(int argc, char *argv[]){
  signal(SIGINT, cleanup);
  struct sockaddr_in addr, client_addr;
  //Cria cabeça da lista ligada
  client* head = malloc(sizeof(client));
  client* current;

  thread_args *args = malloc(sizeof(thread_args));

  head->next = NULL;
  int fd;

  //ID's das threads
  pthread_t thread_id[MAX_CLIENTS], accept_thread;

  bzero((void *) &addr, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SERVER_PORT);

  if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) erro("na funcao socket");
  if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0) erro("na funcao bind");
  if( listen(fd, 5) < 0) erro("na funcao listen");

  args->fd = fd;
  args->head = head;

  //Thread que vai aceitar os clientes
  pthread_create(&accept_thread, 0, acceptClient, (void*)args);

  //Espera que haja pelo menos um cliente
  while(1){
    if(head->next != NULL) break;
    sleep(2);
  }

  //Select
  fd_set read_set;
  char str[MAX_BUFFER];
  int nread,close_client=0;
  while(1){
    close_client=0;
    current = NULL;
    FD_ZERO(&read_set);
    for(current = head->next; current->next!=NULL; current = current->next) FD_SET(current->fd, &read_set);
    FD_SET(current->fd, &read_set);

    if(select(current->fd+1, &read_set,0,0,0) > 0){
      for(current = head->next; current!=NULL; current = current->next){
        if(FD_ISSET(current->fd, &read_set)){
          nread = read(current->fd, str, sizeof(str));
          str[nread] = '\0';
          if(strcmp(str, "exit") == 0){
            printf("Cliente terminou conexao.\n");
            close_client=1;
            break;
            }
          else{
            printf("%s: %s\n", current->nome, str);
            break;
          }
        }
      }
    }
    if(close_client == 1){
      //close(current->fd);
      pthread_mutex_lock(&mutex);
      FD_CLR(current->fd, &read_set);
      remove_from_list(head, current->fd);
      pthread_mutex_unlock(&mutex);
    }
  }
}

void* acceptClient(void* args){
  struct sockaddr_in client_addr;
  int client_addr_size = sizeof(client_addr);

  thread_args* args_ptr = (thread_args*)args;

  int fd;
  while(1){
    fd = accept(args_ptr->fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_size);
    printf("Novo cliente conectado.\n");
    pthread_mutex_lock(&mutex);
    add_to_list(args_ptr->head, fd, "nome", client_addr);
    pthread_mutex_unlock(&mutex);
  }
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
