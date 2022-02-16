
#ifndef MAIN_C
#error "nn_cmd.c is not standalone"
#endif

int nnTRAIN(ParsedArguments *args){
	signal(SIGINT, mhandler);

	Dataset ds;
	NN nn;
	LossFunction lossFunction=squaredLossFunction;
	register int group = !ParsedArguments_getBoolean(args, "no-group");
	int c = ParsedArguments_getInteger(args, "cycle");
	int sc = ParsedArguments_getInteger(args, "subcycle");
	float lr = ParsedArguments_getFloat(args, "learnrate");
	char *sv = ParsedArguments_getString(args, "output");
	ds_loadf(&ds, ParsedArguments_getFile(args, "ds"));
	{
		char *nnfile = ParsedArguments_getString(args, "nn");
		nn_loadp(&nn, nnfile);
		if(sv==0){
			sv = nnfile;
		}
	}

	if(ds.out[0][0].size!=nn.o){
		printf("\e[33mDataset.output=%d, but NN.output=%d\e[0m\n", ds.out[0][0].size, nn.o);
		return 0;
	}
	if(ds.in[0][0].size!=nn.i){
		printf("\e[33mDataset.input=%d, but NN.input=%d\e[0m\n", ds.in[0][0].size, nn.i);
		return 0;
	}
	LearnRate=lr/sc;
	WeightRate=0.0f;
	if(!group) ds_collect(&ds);
	
	register int i,j,k,l;
	register float err;
	for(i=1; i<=c & run; i++){
		err=0;
		for(j=0; j<sc; j++){
			if(group){
				k=rand()%ds.count;
				l=rand()%ds.size[k];
				nn_out(&nn, &ds.in[k][l]);
				err+=nn_train(&nn, &ds.out[k][l], lossFunction);
			}else{
				k=rand()%ds.all_size;
				nn_out(&nn, ds.all_in[k]);
				err+=nn_train(&nn, ds.all_out[k], lossFunction);
			}
		}
		err/=sc;
		printf("%5d %.5f\n", i, err);
		nn_update(&nn);
	}
	nn_savep(&nn, sv);
	return 0;
}

int maxvi(vec* v){
	int n=0;
	float val=0;
	for(register int i=0;i<v->size;i++){
		if(v->v[i]>val){
			val=v->v[i];
			n=i;
		}
	}
	return n;
}

int nnSTAT(ParsedArguments *args){
	Dataset ds;
	NN nn;
	LossFunction lossFunction = squaredLossFunction;
	ds_loadf(&ds, ParsedArguments_getFile(args, "ds"));
	nn_loadf(&nn, ParsedArguments_getFile(args, "nn"));
	
	register int i,j, correct, ccorrect=0, total=0;
	register float err, cerr=0;
	for(i=0; i<ds.count; i++){
		err=0; correct=0;
		for(j=0; j<ds.size[i]; j++){
			if(maxvi(nn_out(&nn, &ds.in[i][j]))==i){
				correct++;
			}
			err+=nn_err(&nn, &ds.out[i][j], lossFunction);
		}
		err/=j;
		cerr+=err; ccorrect+=correct; total+=ds.size[i];
		printf("%2d %3d/%d %.3f%% %f\n", i, correct, ds.size[i], (float)correct*100/ds.size[i], err);
	}
	cerr/=ds.count;
	printf("   %3d/%d %.3f%% %f\n", ccorrect, total, (float)ccorrect*100/total, cerr);
	return 0;
}

void max2(vec*a, int n, int* idx){
	register int i1=0, i=0;
	register float v1=0, v;
	for(i=0; i<a->size; i++){
		if(i==n)continue;
		v=a->v[i];
		if(v>v1){
			v1=v;
			i1=i;
		}
	}
	*idx=i1;
}
int nnSEP(int ac, char**args){
	needMin(2)
	Dataset ds;
	NN nn;
	ds_loadp(&ds, args[0]);
	nn_loadp(&nn, args[1]);
	
	register int i,j,n=0;
	int m;
	for(i=0; i<ds.count; i++){
		printf("# [sep%d]\n", i);
		for(j=0; j<ds.size[i]; j++){
			vec* o=nn_out(&nn, &ds.in[i][j]);
			max2(o, i, &m);
			printf("%3d %d %f %d %f\n", n++, o->v[i]>o->v[m], o->v[i], m, o->v[m]);
		}
		printf("\n\n");
	}
	return 0;
}

