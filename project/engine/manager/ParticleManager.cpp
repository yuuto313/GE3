#include "ParticleManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleCommon.h"
#include "Camera.h"
#include "Model.h"
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
	randomEngine_ = std::mt19937(seedGenerator_());

	//-------------------------------------
	// パーティクル共通部の初期化
	//-------------------------------------

	// パイプライン生成はParticleCommon内で実装
	ParticleCommon::GetInstance()->Initialize(pDxCommon_);

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
	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroupIterator = particleGroup_.begin();
		particleGroupIterator != particleGroup_.end();
		++particleGroupIterator){
		// グループ内の全てのパーティクルについて処理する
		for (std::list<Particle>::iterator particleIterator = particleGroupIterator->second.particles.begin();
			particleIterator != particleGroupIterator->second.particles.end();) {
			
			// 寿命に達していたらグループから外す

			// 場の影響を計算

			// 移動処理

			// 経過時間を加算

			// ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(particleIterator->transform.scale, particleIterator->transform.rotate, particleIterator->transform.translate);
			// ワールドビュープロジェクション行列を合成
			Matrix4x4 worldViewProjection = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
			// インスタンシング用データ1個分の書き込み
			particleGroupIterator->second.instancingData_->WVP = worldViewProjection;
			particleGroupIterator->second.instancingData_->World = worldMatrix;

			++particleIterator;
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
	for (std::unordered_map<std::string, ParticleGroup>::iterator particleGroupIterator = particleGroup_.begin();
		particleGroupIterator != particleGroup_.end();
		++particleGroupIterator) {

		//-------------------------------------
		// マテリアルCBufferの場所を設定
		//-------------------------------------
	
		//マテリアルCBufferの場所を設定
		pDxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, pModel->GetMaterialResource()->GetGPUVirtualAddress());

		//-------------------------------------
		// SRVのDescriptorTableの先頭を設定
		//-------------------------------------

		pSrvManager_->SetGraphicsRootDescriptorTable(1, particleGroupIterator->second.srvIndex);
		pSrvManager_->SetGraphicsRootDescriptorTable(2, particleGroupIterator->second.materialData.textureIndex);

		pDxCommon_->GetCommandList()->DrawInstanced(UINT(pModel->GetModelData().vertices.size()), particleGroupIterator->second.kNumInstance, 0, 0);
	}
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath)
{
	// 登録済みの名前かチェックしてassert
	assert(!particleGroup_.contains(name));

	//新たな空のパーティクルグループを作成し、コンテナに登録
	ParticleGroup& particleGroup = particleGroup_[name];
	// テクスチャファイルパスを設定
	particleGroup.materialData.textureFilePath = textureFilePath;

	// テクスチャのSRVインデックスを記録
	particleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetSrvIndex(particleGroup.materialData.textureFilePath);

	// インスタンシング用リソースの作成
	// Instancing用のTransformationMatrixリソースを作る
	particleGroup.instancingResource_ = pDxCommon_->CreateBufferResource(sizeof(TransformationMatrix) * particleGroup.kNumInstance);

	particleGroup.instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData_));

	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < particleGroup.kNumInstance; ++index) {
		particleGroup.instancingData_[index].WVP = MyMath::MakeIdentity4x4();
		particleGroup.instancingData_[index].World = MyMath::MakeIdentity4x4();
	}

	// インスタンシング用にSRVを確保してSRVインデックスを記録
	particleGroup.srvIndex = pSrvManager_->Allocate();

	// SRV生成
	pSrvManager_->CreateSRVforParticle(particleGroup.srvIndex,particleGroup.instancingResource_.Get(), sizeof(TransformationMatrix));

}

void ParticleManager::SetModel(const std::string filePath)
{
	// モデルの情報を得る
	pModel = ModelManager::GetInstance()->FindModel(filePath);
}

Particle ParticleManager::MakeNewParticle(Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	const float deltaTime = 1.0f / 60.0f;

	Particle particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.translate = { translate };
	// 発生場所
	/*Vector3 randomTranslate{ distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };
	particle.transform.translate = randomTranslate + translate;
	particle.velocity = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };

	particle.transform.translate += particle.velocity * deltaTime;*/

	return particle;
}

void ParticleManager::Emit(const std::string name, const Vector3& translate, uint32_t count)
{
	// 登録済みのパーティクルグループ名かチェック
	assert(particleGroup_.contains(name));

	// 新たなパーティクルを作成し、指定されたパーティクルグループに登録
	for (uint32_t index = 0; index < count; ++index) {
		Vector3 newTranslate = { index * translate.x,index * translate.y,index * translate.z };
		particleGroup_[name].particles.push_back(MakeNewParticle(newTranslate));
	}
}
