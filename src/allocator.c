#include"allocator.h"

#ifdef __GNUC__
__attribute__((constructor))
void init(){
	initialize_allocator();
}
#endif
