#include "input.h"
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_error.h>

/** @brief 手柄设备最大数量 */
static const int GAMEPAD_CNT = 2;

/** @brief 手柄设备数组 */
static SDL_Gamepad **gamepads = NULL;

/** @brief 键盘键码表 */
static int scancodes[SDL_SCANCODE_COUNT];

/** @brief 手柄键码表 */
static int *gamepadbtncodes;

void INPUT_Init(lua_State *L)
{
        /* 初始化手柄结构体 */
        gamepads = SDL_calloc(GAMEPAD_CNT, sizeof(SDL_Gamepad *));

        /* 初始化手柄键码表 */
        gamepadbtncodes = SDL_calloc(GAMEPAD_CNT * SDL_GAMEPAD_BUTTON_COUNT, sizeof(*gamepadbtncodes));
}

void INPUT_DoKeyDown(SDL_Event *event)
{
        if (event->key.repeat == 0)
        {
                scancodes[event->key.scancode] = 1;
        }
}

void INPUT_DoKeyUp(SDL_Event *event)
{
        if (event->key.repeat == 0)
        {
                scancodes[event->key.scancode] = 0;
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
                        SDL_Log("Gamepad removed: %s", SDL_GetGamepadName(gamepads[i]));
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
                                gamepadbtncodes[i * SDL_GAMEPAD_BUTTON_COUNT + buttonIdx] = (int)event->gbutton.down;
                        }
                        break;
                }
        }
}

int INPUT_GetInput(InputCode inputcode, InputDeviceType device, int gamepadIdx, TriggerType trigger)
{
        switch (device)
        {
        case INPUT_DEVICE_KEYBOARD:
                if (inputcode.scancode >= 0 && inputcode.scancode < SDL_SCANCODE_COUNT)
                {
                        const bool *keyState = SDL_GetKeyboardState(NULL);
                        switch (trigger)
                        {
                        case TRIGGER_HOLD:
                                return scancodes[inputcode.scancode];
                        case TRIGGER_PRESS:
                                return keyState[inputcode.scancode];
                        default:
                                SDL_SetError("Unknown trigger type: %d", trigger);
                                break;
                        }
                }
                else
                {
                        SDL_SetError("Invalid scancode: %d , expect value between 0 and %d",
                                     inputcode.scancode,
                                     SDL_SCANCODE_COUNT - 1);
                }
                break;
        case INPUT_DEVICE_GAMEPAD:
                if (inputcode.gamepadbtn >= 0 && inputcode.gamepadbtn < SDL_GAMEPAD_BUTTON_COUNT)
                {
                        switch (trigger)
                        {
                        case TRIGGER_HOLD:
                                if (gamepadIdx > 0 && gamepadIdx < 2)
                                {
                                        return gamepadbtncodes[gamepadIdx * SDL_GAMEPAD_BUTTON_COUNT + inputcode.gamepadbtn];
                                }
                                break;
                        case TRIGGER_PRESS:
                                return SDL_GetGamepadButton(gamepads[gamepadIdx], inputcode.gamepadbtn);
                        default:
                                SDL_SetError("Unknown trigger type: %d", trigger);
                                break;
                        }
                }
                else
                {
                        SDL_SetError("Invalid gamepadbtn: %d , expect value between 0 and %d",
                                     inputcode.gamepadbtn,
                                     SDL_GAMEPAD_BUTTON_COUNT - 1);
                }
                break;
        default:
                SDL_SetError("Unknown device type: %d", device);
                break;
        }
        return -1;
}
