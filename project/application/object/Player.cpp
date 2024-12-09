#include "Player.h"

void Player::Initialize()
{
	position_ = object_->GetTranslate();
}

void Player::Update()
{
	object_->SetTranslate(position_);
	object_->Update();
}

void Player::Draw()
{
	object_->Draw();
}

void Player::MoveRight()
{
	this->position_.x += this->speed_;
}

void Player::MoveLeft()
{
	this->position_.x -= this->speed_;
}

void Player::MoveForward()
{
	this->position_.z += this->speed_;
}

void Player::MoveBack()
{
	this->position_.z -= this->speed_;
}
