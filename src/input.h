#ifndef SG_INPUT_H
#define SG_INPUT_H

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_error.h>

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

typedef union
{
        SDL_Scancode scancode;
        SDL_GamepadButton gamepadbtn;
} InputCode;

/** @breif 初始化输入模块 */
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
 *  @param inputcode 按键键码，既可以是键盘键码也可以是手柄键码
 *  @param device 设备类型，
 *  @param gamepadIdx 手柄编号，仅当设备类型为手柄时有效
 *  @param trigger 触发方式
 *  @return 1 或 0 是正常的信号值，-1 表示出现错误，
 *  可以使用 SDL_GetErr 获取
 */
int INPUT_GetInput(InputCode inputcode, InputDeviceType device, int gamepadIdx, TriggerType trigger);

#endif /* SG_INPUT_H */
