#include "Player.h"
#include "WinApp.h"
#include "ImGuiManager.h"
#include <assert.h>

void Player::Initialize(std::vector<std::unique_ptr<Object3d>>& objects)
{
	this->objects_ = std::move(objects);
	// 本体
	transform_.Initilaize();
	transform_ = objects_[0]->GetTransform();
	// スケールを設定
	transform_.scale_ = { 0.7f,0.7f,0.7f };

	// レティクル
	transformReticle_.Initilaize();
	transformReticle_ = objects_[2]->GetTransform();
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

	// 3Dレティクルの配置
	UpdateReticle();
	
	transform_.UpdateMatrix();
	objects_[0]->SetTransform(transform_);
	objects_[0]->Update();

	transformReticle_.UpdateMatrix();
	objects_[2]->SetTransform(transformReticle_);
	objects_[2]->Update();
}

void Player::Draw()
{
	// 弾の描画処理
	for (const auto& bullet : bullets_) {
		bullet->Draw();
	}

	// モデル描画
	objects_[0]->Draw();

	// レティクル描画
	objects_[2]->Draw();
}

void Player::ImGui()
{
	ImGui::Begin("Player");
	ImGui::SliderFloat3("translate", &transform_.translate_.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("reticle_translate", &transformReticle_.translate_.x, -10.0f, 10.0f);
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

void Player::UpdateReticle()
{
	// 自機のワールド座標から3Dレティクルのワールド座標を計算

	// 自機から3Dレティクルへの距離
	const float kDistancePlayerTo3DReticle = 40.0f;
	// 自機から3Dレティクルへのオフセット（Z+向き）
	Vector3 offset = { 0.0f,0.0f,1.0f };
	// 自機のワールド行列の回転を反映
	offset = TransformNormal(offset, transformReticle_.matWorld_);
	// ベクトルの長さを整える
	offset = Normalize(offset) * kDistancePlayerTo3DReticle;
	// レティクルの座標を設定
	transformReticle_.translate_ = GetWorldPosition() + offset;
}

Vector3 Player::GetWorldPosition() {
	//ワールド座標を入れる変数
	Vector3 worldPos;
	//ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = transform_.matWorld_.m[3][0];
	worldPos.y = transform_.matWorld_.m[3][1];
	worldPos.z = transform_.matWorld_.m[3][2];
	return worldPos;
}

Vector3 Player::GetReticleWorldPosition(const Transform& transform)
{
	//ワールド座標を入れる変数
	Vector3 worldPos;
	//ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = transform.matWorld_.m[3][0];
	worldPos.y = transform.matWorld_.m[3][1];
	worldPos.z = transform.matWorld_.m[3][2];
	return worldPos;
}

void Player::Attack()
{
	// 弾の速度
	const float kBulletSpeed = 1.0f;
	Vector3 velocity = { 0.0f,0.0f,kBulletSpeed };
	// 自機から照準オブジェクトへのベクトル
	velocity = GetReticleWorldPosition(transformReticle_) - GetWorldPosition();
	velocity = Normalize(velocity) * kBulletSpeed;

	// objectを1つ用意
	std::unique_ptr<Object3d> object = std::make_unique<Object3d>();
	object->Initialize(objects_[1]->GetCamera(), objects_[1]->GetModel());

	// 弾を生成
	std::unique_ptr<PlayerBullet> bullet = std::make_unique<PlayerBullet>();
	bullet->Initialize(std::move(object), this->transform_.translate_, velocity);
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
