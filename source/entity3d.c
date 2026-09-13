#include "entity3d.h"
#include "global.h"
#include "models.h"
#include "level3d.h"

void apply_entity_template(Entity3d* e, const EntityTemplate* t)
{
    *e = ENTITY3D_DEFAULT;
    e->id = t->id;
    e->num_vertices = t->num_vertices;
    e->num_edges = t->num_edges;
    e->model_vertices = t->model_vertices;
    e->model_edges = t->model_edges;
    e->bounding_radius = t->bounding_radius;
    e->color = t->color;

    e->dx = t->dx;
    e->dy = t->dy;
    e->dz = t->dz;

    e->hitbox_min_x = t->hitbox_min_x;
    e->hitbox_min_y = t->hitbox_min_y;
    e->hitbox_min_z = t->hitbox_min_z;

    e->hitbox_max_x = t->hitbox_max_x;
    e->hitbox_max_y = t->hitbox_max_y;
    e->hitbox_max_z = t->hitbox_max_z;

    e->health = t->health;
    e->damage = t->damage;
}

void spawn_entity(Entity3d* objs, fx x, fx y, fx z, int model)
{
    for(int i = 0; i < ENTITY_ARRAY_SIZE; ++i)
    {
        Entity3d* ent = &objs[i];
        if(ent->id != -1) continue;
        switch(model)
        {
            default: return;
            case MODEL_ARCHWAY: apply_entity_template(ent, &archway_template); break;
            case MODEL_BARRIER: apply_entity_template(ent, &barrier_template); break;
            case MODEL_PILLAR:  apply_entity_template(ent, &pillar_template); break;
            case MODEL_WALL:    apply_entity_template(ent, &wall_template); break;
            case MODEL_PILLAR2: apply_entity_template(ent, &pillar2_template); break;
            case MODEL_MINE:    apply_entity_template(ent, &mine_template); break;
            case MODEL_RING:    apply_entity_template(ent, &ring_template); break;
            case MODEL_POWERUP: apply_entity_template(ent, &powerup_template); break;
            case MODEL_SPHERE: {
                apply_entity_template(ent, &sphere_template);
                ent->target[0] = rand() % FX(24);
                ent->target[1] = rand() % FX(24);
                ent->target[2] = rand() % FX(24);
                ent->scale = rand() % FX(2);
                break;
            }
            case MODEL_CUBOID: { 
                apply_entity_template(ent, &cuboid_template);
                ent->target[0] = rand() % FX(24);
                ent->target[1] = rand() % FX(24);
                ent->target[2] = rand() % FX(24);
                ent->scale = rand() % FX(2);
                break;
            }
        }
        ent->x = x;
        ent->y = y;
        ent->z = z;
        ent->owner = -1;
        if(model == MODEL_ARCHWAY)
        {
            int idx = -1;

            for(int j = i; j < ENTITY_ARRAY_SIZE; ++j)
            {
                Entity3d* ent2 = &objs[j];
                if(ent2->id != -1) continue;

                apply_entity_template(ent2, &pillar_template);
                ent2->x = ent->x;
                ent2->z = ent->z;
                ent2->color = ent->color;
                ent2->owner = i;

                if(idx == -1)
                {
                    ent2->x += ent->hitbox_min_x + 20;
                    idx = j;
                }
                else
                {
                    ent2->x += ent->hitbox_max_x - 20;
                    break;
                }
            }
            ent->y = objs[idx].hitbox_max_y + 20;
        }
        return;
    }
}

void spawn_enemy(Entity3d* objs, enum EnemySpawnDirection esd, 
    enum EnemySpawnLocation esl, fx x, fx y, int model)
{
    for(int i = 0; i < ENTITY_ARRAY_SIZE; ++i)
    {
        Entity3d* ent = &objs[i];
        if(ent->id != -1) continue;
        switch(model)
        {
            default: return;
            case MODEL_ENEMY_BASIC:   apply_entity_template(ent, &enemy_basic_template); break;
            case MODEL_ENEMY_CRYSTAL: apply_entity_template(ent, &enemy_crystal_template); break;
        }
        switch(esd)
        {
            case ESD_Up:
                ent->y = BOUND_UP;
                break;
            case ESD_Down:
                ent->y = BOUND_DOWN;
                break;
            case ESD_Right:
                ent->x = BOUND_RIGHT;
                break;
            case ESD_Left:
                ent->x = BOUND_LEFT;
                break;
            default:
                break;
        }
        ent->target[0] = x;
        ent->target[1] = y;
        switch(esl)
        {
            case ESL_Behind:
                ent->z = FX(2);
                //ent->dz = -ent->dz;
                ent->yaw = FX(128);
                break;
            case ESL_Far:
                ent->z = FAR;
                ent->dz = -ent->dz;
                break;
            case ESL_MidField:
                ent->z = FX(6);
                ent->dz = -ent->dz;
                break;
            default:
                break;
        }
        ent->target[2] = ent->z;
        ent->dx = (ent->target[0] > ent->x) ? ent->dx : -ent->dx;
        ent->dy = (ent->target[1] > ent->y) ? ent->dy : -ent->dy;
        ent->owner = i;
        ent->state = ES_GotoTarget;
        return;
    }
}       

void follow_target(Entity3d* chaser, Entity3d* target)
{
    fx dz = target->z - chaser->z;

    if (dz > 0)
        chaser->dz = chaser->dz;
    else if (dz < 0)
        chaser->dz = -chaser->dz;
}


