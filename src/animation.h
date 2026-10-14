#ifndef SG_ANIMATION_H
#define SG_ANIMATION_H

#include "graphic.h"

/** @brief 帧动画数据结构 */
typedef struct
{
        /** @brief 动画帧的纹理的索引数组 */
        int *frames;

        /** @brief 动画帧的数量 */
        int count;

        /** @brief 动画帧的纹理的宽度和高度 */
        int width, height;
} FrameAnim;

#endif /* SG_ANIMATION_H */