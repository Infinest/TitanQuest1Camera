#pragma once
#include "includes.h"

class Vector3d
{
public:
	float x;
	float y;
	float z;

	Vector3d(float x = 0, float y = 0, float z = 0);

	void setVectorYaw(float angleRad);
	void setVectorPitch(float angleRad);
	void normalize();
	void scale(float speed);
};