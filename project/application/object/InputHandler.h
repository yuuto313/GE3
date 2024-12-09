#pragma once
#include "ICommand.h"
#include <memory>

/// <summary>
/// 入力を取りまとめるクラス
/// </summary>
class InputHandler
{
public:
	// キーの割り当てを行う
	std::unique_ptr<ICommand> HandleInput();
};

