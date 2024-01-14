#include"allocator.h"

#include<stdio.h>

int main(){
	//to allocate memory use
	void*ptr = alloc(10*sizeof(size_t));
	char*alpha = alloc(7*sizeof(char));
	
	//dump memory info with
	dump_memory_info();
	
	//usage of allocated memory
	for(int i = 0; i < 7; ++i){
		*(alpha + i) = 'A' + i;
	}
	
	for(int i = 0; i < 7; ++i){
		printf("%c\n", *(alpha+i));
	}
	
	//freeing memory
	free(alpha);
	free(ptr);
	
	//no memory allocated will be printed since all the blocks been freed
	dump_memory_info();
	
	//now statistics about memory usage will be printed after exiting the program
	
	//info unfreed blocks should be also printed on the terminal
	void*unfreed1 = alloc(10*sizeof(int));
	void*unfreed2 = alloc(sizeof(double));
	
	//dont worry, these blocks will be freed just after printing info about them
	
	return 0;
}
