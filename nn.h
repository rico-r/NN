
#ifndef _NN_H
#define _NN_H

#include <stdio.h>
#include "layer.h"
#include "mat.h"
#include "vec.h"

struct nn_struct{
	int i;
	int o;
	int layer_size;
	layer* l;
};
typedef struct nn_struct NN;

void nn_init(NN*nn, int layer_count, int*l, activation* act);
vec*  nn_out(NN*nn, vec*in);
float nn_err(NN*nn, vec*dst, LossFunction);
float nn_train(NN*nn, vec*out, LossFunction);
void  nn_update(NN*nn);
void  nn_savef(NN*nn, FILE*f);
void  nn_savep(NN*nn, const char*path);
void  nn_loadf(NN*nn, FILE*f);
void  nn_loadp(NN*nn, const char*path);
#endif