
CFLAGS := -Wall -Wextra -ansi -pedantic -O0 -ggdb3

.PHONY: all test clean

all: test

clean:
	$(RM) $(bin)

test: mask.c
	$(CC) $(CFLAGS) -omask mask.c
	./mask
	valgrind ./mask

