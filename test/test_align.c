#include<assert.h>

#include"test.h"
#include"allocator.h"

void test_align(){
	
	struct block{
		int a;
		int b;
		char c;
	};
	
	int proper_block_size;
	if(sizeof(struct block)%sizeof(size_t) == 0){
		proper_block_size = sizeof(struct block);
	}
	else{
		proper_block_size = (sizeof(struct block) + sizeof(size_t)) / sizeof(size_t);
		proper_block_size *= sizeof(size_t);
	}
	
	printf("sizeof block: %d\n", (int)sizeof(struct block));
	printf("proper size: %d\n", (int)proper_block_size);
	
	struct block*block1;
	
	assert(block1 == NULL);
	
	block1 = alloc(sizeof(struct block));
	
	assert(block1 != NULL);
	
	dump_memory_info();
}
