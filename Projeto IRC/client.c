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
void options(int, int);
void blockUser(char*);
void erro(char *msg);

char bufOut[MAX_BUFFER];
char names[MAX_BUFFER*MAX_NOME];
int messageID=0;

block_list* head_block = NULL;

pthread_t thread_read, thread_write, thread_delay;

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

  write(*fd, argv[3], strlen(argv[3])+1);

  //Inicia a lista de utilizadores bloqueados
  head_block = malloc(sizeof(block_list));
  head_block->next = NULL;

  //Argumentos da thread para escrever mensganes
  thread_args_write* args = malloc(sizeof(thread_args_write));
  args->fd = *fd;
  strcpy(args->argv, argv[3]);

  //Thread para escrever mensagens
  printf("Waiting for input...\n");
  pthread_create(&thread_write, 0, writeMessages, (void*) args);

  //Thread para ler mensagens
  pthread_create(&thread_read, 0, readMessages, (void*) fd);
  pthread_join(thread_read, 0);

  printf("Servidor desligou.\n");
  close(*fd);

  exit(0);
}

void* writeMessages(void* args){
  thread_args_write* ptr = (thread_args_write*) args;
  int server_fd = ptr->fd;
  char buf[MAX_BUFFER];
  char argv[MAX_NOME];
  strcpy(argv, ptr->argv); //Nome do utilizador, será usado em blockUser()

  while(1){
    fgets(buf, MAX_BUFFER, stdin); //Recebe a mensagem do terminal
    buf[strlen(buf)-1] = '\0';

    //Verifica se a mensagem é um comando
    if(strcmp(buf, "/history") == 0){
      options(1, server_fd);
    }
    else if(strcmp(buf, "/private") == 0){
      options(2, server_fd);
    }
    else if(strcmp(buf, "/timed") == 0){
      options(3, server_fd);
    }
    else if(strcmp(buf, "/block") == 0){
      strcpy(names,"");
      strcpy(bufOut, "requestnames");

      write(server_fd, bufOut, strlen(bufOut)+1); //Pede ao servidor os nomes
      read(server_fd, names, sizeof(names)); //Recebe do servidor os nomes

      blockUser(argv);
    }
    else{
      strcpy(bufOut, buf);

      write(server_fd, bufOut, strlen(bufOut)+1);

      //Verifica se pretende acabar a conexao
      if(strcmp(bufOut, "/exit") == 0){
        printf("A sair\n");
        pthread_kill(thread_read, SIGTERM);
        pthread_exit(NULL);
      }
    }
  }
}

void* readMessages(void* fd){
  int* server_fd = (int*) fd;
  char bufIn[(MAX_BUFFER+MAX_NOME)*MAX_MESSAGES], temp[(MAX_BUFFER+MAX_NOME)*MAX_MESSAGES];

  while(1){
    int nread = read(*server_fd, bufIn, sizeof(bufIn)); //Lê do servidor
    if(nread == 0) break;
    bufIn[nread] = '\0';
    strcpy(temp, bufIn);

    //Verifica se o user esta bloqueado
    char* tokens = strtok(temp, ":");
    if(tokens!=NULL){
      if(check_block(head_block, tokens) == 1) continue;
    }

    printf("%s\n", bufIn);
  }

  pthread_exit(NULL);
}

void options(int esc, int server_fd){
  char nomeDest[MAX_NOME];
  char buf[MAX_BUFFER];
  char message[MAX_BUFFER+MAX_NOME];
  int delay_time=-1;
  char temp[2];
  char temp_num_msg[10];
  char request_history[50];
  char temp_sleep[6];

  switch(esc){
    case 1: //historico
      printf("Numero de mensagens a ver (entre 1 e 200): ");
      int num = 0;
      while(num<=0 || num>200){ //Limite de mensagens
        scanf("%d", &num);
      }
      strcpy(request_history, "/history_"); //Escreve o comando que vai enviar ao servidor
      sprintf(temp_num_msg, "%d", num);
      strcat(request_history, temp_num_msg);
      write(server_fd, request_history, strlen(request_history)+1);
      break;

    case 2: //Mensagem privada
      printf("Nome do destinatario: ");
      fgets(nomeDest, MAX_NOME, stdin);
      nomeDest[strlen(nomeDest)-1]= '\0';

      printf("Mensagem: ");
      fgets(buf, MAX_BUFFER, stdin);
      buf[strlen(buf)-1]= '\0';

      strcpy(message, "/private_");
      strcat(message, buf);
      strcat(message, "_");
      strcat(message, nomeDest);

      write(server_fd, message, strlen(message)+1);
      break;

    case 3: //Mensagem com temporizador
      printf("Tempo: ");
      while(delay_time<1){
          fgets(temp, 10, stdin);
          sscanf(temp, "%d", &delay_time);
      }
      printf("Mensagem: ");
      fgets(buf, MAX_BUFFER, stdin);
      buf[strlen(buf)-1] = '\0';

      strcpy(message, "/timed_");
      strcat(message, buf); strcat(message, "_"); sprintf(temp_sleep, "%d", delay_time);
      strcat(message, temp_sleep);

      write(server_fd, message, strlen(message)+1);
      break;
  }
}

void blockUser(char argv[MAX_NOME]){
  char name_to_block[MAX_NOME]; strcpy(name_to_block, "");
  char* tokens = strtok(names, "_");

  //Escreve os nomes no ecrã
  while(tokens!=NULL){
    printf("\t%s\n", tokens); fflush(stdout);
    tokens = strtok(NULL, "_");
  }

  //Pede um nome de utilizador para ser bloqueado, tem de ser diferente do utilizador
  while((strcmp(name_to_block, "")==0) || (strcmp(name_to_block, argv) == 0)){
    printf("Bloquear que utilizador? ('exit' para cancelar): ");
    fgets(name_to_block, MAX_NOME, stdin);
    name_to_block[strlen(name_to_block)-1] = '\0';
  }

  tokens = strtok(names, "_");
  while(tokens!=NULL){
    if(strcmp(tokens, name_to_block) == 0){
      add_to_block_list(head_block, name_to_block);
      printf("Utilizador '%s' foi bloqueado.", name_to_block);
      break;
    }
  }
}

void erro(char *msg)
{
  printf("Erro: %s\n", msg);
  exit(-1);
}
