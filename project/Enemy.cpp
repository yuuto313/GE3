#include "Enemy.h"

void Enemy::Initialize(Object3d* object)
{

	this->object_ = object;

	transform_.Initilaize();
	transform_ = object_->GetTransform();
	transform_.translate_ = { 3.0f,3.0f,30.0f };

	velocity_ = { 0.0f,0.0f,-0.05f };

}

void Enemy::Update()
{
	transform_.translate_ += velocity_;

	transform_.UpdateMatrix();

	object_->SetTransform(transform_);
	object_->Update();
}

void Enemy::Draw()
{
	object_->Draw();
}
