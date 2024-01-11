#include<stdio.h>
#include<assert.h>
#include<stdbool.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<signal.h>

#include"allocator_stats.h"
#include"allocator.h"
#include"test.h"

void(*tests[])() = {
	test_align, 
	test_alloc_free,
	test_alloc_use_empty_block,
	test_alloc_zero,
	test_multithread,
	test_huge_alloc,
	test_alloc_and_data_usage,
//	test_lots_of_threads		//SHOULDNT BE RUN WITH VALGRIND
	test_increment_alloc
};

size_t round_up(size_t size){
	if(size % sizeof(size_t) == 0){
		return size;
	}
	size = (size + sizeof(size_t)) / sizeof(size_t);
	return size * sizeof(size_t);
}

void run_unit_tests(){
	printf("UNIT TESTS:\n");
	
	int number_of_tests = sizeof(tests) / sizeof(tests[0]);
	for(int i = 0; i < number_of_tests; ++i){
		printf("%d ", i+1);
		tests[i]();
		free_all();
		clean_stats(&alloc_stats);
	}
	printf("UNIT TESTS Ok.\n");
}

void test_e2e_no_seg(){
	int fd_out;
	int fd_err;
	int fd_terminal_out = dup(STDOUT_FILENO);
	int fd_terminal_err = dup(STDERR_FILENO);
	
	if((fd_out = open("test/scripted/results/e2e_result1.txt", O_TRUNC | O_WRONLY | O_CREAT, 0644)) == -1){
		perror("err opening file1");
		exit(EXIT_FAILURE);
	}
	if((fd_err = open("test/scripted/results/e2e_err1.txt", O_TRUNC | O_WRONLY | O_CREAT, 0644)) == -1){
		perror("err opening file2");
		exit(EXIT_FAILURE);
	}
	
	if(dup2(fd_out, STDOUT_FILENO) == -1){
		perror("dup2 out err");
		exit(EXIT_FAILURE);
	}
	
	if(dup2(fd_err, STDERR_FILENO) == -1){
		perror("dup2 err");
		exit(EXIT_FAILURE);
	}
	
	
	size_t expected_bytes_alloced = 0;
	size_t expected_memory_usage = 0;
	size_t expected_max_memory_usage = 0;
	int expected_alloc_calls = 0;
	int expected_sbrk_calls = 0;
	int expected_broken_blocks = 0;
	
	int*a = alloc(sizeof(int));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(sizeof(int));
	expected_memory_usage += round_up(sizeof(int));
	if(expected_max_memory_usage < expected_memory_usage){
		expected_max_memory_usage = expected_memory_usage;
	}
	
	void*b = alloc(20*sizeof(size_t));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(20*sizeof(size_t));
	expected_memory_usage += round_up(20*sizeof(size_t));
	if(expected_max_memory_usage < expected_memory_usage){
		expected_max_memory_usage = expected_memory_usage;
	}
	
	char*c = alloc(sizeof(char));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(sizeof(char));
	expected_memory_usage += round_up(sizeof(char));
	if(expected_max_memory_usage < expected_memory_usage){
		expected_max_memory_usage = expected_memory_usage;
	}
	
	expected_memory_usage -= round_up(20*sizeof(size_t));
	free(b);
	
	void*d = alloc(10*sizeof(size_t));
	++expected_alloc_calls;
	expected_bytes_alloced += round_up(10*sizeof(size_t));
	expected_memory_usage += round_up(10*sizeof(size_t));
	if(expected_max_memory_usage < expected_memory_usage){
		expected_max_memory_usage = expected_memory_usage;
	}
	
	void*e = alloc(3000*sizeof(size_t));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(3000*sizeof(size_t));
	expected_memory_usage += round_up(3000*sizeof(size_t));
	if(expected_max_memory_usage < expected_memory_usage){
		expected_max_memory_usage = expected_memory_usage;
	}
	
	free(a);
	free(c);
	free(d);
	free(e);
	
	expected_memory_usage -= round_up(sizeof(int));
	expected_memory_usage -= round_up(sizeof(char));
	expected_memory_usage -= round_up(10*sizeof(size_t));
	expected_memory_usage -= round_up(3000*sizeof(size_t));
	
	for(int i = 0; i < 1000; ++i){
		//alloc and free a lot of small chunks
		void*ptr = alloc(i*sizeof(size_t));
		++expected_alloc_calls;
		expected_bytes_alloced += round_up(i*sizeof(size_t));
		expected_memory_usage += round_up(i*sizeof(size_t));
		if(expected_max_memory_usage < expected_memory_usage){
			expected_max_memory_usage = expected_memory_usage;
		}
				
		free(ptr);
		expected_memory_usage -= round_up(i*sizeof(size_t));
	}
	
	void*ptrs[50];
	for(int i = 0; i < 50; ++i){
		//alloc a couple of chunks at a time
		//shouldnt pass the breeak
		
		ptrs[i] = alloc(i*sizeof(size_t));
		++expected_alloc_calls;
		expected_bytes_alloced += round_up(i*sizeof(size_t));
		expected_memory_usage += round_up(i*sizeof(size_t));
		if(expected_max_memory_usage < expected_memory_usage){
			expected_max_memory_usage = expected_memory_usage;
		}	
	}
	
	for(int i = 0; i < 50; ++i){
		//and free them
		free(ptrs[i]);
		expected_memory_usage -= round_up(i*sizeof(size_t));
	}
	
	//alloc huge amount at a time, should raise break -> sbrk++
	void*ptr = alloc(40300*sizeof(size_t));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(40300*sizeof(size_t));
	expected_memory_usage += round_up(40300*sizeof(size_t));
	if(expected_max_memory_usage < expected_memory_usage){
		expected_max_memory_usage = expected_memory_usage;
	}
	
	free(ptr);
	
	printf("EXPECTED ALLOCATOR STATS\n");
	
	printf("%-12s %-12s %-22s %-22s %-20s %-12s \n", "alloc_calls", "sbrk_calls", "total_bytes_alloced",
		"mean_bytes_alloced", "max_memory_usage", "broken_blocks"
		);
	
	printf("%-12d %-12d %-22zu %-22zu %-20zu %-12d \n",
		expected_alloc_calls,
		expected_sbrk_calls,
		expected_bytes_alloced,
		expected_bytes_alloced / expected_alloc_calls,
		expected_max_memory_usage,
		expected_broken_blocks
		);
		
	report_stats();
	free_all();
	clean_stats(&alloc_stats);
	
	close(fd_out);
	close(fd_err);
	
	if(dup2(fd_terminal_out, STDOUT_FILENO) == -1){
		perror("dup2 restore err");
		exit(EXIT_FAILURE);
	}
	if(dup2(fd_terminal_err, STDERR_FILENO) == -1){
		perror("dup2 restore err");
		exit(EXIT_FAILURE);
	}
	
	close(fd_terminal_out);
	close(fd_terminal_err);
}

