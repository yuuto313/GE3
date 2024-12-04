#pragma once
#include "Vector.h"
#include "Matrix.h"
#include <cstdint>
#include <string>

// マテリアル関連の構造体やデータをまとめたヘッダー

/// <summary>
/// マテリアル(構造体)
/// </summary>
struct Material final{
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;

};



/// <summary>
/// マテリアルデータ(構造体)
/// </summary>
struct MaterialData final{
	std::string textureFilePath;
	// テクスチャ番号を保存する
	uint32_t textureIndex = 0;
};
