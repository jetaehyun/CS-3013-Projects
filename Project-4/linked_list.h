#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct _package_t {
  int id;
  int actions[4];
} package_t;

typedef struct _node_t {
  package_t data;
  struct _node_t *next;
} node_t;

node_t *pop(node_t **node);
bool findNode(node_t **node, int package_id);
void insert(node_t **node, int id, int *actions);
void printList(node_t **node);
void freeAll(node_t **node);
int getNumOfNodes();


#endif
