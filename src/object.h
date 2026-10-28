#ifndef SG_OBJECT
#define SG_OBJECT

#include <stdio.h>

#include "vector.h"
#include "tween.h"

typedef struct
{
        FVec2 pos, vel, acc;
} Object;

void OBJ_Create(Object *obj, FVec2 pos, FVec2 vel, FVec2 acc);
void OBJ_Update(Object *obj, float dt);

#endif /* SG_OBJECT */
