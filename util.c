
#ifdef JAVA

#include "util.h"

void log(char*c){
	FILE*f=fopen("/sdcard/log","wb+");
	fwrite(c,strlen(c),1,f);
	fclose(f);
}
void log(int v){
	char*c=new char[v];
	sprintf(c,"%d\n",v);
	log(c);
}

int get(JNIEnv*env,jobject thiz){
	return env->GetIntField(thiz,env->GetFieldID(env->GetObjectClass(thiz),"_m","I"));
}
FILE*open(JNIEnv*env,jstring path,char*mode){
	int sz=env->GetStringUTFLength(path);
	char* p=(char*)calloc(sz+1,1);
	env->GetStringUTFRegion(path,0,sz,p);
	FILE*f=fopen(p,mode);
	free(p);
	return f;
}
#endif
