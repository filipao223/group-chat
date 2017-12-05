#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "structs.h"

void add_to_list(client*, int, char*, struct sockaddr_in);
void remove_from_list(client*, int);

void add_to_block_list(block_list*, char*);
int check_block(block_list*, char*);

void add_to_history(history*, char*, int);
void remove_from_history(history*, int);
void print_history(history*, int);

#endif //LINKEDLIST_H
