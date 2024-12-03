#pragma once
#include "ParticleManager.h"

/// <summary>
/// パーティクル発生器
/// </summary>
class ParticleEmitter
{
public:

	void Initialize();

	void Update(const std::string name,const Vector3& position,uint32_t count);

	void Draw();

private:

	ParticleManager* particleManager_ = nullptr;

	Transform transform;	// エミッタのTransform
	uint32_t count;			// 発生数
	float frequency;		// 発生頻度
	float frequencyTime;	// 頻度用時刻

private:
	void Emit(const std::string name, const Vector3& position, uint32_t count);
};

