#include "linked_list.h"

// keep track of the number of nodes
int numOfNodes;

/* --------------------------------------------------------------------------*/
/**
 * @Brief get the number of nodes in the queue
 *
 * @Returns number of nodes
 */
/* --------------------------------------------------------------------------*/
int getNumOfNodes() {
  return numOfNodes;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief FIFO
 *
 * @Param node queue
 *
 * @Returns ptr to popped node
 */
/* --------------------------------------------------------------------------*/
node_t *pop(node_t **node) {

  if(*node == NULL)
    return NULL;

  node_t *cur = *node;
  (*node) = (*node)->next;
  numOfNodes--;

  return cur;
}


/* --------------------------------------------------------------------------*/
/**
 * @Brief used to see if a package exists already
 *
 * @Param node queue
 * @Param package_id package identifier
 *
 * @Returns true = package exists, false = package does not exist
 */
/* --------------------------------------------------------------------------*/
bool findNode(node_t **node, int package_id) {

  if(*node==NULL)
    return false;

  node_t *cur = *node;

  while(cur != NULL) {
    package_t curPackage = cur->data;
    if(curPackage.id == package_id) {
      return true;
    }

    cur = cur->next;
  }

  return false;
}


/* --------------------------------------------------------------------------*/
/**
 * @Brief insert new node into queue
 *
 * @Param node queue
 * @Param id package identifier
 * @Param actions actions needed to be taken to process package
 */
/* --------------------------------------------------------------------------*/
void insert(node_t **node, int id, int *actions) {

  if(findNode(node, id)) {
    puts("Package already exists...");
    return;
  }

  package_t package;
  package.id = id;

  for(int i = 0; i < 4; i++) {
    package.actions[i] = actions[i];
  }

  node_t *newNode = malloc(sizeof(node_t));
  newNode->next = NULL;
  newNode->data = package;

  if(*node == NULL) {

    *node = newNode;
    numOfNodes = 0;

  } else {

    node_t *cur = *node;
    while(cur->next != NULL) {
      cur = cur->next;
    }

    cur->next = newNode;  

  }

  numOfNodes++;

}


/* --------------------------------------------------------------------------*/
/**
 * @Brief Debug print
 *
 * @Param node queue
 */
/* --------------------------------------------------------------------------*/
void printList(node_t **node) {
  if(*node == NULL) {
    puts("List is empty...");
    return;
  }

  node_t *cur = *node;

  while(cur != NULL) {
    printf("Package Id: %d, actions: ", cur->data.id);

    for(int i = 0; i < 4; i++) {
      printf("%d ", cur->data.actions[i]);
    }

    cur = cur->next;
  }
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief free queue
 *
 * @Param node queue
 */
/* --------------------------------------------------------------------------*/
void freeAll(node_t **node) {
  
  node_t *cur = *node;

  while(cur != NULL) {
    cur = pop(node);
    free(cur);
  }
  numOfNodes = 0;

}
