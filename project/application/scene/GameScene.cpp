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

	// 敵本体
	enemyObj_ = std::make_unique<Object3d>();
	enemyObj_->Initialize(camera_.get(), "cube.obj");
	// 弾
	enemyBullet_ = std::make_unique<Object3d>();
	enemyBullet_->Initialize(camera_.get(), "cube.obj");

	std::vector<Object3d*> enemyModels = { enemyObj_.get(),enemyBullet_.get() };

	// プレイヤー本体
	playerObj_ = std::make_unique<Object3d>();
	playerObj_->Initialize(camera_.get(), "cube.obj");
	// 弾
	playerBullet_ = std::make_unique<Object3d>();
	playerBullet_->Initialize(camera_.get(), "cube.obj");
	// レティクル
	playerReticle_ = std::make_unique<Object3d>();
	playerReticle_->Initialize(camera_.get(), "cube.obj");

	std::vector<Object3d*> playerModels = { playerObj_.get(),playerBullet_.get(),playerReticle_.get() };

	//-------------------------------------
	// 天球の生成
	//-------------------------------------

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(skydomeObj_.get());

	//-------------------------------------
	// プレイヤーの生成
	//-------------------------------------

	player_ = std::make_unique<Player>();
	player_->Initialize(playerModels);

	//-------------------------------------
	// エネミーの生成
	//-------------------------------------

	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize(enemyModels);
	enemy_->SetPlayer(player_.get());

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
		enemy_.reset();

		player_.reset();

		skydome_.reset();

		enemyObj_.reset();
		enemyBullet_ .reset();
		playerObj_ .reset();
		playerBullet_.reset();
		playerReticle_.reset();
	
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
	// プレイヤーの更新
	//-------------------------------------

	enemy_->Update();

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

	enemy_->Draw();

	particleEmitter_->Draw();
}


