#pragma once
#include "Object3d.h"

/// <summary>
/// 自機弾
/// </summary>
class PlayerBullet
{
public:// メンバ関数

	void Initialize(std::unique_ptr<Object3d> object,const Vector3& translate,const Vector3& velocity);

	void Update();

	void Draw();

	bool IsDead()const { return isDead_; }


private:// メンバ変数

	std::unique_ptr<Object3d> object_;

	Vector3 translate_ = {};

	// 速度
	Vector3 velocity_ = {};

	// 寿命<frm>
	static const int32_t kLifeTime = 60 * 3;
	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};

