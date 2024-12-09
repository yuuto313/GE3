#include "InputHandler.h"
#include "Input.h"

std::unique_ptr<ICommand> InputHandler::HandleInput()
{
    if (Input::GetInstance()->PushKey(DIK_A)) {
        return std::make_unique<MoveLeftCommand>();
    }

    if (Input::GetInstance()->PushKey(DIK_D)) {
        return std::make_unique<MoveRightCommand>();
    }

    if (Input::GetInstance()->PushKey(DIK_W)) {
        return std::make_unique<MoveForwardCommand>();
    }

    if (Input::GetInstance()->PushKey(DIK_S)) {
        return std::make_unique<MoveBackCommand>();
    }

    // 何のコマンドも実行しないときはnullptrを返す
    return nullptr;
}
