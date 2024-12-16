#pragma once
#include "Object3d.h"
#include "Transform.h"

class Enemy
{
public:// メンバ関数

	void Initialize(Object3d* object);

	void Update();

	void Draw();


private:// メンバ変数

	Object3d* object_ = nullptr;

	Transform transform_;
	Vector3 velocity_;

};

