#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int *numForChildren = NULL;

void printDebug(int number) {
  printf("Number: %d\n", number);
}

int main(int argc, const char* argv[]) {
  char seedString[10];

  // Open seed.txt for SRAND
  FILE *file = fopen("seed.txt", "r");
  int seed = -1;

  // store the text in seedString
  fgets(seedString, sizeof(seedString), file);
  printf("Read seed value: %s\n", seedString);
  
  // convert string to int
  seed = atoi(seedString);
  printf("Read seed value(converted to integer): %d\n", seed);

  fclose(file);
  
  srand(seed);

  // Get number of children in 10 - 15 range and allocate memory to store random values for each child
  int children = rand() % 6 + 10;
  numForChildren = malloc(sizeof(int) * children);
  printf("Random Child Count: %d\n", children);
  printf("I'm feeling prolific!\n");
 
  for(int i = 0; i < children; i++) {
    numForChildren[i] = rand();
  }

  // fork() stuff
  for(int i = 0; i < children; i++) {
    
    int process = fork();

    if(process != 0) { // Parent Code

      int status = 0;

      printf("[Parent]: I am waiting for PID %d to finish.\n", process);
      waitpid(process, &status, 0);
      printf("[Parent]: Child %d finished with status code %d. Onward!\n", process, WEXITSTATUS(status));

    } else { // Child code
      int exitCode = numForChildren[i] % 50 + 1;
      int waitTime = numForChildren[i] % 3 + 1;
      int processID = getpid();

      printf("\t[Child, PID: %d]: I am the child and I will wait %d seconds and exit with code %d.\n", processID, waitTime, exitCode);
      sleep(waitTime);
      printf("\t[Child, PID: %d]: Now exiting...\n", processID);

      exit(exitCode);
    }
  }
  
  free(numForChildren);
  return 0;
}
