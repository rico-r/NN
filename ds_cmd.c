
#ifndef MAIN_C
#error "ds_cmd.c is not standalone"
#endif

/*
  $0 ds encode <Dataset> <AutoEncoder> <UsedLayer> <dstPath>
*/
int dsENCODE(int ac, char**args){
	needMin(4)
	int usedLayer;
	Dataset ds, result;
	NN nn;
	
	if(checkFile(args[0], 1))return 1;
	ds_loadp(&ds, args[0]);
	if(checkFile(args[1], 1))return 1;
	nn_loadp(&nn, args[1]);
	if(ds.in[0][0].size!=nn.i){
		printf("\e[33mDataset.input=%d, but AutoEncoder.input=%d\e[0m\n", ds.in[0][0].size, nn.i);
		return 0;
	}
	if(!sscanf(args[2], "%d", &usedLayer)){
		printf("\e[33minvaild int \"%s\"\e[0m\n", args[2]);
		return 0;
	}
	if(usedLayer<usedLayer){
		printf("\e[33mUsedLayer is negative\e[m\n");
		return 0;
	}
	if(usedLayer<=0){
		printf("\e[33m(UsedLayer=%d)<=0\e[m\n", usedLayer);
		return 0;
	}
	if(usedLayer>nn.layer_size){
		printf("\e[33m(UsedLayer=%d)>(AutoEncoder.LayerSize=%d)\e[m\n", usedLayer, nn.layer_size);
		return 0;
	}
	if(checkFile(args[3], 0))return 1;
	
	nn.layer_size=usedLayer;
	int out_size=nn.l[usedLayer-1].o;
	fprintf(stderr, "output: %d\n", out_size);
	//TODO: put back original nn.layer_size
	ds_init(&result, ds.count, ds.size);
	for(int i=0; i<ds.count; i++){
		vec* in=ds.in[i];
		vec* dst=result.in[i];
		int size=ds.size[i];
		for(int j=0; j<size; j++){
			vec_m_init(&dst[j], out_size);
			vec* out=nn_out(&nn, &in[j]);
			memcpy(dst[j].v, out->v, out_size*4);
		}
	}
	ds_savep(&result, args[3]);
	//TODO: release memory
	return 0;
}