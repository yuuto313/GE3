#include "EnemyBullet.h"

void EnemyBullet::Initialize(std::unique_ptr<Object3d> object,const Vector3& translation)
{
	this->object_ = std::move(object);
	this->translation_ = translation;

	velocity_ = { 0.0f,0.0f,-0.5f };

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
