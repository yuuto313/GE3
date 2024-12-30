#pragma once
#include "Object3d.h"
#include "Transform.h"
#include "EnemyBullet.h"

#include <list>
#include <vector>
#include <chrono>

class Player;

class Enemy
{
public:// メンバ関数

	// 行動フェーズ
	enum class Phase {
		Approach,	// 接近フェーズ
		Leave,		// 離脱する
	};

	void Initialize(const std::vector<Object3d*>& objects);

	void Update();

	void Draw();

	void Fire();

	void ApproachUpdate();

	void LeaveUpdate();

	Vector3 GetWorldPosition();

	void SetPlayer(Player* player) { player_ = player; }

private:// メンバ変数

	std::vector<Object3d*> pObjects_;
	std::list<std::unique_ptr<EnemyBullet>> bullets_;
	Player* player_ = nullptr;

	Transform transform_;

	// 接近速度
	Vector3 approachVelocity_;
	// 離脱速度
	Vector3 leaveVelocity_;

	// フェーズ
	Phase phase_ = Phase::Approach;

	// メンバ関数ポインタ
	static void (Enemy::* pFunc[])();

	// 弾のクールタイム
	float bulletCoolTime_ = 0.5f;
	std::chrono::steady_clock::time_point lastAttackTime_;

};

