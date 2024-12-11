#include "InputHandler.h"
#include "Input.h"

std::vector<std::unique_ptr<ICommand>> InputHandler::HandleInput()
{
    std::vector<std::unique_ptr<ICommand>> commands;

    if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
        commands.push_back(std::make_unique<AttackCommand>());
    }

    if (Input::GetInstance()->PushKey(DIK_A)) {
        commands.push_back(std::make_unique<MoveLeftCommand>());
    }

    if (Input::GetInstance()->PushKey(DIK_D)) {
        commands.push_back(std::make_unique<MoveRightCommand>());
    }

    if (Input::GetInstance()->PushKey(DIK_W)) {
        commands.push_back(std::make_unique<MoveForwardCommand>());
    }

    if (Input::GetInstance()->PushKey(DIK_S)) {
        commands.push_back(std::make_unique<MoveBackCommand>());
    }

    // 何のコマンドも実行しないときはnullptrを返す
    return commands;
}
