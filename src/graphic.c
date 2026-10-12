#include "graphic.h"

/* 逻辑分辨率 */
static const int LOGICAL_WIDTH = 640;
static const int LOGICAL_HEIGHT = 480;

/* 精灵的位置和纹理顶点 */
static const float QUAD_VERTICES[] = {
    // 位置     // 纹理坐标
    0.0f, 0.0f, 0.0f, 0.0f, // 左上
    1.0f, 0.0f, 1.0f, 0.0f, // 右上
    1.0f, 1.0f, 1.0f, 1.0f, // 右下
    0.0f, 1.0f, 0.0f, 1.0f  // 左下
};

/* 精灵的 VBO */
static GLuint spriteVBO;

/* 纹理数组 */
static TextureArray textures;

static void updateViewport(int windowW, int windowH)
{
        const float targetAspect = (float)LOGICAL_WIDTH / (float)LOGICAL_HEIGHT;
        float windowAspect = (float)windowW / (float)windowH;

        int viewportW, viewportH;
        if (windowAspect > targetAspect)
        {
                viewportH = windowH;
                viewportW = (int)(windowH * targetAspect);
        }
        else
        {
                viewportW = windowW;
                viewportH = (int)(windowW / targetAspect);
        }

        int viewportX = (windowW - viewportW) / 2;
        int viewportY = (windowH - viewportH) / 2;

        glViewport(viewportX, viewportY, viewportW, viewportH);
}

static void initQuadVBO(void)
{
        glGenBuffers(1, &spriteVBO);
        glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES),
                     QUAD_VERTICES, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/** @brief 初始化 GRAPHIC 模块
 *
 * 在 GRAPHIC 开头的其他函数调用之前调用
 */
void GRAPHIC_Init(lua_State *L)
{
        /* 加载配置 */
        lua_getfield(L, -1, "textures");
        if (!lua_istable(L, -1))
        {
                SDL_Log("Field 'textures' is not a table\n");
                lua_close(L);
                return;
        }
        lua_Integer n = luaL_len(L, -1);
        textures.size = n;
        textures.data = calloc(n, sizeof(Texture));
        for (lua_Integer i = 1; i <= n; ++i)
        {
                lua_rawgeti(L, -1, i);

                /* 读取 file */
                lua_getfield(L, -1, "file");
                const char *file = lua_tostring(L, -1);
                textures.data[i - 1] = GRAPHIC_LoadTexture(file);
                lua_pop(L, 1);

                /* 弹出 textures[i] */
                lua_pop(L, 1);
        }
        /* 弹出 textures */
        lua_pop(L, 1);

        /* 初始化 VBO */
        initQuadVBO();
}

void GRAPHIC_SetMode(SDL_Window *window, GraphicMode mode)
{
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);

        /* 设置窗口中的矩形渲染区域 */
        updateViewport(w, h);

        switch (mode)
        {
        case MODE_2D:

                /* 投影 */
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();                                    // 将投影矩阵重置为单位矩阵
                glOrtho(0, LOGICAL_WIDTH, LOGICAL_HEIGHT, 0, -1, 1); // 将正交矩阵写入投影矩阵

                /* 模型视图 */
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                /* 2D 状态设置 */
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glDisable(GL_CULL_FACE);
                glDisable(GL_LIGHTING);
                glEnable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
        case MODE_3D:
                /* 投影 */
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                gluPerspective(60.0, (double)LOGICAL_WIDTH / (double)LOGICAL_HEIGHT, 0.1, 100.0);

                /* 模型视图 */
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                /* 3D 状态 */
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_TRUE);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(GL_CCW);
                glDisable(GL_TEXTURE_2D); // 或者按需开启
                glDisable(GL_BLEND);      // 透明物体单独开启
                /* 光照按需开启 */
                break;
        }
}

void GRAPHIC_DrawSprite(GLuint texture, float x, float y, float w, float h)
{
        glBindTexture(GL_TEXTURE_2D, texture);

        /* 模型视图矩阵: 平移到 (x,y)，缩放到 (w,h) */
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(x, y, 0);
        glScalef(w, h, 1);

        /* 绑定 VBO */
        glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);

        /* 启用固定管线的顶点数组 */
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        /* 设置指针 */
        /* 步长 = 4 个 float（位置 2 + 纹理 2） */
        glVertexPointer(2, GL_FLOAT, 4 * sizeof(float),
                        (void *)0); // 位置偏移 0
        glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float),
                          (void *)(2 * sizeof(float))); // 纹理偏移 2 个 float

        /* 一次绘制 4 个顶点 */
        glDrawArrays(GL_QUADS, 0, 4);

        /* 清理 */
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Texture GRAPHIC_LoadTexture(const char *path)
{
        Texture tex = {0, 0, 0};

        SDL_Surface *surface = IMG_Load(path);
        if (!surface)
                return tex;

        SDL_Surface *converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);
        SDL_DestroySurface(surface);
        if (!converted)
                return tex;

        tex.width = converted->w;
        tex.height = converted->h;

        glGenTextures(1, &(tex.id));
        glBindTexture(GL_TEXTURE_2D, tex.id);

        /* 将 CPU 中的像素数据上传至 GPU */
        /* ABGR8888 对应 GL_RGBA + GL_UNSIGNED_BYTE */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     converted->w, converted->h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, converted->pixels);

        /* 设置纹理采样规则 */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   /* 纹理缩小时采用最邻近采样 */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   /* 纹理放大时采用最邻近采样 */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); /* 纹理环绕时水平方向以边缘色重复 */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); /* 纹理环绕时垂直方向以边缘色重复 */

        SDL_DestroySurface(converted);
        return tex;
}
