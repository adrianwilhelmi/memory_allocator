#include<assert.h>

#include"allocator.h"

void test_allign(){
	size_t size = 7;
	allign(&size);
	assert(size == 8);
	
	size = 8;
	allign(&size);
	assert(size == 8);
	
	size = 9;
	allign(&size);
	assert(size == 16);
}
