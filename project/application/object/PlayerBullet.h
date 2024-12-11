#pragma once
#include "Object3d.h"
#include "Transform.h"

/// <summary>
/// 自機弾
/// </summary>
class PlayerBullet
{
public:// メンバ関数

	void Initialize(Object3d* object,const Vector3& translate);

	void Update();

	void Draw();

	bool IsDead()const { return isDead_; }


private:// メンバ変数

	Object3d* pObject_ = nullptr;
	Transform transform_ = {};

	// 速度
	Vector3 velocity_ = {};

	// 寿命<frm>
	static const int32_t kLifeTime = 60 * 3;
	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};

