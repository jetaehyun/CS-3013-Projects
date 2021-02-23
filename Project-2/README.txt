Tae Hyun Je
CS3013
Project 2 - Scheduling Policies

The scheduler program simulates the scheduling policies of an OS (e.g. FIFO, SJF, RR). It prints the execution stack of a workload and the analysis of the metrics of each policy. The novel workloads are:

workload_1.in
	- Response time: 0 -> 4
	- wait time: 0 -> 4

workload_2.in - THIS ONE WILL TAKE A LONG TIME
	- FIFO: avg turnaround: 190.50
	- SJF: avg turnaround: 19.50

workload_3.in
	- FIFO: avg response: 4, turnaround: 6, wait: 4
	- SJF: avg response: 4, turnaround: 6, wait: 4
	- RR: avg response: 4, turnaround: 6, wait: 4

workload_4.in
	- RR: avg turnaround: 105.20 wait: 4.40

workload_5.in
	- FIFO: avg response: 5 turnaround: 13

--------------ETC---------------
* The execution trace functions just print out their response time, wait time, and turnaround time,
	and uses floats to keep track of the simulated time.
* I created a linked list implementation that uses FIFO (linked_list.c, linked_list.h).
* I made a function for each job execution trace and analysis, they exist in scheduler.c 
	The only analysis function that is different is RR_Analysis(). It essentially
	calculates each jobs response, turnaround, and wait times first before printing it
	out. This is because it was a modified version of my RR() function so it will not
	pass the test cases unless I stored them and then printed them in job ID order.
* I added comments and Oxygen to my work to explain how my code works