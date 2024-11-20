#include "Particle.h"

void Particle::Initialzie(DirectXCommon* dxCommon)
{
	this->pDxCommon_ = dxCommon;

	// Resourceの作成
	CreateResource();

}

void Particle::Update()
{
	// Transformの書き込み
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		
	}

}

void Particle::Draw()
{
}

void Particle::CreateResource()
{

	// Instancing用のTransformationMatrixリソースを作る
	instancingResource_ = pDxCommon_->CreateBufferResource(sizeof(TransformationMatrix) * kNumInstance);

	instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	// 単位行列を書き込んでおく
	for (uint32_t index = 0; index < kNumInstance; ++index) {
		instancingData_[index].WVP = MyMath::MakeIdentity4x4();
		instancingData_[index].World = MyMath::MakeIdentity4x4();
	}

}
