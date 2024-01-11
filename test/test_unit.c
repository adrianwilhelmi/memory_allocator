#include<stdio.h>
#include<string.h>
#include<assert.h>

#include"test.h"
#include"allocator.h"

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
	int number_of_allocations = 20;
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

void*thread_func(){
	int max_num_of_allocs = 10;
	int incr = 1;
	int num_of_allocs = 1;
	
	for(num_of_allocs = 0; num_of_allocs < max_num_of_allocs; num_of_allocs += incr){
		void*ptrs[num_of_allocs];
		for(int i = 0; i < num_of_allocs; ++i){
			ptrs[i] = alloc(i*sizeof(size_t));
		}
		
		for(int i = 0; i < num_of_allocs; ++i){
			free(ptrs[i]);
		}
		
	}
		
	return NULL;
}

void test_lots_of_threads(){	
	int num_of_threads = 200;
	pthread_t threads[num_of_threads];
	for(int i = 0; i < num_of_threads; ++i){
		int result = pthread_create(&threads[i], NULL, thread_func, NULL);
		assert(result == 0);
	}
	for(int i = 0; i < num_of_threads; ++i){
		int result = pthread_join(threads[i], NULL);
		assert(result == 0);
	}
	
	printf("lots of threads ok\n");
}

void test_increment_alloc(){
	for(int i = 1; i < 20; ++i){
		char*memory = alloc(4*i*sizeof(char));
		/*
		printf("alloced memory at %p\n", memory);
		printf("of size %zu\n", i*4*sizeof(char));		
		dump_full_memory_info();
		printf("freeing %p\n", memory);
		*/
		free(memory);
		
		//dump_full_memory_info();
	}	
	
	printf("increment ok\n");
}
