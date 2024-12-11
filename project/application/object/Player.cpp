#include "Player.h"
#include "WinApp.h"
#include "ImGuiManager.h"
#include <assert.h>

void Player::Initialize(const std::vector<Object3d*>& objects)
{
	this->objects_ = objects;

	transform_.Initilaize();

	// モデルのスケールを設定
	transform_.scale_ = { 0.7f,0.7f,0.7f };
}

void Player::Update()
{
	// デスフラグの立った弾を削除
	//bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {return bullet->IsDead(); });

	// 弾の更新処理
	for (const auto& bullet : bullets_) {
 		bullet->Update();		
	}

	// 画面内制限
	ClampPosition();

	transform_.UpdateMatrix();
}

void Player::Draw()
{
	// 弾の描画処理
	for (const auto& bullet : bullets_) {
		bullet->Draw();
	}

	// モデル描画
	objects_[0]->Draw(transform_);
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
	// 弾を生成
	std::unique_ptr<PlayerBullet> bullet = std::make_unique<PlayerBullet>();
	bullet->Initialize(objects_[1], this->transform_.translate_);
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
