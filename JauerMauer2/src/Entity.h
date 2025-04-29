#pragma once
#include "../pch.h"
#include "MCH.h"

class Entity {
public:
	Entity() = default;
	Entity(jobject parentObject, jobject objectEntity) : parentObject(parentObject) {
		this->objectEntity = objectEntity;
		jcEntity = getObject("net.minecraft.entity.Entity");
		fdRotationYaw = env->GetFieldID(jcEntity, "field_70177_z", "F");
		fdRotationPitch = env->GetFieldID(jcEntity, "field_70125_A", "F");
		fdposX = env->GetFieldID(jcEntity, "field_70165_t", "D");
		fdposY = env->GetFieldID(jcEntity, "field_70163_u", "D");
		fdposZ = env->GetFieldID(jcEntity, "field_70161_v", "D");
		mdisSneaking = env->GetMethodID(jcEntity, "func_70093_af", "()Z");

	}

	float getRotationYaw() {
		return env->GetFloatField(objectEntity, fdRotationYaw);
	}
	void setRoationYaw(float value) {
		env->SetFloatField(objectEntity, fdRotationYaw, value);
	}

	float getRotationPitch() {
		return env->GetFloatField(objectEntity, fdRotationPitch);
	}
	void setRoationPitch(float value) {
		return env->SetFloatField(objectEntity, fdRotationPitch, value);
	}
	double getPosX() {
		return env->GetDoubleField(objectEntity, fdposX);
	}

	void setPosX(double value) {
		env->SetDoubleField(objectEntity, fdposX, value);
	}
	double getPosY() {
		return env->GetDoubleField(objectEntity, fdposY);
	}
	void setPosY(double value) {
		env->SetDoubleField(objectEntity, fdposY, value);
	}
	double getPosZ() {
		return env->GetDoubleField(objectEntity, fdposZ);
	}
	void setPosZ(double value) {
		env->SetDoubleField(objectEntity, fdposZ, value);
	}
	bool isSneaking() {
		return env->CallBooleanMethod(objectEntity, mdisSneaking);
	}

protected:

	jobject parentObject;
	jobject objectEntity;

	jclass jcEntity;
	//fields
	jfieldID fdRotationYaw;
	jfieldID fdRotationPitch;
	jfieldID fdposX;
	jfieldID fdposY;
	jfieldID fdposZ;

	jmethodID mdisSneaking;
};
