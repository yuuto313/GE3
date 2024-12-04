#pragma once
#include "BaseScene.h"
#include "ParticleManager.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Camera.h"
#include "Input.h"
#include "ParticleEmitter.h"

/// <summary>
/// ゲームプレイシーン
/// </summary>
class GameScene : public BaseScene
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui() override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

private:
	std::unique_ptr<Camera> camera_;
	std::unique_ptr<ParticleEmitter> particleEmitter_;

	// 仮
	bool changeTexture_ = false;
	std::string textureFilePath_;

};

