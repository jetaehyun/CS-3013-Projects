Tae Hyun Je
CS 3013 Operating Systems
Project 1 - Process Party

--------------The Prolific Party----------------

I dynamically allocated space to store the pseudo random numbers for the children. I then used a for loop to fork() to create a
random number of children from the rand() function. I used the process ID to identify the parent and child, which helped to 
execute the proper code for each.

-------------Lifespan Generation----------------

I used rand() to determine the number of children. From there, I created an infinite while loop that used an if statement to
seperate the parent and child codes. For each parent, they have the the process ID of the spawned child. That information is then used
with waitpid() to obtain the exit code of the child and block the parent process until the child process terminated. A simple count
check was used to keep track of the number of children created.

----------------The Explorer--------------------

The parent waits for the child to terminate after it starts a new process via execvp(). I created a helper function to report if
a process was able to change directories using chdir(). I also created a global variable that contained an array of paths as strings.
This made it easier to identify which directory the child needed to change directories too by simply accessing the array.

-------------------The Slug---------------------

I used the fprintf() function to make sure that the program could run any kind of seed_slug_x.txt, if there are more than 4 files.
The sleep() function was used to make the process wait.
A simply if statement was used to identify which command to execute based on the coin flip.

----------------The Slug Race------------------

I used a for loop to spawn the the four different slug processes. To keep track of the different children processes, I used an 
int array, children, to store the active PIDs. A simple while loop was used to keep track of active racers. A variable called "count" was
used as the condition for the while loop termination. I used the WNOHANG flag for waitpid() to make sure that the call was NONBLOCKING.
This allowed to me to check every 0.25 seconds using usleep(). If a child process was terminated, waitpid() would return its process id 
because -1 was passed to the first parameter of waitpid(); -1 basically told waitpid() to just check for any processes that have changed 
states. If waitpid() returned a proper pid, it would overwrite the pid in the children array with -11. This made it easier 
to use the helper function called printRacers(). This function loops through a given array and prints the pid of anything that isn't -11.

A helper function called getTimestamp() was created to return the difference between the provided start time and current time.

---------------------ETC.-----------------------

I tested my program using the print statements and examples provided in the pdf.
