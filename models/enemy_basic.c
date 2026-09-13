#include "enemy_basic.h"

const int NUM_VERTICES_enemy_basic = 5;
const int NUM_EDGES_enemy_basic = 8;

const s16 model_vertices_enemy_basic[][3] = {
    { -19, 19, 20 },
    { -19, -19, 20 },
    { 19, -19, 20 },
    { 19, 19, 20 },
    { 0, 0, -80 },
};

const u8 model_edges_enemy_basic[][2] = {
    { 0, 1 },
    { 2, 4 },
    { 1, 2 },
    { 0, 4 },
    { 3, 4 },
    { 0, 3 },
    { 1, 4 },
    { 2, 3 },
};

const EntityTemplate enemy_basic_template = {
    .id = 4,
    .num_vertices = NUM_VERTICES_enemy_basic,
    .num_edges = NUM_EDGES_enemy_basic,
    .model_vertices = model_vertices_enemy_basic,
    .model_edges = model_edges_enemy_basic,
    .bounding_radius = 80,
    .color = 5,
    .dx = FX(1) >> 5,
    .dy = FX(1) >> 5,
    .dz = FX(1) >> 5,
    .hitbox_min_x = -19,
    .hitbox_min_y = -19,
    .hitbox_min_z = -80,
    .hitbox_max_x = 19,
    .hitbox_max_y = 19,
    .hitbox_max_z = 20,
    .health = 10,
    .damage = 5,
};
