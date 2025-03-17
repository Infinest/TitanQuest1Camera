#include "Vector3d.h"


Vector3d::Vector3d(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Vector3d::setVectorYaw(float angleRad)
{
	float originalX = this->x;
	float originalY = this->y;
	this->x = originalX * cos(angleRad) - originalY * sin(angleRad);
	this->y = originalX * sin(angleRad) + originalY * cos(angleRad);
}

void Vector3d::setVectorPitch(float angleRad)
{
	float originalY = this->y;
	float originalZ = this->z;
	this->y = originalY * cos(angleRad) - originalZ * sin(angleRad);
	this->z = originalY * sin(angleRad) + originalZ * cos(angleRad);
}

void Vector3d::normalize() {
    float magnitude = std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
    if (magnitude > 0) { // Avoid division by zero
        this->x /= magnitude;
        this->y /= magnitude;
        this->z /= magnitude;
    }
}

void Vector3d::scale(float speed) {
	x *= speed;
	y *= speed;
	z *= speed;
}