int nnOut(ParsedArguments *args){
	NN nn;
	Dataset ds;
	ds_loadf(&ds, ParsedArguments_getFile(args, "ds"));
	nn_loadf(&nn, ParsedArguments_getFile(args, "nn"));
	if(ds.in[0][0].size!=nn.i){
		printf("\e[33mDataset.input=%d, but NN.input=%d\e[0m\n", ds.in[0][0].size, nn.i);
		return 0;
	}
	int i = ParsedArguments_getInteger(args, "index1");
	int j = ParsedArguments_getInteger(args, "index2");
	vec *out = &ds.in[i][j];
	out = nn_out(&nn, out);
	for(int k=0; k<out->size; k++){
		if(k){
			printf(", ");
		}
		printf("%f", out->v[k]);
	}
	printf("\n");
	return 0;
}

int nnNEW(ParsedArguments *args){
	int count = ParsedArguments_count(args, "layer");
	int layers[count];
	activation activations[count];
	printf("create simple neural network layer:");
	for(int i=0; i<count; i++){
		int layer = ParsedArguments_getInteger(args, "layer");
		if(layer==0){
			fprintf(stderr,"\e[33mlayer size can't be zero\e[0m\n");
			return 1;
		}
		activations[i] = logsig;
		layers[i] = layer;
		printf(" %d", layer);
	}
	printf("\n");
	NN nn;
	nn_init(&nn, count, layers, activations);
	nn_savef(&nn, ParsedArguments_getFile(args, "dst"));
	return 0;
}

int nnINFO(ParsedArguments *args){
	NN nn;
	nn_loadf(&nn, ParsedArguments_getFile(args, "nn"));
	printf("layer  1 (%s): %d\n", nn.l[0].act.name, nn.i);
	for(int i=0; i<nn.layer_size; i++){
		printf("layer %2d (%s): %d\n", i+2, nn.l[i].act.name, nn.l[i].o);
	}
	return 0;
}

void decompose(int* m,int* n, int I){
	int O=sqrtf(I);
	while(I%O)O--;
	*m=O;
	*n=I/O;
}

int nnWEIGHT(ParsedArguments *args){
	NN nn;
	nn_loadf(&nn, ParsedArguments_getFile(args, "nn"));
	for(int i=0;i<nn.layer_size;i++){
//		printf("layer %d-%d: %dx%d\n", i+1, i+2, nn.l[i].i, nn.l[i].o);
//		float MIN=999,MAX=0,AVG=0;
		int I=nn.l[i].i, O=nn.l[i].o;
		int m, n;
		decompose(&m, &n, I);
		assert(m*n==I);
		float* w=nn.l[i].w.m;
		for(int j=0; j<O; j++){
			printf("# [w%d-%d]\n", i, j);
			for(int k=0; k<n; k++){
				for(int l=0; l<m; l++){
					printf("%f ", w[j*I+k*m+l]);
				}
				printf("0\n");
			}
			for(int l=0; l<m; l++){
				printf("0 ");
			}
			printf("0\n\n\n");
/*			float v=fabsf(w[j]);
			if(v<MIN)MIN=v;
			if(v>MAX)MAX=v;
			AVG+=v;*/
		}
//		printf("weight:\n   min= %f\n   max= %f\n   avg= %f\n", MIN,MAX,AVG/n);
		
/*		MIN=999; MAX=0; AVG=0;
		n=nn.l[i].o;
		w=nn.l[i].b.v;
		for(int j=0;j<n;j++){
			printf("%f\n", w[j]);
			float v=fabsf(w[j]);
			if(v<MIN)MIN=v;
			if(v>MAX)MAX=v;
			AVG+=v;
		}*/
//		printf("bias:\n   min= %f\n   max= %f\n   avg= %f\n", MIN,MAX,AVG/n);
//		putchar('\n');
	}
	return 0;
}

int nnActivation(ParsedArguments *args){
	NN nn;
	char *path = ParsedArguments_getString(args, "nn");
	int layer = ParsedArguments_getInteger(args, "layer");
	char *activationName = ParsedArguments_getString(args, "activation");
	nn_loadp(&nn, path);
	if(layer<0 || layer>nn.layer_size){
		printf("invalid layer %d\n", layer);
		return 1;
	}
	activation act = {0, 0, -1, 0};
	for(int i=0; act_list[i].name; i++){
		if(!strcmp(act_list[i].name, activationName)){
			act = act_list[i];
			break;
		}
	}
	if(!act.name){
		printf("unknown activation function named \"%s\"\n", activationName);
		return 1;
	}
	nn.l[layer].act = act;
	nn_savep(&nn, path);
	return 0;
}