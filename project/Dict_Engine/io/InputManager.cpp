#include "InputManager.h"

std::unique_ptr<InputManager> InputManager::instance_ = nullptr;

InputManager* InputManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = std::make_unique<InputManager>(ConstructorKey());
	}
	return instance_.get();
}

void InputManager::Finalize()
{
	instance_.reset();
}

void InputManager::Initialize(WindowsAPI* winAPI)
{
	HRESULT hr;

	// ポインタで渡されたWinAPIのインスタンスを記録

	winAPI_ = winAPI;

	// DirectInputのオブジェクトを生成
	hr = DirectInput8Create(
		winAPI_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの生成
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));
	// 入力データ形式のセット
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);	// 標準形式
	assert(SUCCEEDED(hr));
	// 排他制御レベルのセット
	hr = keyboard_->SetCooperativeLevel(
		winAPI_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	// マウスデバイスの生成
	hr = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(hr));
	// 入力データ形式のセット
	hr = mouse_->SetDataFormat(&c_dfDIMouse);	// 標準形式
	assert(SUCCEEDED(hr));
	// 排他制御レベルのセット
	hr = mouse_->SetCooperativeLevel(
		winAPI_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
}

void InputManager::Update()
{
	HRESULT hr;

	// 前回のキー入力を保存
	memcpy(keyPre_, key_, sizeof(key_));

	// 前回のマウス入力を保存
	memcpy(&mouseStatePre_, &mouseState_, sizeof(mouseState_));

	// 前回のコントローラー状態を保存
	controllerStatePre_ = controllerState_;

	// キーボード情報の取得開始
	hr = keyboard_->Acquire();

	// 全キーの入力状態を取得する
	hr = keyboard_->GetDeviceState(sizeof(key_), key_);

	// マウス情報の取得開始
	hr = mouse_->Acquire();

	// マウスカーソルとマウスボタンの入力状態を取得する
	hr = mouse_->GetDeviceState(sizeof(mouseState_), &mouseState_);

	// コントローラー状態を取得（第一引数はパッドの番号 0〜3）
	DWORD result = XInputGetState(0, &controllerState_);
	isControllerConnected_ = (result == ERROR_SUCCESS);

}

bool InputManager::PushKey(BYTE keyNum)
{
	if (key_[keyNum])
	{
		return true;
	}
	
	return false;
}

bool InputManager::TriggerKey(BYTE keyNum)
{
	if(!keyPre_[keyNum] && key_[keyNum])
	{
		return true;
	}

	return false;
}

bool InputManager::ReleaseKey(BYTE keyNum)
{
	return keyPre_[keyNum] && !key_[keyNum];
}

bool InputManager::PushMouse(BYTE mouseButton)
{
	if (mouseState_.rgbButtons[mouseButton] & 0x80)
	{
		return true;
	}
	return false;
}

bool InputManager::TriggerMouse(BYTE mouseButton)
{
	if (!(mouseStatePre_.rgbButtons[mouseButton] & 0x80) 
		 && (mouseState_.rgbButtons[mouseButton] & 0x80))
	{
		return true;
	}
	return false;
}

POINT InputManager::MousePoint()
{
	// マウスのスクリーン座標を取得する
	POINT point;
	GetCursorPos(&point);

	// スクリーン座標をクライアント座標へ変換する
	ScreenToClient(winAPI_->GetHwnd(), &point);

	return point;
}

LONG InputManager::MouseWheel()
{

	LONG wheelData = mouseState_.lZ;

	return wheelData;
}

bool InputManager::PushButton(WORD button)
{
	if (!isControllerConnected_) return false;
	return (controllerState_.Gamepad.wButtons & button) != 0;
}

bool InputManager::TriggerButton(WORD button)
{
	if (!isControllerConnected_) return false;
	if (!(controllerStatePre_.Gamepad.wButtons & button)
		 && (controllerState_.Gamepad.wButtons & button))
	{
		return true;
	}
	return false;
}

bool InputManager::ReleaseButton(WORD button)
{
	if (!isControllerConnected_) return false;
	return (controllerStatePre_.Gamepad.wButtons & button) != 0 &&
		(controllerState_.Gamepad.wButtons & button) == 0;
}

float InputManager::GetLeftStickX()
{
	SHORT raw = controllerState_.Gamepad.sThumbLX;
	if (abs(raw) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) return 0.0f;
	return raw / 32767.0f;
}

float InputManager::GetLeftStickY()
{
	if (!isControllerConnected_) return false;
	SHORT raw = controllerState_.Gamepad.sThumbLY;
	if (abs(raw) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) return 0.0f;
	return raw / 32767.0f;
}

float InputManager::GetRightStickX()
{
	if (!isControllerConnected_) return false;
	SHORT raw = controllerState_.Gamepad.sThumbRX;
	if (abs(raw) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) return 0.0f;
	return raw / 32767.0f;
}

float InputManager::GetRightStickY()
{
	if (!isControllerConnected_) return false;
	SHORT raw = controllerState_.Gamepad.sThumbRY;
	if (abs(raw) < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) return 0.0f;
	return raw / 32767.0f;
}

float InputManager::GetLeftTrigger()
{
	if (!isControllerConnected_) return false;
	BYTE raw = controllerState_.Gamepad.bLeftTrigger;
	// こちらのデッドゾーン定数は XINPUT_GAMEPAD_TRIGGER_THRESHOLD（= 30）
	if (raw < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) return 0.0f;
	return raw / 255.0f;
}

float InputManager::GetRightTrigger()
{
	if (!isControllerConnected_) return false;
	BYTE raw = controllerState_.Gamepad.bRightTrigger;
	if (raw < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) return 0.0f;
	return raw / 255.0f;
}


