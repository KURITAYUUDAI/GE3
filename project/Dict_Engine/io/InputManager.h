#pragma once
#include <Windows.h>
#include <cstdint>
#include <cassert>
#include <wrl.h>

#include "WindowsAPI.h"

#define DIRECTINPUT_VERSION		0x0800	// DirectInputのバージョンを指定
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <memory>

class InputManager
{
public:
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	// シングルトンインスタンスの取得
	static InputManager* GetInstance();
	// 終了
	void Finalize();

	// コンストラクタに渡すための鍵
	class ConstructorKey
	{
	private:
		ConstructorKey() = default;
		friend class InputManager;
	};

	// PassKeyを受け取るコンストラクタ
	explicit InputManager(ConstructorKey){}

private: 	// シングルトンインスタンス

	static std::unique_ptr<InputManager> instance_;

	~InputManager() = default;
	InputManager(InputManager&) = delete;
	InputManager& operator=(InputManager&) = delete;

	friend struct std::default_delete<InputManager>;

public:

	void Initialize(WindowsAPI* winAPI);

	void Update();

	bool PushKey(BYTE keyNum);
	bool TriggerKey(BYTE keyNum);
	bool ReleaseKey(BYTE keyNum);

	POINT MousePoint();
	bool PushMouse(BYTE mouseButton);
	bool TriggerMouse(BYTE mouseButton);
	LONG MouseWheel();

	// コントローラー
	bool IsControllerConnected(){ return isControllerConnected_; }
	bool PushButton(WORD button);    // 例: XINPUT_GAMEPAD_A
	bool TriggerButton(WORD button);
	bool ReleaseButton(WORD button);
	float GetLeftStickX();
	float GetLeftStickY();
	float GetRightStickX();
	float GetRightStickY();
	float GetLeftTrigger();
	float GetRightTrigger();

private:

	// WindowsAPI
	WindowsAPI* winAPI_ = nullptr;

	// DirecctInputの初期化
	ComPtr<IDirectInput8> directInput_ = nullptr;

	ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	BYTE key_[256] = { 0 };	// キーの状態。256キー分の状態を保持する
	BYTE keyPre_[256] = { 0 };

	ComPtr<IDirectInputDevice8> mouse_ = nullptr;
	DIMOUSESTATE mouseState_ = { 0 };
	DIMOUSESTATE mouseStatePre_ = { 0 };

	// コントローラー
	XINPUT_STATE controllerState_ = { 0 };
	XINPUT_STATE controllerStatePre_ = { 0 };
	bool isControllerConnected_ = false;

};



