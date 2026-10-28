#ifndef SG_INPUT_H
#define SG_INPUT_H

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_log.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

typedef enum
{
        TRIGGER_PRESS,
        TRIGGER_HOLD,
} TriggerType;

typedef enum
{
        INPUT_DEVICE_KEYBOARD,
        INPUT_DEVICE_GAMEPAD,
} InputDeviceType;

void INPUT_Init(lua_State *L);

/** @brief 用于在键盘按下时设置信号表 */
void INPUT_DoKeyDown(SDL_Event *event);

/** @brief 用于在键盘松开时设置信号表 */
void INPUT_DoKeyUp(SDL_Event *event);

/** @brief 处理手柄连接 */
void INPUT_GamePadAdded(SDL_Event *event);

/** @brief 处理手柄移除 */
void INPUT_GamePadRemoved(SDL_Event *event);

/** @brief 处理手柄按键按下或松开 */
void INPUT_DoGamePadBtnEvent(SDL_Event *event);

/** @brief 获取所有种类信号的表
 *  @return 1 或 0 是正常的信号值，-1 表示数组越界
 */
int INPUT_GetSignal(SDL_Scancode scancode, SDL_GamepadButton gamepadbtn,
                    InputDeviceType device, int gamepadIdx, TriggerType trigger);

#endif /* SG_INPUT_H */
