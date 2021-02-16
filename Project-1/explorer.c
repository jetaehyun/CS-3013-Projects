#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

const char *paths[] = {"/home", "/proc", "/proc/sys", "/usr", "/usr/bin", "/bin"};

void printDebug(int number) {
  printf("Number: %d\n", number);
}

/* --------------------------------------------------------------------------*/
/**
 * @Brief helper function to move to a directory
 *
 * @Param selection identify which selection number this is
 * @Param path which path to go 
 */
/* --------------------------------------------------------------------------*/
void goToDir(int selection, const char *path) {
  char cwd[1000];
  char msg[8];

  if(chdir(path) == 0) {
    strncpy(msg,"SUCCESS", 7);
  } else {
    strncpy(msg, "FAILED", 7);
  }

  printf("Selection #%d %s [%s]\n", selection, path, msg);
  printf("Current reported directory: %s\n", getcwd(cwd, sizeof(cwd)));

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

  printf("It's time to see the world/file system!\n");
  
  for(int i = 0; i < 5; i++) {
    
    int path = rand() % 6;
    goToDir(i + 1, paths[path]);
    
    int process = fork();
    int status = 0;

    if(process != 0) {
      printf("[Parent]: I am waiting for PID %d to finish.\n", process);

      waitpid(process, &status, 0);

      printf("[Parent]: Child %d finished with status code %d. Onward!\n", process, status);

    } else {
      printf("\t[Child, PID: %d]: Executing 'ls -alh' command...\n", getpid());
      
      // create argv for program execution
      char *arg[3];
      arg[0] = "ls";
      arg[1] = "-alh";
      arg[2] = NULL;
      execvp("ls", arg);

      exit(0);
    }

  }

  return 0;
}
