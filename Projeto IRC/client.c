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
void erro(char *msg);

int main(int argc, char *argv[]){
  char endServer[100];
  int *fd = malloc(sizeof(int));

  pthread_t thread_read, thread_write;

  struct sockaddr_in addr;
  struct hostent *hostPtr;

  if(argc != 3){
   printf("cliente <host> <port>\n");
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

  //Thread para escrever mensagens
  printf("Waiting for input...\n");
  pthread_create(&thread_write, 0, writeMessages, (void*) fd);

  pthread_join(thread_write, 0);
  printf("A fechar conexao.");

  close(*fd);
  exit(0);
}

void* writeMessages(void* fd){
  int* server_fd = (int*) fd;
  char buf[MAX_BUFFER];

  while(1){
    fgets(buf, MAX_BUFFER, stdin);
    buf[strlen(buf)-1] = '\0';
    write(*server_fd, buf, sizeof(buf));
    //Verifica se pretende acabar a conexao
    if(strcmp(buf, "exit") == 0){
      printf("A sair\n");
      sleep(4);
      pthread_exit(NULL);
    }
  }
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
