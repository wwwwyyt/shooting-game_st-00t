#include "audio.h"

static MIX_Mixer *mixer;
static MIX_Audio **audios;
static int audioCnt;

static int loadAudio(const char *path, MIX_Audio **audio)
{
        *audio = MIX_LoadAudio(mixer, path, true);
        if (*audio)
        {
                SDL_Log("Failed to load audio: %s", SDL_GetError());
                return 1;
        }
        return 0;
}

int AUDIO_Init(lua_State *L)
{
        /* 创建默认音频设备混音器 */
        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
        if (!mixer)
        {
                SDL_Log("Failed to create mixer: %s", SDL_GetError());
                return -1;
        }

        /* 根据 Lua 配置加载音频资源文件 */
        /* 加载配置 */
        lua_getfield(L, -1, "audios");
        if (!lua_istable(L, -1))
        {
                SDL_Log("Field 'audios' is not a table\n");
                lua_close(L);
                return 1;
        }

        audioCnt = (int)luaL_len(L, -1);
        audios = SDL_calloc(audioCnt, sizeof *audios);

        for (lua_Integer i = 1; i <= audioCnt; ++i)
        {
                lua_rawgeti(L, -1, i);

                /* 读取音频文件 */
                lua_getfield(L, -1, "file");
                const char *file = lua_tostring(L, -1);
                if (loadAudio(file, &(audios[i - 1])))
                {
                        return 1;
                }

                lua_pop(L, 1);

                /* 弹出 audios[i] */
                lua_pop(L, 1);
        }
        /* 弹出 audios */
        lua_pop(L, 1);

        return 0;
}

void AUDIO_PlayAudio(int audioIdx)
{
        MIX_PlayAudio(mixer, audios[audioIdx]);
}

void AUDIO_Quit(void)
{
        for (int i = 0; i < audioCnt; ++i)
        {
                MIX_DestroyAudio(audios[i]);
        }
        MIX_DestroyMixer(mixer);
}