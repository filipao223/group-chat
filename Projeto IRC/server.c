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

history* head_list;
int messageID=0;

void* acceptClient(void*);
void erro(char *msg);
void sendToAll(client*, client*, int, char*);
void* sendToOne(client*, client*, char*, int, char*);
void requestNames(client*, int);

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

  //Cria cabeça das listas ligadas
  client* head = malloc(sizeof(client));
  head->next = NULL;
  client* current;
  history* head_list = malloc(sizeof(history));
  head->next = NULL;

  thread_args *args = malloc(sizeof(thread_args));

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
    while(1){
      if(head->next != NULL) break;
      sleep(2);
    }

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

          char *tokens;
          tokens = strtok(str, "_");
          if(tokens == NULL) break;

          if(strcmp(tokens, "1")==0){
            //Conversa privada
            tokens = strtok(NULL, "_");
            char strToSend[MAX_BUFFER];
            strcpy(strToSend, tokens);
            tokens = strtok(NULL, "_");
            char nomeDest[MAX_NOME];
            strcpy(nomeDest, tokens);
            sendToOne(head, current, nomeDest, current->fd, strToSend);
            break;
          }
          else{
            tokens= strtok(NULL, "_");
            if(tokens==NULL) break;
            //Não é privada
            if(strcmp(tokens, "exit") == 0){
              printf("%s terminou conexao.\n", current->nome);
              close_client=1;
              break;
              }
            else if(strcmp(tokens, "requestnames") == 0){ //Cliente fez pedido da lista de users
              printf("Names Requested\n");
              requestNames(head, current->fd);
              break;
            }
            else if(strcmp(tokens, "/history") == 0){ //Utilizador pediu o historico
              int num; //Numero de mensagens
              tokens = strtok(NULL, "_");
              sscanf(tokens, "%d", &num);
              char *strToReturn = print_history(head_list, num);
              write(current->fd, strToReturn, (MAX_NOME+MAX_BUFFER)*MAX_MESSAGES);
              free(strToReturn);
              strToReturn=NULL;
              break;
            }
            else{
              printf("%s: %s\n", current->nome, tokens); //Escreve no terminal
              char strTemp[MAX_BUFFER+MAX_NOME]; sprintf(strTemp, "%s: %s\n", current->nome, tokens); //Formata nome e mensagem numa string
              add_to_history(head_list, strTemp, messageID);
              messageID++;
              sendToAll(head, current, current->fd, tokens); //Envia para todos
              break;
            }
          }
        }
      }
    }

    if(close_client == 1){
      close(current->fd);
      FD_CLR(current->fd, &read_set);
      remove_from_list(head, current->fd);
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

void* sendToOne(client* head, client* user, char dest[MAX_NOME], int fd, char str[MAX_BUFFER]){
  client* current;
  char message[MAX_BUFFER];
  sprintf(message, "P %s: %s", user->nome, str);
  for(current = head->next; current!=NULL; current = current->next){
    if(current->fd == fd) continue;
    if(strcmp(current->nome, dest) == 0){
      write(current->fd, message, sizeof(message));
      return 0;
    }
  }
  return 0;
}

void requestNames(client* head, int fd){
  char names[MAX_BUFFER*MAX_NOME]; strcpy(names, "");
  client* current=NULL;
  for(current = head->next; current!=NULL; current = current->next){
    strcat(names, current->nome);
    if(current->next != NULL) strcat(names, "_");
  }
  write(fd, names, sizeof(names));
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
