Tae Hyun Je
CS3013 Operating Systems
Project 4 - Synchronization and Concurrency

Problem 1:

I used mutex locks and condition variables to solve this problem.

Each performer (dancer, juggler, and soloist) and stage have their own thread to run on. They signal() one another to wake each other up from suspension. The stage thread
wakes a "random" performer and then call wait(). The performer thread initially calls wait() performs their performance, if they can. Once they finish, they signal() the 
stage thread that they finished and the stage is now open. Performances are bounded between 1-5 seconds.

The output1.txt contains the print out message with seed 12345. Reading the printments MIGHT sometimes not make sense because of how the scheduler determines which thread
will print out first, depending on the situation.

Problem 2:

I used semaphores to solve this problem.

Each worker runs a copy of workerThread(). From there, it obtains a sem resource depending on their team (e.g. red, blue, green, yellow). When they are popping a package
from the queue and attempting to process them, a reservation style approach prevents deadlocks. Each station reservation is about ~1 second; I gave the station processing
time an arbitrary value. Once the worker is done, they sleep() and the next worker in their team can take over. Each worker is denoted by their team color and id.

The output2.txt contains the solution from seed 12345. Like problem 1, it is handled in the same way.

Testing:

I tested the program using various seeds to test the randomness and potential issues that might rise from those random changes. The original seed I tested was 12345. I also
tested seeds like 8, 50, 7892. The outputx.txt files are all based on 12345, although.
