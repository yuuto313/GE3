#include "MyGame.h"
#include "SceneFactory.h"
#include "TextureManager.h"

void MyGame::Initialize()
{
#pragma region 基盤システムの初期化
	
	//-------------------------------------
	// 基底クラスの初期化
	//-------------------------------------

	OYFramework::Initialize();

	//-------------------------------------
	// テクスチャファイルの読み込み
	//-------------------------------------

	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resource/circle.png");
	TextureManager::GetInstance()->LoadTexture("resource/cube.jpg");
	TextureManager::GetInstance()->LoadTexture("resource/sky.jpg");

	//-------------------------------------
	// 3dモデルの読み込み
	//-------------------------------------

	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("skydome.obj");
	ModelManager::GetInstance()->LoadModel("cube.obj");

	//-------------------------------------
	// キーボード入力の初期化
	//-------------------------------------

	Input::GetInstance()->Initialize(WinApp::GetInstance());

	//-------------------------------------
	// Audioの初期化
	//-------------------------------------

	Audio::GetInstance()->InitXAudio2();

	
#pragma endregion 基盤システムの初期化

#pragma region シーン

	//-------------------------------------
	// シーンの生成
	//-------------------------------------

	// シーンファクトリーを生成し、マネージャにセット
	sceneFactory_ = std::make_unique<SceneFactory>();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());
	// シーンマネージャに最初のシーンをセット
	SceneManager::GetInstance()->ChangeScene("TITLE");


#pragma endregion シーン

}

void MyGame::Finalize()
{
	//-------------------------------------
	// COMの終了処理
	//-------------------------------------

	CoUninitialize();

	//-------------------------------------
	// Audioクラスの後始末
	//-------------------------------------

	Audio::GetInstance()->ResetXAudio2();
	Audio::GetInstance()->Finalize();

	//-------------------------------------
	// Inputクラスの後始末
	//-------------------------------------

	Input::GetInstance()->Finalize();

	//-------------------------------------
	// 基底クラスの終了処理
	//-------------------------------------

	OYFramework::Finalize();

}

void MyGame::Update()
{	
	//-------------------------------------
	// 基底クラスの更新処理
	//-------------------------------------

	OYFramework::Update();

	//-------------------------------------
	// キーボード入力の更新
	//-------------------------------------

	Input::GetInstance()->Update();

}

void MyGame::PreDraw()
{	
	
	//-------------------------------------
	// 基底クラスの描画前処理
	//-------------------------------------

	OYFramework::PreDraw();

}

void MyGame::Draw()
{
	//-------------------------------------
	// シーンマネージャの描画処理
	//-------------------------------------

	SceneManager::GetInstance()->Draw();

}

void MyGame::PostDraw()
{
	//-------------------------------------
	// 基底クラスの描画後処理
	//-------------------------------------

	OYFramework::PostDraw();

}
