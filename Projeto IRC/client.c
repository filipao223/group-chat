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

#include "structs.h"
#include "linkedList.h"

void* writeMessages(void*);
void* readMessages(void*);
void* threadWatch();
void options(int, int*);
void erro(char *msg);

char bufOut[MAX_BUFFER];
char messageHistory[MAX_MESSAGES][MAX_BUFFER];
char controlChar[1];
int messageCount=0;

pthread_t thread_read, thread_write;

int main(int argc, char *argv[]){
  char endServer[100];
  int *fd = malloc(sizeof(int));

  struct sockaddr_in addr;
  struct hostent *hostPtr;

  signal(SIGINT, SIG_IGN);

  if(argc != 4){
   printf("cliente <host> <port> <nome>\n");
   exit(-1);
  }

  strcpy(endServer, argv[1]);
  if ((hostPtr = gethostbyname(endServer)) == 0) erro("Nao consegui obter endereço");

  bzero((void *) &addr, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
  addr.sin_port = htons((short) atoi(argv[2]));

  if((*fd = socket(AF_INET,SOCK_STREAM,0)) == -1) erro("socket");
  if( connect(*fd,(struct sockaddr *)&addr,sizeof (addr)) < 0) erro("Connect");
  printf("Conectado.\n");

  write(*fd, argv[3], sizeof(argv[3]));

  //Thread para escrever mensagens
  printf("Waiting for input...\n");
  pthread_create(&thread_write, 0, writeMessages, (void*) fd);

  //Thread para ler mensagens
  pthread_create(&thread_read, 0, readMessages, (void*) fd);
  pthread_join(thread_write, 0);

  printf("A fechar conexao.");
  close(*fd);

  exit(0);
}

void* writeMessages(void* fd){
  int* server_fd = (int*) fd;
  char buf[MAX_BUFFER];

  while(1){
    strcpy(controlChar, "0");
    fgets(buf, MAX_BUFFER, stdin);
    buf[strlen(buf)-1] = '\0';

    //Verifica se pretende fazer outras operaçoes
    if(strcmp(buf, "command_history") == 0){
      options(1, 0);
    }
    else if(strcmp(buf, "command_private") == 0){
      options(2, server_fd);
    }
    else{
      strcpy(bufOut, ""); //Para o servidor saber se a mensagem é privada ou nao
      strcat(bufOut, controlChar); //Control char é 0 se nao for, 1 se for
      strcat(bufOut, "_"); //servidor tera que fazer splits, se for privada, faz mais um split do que se nao for
      strcat(bufOut, buf);

      write(*server_fd, bufOut, sizeof(bufOut));
      //Verifica se pretende acabar a conexao
      if(strcmp(bufOut, "0_exit") == 0){
        printf("A sair\n");
        pthread_kill(thread_read, SIGTERM);
        pthread_exit(NULL);
      }
    }
  }
}

void* readMessages(void* fd){
  int* server_fd = (int*) fd;
  char bufIn[MAX_BUFFER];

  while(1){
    read(*server_fd, bufIn, sizeof(bufIn));
    printf("%s\n", bufIn);
    strcpy(messageHistory[messageCount], bufIn); //Adiciona ao historico de mensagens
    messageCount++;
  }
}

void options(int esc, int* server_fd){
  char nomeDest[MAX_NOME];
  char buf[MAX_BUFFER];
  char message[MAX_BUFFER+MAX_NOME];

  switch(esc){
    case 1:
      printf("Numero de mensagens a ver (entre 1 e 200): ");
      int num;
      while(num<=0 || num>200){
        scanf("%d", &num);
      }
      for(int i=0; i<messageCount; i++){
        if(i==num) break;
        printf("%s\n", messageHistory[i]);
      }
      printf("Fim do historico\n");
      break;

    case 2:
      strcpy(controlChar, "1");

      printf("Nome do destinatario: ");
      fgets(nomeDest, MAX_NOME, stdin);
      nomeDest[strlen(nomeDest)-1]= '\0';
      printf("Mensagem: ");
      fgets(buf, MAX_BUFFER, stdin);
      buf[strlen(buf)-1]= '\0';

      strcpy(message, "");
      strcat(message, controlChar);
      strcat(message, "_");
      strcat(message, buf);
      strcat(message, "_");
      strcat(message, nomeDest);

      write(*server_fd, message, sizeof(message));
      break;
  }
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