void test_e2e_seg(){
	int fd_out;
	int fd_err;
	int fd_terminal_out = dup(STDOUT_FILENO);
	int fd_terminal_err = dup(STDERR_FILENO);
	
	if((fd_out = open("test/scripted/results/e2e_result2.txt", O_TRUNC | O_WRONLY | O_CREAT, 0644)) == -1){
		perror("err opening file");
		exit(EXIT_FAILURE);
	}
	if((fd_err = open("test/scripted/results/e2e_err2.txt", O_TRUNC | O_WRONLY | O_CREAT, 0644)) == -1){
		perror("err opening file");
		exit(EXIT_FAILURE);
	}
	
	if(dup2(fd_out, STDOUT_FILENO) == -1){
		perror("dup2 out err");
		exit(EXIT_FAILURE);
	}
	
	if(dup2(fd_err, STDERR_FILENO) == -1){
		perror("dup2 err");
		exit(EXIT_FAILURE);
	}
	
	signal(SIGSEGV, SIG_IGN);
	
	void*a = alloc(sizeof(size_t));
	
	free(a);
	//double free shuld raise seg fault
	printf("expected ");
	invalid_block_message("double free", "Raising seg fault...");
	printf("actual ");
	free(a);
	
	
	//freeing random block should raise seg fault
	printf("expected ");
	invalid_block_message("no such block", "Raising seg fault...");
	printf("actual ");
	free((int*)(0x5555A));
	
	
	//overwriting memory where block metadata is should raise seg fault (overwriting magic number)
	printf("expected ");
	invalid_block_message("magic number failure", "Raising seg fault...");
	printf("actual ");
	void*ptr = alloc(sizeof(size_t));
	char*illegal = ((char*)ptr) - sizeof(mem_block); //illegal should point at the beggining of the metadata
	memset(illegal, 0, sizeof(mem_block));
	//now freeing ptr should raise segmentation fault since its magic number is overwritten
	free(ptr);
	
	close(fd_out);
	close(fd_err);
	
	if(dup2(fd_terminal_out, STDOUT_FILENO) == -1){
		perror("dup2 restore err");
		exit(EXIT_FAILURE);
	}
	if(dup2(fd_terminal_err, STDERR_FILENO) == -1){
		perror("dup2 restore err");
		exit(EXIT_FAILURE);
	}
	
	close(fd_terminal_out);
	close(fd_terminal_err);
	
	clean_stats(&alloc_stats);
	free_all();
}

int main(){
	initialize_allocator();
	
	run_unit_tests();
	
	test_e2e_no_seg();
	test_e2e_seg();
	
	return 0;
}
