#pragma once
#include "TransformationMatrix.h"
#include "Transform.h"
#include "DirectionalLight.h"

#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"

#include <d3d12.h>
#include <wrl.h>
#include <vector>

using namespace GameMath;

class Object3dCommon;

/// <summary>
/// 3Dオブジェクト（配置）
/// </summary>
class Object3d
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Camera* camera,std::string filePath);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

public:// ゲッター・セッター

	const Vector3& GetScale()const { return transform_.scale_; }
	const Vector3& GetRotate()const { return transform_.rotate_; }
	const Vector3& GetTranslate()const { return transform_.translate_; }
	const Transform& GetTransform()const { return transform_; }

	void SetScale(const Vector3& scale) { transform_.scale_ = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate_ = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate_ = translate; }
	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetCamera(Camera* camera) { this->pCamera_ = camera; }

	/// <summary>
	/// モデル検索してセットする
	/// </summary>
	/// <param name="filePath"></param>
	void SetModel(const std::string& filePath);

private:// メンバ変数

	Object3dCommon* pObject3dCommon_ = nullptr;
	Model* pModel_ = nullptr;
	Camera* pCamera_ = nullptr;

	// Transform
	Transform transform_;
	Matrix4x4 worldViewProjectionMatrix_ = {};

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;

	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;


private:// メンバ関数

	/// <summary>
	/// 座標変換行列データ作成
	/// </summary>
	void CreateTrasnformationMatrixData();

	/// <summary>
	/// 並行光源データ作成
	/// </summary>
	void CreateDirectionalLightData();

	/// <summary>
	/// WVP行列を作成する
	/// </summary>
	void CreateWVPMatrix();

};

