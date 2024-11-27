#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TransformationMatrix.h"
#include "MyMath.h"
#include "Model.h"
#include "Object3d.h"
#include "ModelData.h"

class Camera;

// Particle構造体
struct Particle {
	Transform transform;
	Vector3 velocity;
};

// 場


struct ParticleGroup
{
	MaterialData materialData;                                 // マテリアルデータ
	std::list<Particle> particles;                             // パーティクルのリスト
	uint32_t srvIndex;										   // インスタンシング用SRVインデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;// インスタンシングリソース
	static const uint32_t kNumInstance = 10;				   // インスタンス数
	TransformationMatrix* instancingData_ = nullptr;		   // インスタンシングデータを書き込むためのポインタ
};

class ParticleManager
{
public:

	static const uint32_t kNumInstance = 10;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	void Update();

	void Draw();

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="name"></param>
	/// <param name="textureFilePath"></param>
	void CreateParticleGroup(const std::string name, const std::string textureFilePath);

	void SetCamera(Camera* camera) { this->pCamera_ = camera; }

private:

	DirectXCommon* pDxCommon_ = nullptr;
	SrvManager* pSrvManager_ = nullptr;
	Camera* pCamera_ = nullptr;
	std::unique_ptr<Object3d> pObjects_;

	std::unordered_map<std::string, ParticleGroup> particleGroup_;

	// objファイルのデータ
	ModelData modelData_;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;
	Material* materialData_ = nullptr;
	TransformationMatrix* instancingData_ = nullptr;

	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

private:

	/// <summary>
	/// 頂点データ作成
	/// </summary>
	void CreateVertexData();

	/// <summary>
	/// マテリアルデータ作成
	/// </summary>
	void CreateMaterialData();

	/// <summary>
	/// インスタンシングResourceの作成
	/// </summary>
	void CreateInstancingResource();

};

