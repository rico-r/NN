
#ifndef _VEC_H
#define _VEC_H
#include <stdio.h>

struct vec_struct{
	int size;
	float*v;
};
typedef struct vec_struct vec;
void vec_c_init(vec* v, int size);
void vec_m_init(vec* v, int size);
void vec_fill_rand(vec*v);
void vec_fill_init(vec*v, int size);
void vec_rem(vec*v);
void vec_load(vec* v,FILE* f);
void vec_save(vec* v,FILE* f);
#endif