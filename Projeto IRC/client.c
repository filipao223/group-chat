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
void erro(char *msg);

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

char bufOut[MAX_BUFFER];

pthread_t thread_read, thread_write;// thread_watch;

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

  //Thread para monitorizar se o cliente pretende sair
  //pthread_create(&thread_watch, 0, threadWatch,0);

  pthread_join(thread_write, 0);
  //pthread_join(thread_watch, 0);
  //pthread_mutex_destroy(&mutex);
  printf("A fechar conexao.");

  close(*fd);
  exit(0);
}

void* writeMessages(void* fd){
  int* server_fd = (int*) fd;

  while(1){
    fgets(bufOut, MAX_BUFFER, stdin);
    bufOut[strlen(bufOut)-1] = '\0';
    write(*server_fd, bufOut, sizeof(bufOut));
    //Verifica se pretende acabar a conexao
    if(strcmp(bufOut, "exit") == 0){
      printf("A sair\n");
      //pthread_cond_signal(&cond_var);
      pthread_kill(thread_read, SIGTERM);
      sleep(4);
      pthread_exit(NULL);
    }
  }
}

void* readMessages(void* fd){
  int* server_fd = (int*) fd;
  int oldstate=0;
  char bufIn[MAX_BUFFER];

  while(1){
    //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
    read(*server_fd, bufIn, sizeof(bufIn));
    printf("Cliente leu\n");
    printf("%s\n", bufIn);
    //pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
  }
}

/*void* threadWatch(){
  pthread_cond_wait(&cond_var, &mutex);
  pthread_cancel(thread_read);
  pthread_exit(NULL);
}*/

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
