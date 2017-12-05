CFLAGS=-std=c99 -Wall
TARGETS=chat-client chat-server

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

chat-client: chat-client.c
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

zip: lab8.zip

lab8.zip: *.c *.h Makefile README.md

%: %.c
