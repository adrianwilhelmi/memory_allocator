CFLAGS=-Wall -Wextra -pedantic -ggdb -ggdb3 -O3 -pthread
LFLAGS=-Iinclude
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
GCOV_FLAGS=-fprofile-arcs -ftest-coverage

all: clean compile_tests

compile_tests: src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c 
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c src/allocator.c src/allocator_stats.c src/mem_block.c

compile_gcov: src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c
	$(CC) $(CFLAGS) $(GCOV_FLAGS) $(LFLAGS) -o tests test/test.c src/allocator.c src/allocator_stats.c src/mem_block.c

run_tests:
	./tests

test: compile_tests run_tests

run_tests_valgrind:
	valgrind $(VALGRIND_FLAGS) ./tests

run_gcov:
	./tests
	mv *.gcda src
	mv *.gcno src
	cd src && gcov tests-allocator.c
	cd src && rm *.gcda
	cd src && rm *.gcno
	cd src && rm *.gcov
	
gcov: compile_gcov run_gcov
	rm gcov_tests
	
regression:
	make clean
	make compile_gcov
	make run_gcov
	make run_tests_valgrind
	make clean

install:
	scripts/install_lib.sh

clean:
	rm -f allocator tests *.o *.gcda *.gcno
