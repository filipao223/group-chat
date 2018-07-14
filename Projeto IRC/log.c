#include <stdio.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "log.h"

#define MAX_TIME_STRING 255
#define MAX_LOG_STRING (MAX_TIME_STRING+MAX_MESSAGES)

void log_msg(char* message, char* message_buffer){

  char* time_buffer = malloc(MAX_TIME_STRING);

  //Get time of occurence
  return_time_format(time_buffer);
  strcpy(message_buffer, time_buffer);
  strcat(message_buffer, message); //Append the message

  free(time_buffer); //Frees the time message buffer
}

void return_time_format(char* time_buffer){
  time_t timenow = time(NULL);

  struct tm* timeformatted;
  timeformatted = localtime(&timenow);

  sprintf(time_buffer, "[%d:%d:%d]", timeformatted->tm_hour, timeformatted->tm_min, timeformatted->tm_sec);
}
