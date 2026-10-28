#ifndef SG_GRAPHIC_H
#define SG_GRAPHIC_H

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

int GRAPHIC_Init(lua_State *L);
void GRAPHIC_SetMode(SDL_Window *window, GraphicMode mode);
int GRAPHIC_DrawSprite(int idx, SDL_FRect *srcrect, SDL_FRect *dstrect);
int GRAPHIC_LoadTexture(const char *path, Texture *texture);
int GRAPHIC_GetTextureSize(int idx, int *width, int *height);

#endif /* SG_GRAPHIC_H */
