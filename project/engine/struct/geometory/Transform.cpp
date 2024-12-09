#include "Transform.h"

using namespace GameMath;

Transform::Transform()
{
	// 生成時に初期化
	Initilaize();
}

void Transform::Initilaize()
{
	this->scale_ = { 1.0f,1.0f,1.0f };
	this->rotate_ = { 0.0f,0.0f,0.0f };
	this->translate_ = { 0.0f,0.0f,0.0f };
}

void Transform::UpdateMatrix()
{
	// スケール、回転、平行移動を合成して行列を計算する
	matWorld_ = MakeAffineMatrix(scale_, rotate_, translate_);
	// 親があれば親のワールド行列を掛ける
	if (parent_) {
		matWorld_ *= parent_->matWorld_;
	}
}
