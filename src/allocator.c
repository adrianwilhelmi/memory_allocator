#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>

#include"mem_block.h"
#include"allocator_stats.h"
#include"allocator.h"

mem_block*heap_head = NULL;
mem_block*heap_tail = NULL;
bool was_initialized = 0;
pthread_mutex_t allocator_mutex;
allocator_stats alloc_stats;
unsigned int magic_number = 0x6164726E;
static unsigned int global_block_id = 0;

void initialize_allocator(){
	//initializes the mutex and sets up free_all as function to be called at exit
	
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

//constructor
#ifdef __GNUC__
__attribute__((constructor))
void init(){
	initialize_allocator();
}
#endif

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
	
	//calculate total size
	size_t total_size = size + sizeof(mem_block);
	
	mem_block*new_block = (mem_block*)sbrk(0);
	
	//null if out of memory
	if(sbrk(total_size) == (void*)(-1)){
		perror("out of memory");
		return NULL;
	}
	
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
	
	alloc_stats.sbrk_calls += 1;
	
	return new_block;
}

void*allocate(size_t bytes, const char*file, int line){
	//returns pointer to existing free memory block, or raises the program break if no free block is found
	
	if(bytes == 0){
		return NULL;
	}
	
	pthread_mutex_lock(&allocator_mutex);
	
	bytes = align(bytes);
	
	mem_block*mb;
	
	//search for free memory in previously allocated blocks
	if(heap_head != NULL){
		for(mb = heap_head; mb; mb = mb->next){
			if(mb->is_free == true && mb->size >= bytes){
				//return this, maybe chop it into 2 blocks
				if((mb->size > bytes + sizeof(mem_block) + sizeof(size_t))){
					//enough memory to chop the block 2 into 2 smaller blocks
					size_t new_block_size = mb->size - bytes - sizeof(mem_block);
					mb->is_free = false;
					mb->size = bytes;
					
					mem_block*new_block = (mem_block*)((uintptr_t)(mb + 1) + bytes);
					new_block->is_free = true;
					new_block->size = new_block_size;
					
					new_block->next = mb->next;
					mb->next = new_block;
				}
				else{
					mb->is_free = false;
				}

				mb->file = file;
				mb->line = line;

				alloc_stats.alloc_calls += 1;
				alloc_stats.bytes_alloced += bytes;
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
	
	alloc_stats.alloc_calls += 1;
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
	
	if(addr == NULL){
		//do nothing if got null pointer
		return;
	}
	
	pthread_mutex_lock(&allocator_mutex);
	
	mem_block*to_free = (mem_block*)addr - 1;
	to_free->is_free = true;
	
	alloc_stats.memory_usage -= to_free->size;
	
	//merge neighbor free blocks
	mem_block*mb = heap_head;
	while(mb && mb->next){
		if(mb->next == to_free && mb->is_free){
			mb->size += to_free->size + sizeof(mem_block);
			mb->next = to_free->next;
			if(mb->next == NULL){
				heap_tail = mb;
			}
		}
		mb = mb->next;
	}
	if(to_free->next && to_free->next->is_free){
		to_free->size += to_free->next->size + sizeof(mem_block);
		to_free->next = to_free->next->next;
		if(to_free->next == NULL){
			heap_tail = to_free;
		}
	}
	
	pthread_mutex_unlock(&allocator_mutex);
}

void free_all(){
	//frees all allocated memory
	//called on exit
	
	mem_block*mb;
	for(mb = heap_head; mb; mb = mb->next){
		if(!mb->is_free){
			my_free((void*)(mb + 1));
		}
	}
}

void dump_memory_info(){
	//prints general information about every block of memory allocated
	
	pthread_mutex_lock(&allocator_mutex);
	
	mem_block*mb;
//	printf("MEMORY INFO\n");
	printf("%-12s %-12s %-22s %-22s %-20s %-12s", "block", "size", "start", "end", "line", "file");
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
			
		/*
		printf("%d\t%p\t%p\t%zu\t%s\t%d\n",
			counter++,
			(void*)((uintptr_t)mb + sizeof(mem_block)),
			(void*)((uintptr_t)mb + sizeof(mem_block) + mb->size),
			mb->size,
			mb->file ? mb->file : "unknown file",
			mb->file ? mb->line : -1
			);
		*/
		/*
		printf("block %d:\n", counter++);
		printf("start:	%p\n", (void*)((uintptr_t)mb + sizeof(mem_block)));
		printf("end: 	%p\n", (void*)((uintptr_t)mb + sizeof(mem_block) + mb->size));
		printf("size: 	%zu\n", mb->size);
		printf("file:	%s\n", mb->file ? mb->file : "unknown file");
		printf("line: 	%d\n", mb->file ? mb->line : -1);
		printf("\n");
		*/
	}
	printf("\n");
	
	pthread_mutex_unlock(&allocator_mutex);
}
