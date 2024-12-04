#pragma once
#include "Transform.h"
#include "Vector.h"
#include "MaterialLibrary.h"
#include "ParticleForGPU.h"

#include <list>
#include <cstdint>
#include <d3d12.h>
#include <wrl.h>

// パーティクルの情報をまとめた構造体群

// Particle構造体
struct Particle {
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;		// 生存可能時間
	float currentTime;	// 発生してからの経過時間
};

struct ParticleGroup
{
	MaterialData materialData;                                 // マテリアルデータ
	std::list<Particle> particles;                             // パーティクルのリスト
	uint32_t srvIndex;										   // インスタンシング用SRVインデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;// インスタンシングリソース
	uint32_t kNumInstance = 0;								   // インスタンス数
	static const uint32_t kNumMaxInstance = 100;				   // 最大数
	ParticleForGPU* instancingData_ = nullptr;				   // インスタンシングデータを書き込むためのポインタ
};

