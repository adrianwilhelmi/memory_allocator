#include<stdio.h>
#include<assert.h>
#include<stdbool.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>

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
	printf("UNIT TESTS Ok.\n\n");
}

void test_e2e_no_seg(){
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
	
	printf("\nEXPECTED ALLOCATOR STATS\n");
	
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
}



int main(){
	initialize_allocator();
	
	run_unit_tests();
	
	test_e2e_no_seg();
	
	return 0;
}
