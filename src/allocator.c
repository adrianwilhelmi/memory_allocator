#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>

#include"allocator.h"

pthread_mutex_t allocator_mutex;
mem_block*heap_head = NULL;
mem_block*heap_tail = NULL;

void initialize_allocator(){
	if(pthread_mutex_init(&allocator_mutex, NULL) != 0){
		perror("err initializing mutex");
		exit(EXIT_FAILURE);
	}
	
	printf("allocator initialized\n");
}

#ifdef __GNUC__
__attribute__((constructor))
void init(){
	initialize_allocator();
}
#endif

static size_t align(size_t size){
	//rounds up size to size of size_t

	size_t remainder = size % sizeof(size_t);
	if(remainder == 0){
		return size;
	}
	return size + sizeof(size_t) - remainder;
}

static mem_block*get_new_memory_block(size_t size){
	//allocates size bytes of memory + memory for metadata
	
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
	
	//initialize global vars if its first allocated block
	if(heap_head == NULL){
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
				
				pthread_mutex_unlock(&allocator_mutex);
				
				return mb + 1;
			}
		}
	}
	

	//get new memory block if no available memory found
	mb = get_new_memory_block(bytes);

//	mb = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	
	pthread_mutex_unlock(&allocator_mutex);
	
	return mb + 1;
}

void free(void*addr){
	if(addr == NULL){
		return;
	}
	
	pthread_mutex_lock(&allocator_mutex);
	
	mem_block*to_free = (mem_block*)addr - 1;
	to_free->is_free = true;
	
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

void dump_memory_info(){
	pthread_mutex_lock(&allocator_mutex);
	
	int counter = 1;
	mem_block*mb;
	printf("Memory info:\n");
	for(mb = heap_head; mb; mb = mb->next){
		if(mb->is_free){
			continue;
		}
		printf("block %d:\n", counter++);
		printf("\tstart:	%p\n", (void*)((uintptr_t)mb + sizeof(mem_block)));
		printf("\tend: 	%p\n", (void*)((uintptr_t)mb + sizeof(mem_block) + mb->size));
		printf("\tsize: 	%zu\n", mb->size);
		printf("\tfile:	\n");
		printf("\tline: 	\n");
	}
	
	pthread_mutex_unlock(&allocator_mutex);
}
