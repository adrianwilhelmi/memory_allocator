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
	
	//if 32bit arch
	if(sizeof(b3) == 12){
		assert(alloc_stats.bytes_alloced == 12);
		assert(alloc_stats.max_memory_usage == 12);
	}
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
	
	//char*to_be_freed = alloc(sizeof(mem_block) + sizeof(char) + sizeof(size_t));
	
	b4*to_be_freed = alloc(sizeof(b4));
	
	int*b = alloc(sizeof(int));
	
	//check if everything is ok
	
	dump_memory_info();
	report_stats();
	
	printf("round up sizeof int: %ld\n", round_up(sizeof(int)));
	printf("round up sizeof b4: %ld\n", round_up(sizeof(b4)));
	
	size_t total = round_up(sizeof(int));
	total += round_up(sizeof(b4));
	total += round_up(sizeof(int));
	
	printf("total: %zu\n", total);
	
	assert(alloc_stats.bytes_alloced == total);
	assert(alloc_stats.alloc_calls == 3);
	assert(alloc_stats.sbrk_calls == 3);
	
	free(to_be_freed);
	
	char*should_be_in_the_middle = alloc(sizeof(char));
	
	total += round_up(sizeof(char));
	
	assert(alloc_stats.bytes_alloced == total);
	assert(alloc_stats.alloc_calls == 4);
	assert(alloc_stats.sbrk_calls == 3);
	
	free(should_be_in_the_middle);
	free(a);
	free(b);
	
	printf("empty block test ok\n");
}


int main(){
	
	test_align();
	dump_memory_info();
	free_all();
	clean_stats(&alloc_stats);
	
	test_alloc_free();
	dump_memory_info();
	free_all();
	clean_stats(&alloc_stats);	
	
	test_alloc_use_empty_block();
	free_all();
	clean_stats(&alloc_stats);
	
	printf("tests ok\n");
	
	return 0;
}
