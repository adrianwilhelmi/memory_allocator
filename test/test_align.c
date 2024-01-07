#include<assert.h>

#include"test.h"
#include"allocator.h"

void test_align(){
	
	struct block{
		int a;
		double b;
		double c;
		double d;
		double e;
		double f;
		double g;
	};
	
	char*a = my_alloc(sizeof(char));
	dump_memory_info();
	
	struct block*block = my_alloc(sizeof(struct block));
	dump_memory_info();
	
	int*b = my_alloc(sizeof(int));
	dump_memory_info();
	
	my_free(block);
	dump_memory_info();
	
	int*c = my_alloc(sizeof(int));
	dump_memory_info();
	
	int*d = my_alloc(sizeof(int));
	dump_memory_info();
}
