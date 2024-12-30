#pragma once
#include <wrl.h>
#include <cassert>
#include <array>
#include <windows.h>
#include <Xinput.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "WinApp.h"

/// <summary>
/// 入力
/// </summary>
class Input
{
public:// メンバ関数

	//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	/// <summary>
	///	シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Input* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber"></param>
	/// <returns></returns>
	bool PushKey(BYTE keyNumber) const;

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber"></param>
	/// <returns></returns>
	bool TriggerKey(BYTE keyNumber) const;

	/// <summary>
	/// 現在のジョイスティック状態を取得する
	/// </summary>
	/// <param name="stickNo"></param>
	/// <param name="out"></param>
	/// <returns></returns>
	bool GetJoystickState(int32_t stickNo, XINPUT_STATE& out) const;

	/// <summary>
	/// 前回のジョイスティック状態を取得する
	/// </summary>
	/// <param name="stickNo"></param>
	/// <param name="out"></param>
	/// <returns></returns>
	bool GetJoystickStatePrevious(int32_t stickNo, XINPUT_STATE& out) const;

private:// メンバ変数
	//DirectInputのインスタンス生成
	ComPtr<IDirectInput8> directInput_;

	//キーボードデバイス生成
	ComPtr<IDirectInputDevice8> keyboard_;

	//全キーの状態
	BYTE key_[256] = {};

	//前回の全キーの状態
	BYTE keyPre_[256] = {};

	// コントローラー
	// XInputで扱う状態
	XINPUT_STATE state_;
	XINPUT_STATE statePre_;

private:// シングルトン設計

	static Input* instance;

	/// <summary>
	/// コンストラクタ、デストラクタの隠蔽
	/// </summary>
	Input() = default;
	~Input() = default;

	/// <summary>
	/// コピーコンストラクタの封印
	/// </summary>
	/// <param name=""></param>
	Input(Input&) = delete;

	/// <summary>
	/// コピー代入演算の封印
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	Input& operator=(Input&) = delete;
};

