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

class ParticleManager
{
public:

	static ParticleManager* GetInstance();

	struct ParticleGroup
	{
		MaterialData materialData;                                 // マテリアルデータ
		std::list<Particle> particles;                             // パーティクルのリスト
		uint32_t srvIndex;										   // インスタンシング用SRVインデックス
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;// インスタンシングリソース
		static const uint32_t kNumInstance = 10;				   // インスタンス数
		TransformationMatrix* instancingData_ = nullptr;		   // インスタンシングデータを書き込むためのポインタ
		// kNumInstance を返すゲッター
		static uint32_t GetNumInstance()
		{
			return kNumInstance;
		}
	};

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	void Finalize();

	void Update();

	void Draw();

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="name"></param>
	/// <param name="textureFilePath"></param>
	void CreateParticleGroup(const std::string name, const std::string textureFilePath);

	/// <summary>
	/// nameで指定した名前のパーティクルグループにパーティクルを発生させる
	/// </summary>
	/// <param name="name"></param>
	/// <param name="position"></param>
	/// <param name="count"></param>
	void Emit(const std::string name, const Vector3& position, uint32_t count);

	void SetCamera(Camera* camera) { this->pCamera_ = camera; }

private:

	DirectXCommon* pDxCommon_ = nullptr;
	SrvManager* pSrvManager_ = nullptr;
	Model* pModel = nullptr;
	Camera* pCamera_ = nullptr;

	std::unordered_map<std::string, ParticleGroup> particleGroup_;

private:

	/// <summary>
	/// インスタンシングResourceの作成
	/// </summary>
	void CreateInstancingResource(ParticleGroup particleGroup);

private:// シングルトン設計

	static ParticleManager* instance;

	/// <summary>
	/// コンストラクタ、デストラクタの隠蔽
	/// </summary>
	ParticleManager() = default;
	~ParticleManager() = default;

	/// <summary>
	/// コピーコンストラクタの封印
	/// </summary>
	/// <param name=""></param>
	ParticleManager(ParticleManager&) = delete;

	/// <summary>
	/// コピー代入演算の封印
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	ParticleManager& operator=(ParticleManager&) = delete;

};

