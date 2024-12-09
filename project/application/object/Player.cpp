#include "Player.h"
#include "WinApp.h"
#include "ImGuiManager.h"

void Player::Initialize(Object3d* object)
{
	this->object_ = object;

	transform_.Initilaize();
	transform_ = this->object_->GetTransform();

	// モデルのスケールを設定
	transform_.scale_ = { 0.7f,0.7f,0.7f };
	this->object_->SetScale(transform_.scale_);
}

void Player::Update()
{
	// 画面内制限
	ClampPosition();

	// モデル更新
	object_->SetTranslate(transform_.translate_);
	object_->Update();
}

void Player::Draw()
{
	// モデル描画
	object_->Draw();
}

void Player::ImGui()
{
	ImGui::Begin("Player");
	ImGui::SliderFloat3("translate", &transform_.translate_.x, -10.0f, 10.0f);
	ImGui::End();
}

void Player::ClampPosition()
{
	// 右側の制御
	if (transform_.translate_.x >= 6.2f) {
		transform_.translate_.x -= speed_;
	}

	// 左側
	if (transform_.translate_.x <= -6.2f) {
		transform_.translate_.x += speed_;
	}

	// 上側
	if (transform_.translate_.y >= 4.2f) {
		transform_.translate_.y -= speed_;
	}

	// 下側
	if (transform_.translate_.y <= -2.5f) {
		transform_.translate_.y += speed_;
	}

}

void Player::MoveRight()
{
	this->transform_.translate_.x += this->speed_;
}

void Player::MoveLeft()
{
	this->transform_.translate_.x -= this->speed_;
}

void Player::MoveUp()
{
	this->transform_.translate_.y += this->speed_;
}

void Player::MoveDown()
{
	this->transform_.translate_.y -= this->speed_;
}
