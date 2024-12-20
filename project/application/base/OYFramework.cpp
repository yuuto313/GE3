#include "OYFramework.h"

void OYFramework::Initialize()
{
	//-------------------------------------
	// リークチェッカー
	//-------------------------------------

	pLeakChecke_->~D3DResourceLeakChecker();

	//-------------------------------------
	// WinAppAPIの初期化
	//-------------------------------------

	pWinApp_ = WinApp::GetInstance();
	pWinApp_->Initialize();

	//-------------------------------------
	// DirectXの初期化
	//-------------------------------------

	pDxCommon_ = DirectXCommon::GetInstance();
	pDxCommon_->Initialize(pWinApp_);

	//-------------------------------------
	// SRVマネージャの初期化
	//-------------------------------------

	SrvManager::GetInstance()->Initialize(pDxCommon_);
	
	//-------------------------------------
	// テクスチャマネージャの初期化
	//-------------------------------------

	TextureManager::GetInstance()->Initialize(pDxCommon_);

	//-------------------------------------
	// スプライト共通部の初期化
	//-------------------------------------

	SpriteCommon::GetInstance()->Initialize(pDxCommon_);
	
	//-------------------------------------
	// 3dオブジェクト共通部の初期化
	//-------------------------------------

	Object3dCommon::GetInstance()->Initialize(pDxCommon_);

	//-------------------------------------
	// 3dモデルマネージャの初期化
	//-------------------------------------

	ModelManager::GetInstance()->Initialize(pDxCommon_);
	
	//-------------------------------------
	// パーティクルマネージャの初期化
	//-------------------------------------

	ParticleManager::GetInstance()->Initialize(DirectXCommon::GetInstance(), SrvManager::GetInstance());

	//-------------------------------------
	// ImGui（デバッグテキスト）の初期化
	//-------------------------------------

	ImGuiManager::GetInstance()->Initialize(pDxCommon_, pWinApp_);
	
	//-------------------------------------
	// シーンマネージャの生成
	//-------------------------------------

	SceneManager::GetInstance();

}

void OYFramework::Finalize()
{
	//-------------------------------------
	// シーンマネージャの終了処理
	//-------------------------------------

	SceneManager::GetInstance()->Finalize();

	//-------------------------------------
	// ImGuiの終了処理
	//-------------------------------------

	ImGuiManager::GetInstance()->Finalize();

	//-------------------------------------
	// パーティクルマネージャの終了処理
	//-------------------------------------	

	ParticleManager::GetInstance()->Finalize();

	//-------------------------------------
	// 3dモデルマネージャの終了処理
	//-------------------------------------

	ModelManager::GetInstance()->Finalize();

	//-------------------------------------
	// 3dオブジェクト共通部の終了処理
	//-------------------------------------

	Object3dCommon::GetInstance()->Finalize();

	//-------------------------------------
	// スプライト共通部の終了処理
	//-------------------------------------

	SpriteCommon::GetInstance()->Finalize();

	//-------------------------------------
	// テクスチャマネージャの終了処理
	//-------------------------------------

	TextureManager::GetInstance()->Finalize();

	//-------------------------------------
	// SRVマネージャの終了処理
	//-------------------------------------

	SrvManager::GetInstance()->Finalize();

	//-------------------------------------
	// DirectXCommonの終了処理
	//-------------------------------------

	pDxCommon_->Finalize();

	//-------------------------------------
	// WindowsAPIの終了処理
	//-------------------------------------

	pWinApp_->Finalize();

}

void OYFramework::Update()
{	
	//-------------------------------------
	// 終了リクエストを確認
	//-------------------------------------

	if (pWinApp_->ProcessMessage()) {
		endRequest_ = true;
	}

	//-------------------------------------
	// フレームの始まる旨を告げる
	//-------------------------------------

	ImGuiManager::GetInstance()->Begin();

	//-------------------------------------
	// ImGui（デバッグテキスト）の更新
	//-------------------------------------

	SpriteCommon::GetInstance()->ImGui();

	ImGuiManager::GetInstance()->UpdateGameUI();

	//-------------------------------------
	// シーンマネージャの更新
	//-------------------------------------

	SceneManager::GetInstance()->Update();


}

void OYFramework::PreDraw()
{
	//-------------------------------------
	// 描画前処理
	//-------------------------------------

	// DirectXの描画準備。すべての描画に共通のグラフィックスコマンドを積む
	pDxCommon_->PreDraw();

	// DescriptorHeapを設定
	SrvManager::GetInstance()->PreDraw();

	//-------------------------------------
	// スプライト描画準備
	//-------------------------------------

	// Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->PreDraw();

	//-------------------------------------
	// 3dオブジェクト描画準備
	//-------------------------------------

	// 3dオブジェクトの描画準備。3dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3dCommon::GetInstance()->PreDraw();

}

void OYFramework::PostDraw()
{
	//-------------------------------------
	// ゲームの処理が終わり描画処理に入る前に、ImGuiの内部コマンドを生成する
	//-------------------------------------

	ImGuiManager::GetInstance()->End();

	//-------------------------------------
	// 画面表示できるようにする
	//-------------------------------------

	ImGuiManager::GetInstance()->Draw();

	//-------------------------------------
	// 描画後処理
	//-------------------------------------

	pDxCommon_->PostDraw();

}

void OYFramework::Run()
{

	//-------------------------------------
	// ゲームの初期化
	//-------------------------------------

	Initialize();

	//-------------------------------------
	// ゲームループ
	//-------------------------------------

	while (true)
	{
		//-------------------------------------
		// 毎フレーム更新処理
		//-------------------------------------

		Update();

		//-------------------------------------
		// 終了リクエストが来たら抜ける
		//-------------------------------------

		if (IsEndRequest()) {
			break;
		}

		//-------------------------------------
		// 描画前処理
		//-------------------------------------

		PreDraw();

		//-------------------------------------
		// 描画
		//-------------------------------------

		Draw();

		//-------------------------------------
		// 描画後処理
		//-------------------------------------

		PostDraw();

	}

	//-------------------------------------
	// ゲームの終了
	//-------------------------------------

	Finalize();

}
