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
} NoiseType;

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
void AUDIO_PlayNoise(NoiseType type);

/** @brief 用于打断队列中最旧的音频
 *
 *  所有音频流位于循环队列中，当队列满时，
 *  在 SDL_AppIterate 中调用它以销毁最旧的音频流
 */
void AUDIO_BreakStream(void);

void AUDIO_Quit(void);

#endif /* SG_AUDIO_H */