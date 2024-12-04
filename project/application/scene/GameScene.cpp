#include "GameScene.h"
#include "SceneManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"

void GameScene::Initialize()
{
	//-------------------------------------
	// テクスチャファイルの読み込み
	//-------------------------------------

	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resource/monsterBall.png");
	TextureManager::GetInstance()->LoadTexture("resource/eto_tora_family.png");
	TextureManager::GetInstance()->LoadTexture("resource/circle.png");
	
	//-------------------------------------
	// 3dモデルの読み込み
	//-------------------------------------
	
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("axis.obj");

	//-------------------------------------
	// カメラの初期化
	//-------------------------------------

	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({ 0.3f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,4.0f,-10.0f });

	//-------------------------------------
	// パーティクルマネージャ生成
	//-------------------------------------

	ParticleManager::GetInstance()->SetCamera(camera_.get());
	ParticleManager::GetInstance()->SetModel("plane.obj");
	// パーティクルグループ生成
	ParticleManager::GetInstance()->CreateParticleGroup("Particle", "resource/circle.png");

	//-------------------------------------
	// パーティクルエミッタ生成
	//-------------------------------------

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize("Particle", {}, 10);


}

void GameScene::Finalize()
{
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
	// カメラの更新
	//-------------------------------------

	// 3dオブジェクトの更新より前に行う
	camera_->Update();


	//-------------------------------------
	// パーティクルエミッターの更新
	//-------------------------------------

	particleEmitter_->Update();

}

void GameScene::ImGui()
{
}

void GameScene::Draw()
{
	particleEmitter_->Draw();
}


