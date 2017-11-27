CFLAGS=-std=c99 -Wall
TARGETS=client server

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

zip: lab6.zip

lab6.zip: *.c *.h Makefile README.md

%: %.c
