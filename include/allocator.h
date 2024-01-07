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
	struct mem_block*next;
	const char*file;
	size_t size;
	unsigned int magic_number;
	int line;
	bool is_free;
} mem_block;

typedef struct allocator_stats{
	size_t total_bytes_alloced;
	size_t max_peek;
	size_t mean_bytes_alloced;
	size_t unfreed_bytes;
	int alloc_calls;
	int sbrk_calls;
	int broken_chunks;
	int unfreed_blocks;
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
