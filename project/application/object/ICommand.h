#pragma once
#include "Player.h"

class ICommand
{
public:

	virtual ~ICommand();
	virtual void Exec(Player& player) = 0;
};

class AttackCommand : public ICommand
{
	void Exec(Player& player) override;
};

class MoveRightCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

class MoveLeftCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

class MoveForwardCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

class MoveBackCommand : public ICommand {
public:
	void Exec(Player& player) override;
};

