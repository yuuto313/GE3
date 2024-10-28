#pragma once
#include "VertexData.h"
#include "Material.h"
#include "TransformationMatrix.h"
#include "DirectXCommon.h"

class SpriteCommon;

/// <summary>
/// スプライト
/// </summary>
class Sprite
{
public:

	//-------------基本処理-------------//

	Sprite();
	~Sprite();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="spriteCommon"></param>
	/// <param name="dxCommon"></param>
	void Initialize(SpriteCommon* spriteCommon,DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU, Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource);

	//-------------ゲッター・セッター-------------//


private:
	//-------------メンバ変数-------------//

	SpriteCommon* spriteCommon_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;

	Transform transform_ = {};
	Transform uvTransform_ = {};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrix_;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	Material* materialData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};

	//-------------メンバ関数-------------//

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// 配列データ作成
	/// </summary>
	void CreateIndexData();

	/// <summary>
	/// マテリアルデータ作成
	/// </summary>
	void CreateMaterialData();

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTrasnformationMatrixData();

	/// <summary>
	/// WVP行列を作成する
	/// </summary>
	void CreateWVPMatrix();

};

