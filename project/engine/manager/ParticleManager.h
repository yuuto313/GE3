#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "Material.h"

struct ParticleGroup
{
	MaterialData materialData;

};

class ParticleManager
{
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

public:

	void Initialize();

	void Update();

	void Draw();


private:

	std::unique_ptr<DirectXCommon> dxCommon_;
	std::unique_ptr<SrvManager> srvManager_;

};

