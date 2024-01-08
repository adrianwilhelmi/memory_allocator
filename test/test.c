#include<stdio.h>
#include<assert.h>
#include<stdbool.h>

#include"allocator_stats.h"
#include"allocator.h"
#include"test.h"

size_t round_up(size_t size){
	if(size % sizeof(size_t) == 0){
		return size;
	}
	size = (size + sizeof(size_t)) / sizeof(size_t);
	return size * sizeof(size_t);
}

void test_align(){
	b1*blck3 = alloc(sizeof(b3));
	
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


int main(){
	printf("unit tests:\n");
	printf("1 ");
	test_align();
	free_all();
	clean_stats(&alloc_stats);
	
	printf("2 ");
	test_alloc_free();
	free_all();
	clean_stats(&alloc_stats);	
	
	printf("3 ");
	test_alloc_use_empty_block();
	free_all();
	clean_stats(&alloc_stats);
	
	printf("unit tests ok\n");
	
	return 0;
}
