#include "PlayerState.h"
#include "Player.h"

void PlayerIdleState::Initialize(Player* player)
{
	moveCommand_ = std::make_unique<MoveHorizontalCommand>(
		player->GetInputHandlerSelector()->GetHandler());
	shotCommand_ = std::make_unique<ShotCommand>(
		player->GetInputHandlerSelector()->GetHandler());
	avoidCommand_ = std::make_unique<AvoidCommand>(
		player->GetInputHandlerSelector()->GetHandler());
	meleeAttackCommand_ = std::make_unique<MeleeAttackCommand>(
		player->GetInputHandlerSelector()->GetHandler());
}

void PlayerIdleState::Update(Player * player, const float& deltaTime)
{
	IInputHandler* handler = player->GetInputHandlerSelector()->GetHandler();

	moveCommand_->Execute(player);
	
	if (handler->IsActionTriggerd("shot"))
	{
		shotCommand_->Execute(player);
		return; // ★ 追加: Shot()内部でChangeStateされる
	}
	if (handler->IsActionTriggerd("avoid"))
	{
		avoidCommand_->Execute(player);
		return; // ★ 追加: Avoid()内部でChangeStateされる
	}
	if (handler->IsActionTriggerd("melee"))
	{
		meleeAttackCommand_->Execute(player);
		return;
	}
}

void PlayerIdleState::Draw(Player* player)
{
	(void)player;
}

void PlayerIdleState::Finalize(Player* player)
{
	(void)player;
}

void PlayerShotState::Initialize(Player* player)
{
	moveCommand_ = std::make_unique<MoveHorizontalCommand>(
		player->GetInputHandlerSelector()->GetHandler());
}

void PlayerShotState::Update(Player* player, const float& deltaTime)
{
	IInputHandler* handler = player->GetInputHandlerSelector()->GetHandler();

	moveCommand_->Execute(player);

	timer_ += deltaTime;
	if (timer_ >= duration_)
	{
		/*player->ClearLockOn();*/
		player->ChangeState(std::make_unique<PlayerIdleState>());
	}
}

void PlayerShotState::Draw(Player* player)
{
	(void)player;
}

void PlayerShotState::Finalize(Player* player)
{
	(void)player;
}

void PlayerAvoidState::Initialize(Player* player)
{
	if (inputDirection_.x != 0.0f && inputDirection_.y != 0.0f)
	{
		Vector3 currentVelocity = player->GetVelocity();
		if (currentVelocity.x != 0.0f || currentVelocity.y != 0.0f)
		{
			// 入力方向に回避速度を加算
			avoidDirection_ = currentVelocity;
		}
		else
		{
			avoidDirection_ = {1.0f, 0.0f, 0.0f};
		}
	}
	else
	{
		avoidDirection_ = { inputDirection_.x, inputDirection_.y, 0.0f };
	}

	if (inputDirection_.x != 0.0f || inputDirection_.y != 0.0f)
	{
		avoidDirection_ = Normalize(avoidDirection_);
	}
	else
	{
		avoidDirection_ = Normalize(Vector3{1.0f, 0.0f, 0.0f});
	}

	//float maxAvoidRoll = 2.0f * pi; // 最大で傾ける角度（ラジアン。0.5f は約30度）

	//if (avoidDirection_.x < 0.0f) 
	//{
	//	player->SetTargetRoll({ 0.0f, 0.0f, maxAvoidRoll});  // 左回避時のロール
	//} 
	//else // if (avoidDirection_.x > 0.0f) 
	//{
	//	player->SetTargetRoll({0.0f, 0.0f, -maxAvoidRoll}); // 右回避時のロール
	//}
	
	player->SetIsHit(false);

	player->SetAvoidDirection(avoidDirection_);
	player->SetJustAvoidAccept(true);
}

void PlayerAvoidState::Update(Player* player, const float& deltaTime)
{
	IInputHandler* handler = player->GetInputHandlerSelector()->GetHandler();

	timer_ += deltaTime;

	float progress = timer_ / duration_;
	if (progress > 1.0f) progress = 1.0f;

	float oneRotation = 2.0f * pi;
	float currentRoll = 0.0f;

	if (avoidDirection_.x < 0.0f) 
	{
		currentRoll = progress * oneRotation; // 左回転
	}
	else 
	{
		currentRoll = -progress * oneRotation;  // 右回転
	}

	Vector3 currentRotate = player->GetRotate();
	currentRotate.z = currentRoll;
	player->SetRotate(currentRotate);

	float speedRate = 1.0f - (progress * progress);

	float currentSpeed = avoidSpeed_ * speedRate;

	player->MoveAvoid(avoidDirection_, currentSpeed);

	/*if (timer_ / duration_ > 0.8f)
	{
		player->SetTargetRoll({0.0f, 0.0f, 0.0f});
	}*/
	
	if (player->GetJustAvoidAccept())
	{
		if (timer_ >= justDuration_)
		{
			player->SetJustAvoidAccept(false);
		}
	}

	if (timer_ >= duration_)
	{
		player->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
		
		/*player->ClearLockOn();*/
		player->ChangeState(std::make_unique<PlayerIdleState>());
		
	}
}

