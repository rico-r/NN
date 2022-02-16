
#ifndef _LAYER_H
#define _LAYER_H

#include "mat.h"
#include "vec.h"
extern float LearnRate;
extern float WeightRate;

struct _activation{
	float (*o)(vec* v);
	float (*d)(vec* dst, vec* src);
	int id;
	char *name;
};
typedef struct _activation activation;

typedef struct{
	float (*o)(float);
	float (*d)(float);
} LossFunction;

struct _layer{
	int i,o;
	mat w;
	vec b;
	vec _in, _out;
	vec terr;//transfered error
	vec oerr;//output error
	vec nb;
	mat nw;
	activation act;
};
typedef struct _layer layer;

extern activation logsig, softmax;
extern activation *act_list;
extern LossFunction logLossFunction, squaredLossFunction;

void layer_init();
void layer_m_init(layer*l, int i, int o, activation act);
void layer_fill_init(layer*l,int i,int o,activation act);
void layer_setinput(layer*l, vec* in);
void layer_out(layer*l);
float layer_err(layer*l, vec*out, LossFunction);
void layer_train(layer *l);
void layer_update(layer*l);
#endif