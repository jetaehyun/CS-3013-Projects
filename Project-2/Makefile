all: scheduler

scheduler: scheduler.c linked_list.c
		gcc -o scheduler scheduler.c linked_list.c -Wall -Werror -lm

clean:
		rm -f scheduler
