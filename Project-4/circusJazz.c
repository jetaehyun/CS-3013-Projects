#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

// performer struct
typedef struct _performer_t {
  char name[10];
  bool leftStage;
  int stage_id;
  int performer_id;
  pthread_t *thread_id;
} performer_t;

// global variables related to pthread
pthread_t id[29];
pthread_mutex_t lock, lock1;
pthread_mutex_t stage_lock[4];
pthread_cond_t stage_open[4];
pthread_cond_t stage_close[4];

// global variables regarding stage and performers
performer_t *performers[25]; // pointer to performers
bool stage[4];               // check to see if stage is open or not
int performersOnStage[4];    // keep track of the number of performers at each stage
char *stagePerformer[4];     // name of the current performer at the stage
char *lastPerformerType;     // name of the last performer type to go on stage
int lastPerformerCount;      // keep track of the number of performers of the same type that went last

// function prototypes
bool allowNextPerformer(char *performer);
void setSeed();
void initPerformers();
void initCondnMutex();
void destCondnMutex();
bool isStageOpen(int stageId);
void setStage(int stageId, bool isOpen, bool hasPerformed);
void freeAll();
void *dancerThread(void *ptr);
void *jugglerThread(void *ptr);
void *soloistThread(void *ptr);
void *stageThread(void *ptr);
int checkStage(char *performer, int stageId);

