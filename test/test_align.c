#include<assert.h>

#include"test.h"
#include"allocator.h"

static size_t round_up(size_t size){
	if(size % sizeof(size_t) == 0){
		return size;
	}
	size =  (size + sizeof(size_t)) / sizeof(size_t);
	return size*sizeof(size_t);
}

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
	
	printf("sizeof(struct block) %zu\n", sizeof(struct block));
	printf("sizeof(char) %zu\n", sizeof(char));
	printf("sizeof(int) %zu\n", sizeof(int));
	printf("sizeof(size_t) %zu\n", sizeof(size_t));

	printf("sizeof(struct block) rounded up %zu\n", round_up(sizeof(struct block)));	
	printf("sizeof(char) rounded up %zu\n", round_up(sizeof(char)));
	
	char*a = alloc(sizeof(char));
	dump_memory_info();
	
	struct block*block = alloc(sizeof(struct block));
	dump_memory_info();
	
	int*b = alloc(sizeof(int));
	dump_memory_info();
	
	free(block);
	dump_memory_info();
	
	int*c = alloc(sizeof(int));
	dump_memory_info();
	
	int*d = alloc(sizeof(int));
	dump_memory_info();
}
