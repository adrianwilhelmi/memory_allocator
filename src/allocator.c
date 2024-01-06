#include<stdlib.h>
#include<stdio.h>

#include"allocator.h"

pthread_mutex_t allocator_mutex;
mem_chunk*heap_head = NULL;

void initialize_allocator(){
	pthread_mutex_init(&allocator_mutex, NULL);
	
	if((heap_head = sbrk(0)) == (void*)(-1)){
		perror("err initializing allocator");
		exit(EXIT_FAILURE);
	}
}

#ifdef __GNUC__
__attribute__((constructor))
void init(){
	initialize_allocator();
}
#endif

void align(size_t*size){
	size_t remainder = (*size) % sizeof(size_t);
	if(remainder == 0){
		return;
	}
	*size = *size + sizeof(size_t) - remainder;
}
