#pragma once
#include "GameMath.h"

/// <summary>
/// ワールド変換（KamataEngine参考）
/// </summary>
class Transform {
public:// メンバ関数

	Transform();
	~Transform() = default;

	void Initilaize();

	// アフィン変換
	void UpdateMatrix();

public:// メンバ変数
	// ローカルスケール
	Vector3 scale_ = {};
	// ローカル回転角
	Vector3 rotate_ = {};
	// ローカル座標
	Vector3 translate_ = {};
	// ローカルからワールド変換行列
	Matrix4x4 matWorld_ = {};
	// 親となるワールド変換へのポインタ
	const Transform* parent_ = nullptr;
};

