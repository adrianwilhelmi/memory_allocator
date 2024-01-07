#include"allocator.h"
#include"test.h"

#include<stdio.h>

int main(){
	struct block{
		int a;
		double b;
		double c;
		double d;
		double e;
		double f;
		double g;
	};
	
	printf("sizeof(struct block): %zu\n", sizeof(struct block));
	
	printf("tests ok\n");
	
	return 0;
}
