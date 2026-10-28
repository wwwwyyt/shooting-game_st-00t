#include "tween.h"

static float linear(float t)
{
        return t;
}

static float quadEaseIn(float t)
{
        return t * t;
}

static float quadEaseOut(float t)
{
        return 1 - (1 - t) * (1 - t);
}

static float quadEaseInOut(float t)
{
        return t < 0.5 ? (2 * t * t) : (1 - SDL_powf(-2 * t + 2, 2) / 2);
}

static float cubicEaseIn(float t)
{
        return t * t * t;
}

static float cubicEaseOut(float t)
{
        return 1 - SDL_powf(1 - t, 3);
}

static float cubicEaseInOut(float t)
{
        return t < 0.5 ? (4 * t * t * t) : (1 - SDL_powf(-2 * t + 2, 3) / 2);
}

static float sineEaseInOut(float t)
{
        return -(SDL_cosf(SDL_PI_F * t) - 1) / 2;
}

static float expoEaseIn(float t)
{
        return t == 0 ? 0 : SDL_powf(2, 10 * (t - 1));
}

static float expoEaseOut(float t)
{
        return t == 1 ? 1 : 1 - SDL_powf(2, -10 * t);
}

static float elasticEaseOut(float t)
{
        float c4 = 2 * SDL_PI_F / 3;
        return t == 0 ? 0 : t == 1 ? 1
                                   : SDL_powf(2, -10 * t) * SDL_sinf((t * 10 - 0.75) * c4) + 1;
}

static float lerp(float start, float end, float t,
                  float (*easingFunc)(float t))
{
        return start + (end - start) * easingFunc(t);
}

static float (*easingFunc[])(float) = {
    linear,
    quadEaseIn,
    quadEaseOut,
    quadEaseInOut,
    cubicEaseIn,
    cubicEaseOut,
    cubicEaseInOut,
    sineEaseInOut,
    expoEaseIn,
    expoEaseOut,
    elasticEaseOut,
};

void TWEEN_Create(Tween *tween,
                  FVec2 a, FVec2 z, FVec2 *y,
                  EaseType ease, float duration)
{
        if (tween)
        {
                tween->a = a;
                tween->z = z;
                tween->y = y;
                tween->ease = ease;
                tween->duration = duration;
        }
}

void TWEEN_Update(Tween *tween, float dt)
{
        FVec2 a = tween->a;
        FVec2 z = tween->z;
        FVec2 *y = tween->y;
        float t = tween->t;
        float T = tween->duration;
        EaseType i = tween->ease;
        if (t < T)
        {
                y->x = lerp(a.x, z.x, t / T, easingFunc[i]);
                y->y = lerp(a.y, z.y, t / T, easingFunc[i]);
                tween->t += dt;
        }
}
