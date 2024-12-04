#pragma once
#include "Vector.h"
#include "AABB.h"

// 場
struct AccelerationField {
	Vector3 acceleration;	// 加速度
	AABB area;				// 範囲
};

