CFLAGS=-Wall -Wextra -pedantic -ggdb -ggdb3 -O3 -pthread
LFLAGS=-Iinclude
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
GCOV_FLAGS=-fprofile-arcs -ftest-coverage

all: clean allocator compile_tests

allocator: src/allocator.c src/allocator_stats.c
	$(CC) $(CFLAGS) $(LFLAGS) -o allocator src/allocator.c src/allocator_stats.c

compile_tests: test/test.c 
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c src/allocator.c src/allocator_stats.c

compile_gcov: src/allocator.c src/allocator_stats.c test/test.c
	$(CC) $(CFLAGS) $(GCOV_FLAGS) $(LFLAGS) -o gcov_tests test/test.c src/allocator.c src/allocator_stats.c

run_tests:
	./tests

test: compile_tests run_tests

run_tests_valgrind:
	valgrind $(VALGRIND_FLAGS) ./tests

run_gcov:
	./gcov_tests
	mv *.gcda src
	mv *.gcno src
	cd src && gcov gcov_tests-allocator.c
	cd src && rm *.gcda
	cd src && rm *.gcno
	rm gcov_tests
	
gcov: compile_gcov run_gcov

regression:
	make clean
	make compile_gcov
	make run_tests_valgrind
	make clean

install:
	scripts/install_lib.sh

clean:
	rm -f allocator tests gcov_tests *.o
