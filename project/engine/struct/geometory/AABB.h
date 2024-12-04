#pragma once
#include "Vector.h"

//直方体
//Axis Aligned Bounding Box（軸平行境界箱）
struct AABB final{
	Vector3 min;//最小点
	Vector3 max;//最大点
};
