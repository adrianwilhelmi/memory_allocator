#ifndef __ALLOCATOR_H__
#define  __ALLOCATOR_H__

#include<stddef.h>
#include<stdbool.h>
#include<unistd.h>
#include<pthread.h>

//#define allocate(bytes) allocate(bytes, __FILE__, __LINE__);

typedef struct mem_chunk{
	size_t size;
	bool is_free;
	struct mem_chunk*next;
} mem_chunk;

extern mem_chunk*heap_head;
extern pthread_mutex_t allocator_mutex;

void initialize_allocator();
//void*allocate(size_t bytes, __FILE__, __LINE__);
void*allocate(size_t bytes);
void free(void*addr);
void dump_memory();

void free_all();
void align(size_t*size);

#endif
