#include "laser.h"

const int NUM_VERTICES_laser = 4;
const int NUM_EDGES_laser = 6;

const s16 model_vertices_laser[][3] = {
    { -14, -2, 14 },
    { 0, 7, 13 },
    { 0, -2, -42 },
    { 14, -2, 14 },
};

const u8 model_edges_laser[][2] = {
    { 0, 1 },
    { 1, 2 },
    { 0, 3 },
    { 2, 3 },
    { 0, 2 },
    { 1, 3 },
};

const EntityTemplate laser_template = {
    .id = 6,
    .num_vertices = NUM_VERTICES_laser,
    .num_edges = NUM_EDGES_laser,
    .model_vertices = model_vertices_laser,
    .model_edges = model_edges_laser,
    .bounding_radius = 42,
    .color = 1,
    .dx = 0,
    .dy = 0,
    .dz = FX(1) >> 3,
    .hitbox_min_x = -14,
    .hitbox_min_y = -2,
    .hitbox_min_z = -42,
    .hitbox_max_x = 14,
    .hitbox_max_y = 7,
    .hitbox_max_z = 14,
    .health = INT_MAX,
    .damage = 5,
};
