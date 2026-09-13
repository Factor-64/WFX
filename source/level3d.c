#include "level3d.h"

static void apply(Level3d* level, const LevelPreset* p)
{
    if (p->mask & LM_Type)            level->type = p->data.type;
    if (p->mask & LM_FloorHeight)     level->floor_height = p->data.floor_height;
    if (p->mask & LM_CeilingHeight)   level->ceiling_height = p->data.ceiling_height;
    if (p->mask & LM_MaxWidth)        level->max_width = p->data.max_width;
    if (p->mask & LM_MinWidth)        level->min_width = p->data.min_width;
}

void apply_level_preset(Level3d* level, enum LevelType type)
{
    switch(type)
    {
        case LT_Space:          apply(level, &PRESET_SPACE); break;
        case LT_Ground:         apply(level, &PRESET_GROUND); break;
        case LT_Walls:          apply(level, &PRESET_WALLS); break;
        case LT_Wall_L:         apply(level, &PRESET_WALL_L); break;
        case LT_Wall_R:         apply(level, &PRESET_WALL_R); break;
        case LT_Valley:         apply(level, &PRESET_VALLEY); break;
        case LT_Inverse_Valley: apply(level, &PRESET_INVERSE_VALLEY); break;
        case LT_Tunnel:         apply(level, &PRESET_TUNNEL); break;
        case LT_Sky:            apply(level, &PRESET_SKY); break;
        case LT_Sky_Ground:     apply(level, &PRESET_SKY_GROUND); break;
        default: break;
    }
}