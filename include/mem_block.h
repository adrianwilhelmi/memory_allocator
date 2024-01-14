//Author: Adrian Wilhelmi

#ifndef __MEM_BLOCK_H__
#define __MEM_BLOCK_H__

#include<stddef.h>
#include<stdbool.h>

#define MAGIC_NUMBER 0x6164726E

extern unsigned int magic_number;
extern unsigned int global_block_id;

typedef enum{
	NO_ERR,
	INVALID_BLOCK
} error_code;

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

extern mem_block*heap_head;
extern mem_block*heap_tail;

mem_block*get_new_memory_block(size_t size);
int is_block_valid(mem_block*mb);
mem_block*search_first_fit(size_t bytes, const char*file, int line, error_code*err_code);
mem_block*split_block(mem_block*mblock, size_t bytes);
mem_block*merge_blocks(mem_block*mb1, mem_block*mb2);
void invalid_block_message(char*cause, char*consequence);
void dump_full_memory_info();

#endif
