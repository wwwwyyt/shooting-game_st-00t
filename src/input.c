#include "input.h"

static const int GAMEPAD_CNT = 2;
static SDL_Gamepad **gamepads = NULL;

void INPUT_Init(lua_State *L)
{
        gamepads = (SDL_Gamepad **)SDL_calloc(GAMEPAD_CNT, sizeof(SDL_Gamepad *));
}

void INPUT_DoKeyDown(SDL_Event *event)
{
        if (event->key.repeat == 0)
        {
                /* 在 lua 中处理不同键位 */
        }
}

void INPUT_DoKeyPressing(void)
{
        // const bool *keyState = SDL_GetKeyboardState(NULL);
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

void INPUT_DoGamePadBtnDown(SDL_Event *event)
{
        for (int i = 0; i < GAMEPAD_CNT; ++i)
        {
                if (gamepads[i] && SDL_GetGamepadID(gamepads[i]) == event->gdevice.which)
                {
                        /* 在 lua 中处理不同键位 */
                }
        }
}

void INPUT_DoGamePadBtnPressing(void)
{
        for (int i = 0; i < GAMEPAD_CNT; ++i)
        {
                if (gamepads[i])
                {
                        /* 获取手柄摇杆值和进行按键处理 */
                }
        }
}