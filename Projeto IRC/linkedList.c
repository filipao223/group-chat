#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include "structs.h"
#include "linkedList.h"

void add_to_list(client* head, int fd, char nome[MAX_NOME], struct sockaddr_in client_addr){
  client* current, *last;

  //Se for o primeiro clientes
  if(head->next == NULL){
    head->next = malloc(sizeof(client));
    head->next->fd = fd;
    head->next->client_addr = client_addr;
    strcpy(head->next->nome, nome);
    head->next->next = NULL;
  }
  else{
    //Não é o primeiro, percorre a lista até ao fim
    for(current = head->next; current!=NULL; current = current->next) last = current;

    last->next = malloc(sizeof(client));
    last->next->fd = fd;
    strcpy(last->next->nome, nome);
    last->next->client_addr = client_addr;
    last->next->next = NULL;
  }
}

void remove_from_list(client* head, int fd){
  client* current, *previous = head, *next = head->next->next;

  for(current = head->next; current!=NULL; next = current->next){
    if(current->fd == fd){
      previous->next = next;
      free(current);
      break;
    }
    previous = current;
    current = next;
  }
}

void add_to_block_list(block_list* head, char nome[MAX_NOME]){
  block_list* current, *last;

  //Se for o primeiro block user
  if(head->next == NULL){
    head->next = malloc(sizeof(block_list));
    strcpy(head->next->nome, nome);
    head->next->next = NULL;
  }
  else{
    //Não é o primeiro, percorre a lista até ao fim
    for(current = head->next; current!=NULL; current = current->next) last = current;

    last->next = malloc(sizeof(block_list));
    strcpy(last->next->nome, nome);
    last->next->next = NULL;
  }
}

int check_block(block_list* head, char nome[MAX_NOME]){
  block_list* current=NULL;
  for(current = head->next; current!=NULL; current = current->next){
    if(strcmp(current->nome, nome) == 0) return 1;
  }
  return 0;
}

void add_to_history(history* head, char message[MAX_NOME+MAX_BUFFER], int messageNumber){
  history *current, *last;

  //Se for a primeira mensagem
  if(head->next == NULL){
    head->next = malloc(sizeof(history));
    head->next->messageNumber = messageNumber;
    strcpy(head->next->message, message);
    head->next->previous = head;
    head->next->next = NULL;
  }
  else{
    //Não é o primeiro, percorre a lista até ao fim
    for(current = head->next; current!=NULL; current = current->next){
      last = current;
    }

    last->next = malloc(sizeof(history));
    last->next->messageNumber = messageNumber;
    strcpy(last->next->message, message);
    last->next->previous = last;
    last->next->next = NULL;
  }
}

void remove_from_history(history* head, int messageID){
  history* current, *previous = head, *next = head->next->next;

  for(current = head->next; current!=NULL; next = current->next){
    if(current->messageNumber == messageID){
      previous->next = next;

      //Verifica se o proximo não é NULL
      if(next != NULL){
        //Não é, logo o previous do proximo sera o previous do atual (o que se quer remover)
        next->previous = current->previous;
      }

      //Não entrou no if, o proximo é NULL, como tal, não tem previous

      free(current);
      break;
    }
    previous = current;
    current = next;
  }
}

char* print_history(history* head, int number){
  history* current, *last;
  int cont=0;
  char *strToReturn = malloc((MAX_NOME+MAX_BUFFER)*MAX_MESSAGES);
  char temp[MAX_BUFFER+MAX_NOME];
  strToReturn[0] = '\0';

  //Percorre a lista ate ao fim
  for(current = head->next; current!=NULL; current = current->next){
    last = current;
  }

  //Imprime ao contrario (mais recentes primeiro)
  for(current = last; current!=head; current = current->previous){
    if(cont == number) break;
    sprintf(temp, "%s", current->message);
    strcat(strToReturn, temp);
    cont++;
  }

  strcat(strToReturn, "Fim do historico");
  return strToReturn;
}
