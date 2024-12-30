#pragma once
#include "Object3d.h"

#include <memory>
class EnemyBullet
{
public:

	void Initialize(std::unique_ptr<Object3d> object,const Vector3& translation,const Vector3& velocity);

	void Update();

	void Draw();

	bool IsDead()const { return isDead_; }

private:
	std::unique_ptr<Object3d> object_;

	Vector3 translation_;
	Vector3 velocity_;

	// 寿命<frm>
	static const int32_t kLifeTime = 60 * 3;
	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};

