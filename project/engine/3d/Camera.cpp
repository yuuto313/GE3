#include "Camera.h"
#include "WinApp.h"
#include "ImGuiManager.h"

Camera::Camera()
	: transform_()
	,fovY_(0.45f)
	,aspectRatio_(float(WinApp::kClientWidth)/float(WinApp::kClientHeight))
	,nearClip_(0.1f)
	,farClip_(100.0f)
	,worldMatrix_(MakeAffineMatrix(transform_.scale_,transform_.rotate_,transform_.translate_))
	,viewMatrix_(Inverse(worldMatrix_))
	,projectionMatrix_(MakePerspectiveFovMatrix(fovY_,aspectRatio_,nearClip_,farClip_))
	,viewProjectionMatrix_(Multiply(viewMatrix_,projectionMatrix_))
{}

void Camera::Update()
{

	// transformからアフィン変換行列を計算
	worldMatrix_ = MakeAffineMatrix(transform_.scale_, transform_.rotate_, transform_.translate_);

	// worldMatrixの逆行列
	viewMatrix_ = Inverse(worldMatrix_);

	// プロジェクション行列計算
	projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);

	// 合成行列
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);

}

void Camera::ImGui()
{
#ifdef _DEBUG
	ImGui::Begin("Camera");
	ImGui::DragFloat3("translate", &transform_.translate_.x, 0.1f);
	ImGui::DragFloat3("rotate", &transform_.rotate_.x, 0.05f);
	ImGui::End();
#endif //_DEBUG
}
