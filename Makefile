CFLAGS=-Wall -Wextra -pedantic -ggdb -ggdb3 -O3 -pthread
LFLAGS=-Iinclude
VALGRIND_FLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
GCOV_FLAGS=-fprofile-arcs -ftest-coverage
CLANG_TIDY_FLAGS=--quiet -checks=bugprone-*,-bugprone-easily-swappable-parameters,clang-analyzer-*,cert-*,concurrency-*,misc-*,modernize-*,performance-*,readability-* --warnings-as-errors=*
SCANBUILD_FLAGS=--status-bugs --keep-cc --show-description
XANALYZER_FLAGS=--analyze -Xanalyzer -analyzer-output=text

all: compile_tests example

example: example/example.c
	gcc example/example.c -Iexample/include -Lexample/lib -lallocator -o example_out

compile_tests: src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	$(CC) $(CFLAGS) $(LFLAGS) -o tests test/test.c src/allocator.c src/allocator_stats.c src/mem_block.c test/test_unit.c

compile_gcov: src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	$(CC) $(CFLAGS) $(GCOV_FLAGS) $(LFLAGS) -o tests test/test.c test/test_unit.c src/allocator.c src/allocator_stats.c src/mem_block.c

compile_xanalyzer: src/allocator.c src/allocator_stats.c src/mem_block.c
	clang $(XANALYZER_FLAGS) $(LFLAGS) src/allocator.c
	clang $(XANALYZER_FLAGS) $(LFLAGS) src/allocator_stats.c
	clang $(XANALYZER_FLAGS) $(LFLAGS) src/mem_block.c
	@make clean
	
compile_sanitizer: src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	clang -fsanitize=address $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	
	clang -fsanitize=thread $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	
	clang -fsanitize=memory $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	
	clang -fsanitize=undefined $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	
	clang -fsanitize=dataflow $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	
	clang -flto -fsanitize=cfi -fvisibility=default $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	
	clang -fsanitize=safe-stack $(LFLAGS) src/allocator.c src/allocator_stats.c src/mem_block.c test/test.c test/test_unit.c
	@./a.out
	@make clean
	
compile_full_analyze:
	@make compile_xanalyzer
	@make compile_sanitizer
	@scan-build $(SCANBUILD_FLAGS) make compile_gcov

run_scripted_tests:
	@cd test/scripted && ./test_scripted.sh

run_tests:
	@./tests
	@make run_scripted_tests
	
run_tests_valgrind:
	@valgrind $(VALGRIND_FLAGS) ./tests

check_gcov:
	@mv *.gcda src
	@mv *.gcno src
	@cd src && gcov tests-allocator.c
	@cd src && gcov tests-mem_block.c
	@cd src && gcov tests-allocator_stats.c
	@cd src && rm *.gcda *.gcno *.gcov

test: compile_tests run_tests
	
gcov: compile_gcov run_tests check_gcov
	@rm gcov_tests
	
clangtidy:
	@clang-tidy $(CLANG_TIDY_FLAGS) --extra-arg=-Iinclude src/*.c --

regression:
	@make clean
	@make compile_full_analyze
	@make run_tests_valgrind
	@make run_scripted_tests
	@make check_gcov
	@make clean

install_env:
	@chmod +x scripts/install_env.sh
	@scripts/install_env.sh

install:
	@chmod +x scripts/install_lib.sh
	@scripts/install_lib.sh

clean:
	@rm -f allocator tests *.o *.gcda *.gcno a.out example_out
