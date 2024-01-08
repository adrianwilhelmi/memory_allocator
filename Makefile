CFLAGS=-Wall -Wextra -pedantic -ggdb -ggdb3 -O3
LFLAGS=-Iinclude

all: clean allocator

allocator: src/allocator.c src/allocator_stats.c src/main.c
	$(CC) $(CFLAGS) $(LFLAGS) -o allocator src/allocator.c src/allocator_stats.c src/main.c

compile_tests: allocator test/test.c 
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c src/allocator.c src/allocator_stats.c

run_tests:
	./tests

tests: compile_tests run_tests

regression:
	make tests

install:
	scripts/install_lib.sh

clean:
	rm -f allocator tests *.o
