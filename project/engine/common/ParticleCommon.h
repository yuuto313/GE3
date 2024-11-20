#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

/// <summary>
/// パーティクル共通部
/// </summary>
class ParticleCommon
{

	void Initialize();

	void Upadate();

	void Draw();

private:

	// インスタンス数
	const uint32_t kNumInstance = 10;

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;


};

