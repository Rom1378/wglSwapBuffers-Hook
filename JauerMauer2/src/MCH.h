#include "../pch.h"
#include <gl/GL.h>
#include <gl/GLU.h>

namespace MCH {

	static jobject getClassLoader() {
		jclass launch = env->FindClass("net/minecraft/launchwrapper/Launch");
		jfieldID sfid = env->GetStaticFieldID(launch, "classLoader", "Lnet/minecraft/launchwrapper/LaunchClassLoader;");
		jobject classLoader = env->GetStaticObjectField(launch, sfid);
		return classLoader;
	}

	static jclass getObject(const char* className) {
		jstring name = env->NewStringUTF(className);
		jobject classLoader = getClassLoader();
		jmethodID mid = env->GetMethodID(env->GetObjectClass(classLoader), "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
		return (jclass)env->CallObjectMethod(classLoader, mid, name);
		env->DeleteLocalRef(name);
	}
	static jobject getMinecraft() {
		jclass mcClass = getObject("net.minecraft.client.Minecraft");
		jmethodID smid = env->GetStaticMethodID(mcClass, "func_71410_x", "()Lnet/minecraft/client/Minecraft;");
		return env->CallStaticObjectMethod(mcClass, smid);
	}

	static jobject getThePlayer() {
		jfieldID fdThePlayer = env->GetFieldID(env->GetObjectClass(getMinecraft()), "field_71439_g", "Lnet/minecraft/client/entity/EntityClientPlayerMP;");
		return env->GetObjectField(getMinecraft(), fdThePlayer);
	}
	static jobject getTheWorld() {
		jfieldID fdTheWorld = env->GetFieldID(env->GetObjectClass(getMinecraft()), "field_71441_e", "Lnet/minecraft/client/multiplayer/WorldClient;");
		return env->GetObjectField(getMinecraft(), fdTheWorld);
	}


	static std::vector<int> Get_VIEWPORT()
	{
		std::vector<int> VIEWPORT;
		jclass CActiveRenderInfo = getObject("net.minecraft.client.renderer.ActiveRenderInfo");
		jfieldID FIDVIEWPORT = env->GetStaticFieldID(CActiveRenderInfo, "field_74597_i", "Ljava/nio/IntBuffer;");
		jobject OVIEWPORT = env->GetStaticObjectField(CActiveRenderInfo, FIDVIEWPORT);
		env->DeleteLocalRef(CActiveRenderInfo);
		jclass CIntBuffer = env->FindClass("java/nio/IntBuffer");
		jmethodID MIDget = env->GetMethodID(CIntBuffer, "get", "(I)I");
		for (int i = 0; i < 16; i++) {
			VIEWPORT.push_back(env->CallIntMethod(OVIEWPORT, MIDget, i));
		}
		env->DeleteLocalRef(OVIEWPORT);
		env->DeleteLocalRef(CIntBuffer);
		return VIEWPORT;
	}
	static std::vector<float> Get_MODELVIEW()
	{
		std::vector<float> MODELVIEW;
		MODELVIEW.clear();
		jclass CActiveRenderInfo = getObject("net.minecraft.client.renderer.ActiveRenderInfo");
		jfieldID FIDMODELVIEW = env->GetStaticFieldID(CActiveRenderInfo, "field_74594_j", "Ljava/nio/FloatBuffer;");
		jobject OMODELVIEW = env->GetStaticObjectField(CActiveRenderInfo, FIDMODELVIEW);
		env->DeleteLocalRef(CActiveRenderInfo);
		jclass CFloatBuffer = env->FindClass("java/nio/FloatBuffer");
		jmethodID MIDget = env->GetMethodID(CFloatBuffer, "get", "(I)F");
		for (int i = 0; i < 16; i++)
		{
			MODELVIEW.push_back(env->CallFloatMethod(OMODELVIEW, MIDget, i));
		}
		env->DeleteLocalRef(OMODELVIEW);
		env->DeleteLocalRef(CFloatBuffer);
		return MODELVIEW;
	}
	static std::vector<float> Get_PROJECTION()
	{
		std::vector<float> PROJECTION;
		PROJECTION.clear();
		jclass CActiveRenderInfo = getObject("net.minecraft.client.renderer.ActiveRenderInfo");
		jfieldID FIDPROJECTION = env->GetStaticFieldID(CActiveRenderInfo, "field_74595_k", "Ljava/nio/FloatBuffer;");
		jobject OPROJECTION = env->GetStaticObjectField(CActiveRenderInfo, FIDPROJECTION);
		env->DeleteLocalRef(CActiveRenderInfo);
		jclass CFloatBuffer = env->FindClass("java/nio/FloatBuffer");
		jmethodID MIDget = env->GetMethodID(CFloatBuffer, "get", "(I)F");
		for (int i = 0; i < 16; i++)
		{
			PROJECTION.push_back(env->CallFloatMethod(OPROJECTION, MIDget, i));
		}
		env->DeleteLocalRef(OPROJECTION);
		env->DeleteLocalRef(CFloatBuffer);
		return PROJECTION;
	}
}
