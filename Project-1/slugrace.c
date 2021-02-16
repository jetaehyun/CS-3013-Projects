#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>


struct timespec t;
clockid_t clk = CLOCK_MONOTONIC;

void printDebug(int number) {
  printf("Number: %d\n", number);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief gets time in seconds
 *
 * @Param start initial timestamp
 *
 * @Returns time since start
 */
/* ----------------------------------------------------------------------------*/
float getTimestamp(long long int start) {
    clock_gettime(clk, &t);
    float dif = (t.tv_sec - start);

    return dif;
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief prints out active processes based on array
 *
 * @Param children[] list of pids
 */
/* ----------------------------------------------------------------------------*/
void printRacers(int children[]) {
    printf("The race is ongoing. The following children are still racing: ");

    for(int i = 0; i < 4; i++) {
      int child = children[i];

      if(child == -11) continue;

      printf("%d ", child);
    }

    printf("\n");

}

int main() {

  int children[4];
  clock_gettime(clk, &t);
  long long int start = t.tv_sec;

  char *argv[3];
  argv[0] = "./slug";
  argv[2] = NULL;

  for(int i = 0; i < 4; i++) {
    children[i] = fork();

    if(children[i] != 0) {

      printf("[Parent]: I forked off %d.\n", children[i]);

    } else {
      // create string for different slug text files
      char slug[2];
      snprintf(slug, 2, "%d", i + 1);
      argv[1] = slug;

      printf("\t[Child, PID: %d]: Executing './slug %s' command...\n", getpid(), argv[1]);
      execvp("./slug", argv);
      
      exit(0);
    }
  }

  int count = 0; // keep track of completed racers
  while(count < 4) { 
    
    int process = waitpid(-1, NULL, WNOHANG); // non blocking and wait for any process

    if(process > 0) {
      count++;

      printf("Child %d has crossed the finish line! It took %f seconds\n", process, getTimestamp(start));

      for(int i = 0; i < 4; i++) {
        if(children[i] == process) { // remove child from array since its process has ended
          
          children[i] = -11;
          break;
        
        }
      }

      continue;
    } 
  
    usleep(250000); // sleep for .25 seconds

    printRacers(children); 

        
  }

  printf("The race is over! It took %f seconds\n", getTimestamp(start));

  return 0;
}

