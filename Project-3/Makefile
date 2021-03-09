all: goatmalloc

goatmalloc: test_goatmalloc.c goatmalloc.c
		gcc -Wall goatmalloc.c test_goatmalloc.c -o goatmalloc -lm

clean: 
		rm goatmalloc
