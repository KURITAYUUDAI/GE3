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

class MeleeAttackCommand : public IPlayerCommand
{
public:
	MeleeAttackCommand(IInputHandler* inputHandler) : inputHandler_(inputHandler) {}
	void Execute(Player* player) override;

private:
	IInputHandler* inputHandler_;
};
