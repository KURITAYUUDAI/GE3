#pragma once
#include "myMath.h"

class Player;

class EnemyManager;

class IInputHandler;

class IPlayerCommand
{
public:

	virtual ~IPlayerCommand() = default;
	virtual void Execute(Player* player) = 0;
};

class MoveHorizontalCommand : public IPlayerCommand
{
public:
	MoveHorizontalCommand(IInputHandler* inputHandler) : inputHandler_(inputHandler) {}
	void Execute(Player* player) override;

private:
	IInputHandler* inputHandler_;
};

class ShotCommand : public IPlayerCommand
{
public:
	ShotCommand(IInputHandler* inputHandler) : inputHandler_(inputHandler) {}
	void Execute(Player* player) override;

private:
	IInputHandler* inputHandler_;
};

class AvoidCommand : public IPlayerCommand
{
public:
	AvoidCommand(IInputHandler* inputHandler) : inputHandler_(inputHandler) {}
	void Execute(Player* player) override;

private:
	IInputHandler* inputHandler_;
};

class LongPressShotCommand
{
public:
	enum class Result
	{
		None,
		NormalShot,
		ChargedShot,
	};

	LongPressShotCommand(IInputHandler* inputHandler) : inputHandler_(inputHandler) {}
	Result Update(Player* player, float deltaTime);
	void Cancel(Player* player);
	bool IsCharging() const { return isCharging_; }

private:
	IInputHandler* inputHandler_ = nullptr;
	float holdTime_ = 0.0f;
	bool isCharging_ = false;
	bool isChargeEffectVisible_ = false;
	static inline constexpr float kChargeDuration = 0.7f;
};

class MeleeAttackCommand : public IPlayerCommand
{
public:
	MeleeAttackCommand(IInputHandler* inputHandler) : inputHandler_(inputHandler) {}
	void Execute(Player* player) override;

private:
	IInputHandler* inputHandler_;
};
