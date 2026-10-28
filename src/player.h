#ifndef SG_PLAYER_H
#define SG_PLAYER_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <SDL3/SDL_stdinc.h>

#include "object.h"
#include "input.h"

typedef struct
{
        Object object;
        int *frameAnims;
        int frameAnimCnt;
        int active;
} Player;

/** @brief 初始化玩家模块 */
int PLAYER_Init(lua_State *L);

/** @brief 更新玩家状态 */
void PLAYER_Update(float dt);

/** @brief 新增玩家时调用的函数 */
int PLAYER_Add(void);

#endif /* SG_PLAYER_H */
