#ifndef GRAPHIC_H
#define GRAPHIC_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <glad/glad.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <stdio.h>

typedef enum
{
        MODE_2D,
        MODE_3D
} GraphicMode;

typedef struct
{
        GLuint id;
        int width;
        int height;
} Texture;

typedef struct
{
        Texture *data;
        size_t size;
} TextureArray;

void GRAPHIC_Init(lua_State *L);
void GRAPHIC_SetMode(SDL_Window *window, GraphicMode mode);
void GRAPHIC_DrawSprite(int idx, float x, float y, float w, float h);
Texture GRAPHIC_LoadTexture(const char *path);

#endif /* GRAPHIC_H */
