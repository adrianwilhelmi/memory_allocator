#ifndef __TEST_H__
#define __TEST_H__

size_t round_up(size_t size);

void test_alloc_free();
void test_align();
void test_alloc_use_empty_block();
void test_alloc_zero();
void test_multithread();
void test_huge_alloc();
void test_alloc_and_data_usage();
void test_lots_of_threads();

void test_e2e_no_seg();
void test_e2e_seg();

typedef struct b1{
	//size 56
	int a;
	double b;
	double c;
	double d;
	double e;
	double f;
	double g;
} b1;

typedef struct b3{
	//32bit -> size 12
	//64bit -> size 16
	
	double b;
	char a;
} b3;

typedef struct b4{
	//size 128
	double a[16];
} b4;

#endif
