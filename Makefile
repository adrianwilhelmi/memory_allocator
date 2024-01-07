CFLAGS=-Wall -Wextra -pedantic -ggdb
LFLAGS=-Iinclude

all: allocator

allocator: include/allocator.h src/allocator.c src/main.c
	$(CC) $(CFLAGS) $(LFLAGS) -o allocator src/allocator.c src/main.c

tests: allocator test/test.c test/test_align.c
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c test/test_align.c src/allocator.c

regression:
	make tests

install:
	scripts/install_lib.sh

clean:
	rm -f allocator tests *.o
