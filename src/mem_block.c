#include<signal.h>
#include<stdio.h>

#include"mem_block.h"
#include"allocator.h"

unsigned int magic_number = 0x6164726E;

int is_block_valid(mem_block*mb){
	//ret 0 if block valid, -1 otherwise
	
	if(mb->magic_number != magic_number){
//		printf("UNVALID BLOCK. Raising sig fault...\n");
//		raise(SIGSEGV);
		return -1;
	}
	return 0;
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
	
	mb1->size += mb2->size + sizeof(mem_block);
	mb1->next = mb2->next;
	if(mb2 == heap_tail){
		heap_tail = mb1;
	}
	
	return mb1;
}