int main() {
  
  // initialize stuff
  lastPerformerType=malloc(sizeof(char)*8);
  memset(lastPerformerType, 0, 8);
  lastPerformerCount=0;
  initCondnMutex();

  for(int i=0; i<4; i++) {
    stage[i]=true;
    stagePerformer[i] = malloc(sizeof(char)*8);
    strncpy(stagePerformer[i], "EMPTYST", 7);
  }

  // set seed for rand()
  setSeed();

  puts("Performers are entering the stage...");
  //start performers/threads
  initPerformers();
 
  for(int i=0; i<29; i++) 
    pthread_join(id[i], NULL);

  // clean up
  freeAll();
  destCondnMutex();

  puts("Performance is done...");
  return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief thread for dancer
 *
 * @Param ptr performer data
 *
 * @Returns 
 */
/* --------------------------------------------------------------------------*/
void *dancerThread(void *ptr) {
  performer_t *perf = (performer_t*)ptr;

  pthread_mutex_lock(&stage_lock[perf->stage_id]);
  pthread_cond_wait(&stage_open[perf->stage_id], &stage_lock[perf->stage_id]);

  while(checkStage(perf->name, perf->stage_id)==-1 || !allowNextPerformer(perf->name)) { // check if performance is allowed
    setStage(perf->stage_id, true, false);
    pthread_cond_signal(&stage_close[perf->stage_id]);
    pthread_cond_wait(&stage_open[perf->stage_id], &stage_lock[perf->stage_id]);
  }

  printf("[Name: %s, ID: %2d] Entering stage#: %d\n", perf->name, perf->performer_id, perf->stage_id);
  sleep(rand()%5+1);
  printf("[Name: %s, ID: %2d] Exiting stage#: %d\n", perf->name, perf->performer_id, perf->stage_id);
  perf->leftStage = true;
  
  setStage(perf->stage_id, true, true);
  pthread_cond_signal(&stage_close[perf->stage_id]);
  pthread_mutex_unlock(&stage_lock[perf->stage_id]);
  pthread_exit(perf->thread_id);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief juggler thread
 *
 * @Param ptr juggler data
 *
 * @Returns 
 */
/* --------------------------------------------------------------------------*/
void *jugglerThread(void *ptr) {
  performer_t *perf = (performer_t*)ptr;

  pthread_mutex_lock(&stage_lock[perf->stage_id]);
  pthread_cond_wait(&stage_open[perf->stage_id], &stage_lock[perf->stage_id]);

  while(checkStage(perf->name, perf->stage_id)==-1 || !allowNextPerformer(perf->name)) {
    setStage(perf->stage_id, true, false);
    pthread_cond_signal(&stage_close[perf->stage_id]);
    pthread_cond_wait(&stage_open[perf->stage_id], &stage_lock[perf->stage_id]);
  }

  printf("[Name: %s, ID: %2d] Entering stage#: %d\n", perf->name, perf->performer_id, perf->stage_id);
  sleep(rand()%5+1);
  //sleep(1);
  printf("[Name: %s, ID: %2d] Exiting stage#: %d\n", perf->name, perf->performer_id, perf->stage_id);
  perf->leftStage = true;

  setStage(perf->stage_id, true, true);
  pthread_cond_signal(&stage_close[perf->stage_id]);
  pthread_mutex_unlock(&stage_lock[perf->stage_id]);
  pthread_exit(perf->thread_id);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief soloist thread
 *
 * @Param ptr soloist data
 *
 * @Returns 
 */
/* --------------------------------------------------------------------------*/
void *soloistThread(void *ptr) {
  performer_t *perf = (performer_t*)ptr;

  pthread_mutex_lock(&stage_lock[perf->stage_id]);
  pthread_cond_wait(&stage_open[perf->stage_id], &stage_lock[perf->stage_id]);

  while(checkStage(perf->name, perf->stage_id)==-1 || !allowNextPerformer(perf->name)) {
    setStage(perf->stage_id, true, false);
    pthread_cond_signal(&stage_close[perf->stage_id]);
    pthread_cond_wait(&stage_open[perf->stage_id], &stage_lock[perf->stage_id]);
  }

  printf("[Name: %s, ID: %2d] Entering stage#: %d\n", perf->name, perf->performer_id, perf->stage_id);
  sleep(rand()%5+1);
  //sleep(1);
  printf("[Name: %s, ID: %2d] Exiting stage#: %d\n", perf->name, perf->performer_id, perf->stage_id);
  perf->leftStage = true;

  setStage(perf->stage_id, true, true);
  pthread_cond_signal(&stage_close[perf->stage_id]);
  pthread_mutex_unlock(&stage_lock[perf->stage_id]);
  pthread_exit(perf->thread_id);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief stage thread; used to send signals to the approriate performer thread
 *                      that the stage is open for them to try to take
 *
 * @Param ptr which stage they will signal
 *
 * @Returns 
 */
/* --------------------------------------------------------------------------*/
void *stageThread(void *ptr) {

  int stageId = *(int*)ptr;

  while(true) { // will break when all performers are done
    pthread_mutex_lock(&stage_lock[stageId]);

    while(!isStageOpen(stageId)) // if stage is being occupied, then wait()
      pthread_cond_wait(&stage_close[stageId], &stage_lock[stageId]);
    
    if(performersOnStage[stageId]<1) { // all expected performers for this stage are done; break and exit
      pthread_mutex_unlock(&stage_lock[stageId]);
      break;
    }

    // occupy stage and wake one of the performer threads
    setStage(stageId, false, false);
    pthread_cond_signal(&stage_open[stageId]);
    pthread_mutex_unlock(&stage_lock[stageId]);
  }

  int ref = stageId;
  free((int*)ptr);
  pthread_exit(&id[ref+25]);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief check to see if a performer should be allowed to perform. This 
 *        function is primarily used to ensure diversity in the performances.
 *        If the same type of performer tries to perform and they have exceeded
 *        the count, then they will have to wait (nap)
 *
 * @Param performer performer expecting to perform
 *
 * @Returns true if allowed, false if not
 */
/* --------------------------------------------------------------------------*/
bool allowNextPerformer(char *performer) {

  int typeLimit = 0;
  bool allowPerformer = true;

  // determine type limit by checking the performer type
  if(strncmp(performer, "soloist", 7)==0) typeLimit = 1;
  else {
    for(int i=0; i<25; i++) {
      if(!performers[i]->leftStage && strncmp(lastPerformerType, performers[i]->name, 7)==0) {
        typeLimit++;
        if(typeLimit>3)
          break;
      }
    }
  }

  // protect global variables being accessed
  pthread_mutex_lock(&lock);
  
  if(strncmp(performer, lastPerformerType, 7)==0) { // new performer is the same as the last one
    if(lastPerformerCount<typeLimit) {              // less than 4 performers of that type of been on stage
      lastPerformerCount++;
    } 
    else { // exceeded amount
      allowPerformer = false;
    }
  } 
  else if(strncmp(performer, lastPerformerType, 7)!=0 && lastPerformerCount>typeLimit) { // different performer and the lastPerformer count has been exceeded, time for new performer
    memset(lastPerformerType, 0, 8);
    lastPerformerCount=0;
    strncpy(lastPerformerType, performer, 7);
  }
  else {
    strncpy(lastPerformerType, performer, 7);
  }

  pthread_mutex_unlock(&lock);

  return allowPerformer;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief Check if a performer is safe to go on stage
 *
 * @Param performer expected performer
 * @Param stageId the stage they are going on
 *
 * @Returns 1 if good, -1 not good
 */
/* --------------------------------------------------------------------------*/
int checkStage(char *performer, int stageId) {

  // protect shared data
  pthread_mutex_lock(&lock);

  int status=1;
  for(int i=0; i<4; i++) {

    if(i==stageId)
      continue;
    else if(status==-1) 
      break;
    
    if(strncmp(stagePerformer[i], "soloist", 7) == 0) {
      status=-1;
    }
    else if(strncmp(stagePerformer[i], "dancers", 7) == 0 &&
        strncmp(performer, "juggler", 7) == 0) { // juggler can't be with dancer
      status=-1;
    }
    else if(strncmp(stagePerformer[i], "juggler", 7) == 0 &&
        strncmp(performer, "dancers", 7) == 0) { // dancer can't be with juggler
      status=-1;
    }
  }

  // stage is closed(means it just opened up)
  if(status==1&&!isStageOpen(stageId)){
    strncpy(stagePerformer[stageId], performer, 7);
  }

  pthread_mutex_unlock(&lock);
  return status;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief simple function to check if stage is opn
 *
 * @Param stageId stage to look at
 *
 * @Returns 
 */
/* --------------------------------------------------------------------------*/
bool isStageOpen(int stageId) {
  if(stage[stageId]) return true;
  
  return false;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief modify stage settings
 *
 * @Param stageId stage to modify
 * @Param isOpen open or close stage
 * @Param hasPerformed has a performance just been completed?
 */
/* --------------------------------------------------------------------------*/
void setStage(int stageId, bool isOpen, bool hasPerformed) {

  pthread_mutex_lock(&lock);

  if(isOpen&&hasPerformed) { // performance has just been completed so one less performer expected
    performersOnStage[stageId]--;
  }

  if(isOpen) { // change performer name on stage to empty
    memset(stagePerformer[stageId], 0, 8);
    strncpy(stagePerformer[stageId], "EMPTYST", 7);

  }
  stage[stageId] = isOpen;
  pthread_mutex_unlock(&lock);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief Initialize threads and data
 */
/* --------------------------------------------------------------------------*/
void initPerformers() {



  for(int i = 0; i < 25; i++) {

    performer_t *perf = malloc(sizeof(performer_t));
    memset(perf->name, 0, 10);
    int stageToGo = rand() % 4;

    // randomly determine performer placement
    if(stageToGo==0&&performersOnStage[0]>5) stageToGo++;
    else if(stageToGo==0) performersOnStage[0]++;
    if(stageToGo==1&&performersOnStage[1]>5) stageToGo++;
    else if(stageToGo==1) performersOnStage[1]++;
    if(stageToGo==2&&performersOnStage[2]>5) stageToGo++;
    else if(stageToGo==2) performersOnStage[2]++;
    if(stageToGo==3&&performersOnStage[3]>5) {
      stageToGo = rand() % 4;
      performersOnStage[stageToGo]++;
    }
    else if(stageToGo==3) performersOnStage[stageToGo]++;

    perf->stage_id = stageToGo;
    perf->thread_id = &id[i];
    perf->performer_id = i;
    perf->leftStage = false;

    if(i<15) {
      strncpy(perf->name, "dancers", 7);
      pthread_create(&id[i], NULL, dancerThread, (void*)perf);
    } else if(i<23) {
      strncpy(perf->name, "juggler", 7);       
      pthread_create(&id[i], NULL, jugglerThread, (void*)perf);
    } else {
      strncpy(perf->name, "soloist", 7);       
      pthread_create(&id[i], NULL, soloistThread, (void*)perf);
    }
    performers[i] = perf;
  }

  // create the 4 stages
  for(int i=0; i<4; i++) {
    int *stageId = malloc(sizeof(int));
    *stageId = i;
    pthread_create(&id[i+25], NULL, stageThread, (void*)stageId);
  }
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief free allocated data
 */
/* --------------------------------------------------------------------------*/
void freeAll() {

  for(int i=0; i<4; i++) {
    free(stagePerformer[i]);
  }

  for(int i=0; i<25; i++) {
    free(performers[i]);
  }

  free(lastPerformerType);
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

/* --------------------------------------------------------------------------*/
/**
 * @Brief initialized condition and mutex primitives
 */
/* --------------------------------------------------------------------------*/
void initCondnMutex() {
  pthread_mutex_init(&lock, NULL);
  pthread_mutex_init(&lock1, NULL);
  for(int i=0; i<4; i++) {
    pthread_mutex_init(&stage_lock[i], NULL);
    pthread_cond_init(&stage_open[i], NULL);
    pthread_cond_init(&stage_close[i], NULL);
  }
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief destry condition and mutex primitives
 */
/* --------------------------------------------------------------------------*/
void destCondnMutex() {
  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock1);
  for(int i=0; i<4; i++) {
    pthread_mutex_destroy(&stage_lock[i]);
    pthread_cond_destroy(&stage_open[i]);
    pthread_cond_destroy(&stage_close[i]);

  }
}
