#include<assert.h>

#include"allocator.h"

void test_align(){
	size_t size;
	size = 7;
	align(&size);
	assert(size == 8);
	
	size = 8;
	align(&size);
	assert(size == 8);
	
	size = 9;
	align(&size);
	assert(size == 16);
}
