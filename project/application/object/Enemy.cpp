#include "Enemy.h"

void (Enemy::* Enemy::pFunc[])() = {
	&Enemy::ApproachUpdate,
	&Enemy::LeaveUpdate,
};

void Enemy::Initialize(const std::vector<Object3d*>& objects)
{

	this->pObjects_ = objects;

	transform_.Initilaize();
	transform_ = pObjects_[0]->GetTransform();
	transform_.translate_ = { 3.0f,3.0f,30.0f };

	approachVelocity_ = { 0.0f,0.0f,-0.05f };
	leaveVelocity_ = { 0.03f,0.03f,0.03f };

}

void Enemy::Update()
{
	// デスフラグの立った弾を削除
	bullets_.remove_if([](const std::unique_ptr<EnemyBullet>& bullet) {return bullet->IsDead(); });

	// 行動処理
	(this->*pFunc[static_cast<size_t>(phase_)])();

	// 攻撃
	Fire();

	for (const auto& bullet : bullets_) {
		bullet->Update();
	}

	transform_.UpdateMatrix();

	pObjects_[0]->SetTransform(transform_);
	pObjects_[0]->Update();
}

void Enemy::Draw()
{
	pObjects_[0]->Draw();

	// 弾の描画
	for (const auto& bullet : bullets_) {
		bullet->Draw();
	}

}

void Enemy::Fire()
{
	std::unique_ptr<Object3d> object = std::make_unique<Object3d>();
	object->Initialize(pObjects_[1]->GetCamera(), pObjects_[1]->GetModel());

	std::unique_ptr<EnemyBullet> bullet = std::make_unique<EnemyBullet>();
	bullet->Initialize(std::move(object), transform_.translate_);
	bullets_.push_back(std::move(bullet));

}

void Enemy::ApproachUpdate()
{
	// 移動
	transform_.translate_ += approachVelocity_;

	if (transform_.translate_.z < 10.0f) {
		phase_ = Phase::Leave;
	}
}

void Enemy::LeaveUpdate()
{
	// 移動
	transform_.translate_ += leaveVelocity_;
}
