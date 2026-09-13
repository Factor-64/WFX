#ifndef ENTITY3D_H
#define ENTITY3D_H

#include <tonc.h>
#include <limits.h>
#include "global.h"

enum EntityState {
    ES_NULL,
    ES_Exploding,
    ES_GotoTarget,
    ES_Moving,
    ES_Stop
};

#define ENTITY3D_DEFAULT \
(Entity3d){ \
    .id = -1, \
    .num_vertices = 0, \
    .num_edges = 0, \
    .model_vertices = NULL, \
    .model_edges = NULL, \
    .bounding_radius = 0, \
    .roll = 0, \
    .pitch = 0, \
    .yaw = 0, \
    .x = 0, \
    .y = 0, \
    .z = FX(1), \
    .dx = 0, \
    .dy = 0, \
    .dz = 0, \
    .target = {INT_MAX,INT_MAX,INT_MAX}, \
	.health = INT_MAX, \
    .damage = 0, \
    .state = ES_NULL, \
    .timer = 0, \
    .i_frames = 0, \
    .color = 0, \
    .owner = -1, \
    .scale = FX(1), \
    .hitbox_max_x = 0, \
    .hitbox_max_y = 0, \
    .hitbox_max_z = 0, \
    .hitbox_min_x = 0, \
    .hitbox_min_y = 0, \
    .hitbox_min_z = 0 \
}

typedef struct ExplodeVert {
    fx vx, vy, vz;
    fx dx, dy, dz;
    int active;
} ExplodeVert;

typedef struct Entity3d {
    u32 id;
    u32 num_vertices;
    u32 num_edges;
    const s16 (*model_vertices)[3];
    const u8 (*model_edges)[2];
    fx bounding_radius;
    fx roll;
    fx pitch;
    fx yaw;
    fx x;
    fx y;
    fx z;
    fx dx;
    fx dy;
    fx dz;
    fx target[3];
	int health;
    int damage;
    u8 color;
    u8 old_color;
    int state;
    int timer;
    int i_frames;
    ExplodeVert explodeVerts[32];
    int owner;
    fx scale;
    fx hitbox_max_x;
    fx hitbox_max_y;
    fx hitbox_max_z;
    fx hitbox_min_x;
    fx hitbox_min_y;
    fx hitbox_min_z;
} Entity3d;

typedef struct EntityTemplate {
    int id;
    u32 num_vertices;
    u32 num_edges;
    const s16 (*model_vertices)[3];
    const u8 (*model_edges)[2];
    fx bounding_radius;
    int color;
    fx dx, dy, dz;
    fx hitbox_min_x, hitbox_min_y, hitbox_min_z;
    fx hitbox_max_x, hitbox_max_y, hitbox_max_z;
    int health;
    int damage;
} EntityTemplate;

enum EnemySpawnDirection {
    ESD_Up,
    ESD_Down,
    ESD_Left,
    ESD_Right,
    ESD_MAX
};

enum EnemySpawnLocation {
    ESL_Far,
    ESL_MidField,
    ESL_Behind,
    ESL_MAX
};

void spawn_entity(Entity3d* objs, fx x, fx y, fx z, int model);
void apply_entity_template(Entity3d* e, const EntityTemplate* t);
void follow_target(Entity3d* chaser, Entity3d* target);
void spawn_enemy(Entity3d* objs, enum EnemySpawnDirection esd, 
    enum EnemySpawnLocation esl, fx x, fx y, int model);

#endif