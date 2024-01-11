#include<signal.h>
#include<stdio.h>

#include"mem_block.h"
#include"allocator.h"

unsigned int magic_number = 0x6164726E;

int is_block_valid(mem_block*mb){
	//ret 0 if block valid, -1 otherwise
	
	if(mb->magic_number != magic_number){
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

void dump_full_memory_info(){
	//prints information about every memory block
	
	pthread_mutex_lock(&allocator_mutex);
	if(heap_head == NULL){
		printf("no memory allocated\n");
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
	
	printf("%-12s %-12s %-22s %-22s %-20s %-12s %-12s", "block_id", "size", "start", "end", "line", "file", "is_free");
	printf("\n");
	
	mem_block*mb;
	for(mb = heap_head; mb; mb = mb->next){
		printf("%-12d %-12zu %-22p %-22p %-20d %-12s %-12d\n",
			mb->block_id,
			mb->size,
			(void*)((uintptr_t)mb + sizeof(mem_block)),
			(void*)((uintptr_t)mb + sizeof(mem_block) + mb->size),
			mb->file ? mb->line : -1,
			mb->file ? mb->file : "unknown file",
			mb->is_free
			);
	}
	printf("\n");
	
	printf("heap tail:\n");	
	printf("%-12d %-12zu %-22p %-22p %-20d %-12s\n",
		heap_tail->block_id,
		heap_tail->size,
		(void*)((uintptr_t)heap_tail + sizeof(mem_block)),
		(void*)((uintptr_t)heap_tail + sizeof(mem_block) + heap_tail->size),
		heap_tail->file ? heap_tail->line : -1,
		heap_tail->file ? heap_tail->file : "unknown file"
		);
	printf("\n");

	pthread_mutex_unlock(&allocator_mutex);
}
