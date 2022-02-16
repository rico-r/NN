
#include "dataset.h"
#include <malloc.h>

void ds_init(Dataset*ds, int group, const int* each){
	ds->count=group;
	int*all_size=malloc(group*sizeof(int));
	vec**in=(vec**)malloc(group*sizeof(vec*));
	vec**out=(vec**)malloc(group*sizeof(vec*));
	
	for(register int i=0;i<group;i++){
		int size=each[i];
		all_size[i]=size;
		in[i]=(vec*)malloc(size*sizeof(vec));
		out[i]=(vec*)malloc(size*sizeof(vec));
		for(register int j=0;j<size;j++){
			vec_m_init(&out[i][j], group);
			for(register int k=0;k<group;k++)
				out[i][j].v[k]=i==k?0.9f:0.1f;
		}
	}
	
	ds->size=all_size;
	ds->in=in;
	ds->out=out;
}
void ds_collect(Dataset* ds){
	int all_size=0;
	ds->count--;
	for(int i=0;i<ds->count;i++){
		all_size+=ds->size[i];
	}
	ds->all_size=all_size;
	ds->all_in=(vec**)malloc(all_size*sizeof(vec*));
	ds->all_out=(vec**)malloc(all_size*sizeof(vec*));
	int n=0;
	for(int i=0; i<ds->count; i++){
		for(int j=0; j<ds->size[i]; j++){
			ds->all_in[n]=&ds->in[i][j];
			ds->all_out[n]=&ds->out[i][j];
			n++;
		}
	}
}
void ds_recollect(Dataset* ds){
	int all_size=0;
	for(int i=0;i<ds->count;i++){
		all_size+=ds->size[i];
	}
	ds->all_size=all_size;
	ds->all_in=(vec**)realloc(ds->all_in, all_size*sizeof(vec*));
	ds->all_out=(vec**)realloc(ds->all_out, all_size*sizeof(vec*));
	int n=0;
	for(int i=0; i<ds->count; i++){
		for(int j=0; j<ds->size[i]; j++){
			ds->all_in[n]=&ds->in[i][j];
			ds->all_out[n]=&ds->out[i][j];
			n++;
		}
	}
}
void ds_loadf(Dataset*ds, FILE*f){
	int c;
	fread(&c, 4, 1, f);
	ds->count = c;
//	ds->in=new std::vector<vec*>[c];
	int *all_size = malloc(c*sizeof(int));
	vec **in = (vec**) malloc(c*sizeof(vec*));
	vec **out = (vec**) malloc(c*sizeof(vec*));
	
//	int total=
	for(register int i=0;i<c;i++){
		int size;
		fread(&size, 4, 1, f);
		all_size[i] = size;
		in[i]=(vec*)malloc(size*sizeof(vec));
		out[i]=(vec*)malloc(size*sizeof(vec));
		for(register int j=0;j<size;j++){
//			vec* in=(vec*)malloc(sizeof(vec));
			vec_load(&in[i][j], f);
			vec_m_init(&out[i][j],c);
			for(register int k=0; k<c; k++)
				out[i][j].v[k] = i == k ? 1 : 0;
//			total++;
		}
	}
	
	ds->size=all_size;
	ds->in=in;
	ds->out=out;
	ds->count=c;
}
void ds_savef(Dataset*ds, FILE*f){
	fwrite(&ds->count, 4, 1, f);
	
	for(register int i=0;i<ds->count;i++){
		int size=ds->size[i];
		fwrite(&size, 4, 1, f);
		for(register int j=0;j<size;j++){
			vec_save(&ds->in[i][j], f);
		}
	}
}

void ds_loadp(Dataset*ds, const char*path){
	FILE*f=fopen(path,"rb");
	ds_loadf(ds, f);
	fclose(f);
}

void ds_savep(Dataset*ds, const char*path){
	FILE*f=fopen(path,"wb");
	ds_savef(ds, f);
	fclose(f);
}