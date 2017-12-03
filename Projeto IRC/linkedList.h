#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "structs.h"

void add_to_list(client*, int, char*, struct sockaddr_in);
void remove_from_list(client*, int);

void add_to_block_list(block_list*, char*);
int check_block(block_list*, char*);

#endif //LINKEDLIST_H
