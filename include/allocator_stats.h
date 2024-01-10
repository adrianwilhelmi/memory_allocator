#ifndef __ALLOCATOR_STATS_H__
#define __ALLOCATOR_STATS_H__

#include<stddef.h>

typedef struct byte{
	size_t amount;
	char prefix;
} byte;

typedef struct allocator_stats{
	size_t bytes_alloced;
	size_t memory_usage;
	size_t max_memory_usage;
	int alloc_calls;
	int sbrk_calls;
} allocator_stats;

void report_stats();
void clean_stats();

#endif
