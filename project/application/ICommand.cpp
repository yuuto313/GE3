#include "ICommand.h"

ICommand::~ICommand()
{
}

void AttackCommand::Exec(Player& player)
{
	player.Attack();
}

void MoveRightCommand::Exec(Player& player)
{
	player.MoveRight();
}

void MoveLeftCommand::Exec(Player& player)
{
	player.MoveLeft();
}

void MoveForwardCommand::Exec(Player& player)
{
	player.MoveUp();
}

void MoveBackCommand::Exec(Player& player)
{
	player.MoveDown();
}