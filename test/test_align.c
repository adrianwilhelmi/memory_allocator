#include<assert.h>

#include"test.h"
#include"allocator.h"

void test_align(){
	size_t size;
	
	size = 7;
	size = align(size);
	if(sizeof(size_t) == 8){
		assert(size == 8);
	}
	else if(sizeof(size_t) == 4){
		assert(size == 8);	
	}
	
	size = 8;
	size = align(size);
	if(sizeof(size_t) == 8){
		assert(size == 8);
	}
	else if(sizeof(size_t) == 4){
		assert(size == 8);	
	}

	size = 9;
	size = align(size);
	if(sizeof(size_t) == 8){
		assert(size == 16);
	}
	else if(sizeof(size_t) == 4){
		assert(size == 8);	
	}
	
	size = 4;
	size = align(size);
	if(sizeof(size_t) == 8){
		assert(size == 8);
	}
	else if(sizeof(size_t) == 4){
		assert(size == 4);
	}
}
