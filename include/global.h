#ifndef GLOBAL_H
#define GLOBAL_H

#include <tonc.h>
#include <stdlib.h>

#define ALIGN4 __attribute__((aligned(4)))
#define FX(x) ((x) << FIX_SHIFT)

typedef FIXED fx;
#define MUL240(y) (((y) << 8) - ((y) << 4))

typedef struct { fx m[3][3]; } Mat3;
static inline fx abs_fx(fx v) { return v < 0 ? -v : v; }

#define ENTITY_ARRAY_SIZE 32
#define NUM_FLOOR_LINES 8
#define NEAR FX(1)
#define FAR FX(NUM_FLOOR_LINES)

#define RECIP_TAB_SIZE 512
extern const fx recip_tab[RECIP_TAB_SIZE];

enum GameState {
    GS_Menu,
    GS_init2d,
    GS_init3d,
    GS_run2d,
    GS_attract3d,
    GS_model_view3d,
    GS_run3d,
};

extern enum GameState current_state;

extern u32 frame_count;

static inline fx fast_fxdiv(fx num, fx den)
{
    if(den < 1) den = 1;

    int shift = 0;
    while(den >= 512)
    {
        den >>= 1;
        ++shift;
    }

    fx inv = recip_tab[den];
    fx result = fxmul(num, inv);

    return result >> shift;
}

static inline int rand_range(int min, int max)
{
    return min + (rand() % (max - min + 1));
}

#endif