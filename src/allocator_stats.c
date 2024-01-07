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
	
	printf("\nALLOCATOR STATS\n");
	
	printf("%-12s %-12s %-22s %-22s %-20s %-12s \n", "alloc_calls", "sbrk_calls", "total_bytes_alloced",
		"mean_bytes_alloced", "max_memory_usage", "broken_blocks"
		);
	
	printf("%-12d %-12d %-22zu %-22zu %-20zu %-12d \n",
		alloc_stats.alloc_calls,
		alloc_stats.sbrk_calls,
		alloc_stats.bytes_alloced,
		alloc_stats.bytes_alloced / alloc_stats.alloc_calls,
		alloc_stats.max_memory_usage,
		broken_blocks
		);
	
	/*
	printf("alloc_calls	%d\n", alloc_stats.alloc_calls);
	printf("sbrk_calls	%d\n", alloc_stats.sbrk_calls);
	printf("total_bytes_alloced	%zu\n", alloc_stats.bytes_alloced);
	printf("mean_bytes_alloced	%zu\n", alloc_stats.bytes_alloced / alloc_stats.alloc_calls);
	printf("max_memory_usage	%zu\n", alloc_stats.max_memory_usage);
	printf("broken_blocks	%d\n", broken_blocks);
	*/
	
	printf("\nUNFREED BLOCKS\n");
	printf("%-6s %-5s %-20s\n", "block", "line", "file");
	int counter = 1;
	for(mb = heap_head; mb; mb = mb->next){
		if(!mb->is_free){
			printf("%-6d %-5d %-20s\n", counter++, mb->line, mb->file);
		}
	}
}
