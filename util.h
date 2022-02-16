
#include <stdio.h>
#include <jni.h>
#include <string.h>
#include <malloc.h>

void log(char*c);
void log(int v);
int get(JNIEnv*env,jobject thiz);
FILE*open(JNIEnv*env,jstring path,char*mode);
