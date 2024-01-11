#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include<signal.h>

#include"mem_block.h"
#include"allocator_stats.h"
#include"allocator.h"

mem_block*heap_head = NULL;
mem_block*heap_tail = NULL;
bool was_initialized = 0;
pthread_mutex_t allocator_mutex;
allocator_stats alloc_stats;
static unsigned int global_block_id = 0;

void initialize_allocator(){
	//initializes the mutex and sets up free_all as function to be called at exit
	//if no gnuc then allocator will be initalized with first allocation
	
	if(pthread_mutex_init(&allocator_mutex, NULL) != 0){
		perror("err initializing mutex");
		exit(EXIT_FAILURE);
	}
	
	//first report stats about unfreed blocks and then free them
	atexit(free_all);
	atexit(report_stats);
	
	was_initialized = 1;
	printf("allocator initialized\n");
}

static size_t align(size_t size){
	//rounds up size to be divisible by sizeof(size_t)

	size_t remainder = size % sizeof(size_t);
	if(remainder == 0){
		return size;
	}
	return size + sizeof(size_t) - remainder;
}

static mem_block*get_new_memory_block(size_t size){
	//allocates size bytes of memory + memory for metadata by raising the program break with sbrk()
	
	alloc_stats.sbrk_calls += 1;
	
	//calculate total size
	size_t total_size = size + sizeof(mem_block);
	
	mem_block*new_block = (mem_block*)sbrk(0);
	
	//null if out of memory
	if(sbrk(total_size) == (void*)(-1)){
		perror("out of memory");
		return NULL;
	}
	
	new_block->true_size = size;
	new_block->size = size;
	new_block->is_free = false;
	new_block->next = NULL;
	new_block->magic_number = magic_number;
	new_block->block_id = global_block_id++;
	
	//initialize global vars if its first allocated block
	if(heap_head == NULL){
		if(!was_initialized){
			initialize_allocator();
		}
		heap_head = new_block;
		heap_tail = new_block;
	}
	else{
		heap_tail->next = new_block;
		heap_tail = heap_tail->next;
		heap_tail->next = NULL;
	}
	
	return new_block;
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
	
	mem_block*mb;
	
	//search for free memory in previously allocated blocks
	if(heap_head != NULL){
		for(mb = heap_head; mb; mb = mb->next){
			if(mb->is_free && mb->size >= bytes){
				if(is_block_valid(mb) == -1){
					printf("invalid in searching\n");
					printf("INVALID BLOCK. Raising sig fault...\n");
					pthread_mutex_unlock(&allocator_mutex);
					raise(SIGSEGV);
				}
				
				if((mb->true_size > bytes + sizeof(mem_block) + sizeof(size_t))){
					//enough memory to chop the block into 2 smaller blocks
					size_t new_block_size = mb->size - bytes - sizeof(mem_block);
					mb->is_free = false;
					mb->size = bytes;
					mb->true_size = bytes;
					
					bool is_tail = false;
					if(mb == heap_tail){
						is_tail = true;
					}
					
					mem_block*new_block = (mem_block*)((uintptr_t)(mb + 1) + bytes);
					new_block->is_free = true;
					new_block->size = new_block_size;
					new_block->true_size = new_block_size;
					new_block->magic_number = magic_number;
					new_block->block_id = global_block_id++;
					new_block->file = file;
					new_block->line = line;
					
					new_block->next = mb->next;
					if(is_tail){
						heap_tail = new_block;
					}
					
					mb->next = new_block;					
					mb->magic_number = magic_number;

				}
				else{
					mb->is_free = false;
					
					//WRONG
					mb->size = bytes;
				}

				mb->file = file;
				mb->line = line;

				alloc_stats.bytes_alloced += bytes;
				alloc_stats.memory_usage += bytes;
				if(alloc_stats.memory_usage > alloc_stats.max_memory_usage){
					alloc_stats.max_memory_usage = alloc_stats.memory_usage;
				}
				
				pthread_mutex_unlock(&allocator_mutex);
								
				return mb + 1;
			}
		}
	}

	//get new memory block if no available memory found
	mb = get_new_memory_block(bytes);

	mb->file = file;
	mb->line = line;
	
	alloc_stats.bytes_alloced += bytes;
	alloc_stats.memory_usage += bytes;
	if(alloc_stats.memory_usage > alloc_stats.max_memory_usage){
		alloc_stats.max_memory_usage = alloc_stats.memory_usage;
	}
	
	pthread_mutex_unlock(&allocator_mutex);
	
	return mb + 1;
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
	mem_block*mb;
	for(mb = heap_head; mb; mb = mb->next){
		if(mb == to_free){
			break;
		}
	}
	
	if(mb != to_free){
		printf("No such block\n");
		printf("UNVALID BLOCK. Raising sig fault...\n");
		pthread_mutex_unlock(&allocator_mutex);
		raise(SIGSEGV);
	}
	if(to_free->is_free){
		printf("double free\n");
		printf("UNVALID BLOCK. Raising sig fault...\n");
		pthread_mutex_unlock(&allocator_mutex);
		raise(SIGSEGV);
	}
	if(is_block_valid(to_free) == -1){
		printf("UNVALID BLOCK. Raising sig fault...\n");
		pthread_mutex_unlock(&allocator_mutex);
		raise(SIGSEGV);
	}

	to_free->is_free = true;

	long long mem_usage = alloc_stats.memory_usage;
	alloc_stats.memory_usage -= to_free->size;
	
	
	if(mem_usage - (int)to_free->size < 0){
		printf("ALLOC STATS: mem usage < 0\n");
		pthread_mutex_unlock(&allocator_mutex);
		raise(SIGSEGV);
	}
	
	
	//merge neighbor free blocks
	//next block
	if(to_free != heap_tail && to_free->next->is_free){
		to_free = merge_blocks(to_free, to_free->next);
	}
	//prev block
	mb = heap_head;
	if(mb != heap_tail){
		while(mb->next && mb->next != to_free){
			mb = mb->next;
		}
		if(mb->next == to_free && mb->is_free){
			mb = merge_blocks(mb, to_free);
		}
	}
	
	pthread_mutex_unlock(&allocator_mutex);
}


void free_all(){
	//frees all allocated memory by setting break back to heap head
	//called on exit
	
	pthread_mutex_lock(&allocator_mutex);
	
	if(brk(heap_head) == -1){
		perror("err freeing mem");
		pthread_mutex_unlock(&allocator_mutex);
		exit(EXIT_FAILURE);
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
	mem_block*mb;
	for(mb = heap_head; mb; mb = mb->next){
		if(mb->is_free){
			continue;
		}else{
			allocated_exists = 1;
			break;
		}
	}
	
	if(allocated_exists == 0){
		printf("no memory allocated\n\n");
		pthread_mutex_unlock(&allocator_mutex);
		return;
	}
		
	printf("%-12s %-12s %-22s %-22s %-20s %-12s", "block_id", "size", "start", "end", "line", "file");
	printf("\n");
	
	for(mb = heap_head; mb; mb = mb->next){
		if(mb->is_free){
			continue;
		}
		printf("%-12d %-12zu %-22p %-22p %-20d %-12s\n",
			mb->block_id,
			mb->size,
			(void*)((uintptr_t)mb + sizeof(mem_block)),
			(void*)((uintptr_t)mb + sizeof(mem_block) + mb->size),
			mb->file ? mb->line : -1,
			mb->file ? mb->file : "unknown file"
			);
	}
	printf("\n");
	
	pthread_mutex_unlock(&allocator_mutex);
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
