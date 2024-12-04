#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include "ImGuiManager.h"

void ParticleEmitter::Initialize(const std::string name, const Vector3& translate, uint32_t count)
{
	particleManager_ = ParticleManager::GetInstance();

	// 引数で受け取ってメンバ変数に記録
	groupName_ = name;
	count_ = count;

	// Emitterの情報を初期化
	transform_.scale = { 1.0f,1.0f,1.0f };
	transform_.rotate = { 0.0f,0.0f,0.0f };
	transform_.translate = translate;

	// 0.5秒ごとに発生
	frequency_ = 0.5f;
	// 発生頻度用の時刻、0で初期化
	frequencyTime_ = 0.0f;

	Emit();
}

void ParticleEmitter::Update()
{
	particleManager_->Update();

	// 時刻を進める
	float deltaTime = 1.0f / 60.0f;
	frequencyTime_ += deltaTime;
	// 発生頻度より大きいなら発生
	if (frequency_ <= frequencyTime_) {
		// 発生処理
		Emit();
		// 余計に過ぎた時間も加味して頻度計算する
		frequencyTime_ -= frequency_;
	}

	ImGui::Begin("Emitter");
	ImGui::DragFloat3("EmitterTranslate", &transform_.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::End();

}

void ParticleEmitter::Draw()
{
	//-------------------------------------
	// パーティクル個々の更新
	//-------------------------------------

	particleManager_->Draw();
}

void ParticleEmitter::Emit()
{
	// パーティクルマネージャから呼び出す
	particleManager_->Emit(groupName_, transform_.translate, count_);
}
