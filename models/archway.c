#include "archway.h"

const int NUM_VERTICES_archway = 8;
const int NUM_EDGES_archway = 12;

const s16 model_vertices_archway[][3] = {
    { 72, 19, 19 },
    { 72, 19, -19 },
    { 120, -19, 19 },
    { 120, -19, -19 },
    { -72, 19, 19 },
    { -72, 19, -19 },
    { -120, -19, 19 },
    { -120, -19, -19 },
};

const u8 model_edges_archway[][2] = {
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

const EntityTemplate archway_template = {
    .id = 0,
    .num_vertices = NUM_VERTICES_archway,
    .num_edges = NUM_EDGES_archway,
    .model_vertices = model_vertices_archway,
    .model_edges = model_edges_archway,
    .bounding_radius = 121,
    .color = 3,
    .dx = 0,
    .dy = 0,
    .dz = 0,
    .hitbox_min_x = -120,
    .hitbox_min_y = -19,
    .hitbox_min_z = -19,
    .hitbox_max_x = 120,
    .hitbox_max_y = 19,
    .hitbox_max_z = 19,
    .health = INT_MAX,
    .damage = 10,
};
