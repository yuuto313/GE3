#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

#include "DirectXCommon.h"
#include "MyTransform.h"
#include "TransformationMatrix.h"
#include "MyMath.h"

/// <summary>
/// パーティクル
/// </summary>
class Particle
{
public:

	void Initialzie(DirectXCommon* dxCommon);

	void Update();

	void Draw();

private:

	DirectXCommon* pDxCommon_ = nullptr;

	// インスタンス数
	const uint32_t kNumInstance = 10;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* instancingData_ = nullptr;

	Transform transforms_;

private:

	/// <summary>
	/// Resourceの作成
	/// </summary>
	void CreateResource();


};

