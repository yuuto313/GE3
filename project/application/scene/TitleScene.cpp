#include "TitleScene.h"
#include "SceneManager.h"
#include "ImGuiManager.h"

void TitleScene::Initialize()
{
	//-------------------------------------
	// オーディオファイルの読み込み
	//-------------------------------------



	//-------------------------------------
	// BGM再生開始
	//-------------------------------------

	

	//-------------------------------------
	// テクスチャファイルの読み込み
	//-------------------------------------

	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");

	//-------------------------------------
	// スプライト生成
	//-------------------------------------

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "resource/uvChecker.png");

	position_ = sprite_->GetPosition();
	position_ = Vector2(100.0f, 100.0f);
	sprite_->SetPosition(position_);

}

void TitleScene::Finalize()
{
	if(sprite_) {
		sprite_.reset();
	}
}

void TitleScene::Update()
{
	//-------------------------------------
	// シーン切り替え依頼
	//-------------------------------------

	// enterキーを押したら
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("GAME");
	}

	//-------------------------------------
	// スプライトの更新
	//-------------------------------------

	sprite_->Update();
}

void TitleScene::ImGui()
{
#ifdef _DEBUG
	ImGui::Begin("Info");
	ImGui::Text("ENTER : GameScene\n");
	ImGui::End();
#endif _DEBUG
}

void TitleScene::Draw()
{
	//-------------------------------------
	// Sprite個々の描画
	//-------------------------------------

	sprite_->Draw();
}
