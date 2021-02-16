#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

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

  // Get number of children in 7 - 11 range
  int count = rand() % 4 + 7;
  printf("Random Descendant Count: %d\n", count);
  printf("Time to meet the kids/grandkids/great grandkids/...!\n");

  // infinite loop, will terminate on exit of child processes
  while(1) {

    int process = fork();

    if(process != 0) { // parent process
      int status = 0;

      printf("[Parent, PID: %d]: I am waiting for PID %d to finish.\n", getpid(), process);
      waitpid(process, &status, 0);
      
      int exitStatus = WEXITSTATUS(status);

      printf("[Parent, PID: %d]: Child %d finished with status code %d. It's now my turn to exit.\n", getpid(), process, exitStatus);

      exit(exitStatus + 1);

    } else { // child process

      int prev = count;
      count -= 1;

      printf("\t[Child, PID: %d]: I was called with descendant count=%d. I'll have %d descendant(s).\n", getpid(), prev, count);
      
      
      if(count == 0) {
        exit(0);
      }

    }

  }
 
  return 0;
}