void PlayerAvoidState::Draw(Player* player)
{
	(void)player;
}

void PlayerAvoidState::Finalize(Player * player)
{
	/*player->SetTargetRoll({0.0f, 0.0f, 0.0f});*/
	
	player->SetIsHit(true);
}

void PlayerJustAvoidState::Initialize(Player* player)
{
	shotCommand_ = std::make_unique<ShotCommand>(
		player->GetInputHandlerSelector()->GetHandler());

	justAvoidDarken_->SetSetting(setting);
	justAvoidDarken_->Play();

	player->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
	player->SetIsHit(false);
	player->SetJustAvoidAccept(false);
}

void PlayerJustAvoidState::Update(Player* player, const float& deltaTime)
{
	IInputHandler* handler = player->GetInputHandlerSelector()->GetHandler();

	timer_ += deltaTime;

	

	if (timer_ < duration_)
	{
		float progress = timer_ / duration_;
		if (progress > 1.0f) progress = 1.0f;

		float oneRotation = 2.0f * pi;
		float currentRoll = 0.0f;

		if (avoidDirection_.x < 0.0f)
		{
			currentRoll = progress * oneRotation; // 左回転
		} else
		{
			currentRoll = -progress * oneRotation;  // 右回転
		}

		Vector3 currentRotate = player->GetRotate();
		currentRotate.y = currentRoll;
		player->SetRotate(currentRotate);

		float speedRate = 1.0f - (progress * progress);

		float currentSpeed = avoidSpeed_ * speedRate;

		player->MoveJustAvoid(avoidDirection_, currentSpeed);
	}
	else
	{
		if (handler->IsActionTriggerd("shot"))
		{
			shotCommand_->Execute(player);
			isCounter_ = true;
			return; // ★ 追加: Shot()内部でChangeStateされる
		}
	}

	if (timer_ >= waitDuration_ || isCounter_)
	{
		player->SetRotate(Vector3(0.0f, 0.0f, 0.0f));
		//player->StopJustAvoid(0.05f);
		/*player->ClearLockOn();*/
		player->ChangeState(std::make_unique<PlayerIdleState>());
	}
}

void PlayerJustAvoidState::Draw(Player * player)
{

}

void PlayerJustAvoidState::Finalize(Player * player)
{
	player->SetIsHit(true);
}

void PlayerMeleeAttackState::Initialize(Player* player)
{
	phase_ = AttackPhase::Approach;
	timer_ = 0.0f;
	startPosition_ = player->GetTranslate();
	approachPosition_ = startPosition_;
	player->SetVelocity({ 0.0f, 0.0f, 0.0f });
	player->SetAttackColliderActive(false);

	if (player->HasNearestEnemy())
	{
		const Vector3 playerWorld = player->GetWorldPosition();
		Vector3 toTarget = player->GetNearestEnemyPosition() - playerWorld;
		if (Length(toTarget) > 0.0001f)
		{
			attackDirection_ = Normalize(toTarget);
			Vector3 destinationWorld = player->GetNearestEnemyPosition() - attackDirection_ * 2.0f;
			if (player->GetParentWorldTransform())
			{
				approachPosition_ = TransformPosition(destinationWorld,
					Inverse(player->GetParentWorldTransform()->worldMatrix_));
			}
			else
			{
				approachPosition_ = destinationWorld;
			}
		}
	}
	else
	{
		attackDirection_ = Normalize(TransformNormal(
			{ 0.0f, 0.0f, 1.0f }, player->GetParentWorldTransform()
				? player->GetParentWorldTransform()->worldMatrix_ : MakeIdentity4x4()));
	}

	player->SetMeleeAttackDirection(attackDirection_);
}

void PlayerMeleeAttackState::Update(Player* player, const float& deltaTime)
{
	timer_ += deltaTime;

	switch (phase_)
	{
	case AttackPhase::Approach:
		player->SetTranslate(Lerp(startPosition_, approachPosition_, std::min(timer_ / 0.25f, 1.0f)));
		if (timer_ >= 0.25f) { phase_ = AttackPhase::Windup; timer_ = 0.0f; }
		break;
	case AttackPhase::Windup:
		if (timer_ >= 0.15f)
		{
			phase_ = AttackPhase::Attack;
			timer_ = 0.0f;
			player->SetAttackColliderActive(true);
		}
		break;
	case AttackPhase::Attack:
		if (timer_ >= 0.20f)
		{
			phase_ = AttackPhase::Recovery;
			timer_ = 0.0f;
			player->SetAttackColliderActive(false);
		}
		break;
	case AttackPhase::Recovery:
		if (timer_ >= 0.30f) player->ChangeState(std::make_unique<PlayerIdleState>());
		break;
	}
}

void PlayerMeleeAttackState::Draw(Player* player) { (void)player; }

void PlayerMeleeAttackState::Finalize(Player* player)
{
	player->SetAttackColliderActive(false);
	player->SetVelocity({ 0.0f, 0.0f, 0.0f });
}
