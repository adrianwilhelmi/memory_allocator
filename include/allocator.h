#ifndef __ALLOCATOR_H__
#define  __ALLOCATOR_H__

#include<stddef.h>
#include<stdbool.h>
#include<unistd.h>
#include<pthread.h>

typedef struct memory_chunk{
	size_t size;
	bool is_free;
	struct memory_chunk*next;
} memory_chunk;

extern memory_chunk*heap_head;
extern pthread_mutex_t allocator_mutex;

void initialize_allocator();
void*allocate(size_t bytes);
void free(void*addr);
void free_all();
void dump_memory();

#endif
