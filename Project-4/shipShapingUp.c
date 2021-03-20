#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>

#include "linked_list.h"

// actions: 0 = weighing, 1 = barcoding, 2 = x raying, 3 = jostling

typedef struct _worker_t {
  int workerId;
  char team; // r, b, g, y
  int teamNum;
  bool isLoading;
} worker_t;

// thread and semaphore stuff
pthread_t id[40];
sem_t worker_sem[4];
sem_t package_queue_sem;
sem_t check_stations_sem;

// global variables
node_t *ll;
worker_t **workerList;
char *stationNames[] = {"scale", "barcoder" ,"x-ray", "shaker"};
bool busyStations[4];
int numWorkers; // used to keep track of the current number of workers
int completedR;
int completedB;
int completedG;
int completedY;

// prototype functions
void generatePPP(node_t **queue, int numPackages);
void initWorkers(int numGWorkers, int numRWorkers, int numBWorkers, int numYworkers);
void initSem();
void destSem();
void freeMem();
void setSeed();
node_t *getPackage();
bool checkStations(int *actions);
void setStationStatus(int *actions, bool releaseStation);
void handlePackage(package_t package, worker_t worker);
void *workerThread(void *ptr);

int main() {

  puts("Starting: Shipping Shape-Up");

  setSeed();

  // initialize variables, queue, packages, etc.
  ll = NULL;
  workerList = NULL;
  initSem();
  generatePPP(&ll, 80);
  initWorkers(10, 10, 10, 10);

  for(int i = 0; i < 40; i++)
    pthread_join(id[i], NULL);


  puts("ALL packages have been processed...\n");

  freeMem();
  destSem();

  puts("Results:");
  printf("...Red team:\t%d\n", completedR);
  printf("...Blue team:\t%d\n", completedB);
  printf("...Green team:\t%d\n", completedG);
  printf("...Yellow team:\t%d\n", completedY);


  return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief handle packages and display message
 *
 * @Param package package being worked on
 * @Param worker worker that is dealing with the package
 */
/* --------------------------------------------------------------------------*/
void handlePackage(package_t package, worker_t worker) {

  int *actions = package.actions; // pointer to the actions the worker needs to take at the stations

  for(int i = 0; i < 4; i++) {
    if(actions[i]==0)
      continue;

    printf("[worker: %c id: %2d]:\tTaking package #%d to the %s station...\n", worker.team, worker.workerId, package.id, stationNames[i]);
    sleep(1);
    printf("[worker: %c id: %2d]:\tpackage #%d has finished processing in the %s station...\n", worker.team, worker.workerId, package.id, stationNames[i]);
  }

  printf("[worker: %c id: %2d]:\tpackage #%d has finished processing...\n", worker.team, worker.workerId, package.id);

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief worker thread for teams: red, green, blue, yello
 *
 * @Param ptr data
 *
 * @Returns 1
 */
/* --------------------------------------------------------------------------*/
void *workerThread(void *ptr) {

  worker_t wrker = *(worker_t*)ptr;
  int wrkerNum = wrker.teamNum;

  // loop until there are no more packages
  while(getNumOfNodes() > 0) {
    // only one worker from a team at a time
    sem_wait(&worker_sem[wrkerNum]);

    printf("[worker: %c id: %2d]:\tI'm up next!\n", wrker.team, wrker.workerId);

    node_t *packageNode = getPackage(); // get package from queue
    if(packageNode == NULL) { // no more packages

      printf("[worker: %c id: %2d]:\tThere are no more packages to be processed...\n", wrker.team, wrker.workerId);

    }
    else {

      package_t pkge = (packageNode->data);

      // check to see if the stations can be reserverd
      sem_wait(&check_stations_sem);
      while(!checkStations(pkge.actions))
        sem_wait(&check_stations_sem);
      
      setStationStatus(pkge.actions, false);

      // build message
      printf("[worker: %c id: %2d]:\tI have package# %d, which needs: ", wrker.team, wrker.workerId, pkge.id);
      
      int *actions = pkge.actions;
      for(int i = 0; i < 4; i++) {
        if(actions[i] == 0)
          continue;

        printf("%s ", stationNames[i]);
      }

      puts("");
      
      handlePackage(pkge, wrker);
      setStationStatus(pkge.actions, true);
      
      sem_post(&check_stations_sem);
      printf("[worker: %c id: %2d]:\tMy job is done...\n", wrker.team, wrker.workerId);
      
      // tally up completed packages by team
      if(wrkerNum == 0)      completedG++;
      else if(wrkerNum == 1) completedR++;
      else if(wrkerNum == 2) completedB++;
      else                   completedY++;

    }

    // worker is done, next team member
    sem_post(&worker_sem[wrkerNum]);
    sleep(1);

  }

  return (void*)1;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief check to see what the status of each station is
 *
 * @Param actions
 *
 * @Returns 
 */
/* --------------------------------------------------------------------------*/
bool checkStations(int *actions) {

  for(int i = 0; i < 4; i++) {
    if(actions[i] == 0)
      continue;

    if((actions[i] & !busyStations[i]) != 1) // worker wants to reserve but can't
      return false;
      
  }

  return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief set the values for the stations, are they still being used or not?
 *
 * @Param actions int array of the actions the worker is expected to do for the
 *                package
 * @Param releaseStation 1 = station not reserved, 0 = station is reserved
 */
/* --------------------------------------------------------------------------*/
void setStationStatus(int *actions, bool releaseStation) {
  for(int i = 0; i < 4; i++) {
    if(actions[i] == 0) // skip the stations not being used
      continue;

    if(releaseStation) 
      busyStations[i] = false;
    else
      busyStations[i] = true;
  }

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief create the worker threads for green, red, blue, and yellow
 *
 * @Param numGWorkers number of green workers
 * @Param numRWorkers number of red workers
 * @Param numBWorkers number of blue workers
 * @Param numYworkers number of yellow workers
 */
/* --------------------------------------------------------------------------*/
void initWorkers(int numGWorkers, int numRWorkers, int numBWorkers, int numYworkers) {

  int numRWorkersFlag = numGWorkers + numRWorkers;
  int numBWorkersFlag = numRWorkersFlag + numBWorkers;
  int numYWorkersFlag = numBWorkersFlag + numYworkers;
  numWorkers = numYWorkersFlag;

  workerList = malloc(sizeof(worker_t*) * numYWorkersFlag); // array of worker_t *

  for(int i = 0; i < numYWorkersFlag; i++) {

    worker_t *wrker = malloc(sizeof(worker_t));
    wrker->isLoading = false;
    wrker->workerId = i;

    if(i < numGWorkers) {
      wrker->team = 'g';
      wrker->teamNum = 0;
    }
    else if(i < numRWorkersFlag) {
      wrker->team = 'r';
      wrker->teamNum = 1;
    }
    else if(i < numBWorkersFlag) {
      wrker->team = 'b';
      wrker->teamNum = 2;
    } 
    else if(i < numYWorkersFlag) {
      wrker->team = 'y';
      wrker->teamNum = 3;
    }

    workerList[i] = wrker;
    pthread_create(&id[i], NULL, workerThread, wrker);

  }
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief pop from the queue to get package
 *
 * @Returns ptr to package obj
 */
/* --------------------------------------------------------------------------*/
node_t *getPackage() {

  // protect access to queue
  sem_wait(&package_queue_sem);

  node_t *packageNode = pop(&ll);

  sem_post(&package_queue_sem);

  return packageNode;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief create packages and insert them into queue
 *
 * @Param queue queue data structure to hold packages
 * @Param numPackages number of packages to make
 */
/* --------------------------------------------------------------------------*/
void generatePPP(node_t **queue, int numPackages) {
  
  for(int i = 0; i < numPackages; i++) {
    int id = i;
    int actions[4];

    for(int j = 0; j < 4; j++) {
      actions[j] = rand() % 2; // randomize the required actions needed (e.g. weigh, scan, x-ray, and shake)
    }

    insert(queue, id, actions);
  }

}


/* --------------------------------------------------------------------------*/
/**
 * @Brief initialize semaphore
 */
/* --------------------------------------------------------------------------*/
void initSem() {
  sem_init(&package_queue_sem, 0, 1);
  sem_init(&check_stations_sem, 0, 4);
  for(int i = 0; i < 4; i++) 
    sem_init(&worker_sem[i], 0, 1);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief destroy semaphore 
 */
/* --------------------------------------------------------------------------*/
void destSem() {
  sem_destroy(&package_queue_sem);
  sem_destroy(&check_stations_sem);
  for(int i = 0; i < 4; i++) 
    sem_destroy(&worker_sem[i]);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief free allocated memory
 */
/* --------------------------------------------------------------------------*/
void freeMem() {

  if(ll != NULL)
    freeAll(&ll);

  if(workerList == NULL)
    return;

  for(int i = 0; i < numWorkers; i++)
      free(workerList[i]);

  free(workerList);

}

/* --------------------------------------------------------------------------*/
/**
 * @Brief read seed from .txt
 */
/* --------------------------------------------------------------------------*/
void setSeed() {
  char seedString[10];

  // Open seed.txt for SRAND
  FILE *file = fopen("seed.txt", "r");
  int seed = -1;

  // store the text in seedString
  fgets(seedString, sizeof(seedString), file);
  
  // convert string to int
  seed = atoi(seedString);

  fclose(file);

  if(seed==0) {
    puts("seed.txt either does not contain numbers or there was another issue...look at seed.txt file...");
    puts("Seed value defaulting to: 12345");
    seed=12345;
  }
  
  srand(seed);
}
