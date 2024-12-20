#include "Input.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"xinput.lib")

Input* Input::instance = nullptr;

Input* Input::GetInstance()
{
	if (instance == nullptr) {
		instance = new Input;
	}

	return instance;
}

void Input::Initialize(WinApp* winApp)
{
	HRESULT result;

	result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイス生成
	result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard_->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

}

void Input::Finalize()
{
	delete instance;
	instance = nullptr;
}

void Input::Update()
{
	// キーボード

	// 前回のキー入力を保存
	memcpy(keyPre_, key_, sizeof(key_));

	// キーボード情報の取得開始
	keyboard_->Acquire();
	// 全キーの入力情報を取得する
	keyboard_->GetDeviceState(sizeof(key_), key_);


	// コントローラー
	// 前回の状態を保存
	statePre_ = state_;
	// コントローラーの状態を取得
	DWORD result = XInputGetState(0, &state_);
	if (result != ERROR_SUCCESS) {
		// コントローラーが接続されていない場合はゼロ初期化
		ZeroMemory(&state_, sizeof(XINPUT_STATE));
	}
}

bool Input::PushKey(BYTE keyNumber) const
{
	// 指定キーを押していればtrueを返す
	if (key_[keyNumber]) {
		return true;
	} else {
		return false;
	}
}

bool Input::TriggerKey(BYTE keyNumber) const
{
	if (!keyPre_[keyNumber] && key_[keyNumber]) {
		return true;
	} else {
		return false;
	}
}

bool Input::GetJoystickState(int32_t stickNo, XINPUT_STATE& out) const
{
	// 今回はPlayer0だけ
	if (stickNo == 0) {
		out = state_;
		return true;
	}
	return false;
}

bool Input::GetJoystickStatePrevious(int32_t stickNo, XINPUT_STATE& out) const
{
	if (stickNo == 0) {
		out = statePre_;
		return true;
	}
	return false;
}
