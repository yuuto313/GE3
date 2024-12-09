#include "GameScene.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"

void GameScene::Initialize()
{
	//-------------------------------------
	// InputHandlerクラスの生成
	//-------------------------------------

	inputHandler_ = std::make_unique<InputHandler>();

	//-------------------------------------
	// カメラの初期化
	//-------------------------------------

	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({ 0.3f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,4.0f,-10.0f });

	//-------------------------------------
	// 3dオブジェクト生成
	//-------------------------------------

	playerObject_ = std::make_unique<Object3d>();
	playerObject_->Initialize(camera_.get(), "cube.obj");

	//-------------------------------------
	// プレイヤーの初期化
	//-------------------------------------

	player_ = std::make_unique<Player>();
	player_->SetObject(playerObject_.get());
	player_->Initialize();

	//-------------------------------------
	// パーティクルマネージャ生成
	//-------------------------------------

	ParticleManager::GetInstance()->SetCamera(camera_.get());
	ParticleManager::GetInstance()->SetModel("plane.obj");
	// パーティクルグループ生成
	ParticleManager::GetInstance()->CreateParticleGroup("Particle", textureFilePath_);

	//-------------------------------------
	// パーティクルエミッタ生成
	//-------------------------------------

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("Particle", {0.0f,0.0f,0.0f}, 5);

}

void GameScene::Finalize()
{
	ParticleManager::GetInstance()->Reset();
	{
		// 解放処理を入れないとメモリリークするため記述
		// 原因が分かり次第削除
		player_.reset();

		playerObject_.reset();
	}

}

void GameScene::Update()
{
	//-------------------------------------
	// シーン切り替え依頼
	//-------------------------------------

	// enterキーを押したら
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		// シーン切り替え
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

	//-------------------------------------
	// コマンド系クラスの更新
	//-------------------------------------

	// get Input
	iCommand_ = inputHandler_->HandleInput();

	// set Command
	if (this->iCommand_) {
		iCommand_->Exec(*player_.get());
	}


	//-------------------------------------
	// カメラの更新
	//-------------------------------------

	// 3dオブジェクトの更新より前に行う
	camera_->Update();

	//-------------------------------------
	// プレイヤーの更新
	//-------------------------------------

	player_->Update();

	//-------------------------------------
	// パーティクルエミッターの更新
	//-------------------------------------

	ParticleManager::GetInstance()->SetTexture("Particle", textureFilePath_);
	particleEmitter_->Update();

}

void GameScene::ImGui()
{
	ImGui::Begin("Particle");

	// チェックボックスでテクスチャを変更
	ImGui::Checkbox("Change Texture", &changeTexture_);

	// テクスチャを変更する
	if (changeTexture_) {
		textureFilePath_ = "resource/circle.png";  // 切り替えるテクスチャのパス
	} else {
		textureFilePath_ = "resource/uvChecker.png";
	}

	ImGui::End();

	camera_->ImGui();

}

void GameScene::Draw()
{
	player_->Draw();

	particleEmitter_->Draw();
}


