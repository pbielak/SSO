CFLAGS=-std=c99 -Wall
TARGETS=zad1

all: $(TARGETS)
clean:
	rm -f *.o $(TARGETS)

zip: lab2.zip

lab2.zip: *.c Makefile README.txt
	zip $@ $^

%: %.c
