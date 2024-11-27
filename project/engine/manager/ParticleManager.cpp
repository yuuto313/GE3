#include "ParticleManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Camera.h"
#include <random>
#include <numbers>

void ParticleManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	this->pDxCommon_ = dxCommon;
	this->pSrvManager_ = srvManager;

	// ランダムエンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	// パイプライン生成はParticleCommon内で実装

	// 頂点データの初期化
	CreateVertexData();
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
	for (const auto& particle : particleGroup_) {
		ParticleGroup group = particle.second;
		// グループ内の全てのパーティクルについて処理する
		for (auto it = group.particles.begin(); it != group.particles.end();++it) {
			Particle& particle = *it;
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
	// VBVを設定
	pDxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// 全てのパーティクルグループについて処理する
	for (const auto& particle : particleGroup_) {

		//-------------------------------------
		// マテリアルCBufferの場所を設定
		//-------------------------------------
	
		//マテリアルCBufferの場所を設定
		pDxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

		//-------------------------------------
		// SRVのDescriptorTableの先頭を設定
		//-------------------------------------

		pSrvManager_->SetGraphicsRootDescriptorTable(1, 3);

		pSrvManager_->SetGraphicsRootDescriptorTable(2, particle.second.srvIndex);

		pDxCommon_->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), particle.second.kNumInstance, 0, 0);
	}
}

void ParticleManager::CreateParticleGroup(const std::string name, const std::string textureFilePath)
{
	// 登録済みの名前かチェックしてassert
	assert(particleGroup_.contains(name));

	//新たな空のパーティクルグループを作成し、コンテナに登録
	ParticleGroup particleGroup;
	// テクスチャファイルパスを設定
	particleGroup.materialData.textureFilePath = textureFilePath;
	// テクスチャを読み込む
	TextureManager::GetInstance()->LoadTexture(particleGroup.materialData.textureFilePath);
	// テクスチャのSRVインデックスを記録
	particleGroup.materialData.textureIndex = TextureManager::GetInstance()->GetSrvIndex(particleGroup.materialData.textureFilePath);
	// インスタンシング用リソースの生成
	CreateInstancingResource();
	// インスタンシング用にSRVを確保してSRVインデックスを記録
	particleGroup.srvIndex = pSrvManager_->Allocate();

	// SRV生成
	pSrvManager_->CreateSRVforParticle(particleGroup.srvIndex,particleGroup.instancingResource_.Get(), sizeof(TransformationMatrix));

	particleGroup_.emplace(name,particleGroup);

}

void ParticleManager::CreateVertexData()
{
	//-------------------------------------
	// VertexResourceを作る
	//-------------------------------------

	vertexResource_ = pDxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	//-------------------------------------
	// VertexBufferViewを作成する(値を設定するだけ)
	//-------------------------------------

	//リソースの先頭アドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	//１頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//-------------------------------------
	// VertexResourceにデータを書き込むためのアドレスを取得してVertexDataに割り当てる
	//-------------------------------------

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	//頂点データをリソースにコピー
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}

void ParticleManager::CreateMaterialData()
{
	//-------------------------------------
	// マテリアルリソースを作る
	//-------------------------------------

	materialResource_ = pDxCommon_->CreateBufferResource(sizeof(Material));

	//-------------------------------------
	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	//-------------------------------------

	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	//-------------------------------------
	// マテリアルデータの初期値を書き込む
	//-------------------------------------

	// 今回は白を書き込み
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// SpriteはLightingしないのでfalseを設定する
	materialData_->enableLighting = false;
	// 単位行列で初期化
	materialData_->uvTransform = MyMath::MakeIdentity4x4();
}

void ParticleManager::CreateInstancingResource()
{
	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = pDxCommon_->CreateBufferResource(sizeof(TransformationMatrix));

	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	instancingData_->WVP = MyMath::MakeIdentity4x4();
	instancingData_->World = MyMath::MakeIdentity4x4();
}
