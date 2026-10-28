#include "input.h"

/** @brief 手柄设备最大数量 */
static const int GAMEPAD_CNT = 2;

/** @brief 手柄设备数组 */
static SDL_Gamepad **gamepads = NULL;

/** @brief 键盘扫描码（scancode）表 */
static int scancodeTab[SDL_SCANCODE_COUNT];

/** @brief 手柄按键表 */
static int gamepadButtonTab[SDL_GAMEPAD_BUTTON_COUNT];

static bool gamepadButtonState[GAMEPAD_CNT][SDL_GAMEPAD_BUTTON_COUNT];

/** @brief 从信号映射到扫描码的映射表 */
static int *signalMap;

/** @brief 信号变量数 */
static int signalCnt;

void INPUT_Init(lua_State *L)
{
        /* 初始化手柄结构体 */
        gamepads = (SDL_Gamepad **)SDL_calloc(GAMEPAD_CNT, sizeof(SDL_Gamepad *));
}

void INPUT_DoKeyDown(SDL_Event *event)
{
        if (event->key.repeat == 0)
        {
                scancodeTab[event->key.scancode] = 1;
        }
}

void INPUT_DoKeyUp(SDL_Event *event)
{
        if (event->key.repeat == 0)
        {
                scancodeTab[event->key.scancode] = 0;
        }
}

void INPUT_GamePadAdded(SDL_Event *event)
{
        for (int i = 0; i < GAMEPAD_CNT; ++i)
        {
                if (!gamepads[i])
                {
                        gamepads[i] = SDL_OpenGamepad(event->gdevice.which);
                        if (gamepads[i])
                        {
                                SDL_Log("Gamepad connected: %s", SDL_GetGamepadName(gamepads[i]));
                        }
                }
        }
}

void INPUT_GamePadRemoved(SDL_Event *event)
{
        for (int i = 0; i < GAMEPAD_CNT; ++i)
        {
                if (gamepads[i] && SDL_GetGamepadID(gamepads[i]) == event->gdevice.which)
                {
                        SDL_CloseGamepad(gamepads[i]);
                        gamepads[i] = NULL;
                }
        }
}

void INPUT_DoGamePadBtnEvent(SDL_Event *event)
{
        for (int i = 0; i < GAMEPAD_CNT; ++i)
        {
                if (gamepads[i] && SDL_GetGamepadID(gamepads[i]) == event->gdevice.which)
                {
                        int buttonIdx = event->gbutton.button;
                        if (buttonIdx >= 0 && buttonIdx < SDL_GAMEPAD_BUTTON_COUNT)
                        {
                                gamepadButtonState[i][buttonIdx] = event->gbutton.down;
                        }
                        break;
                }
        }
}

int INPUT_GetSignal(SDL_Scancode scancode, SDL_GamepadButton gamepadbtn,
                    InputDeviceType device, int gamepadIdx, TriggerType trigger)
{
        switch (device)
        {
        case INPUT_DEVICE_KEYBOARD:
                if (scancode >= 0 && scancode < SDL_SCANCODE_COUNT)
                {
                        const bool *keyState = SDL_GetKeyboardState(NULL);
                        switch (trigger)
                        {
                        case TRIGGER_HOLD:
                                return scancodeTab[scancode];
                        case TRIGGER_PRESS:
                                return keyState[scancode];
                        }
                }
                break;
        case INPUT_DEVICE_GAMEPAD:
                if (gamepadbtn >= 0 && gamepadbtn < SDL_GAMEPAD_BUTTON_COUNT)
                {
                        switch (trigger)
                        {
                        case TRIGGER_HOLD:
                                if (gamepadIdx > 0 && gamepadIdx < 2)
                                        return gamepadButtonState[gamepadIdx][gamepadbtn];
                                break;
                        case TRIGGER_PRESS:
                                return SDL_GetGamepadButton(gamepads[gamepadIdx], gamepadbtn);
                        }
                }
                break;
        }
        return -1;
}
