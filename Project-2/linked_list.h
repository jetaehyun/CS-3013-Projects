#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

// struct for jobs
typedef struct _job_t {
  
  int id;
  int length;
  struct _job_t *next;

} job_t;

// function prototypes
void insert(job_t **node, int id, int length);
job_t *pop(job_t **node);
job_t *remove_SJ(job_t **node);
void update_job(job_t **node, int id, int length);
void remove_job(job_t **node, int id);
bool search(job_t **node, int id);
void printList(job_t **node);


#endif
