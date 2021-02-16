#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

void printDebug(int number) {
  printf("Number: %d\n", number);
}

int main(int argc, const char* argv[]) {
  char seedString[10];
  char textFile[50];
  int id = getpid();

  // Open seed.txt for SRAND
  sprintf(textFile, "seed_slug_%s.txt", argv[1]);
  FILE *file = fopen(textFile, "r");
  
  if(file == NULL) {
    printf("File does not exist...\n");
    return 0;
  }

  int seed = -1;

  // store the text in seedString
  fgets(seedString, sizeof(seedString), file);
  printf("[Slug PID: %d]: Read seed value: %s\n", id, seedString);
  
  // convert string to int
  seed = atoi(seedString);
  printf("[Slug PID: %d]: Read seed value(converted to integer): %d\n", id, seed);

  fclose(file);
  
  srand(seed);

  int wait = rand() % 4 + 1;
  int coin = rand() % 2;

  printf("[Slug PID: %d]: Delay time is %d seconds. Coin flip %d\n", id, wait, coin);
  printf("[Slug PID: %d]: I'll get the job done. Eventually...\n", id);

  sleep(wait);

  if(coin == 0) {

    char *com0[4];
    com0[0] = "last";
    com0[1] = "-d";
    com0[2] = "--fulltimes";
    com0[3] = NULL;

    printf("[Slug: PID %d]: Break time is over! I am running the 'last -d --fulltimes' command.\n", id);
    execvp("last", com0);

  } else {

    char *com1[3];
    com1[0] = "id";
    com1[1] = "-u";
    com1[2] = NULL;
    
    printf("[Slug: PID %d]: Break time is over! I am running the 'id -u' command.\n", id);
    execvp("id",  com1);
  }

  return 0;
}
