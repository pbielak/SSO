CC=gcc
CFLAGS=-std=c99 -Wall
TARGETS=frontend

frontend: frontend.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses
	rm -f *.o

clean:
	rm -f *.o $(TARGETS)

%: %.c
