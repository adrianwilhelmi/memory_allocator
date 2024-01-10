#ifndef __ALLOCATOR_H__
#define  __ALLOCATOR_H__

#include<stddef.h>
#include<stdbool.h>
#include<unistd.h>
#include<pthread.h>
#include<stdint.h>

#include"mem_block.h"
#include"allocator_stats.h"

#define alloc(bytes) allocate(bytes, __FILE__, __LINE__)
#define free(addr) my_free(addr)

extern mem_block*heap_head;
extern mem_block*heap_tail;
extern allocator_stats alloc_stats;
extern pthread_mutex_t allocator_mutex;

void initialize_allocator();
void*allocate(size_t bytes, const char*file, int line);
void my_free(void*addr);
void free_all();
void dump_memory_info();
void dump_full_memory_info();

#endif
