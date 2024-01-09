#ifndef __MEM_BLOCK_H__
#define __MEM_BLOCK_H__

#include<stddef.h>
#include<stdbool.h>

extern unsigned int magic_number;

typedef struct mem_block{
	struct mem_block*next;
	const char*file;
	size_t size;
	unsigned int magic_number;
	int line;
	unsigned int block_id;
	bool is_free;
} mem_block;

void check_block(mem_block*mb);

#endif
