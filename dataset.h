
#include "vec.h"

struct dataset_struct{
	int count;
	int*size;
	vec** in;
	vec** out;
	int all_size;
	vec** all_in;
	vec** all_out;
};
typedef struct dataset_struct Dataset;

void ds_init(Dataset*ds, int group, const int* each);
void ds_collect(Dataset* ds);
void ds_recollect(Dataset* ds);
void ds_loadf(Dataset*ds, FILE*f);
void ds_loadp(Dataset*ds, const char*path);
void ds_savef(Dataset*ds, FILE*f);
void ds_savep(Dataset*ds, const char*path);