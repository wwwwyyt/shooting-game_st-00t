#include "object.h"

void OBJ_Create(Object *obj, FVec2 pos, FVec2 vel, FVec2 acc)
{
        if (obj)
        {
                obj->pos.x = pos.x;
                obj->pos.y = pos.y;
                obj->vel.x = vel.x;
                obj->vel.y = vel.y;
                obj->acc.x = acc.x;
                obj->acc.y = acc.y;
        }
}

void OBJ_Update(Object *obj, float dt)
{
        FVec2 *pos = &obj->pos;
        FVec2 *vel = &obj->vel;
        FVec2 *acc = &obj->acc;

        /* 加速度较小时直接归零 */
        if (VEC2_Len(*acc) < 1e-4)
        {
                *acc = (FVec2){0, 0};
        }

        /* 运动状态更新逻辑 */
        *vel = VEC2_Add(*vel, *acc);
        *pos = VEC2_Add(*pos, *vel);
}
