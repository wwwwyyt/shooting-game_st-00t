#ifndef AUDIO_H
#define AUDIO_H

/* 导入 Lua */
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <SDL3_mixer/SDL_mixer.h>

int AUDIO_Init(lua_State *L);
void AUDIO_PlayAudio(int audioIdx);
void AUDIO_Quit(void);

#endif /* AUDIO_H */