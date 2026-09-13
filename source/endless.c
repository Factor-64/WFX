#include "endless.h"

void run_endless3d(Level3d* level, Entity3d* objs)
{
    static int last_segment = -1;
    int segment = level->current_position >> 10;
    if(segment != last_segment)
    {
        last_segment = segment;
        enum CellScript cs = rand() % CS_MAX;
        if(cs == CS_Spawn_Enemy3d)
            cs -= rand() & 1;
        else if(cs == CS_Change_LevelPreset)
            cs += rand() & 1;
        
        switch(cs)
        {
            default: break;
            case CS_Change_LevelProperty:
            case CS_Spawn_Entity3d: {
                int model = rand() % MODEL_MAX;
                if(level->type != LT_Space && (rand() & 1))
                {
                    if((rand() & 1))
                        model = MODEL_CUBOID;
                    else
                        model = MODEL_SPHERE;
                }
                int left = rand() & 1;
                int down = rand() & 1;

                fx minW = abs(level->min_width) >> 1;
                fx maxW = level->max_width >> 1;
                fx minH = abs(level->floor_height) >> 1;
                fx maxH = level->ceiling_height >> 1;

                if(minW > FX(1)) minW = FX(1);
                if(maxW > FX(1)) maxW = FX(1);
                if(minH > FX(1)) minH = FX(1);
                if(maxH > FX(1)) maxH = FX(1);

                fx x = rand() % (left ? minW : maxW);
                fx y = rand() % (down ? minH : maxH);

                if(left) x = -x;
                if(down) y = -y;

                spawn_entity(objs, x, y, FAR, model);
                break;
            }
            case CS_Spawn_Enemy3d: {
                enum EnemySpawnDirection esd = rand() % ESD_MAX;
                enum EnemySpawnLocation esl = rand() % ESL_MAX;
                int left = rand() & 1;
                int down = rand() & 1;
                fx minW = abs(level->min_width) >> 2;
                fx maxW = level->max_width >> 2;
                fx minH = abs(level->floor_height) >> 2;
                fx maxH = level->ceiling_height >> 2;

                if(minW > (FX(1) >> 1)) minW = FX(1) >> 2;
                if(maxW > (FX(1) >> 1)) maxW = FX(1) >> 2;
                if(minH > (FX(1) >> 1)) minH = FX(1) >> 2;
                if(maxH > (FX(1) >> 1)) maxH = FX(1) >> 2;

                fx x = rand() % (left ? minW : maxW);
                fx y = rand() % (down ? minH : maxH);

                if(left) x = -x;
                if(down) y = -y;
                int model = rand_range(MODEL_ENEMY_BASIC, MODEL_ENEMY_CRYSTAL);
                spawn_enemy(objs, esd, esl, x, y, model);
                break;
            }
            case CS_Change_LevelPreset: {
                if((rand() & 1))
                    return;
                enum LevelType lt = rand() % LT_MAX;
                apply_level_preset(level, lt);
                break;
            }
        }
    }
}