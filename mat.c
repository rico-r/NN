
#include <malloc.h>
#include <stdlib.h>
#include <limits.h>

#include "mat.h"

void mat_m_init(mat*m,int w,int h){
	m->w=w;
	m->h=h;
	m->m=(float*)malloc(w*h*4);
}
void mat_c_init(mat*m,int w,int h){
	m->w=w;
	m->h=h;
	m->m=(float*)calloc(w*h*4,4);
}
void mat_fill_rand(mat*m){
	int c=m->w*m->h;
	for(int i=0;i<c;i++){
		m->m[i]=0.1f*((float)rand()*2/RAND_MAX-1);
	}
}