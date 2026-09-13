#include "pillar.h"

const int NUM_VERTICES_pillar = 8;
const int NUM_EDGES_pillar = 12;

const s16 model_vertices_pillar[][3] = {
    { 19, 43, -19 },
    { 19, 43, 19 },
    { 19, -43, 19 },
    { 19, -43, -19 },
    { -19, -43, 19 },
    { -19, 43, 19 },
    { -19, 43, -19 },
    { -19, -43, -19 },
};

const u8 model_edges_pillar[][2] = {
    { 0, 1 },
    { 2, 4 },
    { 1, 2 },
    { 1, 5 },
    { 3, 7 },
    { 0, 3 },
    { 0, 6 },
    { 2, 3 },
    { 6, 7 },
    { 4, 5 },
    { 5, 6 },
    { 4, 7 },
};

const EntityTemplate pillar_template = {
    .id = 8,
    .num_vertices = NUM_VERTICES_pillar,
    .num_edges = NUM_EDGES_pillar,
    .model_vertices = model_vertices_pillar,
    .model_edges = model_edges_pillar,
    .bounding_radius = 26,
    .color = 3,
    .dx = 0,
    .dy = 0,
    .dz = 0,
    .hitbox_min_x = -19,
    .hitbox_min_y = -43,
    .hitbox_min_z = -19,
    .hitbox_max_x = 19,
    .hitbox_max_y = 43,
    .hitbox_max_z = 19,
    .health = INT_MAX,
    .damage = 10,
};
