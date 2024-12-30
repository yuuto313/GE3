#include "Enemy.h"
#include "Player.h"

void (Enemy::* Enemy::pFunc[])() = {
	&Enemy::ApproachUpdate,
	&Enemy::LeaveUpdate,
};

void Enemy::Initialize(const std::vector<Object3d*>& objects)
{

	this->pObjects_ = objects;

	transform_.Initilaize();
	transform_ = pObjects_[0]->GetTransform();
	transform_.translate_ = { 3.0f,3.0f,100.0f };

	approachVelocity_ = { 0.0f,0.0f,-0.08f };
	leaveVelocity_ = { 0.03f,0.03f,0.03f };


	// クールタイム変数をリセット
	lastAttackTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(2);
}

void Enemy::Update()
{
	// デスフラグの立った弾を削除
	bullets_.remove_if([](const std::unique_ptr<EnemyBullet>& bullet) {return bullet->IsDead(); });

	// 行動処理
	(this->*pFunc[static_cast<size_t>(phase_)])();

	for (const auto& bullet : bullets_) {
		bullet->Update();
	}

	transform_.UpdateMatrix();

	pObjects_[0]->SetTransform(transform_);
	pObjects_[0]->Update();
}

void Enemy::Draw()
{
	pObjects_[0]->Draw();

	// 弾の描画
	for (const auto& bullet : bullets_) {
		bullet->Draw();
	}

}

void Enemy::Fire()
{
	// 現在の時刻を取得
	auto now = std::chrono::steady_clock::now();

	// 前回の時刻との差を計算
	float elapsedSeconds = std::chrono::duration<float>(now - lastAttackTime_).count();

	// クールタイム中なら攻撃しない
	if (elapsedSeconds < bulletCoolTime_) {
		return;
	}

	// 弾の速さ
	const float kBulletSpeed = -1.8f;
	// 自キャラのワールド座標
	Vector3 playerWorldPos = player_->GetWorldPosition();
	// 敵キャラのワールド座標
	Vector3 worldPos = GetWorldPosition();
	// 敵キャラから自キャラへの差分ベクトルを求める
	Vector3 diff = worldPos - playerWorldPos;
	// ベクトルを正規化
	diff = Normalize(diff);
	// ベクトルの長さを速さに合わせる
	diff = diff * kBulletSpeed;

	Vector3 velocity = diff;


	std::unique_ptr<Object3d> object = std::make_unique<Object3d>();
	object->Initialize(pObjects_[1]->GetCamera(), pObjects_[1]->GetModel());

	std::unique_ptr<EnemyBullet> bullet = std::make_unique<EnemyBullet>();
	bullet->Initialize(std::move(object), transform_.translate_,velocity);
	bullets_.push_back(std::move(bullet));

	// 攻撃時刻を更新
	lastAttackTime_ = now;

}

void Enemy::ApproachUpdate()
{
	// 攻撃
	Fire();

	// 移動
	transform_.translate_ += approachVelocity_;

	if (transform_.translate_.z < 30.0f) {
		phase_ = Phase::Leave;
	}
}

void Enemy::LeaveUpdate()
{
	// 移動
	transform_.translate_ += leaveVelocity_;
}

Vector3 Enemy::GetWorldPosition()
{
	//ワールド座標を入れる変数
	Vector3 worldPos;
	//ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = transform_.matWorld_.m[3][0];
	worldPos.y = transform_.matWorld_.m[3][1];
	worldPos.z = transform_.matWorld_.m[3][2];
	return worldPos;
}
