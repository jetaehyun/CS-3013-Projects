#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "linked_list.h"


// arg: policy name, job trace, and time slice
//
// policies: FIFO, SJF, RR

// function prototypes
void FIFO(job_t **node, int length);
void SJF(job_t **node, int length);
void RR(job_t **node, int length, int timeSlice);
void FIFO_Analyze(job_t **node, int length);
void SJF_Analyze(job_t **node, int length);
void RR_Analyze(job_t **node, int length, int timeSlice);
int populateJobs(char *file, job_t **node);

int main(int argc, char **argv){

  job_t *node = NULL;                         // create pointer to linked list of jobs
  int length = populateJobs(argv[2], &node);  // If file exists, populate linked list and return the number of jobs

  if(length == -1) { // this means that the file does not exist
    return 0;
  }

  if (strncmp(argv[1], "FIFO", 4) == 0) {

    FIFO(&node, length);                    // FIFO policy implementation
    node = NULL;
    length = populateJobs(argv[2], &node);  // repopulate linked list of jobs since FIFO() will finish jobs and free it to simulate job executions
    FIFO_Analyze(&node, length);            // FIFO policy metrics

  } else if(strncmp(argv[1], "SJF", 3) == 0){ 
    
    SJF(&node, length); 
    length = populateJobs(argv[2], &node);
    SJF_Analyze(&node, length);

  } else if(strncmp(argv[1], "RR", 2) == 0) {
    
    RR(&node, length, atoi(argv[3]));
    length = populateJobs(argv[2], &node);
    RR_Analyze(&node, length, atoi(argv[3]));
  
  } else {
    
    puts("Incorrect input(s)");
    puts("Ex: ./scheduler [FIFO/SJF/RR] [NUMBER]");
  
  }

  return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief Display metrics on FIFO policy
 *
 * @Param node linked list of jobs
 * @Param length number of jobs
 */
/* --------------------------------------------------------------------------*/
void FIFO_Analyze(job_t **node, int length) {
  puts("Begin analyzing FIFO:");

  float turnaroundTot = 0.0, respTimeTot = 0.0, waitTot = 0.0; // variables to hold sum of turnaround, response time, and wait time
  float respTime = 0.0, turnaround = 0.0;
  

  for(int i = 0; i < length; i++) {
    job_t *job = pop(node);

    respTime = turnaround;
    turnaround += job->length;
    
    // add up the metrics
    turnaroundTot += turnaround;
    respTimeTot += respTime;
    waitTot += respTime;

    printf("Job %d -- Response time: %.0f  Turnaround: %.0f  Wait: %.0f\n", job->id, respTime, turnaround, respTime);

    free(job);
  }

  // calculate the avgs
  float turnaroundAvg = turnaroundTot / length;
  float respTimeAvg = respTimeTot / length;
  float waitAvg = waitTot / length;
  printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", respTimeAvg, turnaroundAvg, waitAvg);

  puts("End analyzing FIFO.");

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief Display metrics for SJF
 *
 * @Param node linked list of jobs
 * @Param length number of jobs
 */
/* --------------------------------------------------------------------------*/
void SJF_Analyze(job_t **node, int length) {
  puts("Begin analyzing SJF:");

  // variables to sum up turnaround time, response time, and wait time
  float turnaroundTot = 0.0, respTimeTot = 0.0, waitTot = 0.0;
  float respTime = 0.0, turnaround = 0.0;


  // keep track of number of running jobs 
  int len = length; 
  while(len > 0) {

    job_t *job = remove_SJ(node); // get a ptr to the job with the smallest length
    respTime = turnaround;
    turnaround += job->length;
    
    // sum up the metrics
    turnaroundTot += turnaround;
    respTimeTot += respTime;
    waitTot += respTime;

    printf("Job %d -- Response time: %.0f  Turnaround: %.0f  Wait: %.0f\n", job->id, respTime, turnaround, respTime);
    
    free(job);
    len--;
  }

  // calculate the avgs
  float turnaroundAvg = turnaroundTot / length;
  float respTimeAvg = respTimeTot / length;
  float waitAvg = waitTot / length;
  printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", respTimeAvg, turnaroundAvg, waitAvg);

  puts("End analyzing SJF.");

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief Display metrics for round robin
 *
 * @Param node linked list of jobs
 * @Param length number of jobs
 * @Param timeSlice length of timeslice in seconds
 */
/* --------------------------------------------------------------------------*/
void RR_Analyze(job_t **node, int length, int timeSlice) {
  puts("Begin analyzing RR:");

  job_t *cur = *node; // ptr to first job
  
  // keep track of each jobs response time
  float respTime[length];
  memset(respTime, 0, sizeof(float)*length);

  // keep track of each jobs turnaround time
  float turnaroundTime[length];
  memset(turnaroundTime, 0, sizeof(float)*length);

  // keep track of each jobs wait time
  float waitTime[length];
  memset(waitTime, 0, sizeof(float)*length);

  // keep a reference of the number of seconds each job did not run
  float waitRefTime[length];
  memset(waitRefTime, 0, sizeof(float)*length);

  // set the elements to -1
  for(int i = 0; i < length; i++) {
    respTime[i] = -1;
    turnaroundTime[i] = 0;
    waitTime[i] = -1;
  }

  int len = length;
  int elapsedTime = 0; // variable to keep track of the elapsed wait time during the entire process
  // terminate when all jobs are completed
  while(len > 0) {
      
    int sliced = cur->length - timeSlice; // variable to check if a job can be completed in a single time slice
    job_t *next = cur->next;              
    int id = cur->id;
 
    // check to see if we wait on the duration of the time slice or if the job can finish before
    int w = 0;
    if(sliced > 0) w = timeSlice;
    else           w = cur->length;
    
    // sum elapsed time, calculate the time the job didn't run, and calc the wait time of the job
    elapsedTime += w;
    waitRefTime[id] += w;
    waitTime[id] = elapsedTime - waitRefTime[id];


    turnaroundTime[id] = elapsedTime;
    if(sliced < 1) { // job finished
      
      remove_job(node, id);
      len--;

    } else { // job not finished so update
      update_job(node, id, sliced);
    }

    // set the response time of the job during firstrun
    if(respTime[id] == -1) { 
      respTime[id] = waitTime[id];
    }

    cur = next;
    if(cur == NULL) { // if ptr to list at the end, then reset ptr back to head
      cur = *node;
    } 
  }


  float turnaroundTot = 0.0, respTimeTot = 0.0, waitTot = 0.0;

  for(int i = 0; i < length; i++) {
    turnaroundTot += turnaroundTime[i];
    respTimeTot += respTime[i];
    waitTot += waitTime[i];  
    printf("Job %d -- Response time: %.0f  Turnaround: %.0f  Wait: %.0f\n", i, respTime[i], turnaroundTime[i], waitTime[i]);
  }

  float turnaroundAvg = turnaroundTot / length;
  float respTimeAvg = respTimeTot / length;
  float waitAvg = waitTot / length;
  printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", respTimeAvg, turnaroundAvg, waitAvg);
  
  puts("End analyzing RR.");
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief FIFO policy implementation
 *
 * @Param node linked list of jobs
 * @Param length number of jobs
 */
/* --------------------------------------------------------------------------*/
void FIFO(job_t **node, int length) {

  puts("Execution trace with FIFO:");

  for(int i = 0; i < length; i++) {
    job_t *job = pop(node);
    printf("Job %d ran for: %d\n", job->id, job->length);
    free(job);
  }

  puts("End of execution with FIFO.");

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief SJF policy implementation
 *
 * @Param node linked list of jobs
 * @Param length number of jobs
 */
/* --------------------------------------------------------------------------*/
void SJF(job_t **node, int length) {
  puts("Execution trace with SJF:");
  
  int len = length;

  while(len > 0) {
    job_t *job = remove_SJ(node);
    printf("Job %d ran for: %d\n", job->id, job->length);
    free(job);
    len--;
  }

  puts("End of execution with SJF.");

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief round robin policy implementation
 *
 * @Param node linked list of jobs
 * @Param length number of jobs
 * @Param timeSlice duration of time slice
 */
/* --------------------------------------------------------------------------*/
void RR(job_t **node, int length, int timeSlice) {
    job_t *cur = *node; // ptr to head

    puts("Execution trace with RR:");
    int len = length;
    while(len > 0) {
      
      int sliced = cur->length - timeSlice; // used to see if job can finish or not
      job_t *next = cur->next;


      if(sliced < 1) { // job finished

        if(cur->length != 0) {
          printf("Job %d ran for: %d\n", cur->id, cur->length);
        }

        remove_job(node, cur->id);
        len--;

      } else { // job not finished

        update_job(node, cur->id, sliced);
        printf("Job %d ran for: %d\n", cur->id, timeSlice);

      }

      cur = next;
      if(cur == NULL) { // at end of list, so reset ptr to head
        cur = *node;
      } 
    }
    
    puts("End of execution with RR.");
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief fill linked list with jobs provided
 *
 * @Param file name of file
 * @Param node ptr to empty linked list
 *
 * @Returns length of jobs if file exists, -1 if file does not exist
 */
/* --------------------------------------------------------------------------*/
int populateJobs(char *file, job_t **node) {
  
  FILE *fptr = fopen(file, "r");
  char job_time[10];

  if(fptr == NULL) {
    puts("No such file...\n");
    return -1;
  }

  int id = 0;
  while(fgets(job_time, 10, fptr) != NULL) {
  
    if(strncmp(job_time, "\n", 1) == 0) continue;
    int time = atoi(job_time);
    insert(node, id++, time);

  }

  fclose(fptr);
  return id;

}

