#include "input.h"

static const int PLAYER_CNT = 2;

/** @brief 动作函数列表
 *
 * 返回 0 代表成功，返回 1 代表失败，可以使用 SDL_GetError 获取错误信息
 */
static int (**actions)(void);

static const int GAMEPAD_CNT = 2;
static SDL_Gamepad **gamepads = NULL;

static int actionMap[SDL_SCANCODE_COUNT];

int dummy(void)
{
        printf("event\n");
        return 0;
}

void INPUT_Init(lua_State *L)
{
        /* 初始化手柄结构体 */
        gamepads = (SDL_Gamepad **)SDL_calloc(GAMEPAD_CNT, sizeof(SDL_Gamepad *));

        /* 初始化动作函数表 */
        actions = SDL_calloc(PLAYER_CNT * ACTION_CNT, sizeof *actions);
        for (int i = 0; i < sizeof actions; ++i)
        {
                /* 将所有动作函数置为空函数 */
                actions[i] = dummy;
        }

        /* TODO: 注册动作函数 */
}

void INPUT_DoKeyDown(SDL_Event *event)
{
        if (event->key.repeat == 0)
        {
                int (*action)(void) = actions[actionMap[event->key.scancode]];
                if (action && action())
                {
                        SDL_Log("Action failed for scancode %d (%s): %s\n",
                                (int)event->key.scancode,
                                SDL_GetScancodeName(event->key.scancode),
                                SDL_GetError());
                }
        }
}

void INPUT_DoKeyPressing(void)
{
        // const bool *keyState = SDL_GetKeyboardState(NULL);
        /* 需要根据这个 keyState 判断某个动作是否需要执行 */
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

void INPUT_RegisterAction(int (*actionFunc)(void), int player, ActionType type)
{
        actions[player * ACTION_CNT + (int)type] = actionFunc;
}
