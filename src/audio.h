#ifndef SG_AUDIO_H
#define SG_AUDIO_H

/* 导入 Lua */
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <SDL3_mixer/SDL_mixer.h>

#include <math.h>

#include "queue.h"

/** @brief 波形 */
typedef enum
{
        /** @brief 正弦波 */
        WAVE_SINE,

        /** @brief 方波 */
        WAVE_SQUARE,

        /** @brief 三角波 */
        WAVE_TRIANGLE,

        /** @brief 锯齿波 */
        WAVE_SAWTOOTH
} WaveType;

/** @brief 噪声类型 */
typedef enum
{
        /** @brief 白噪声 */
        NOISE_WHITE,

        /** @brief 粉红噪声 */
        NOISE_PINK,

        /** @brief 小爆炸 */
        NOISE_EXPLOSION_SMALL,

        /** @brief 大爆炸 */
        NOISE_EXPLOSION_LARGE,

        NOISE_EXPLOSION_SHOOT
} NoiseType;

/** @brief 粉红噪声的状态 */
typedef struct
{
        float b0, b1, b2, b3, b4, b5, b6;
} PinkState;

/** @brief 一阶低通滤波器状态 */
typedef struct
{
        float y;
} LowPass1;

/** @brief 爆炸参数，方便调不同大小的爆炸 */
typedef struct
{
        /** @brief 指数衰减速率，越大越短促（建议 4~10） */
        float decay_rate;

        /** @brief 初始低通截止，越大越"亮"（建议 0.3~0.7） */
        float initial_cutoff;

        /** @brief 末尾截止，越小越"沉"（建议 0.01~0.05） */
        float final_cutoff;

        /** @brief 低频冲击频率 Hz（建议 50~80） */
        float boom_freq;

        /** @brief 低频冲击占比（建议 0.2~0.5） */
        float boom_amount;

        /** @brief 噪声增益 */
        float noise_gain;

        /** @brief 归一化目标峰值（建议 0.8~0.95） */
        float peak;
} ExplosionParams;

int AUDIO_Init(lua_State *L);
void AUDIO_PlayAudio(int audioIdx);

/** @brief 用于播放简单波形
 *
 *  @param type 波形
 *  @param freq 频率
 *  @param duration 持续时间，单位是 s
 *  @param amplitude 振幅，为 [0, 1] 之间的一个数
 */
void AUDIO_PlayTone(WaveType type, float freq, float duration, float amplitude);

/** @brief 用于播放噪声 */
void AUDIO_PlayNoise(NoiseType type, float duration, float amplitude);

/** @brief 用于打断队列中最旧的音频
 *
 *  所有音频流位于循环队列中，当队列满时，
 *  在 SDL_AppIterate 中调用它以销毁最旧的音频流
 */
void AUDIO_BreakStream(void);

void AUDIO_Quit(void);

#endif /* SG_AUDIO_H */