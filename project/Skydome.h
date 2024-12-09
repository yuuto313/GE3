#pragma once
#include "Object3d.h"

class Skydome
{
public:// メンバ関数

	void Initialize(Object3d* object);

	void Update();

	void Draw();

private:// メンバ変数

	Object3d* object_ = nullptr;

};

