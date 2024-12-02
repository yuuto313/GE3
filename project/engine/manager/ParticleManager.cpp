#include "ParticleManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleCommon.h"
#include "Camera.h"
#include "Model.h"
#include <random>
#include <numbers>

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new ParticleManager();
	}
	return instance;
}

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	this->pDxCommon_ = dxCommon;
	this->pSrvManager_ = srvManager;

	// ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	//-------------------------------------
	// パーティクル共通部の初期化
	//-------------------------------------

	// パイプライン生成はParticleCommon内で実装
	ParticleCommon::GetInstance()->Initialize(pDxCommon_);

	// モデルの情報を得る
	pModel = ModelManager::GetInstance()->FindModel("plane.obj");

}

void ParticleManager::Finalize()
{

	//-------------------------------------
	// パーティクル共通部の終了処理
	//-------------------------------------

	ParticleCommon::GetInstance()->Finalize();

	delete instance;
	instance = nullptr;
}

void ParticleManager::Update()
{
	// ビルボード行列の計算
	// Y軸でπ/2回転させる
	Matrix4x4 backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);
	// カメラの回転を適用する
	Matrix4x4 billboardMatrix = MyMath::Multiply(backToFrontMatrix, pCamera_->GetWorldMatrix());
	// 平行移動成分はいらない
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;

	// ビュー行列とプロジェクション行列をカメラから取得
	Matrix4x4 viewMatrix = pCamera_->GetViewMatrix();
	Matrix4x4 projectionMatrix = pCamera_->GetProjectionMatrix();

	// 全てのパーティクルグループについて処理する
	for (const auto& particleGroup : particleGroup_) {
		ParticleGroup group = particleGroup.second;
		// グループ内の全てのパーティクルについて処理する
		for (auto it = group.particles.begin(); it != group.particles.end();++it) {
			Particle& particle = *it;
			particle.transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f,},{0.0f,0.0f,0.0f} };
			// 寿命に達していたらグループから外す

			// 場の影響を計算

			// 移動処理

			// 経過時間を加算

			// ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(particle.transform.scale, particle.transform.rotate, particle.transform.translate);
			// ワールドビュープロジェクション行列を合成
			Matrix4x4 worldViewProjection = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
			// インスタンシング用データ1個分の書き込み
			group.instancingData_->WVP = worldViewProjection;
			group.instancingData_->World = worldMatrix;
		}
	}

}

void ParticleManager::Draw()
{
	//-------------------------------------
	// パーティクル共通部の描画準備
	//-------------------------------------

	ParticleCommon::GetInstance()->PreDraw();

	// VBVを設定
	pDxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, pModel->GetVertexBufferView());
	// 全てのパーティクルグループについて処理する
	for (const auto& particle : particleGroup_) {

		//-------------------------------------
		// マテリアルCBufferの場所を設定
		//-------------------------------------
	
		//マテリアルCBufferの場所を設定
		pDxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, pModel->GetMaterialResource()->GetGPUVirtualAddress());

		//-------------------------------------
		// SRVのDescriptorTableの先頭を設定
		//-------------------------------------

		pSrvManager_->SetGraphicsRootDescriptorTable(1, particle.second.srvIndex);
		pSrvManager_->SetGraphicsRootDescriptorTable(2, particle.second.materialData.textureIndex);

		pDxCommon_->GetCommandList()->DrawInstanced(UINT(pModel->GetModelData().vertices.size()), particle.second.kNumInstance, 0, 0);
	}
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath)
{
	// 登録済みの名前かチェックしてassert
	assert(!particleGroup_.contains(name));

	//新たな空のパーティクルグループを作成し、コンテナに登録
	ParticleGroup particleGroup = {};
	// パーティクルの個数を確保
	for (uint32_t i = 0; i < particleGroup.kNumInstance; i++) {
		Particle particle = {};
		particle.transform.scale = { 1.0f,1.0f,1.0f };
		particle.transform.rotate = { 0.0f,0.0f,0.0f };
		particle.transform.translate = { i * 0.1f,i * 0.1f,i * 0.1f };
		particleGroup.particles.push_back(particle);
	}
	// テクスチャファイルパスを設定
	particleGroup.materialData.textureFilePath = textureFilePath;
	// テクスチャを読み込む
	TextureManager::GetInstance()->LoadTexture(particleGroup.materialData.textureFilePath);
	// テクスチャのSRVインデックスを記録
	particleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetSrvIndex(particleGroup.materialData.textureFilePath);
	// インスタンシング用リソースの生成
	CreateInstancingResource(particleGroup);
	// インスタンシング用にSRVを確保してSRVインデックスを記録
	particleGroup.srvIndex = pSrvManager_->Allocate();

	// SRV生成
	pSrvManager_->CreateSRVforParticle(particleGroup.srvIndex,particleGroup.instancingResource_.Get(), sizeof(TransformationMatrix));

	particleGroup_.emplace(name,particleGroup);

}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count)
{
	// 登録済みのパーティクルグループ名かチェック
	assert(!particleGroup_.contains(name));
}

void ParticleManager::CreateInstancingResource(ParticleGroup particleGroup)
{
	// Instancing用のTransformationMatrixリソースを作る
	particleGroup.instancingResource_ = pDxCommon_->CreateBufferResource(sizeof(TransformationMatrix));

	particleGroup.instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData_));

	particleGroup.instancingData_->WVP = MyMath::MakeIdentity4x4();
	particleGroup.instancingData_->World = MyMath::MakeIdentity4x4();
}
