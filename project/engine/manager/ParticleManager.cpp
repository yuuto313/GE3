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

	// フィールドを生成
	accelerationField_.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField_.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField_.area.max = { 1.0f,1.0f,1.0f };

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

	const float deltaTime = 1.0f / 60.0f;

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

		ParticleForGPU* newInstancingData = particleGroupIterator->second.instancingData_;
		// 描画すべきインスタンス数
		particleGroupIterator->second.kNumInstance = 0;

		// グループ内の全てのパーティクルについて処理する
		for (std::list<Particle>::iterator particleIterator = particleGroupIterator->second.particles.begin();
			particleIterator != particleGroupIterator->second.particles.end();) {

			// 寿命に達していたらグループから外す
			if (particleIterator->lifeTime <= particleIterator->currentTime) {
				particleIterator = particleGroupIterator->second.particles.erase(particleIterator);
			} else {
				// 場の影響を計算
				if (MyMath::IsCollision(accelerationField_.area, particleIterator->transform.translate)) {
					particleIterator->velocity += accelerationField_.acceleration * deltaTime;
				}
				// 移動処理
				particleIterator->transform.translate += particleIterator->velocity * deltaTime;
				// 経過時間を加算
				particleIterator->currentTime += deltaTime;
				float alpha = 1.0f - (particleIterator->currentTime / particleIterator->lifeTime);
				if (particleGroupIterator->second.kNumInstance <= particleGroupIterator->second.kNumMaxInstance) {
					// ワールド行列を計算
					Matrix4x4 scaleMatrix = MyMath::MakeScaleMatrix(particleIterator->transform.scale);
					Matrix4x4 translateMatrix = MyMath::MakeTranslateMatrix(particleIterator->transform.translate);
					Matrix4x4 worldMatrix = MyMath::Multiply(scaleMatrix, MyMath::Multiply(billboardMatrix, translateMatrix));

					// ワールドビュープロジェクション行列を合成
					Matrix4x4 worldViewProjection = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));

					// インスタンシング用データ1個分の書き込み
					newInstancingData->WVP = worldViewProjection;
					newInstancingData->World = worldMatrix;
					newInstancingData->color = particleIterator->color;
					newInstancingData->color.w = alpha;
					newInstancingData++;

					// 生きているParticleの数を1つカウントする
					++particleGroupIterator->second.kNumInstance;
				}
				++particleIterator;
			}
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
	particleGroup.instancingResource_ = pDxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroup.kNumMaxInstance);

	particleGroup.instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData_));

	// 単位行列とNoneBlendで書き込んでおく
	for (uint32_t index = 0; index < particleGroup.kNumMaxInstance; ++index) {
		particleGroup.instancingData_[index].WVP = MyMath::MakeIdentity4x4();
		particleGroup.instancingData_[index].World = MyMath::MakeIdentity4x4();
		particleGroup.instancingData_[index].color = { 1.0f,1.0f,1.0f,1.0f };
	}

	// インスタンシング用にSRVを確保してSRVインデックスを記録
	particleGroup.srvIndex = pSrvManager_->Allocate();

	// SRV生成
	pSrvManager_->CreateSRVforStructuredBuffer(particleGroup.srvIndex,particleGroup.instancingResource_.Get(),particleGroup.kNumMaxInstance, sizeof(ParticleForGPU));

}

void ParticleManager::SetModel(const std::string filePath)
{
	// モデルの情報を得る
	pModel = ModelManager::GetInstance()->FindModel(filePath);
}

Particle ParticleManager::MakeNewParticle(const Vector3& translate)
{
	// 位置
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	// 色
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	// 生存時間
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);

	// パーティクルを1つ生成
	Particle particle;
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	// 発生場所
	Vector3 randomTranslate = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };
	particle.transform.translate = translate + randomTranslate;
	particle.velocity = { distribution(randomEngine_),distribution(randomEngine_),distribution(randomEngine_) };

	// 色を変更
	particle.color = { distColor(randomEngine_),distColor(randomEngine_),distColor(randomEngine_),1.0f };
	// 1~3秒間生存
	particle.lifeTime = distTime(randomEngine_);
	particle.currentTime = 0;

	return particle;
}

void ParticleManager::Emit(const std::string name, const Vector3& translate, uint32_t count)
{
	// 登録済みのパーティクルグループ名かチェック
	assert(particleGroup_.contains(name));

	// 新たなパーティクルを作成し、指定されたパーティクルグループに登録
	for (uint32_t index = 0; index < count; ++index) {
		particleGroup_[name].particles.push_back(MakeNewParticle(translate));
	}
}
