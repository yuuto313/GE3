#pragma once
#include "Transform.h"
#include "Object3d.h"
#include "Camera.h"

class Player
{
public:// メンバ関数

	void Initialize();

	void Update();

	void Draw();

	// 右
	void MoveRight();
	// 左
	void MoveLeft();
	// 前
	void MoveForward();
	// 後
	void MoveBack();

	void SetObject(Object3d* object) { this->object_ = object; }

private:// メンバ変数

	Object3d* object_ = nullptr;

	float speed_ = 0.3f;
	Vector3 position_ = {};

};

