#include<signal.h>
#include<stdio.h>
#include<stdbool.h>

#include"mem_block.h"
#include"allocator.h"

unsigned int magic_number = MAGIC_NUMBER;
unsigned int global_block_id = 0;
mem_block*heap_head = NULL;
mem_block*heap_tail = NULL;

mem_block*get_new_memory_block(size_t size){
	//allocates size bytes of memory + memory for metadata by raising the program break with sbrk()
	
	alloc_stats.sbrk_calls += 1;
	
	//calculate total size
	size_t total_size = size + sizeof(mem_block);
	
	mem_block*new_block = (mem_block*)sbrk(0);
	//null if out of memory
	if(sbrk((intptr_t)total_size) != new_block){
		perror("out of memory");
		return NULL;
	}
	
	new_block->true_size = size;
	new_block->size = size;
	new_block->is_free = false;
	new_block->next = NULL;
	new_block->magic_number = magic_number;
	new_block->block_id = global_block_id++;
	
	return new_block;
}

int is_block_valid(mem_block*mblock){
	//ret 0 if block valid, -1 otherwise
	
	if(mblock->magic_number != magic_number){
		return -1;
	}
	return 0;
}

mem_block*search_first_fit(size_t bytes, const char*file, int line, error_code*err_code){
	//searches for first free block that has size of at least (bytes)
	
	mem_block*mblock;
	if(heap_head != NULL){
		for(mblock = heap_head; mblock; mblock = mblock->next){
			if(is_block_valid(mblock) == -1){
				*err_code = INVALID_BLOCK;
				return NULL;
			}
			
			if(mblock->is_free && mblock->size >= bytes){
				mblock->file = file;
				mblock->line = line;
				
				if((mblock->true_size > bytes + sizeof(mem_block) + sizeof(size_t))){
					//enough memory to chop the block into 2 smaller blocks
					if(split_block(mblock, bytes) == NULL){
						printf("block too small to split\n");
						return NULL;
					}
				}
				else{
					mblock->is_free = false;
					mblock->size = bytes;
				}

				update_stats_add(bytes);
								
				return mblock;
			}
		}
	}
	
	return NULL;
}

mem_block*split_block(mem_block*mblock, size_t bytes){
	//splits block into two smaller block of sizes bytes, size-bytes-sizeof(mem_block) respectively
	
	size_t new_block_size = mblock->size - bytes - sizeof(mem_block);
	if(new_block_size < sizeof(size_t)){
		return NULL;
	}
	
	mblock->is_free = false;
	mblock->size = bytes;
	mblock->true_size = bytes;

	bool is_tail = false;
	if(mblock == heap_tail){
		is_tail = true;
	}
					
	mem_block*new_block = (mem_block*)((char*)(mblock + 1) + bytes);
	new_block->is_free = true;
	new_block->size = new_block_size;
	new_block->true_size = new_block_size;
	new_block->magic_number = magic_number;
	new_block->block_id = global_block_id++;
	new_block->file = mblock->file;
	new_block->line = mblock->line;
					
	new_block->next = mblock->next;
	if(is_tail){
		heap_tail = new_block;
	}
					
	mblock->next = new_block;
	mblock->magic_number = magic_number;
	
	return mblock;
}

mem_block*merge_blocks(mem_block*mb1, mem_block*mb2){
	//merges given blocks and returns merged block
	//function assumes mb1 is the block before mb2
	
	if(mb1->next != mb2){
		printf("blocks not adjacent\n");
		return mb1;
	}
	
	if(!(mb1->is_free) || !(mb2->is_free)){
		printf("blocks aint free\n");
		return mb1;
	}
	
	mb1->true_size += mb2->true_size + sizeof(mem_block);
	mb1->size = mb1->true_size;
	mb1->next = mb2->next;
	if(mb2 == heap_tail){
		heap_tail = mb1;
	}
	mb1->magic_number = magic_number;
	
	return mb1;
}

void invalid_block_message(char*cause, char*consequence){
	if(cause == NULL || consequence == NULL){
		return;
	}
	
	printf("UNVALID BLOCK: %s. %s\n", cause, consequence);
}
