#include "wall.h"

const int NUM_VERTICES_wall = 8;
const int NUM_EDGES_wall = 12;

const s16 model_vertices_wall[][3] = {
    { -120, -120, -24 },
    { -120, 120, -24 },
    { 120, 120, -24 },
    { 120, -120, -24 },
    { -120, 120, 24 },
    { 120, 120, 24 },
    { -120, -120, 24 },
    { 120, -120, 24 },
};

const u8 model_edges_wall[][2] = {
    { 0, 1 },
    { 1, 2 },
    { 3, 7 },
    { 0, 3 },
    { 4, 6 },
    { 1, 4 },
    { 5, 7 },
    { 2, 3 },
    { 6, 7 },
    { 4, 5 },
    { 0, 6 },
    { 2, 5 },
};

const EntityTemplate wall_template = {
    .id = 14,
    .num_vertices = NUM_VERTICES_wall,
    .num_edges = NUM_EDGES_wall,
    .model_vertices = model_vertices_wall,
    .model_edges = model_edges_wall,
    .bounding_radius = 122,
    .color = 3,
    .dx = 0,
    .dy = 0,
    .dz = 0,
    .hitbox_min_x = -120,
    .hitbox_min_y = -120,
    .hitbox_min_z = -24,
    .hitbox_max_x = 120,
    .hitbox_max_y = 120,
    .hitbox_max_z = 24,
    .health = INT_MAX,
    .damage = 10,
};
