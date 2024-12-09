#pragma once
#include "ParticleManager.h"

/// <summary>
/// パーティクル発生器
/// </summary>
class ParticleEmitter
{
public:

	/// <param name="name">パーティクルグループの名前</param>
	/// <param name="translate">エミッターの場所</param>
	/// <param name="count">パーティクル生成数</param>
	void Initialize(const std::string name, const Vector3& translate, uint32_t count);

	void Update();

	void Draw();

	void SetTranslate(const Vector3& translate) { transform_.translate_ = translate; }

private:

	ParticleManager* particleManager_ = nullptr;

	std::string groupName_;	// パーティクルグループ名
	Transform transform_;
	uint32_t count_;		// 発生数
	float frequency_;		// 発生頻度
	float frequencyTime_;	// 頻度用時刻

private:
	void Emit();
};

