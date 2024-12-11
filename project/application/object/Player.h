#pragma once
#include "Transform.h"
#include "Object3d.h"
#include "PlayerBullet.h"

#include <memory>
#include <vector>
#include <list>

class Player
{
public:// メンバ関数

	void Initialize(const std::vector<Object3d*>& objects);

	void Update();

	void Draw();

	void ImGui();

	// 位置を制限する
	void ClampPosition();

	// 攻撃
	void Attack();

	// 右
	void MoveRight();
	// 左
	void MoveLeft();
	// 上
	void MoveUp();
	// 下
	void MoveDown();

	const Vector3& GetTranslate() { return this->transform_.translate_; }

private:// メンバ変数

	std::vector<Object3d*> objects_;
	std::list<std::unique_ptr<PlayerBullet>> bullets_;


	float speed_ = 0.3f;
	Transform transform_ = {};

};

