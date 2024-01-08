CFLAGS=-Wall -Wextra -pedantic -ggdb -ggdb3 -O3
LFLAGS=-Iinclude
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1

all: clean allocator compile_tests

allocator: src/allocator.c src/allocator_stats.c src/main.c
	$(CC) $(CFLAGS) $(LFLAGS) -o allocator src/allocator.c src/allocator_stats.c src/main.c

compile_tests: test/test.c 
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c src/allocator.c src/allocator_stats.c

run_tests:
	./tests

test: compile_tests run_tests

test_valgrind:
	valgrind $(VALGRIND_FLAGS) ./tests

regression:
	make clean
	make compile_tests
	make test_valgrind
	make clean

install:
	scripts/install_lib.sh

clean:
	rm -f allocator tests *.o *.gcno *.gcda
