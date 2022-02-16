
#ifndef _MAT_H
#define _MAT_H

struct _mat{
	int w,h;
	float*m;
};
#define mat struct _mat
void mat_m_init(mat*m,int w,int h);
void mat_c_init(mat*m,int w,int h);
void mat_fill_rand(mat*m);
#endif