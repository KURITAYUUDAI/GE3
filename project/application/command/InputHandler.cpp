#include "InputHandler.h"

void KeyboardInputHandler::AssignKey(const std::string& actionName, int keyCode)
{
    keyBindings_[actionName] = keyCode;
}

Vector2 KeyboardInputHandler::GetDirection()
{
    float x = 0.0f, y = 0.0f;
    auto* input = InputManager::GetInstance();

    if (input->PushKey(DIK_A)) x -= 1.0f;
    if (input->PushKey(DIK_D)) x += 1.0f;
    if (input->PushKey(DIK_W)) y += 1.0f;
    if (input->PushKey(DIK_S)) y -= 1.0f;

    float magnitude = std::sqrt(x * x + y * y);
    if (magnitude > 1.0f)
    {
        x /= magnitude;
        y /= magnitude;
    }

    return { x, y };
}

bool KeyboardInputHandler::IsActionPressed(const std::string& actionName)
{
    auto it = keyBindings_.find(actionName);
    if (it == keyBindings_.end()) return false;
    return InputManager::GetInstance()->PushKey(it->second);
}

bool KeyboardInputHandler::IsActionTriggerd(const std::string& actionName)
{
    auto it = keyBindings_.find(actionName);
    if (it == keyBindings_.end()) return false;
    return InputManager::GetInstance()->TriggerKey(it->second);
}

bool KeyboardInputHandler::IsActionReleased(const std::string& actionName)
{
    auto it = keyBindings_.find(actionName);
    if (it == keyBindings_.end()) return false;
    return InputManager::GetInstance()->ReleaseKey(it->second);
}

void GamepadInputHandler::AssignKey(const std::string& actionName, int buttonCode)
{
    buttonBindings_[actionName] = buttonCode;
}

Vector2 GamepadInputHandler::GetDirection()
{
    auto* input = InputManager::GetInstance();
    float x = input->GetLeftStickX();
    float y = input->GetLeftStickY();

    float magnitude = std::sqrt(x * x + y * y);
    if (magnitude < deadzone_) return { 0.0f, 0.0f };

    float scale = (magnitude - deadzone_) / (1.0f - deadzone_);
    return { (x / magnitude) * scale, (y / magnitude) * scale };
}

bool GamepadInputHandler::IsActionPressed(const std::string& actionName)
{
    auto it = buttonBindings_.find(actionName);
    if (it == buttonBindings_.end()) return false;
    return InputManager::GetInstance()->PushButton(it->second);
}

bool GamepadInputHandler::IsActionTriggerd(const std::string& actionName)
{
    auto it = buttonBindings_.find(actionName);
    if (it == buttonBindings_.end()) return false;
    return InputManager::GetInstance()->TriggerButton(it->second);
}

bool GamepadInputHandler::IsActionReleased(const std::string& actionName)
{
    auto it = buttonBindings_.find(actionName);
    if (it == buttonBindings_.end()) return false;
    return InputManager::GetInstance()->ReleaseButton(it->second);
}




