#ifndef __ALLOCATOR_H__
#define  __ALLOCATOR_H__

#include<stddef.h>
#include<stdbool.h>
#include<unistd.h>
#include<pthread.h>
#include<stdint.h>

#define alloc(bytes) allocate(bytes, __FILE__, __LINE__)
#define free(addr) my_free(addr)

typedef struct mem_block{
	size_t size;
	bool is_free;
	struct mem_block*next;
	const char*file;
	int line;
} mem_block;

typedef struct allocator_stats{
	int alloc_calls;
	size_t total_bytes_alloced;
	size_t max_peek;
	int sbrk_calls;
	size_t mean_bytes_alloced;
	int broken_chunks;
	int unfreed_blocks;
	size_t unfreed bytes;
} allocator_stats;

extern mem_block*heap_head;
extern mem_block*heap_tail;
extern pthread_mutex_t allocator_mutex;

void initialize_allocator();
void*allocate(size_t bytes, const char*file, int line);
void my_free(void*addr);
void dump_memory_info();
void free_all();

#endif
