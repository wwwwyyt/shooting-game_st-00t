#ifndef SG_TEXT_H
#define SG_TEXT_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <SDL3/SDL_rect.h>

typedef struct
{
        int texID;
        int size;
} Font;

/** @brief 初始化文本模块 */
int TEXT_Init(lua_State *L);

/** @brief 渲染文本 */
int TEXT_RenderText(int fontIdx, int charCnt, const int *charIdxArr, SDL_Rect *textbox);

#endif /* SG_TEXT_H */
