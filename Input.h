#pragma once
#include <wrl.h>
#include <cassert>
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>


/// <summary>
/// 入力
/// </summary>
class Input
{
public:
	//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(HINSTANCE hInstance,HWND hwnd);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();


private:
	//DirectInputのインスタンス生成
	ComPtr<IDirectInput8> directInput;

	//キーボードデバイス生成
	ComPtr<IDirectInputDevice8> keyboard;

};

