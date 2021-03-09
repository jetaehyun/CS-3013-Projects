#include "linked_list.h"

/* --------------------------------------------------------------------------*/
/**
 * @Brief FIFO style
 *
 * @Param node ptr linked list
 *
 * @Returns returns pointer to first item
 */
/* --------------------------------------------------------------------------*/
job_t *pop(job_t **node) {
  job_t *head = *node;
  job_t *cur = head;
  
  *node = (*node)->next;

  
  return cur;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief updates the length of a node
 *
 * @Param node linked list of job_t
 * @Param id identifier for job
 * @Param length new length value for job
 */
/* --------------------------------------------------------------------------*/
void update_job(job_t **node, int id, int length) {
  job_t *cur = *node;

  while(cur != NULL) {
    if(cur->id == id) {
      cur->length = length;
      break;
    }
    cur = cur->next;

  }
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief remove specific job
 *
 * @Param node linked list of jobs
 * @Param id the job to remove
 */
/* --------------------------------------------------------------------------*/
void remove_job(job_t **node, int id) {
  job_t *cur = *node;
  job_t *head = *node;
  job_t *prev = *node;

  if(head->id == id) {
    (*node) = (*node)->next;
    free(head);
    return;
  }

  while(cur != NULL) {
    if(cur->id == id) {
      *node = prev;
      (*node)->next = cur->next;
      (*node) = head;
      free(cur);
      break;
    }
    prev = cur;
    cur = cur->next;
      
  }

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief find the shortest job in the list and pop it. This is strictly for
 *        SJF policy
 *
 * @Param node linked list of jobs
 *
 * @Returns ptr to shortest job
 */
/* --------------------------------------------------------------------------*/
job_t *remove_SJ(job_t **node) {
    job_t *head = *node;    // ptr to head of list
    job_t *cur = *node;     // ptr to current node
    job_t *prev = *node;     // ptr to prev node
    job_t *curRef = *node;   // ptr to shortest length job
    job_t *prevRef = *node;  // ptr to prev node of shortest length job

    int len = INT_MAX;
    while(cur != NULL) {
      if(cur->length < len) { // want jobs that came first
        len = cur->length;
        curRef = cur;
        prevRef = prev;
      }

      prev = cur;
      cur = cur->next;
    }
    
    if(head->id == curRef->id) {
      *node = (*node)->next;
    } else {
      *node = prevRef;
      (*node)->next = curRef->next;
      (*node) = head;
    }

    return curRef;

    
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief check to see if a job exists
 *
 * @Param node linked list of jobs
 * @Param id identifier of job to find
 *
 * @Returns true if exists, false if it doesn't
 */
/* --------------------------------------------------------------------------*/
bool search(job_t **node, int id) {

  job_t *head = *node;

  while(head != NULL) {
    if(head->id == id) {
      return true;
    }
  }

  return false;

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief add node to linked list
 *
 * @Param node linked list of jobs to add to
 * @Param id identifer of job
 * @Param length duration of job
 */
/* --------------------------------------------------------------------------*/
void insert(job_t **node, int id, int length) {

  job_t *cur = *node;
  job_t *head = *node;
  job_t *newNode = (job_t*)malloc(sizeof(job_t));

  newNode->id = id;
  newNode->length = length;
  newNode->next = NULL;

  if(head == NULL) {
    *node = newNode;
    return;
  }

  while(cur->next != NULL) {
    cur = cur->next;
  }

  *node = cur;
  (*node)->next = newNode;
  (*node) = head;

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief debug function, prints out list
 *
 * @Param node linked list of jobs
 */
/* --------------------------------------------------------------------------*/
void printList(job_t **node) {

  job_t *head = *node;

  while(head != NULL) {
    printf("id: %d, length: %d\n", head->id, head->length);

    head = head->next;
  }

}
