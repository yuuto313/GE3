#include "EnemyBullet.h"

void EnemyBullet::Initialize(std::unique_ptr<Object3d> object,const Vector3& translation, const Vector3& velocity)
{
	this->object_ = std::move(object);
	this->translation_ = translation;
	this->velocity_ = velocity;
}

void EnemyBullet::Update()
{
	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	translation_ += velocity_;

	object_->SetTranslate(translation_);
	object_->Update();

}

void EnemyBullet::Draw()
{
	object_->Draw();
}
