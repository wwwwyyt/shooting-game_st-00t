#ifndef SG_TWEEN_H
#define SG_TWEEN_H

#include <SDL3/SDL_stdinc.h>

#include "vector.h"
#include "object.h"

/** @brief 缓动类型 */
typedef enum
{
        /** @brief 线性 */
        EASE_LINEAR,

        /** @brief  */
        EASE_QUAD_IN,
        EASE_QUAD_OUT,
        EASE_QUAD_INOUT,

        EASE_CUBIC_IN,
        EASE_CUBIC_OUT,
        EASE_CUBIC_INOUT,

        EASE_SINE_INOUT,

        EASE_EXPO_IN,
        EASE_EXPO_OUT,

        EASE_ELASTIC_OUT
} EaseType;

typedef struct
{
        /** @brief 补间的起始值 */
        FVec2 a;

        /** @brief 补间的终值 */
        FVec2 z;

        /** @brief 指向被补间值的指针 */
        FVec2 *y;

        /** @brief 缓动类型 */
        EaseType ease;

        /** @brief 持续时间 */
        float duration;

        /** @brief 计时器 */
        float t;
} Tween;

void TWEEN_Create(Tween *tween,
                  FVec2 a, FVec2 z, FVec2 *y,
                  EaseType ease, float duration);

void TWEEN_Update(Tween *tween, float dt);

#endif /* SG_TWEEN_H */
