#include "audio.h"

static MIX_Mixer *mixer;
static MIX_Audio **audios;
static int audioCnt;
static const int STREAM_QUEUE_SIZE = 16;
static Queue streamQueue;

/* 预设：小而脆的爆炸 */
static const ExplosionParams EXPLOSION_SMALL = {
    .decay_rate = 10.0f,
    .initial_cutoff = 0.7f,
    .final_cutoff = 0.05f,
    .boom_freq = 80.0f,
    .boom_amount = 0.25f,
    .noise_gain = 2.0f,
    .peak = 0.9f,
};

/* 预设：大而沉的爆炸 */
static const ExplosionParams EXPLOSION_LARGE = {
    .decay_rate = 4.0f,
    .initial_cutoff = 0.4f,
    .final_cutoff = 0.01f,
    .boom_freq = 50.0f,
    .boom_amount = 0.5f,
    .noise_gain = 2.5f,
    .peak = 0.95f,
};

/* 预设：枪声 */
static const ExplosionParams EXPLOSION_SHOOT = {
    .decay_rate = 18.0f,
    .initial_cutoff = 0.2f,
    .final_cutoff = 0.3f,
    .boom_freq = 120.0f,
    .boom_amount = 0.15f,
    .noise_gain = 2.5f,
    .peak = 0.95f,
};

static int loadAudio(const char *path, MIX_Audio **audio)
{
        *audio = MIX_LoadAudio(mixer, path, true);
        if (!*audio)
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

/** @brief 使用 Paul Kellet 算法生成粉红噪声 */
static float pinkNoise(PinkState *s, float white)
{
        s->b0 = 0.99886f * s->b0 + white * 0.0555179f;
        s->b1 = 0.99332f * s->b1 + white * 0.0750759f;
        s->b2 = 0.96900f * s->b2 + white * 0.1538520f;
        s->b3 = 0.86650f * s->b3 + white * 0.3104856f;
        s->b4 = 0.55000f * s->b4 + white * 0.5329522f;
        s->b5 = -0.7616f * s->b5 - white * 0.0168980f;
        float pink = s->b0 + s->b1 + s->b2 + s->b3 + s->b4 + s->b5 + s->b6 + white * 0.5362f;
        s->b6 = white * 0.115926f;
        return pink * 0.11f; // 缩放，避免过大
}

/** @brief 一阶低通滤波
 *
 *  @param cutoff 0.0~1.0，越小越低沉
 */
static inline float lowpass1(LowPass1 *lp, float input, float cutoff)
{
        lp->y += cutoff * (input - lp->y);
        return lp->y;
}

/** @brief 归一化到指定峰值 */
static void normalize(float *buf, int n, float target_peak)
{
        float peak = 0.0f;
        for (int i = 0; i < n; i++)
        {
                float a = fabsf(buf[i]);
                if (a > peak)
                        peak = a;
        }
        if (peak > 1e-6f)
        {
                float scale = target_peak / peak;
                for (int i = 0; i < n; i++)
                        buf[i] *= scale;
        }
}

/** @brief 就地填充爆炸样本 */
static void generateExplosionSamples(float *buf, int n, int sample_rate,
                                     const ExplosionParams *p)
{
        LowPass1 lp = {0};
        float inv_sr = 1.0f / (float)sample_rate;

        /* 淡入时长：枪声/爆炸用 2ms 左右即可，够消爆音又不影响"啪"的干脆感 */
        const float attack = 0.002f;

        for (int i = 0; i < n; i++)
        {
                float t = (float)i * inv_sr;
                float progress = (float)i / (float)n; // 0~1

                // 1. 白噪声
                float white = SDL_randf() * 2.0f - 1.0f;

                // 2. 低通扫频：从 initial_cutoff 扫到 final_cutoff
                float cutoff = p->initial_cutoff +
                               (p->final_cutoff - p->initial_cutoff) * progress;
                float filtered = lowpass1(&lp, white, cutoff);

                // 3. 指数衰减包络
                float env = expf(-p->decay_rate * t);

                // 4. 低频冲击
                float boom = sinf(2.0f * SDL_PI_F * p->boom_freq * t) * expf(-p->decay_rate * 2.0f * t);

                // 5. 线性淡入，消除开头直流阶跃造成的爆音
                float fade_in = (t < attack) ? (t / attack) : 1.0f;

                // 混合（淡入乘在最后）
                buf[i] = (filtered * p->noise_gain + boom * p->boom_amount) * env * fade_in;
        }

        normalize(buf, n, p->peak);
}

/** @brief 生成噪声 PCM
 *
 * 调用者负责 SDL_free
 *
 * @param out_count 返回样本数
 */
float *generateNoise(NoiseType type, float duration, int sample_rate,
                     int *out_count)
{
        int n = (int)(duration * (float)sample_rate);
        if (n <= 0)
        {
                *out_count = 0;
                return NULL;
        }

        float *buf = SDL_malloc((size_t)n * sizeof(float));
        if (!buf)
        {
                *out_count = 0;
                return NULL;
        }
        *out_count = n;

        switch (type)
        {
        case NOISE_WHITE:
                for (int i = 0; i < n; i++)
                        buf[i] = (SDL_randf() * 2.0f - 1.0f) * 0.5f;
                break;

        case NOISE_PINK:
        {
                PinkState ps = {0};
                for (int i = 0; i < n; i++)
                        buf[i] = pinkNoise(&ps, SDL_randf() * 2.0f - 1.0f) * 3.0f;
                normalize(buf, n, 0.7f);
                break;
        }

        case NOISE_EXPLOSION_SMALL:
                generateExplosionSamples(buf, n, sample_rate, &EXPLOSION_SMALL);
                break;

        case NOISE_EXPLOSION_LARGE:
                generateExplosionSamples(buf, n, sample_rate, &EXPLOSION_LARGE);
                break;
        case NOISE_EXPLOSION_SHOOT:
                generateExplosionSamples(buf, n, sample_rate, &EXPLOSION_SHOOT);
                break;
        }
        return buf;
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

void AUDIO_PlayNoise(NoiseType type, float duration, float amplitude)
{
        SDL_AudioSpec spec = {SDL_AUDIO_F32, 1, 44100};
        SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (!stream)
                return;

        int num_samples = 0;
        float *buffer = generateNoise(type, duration, spec.freq, &num_samples);
        if (!buffer || num_samples <= 0)
        {
                SDL_DestroyAudioStream(stream);
                SDL_free(buffer);
                return;
        }

        /* 应用整体音量（爆炸预设已归一化到 0.9/0.95，注意 amplitude 别超过 1） */
        for (int i = 0; i < num_samples; i++)
                buffer[i] *= amplitude;

        SDL_PutAudioStreamData(stream, buffer, num_samples * sizeof(float));
        SDL_ResumeAudioStreamDevice(stream);
        SDL_free(buffer); /* Put 会复制数据，这里可以立即释放 */

        /* 将音频流压入队列，满了就打断最旧的流 */
        if (QUE_IsFull(&streamQueue))
        {
                SDL_AudioStream *oldestStream = QUE_Pop(&streamQueue);
                SDL_DestroyAudioStream(oldestStream);
        }
        QUE_Push(&streamQueue, stream);
}

void AUDIO_BreakStream(void)
{
        /* 回收所有播放完毕的流 */
        while (!QUE_IsEmpty(&streamQueue))
        {
                SDL_AudioStream *stream = QUE_Tail(&streamQueue);
                if (!stream || SDL_GetAudioStreamQueued(stream) != 0)
                        break;
                QUE_Pop(&streamQueue);
                SDL_DestroyAudioStream(stream);
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