#include "Player.h"
#include "WinApp.h"
#include "ImGuiManager.h"
#include <assert.h>

void Player::Initialize(std::vector<std::unique_ptr<Object3d>>& objects)
{
	this->objects_ = std::move(objects);

	transform_.Initilaize();
	transform_ = objects_[0]->GetTransform();
	// モデルのスケールを設定
	transform_.scale_ = { 0.7f,0.7f,0.7f };
}

void Player::Update()
{
	// デスフラグの立った弾を削除
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {return bullet->IsDead(); });

	// 弾の更新処理
	for (const auto& bullet : bullets_) {
 		bullet->Update();		
	}

	// 画面内制限
	ClampPosition();
	
	objects_[0]->SetTransform(transform_);
	objects_[0]->Update();
}

void Player::Draw()
{
	// 弾の描画処理
	for (const auto& bullet : bullets_) {
		bullet->Draw();
	}

	// モデル描画
	objects_[0]->Draw();
}

void Player::ImGui()
{
	ImGui::Begin("Player");
	ImGui::SliderFloat3("translate", &transform_.translate_.x, -10.0f, 10.0f);
	ImGui::Text("bullets_count : %d", bullets_.size());
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

void Player::Attack()
{
	std::unique_ptr<Object3d> object = std::make_unique<Object3d>();
	object->Initialize(objects_[1]->GetCamera(), objects_[1]->GetModel());

	// 弾を生成
	std::unique_ptr<PlayerBullet> bullet = std::make_unique<PlayerBullet>();
	bullet->Initialize(std::move(object), this->transform_.translate_);
	bullets_.push_back(std::move(bullet));
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
