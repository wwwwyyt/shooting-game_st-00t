#include "audio.h"

static MIX_Mixer *mixer;
static MIX_Audio **audios;
static int audioCnt;
static const int STREAM_QUEUE_SIZE = 16;
static Queue streamQueue;

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

static float generateWave(WaveType type, float freq, float t)
{
        float phase = freq * t; // 相位，单位是周期数
        switch (type)
        {
        case WAVE_SINE:
                return sinf(2.0f * SDL_PI_F * phase);
        case WAVE_SQUARE:
                return sinf(2.0f * SDL_PI_F * phase) >= 0.0f ? 1.0f : -1.0f;
        case WAVE_TRIANGLE:
                return 2.0f * fabsf(2.0f * (phase - floorf(phase + 0.5f))) - 1.0f;
        case WAVE_SAWTOOTH:
                return 2.0f * (phase - floorf(phase)) - 1.0f;
        default:
                return 0.0f;
        }
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

        /* 创建音频流队列 */
        QUE_Create(&streamQueue, STREAM_QUEUE_SIZE);

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

void AUDIO_PlayTone(WaveType type, float freq, float duration, float amplitude)
{
        SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, 44100};
        SDL_AudioStream *toneStream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (!toneStream)
                return;

        int num_samples = (int)(spec.freq * duration);
        float *buffer = SDL_malloc(num_samples * sizeof(float));
        if (!buffer)
        {
                SDL_DestroyAudioStream(toneStream);
                return;
        }

        float attack = 0.005f, release = 0.005f;
        for (int i = 0; i < num_samples; i++)
        {
                float t = (float)i / spec.freq;
                float value = generateWave(type, freq, t);
                float env = 1.0f;
                if (t < attack)
                        env = t / attack;
                else if (t > duration - release)
                        env = (duration - t) / release;
                buffer[i] = value * env * amplitude;
        }

        /* 推入数据并启动播放 */
        SDL_PutAudioStreamData(toneStream, buffer, num_samples * sizeof(float));
        SDL_ResumeAudioStreamDevice(toneStream);
        SDL_free(buffer); /* Put 会复制数据，这里可以立即释放 */

        /* 将音频流压入队列 */
        if (QUE_IsFull(&streamQueue))
        {
                SDL_AudioStream *oldestStream = QUE_Pop(&streamQueue);
                SDL_DestroyAudioStream(oldestStream);
        }
        QUE_Push(&streamQueue, toneStream);
}

void AUDIO_BreakStream(void)
{
        /* 销毁位于队列末尾的已经播放完的音频流 */
        SDL_AudioStream *stream = QUE_Tail(&streamQueue);
        if (stream && SDL_GetAudioStreamQueued(stream) == 0)
        {
                SDL_DestroyAudioStream(stream);
                stream = NULL;
                QUE_Pop(&streamQueue);
        }

        /* 弹出位于队列末尾的空指针 */
        if (!stream)
        {
                QUE_Pop(&streamQueue);
        }
}

void AUDIO_Quit(void)
{
        /* 销毁音频资源 */
        for (int i = 0; i < audioCnt; ++i)
        {
                MIX_DestroyAudio(audios[i]);
        }

        /* 销毁混音器 */
        MIX_DestroyMixer(mixer);

        /* 销毁音频流队列 */
        QUE_Destroy(&streamQueue);
}