#pragma once
#include "Transform.h"
#include "Matrix.h"
#include "GameMath.h"

using namespace GameMath;

class Camera
{
public:

	Camera();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiで値を調整
	/// </summary>
	void ImGui();

	const Matrix4x4& GetWorldMatrix()const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix_; }
	const Vector3& GetRotate()const { return transform_.rotate_; }
	const Vector3& GetTranslate()const { return transform_.translate_; }

	void SetRotate(const Vector3& rotate) { transform_.rotate_ = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate_ = translate; }
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetAspectRatio(float aspectRation) { aspectRatio_ = aspectRation; }
	void SetNearClip(float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(float farClip) { farClip_ = farClip; }

private:
	
	Transform transform_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;

	Matrix4x4 projectionMatrix_;
	// 水平方向視野角
	float fovY_;
	// アスペクト比
	float aspectRatio_;
	// ニアクリップ比
	float nearClip_;
	// ファークリップ比
	float farClip_;

	// 合成行列
	Matrix4x4 viewProjectionMatrix_;


};

