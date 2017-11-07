CFLAGS=-std=c99 -Wall
TARGETS=threaded_bank_account_modifier

all: $(TARGETS)

clean:
	rm -f *.o $(TARGETS)

zip: lab5.zip

lab5.zip: *.c Makefile README.md

%: %.c
