#include "WinApp.h"
#include "externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


//-------------------------------------
//ウィンドウプロシージャ
//-------------------------------------
LRESULT CALLBACK WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//ImGuiにメッセージを渡し、さらにImGuiが処理していたら後続の処理を打ち切る 
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


void WinApp::Initialize()
{
	//-------------------------------------
	//COM（Component　Object　Model）の初期化
	//-------------------------------------

	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	//-------------------------------------
	//ウィンドウプロシージャ
	//-------------------------------------
	
	wc_.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	wc_.lpszClassName = L"GE3";
	//インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc_);


	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//-------------------------------------
	// ウィンドウの生成
	//-------------------------------------

	hwnd_ = CreateWindow(
		wc_.lpszClassName,               //引用するクラス名
		L"GE3",                         //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,            //よく見るウィンドウスタイル
		CW_USEDEFAULT,                  //表示X座標（Windowsに任せる）
		CW_USEDEFAULT,                  //表示Y座標（WindowsOSに任せる）
		wrc.right - wrc.left,           //ウィンドウ横幅
		wrc.bottom - wrc.top,           //ウィンドウ縦幅
		nullptr,                        //親ウィンドウハンドル
		nullptr,                        //メニューハンドル
		wc_.hInstance,                   //インスタンスハンドル
		nullptr                         //オプション
	);

	ShowWindow(hwnd_, SW_SHOW);

}

void WinApp::Update()
{

}

void WinApp::Finalize()
{
	CloseWindow(hwnd_);
	CoUninitialize();
}
