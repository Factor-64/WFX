#include "enemylogic.h"
#include "models.h"
#include "sound.h"
#include "trig_lut.h"
#include "level3d.h"

static int reached(int pos, int vel, int target) 
{
    if (target == INT_MAX) return 1;
    return (vel >= 0 ? pos >= target : pos <= target);
}

static void spawn_projectile(Entity3d* enemy, Entity3d* player, Entity3d* proj, int model, int follow)
{
    for(int i = 0; i < ENTITY_ARRAY_SIZE; ++i)
    {
        Entity3d* ent = &proj[i];
        if(ent->id != -1) continue;
        switch(model)
        {
            default: return;
            case MODEL_LASER: apply_entity_template(ent, &laser_template); break;
            case MODEL_BEAM:  apply_entity_template(ent, &beam_template); break;
        }
        ent->x = enemy->x;
        ent->y = enemy->y;
        if(enemy->dz > 0)
        {
            ent->z = enemy->z - enemy->bounding_radius;
            ent->dz = -ent->dz;
        }
        else
        {
            ent->z = enemy->z + enemy->bounding_radius;
        }
        if(follow)
            follow_target(ent, player);
        ent->pitch = enemy->pitch;
        ent->yaw = enemy->yaw;
        ent->roll = enemy->roll;
        ent->color = enemy->color;
        ent->owner = enemy->owner;
        
        sfx_play(SFX_LASER1, 1, 255);
        break;
    }
}

static inline void basic_enemy(Entity3d* enemy, Entity3d* player, Entity3d* proj)
{
    if(enemy->target[0] == INT_MAX)
    {
        if(enemy->z > FX(1))
        {
            spawn_projectile(enemy, player, proj, MODEL_LASER, 0);
            enemy->target[0] -= 100;
        }
    }
    else
    {
        ++enemy->target[0];
    }
}

static inline void basic_crystal(Entity3d* enemy, Entity3d* player, Entity3d* proj)
{
    if(enemy->target[0] == INT_MAX - 32)
    {
        if(enemy->roll == FX(64))
        {
            spawn_projectile(enemy, player, proj, MODEL_BEAM, 1);
            enemy->roll += FX(16);
            return;
        }
        else
        {
            enemy->roll += FX(16);
            if(enemy->roll == FX(256))
            {
                enemy->roll = 0;
                enemy->state = ES_GotoTarget;
                int left = rand() & 1;
                int down = rand() & 1;
                int forward = rand() & 1;

                fx x = rand() % (FX(1) >> 2);
                fx y = rand() % (FX(1) >> 2);
                fx z = rand() % (FX(1) >> 2);

                if(enemy->x >= FX(1))
                    left = 1;
                else if(enemy->x <= FX(-1))
                    left = 0;
                if(enemy->y >= FX(1))
                    down = 1;
                else if(enemy->y <= FX(-1))
                    down = 0;
                if(enemy->z >= FX(6))
                    forward = 1;
                else if(enemy->z <= FX(1))
                    forward = 0;
                if(left) x = -x;    
                if(down) y = -y;
                if(forward) z = -z;
                enemy->target[0] = x;
                enemy->target[1] = y;
                enemy->target[2] = z;
                
                enemy->dx = (enemy->target[0] > enemy->x) ? enemy->dx : -enemy->dx;
                enemy->dy = (enemy->target[1] > enemy->y) ? enemy->dy : -enemy->dy;
                enemy->dz = (enemy->target[2] > enemy->z) ? enemy->dz : -enemy->dz;
            }
            return;
        }
    }
    else
    {
        --enemy->target[0];
    }
    if(enemy->timer > 96)
    {
        int left = rand() & 1;
        int xyz = rand() & 3;
        if(xyz == 3)
            --xyz;
        fx x = INT_MAX;

        if(left) x = -x;
        enemy->target[xyz] = x;
        enemy->state = ES_GotoTarget;
    }
    else
    {
        ++enemy->timer;
    }
}

static inline void setup_enemy(Entity3d* enemy)
{
    switch(enemy->id)
    {
        default: break;
        case MODEL_ENEMY_CRYSTAL:
            if(enemy->timer <= 0)
            {
                enemy->state = ES_Stop;
                enemy->timer = 0;
            }
            break;
        case MODEL_ENEMY_BASIC:
            enemy->dx = 0;
            enemy->dy = 0;
            break;
    }
}

static inline void run_enemy_logic(Entity3d* enemy, Entity3d* player, Entity3d* proj)
{
    switch(enemy->id)
    {
        default: break;
        case MODEL_ENEMY_CRYSTAL:
            basic_crystal(enemy, player, proj);
            break;
        case MODEL_ENEMY_BASIC:
            basic_enemy(enemy, player, proj);
            break;
    }
}

void handle_enemy_logic(Entity3d* enemy, Entity3d* player, Entity3d* proj)
{
    switch(enemy->state)
    {
        case ES_GotoTarget: {
            int x_done = reached(enemy->x, enemy->dx, enemy->target[0]);
            int y_done = reached(enemy->y, enemy->dy, enemy->target[1]);
            int z_done = reached(enemy->z, enemy->dz, enemy->target[2]);

            if(!x_done)
                enemy->x += enemy->dx;
            if(!y_done)
                enemy->y += enemy->dy;
            if(!z_done)
                enemy->z += enemy->dz;

            if(x_done && y_done && z_done)
            {
                enemy->state = ES_Moving;
                enemy->target[0] = INT_MAX;
                enemy->target[1] = INT_MAX;
                enemy->target[2] = INT_MAX;
                setup_enemy(enemy);
            }
            return;
        }
        case ES_Moving:
            enemy->x += enemy->dx;
            enemy->y += enemy->dy;
            enemy->z += enemy->dz;
            break;
        default:
            break;
    }
    run_enemy_logic(enemy, player, proj);
}
