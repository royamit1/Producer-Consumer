CC = gcc
CFLAGS = -pthread

all: ex3.out

ex3.out: ex3.c
	$(CC) $(CFLAGS) ex3.c -o ex3.out

clean:
	rm -f ex3.out