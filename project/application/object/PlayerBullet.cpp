#include "PlayerBullet.h"
#include <assert.h>

void PlayerBullet::Initialize(std::unique_ptr<Object3d> object,const Vector3& translate,const Vector3& velocity)
{
	this->object_ = std::move(object);
	
	translate_ = translate;

	velocity_ = velocity;
}

void PlayerBullet::Update()
{
	// 時間経過でデス
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}

	translate_ += velocity_;

	object_->SetTranslate(translate_);
	object_->Update();
}

void PlayerBullet::Draw()
{
	object_->Draw();
}
