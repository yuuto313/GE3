#pragma once
#include "BaseScene.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Input.h"

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene : public BaseScene
{
public:// メンバ関数
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

private:// メンバ変数
	std::unique_ptr<Sprite> sprite_;
	Vector2 position_;
};

