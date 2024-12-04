#pragma once
#include "MyMatrix4x4.h"
#include "MyVector4.h"

struct  ParticleForGPU final
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};
