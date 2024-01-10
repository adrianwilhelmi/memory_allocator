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
	test_alloc_and_data_usage
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
	void*ptr = alloc(4030056*sizeof(size_t));
	++expected_alloc_calls;
	++expected_sbrk_calls;
	expected_bytes_alloced += round_up(4030056*sizeof(size_t));
	expected_memory_usage += round_up(4030056*sizeof(size_t));
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
}

void*thread_func(){
	printf("amaizng technology\n");
	int max_num_of_allocs = 10;
	int incr = 1;
	int num_of_allocs = 1;
	
	for(num_of_allocs = 0; num_of_allocs < max_num_of_allocs; num_of_allocs += incr){
		void*ptrs[num_of_allocs];
		for(int i = 0; i < num_of_allocs; ++i){
			ptrs[i] = alloc(i*sizeof(size_t));
			printf("mem usage: %zu\n", alloc_stats.memory_usage);
		}
		
//		dump_memory_info();
		
		for(int i = 0; i < num_of_allocs; ++i){
			free(ptrs[i]);
			printf("mem usage: %zu\n", alloc_stats.memory_usage);
		}
		
//		dump_memory_info();
	}
		
	return NULL;
}

#define NUM_THREADS 2
#define NUM_ALLOCATIONS 100
void* thread_test(void* arg) {
    int thread_id = *((int*)arg);

    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        size_t size = (i + 1) * sizeof(int); // Zmienne rozmiary
        void* memory = alloc(size);

        if (memory == NULL) {
            printf("Thread %d: Błąd alokacji pamięci!\n", thread_id);
            continue;
        }

        printf("Thread %d: Alokacja %zu bajtów.\n", thread_id, size);
	   printf("allocd memory %p\n", memory);

        // Tutaj można wykonać operacje na przydzielonej pamięci
//	   sleep(1);
	   dump_full_memory_info();
  //      sleep(1);
        printf("freeing %p\n", memory);
        free(memory);
        printf("Thread %d: Pamięć zwolniona.\n", thread_id);
        
        printf("actual mem usage: %zu\n", alloc_stats.memory_usage);
	   dump_full_memory_info();
	   report_stats();
    }
    pthread_exit(NULL);
}

void thread_test_actual(){
	pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        int rc = pthread_create(&threads[i], NULL, thread_test, (void*)&thread_ids[i]);
        if (rc) {
            printf("Błąd: pthread_create() zwrócił kod %d\n", rc);
            exit(-1);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

}

void test_e2e_threads(){	
	int num_of_threads = 200;
	pthread_t threads[num_of_threads];
	for(int i = 0; i < num_of_threads; ++i){
		if(pthread_create(&threads[i], NULL, thread_func, NULL) != 0){
			perror("thread create err");
			exit(EXIT_FAILURE);
		}
		
	}
	for(int i = 0; i < num_of_threads; ++i){
		if(pthread_join(threads[i], NULL) != 0){
			perror("thread join err");
			exit(EXIT_FAILURE);
		}
		
	}
	/*
	pthread_t thread1;
	pthread_t thread2;

	
	if(pthread_create(&thread1, NULL, thread_func, NULL) != 0){
		perror("thread create err");
		exit(EXIT_FAILURE);
	}
	
	if(pthread_create(&thread2, NULL, thread_func, NULL) != 0){
		perror("thread create err");
		exit(EXIT_FAILURE);
	}
	
	if(pthread_join(thread1, NULL) != 0){
		perror("thread join err");
		exit(EXIT_FAILURE);
	}
	
	if(pthread_join(thread2, NULL) != 0){
		perror("thread join err");
		exit(EXIT_FAILURE);
	}
	*/
}

void test_incremented_allocation(){
	for(int i = 1; i < 20; ++i){
		void*memory = alloc(4*i*sizeof(char));
		printf("allocd memory %p\n", memory);
		printf("of size %zu\n", 4*i*sizeof(char));

        // Tutaj można wykonać operacje na przydzielonej pamięci
	   dump_full_memory_info();
        printf("freeing %p\n", memory);
        free(memory);
        
        printf("actual mem usage: %zu\n", alloc_stats.memory_usage);
	   dump_full_memory_info();
	   report_stats();
	}
	
}

int main(){
	run_unit_tests();
	
//	test_e2e_no_seg();
	
//	test_e2e_threads();
	
//	thread_test_actual();
//	test_incremented_allocation();
	return 0;
}
