#include<stdio.h>

#include"allocator_stats.h"
#include"allocator.h"

void report_stats(){
	if(heap_head == NULL){
		return;
	}
	
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
	
	for(mb = heap_head; mb; mb = mb->next){
		if(!mb->is_free){
			printf("\nUNFREED BLOCKS\n");
			printf("%-9s %-5s %-20s\n", "block_id", "line", "file");
			for(mb = heap_head; mb; mb = mb->next){
				if(!mb->is_free){
					printf("%-9d %-5d %-20s\n", mb->block_id, mb->line, mb->file);
				}
			}
		}
		break;
	}
	
	
}

void clean_stats(allocator_stats*stats){
	stats->bytes_alloced = 0;
	stats->memory_usage = 0;
	stats->max_memory_usage = 0;
	stats->alloc_calls = 0;
	stats->sbrk_calls = 0;
}
