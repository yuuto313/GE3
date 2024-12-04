#pragma once
#include "ParticleManager.h"

/// <summary>
/// パーティクル発生器
/// </summary>
class ParticleEmitter
{
public:

	void Initialize(const std::string name, const Vector3& position, uint32_t count);

	void Update();

	void Draw();

private:

	ParticleManager* particleManager_ = nullptr;

	std::string groupName_;	// パーティクルグループ名
	Vector3 position_;
	uint32_t count_;		// 発生数
	float frequency_;		// 発生頻度
	float frequencyTime_;	// 頻度用時刻

private:
	void Emit();
};

