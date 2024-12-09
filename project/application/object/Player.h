#pragma once
#include "Transform.h"
#include "Object3d.h"
#include "Camera.h"

class Player
{
public:// メンバ関数

	void Initialize(Object3d* object);

	void Update();

	void Draw();

	void ImGui();

	// 位置を制限する
	void ClampPosition();

	// 右
	void MoveRight();
	// 左
	void MoveLeft();
	// 上
	void MoveUp();
	// 下
	void MoveDown();

	const Vector3& GetTranslate()const { return object_->GetTranslate(); }

private:// メンバ変数

	Object3d* object_ = nullptr;

	float speed_ = 0.3f;
	Transform transform_ = {};

};

