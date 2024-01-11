#ifndef __MEM_BLOCK_H__
#define __MEM_BLOCK_H__

#include<stddef.h>
#include<stdbool.h>

extern unsigned int magic_number;

typedef struct mem_block{
	struct mem_block*next;
	const char*file;
	size_t size;
	size_t true_size;
	unsigned int magic_number;
	int line;
	unsigned int block_id;
	bool is_free;
} mem_block;

int is_block_valid(mem_block*mb);
mem_block*merge_blocks(mem_block*mb1, mem_block*mb2);

#endif
