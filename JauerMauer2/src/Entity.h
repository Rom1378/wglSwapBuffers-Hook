#pragma once
#include "../pch.h"
#include "MCH.h"

class Entity {
public:
	Entity() = default;

    Entity(jobject parentObject, jobject objectEntity) : parentObject(parentObject) {
        if (env == nullptr) return;

        // Create a global reference to the entity
        if (objectEntity != nullptr) {
            this->objectEntity = env->NewGlobalRef(objectEntity);

            // Find and cache entity class and method IDs
            jclass localEntityClass = MCH::getObject("net.minecraft.entity.Entity");
            if (localEntityClass != nullptr) {
                jcEntity = (jclass)env->NewGlobalRef(localEntityClass);
                env->DeleteLocalRef(localEntityClass);

                // Get field and method IDs
                fdRotationYaw = env->GetFieldID(jcEntity, "field_70177_z", "F");
                fdRotationPitch = env->GetFieldID(jcEntity, "field_70125_A", "F");
                fdposX = env->GetFieldID(jcEntity, "field_70165_t", "D");
                fdposY = env->GetFieldID(jcEntity, "field_70163_u", "D");
                fdposZ = env->GetFieldID(jcEntity, "field_70161_v", "D");
                mdisSneaking = env->GetMethodID(jcEntity, "func_70093_af", "()Z");
            }
        }
    }

    // Copy constructor
    Entity(const Entity& other) : parentObject(other.parentObject), jcEntity(other.jcEntity),
        fdRotationYaw(other.fdRotationYaw), fdRotationPitch(other.fdRotationPitch),
        fdposX(other.fdposX), fdposY(other.fdposY), fdposZ(other.fdposZ),
        mdisSneaking(other.mdisSneaking) {
        // Create a new global reference for the copied object
        if (other.objectEntity != nullptr && env != nullptr) {
            this->objectEntity = env->NewGlobalRef(other.objectEntity);
        }
        else {
            this->objectEntity = nullptr;
        }
    }

    // Move constructor
    Entity(Entity&& other) noexcept : parentObject(other.parentObject), objectEntity(other.objectEntity),
        jcEntity(other.jcEntity), fdRotationYaw(other.fdRotationYaw),
        fdRotationPitch(other.fdRotationPitch), fdposX(other.fdposX),
        fdposY(other.fdposY), fdposZ(other.fdposZ),
        mdisSneaking(other.mdisSneaking) {
        // Take ownership of the global reference
        other.objectEntity = nullptr;
    }

    // Assignment operator
    Entity& operator=(const Entity& other) {
        if (this != &other) {
            // Clean up existing reference
            if (objectEntity != nullptr && env != nullptr) {
                env->DeleteGlobalRef(objectEntity);
                objectEntity = nullptr;
            }

            // Copy fields
            parentObject = other.parentObject;
            jcEntity = other.jcEntity;
            fdRotationYaw = other.fdRotationYaw;
            fdRotationPitch = other.fdRotationPitch;
            fdposX = other.fdposX;
            fdposY = other.fdposY;
            fdposZ = other.fdposZ;
            mdisSneaking = other.mdisSneaking;

            // Create new global reference
            if (other.objectEntity != nullptr && env != nullptr) {
                objectEntity = env->NewGlobalRef(other.objectEntity);
            }
        }
        return *this;
    }

    // Move assignment operator
    Entity& operator=(Entity&& other) noexcept {
        if (this != &other) {
            // Clean up existing reference
            if (objectEntity != nullptr && env != nullptr) {
                env->DeleteGlobalRef(objectEntity);
            }

            // Move fields
            parentObject = other.parentObject;
            objectEntity = other.objectEntity;
            jcEntity = other.jcEntity;
            fdRotationYaw = other.fdRotationYaw;
            fdRotationPitch = other.fdRotationPitch;
            fdposX = other.fdposX;
            fdposY = other.fdposY;
            fdposZ = other.fdposZ;
            mdisSneaking = other.mdisSneaking;

            // Clear source object without deleting reference
            other.objectEntity = nullptr;
        }
        return *this;
    }

    ~Entity() {
        // Delete global reference
        if (objectEntity != nullptr && env != nullptr) {
            env->DeleteGlobalRef(objectEntity);
            objectEntity = nullptr;
        }
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
