#ifndef SG_ANIMATION_H
#define SG_ANIMATION_H

#include "graphic.h"

/** @brief 帧动画数据结构 */
typedef struct
{
        /** @brief 纹理的 ID */
        int texID;

        /** @brief 动画帧的纹理的索引数组 */
        int *frames;

        /** @brief 动画帧的数量 */
        int count;

        /** @brief 动画帧的纹理的宽度和高度 */
        int width, height;

        int loop;
} FrameAnim;

int ANIMATION_Init(lua_State *L);

void ANIMATION_Play(int idx);

/* TODO 管理补间动画的数据结构 */

/* TODO 管理具有计时触发属性的动画的数据结构 */

#endif /* SG_ANIMATION_H */