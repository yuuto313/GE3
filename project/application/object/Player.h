#pragma once
#include "Transform.h"
#include "Object3d.h"
#include "PlayerBullet.h"

#include <memory>
#include <vector>
#include <list>
#include <chrono>

using namespace GameMath;

class Player
{
public:// メンバ関数

	void Initialize(std::vector<std::unique_ptr<Object3d>>& objects);

	void Update();

	void Draw();

	void ImGui();

	// 攻撃
	void Attack();
	// 右
	void MoveRight();
	// 左
	void MoveLeft();
	// 上
	void MoveUp();
	// 下
	void MoveDown();

	Vector3 GetWorldPosition();

	Vector3 GetReticleWorldPosition(const Transform& transform);

	const Vector3& GetTranslate() { return this->transform_.translate_; }

private:// メンバ変数

	std::vector<std::unique_ptr<Object3d>> objects_;
	std::list<std::unique_ptr<PlayerBullet>> bullets_;

	float speed_ = 0.3f;

	// 弾のクールタイム
	float bulletCoolTime_ = 0.2f;
	std::chrono::steady_clock::time_point lastAttackTime_;

	// 自機のワールド行列
	Transform transform_ = {};
	// 3dレティクル用ワールド行列
	Transform transformReticle_ = {};

private:// メンバ関数

	// 位置を制限する
	void ClampPosition();

	// 3Dレティクルの配置
	void UpdateReticle();
};

