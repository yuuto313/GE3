#pragma once
#include "Vector.h"

//平行光源
struct DirectionalLight final{
	Vector4 color;//ライトの色
	Vector3 direction;//ライトの向き
	float intensity;//輝度
};

