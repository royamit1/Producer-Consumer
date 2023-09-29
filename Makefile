CC = gcc
CFLAGS = -pthread

all: main.out

main.out: main.c
	$(CC) $(CFLAGS) main.c -o main.out

clean:
	rm -f main.out
