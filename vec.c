#include "vec.h"
#include <malloc.h>
#include <stdlib.h>

float frand();

void vec_c_init(vec* v,int size){
	v->size=size;
	v->v=(float*)calloc(size,4);
}
void vec_m_init(vec* v,int size){
	v->size=size;
	v->v=(float*)malloc(size*4);
}
void vec_fill_rand(vec*v){
	int sz=v->size;
	for(int i=0;i<sz;i++){
		v->v[i]=0.1f*((float)rand()*2/RAND_MAX-1);
	}
}
void vec_fill_init(vec*v,int size){
	vec_m_init(v,size);
	vec_fill_rand(v);
}
void vec_rem(vec*v){
	free(v->v);
}
void vec_load(vec*v,FILE*f){
	fread(&v->size,4,1,f);
	v->v=(float*)malloc(4*v->size);
	fread(v->v, 4, v->size, f);
}
void vec_save(vec*v,FILE*f){
	fwrite(&v->size, 4, 1,f);
	fwrite(v->v, 4, v->size, f);
}