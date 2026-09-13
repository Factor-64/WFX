#include "beam.h"

const int NUM_VERTICES_beam = 8;
const int NUM_EDGES_beam = 12;

const s16 model_vertices_beam[][3] = {
    { 0, 13, 26 },
    { 13, 0, 26 },
    { 0, 13, -26 },
    { 13, 0, -26 },
    { -13, 0, 26 },
    { 0, -13, 26 },
    { -13, 0, -26 },
    { 0, -13, -26 },
};

const u8 model_edges_beam[][2] = {
    { 0, 1 },
    { 0, 4 },
    { 1, 5 },
    { 3, 7 },
    { 4, 6 },
    { 5, 7 },
    { 2, 3 },
    { 6, 7 },
    { 0, 2 },
    { 4, 5 },
    { 2, 6 },
    { 1, 3 },
};

const EntityTemplate beam_template = {
    .id = 2,
    .num_vertices = NUM_VERTICES_beam,
    .num_edges = NUM_EDGES_beam,
    .model_vertices = model_vertices_beam,
    .model_edges = model_edges_beam,
    .bounding_radius = 29,
    .color = 1,
    .dx = 0,
    .dy = 0,
    .dz = FX(1) >> 3,
    .hitbox_min_x = -13,
    .hitbox_min_y = -13,
    .hitbox_min_z = -26,
    .hitbox_max_x = 13,
    .hitbox_max_y = 13,
    .hitbox_max_z = 26,
    .health = INT_MAX,
    .damage = 10,
};
