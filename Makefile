CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

all: pwc

pwc: main.o helper.o
	$(CC) $(CFLAGS) main.o helper.o -o pwc

main.o: main.c node.h
	$(CC) $(CFLAGS) -o main.o -c main.c

helper.o: helper.c node.h
	$(CC) $(CFLAGS) -o helper.o -c helper.c

clean:
	rm -f *.o pwc