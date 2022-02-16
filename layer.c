
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "layer.h"

float LearnRate=1;
float WeightRate=1;

float logsig_o(vec *v){
	for(register int i=0; i<v->size; i++){
		v->v[i] = 1/(1+expf(-v->v[i]));
	}
}
float logsig_d(vec *dst, vec *out){
	register float o;
	for(register int i=0; i<out->size; i++){
		o = out->v[i];
		dst->v[i] *= o*(1-o);
	}
}

float softmax_o(vec *v){
	register int i, size = v->size;
	register float sum = 0, tmp;
	for(i=0; i<size; i++){
		tmp = expf(v->v[i]);
		v->v[i] = tmp;
		sum += tmp;
	}
	for(i=0; i<size; i++){
		v->v[i] /= sum;
	}
}
float softmax_d(vec *dst, vec *out){
	register float o;
	for(register int i=0; i<out->size; i++){
		o = out->v[i];
		dst->v[i] *= o*(1-o);
	}
}

float logLossFunction_o(float x){
	return -logf(1-x)*logf(1+x);
}
float logLossFunction_d(float x){
	return logf(1+x)/(1-x)-logf(1-x)/(1+x);
}
float squaredLossFunction_o(float x){
	return 0.5f*x*x;
}
float squaredLossFunction_d(float x){
	return x;
}

activation logsig, softmax;
activation *act_list = (activation*) 0;
LossFunction logLossFunction, squaredLossFunction;
void layer_init(){
	logsig = (activation) {logsig_o, logsig_d, 0, "sigmoid"};
	softmax = (activation) {softmax_o, softmax_d, 1, "softmax"};
	act_list = (activation*) malloc(3*sizeof(activation));
	act_list[0] = logsig;
	act_list[1] = softmax;
	act_list[2] = (activation) {0, 0, -1, 0};
	
	logLossFunction=(LossFunction){logLossFunction_o, logLossFunction_d};
	squaredLossFunction=(LossFunction){squaredLossFunction_o, squaredLossFunction_d};
}

void layer_fill_init(layer*l, int i, int o, activation act){
	l->i=i;
	l->o=o;
	l->act=act;
	mat_m_init(&l->w,i,o);
	mat_fill_rand(&l->w);
	vec_fill_init(&l->b,o);
	l->_in.size=i;
	l->_in.v=NULL;
	l->terr.size=i;
	l->terr.v=NULL;
	vec_m_init(&l->oerr, o);
//	vec_fill_init(&l->_in,i);
	vec_fill_init(&l->_out,o);
	vec_c_init(&l->nb,o);
	mat_c_init(&l->nw,i,o);
}
void layer_m_init(layer *l, int i, int o, activation act){
	l->i = i;
	l->o = o;
	l->act = act;
	mat_m_init(&l->w, i, o);
	vec_m_init(&l->b, o);
	l->_in.size = i;
	l->_in.v = NULL;
	l->terr.size = i;
	l->terr.v = NULL;
	vec_m_init(&l->oerr, o);
//	vec_m_init(&l->_in, i);
	vec_m_init(&l->_out, o);
	vec_c_init(&l->nb, o);
	mat_c_init(&l->nw, i, o);
}
void layer_setinput(layer*l, vec*in){
	// assert(l->_in.size==in->size);
	l->_in.v=in->v;
}

void layer_out(layer *l){
//	memcpy(l->_in.v, in->v, in->size*sizeof(float));
	register float sum, *inv=l->_in.v, *m=l->w.m;
	register int o, i, IN=l->i, OUT=l->o;
	for(o=0; o<OUT; o++){
		sum = l->b.v[o];
		for(i=0; i<IN; i++){
			sum += (*m) * inv[i];
			m++;
		}
		// assert(l->_out.v[o]<100);
		// l->_out.v[o] = l->act.o(sum);
		l->_out.v[o] = sum;
	}
	l->act.o(&l->_out);
}
float layer_err(layer*l, vec*out, LossFunction lf){
	register float r=0;
	for(register int i=0;i<out->size;i++){
		r+=lf.o(l->_out.v[i] - out->v[i]);
//		printf("%f\n",powf(out->v[i]-l->_out.v[i],2)/2);
	}
	return r;
}
void layer_train(layer *l){
	register int i, o, I, IN=l->i, OUT=l->o;
	register float u,
		*nb=l->nb.v,
		*nw=l->nw.m,
		*w=l->w.m,
		*inv=l->_in.v,
		// *outv=l->_out.v,
		*Ev=l->oerr.v,
		*ev=l->terr.v;
	l->act.d(&l->oerr, &l->_out);
	if(ev){
		for(I=0, o=0; o<OUT; o++){
			u = Ev[o];
			// u=l->act.d(outv[o])*Ev[o];
			nb[o]+=u;
			for(i=0; i<IN; i++, I++){
				nw[I]+=inv[i]*u;
				ev[i]+=u*w[I];
			}
		}
	}else{
		for(I=0, o=0; o<OUT; o++){
			u = Ev[o];
			// u=l->act.d(outv[o])*Ev[o];
			nb[o]+=u;
			for(i=0; i<IN; i++, I++){
				nw[I]+=inv[i]*u;
			}
		}
	}
}
void layer_update(register layer*l){
	register int i, s=l->i*l->o;
	register float lr=0.1f*LearnRate,
				   wr=WeightRate;
	register float
		*w=l->w.m,
		*nw=l->nw.m,
		*b=l->b.v,
		*nb=l->nb.v;
	for(i=0;i<s;i++){
//		w[i]=fminf(10,fmaxf(-10,w[i]-lr*nw[i]));
		w[i]-=lr*(nw[i]+wr*w[i]);
		nw[i]*=0.9f;
	}
	s=l->o;
	for(i=0;i<s;i++){
		b[i]-=lr*(nb[i]+wr*b[i]);
//		b[i]=fminf(10,fmaxf(-10,b[i]-lr*nb[i]));
		nb[i]*=0.9f;
	}
}