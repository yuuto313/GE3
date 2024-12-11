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
	camera_->SetRotate({ 0.1f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,2.5f,-17.6f });

	//-------------------------------------
	// 3dオブジェクト生成
	//-------------------------------------

	skydomeObj_ = std::make_unique<Object3d>();
	skydomeObj_->Initialize(camera_.get(), "skydome.obj");

	// 必要なサイズにリサイズ
	playerObjects_.resize(2);
	playerObjects_[0] = std::make_unique<Object3d>();
	playerObjects_[0]->Initialize(camera_.get(), "cube.obj");

	playerObjects_[1] = std::make_unique<Object3d>();
	playerObjects_[1]->Initialize(camera_.get(), "cube.obj");

	//-------------------------------------
	// 天球の生成
	//-------------------------------------

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(skydomeObj_.get());


	//-------------------------------------
	// プレイヤーの生成
	//-------------------------------------

	player_ = std::make_unique<Player>();
	player_->Initialize(playerObjects_);

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
	textureFilePath_ = "resource/circle.png";
	particleEmitter_->Initialize("Particle", player_->GetTranslate(), 5);

}

void GameScene::Finalize()
{
	ParticleManager::GetInstance()->Reset();

	// 明示的に解放処理を入れないとメモリリークするため記述
	// 原因が分かり次第削除
	{
		player_.reset();

		skydome_.reset();

		playerObjects_.clear();
	
		skydomeObj_.reset();
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
	// Commandクラスの更新
	//-------------------------------------

	// get Input
	commands_ = inputHandler_->HandleInput();

	// set Command
	for (const auto& command : commands_) {
		if (command) {
			command->Exec(*player_.get());
		}
	}

	//-------------------------------------
	// カメラの更新
	//-------------------------------------

	// ※3dオブジェクトの更新より前に行う※
	camera_->Update();

	//-------------------------------------
	// 天球の更新
	//-------------------------------------

	skydome_->Update();

	//-------------------------------------
	// プレイヤーの更新
	//-------------------------------------

	player_->Update();

	//-------------------------------------
	// パーティクルエミッターの更新
	//-------------------------------------

	ParticleManager::GetInstance()->SetTexture("Particle", textureFilePath_);
	particleEmitter_->SetTranslate(player_->GetTranslate());
	particleEmitter_->Update();

}

void GameScene::ImGui()
{
	camera_->ImGui();
	player_->ImGui();
}

void GameScene::Draw()
{
	skydome_->Draw();

	player_->Draw();

	particleEmitter_->Draw();
}


