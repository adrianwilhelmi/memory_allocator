#include<stdio.h>

#include"allocator_stats.h"
#include"allocator.h"

void report_stats(){
	int broken_blocks = 0;
	mem_block*mb;
	for(mb = heap_head; mb; mb = mb->next){
		if(mb->magic_number != magic_number){
			broken_blocks++;
		}
	}
	
	printf("\n========ALLOCATOR STATS========\n");
	printf("number of alloc calls	%d\n", alloc_stats.alloc_calls);
	printf("number of sbrk calls	%d\n", alloc_stats.sbrk_calls);
	printf("total bytes alloced	%zu\n", alloc_stats.bytes_alloced);
	printf("mean bytes alloced	%zu\n", alloc_stats.bytes_alloced / alloc_stats.alloc_calls);
	printf("max memory usage	%zu\n", alloc_stats.max_memory_usage);
	printf("number of broken blocks	%d\n", broken_blocks);
	
	printf("\nUNFREED BLOCKS\n");
	printf("block\tfile\tline\n");
	int counter = 1;
	for(mb = heap_head; mb; mb = mb->next){
		if(!mb->is_free){
			printf("%d\t%s\t%d\n", counter++, mb->file, mb->line);
		}
	}
}
