#ifndef LEVEL3D_H
#define LEVEL3D_H

#include <tonc.h>
#include "global.h"
#include "entity3d.h"

#define BOUND_LEFT FX(-2)
#define BOUND_RIGHT FX(2)
#define BOUND_UP FX(2)
#define BOUND_DOWN FX(-2)

enum LevelType {
    LT_Space,
    LT_Ground,
    LT_Walls,
    LT_Wall_L,
    LT_Wall_R,
    LT_Valley,
    LT_Inverse_Valley,
    LT_Tunnel,
    LT_Sky,
    LT_Sky_Ground,
    LT_MAX
};

enum LevelProperty {
    LP_Type,
    LP_Floor_Height,
    LP_Ceiling_Height,
    LP_Speed,
    LP_Turn,
    LP_MAX
};

enum CellScript {
    CS_NULL,
    CS_Spawn_Entity3d,
    CS_Spawn_Enemy3d,
    CS_Change_LevelProperty,
    CS_Change_LevelPreset,
    CS_MAX
};

enum LevelMask {
    LM_Type            = 1 << 0,
    LM_FloorHeight     = 1 << 1,
    LM_CeilingHeight   = 1 << 2,
    LM_MaxWidth        = 1 << 3,
    LM_MinWidth        = 1 << 4,
};

typedef struct LevelCell {
    fx coord;
    int data[8];
} LevelCell;

typedef struct Level3d {
    enum LevelType type;
    fx max_player_x;
    fx min_player_x;
    fx max_player_y;
    fx min_player_y;
    fx max_camera_x;
    fx min_camera_x;
    fx max_camera_y;
    fx min_camera_y;
    fx max_player_check_x;
    fx min_player_check_x;
    fx max_player_check_y;
    fx min_player_check_y;
    fx floor_height;
    fx ceiling_height;
    fx max_width;
    fx min_width;
    fx speed;
    fx turn;
    fx current_position;
    int bg_color;
    int floor_color;
    int ceiling_color;
    int wall_l_color;
    int wall_r_color;
    int current_cell;
    int current_obj;
    LevelCell data[64];
    Entity3d objs[64];
} Level3d;

typedef struct LevelPreset {
    Level3d data;
    uint32_t mask;
} LevelPreset;

#define LEVELCELL_DEFAULT \
(LevelCell){ \
    .coord = 0, \
    .data = { 0,0,0,0,0,0,0,0 } \
}

#define LEVEL3D_DEFAULT \
(Level3d){ \
    .type = LT_Ground, \
    .max_player_x = (FX(1) >> 1), \
    .min_player_x = (FX(-1) >> 1), \
    .max_player_y = (FX(1) >> 2) + (FX(1) >> 3), \
    .min_player_y = (FX(-1) >> 2) + (FX(-1) >> 3), \
    .max_camera_x = (FX(1) >> 3) + (FX(1) >> 4), \
    .min_camera_x = (FX(-1) >> 3) + (FX(-1) >> 4), \
    .max_camera_y = (FX(1) >> 3) + (FX(1) >> 4), \
    .min_camera_y = 0, \
    .max_player_check_x = (FX(1) >> 2), \
    .min_player_check_x = (FX(-1) >> 2), \
    .max_player_check_y = (FX(1) >> 2), \
    .min_player_check_y = (FX(-1) >> 4), \
    .floor_height = FX(-1), \
    .ceiling_height = 0, \
    .max_width = FX(64), \
    .min_width = FX(-64), \
    .speed = FX(1) >> 2, \
    .turn = 0, \
    .bg_color = 0, \
    .floor_color = 2, \
    .ceiling_color = 2, \
    .wall_l_color = 2, \
    .wall_r_color = 2, \
    .current_position = 0, \
    .current_cell = 0, \
    .current_obj = 0, \
    .data = { 0 }, \
    .objs = { ENTITY3D_DEFAULT } \
}

static const LevelPreset PRESET_SPACE = {
    .data = {
        .type = LT_Space,
        .floor_height = 0,
        .ceiling_height = 0,
        .max_width = FX(64),
        .min_width = FX(-64),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_GROUND = {
    .data = {
        .type = LT_Ground,
        .floor_height = FX(-1) >> 1,
        .ceiling_height = 0,
        .max_width = FX(64),
        .min_width = FX(-64),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_WALLS = {
    .data = {
        .type = LT_Walls,
        .floor_height = FX(-2),
        .ceiling_height = FX(2),
        .max_width = FX(6),
        .min_width = FX(-6),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_WALL_L = {
    .data = {
        .type = LT_Wall_L,
        .floor_height = FX(-2),
        .ceiling_height = FX(2),
        .max_width = FX(64),
        .min_width = FX(-6),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_WALL_R = {
    .data = {
        .type = LT_Wall_R,
        .floor_height = FX(-2),
        .ceiling_height = FX(2),
        .max_width = FX(6),
        .min_width = FX(-64),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_VALLEY = {
    .data = {
        .type = LT_Valley,
        .floor_height = FX(-1),
        .ceiling_height = FX(2),
        .max_width = FX(6),
        .min_width = FX(-6),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_INVERSE_VALLEY = {
    .data = {
        .type = LT_Inverse_Valley,
        .floor_height = FX(-1),
        .ceiling_height = FX(2),
        .max_width = FX(6),
        .min_width = FX(-6),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth |LM_MinWidth
};

static const LevelPreset PRESET_TUNNEL = {
    .data = {
        .type = LT_Tunnel,
        .floor_height = FX(-2),
        .ceiling_height = FX(2),
        .max_width = FX(6),
        .min_width = FX(-6),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_SKY = {
    .data = {
        .type = LT_Sky,
        .floor_height = 0,
        .ceiling_height = FX(2),
        .max_width = FX(64),
        .min_width = FX(-64),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

static const LevelPreset PRESET_SKY_GROUND = {
    .data = {
        .type = LT_Sky_Ground,
        .floor_height = FX(-1),
        .ceiling_height = FX(2),
        .max_width = FX(64),
        .min_width = FX(-64),
    },
    .mask = LM_Type | LM_FloorHeight | LM_CeilingHeight | LM_MaxWidth | LM_MinWidth
};

void apply_level_preset(Level3d* level, const enum LevelType type);

#endif