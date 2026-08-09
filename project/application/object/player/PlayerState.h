#pragma once
#include "myMath.h"
#include <memory>
#include "InputHandlerSelector.h"
#include "GameSceneCommand.h"
#include "JustAvoidDarken.h"

class Player;

enum class PlayerStateType
{
	Idle,
	Shot,
	Avoid,
	JustAvoid,
	MeleeAttack,
};

class IPlayerState
{
public:
	virtual PlayerStateType GetType() const = 0;

	virtual void Initialize(Player* player) = 0;
	virtual void Update(Player* player, const float& deltaTime) = 0;
	virtual void Draw(Player* player) = 0;
	virtual void Finalize(Player* player) = 0;
};

class PlayerIdleState : public IPlayerState
{
public:
	PlayerStateType GetType() const override
	{
		return PlayerStateType::Idle;
	}

	void Initialize(Player* player) override;
	void Update(Player* player, const float& deltaTime) override;
	void Draw(Player* player) override;
	void Finalize(Player* player) override;

private:

	std::unique_ptr<IPlayerCommand>      moveCommand_;
	std::unique_ptr<IPlayerCommand>      shotCommand_;
	std::unique_ptr<IPlayerCommand>      avoidCommand_;
	std::unique_ptr<IPlayerCommand>      meleeAttackCommand_;
};

class PlayerMeleeAttackState : public IPlayerState
{
private:
	enum class AttackPhase
	{
		Approach,
		Windup,
		Attack,
		Recovery,
	};

public:
	PlayerStateType GetType() const override { return PlayerStateType::MeleeAttack; }
	void Initialize(Player* player) override;
	void Update(Player* player, const float& deltaTime) override;
	void Draw(Player* player) override;
	void Finalize(Player* player) override;

private:
	AttackPhase phase_ = AttackPhase::Approach;
	float timer_ = 0.0f;
	Vector3 startPosition_{};
	Vector3 approachPosition_{};
	Vector3 attackDirection_{ 0.0f, 0.0f, 1.0f };
};

class PlayerShotState : public IPlayerState
{
public:
	PlayerStateType GetType() const override
	{
		return PlayerStateType::Shot;
	}

	void Initialize(Player* player) override;
	void Update(Player* player, const float& deltaTime) override;
	void Draw(Player* player) override;
	void Finalize(Player* player) override;

private:
	float timer_ = 0.0f;
	float duration_ = 0.1f;

	std::unique_ptr<IPlayerCommand>      moveCommand_;
};

class PlayerAvoidState : public IPlayerState
{
public:

	PlayerAvoidState(const Vector2& direction) : inputDirection_(direction) {}

	PlayerStateType GetType() const override
	{
		return PlayerStateType::Avoid;
	}
	
	void Initialize(Player* player) override;
	void Update(Player* player, const float& deltaTime) override;
	void Draw(Player* player) override;
	void Finalize(Player* player) override;

private:
	float timer_ = 0.0f;
	float duration_ = 0.5f;
	float justDuration_ = 0.3f;

	Vector3 avoidDirection_ = { 0.0f, 0.0f, 0.0f };
	Vector2 inputDirection_ = { 0.0f, 0.0f };
	float avoidSpeed_ = 25.0f; // 回避の速度

};

class PlayerJustAvoidState : public IPlayerState
{
public:
	PlayerJustAvoidState(
		const Vector3& direction, JustAvoidDarken* justAvoidDarken) 
		: avoidDirection_(direction), justAvoidDarken_(justAvoidDarken) {}

	PlayerStateType GetType() const override
	{
		return PlayerStateType::JustAvoid;
	}

	void Initialize(Player* player) override;
	void Update(Player* player, const float& deltaTime) override;
	void Draw(Player* player) override;
	void Finalize(Player* player) override;

private:
	float timer_ = 0.0f;
	float duration_ = 0.5f;
	float waitDuration_ = 1.0f;

	Vector3 avoidDirection_ = { 0.0f, 0.0f, 0.0f };
	float avoidSpeed_ = 15.0f; // 回避の速度

	bool isCounter_ = false;

	std::unique_ptr<IPlayerCommand> shotCommand_;

	JustAvoidDarken* justAvoidDarken_;

	JustAvoidDarken::Setting setting
	{
		.duration = 1.0f,
		.maxIntensity = 0.55f,
		.attackRate = 0.05f,
		.returnRate = 0.05f,
		.darkColor = { 0.0f, 0.0f, 0.0f, 1.0f }
	};
};
