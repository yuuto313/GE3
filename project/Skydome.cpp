#include "Skydome.h"

void Skydome::Initialize(Object3d* object)
{
	this->object_ = object;
}

void Skydome::Update()
{
	object_->Update();
}

void Skydome::Draw()
{
	object_->Draw();
}
