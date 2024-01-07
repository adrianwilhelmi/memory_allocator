#ifndef __ALLOCATOR_STATS_H__
#define __ALLOCATOR_STATS_H__

#include<stddef.h>

typedef struct allocator_stats{
	size_t bytes_alloced;
	size_t memory_usage;
	size_t max_memory_usage;
	size_t mean_bytes_alloced;
	size_t unfreed_bytes;
	int alloc_calls;
	int sbrk_calls;
	int broken_chunks;
	int unfreed_blocks;
} allocator_stats;

void report_stats();

#endif