
#ifndef MAIN_C
#error "ae_cmd.c is not standalone"
#endif

#ifndef _WIN32
void toggleCanon(){
	struct termios term;
	tcgetattr(0, &term);
	term.c_lflag^=ICANON|ECHO;
	// tcsetattr(0, TCSANOW, &term);
}
#endif

int aeTRAIN(ParsedArguments *args){
	signal(SIGINT, mhandler);

	Dataset ds;
	NN nn;
	LossFunction lossFunction=squaredLossFunction;
	register int group = !ParsedArguments_getBoolean(args, "no-group");
	int c = ParsedArguments_getInteger(args, "cycle");
	int sc = ParsedArguments_getInteger(args, "subcycle");
	float lr = ParsedArguments_getFloat(args, "learnrate");
	char *sv = ParsedArguments_getString(args, "output");
	char *nnfile = ParsedArguments_getString(args, "nn");
	ds_loadf(&ds, ParsedArguments_getFile(args, "ds"));
	nn_loadp(&nn, nnfile);
	if(sv==0){
		sv = nnfile;
	}

	if(nn.i!=nn.o){
		fprintf(stderr, "\e[33m\"%s\" is not AutoEncoder, input=%d, but output=%d\e[0m\n", nnfile, nn.i, nn.o);
		return 0;
	}
	if(ds.in[0][0].size!=nn.i){
		fprintf(stderr, "\e[33mDataset.input=%d, but AutoEncoder.input=%d\e[0m\n", ds.in[0][0].size, nn.i);
		return 0;
	}
	if(!group)ds_collect(&ds);
	LearnRate=lr/sc;
	WeightRate=0.01f;
	
	register int i,j,k,l;
	register float err;
	
	for(i=1; i<=c&run; i++){
		err=0;
		for(j=0; j<sc; j++){
			if(group){
				k=rand()%ds.count;
				l=rand()%ds.size[k];
				nn_out(&nn, &ds.in[k][l]);
				err+=nn_train(&nn, &ds.in[k][l], lossFunction);
			}else{
				k=rand()%ds.all_size;
				nn_out(&nn, ds.all_in[k]);
				err+=nn_train(&nn, ds.all_in[k], lossFunction);
			}
		}
		nn_update(&nn);
		err/=j;
		printf("%5d %.5f\n", i, err);
	}
	nn_savep(&nn, sv);
	return 0;
}

void printd2(const float *data1, const float *data2, int w, int h){
	int m=0, n=0;
	char* str;
	int v;
	for(int i=0; i<h; i++){
		for(int j=0; j<w; j++){
			float d=data1[m]-data2[m];
			v=24*fabsf(d);
			asprintf(&str, "\e[48;5;%dm  ", 232+(v<0?0:(v>23?23:v)));
			fprintf(stdout, str);
			m++;
		}
		fprintf(stdout, "\e[m|");
		for(int j=0; j<w; j++){
			v=24*data2[n];
			asprintf(&str, "\e[48;5;%dm  ", 232+(v<0?0:(v>23?23:v)));
			fprintf(stdout, str);
			n++;
		}
		fprintf(stdout, "\e[m|\n");
	}
}

#ifndef _WIN32
#define min(x,y) ((x)<(y)?(x):(y))
int aeVIEW(int ac, char**args){
	needMin(2)
	Dataset ds;
	NN nn;
	LossFunction lossFunction=squaredLossFunction;
	if(checkFile(args[0], 1))return 1;
	if(checkFile(args[1], 1))return 1;
	ds_loadp(&ds, args[0]);
	nn_loadp(&nn, args[1]);
	signal(SIGINT, mhandler);
//	signal(SIGWINCH, mhandler);

	toggleCanon();
	run=1;
	int n=0, i=0;
	while(run){
		int c=0;
		ioctl(0, FIONREAD, &c);
		if(c==1);
		else if(c==3){
			if(getchar()!='\e')continue;
			if(getchar()!='[')continue;
			switch(getchar()){
				case 0x41: i=0; n=n==0?0:n-1; break;
				case 0x42: i=0; n=min(n+1, ds.count-1); break;
				case 0x43: i=min(i+1, ds.size[n]-1); break;
				case 0x44: i=i==0?0:i-1; break;
			}
			vec* out=nn_out(&nn, &ds.in[n][i]);
			printf("\e[H\e[J");
			printd2(ds.in[n][i].v, out->v, 20, 20);
			printf("%d/%d %d/%d | %f\n", n+1, ds.count, i+1, ds.size[n]+1, nn_err(&nn, &ds.in[n][i], lossFunction));
		}else{
			for(int i=0;i<c;i++)getchar();
		}
	}
	toggleCanon();
	return 0;
}
#endif

int aeSTAT(ParsedArguments *args){
	Dataset ds;
	NN nn;
	LossFunction lossFunction = squaredLossFunction;
	ds_loadf(&ds, ParsedArguments_getFile(args, "ds"));
	nn_loadf(&nn, ParsedArguments_getFile(args, "nn"));

	ds_collect(&ds);
	float total=0;
	float err[ds.all_size];
	float minv=(float)nn.o, maxv=0;
	for(int i=0; i<ds.all_size; i++){
		nn_out(&nn, ds.all_in[i]);
		float e=err[i]=nn_err(&nn, ds.all_in[i], lossFunction);
		if(e>maxv)maxv=e;
		if(e<minv)minv=e;
		total+=e;
	}
	float avg=total/ds.all_size;
	float dev_total=0;
	for(int i=0; i<ds.all_size; i++){
		dev_total+=fabsf(err[i]-avg);
	}
	printf("sum : %f\navg : %f\n", total, avg);
	printf("min : %f\nmax : %f\n", minv, maxv);
	printf("mean: %f\n", (maxv-minv)/2);
	printf("dev : %f\n", dev_total/ds.all_size);
	return 0;
}