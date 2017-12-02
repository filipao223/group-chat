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
void sendToAll(client*, client*, int, char*);

void cleanup(int signum){
  /*client* current, *next;
  for(current = head->next; next!=NULL;){ //Limpa memoria
    next = current->next;
    free(current);
  }*/
  printf("\n\nA fechar servidor\n\n");
  sleep(1);
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
  int var;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &var, sizeof(int));
  if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0) erro("na funcao bind");
  if( listen(fd, 5) < 0) erro("na funcao listen");

  args->fd = fd;
  args->head = head;

  //Thread que vai aceitar os clientes
  pthread_create(&accept_thread, 0, acceptClient, (void*)args);

  //Select
  char str[MAX_BUFFER];
  int nread,close_client=0;
  while(1){
    fd_set read_set;
    //Espera que haja pelo menos um cliente
    #ifndef DEBUG
    printf("A entrar no ciclo de head->next!=null\n");
    #endif
    while(1){
      if(head->next != NULL) break;
      sleep(2);
    }
    close_client=0;
    current = NULL;
    FD_ZERO(&read_set);
    #ifndef DEBUG
    printf("Ciclo dos FD_SET's\n");
    #endif
    for(current = head->next; current->next!=NULL; current = current->next) FD_SET(current->fd, &read_set);
    FD_SET(current->fd, &read_set);
    #ifndef DEBUG
    printf("A entrar no select\n");
    #endif
    if(select(current->fd+1, &read_set,0,0,0) > 0){
      for(current = head->next; current!=NULL; current = current->next){
        if(FD_ISSET(current->fd, &read_set)){
          nread = read(current->fd, str, sizeof(str));
          str[nread] = '\0';
          if(strcmp(str, "exit") == 0){
            printf("%s terminou conexao.\n", current->nome);
            close_client=1;
            break;
            }
          else if(strcmp(str, "commands")==0) break;
          else{
            printf("%s: %s\n", current->nome, str);
            sendToAll(head, current, current->fd, str);
            break;
          }
        }
      }
    }
    #ifndef DEBUG
    printf("Saiu do select\n");
    #endif
    if(close_client == 1){
      #ifndef DEBUG
      printf("Tentando fechar\n");
      #endif
      close(current->fd);
      #ifndef DEBUG
      printf("Fechou\n");
      printf("Tenta clr\n");
      #endif
      FD_CLR(current->fd, &read_set);
      #ifndef DEBUG
      printf("Tenta remover da lista\n");
      #endif
      remove_from_list(head, current->fd);
      #ifndef DEBUG
      printf("Removeu da lista\n");
      #endif
    }
  }
}

void* acceptClient(void* args){
  struct sockaddr_in client_addr;
  int client_addr_size = sizeof(client_addr);
  char nome[MAX_NOME];

  thread_args* args_ptr = (thread_args*)args;

  int fd;
  while(1){
    fd = accept(args_ptr->fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_size);
    int nread = read(fd, nome, sizeof(nome));
    nome[nread]= '\0';
    printf("Novo cliente conectado - %s\n", nome);
    pthread_mutex_lock(&mutex);
    add_to_list(args_ptr->head, fd, nome, client_addr);
    pthread_mutex_unlock(&mutex);
  }
}

void sendToAll(client* head, client* user, int fd, char str[MAX_BUFFER]){
  client* current;
  char message[MAX_BUFFER];
  sprintf(message,"%s: %s", user->nome, str);
  for(current = head->next; current!=NULL; current = current->next){
    if(current->fd == fd) continue;
    write(current->fd, message, sizeof(message));
  }
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
