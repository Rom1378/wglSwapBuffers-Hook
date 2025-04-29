#pragma once
#include "../pch.h"


class JavaList
{
public:

	JavaList(jobject parentObject, const char* fieldName, const char* sig) : parentObject(parentObject) {
		jcList = env->FindClass("java/util/List");
		jmSize = env->GetMethodID(jcList, "size", "()I");
		jmGet = env->GetMethodID(jcList, "get", "(I)Ljava/lang/Object;");
		field = env->GetFieldID(env->GetObjectClass(parentObject), fieldName, sig);
		oList = env->GetObjectField(parentObject, field);
	}

	int getSize() {
		return env->CallIntMethod(oList, jmSize);
	}

	jobject get(int index) {
		return env->CallObjectMethod(oList, jmGet, index);
	}

private:

	jobject parentObject;

	jobject oList;
	jclass jcList;
	jmethodID jmSize;
	jmethodID jmGet;
	jfieldID field;

};
