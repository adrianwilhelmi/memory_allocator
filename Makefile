CFLAGS=-Wall -Wextra -pedantic -ggdb
LFLAGS=-Iinclude

all: allocator

allocator:
	$(CC) $(CFLAGS) $(LFLAGS) -o allocator src/allocator.c src/main.c

tests:
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c test/test_align.c src/allocator.c

clean:
    rm -f allocator tests *.o
