#include<signal.h>
#include<stdio.h>

#include"mem_block.h"

unsigned int magic_number = 0x6164726E;

void check_block(mem_block*mb){
	if(mb->magic_number != magic_number){
		//raise(SIGSEGV);
		printf("bad block\n");
	}
}
