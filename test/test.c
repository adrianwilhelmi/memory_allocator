#include<stdio.h>
#include<assert.h>
#include<stdbool.h>
#include<pthread.h>
#include<string.h>

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
	test_alloc_and_data_usage
};

size_t round_up(size_t size){
	if(size % sizeof(size_t) == 0){
		return size;
	}
	size = (size + sizeof(size_t)) / sizeof(size_t);
	return size * sizeof(size_t);
}

void test_align(){
	b3*blck3 = alloc(sizeof(b3));
	
	//if 32bit architecture
	if(sizeof(b3) == 12){
		assert(alloc_stats.bytes_alloced == 12);
		assert(alloc_stats.max_memory_usage == 12);
	}
	//if 64bit architecture
	else if(sizeof(b3) == 16){
		assert(alloc_stats.bytes_alloced == 16);
		assert(alloc_stats.max_memory_usage == 16);
	}
	
	assert(alloc_stats.alloc_calls == 1);
	assert(alloc_stats.sbrk_calls == 1);
	
	free(blck3);
	
	printf("allign test ok\n");
}

void test_alloc_free(){
	b1*blck = alloc(sizeof(b1));
	
	assert(blck != NULL);
	
	free(blck);
	
	assert(alloc_stats.alloc_calls == 1);
	assert(alloc_stats.bytes_alloced == sizeof(b1));
	printf("simple alloc ok\n");
}

void test_alloc_use_empty_block(){
	int*a = alloc(sizeof(int));
	b4*to_be_freed = alloc(sizeof(b4));
	int*b = alloc(sizeof(int));
	
	//check if everything is ok
		
	size_t total = round_up(sizeof(int));
	total += round_up(sizeof(b4));
	total += round_up(sizeof(int));
	
	size_t max_mem = total;
	
	assert(alloc_stats.bytes_alloced == total);
	assert(alloc_stats.alloc_calls == 3);
	assert(alloc_stats.sbrk_calls == 3);
	
	
	
	free(to_be_freed);
	
	//now there should be enough empty space, so we dont raise program break with sbrk
	char*should_be_in_the_middle = alloc(sizeof(char));
	
	total += round_up(sizeof(char));
	
	assert(alloc_stats.bytes_alloced == total);
	assert(alloc_stats.alloc_calls == 4);
	assert(alloc_stats.sbrk_calls == 3);
	assert(alloc_stats.max_memory_usage == max_mem);
	free(should_be_in_the_middle);
	free(a);
	free(b);
	
	printf("empty block test ok\n");
}

void test_alloc_zero(){
	void*ptr = alloc(0);
	assert(ptr == NULL);

	assert(alloc_stats.bytes_alloced == 0);
	assert(alloc_stats.sbrk_calls == 0);
	assert(alloc_stats.alloc_calls == 1);
	
	printf("alloc zero ok\n");
}

static void*couple_of_allocations(){
	int number_of_allocations = 40;
	void*ptrs[number_of_allocations];
	for(int i = 0; i < number_of_allocations; ++i){
		size_t size = i + 1;
		ptrs[i] = alloc(size);
		assert(ptrs[i] != NULL);
	}

	for(int i = 0; i < number_of_allocations; ++i){
		free(ptrs[i]);
	}
	
	return NULL;
}

void test_multithread(){	
	int num_of_threads = 10;
	pthread_t threads[num_of_threads];
	
	for(int i = 0; i < num_of_threads; ++i){
		int result = pthread_create(&threads[i], NULL, couple_of_allocations, NULL);
		assert(result == 0);
	}
	for(int i = 0; i < num_of_threads; ++i){
		int result = pthread_join(threads[i], NULL);
		assert(result == 0);
	}
	
	printf("multithreading ok\n");
}

void test_huge_alloc(){
	size_t big_size = sizeof(size_t) * 400000;
	void*ptr = alloc(big_size);
	assert(ptr != NULL);
	
	assert(alloc_stats.alloc_calls == 1);
	assert(alloc_stats.bytes_alloced == big_size);
	
	printf("huge alloc ok\n");
}

void test_alloc_and_data_usage(){
	char*words = alloc(sizeof(char)*7);
	
	strcpy(words, "abcdef");
	
	assert(words != NULL);
	
	free(words);
	
	int*a = alloc(sizeof(int));
	
	assert(a != NULL);
	
	assert(alloc_stats.alloc_calls == 2);
	assert(alloc_stats.sbrk_calls == 1);
	
	printf("data usage ok\n");
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
	
	void*e = alloc(4000*sizeof(size_t));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(4000*sizeof(size_t));
	expected_memory_usage += round_up(4000*sizeof(size_t));
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
	expected_memory_usage -= round_up(4000*sizeof(size_t));
	
	for(int i = 0; i < 100; ++i){
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
}

int main(){
	//run_unit_tests();
	
	printf("e2e tests\n");
	test_e2e_no_seg();
	
	return 0;
}
