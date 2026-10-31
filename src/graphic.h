#ifndef SG_GRAPHIC_H
#define SG_GRAPHIC_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <glad/glad.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL3/SDL_video.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

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
        int size;
} TextureArray;

/** @brief 初始化 GRAPHIC 模块
 *
 * 在 GRAPHIC 开头的其他函数调用之前调用
 */
int GRAPHIC_Init(lua_State *L);

/** @brief 设置图形绘制模式
 *
 *  至少需要在初始化前调用一次，
 *  用于设置 OpenGL 的投影矩阵和模型视图矩阵
 */
void GRAPHIC_SetMode(SDL_Window *window, GraphicMode mode);

/** @brief 将纹理的矩形区域绘制到视口的矩形区域
 *
 *  @param srcrect
 *  @param dstrect
 */
int GRAPHIC_DrawSprite(int texID, SDL_Rect *srcrect, SDL_Rect *dstrect);

int GRAPHIC_LoadTexture(const char *path, Texture *texture);

int GRAPHIC_GetTextureSize(int idx, int *width, int *height);

#endif /* SG_GRAPHIC_H */
