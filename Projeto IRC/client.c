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
void options(int);
void erro(char *msg);

char bufOut[MAX_BUFFER];
char messageHistory[MAX_MESSAGES][MAX_BUFFER];
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

  while(1){
    fgets(bufOut, MAX_BUFFER, stdin);
    bufOut[strlen(bufOut)-1] = '\0';
    //Verifica se pretende fazer outras operaçoes
    if(strcmp(bufOut, "commands") == 0){
      printf("OPCOES:\n\t1. Ver historico de mensagens\n\t2.Voltar");
      int esc=-1;
      while(esc<=0||esc>2){
        scanf("%d", &esc);
      }
      options(esc);
    }
    write(*server_fd, bufOut, sizeof(bufOut));
    //Verifica se pretende acabar a conexao
    if(strcmp(bufOut, "exit") == 0){
      printf("A sair\n");
      pthread_kill(thread_read, SIGTERM);
      pthread_exit(NULL);
    }
  }
}

void* readMessages(void* fd){
  int* server_fd = (int*) fd;
  int oldstate=0;
  char bufIn[MAX_BUFFER];

  while(1){
    read(*server_fd, bufIn, sizeof(bufIn));
    printf("%s\n", bufIn);
    strcpy(messageHistory[messageCount], bufIn); //Adiciona ao historico de mensagens
    messageCount++;
  }
}

void options(int esc){
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
      break;
  }
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
