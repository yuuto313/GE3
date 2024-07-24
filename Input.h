#pragma once
#include <wrl.h>
#include <cassert>
#include <windows.h>

/// <summary>
/// 入力
/// </summary>
class Input
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(HINSTANCE hInstance,HWND hwnd);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();
};

