CFLAGS=-std=c99 -Wall
TARGETS=chat-client chat-server

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

chat-client: chat-client.c
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

zip: lab7.zip

lab7.zip: *.c *.h Makefile README.md

%: %.c
