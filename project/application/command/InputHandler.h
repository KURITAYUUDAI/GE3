#pragma once
#include "myMath.h"
#include "InputManager.h"
#include <cmath>
#include <string>
#include <vector>
#include <unordered_map>

class IInputHandler
{
	public:
	virtual ~IInputHandler() = default;
	virtual Vector2 GetDirection() = 0;
    virtual bool IsActionPressed(const std::string& actionName) { return false; }
	virtual bool IsActionTriggerd(const std::string& actionName) { return false; }
	virtual bool IsActionReleased(const std::string& actionName) { return false; }
};

class KeyboardInputHandler : public IInputHandler
{
public:
    void AssignKey(const std::string& actionName, int keyCode);
    Vector2 GetDirection() override;
    bool IsActionPressed(const std::string& actionName) override;
	bool IsActionTriggerd(const std::string& actionName) override;
	bool IsActionReleased(const std::string& actionName) override;

private:

    std::unordered_map<std::string, int> keyBindings_;
};

class GamepadInputHandler : public IInputHandler
{
public:
    explicit GamepadInputHandler(float deadzone = 0.2f)
        : deadzone_(deadzone) {}

    void AssignKey(const std::string& actionName, int buttonCode);
    Vector2 GetDirection() override;
    bool IsActionPressed(const std::string& actionName) override;
	bool IsActionTriggerd(const std::string& actionName) override;
	bool IsActionReleased(const std::string& actionName) override;

private:
    std::unordered_map<std::string, int> buttonBindings_;
    float deadzone_;
};
