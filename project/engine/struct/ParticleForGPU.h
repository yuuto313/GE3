#pragma once
#include "Matrix.h"
#include "Vector.h"

struct  ParticleForGPU final
{
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};
