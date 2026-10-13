#ifndef TEXT_H
#define TEXT_H

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "graphic.h"

typedef struct
{
        int texIdx;
        int size;
} Font;

int TEXT_Init(lua_State *L);
int TEXT_RenderText(int fontIdx, int charCnt, const int *charIdxArr, SDL_FRect *textbox);

#endif /* TEXT_H */