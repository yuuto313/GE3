#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ModelManager.h"
#include "GameMath.h"
#include "ParticleSystem.h"
#include "Field.h"

#include <random>

class Camera;
class Model;

using namespace GameMath;

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

	/// <summary>
	/// パーティクルグループをリセット
	/// </summary>
	void Reset();

	static uint32_t GetNumMaxInstance() { return ParticleGroup::kNumMaxInstance; }

	// パーティクルグループのテクスチャを切り替える
	void SetTexture(const std::string& name, const std::string& newTextureFilePath);

	void SetModel(const std::string filePath){ pModel = ModelManager::GetInstance()->FindModel(filePath); }

	void SetCamera(Camera* camera) { this->pCamera_ = camera; }

private:// メンバ変数

	DirectXCommon* pDxCommon_ = nullptr;
	SrvManager* pSrvManager_ = nullptr;
	Model* pModel = nullptr;
	Camera* pCamera_ = nullptr;

	// ランダムエンジン
	std::random_device seedGenerator_;
	std::mt19937 randomEngine_;

	// パーティクルグループ
	std::unordered_map<std::string, ParticleGroup> particleGroup_;

	// フィールド
	AccelerationField accelerationField_;

private:// メンバ関数

	/// <summary>
	/// パーティクルを生成
	/// </summary>
	Particle MakeNewParticle(const Vector3& translate);

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

