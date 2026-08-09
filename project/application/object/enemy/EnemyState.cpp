#include "EnemyState.h"
#include "Enemy.h"
#include "EnemyCommand.h"
#include "SplineCurve.h"

void EnemyIdleState::Initialize(Enemy* enemy)
{
	moveCommand_ = std::make_unique<EnemyMoveCommand>(
		enemy->GetAIHandler());
	shotCommand_ = std::make_unique<EnemyShotCommand>(
		enemy->GetAIHandler());
	attackCommand_ = std::make_unique<EnemyAttackCommand>(
		enemy->GetAIHandler());
}

void EnemyIdleState::Update(Enemy* enemy, const float& deltaTime)
{
	AIHandler* handler = enemy->GetAIHandler();

	moveCommand_->Execute(enemy);
	
	if (handler->IsAttack(deltaTime))
	{
		attackCommand_->Execute(enemy);
	}
	else if (handler->IsShot(deltaTime))
	{
		shotCommand_->Execute(enemy);
	}
}

void EnemyIdleState::Draw(Enemy * enemy)
{
	(void)enemy;
}

void EnemyIdleState::Finalize(Enemy * enemy)
{
	(void)enemy;
}


void EnemyMoveState::Initialize(Enemy* enemy)
{
	(void)enemy;
}

void EnemyMoveState::Update(Enemy * enemy, const float& deltaTime)
{
	(void)enemy;
}

void EnemyMoveState::Draw(Enemy * enemy)
{
	(void)enemy;
}

void EnemyMoveState::Finalize(Enemy * enemy)
{
	(void)enemy;
}



void EnemyShotState::Initialize(Enemy* enemy)
{
	moveCommand_ = std::make_unique<EnemyMoveCommand>(
		enemy->GetAIHandler());
}

void EnemyShotState::Update(Enemy * enemy, const float& deltaTime)
{
	moveCommand_->Execute(enemy);

	timer_ += deltaTime;
	if (timer_ >= duration_)
	{
		enemy->ChangeState(std::make_unique<EnemyIdleState>());
	}
}

void EnemyShotState::Draw(Enemy * enemy)
{
	(void)enemy;
}

void EnemyShotState::Finalize(Enemy * enemy)
{
	(void)enemy;
}

void EnemyAttackState::Initialize(Enemy* enemy)
{
	attackPhase_ = AttackPhase::Approach;

	approachPosition_ = { 0.0f, 0.0f, 20.0f };
	beforePosition_ = enemy->GetTranslate();
}

void EnemyAttackState::Update(Enemy * enemy, const float& deltaTime)
{
	timer_ += deltaTime;
	switch (attackPhase_)
	{
	case EnemyAttackState::AttackPhase::Approach:

		enemy->SetTranslate(Lerp(beforePosition_, approachPosition_, timer_ / duration_));

		if (timer_ >= duration_)
		{
			attackPhase_ = AttackPhase::Homing;
			timer_ = 0.0f;
			duration_ = 0.5f;
			handPosition_ = enemy->GetRightHandTransform().translate;
		}
		break;

	case EnemyAttackState::AttackPhase::Homing:

		enemy->SetTranslate({ enemy->GetPlayerWorldPosition().x, 0.0f, 20.0f });
		enemy->SetRightHandTranslate(Lerp(handPosition_, {0.0f, 5.0f, 0.0f}, timer_ / duration_));

		if (timer_ >= duration_)
		{
			attackPhase_ = AttackPhase::Attack;
			timer_ = 0.0f;
			duration_ = 0.4f;
			homingPosition_ = enemy->GetTranslate();
			handPosition_ = enemy->GetRightHandTransform().translate;
			enemy->SetAttackColliderActive(true);
		}
		break;

	case EnemyAttackState::AttackPhase::Attack:

		enemy->SetRightHandTranslate(Lerp(handPosition_, {0.0f, -5.0f, 0.0f}, timer_ / duration_));

		if (timer_ >= duration_)
		{
			attackPhase_ = AttackPhase::Away;
			timer_ = 0.0f;
			duration_ = 1.0f;
			approachPosition_ = enemy->GetTranslate();
			handPosition_ = enemy->GetRightHandTransform().translate;
		}
		break;

	case EnemyAttackState::AttackPhase::Away:

		enemy->SetTranslate(Lerp(approachPosition_, beforePosition_, timer_ / duration_));
		enemy->SetRightHandTranslate(Lerp(handPosition_, { -2.5f, 0.0f, 0.0f }, timer_ / duration_));

		if (timer_ >= duration_)
		{
			enemy->ChangeState(std::make_unique<EnemyIdleState>());
		}
		break;

	default:
		break;
	}
}

void EnemyAttackState::Draw(Enemy * enemy)
{
	(void)enemy;
}

void EnemyAttackState::Finalize(Enemy * enemy)
{
	(void)enemy;
}
