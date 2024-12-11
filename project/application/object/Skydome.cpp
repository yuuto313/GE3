#include "Skydome.h"

void Skydome::Initialize(Object3d* object)
{
	this->object_ = object;
	transform_.Initilaize();
}

void Skydome::Update()
{
	transform_.UpdateMatrix();
}

void Skydome::Draw()
{
	object_->Draw(transform_);
}
