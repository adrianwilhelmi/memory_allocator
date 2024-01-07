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
	int number_of_calls;
	int total_byes_alloced;
	int max_peek;
	int mean_bytes_alloced;
	int number_of_peeks;
	int broken_chunks;
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
