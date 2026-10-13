#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_log.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

typedef enum
{
        ACTION_UP,
        ACTION_DOWN,
        ACTION_LEFT,
        ACTION_RIGHT,
        ACTION_BTN_A,
        ACTION_BTN_B,
        ACTION_BTN_C,
        ACTION_BTN_X,
        ACTION_BTN_Y,
        ACTION_BTN_Z,
        ACTION_L_SHOULDER,
        ACTION_R_SHOULDER,
        ACTION_START,
        ACTION_CNT
} ActionType;

void INPUT_Init(lua_State *L);

/** @brief 处理键盘按下这一瞬时信号 */
void INPUT_DoKeyDown(SDL_Event *event);

/** @brief 处理手柄连接 */
void INPUT_GamePadAdded(SDL_Event *event);

/** @brief 处理手柄移除 */
void INPUT_GamePadRemoved(SDL_Event *event);

/** @brief 处理手柄按键按下这一瞬时信号 */
void INPUT_DoGamePadBtnDown(SDL_Event *event);

/** @brief 处理键盘按住这一连续信号 */
void INPUT_DoKeyPressing(void);

/** @brief 处理手柄按键按住这一连续信号 */
void INPUT_DoGamePadBtnPressing(void);

#endif /* INPUT_H */
