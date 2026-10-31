/* 导入 Lua */
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/* 导入 SDL3 */
#define SDL_MAIN_USE_CALLBACKS 1 /* 使用回调而不是 main() */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <stdio.h>

#include "graphic.h"
#include "input.h"
#include "text.h"
#include "audio.h"

static const char *APP_NAME = "shuguang";
static const char *APP_VERSION = "0.1.0";
static const char *APP_IDENTIFIER = "tech.cano.shuguang";
static const int APP_SCREEN_WIDTH = 640;  /* 窗口默认分辨率 */
static const int APP_SCREEN_HEIGHT = 480; /* 窗口默认分辨率 */
static const char *APP_WINDOW_TITLE = "shuguang";
static const GLfloat APP_CLEAR_COLOR_R = 0.0f;
static const GLfloat APP_CLEAR_COLOR_G = 0.0f;
static const GLfloat APP_CLEAR_COLOR_B = 0.0f;
static const GLfloat APP_CLEAR_COLOR_A = 1.0f;

static SDL_Window *window;
static Uint64 timeNS = 0;
static const Uint64 dtNS = 1e7;
static Uint64 currentTimeNS;
static Uint64 accumulatorNS = 0;

static lua_State *L;

static int init(void)
{
        /* 初始化 Lua 环境 */
        L = luaL_newstate();
        luaL_openlibs(L);
        if (luaL_dofile(L, "scripts/init.lua") != LUA_OK)
        {
                SDL_Log("Couldn't load lua: %s\n", lua_tostring(L, -1));
                lua_close(L);
                return 1;
        }
        if (!lua_istable(L, -1))
        {
                SDL_Log("init.lua must return a table\n");
                lua_pop(L, 1);
                return 1;
        }

        /* 加载配置文件 */
        lua_getfield(L, -1, "load_config");
        if (lua_pcall(L, 0, 1, 0) != LUA_OK)
        {
                SDL_Log("%s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
                return 1;
        }

        /* 初始化图形模块 */
        if (GRAPHIC_Init(L))
        {
                return 1;
        }
        GRAPHIC_SetMode(window, MODE_2D);

        /* 初始化输入模块 */
        INPUT_Init(L);

        /* 初始化文本模块 */
        TEXT_Init(L);

        /* 初始化音频模块 */
        AUDIO_Init(L);

        return 0;
}

static int update(Uint64 timeNS, const Uint64 dtNS)
{

        return 0;
}

static int render(void)
{
        /* 清屏 */
        glClearColor(APP_CLEAR_COLOR_R, APP_CLEAR_COLOR_G, APP_CLEAR_COLOR_B, APP_CLEAR_COLOR_A);
        glClear(GL_COLOR_BUFFER_BIT);

        /* 交换缓冲，显示渲染内容 */
        SDL_GL_SwapWindow(window);
        return 0;
}

static void quit(void)
{    
        /* 清理音频模块 */
        AUDIO_Quit();
        MIX_Quit();
        
        /* 销毁 Lua 环境 */
        lua_close(L);
}

/** 该函数在启动时运行一次 */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
        /* 设置元数据 */
        SDL_SetAppMetadata(APP_NAME,
                           APP_VERSION,
                           APP_IDENTIFIER);

        /* 初始化 SDL */
        /* 包括: 视频子系统、混音子系统、手柄子系统 */
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD))
        {
                SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        /* 初始化 SDL_mixer */
        MIX_Init();

        /* 创建窗口 */
        window = SDL_CreateWindow(APP_WINDOW_TITLE,
                                  APP_SCREEN_WIDTH, APP_SCREEN_HEIGHT,
                                  SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
        if (!window)
        {
                SDL_Log("Couldn't create window: %s", SDL_GetError());
                return SDL_APP_FAILURE;
        }

        /* 声明 OpenGL 版本及兼容性 */
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

        /* 创建并激活 OpenGL 上下文 */
        SDL_GLContext gl_context = SDL_GL_CreateContext(window);
        if (!gl_context)
        {
                SDL_Log("Couldn't create OpenGL context: %s", SDL_GetError());
                SDL_DestroyWindow(window);
                return SDL_APP_FAILURE;
        }
        SDL_GL_MakeCurrent(window, gl_context);

        /* 初始化 GLAD */
        if (gladLoadGL() == 0)
        {
                SDL_Log("Couldn't initialize GLAD");
                return SDL_APP_FAILURE;
        }

        /* 开启垂直同步 */
        if (!SDL_GL_SetSwapInterval(1))
        {
                SDL_Log("VSync 1 not supported: %s", SDL_GetError());
                SDL_GL_SetSwapInterval(0);
        }

        /* 设置 currentTimeNS 初值 */
        currentTimeNS = SDL_GetTicksNS() / 1e9;

        /* 执行自定义初始化逻辑 */
        if (init())
        {
                return SDL_APP_FAILURE;
        }

        /* 程序继续 */
        return SDL_APP_CONTINUE;
}

/** 该函数在新的事件（如鼠标点击，键盘输入）发生时运行 */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
        switch (event->type)
        {
        case SDL_EVENT_QUIT:
                /* 终止程序，向 OS 报告成功 */
                return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_DOWN:
                /* 处理键盘按下 */
                INPUT_DoKeyDown(event);
                break;
        case SDL_EVENT_KEY_UP:
                /* 处理键盘松开 */
                INPUT_DoKeyUp(event);
                break;
        case SDL_EVENT_GAMEPAD_ADDED:
                /* 处理手柄连接 */
                INPUT_GamePadAdded(event);
                break;
        case SDL_EVENT_GAMEPAD_REMOVED:
                /* 处理手柄断开连接 */
                INPUT_GamePadRemoved(event);
                break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                /* 处理手柄按钮按下 */
                INPUT_DoGamePadBtnEvent(event);
                break;
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
                /* 处理手柄按钮松开 */
                INPUT_DoGamePadBtnEvent(event);
                break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                /* 处理窗口像素尺寸变化 */
                GRAPHIC_SetMode(window, MODE_2D);
                break;
        default:
                break;
        }
        return SDL_APP_CONTINUE; /* 程序继续 */
}

/** 该函数在每一帧运行一次，是程序的核心 */
SDL_AppResult SDL_AppIterate(void *appstate)
{
        /* 使用累加器解耦模拟与渲染 */
        Uint64 newTime = SDL_GetTicksNS();
        Uint64 frameTime = newTime - currentTimeNS;
        currentTimeNS = newTime;
        accumulatorNS += frameTime;
        while (accumulatorNS >= dtNS)
        {
                if (update(timeNS, dtNS))
                {
                        return SDL_APP_FAILURE;
                }
                accumulatorNS -= dtNS;
                timeNS += dtNS;
        }

        /* 执行自定义渲染逻辑 */
        if (render())
        {
                return SDL_APP_FAILURE;
        }

        /* 程序继续 */
        return SDL_APP_CONTINUE;
}

/** 该函数在程序关闭时运行一次 */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
        /* 执行自定义退出逻辑 */
        quit();

        /* SDL 会自动销毁 window */
}
