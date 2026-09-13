#include "mine.h"

const int NUM_VERTICES_mine = 10;
const int NUM_EDGES_mine = 24;

const s16 model_vertices_mine[][3] = {
    { 0, -1, 0 },
    { 0, 17, 0 },
    { 11, -1, 28 },
    { 28, -1, 11 },
    { 28, -1, -11 },
    { 11, -1, -28 },
    { -11, -1, -28 },
    { -28, -1, -11 },
    { -28, -1, 11 },
    { -11, -1, 28 },
};

const u8 model_edges_mine[][2] = {
    { 3, 4 },
    { 0, 2 },
    { 8, 9 },
    { 0, 5 },
    { 1, 6 },
    { 0, 8 },
    { 1, 3 },
    { 1, 9 },
    { 4, 5 },
    { 5, 6 },
    { 0, 7 },
    { 1, 2 },
    { 0, 4 },
    { 1, 5 },
    { 1, 8 },
    { 6, 7 },
    { 0, 3 },
    { 0, 9 },
    { 1, 4 },
    { 0, 6 },
    { 2, 3 },
    { 2, 9 },
    { 1, 7 },
    { 7, 8 },
};

const EntityTemplate mine_template = {
    .id = 7,
    .num_vertices = NUM_VERTICES_mine,
    .num_edges = NUM_EDGES_mine,
    .model_vertices = model_vertices_mine,
    .model_edges = model_edges_mine,
    .bounding_radius = 30,
    .color = 6,
    .dx = 0,
    .dy = 0,
    .dz = 0,
    .hitbox_min_x = -28,
    .hitbox_min_y = -1,
    .hitbox_min_z = -28,
    .hitbox_max_x = 28,
    .hitbox_max_y = 17,
    .hitbox_max_z = 28,
    .health = 1,
    .damage = 20,
};
