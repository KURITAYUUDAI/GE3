#include "GameSceneCommand.h"
#include "Player.h"
#include "InputHandler.h"

void MoveHorizontalCommand::Execute(Player* player)
{
	Vector2 direction = inputHandler_->GetDirection();
    if (direction.x == 0.0f && direction.y == 0.0f)
    {
        player->Decelerate(); // 入力なし → 減速
    } 
    else
    {
        player->MoveHorizontal(direction.x, direction.y); // 入力あり → 加速
    }
}

void ShotCommand::Execute(Player* player)
{
	player->Shot();
}

void AvoidCommand::Execute(Player* player)
{
    Vector2 direction = inputHandler_->GetDirection();
    player->Avoid(direction);
}

void MeleeAttackCommand::Execute(Player* player)
{
	player->MeleeAttack();
}


