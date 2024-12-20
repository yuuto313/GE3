#include "InputHandler.h"
#include "Input.h"

std::vector<std::unique_ptr<ICommand>> InputHandler::HandleInput()
{
    std::vector<std::unique_ptr<ICommand>> commands;

    // キーボード入力
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

    // XBOXコントローラー入力
    XINPUT_STATE joystick;

    if (Input::GetInstance()->GetJoystickState(0, joystick)) {
        // Aボタン
        if (joystick.Gamepad.wButtons & XINPUT_GAMEPAD_A) {
            commands.push_back(std::make_unique<AttackCommand>());
        }

        // 左スティック左右入力
        SHORT lx = joystick.Gamepad.sThumbLX;
        if (lx > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            commands.push_back(std::make_unique<MoveRightCommand>());
        } else if (lx < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            commands.push_back(std::make_unique<MoveLeftCommand>());
        }

        // 左スティック上下入力
        SHORT ly = joystick.Gamepad.sThumbLY;
        if (ly > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            commands.push_back(std::make_unique<MoveForwardCommand>());
        } else if (ly < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
            commands.push_back(std::make_unique<MoveBackCommand>());
        }

    }

    // 何のコマンドも実行しないときはnullptrを返す
    return commands;
}
