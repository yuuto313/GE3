#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include "DirectXCommon.h"

/// <summary>
/// パーティクル共通部
/// </summary>
class ParticleCommon
{
private:// シングルトン設計
	static ParticleCommon* instance;

	/// <summary>
	/// コンストラクタ、デストラクタの封印
	/// </summary>
	ParticleCommon() = default;
	~ParticleCommon() = default;

	/// <summary>
	/// コピーコンストラクタの封印
	/// </summary>
	/// <param name=""></param>
	ParticleCommon(ParticleCommon&) = delete;

	/// <summary>
	/// コピー代入演算の封印
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	ParticleCommon& operator=(ParticleCommon&) = delete;

public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static ParticleCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

private:
	DirectXCommon* pDxCommon_ = nullptr;

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;

	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	D3D12_ROOT_PARAMETER rootParameters_[3] = {};
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[3] = {};
	D3D12_BLEND_DESC blendDesc_{};
	D3D12_RASTERIZER_DESC rasterizerDesc_{};
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;

private:
	/// <summary>
	/// ルートシグネイチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraphicsPipeline();

	static Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(Microsoft::WRL::ComPtr<ID3D12Device> device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
};

