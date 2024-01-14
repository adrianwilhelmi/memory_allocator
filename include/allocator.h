//Author: Adrian Wilhelmi

#ifndef __ALLOCATOR_H__
#define  __ALLOCATOR_H__

#include<stddef.h>

#define alloc(bytes) allocate(bytes, __FILE__, __LINE__)
#define free(addr) my_free(addr)

void*allocate(size_t bytes, const char*file, int line);
void my_free(void*addr);
//void free_all();
void dump_memory_info();

#endif
