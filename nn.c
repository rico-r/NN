
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "nn.h"

void nn_init(NN*nn, int layer_count, int*l, activation* act){
	nn->i=l[0];
	nn->layer_size=layer_count-1;
	layer*lay=(layer*)malloc(sizeof (struct _layer)*nn->layer_size);
	int c=l[0];
	for(int i=1; i<layer_count; i++){
		layer_fill_init(&lay[i-1], c, l[i], act[i]);
		
		c=l[i];
	}
	for(int i=1; i<nn->layer_size;i++){
		lay[i]._in.v=lay[i-1]._out.v;
	}
	for(int i=0; i<nn->layer_size-1;i++){
		lay[i+1].terr.v=lay[i].oerr.v;
	}
	nn->o=c;
	nn->l=lay;
}
float clamp(float v){
	if(v<=0)return 0.0f;
	if(v>=1)return 1.0f;
	return v;
}

vec* nn_out(NN*nn, vec*in){
	layer_setinput(nn->l, in);
	int n = nn->layer_size;
	for(register int i=0; i<n; i++){
		layer_out(&nn->l[i]);
	}
	return &nn->l[n-1]._out;
}

float nn_err(NN*nn, vec *dst, LossFunction lf){
	return layer_err(&nn->l[nn->layer_size-1], dst, lf);
}

/**
 * train given NeuralNetwork with last data input
 * note: call nn_update() to actualy "training" and update NeuralNetwork weight and bias
 * @param nn - NeuralNetwork to be train
 * @param out desired output
 * @returns - output error
 */
float nn_train(NN*nn, vec*out, LossFunction lf){
//	vec E, e;
	register int i, k;
	float err=0;
//	vec_m_init(&E, 0);
//	vec_m_init(&e, nn->o);
	{
		layer *l=&nn->l[nn->layer_size-1];
		float *v=l->_out.v;
		float *ev=l->oerr.v;
		float *w=out->v;
		float tmp;
		for(k=0; k<nn->o; k++){
			tmp=v[k] - w[k];
			ev[k]=lf.d(tmp);
			err+=lf.o(tmp);
		}
	}
	for(i=nn->layer_size-1; i>=0; i--){
//		vec_rem(&E);
//		E=e;

		layer* l=&nn->l[i];
		if(l->terr.v)
			memset(l->terr.v, 0, l->i*sizeof(float));
//		vec_c_init(&e, l->i);
		layer_train(l);
	}
//	vec_rem(&E);
//	vec_rem(&e);
	return err;
}

void nn_update(NN*nn){
	for(register int i=0;i<nn->layer_size;i++){
		layer_update(&nn->l[i]);
	}
}

/**
 * write neural network to a file.
 * don't forget to open the fil ein byte mode.
 */
void nn_savef(NN *nn, FILE *f){
	fwrite(&nn->layer_size,4,1,f);
	int file_size = 4;
	for(register int i=0; i<nn->layer_size; i++){
		layer l=nn->l[i];
		int io[]={l.i, l.o, l.act.id};
		fwrite(io, 3*4, 1, f);
		fwrite(l.w.m, l.i*l.o*4, 1,f);
		fwrite(l.b.v, l.o*4, 1,f);
		file_size += 12 + l.i * l.o * 4 + l.o * 4;
	}
}

void nn_savep(NN*nn, const char*path){
	FILE*p=fopen(path,"wb");
	nn_savef(nn, p);
	fclose(p);
}

void nn_loadp(NN* nn, const char* path){
	FILE* file=fopen(path, "rb");
	assert(file);
	nn_loadf(nn, file);
	fclose(file);
}

/**
 * read neural network from a file.
 * don't forget to open the fil ein byte mode.
 */
void nn_loadf(NN* nn, FILE* f){
	int readed;
	int io[3];
	readed = fread(io, sizeof(int), 1, f);
	assert(readed == 1);
	nn->layer_size=io[0];
	assert(nn->layer_size>0);
	assert(nn->layer_size<100);
	layer *lay = (layer*) malloc(sizeof (layer)*nn->layer_size);
	assert(lay!=NULL);
	for(int i=0;i<nn->layer_size;i++){
		readed = fread(io, 4, 3, f);
		assert(readed == 3);
		if(i==0){
			nn->i = io[0];
		}
		layer_fill_init(&lay[i], io[0], io[1], act_list[io[2]]);
		readed = fread(lay[i].w.m, 4, io[0]*io[1], f);
		assert(readed == io[0] * io[1]);
		readed = fread(lay[i].b.v, 4, io[1], f);
		assert(readed == io[1]);
	}
	for(int i=1; i<nn->layer_size; i++){
		lay[i]._in.v = lay[i-1]._out.v;;
	}
	for(int i=0; i<nn->layer_size-1; i++){
		lay[i+1].terr.v = lay[i].oerr.v;
	}
	nn->l=lay;
//	nn->i=lay[0].i;
	nn->o=io[1];
}