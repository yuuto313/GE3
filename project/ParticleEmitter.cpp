#include "ParticleEmitter.h"
#include "ParticleManager.h"

void ParticleEmitter::Initialize()
{
	particleManager_ = ParticleManager::GetInstance();

	// 0.5秒ごとに発生
	frequency = 0.5f;
	// 発生頻度用の時刻、0で初期化
	frequencyTime = 0.0f;
}

void ParticleEmitter::Update(const std::string name, const Vector3& position, uint32_t count)
{
	particleManager_->Update();

	Emit(name, position, count);

	// 時刻を進める
	//float deltaTime = 1.0f / 60.0f;
	//frequencyTime += deltaTime;
	//// 発生頻度より大きいなら発生
	//if (frequency <= frequencyTime) {
	//	Emit(name, position, count);
	//	// 余計に過ぎた時間も加味して頻度計算する
	//	frequencyTime -= frequency;
	//}
}

void ParticleEmitter::Draw()
{
	//-------------------------------------
	// パーティクル個々の更新
	//-------------------------------------

	particleManager_->Draw();
}

void ParticleEmitter::Emit(const std::string name, const Vector3& position, uint32_t count)
{
	// パーティクルマネージャから呼び出す
	particleManager_->Emit(name, position, count);
}
