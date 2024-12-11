#include "PlayerBullet.h"
#include <assert.h>

void PlayerBullet::Initialize(Object3d* object,const Vector3& translate)
{
	this->pObject_ = object;

	transform_.Initilaize();
	transform_.translate_ = translate;
	

	velocity_ = { 0.0f,0.0,0.5f };
}

void PlayerBullet::Update()
{
	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	transform_.translate_ += velocity_;
	transform_.UpdateMatrix();
}

void PlayerBullet::Draw()
{
	if (!isDead_) {
		pObject_->Draw(transform_);
	}
}
