#ifndef __TEST_H__
#define __TEST_H__

void test_alloc_free();

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

typedef struct b2{
	//64bit -> size 64
	//32bit -> size 60
	
	int a;
	double b;
	double c;
	double d;
	double e;
	double f;
	double g;
	int h;
} b2;

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
