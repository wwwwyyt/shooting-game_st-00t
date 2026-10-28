#ifndef SG_VECTOR_H
#define SG_VECTOR_H

#include <SDL3/SDL_stdinc.h>

#define VEC_ZERO ((FVec2){0.0f, 0.0f})

typedef struct
{
        float x, y;
} FVec2;

static inline FVec2 VEC2_Add(FVec2 a, FVec2 b)
{
        return (FVec2){a.x + b.x, a.y + b.y};
}

static inline FVec2 VEC2_Sub(FVec2 a, FVec2 b)
{
        return (FVec2){a.x - b.x, a.y - b.y};
}

static inline FVec2 VEC2_Scale(FVec2 v, float s)
{
        return (FVec2){v.x * s, v.y * s};
}

static inline float VEC2_LenSquare(FVec2 v)
{
        return v.x * v.x + v.y * v.y;
}

static inline float VEC2_Len(FVec2 v)
{
        return SDL_sqrtf(v.x * v.x + v.y * v.y);
}

static inline FVec2 VEC2_Normalize(FVec2 v)
{
        float len = VEC2_Len(v);
        if (len < 1e-6)
                return (FVec2){0, 0};
        return VEC2_Scale(v, 1.0f / len);
}

#endif /* SG_VECTOR_H */
