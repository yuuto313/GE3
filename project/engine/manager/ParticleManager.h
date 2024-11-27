#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TransformationMatrix.h"
#include "MyMath.h"
#include "Object3d.h"

class Camera;
class Model;

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
	Model* pModel = nullptr;
	Camera* pCamera_ = nullptr;

	Transform transforms;

	std::unordered_map<std::string, ParticleGroup> particleGroup_;

private:

	/// <summary>
	/// インスタンシングResourceの作成
	/// </summary>
	void CreateInstancingResource(ParticleGroup particleGroup);

};

