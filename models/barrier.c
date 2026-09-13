#include "barrier.h"

const int NUM_VERTICES_barrier = 8;
const int NUM_EDGES_barrier = 12;

const s16 model_vertices_barrier[][3] = {
    { -240, -52, 0 },
    { 240, -52, 0 },
    { 240, 0, 52 },
    { -240, 0, 52 },
    { 240, 52, 0 },
    { -240, 52, 0 },
    { 240, 0, -52 },
    { -240, 0, -52 },
};

const u8 model_edges_barrier[][2] = {
    { 0, 1 },
    { 0, 7 },
    { 2, 4 },
    { 1, 2 },
    { 0, 3 },
    { 4, 6 },
    { 5, 7 },
    { 2, 3 },
    { 6, 7 },
    { 4, 5 },
    { 1, 6 },
    { 3, 5 },
};

const EntityTemplate barrier_template = {
    .id = 1,
    .num_vertices = NUM_VERTICES_barrier,
    .num_edges = NUM_EDGES_barrier,
    .model_vertices = model_vertices_barrier,
    .model_edges = model_edges_barrier,
    .bounding_radius = 245,
    .color = 3,
    .dx = 0,
    .dy = 0,
    .dz = 0,
    .hitbox_min_x = -240,
    .hitbox_min_y = -52,
    .hitbox_min_z = -52,
    .hitbox_max_x = 240,
    .hitbox_max_y = 52,
    .hitbox_max_z = 52,
    .health = INT_MAX,
    .damage = 10,
};
