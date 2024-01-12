#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<signal.h>

#include"mem_block.h"
#include"allocator_stats.h"
#include"allocator.h"

static bool was_initialized = 0;
static pthread_mutex_t allocator_mutex;
static error_code err_code;

static void initialize_allocator(){
	//initializes the mutex and sets up free_all as function to be called at exit
	//if no gnuc then allocator will be initalized with first allocation
	
	if(pthread_mutex_init(&allocator_mutex, NULL) != 0){
		perror("err initializing mutex");
		return;
	}
	
	//first report stats about unfreed blocks and then free them
	if(atexit(free_all) != 0){
		perror("couldnt register function at exit\n");
		return;
	}
	if(atexit(report_stats) != 0){
		perror("couldnt register function at exit\n");
		return;
	}
	
	err_code = NO_ERR;
	was_initialized = 1;
}

static size_t align(size_t size){
	//rounds up size to be divisible by sizeof(size_t)

	size_t remainder = size % sizeof(size_t);
	if(remainder == 0){
		return size;
	}
	return size + sizeof(size_t) - remainder;
}

void*allocate(size_t bytes, const char*file, int line){
	//returns pointer to existing free memory block, or raises the program break if no free block is found
	
	pthread_mutex_lock(&allocator_mutex);
	
	alloc_stats.alloc_calls += 1;
	
	if(bytes <= 0){
		pthread_mutex_unlock(&allocator_mutex);
		return NULL;
	}
	
	bytes = align(bytes);
	
	//search for free memory in previously allocated blocks
	mem_block*mblock = search_first_fit(bytes, file, line, &err_code);
	if(mblock != NULL){
		pthread_mutex_unlock(&allocator_mutex);
		if(err_code == INVALID_BLOCK){
			invalid_block_message("magic number failure", "Raising seg fault...");
			if(raise(SIGSEGV) != 0){
				printf("err raising sigegv\n");
			}
			return NULL;
		}
		return mblock + 1;
	}
	
	//get new memory block if no available memory found
	mblock = get_new_memory_block(bytes);
	
	//initialize global vars if its first allocated block
	if(heap_head == NULL){
		if(!was_initialized){
			initialize_allocator();
			if(!was_initialized){
				return NULL;
			}
		}
		heap_head = mblock;
		heap_tail = mblock;
	}
	else{
		heap_tail->next = mblock;
		heap_tail = heap_tail->next;
		heap_tail->next = NULL;
	}

	mblock->file = file;
	mblock->line = line;
	
	update_stats_add(bytes);
	
	pthread_mutex_unlock(&allocator_mutex);
	
	return mblock + 1;
}

void my_free(void*addr){
	//frees the memory block allocated for pointer addr
	//if this memory block's neighbors are also empty, the blocks get merged
	//memory block is now again available for alloation
	
	pthread_mutex_lock(&allocator_mutex);
	
	if(addr == NULL){
		//do nothing if got null pointer
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
	
	mem_block*to_free = (mem_block*)addr - 1;
	
	//check if addr is a valid address (allocated by this allocator)
	mem_block*mblock;
	for(mblock = heap_head; mblock; mblock = mblock->next){
		if(mblock == to_free){
			break;
		}
	}
	
	if(mblock != to_free){
		invalid_block_message("no such block", "Raising seg fault...");
		pthread_mutex_unlock(&allocator_mutex);
		if(raise(SIGSEGV) != 0){
			printf("err raising sigsegv\n");
		}
		//if sigsegv is ignored then just return
		return;
	}
	if(to_free->is_free){
		invalid_block_message("double free", "Raising seg fault...");
		pthread_mutex_unlock(&allocator_mutex);
		if(raise(SIGSEGV) != 0){
			printf("err raising sigsegv\n");
		}
		return;
	}
	if(is_block_valid(to_free) == -1){
		invalid_block_message("magic number failure", "Raising seg fault...");
		pthread_mutex_unlock(&allocator_mutex);
		if(raise(SIGSEGV) != 0){
			printf("err raising sigsegv\n");
		}
		return;
	}

	to_free->is_free = true;

	if((long long) alloc_stats.memory_usage - (long long)to_free->size < 0){
		printf("ALLOC STATS: mem usage < 0\n");
		pthread_mutex_unlock(&allocator_mutex);
		if(raise(SIGSEGV) != 0){
			printf("err raising sigsegv\n");
		}
		return;
	}
	alloc_stats.memory_usage -= to_free->size;
	
	//merge neighbor free blocks
	//next block
	if(to_free != heap_tail && to_free->next->is_free){
		merge_blocks(to_free, to_free->next);
	}
	//prev block
	mblock = heap_head;
	if(mblock != heap_tail){
		while(mblock->next && mblock->next != to_free){
			mblock = mblock->next;
		}
		if(mblock->next == to_free && mblock->is_free){
			merge_blocks(mblock, to_free);
		}
	}
	
	pthread_mutex_unlock(&allocator_mutex);
}


void free_all(){
	//frees all allocated memory by setting break back to heap head
	//called on exit
	
	pthread_mutex_lock(&allocator_mutex);
	
	if(heap_head == NULL){
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
	
	if(brk(heap_head) == -1){
		perror("err freeing mem");
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
	
	alloc_stats.memory_usage = 0;
	
	heap_head = NULL;
	heap_tail = NULL;
	
	pthread_mutex_unlock(&allocator_mutex);
}

void dump_memory_info(){
	//prints general information about every allocated memory block
	
	pthread_mutex_lock(&allocator_mutex);
	
	if(heap_head == NULL){
		printf("no memory allocated\n");
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
	
	int allocated_exists = 0;
	mem_block*mblock;
	for(mblock = heap_head; mblock; mblock = mblock->next){
		if(mblock->is_free){
			continue;
		}
		allocated_exists = 1;
		break;
	}
	
	if(allocated_exists == 0){
		printf("no memory allocated\n\n");
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
		
	printf("%-12s %-12s %-22s %-22s %-20s %-12s", "block_id", "size", "start", "end", "line", "file");
	printf("\n");
	
	for(mblock = heap_head; mblock; mblock = mblock->next){
		if(mblock->is_free){
			continue;
		}
		printf("%-12d %-12zu %-22p %-22p %-20d %-12s\n",
			mblock->block_id,
			mblock->size,
			(void*)((char*)mblock + sizeof(mem_block)),
			(void*)((char*)mblock + sizeof(mem_block) + mblock->size),
			mblock->file ? mblock->line : -1,
			mblock->file ? mblock->file : "unknown file"
			);
	}
	printf("\n");
	
	pthread_mutex_unlock(&allocator_mutex);
}
