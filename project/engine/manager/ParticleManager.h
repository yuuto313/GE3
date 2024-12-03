#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TransformationMatrix.h"
#include "MyMath.h"
#include "Material.h"

#include <random>

class Camera;
class Model;

// Particle構造体
struct Particle {
	Transform transform;
	Vector3 velocity;
};

struct ParticleGroup
{
	MaterialData materialData;                                 // マテリアルデータ
	std::list<Particle> particles;                             // パーティクルのリスト
	uint32_t srvIndex;										   // インスタンシング用SRVインデックス
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;// インスタンシングリソース
	static const uint32_t kNumInstance = 10;				   // インスタンス数
	TransformationMatrix* instancingData_ = nullptr;		   // インスタンシングデータを書き込むためのポインタ
};

// 場

class ParticleManager
{
public:

	static ParticleManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);

	void Finalize();

	void Update();

	void Draw();

	/// <summary>
	/// nameで指定した名前のパーティクルグループにパーティクルを発生させる
	/// </summary>
	void Emit(const std::string name, const Vector3& translate, uint32_t count);

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	void CreateParticleGroup(const std::string name, const std::string textureFilePath);

	static uint32_t GetNumInstance() { return ParticleGroup::kNumInstance; }

	void SetModel(const std::string filePath);

	void SetCamera(Camera* camera) { this->pCamera_ = camera; }

private:

	DirectXCommon* pDxCommon_ = nullptr;
	SrvManager* pSrvManager_ = nullptr;
	Model* pModel = nullptr;
	Camera* pCamera_ = nullptr;

	// ランダムエンジン
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	// パーティクルグループ
	std::unordered_map<std::string, ParticleGroup> particleGroup_;

private:// メンバ関数

	/// <summary>
	/// パーティクルを生成
	/// </summary>
	Particle MakeNewParticle(Vector3& translate);

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
	ParticleManager& operator=(ParticleManager&) = delete;

};

