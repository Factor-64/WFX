#include <tonc.h>
#include "global.h"
#include "trig_lut.h"
#include "models.h"
#include "camera.h"
#include "level3d.h"
#include "sound.h"
#include "message3d.h"
#include "endless.h"
#include "enemylogic.h"
#include <string.h>

#define MIN_X 0
#define MAX_X 239
#define MIN_Y 0
#define MAX_Y 159

#define INSIDE 0
#define LEFT   1
#define RIGHT  2
#define BOTTOM 4
#define TOP    8

#define PROJ_SCALE 180

#define BARREL_ROLL_FRAMES 30
#define BARREL_ROLL_SPEED FX(8)
#define BARREL_ROLL_MAX FX(255)
#define BARREL_ROLL_MIN FX(-255)
#define MAX_H_SPEED (FX(1) + (FX(1) >> 2))
#define MIN_H_SPEED (FX(-1) + (FX(-1) >> 2))
#define MAX_ROLL_H_SPEED (FX(1) + (FX(1) >> 2))
#define MIN_ROLL_H_SPEED (FX(-1) + (FX(-1) >> 2))
#define MAX_V_SPEED (FX(1) + (FX(1) >> 1))
#define MIN_V_SPEED (FX(-1) + (FX(-1) >> 1))

#define PITCH_MAX FX(6)
#define PITCH_MIN FX(-6)
#define ADJUST_SPEED (FX(1) >> 4)
#define ROLL_MAX FX(64)
#define ROLL_MIN FX(-64)
#define CAMERA_PITCH_MAX FX(3)
#define CAMERA_PITCH_MIN FX(-3)
#define ROCK_LIMIT FX(4)

#define MAX_LASERS 6
#define LASER_WAIT 8
#define PLAYER_I_FRAMES 20

EWRAM_DATA static u8 backbuf[240 * 160];
EWRAM_DATA static Entity3d obj3d[ENTITY_ARRAY_SIZE];
EWRAM_DATA static Entity3d pro3d[ENTITY_ARRAY_SIZE];

EWRAM_DATA fx floor_lines[NUM_FLOOR_LINES][3];
EWRAM_DATA fx ceiling_lines[NUM_FLOOR_LINES][3];
EWRAM_DATA fx wall_lines_l[NUM_FLOOR_LINES][3];
EWRAM_DATA fx wall_lines_r[NUM_FLOOR_LINES][3];

EWRAM_DATA static Level3d level = LEVEL3D_DEFAULT;

int rolling = 0;
int banking = 0;
int barrel_roll = 0;
u32 last_r_roll = -BARREL_ROLL_FRAMES;
u32 last_l_roll = -BARREL_ROLL_FRAMES;
u32 score = 0;
int barrel_roll_wait = 0;
int laser_wait = 0;
fx speed_x = 0;
fx speed_y = 0;
int paused = 0;
int bucket = 0;
int laser_count = 0;
int dead = 0;
int invert = 0;
int powerup_state = 0;
int powerup_health = 0;

IWRAM_CODE int clip_line(int *x0, int *y0, int *x1, int *y1)
{
    int dx = *x1 - *x0;
    int dy = *y1 - *y0;

    int t0 = 0; 
    int t1 = 1 << 14; 

    int p[4] = { -dx, dx, -dy, dy };
    int q[4] = { *x0 - MIN_X, MAX_X - *x0, *y0 - MIN_Y, MAX_Y - *y0 };

    for(int i = 0; i < 4; ++i) 
    {
        if (p[i] == 0) 
        {
            if (q[i] < 0) 
                return 0;
        } 
        else 
        {
            int t = (q[i] << 14) / p[i];
            if (p[i] < 0) 
            {
                if (t > t0) 
                    t0 = t;
            } 
            else 
            {
                if (t < t1) 
                    t1 = t;
            }
        }
    }

    if (t0 > t1) return 0;

    int nx0 = *x0, ny0 = *y0;
    int nx1 = *x1, ny1 = *y1;

    if(t1 < (1 << 14)) 
    {
        nx1 = *x0 + (dx * t1) / (1 << 14);
        ny1 = *y0 + (dy * t1) / (1 << 14);
    }
    
    if(t0 > 0) 
    {
        nx0 = *x0 + (dx * t0) / (1 << 14);
        ny0 = *y0 + (dy * t0) / (1 << 14);
    }

    if (nx0 < MIN_X) nx0 = MIN_X; 
    else if (nx0 > MAX_X) nx0 = MAX_X;
    if (ny0 < MIN_Y) ny0 = MIN_Y; 
    else if (ny0 > MAX_Y) ny0 = MAX_Y;
    
    if (nx1 < MIN_X) nx1 = MIN_X; 
    else if (nx1 > MAX_X) nx1 = MAX_X;
    if (ny1 < MIN_Y) ny1 = MIN_Y; 
    else if (ny1 > MAX_Y) ny1 = MAX_Y;

    *x0 = nx0; *y0 = ny0;
    *x1 = nx1; *y1 = ny1;

    return 1;
}

IWRAM_CODE void line(int x0, int y0, int x1, int y1, int clr_idx)
{
    if((unsigned)x0 > MAX_X || (unsigned)y0 > MAX_Y ||
       (unsigned)x1 > MAX_X || (unsigned)y1 > MAX_Y)
    {
        if(!clip_line(&x0, &y0, &x1, &y1))
            return;
    }

    if(y0 == y1) 
    {
        if(x0 > x1) { int t = x0; x0 = x1; x1 = t; }
        memset(backbuf + (MUL240(y0)) + x0, clr_idx, x1 - x0 + 1);
        return;
    }
    if(x0 == x1) 
    {
        u8 *p = backbuf + (MUL240(y0)) + x0;
        int step = (y1 > y0) ? 240 : -240;
        int count = abs(y1 - y0) + 1;
        while(count--) 
        { 
            *p = clr_idx; 
            p += step; 
        }
        return;
    }

    int dx=abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy=-abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    u8 *ptr = backbuf + (MUL240(y0)) + x0;
    int row_step = (MUL240(sy));

    while(1)
    {
        *ptr = clr_idx;
            
        if(x0 == x1 && y0 == y1) break;

        int e2 = err<<1;
        if(e2 >= dy) { err += dy; x0 += sx; ptr += sx; }
        if(e2 <= dx) { err += dx; y0 += sy; ptr += row_step; }
    }
}

static inline void putpixel(int x, int y, int color)
{
    if((unsigned)x < 240 && (unsigned)y < 160)
        backbuf[(MUL240(y)) + x] = color;
}

static inline void putpixel_size(int x, int y, int size, int color)
{
    int half = size >> 1;

    for(int dy = -half; dy <= half; dy++) 
    {
        for(int dx = -half; dx <= half; dx++) 
        {
            int px = x + dx;
            int py = y + dy;
            if((unsigned)px < 240 && (unsigned)py < 160)
                backbuf[(MUL240(py)) + px] = color;
        }
    }
}

static inline void rotate_x(fx *x, fx *y, fx *z, int angle)
{
    fx c = cos_tab[angle & 255];
    fx s = sin_tab[angle & 255];

    fx ny = fxmul(*y, c) - fxmul(*z, s);
    fx nz = fxmul(*y, s) + fxmul(*z, c);

    *y = ny;
    *z = nz;
}

static inline void rotate_y(fx *x, fx *y, fx *z, int angle)
{
    fx c = cos_tab[angle & 255];
    fx s = sin_tab[angle & 255];

    fx nx = fxmul(*x, c) - fxmul(*z, s);
    fx nz = fxmul(*x, s) + fxmul(*z, c);

    *x = nx;
    *z = nz;
}

static inline void rotate_z(fx *x, fx *y, fx *z, int angle)
{
    fx c = cos_tab[angle & 255];
    fx s = sin_tab[angle & 255];

    fx nx = fxmul(*x, c) - fxmul(*y, s);
    fx ny = fxmul(*x, s) + fxmul(*y, c);

    *x = nx;
    *y = ny;
}

static inline void smooth_zero_limit(fx *type)
{
    fx dist = ABS(*type);
    fx step = fxmul(dist, ADJUST_SPEED);

    if(step > FX(1) >> 1)
        step = FX(1) >> 1;

    if(*type > 0)
        *type -= step;
    else if(*type < 0)
        *type += step;

    if(ABS(*type) < ADJUST_SPEED)
       *type = 0;
}

static inline void smooth_zero_limit_slow(fx *type)
{
    fx dist = ABS(*type);
    fx step = fxmul(dist, ADJUST_SPEED >> 1);

    if(step > FX(1) >> 1)
        step = FX(1) >> 1;

    if(*type > 0)
        *type -= step;
    else if(*type < 0)
        *type += step;

    if(ABS(*type) < ADJUST_SPEED >> 1)
       *type = 0;
}

static inline void smooth_max_limit(fx *type, fx amount)
{
    if(*type > amount)
    {
        smooth_zero_limit(type);
        return;
    }
    fx dist = ABS(*type - amount);
    fx step = fxmul(dist, ADJUST_SPEED);

    if(step > FX(1) >> 1)
        step = FX(1) >> 1;

    *type += step;
    if(*type > amount)
        *type = amount;
}

static inline void smooth_min_limit(fx *type, fx amount)
{
    if(*type < amount)
    {
        smooth_zero_limit(type);
        return;
    }
    fx dist = ABS(amount - *type);
    fx step = fxmul(dist, ADJUST_SPEED);

    if(step > FX(1) >> 1)
        step = FX(1) >> 1;

    *type -= step;
    if(*type < amount)
        *type = amount;
}

static inline void smooth_zero(fx *type)
{
    fx dist = ABS(*type);
    fx step = fxmul(dist, ADJUST_SPEED);

    if(*type > 0)
        *type -= step;
    else if(*type < 0)
        *type += step;

    if(ABS(*type) < ADJUST_SPEED)
       *type = 0;
}

static inline void smooth_min(fx *type, fx amount)
{
    fx dist = ABS(amount - *type);
    fx step = fxmul(dist, ADJUST_SPEED);

    *type -= step;
    if(*type < amount)
        *type = amount;
}

static inline void smooth_max(fx *type, fx amount)
{
    fx dist = ABS(*type - amount);
    fx step = fxmul(dist, ADJUST_SPEED);

    *type += step;
    if(*type > amount)
        *type = amount;
}

static inline void apply_axis(fx *v, int dir, fx minv, fx maxv)
{
    if(dir < 0) smooth_min_limit(v, minv);
    else if(dir > 0) smooth_max_limit(v, maxv);
}

static inline void project_point(fx wx, fx wy, fx wz, int* sx, int* sy)
{
    fx vx = wx - camera.x;
    fx vy = wy - camera.y;
    fx vz = wz - camera.z;

    fx rx = fxmul(vx, cam_mat.m[0][0]) +
            fxmul(vy, cam_mat.m[0][1]) +
            fxmul(vz, cam_mat.m[0][2]);

    fx ry = fxmul(vx, cam_mat.m[1][0]) +
            fxmul(vy, cam_mat.m[1][1]) +
            fxmul(vz, cam_mat.m[1][2]);

    fx rz = fxmul(vx, cam_mat.m[2][0]) +
            fxmul(vy, cam_mat.m[2][1]) +
            fxmul(vz, cam_mat.m[2][2]);

    int shift = 0;
    fx den = rz;
    if(den < 1) den = 1;
    while(den >= 512) 
    { 
        den >>= 1; 
        ++shift; 
    }
    fx inv = recip_tab[den];
    *sx = fx2int((fxmul(rx, inv) >> shift) * PROJ_SCALE) + 120;
    *sy = fx2int((fxmul(ry, inv) >> shift) * -PROJ_SCALE) + 80;
}

static inline void handle_player_input(void)
{
    int horz = key_tri_horz();
    int vert = key_tri_vert();
    if(invert)
        vert = -vert;
    int shld = key_tri_shoulder();

    int mode_roll = shld != 0 || barrel_roll;

    Entity3d* player = &obj3d[ENTITY_ARRAY_SIZE - 1];
    if(player->health < 1) return;

    if(key_hit(KEY_START))
    {
        sfx_stop_all();
        paused ^= 1;
        return;
    }
    if(paused) 
    {
        if(key_hit(KEY_SELECT))
        {
            invert ^= 1;
            sfx_play(SFX_SELECT, 1, 255);
        }
        return;
    }
    sfx_play(SFX_ENGINE, 0, 127);
    if(player->i_frames >> 1 != 0)
    {
        fx rock = fxmul(sin_tab[player->i_frames & 255], FX(20));
        player->pitch = rock;
        player->yaw = rock;
        return;
    }

    if(key_hit(KEY_A) && laser_count < (8 << powerup_state) && !laser_wait)
    {
        fx offset = 0;
        if(powerup_state)
            offset = player->hitbox_max_x >> 1;
        for(int i = ENTITY_ARRAY_SIZE - 1; i > 0; --i)
        {
            Entity3d* pro = &pro3d[i];
            if(pro->id == -1)
            {
                laser_wait = LASER_WAIT;
                apply_entity_template(pro, &laser_template);
                pro->health = INT_MAX;
                pro->x = player->x + offset;
                pro->y = player->y;
                pro->z = player->z;
                int pitch = fx2int(player->pitch) & 255;
                int yaw   = fx2int(player->yaw) & 255;
                pro->pitch = player->pitch >> 1;
                pro->yaw = player->yaw >> 1;
                pro->roll = player->roll;
                pro->dx = sin_tab[yaw] >> 1;
                pro->dy = sin_tab[pitch] >> 1;
                pro->dz = -pro->dz;
                pro->color = player->old_color + 1 + powerup_state;
                pro->owner = ENTITY_ARRAY_SIZE - 1;
                ++laser_count;
                if(offset <= 0)
                {
                    sfx_play(SFX_LASER1, 255, 255);
                    break;
                }
                else
                {
                    offset = player->hitbox_min_x >> 1;
                }
            }
        }
    }
    else if(laser_wait)
    {
        --laser_wait;
    }

    apply_axis(&player->yaw, horz, PITCH_MIN, PITCH_MAX);
    apply_axis(&player->pitch, vert, PITCH_MIN, PITCH_MAX);
    //apply_axis(&camera.pitch, vert, CAMERA_PITCH_MIN, CAMERA_PITCH_MAX);
    apply_axis(&camera.roll, horz, PITCH_MIN, PITCH_MAX);
    
    if(!mode_roll)
    {
        if(horz != 0 && !rolling)
        {
            banking = 1;
            apply_axis(&player->roll, -horz, PITCH_MIN, PITCH_MAX);
        }
    }
    else
    {
        banking = 0;
        rolling = 1;
    }

    if(barrel_roll_wait == 0 && barrel_roll == 0)
    {
        if(key_hit(KEY_L))
        {
            if(frame_count - last_l_roll <= BARREL_ROLL_FRAMES)
            {
                barrel_roll = 1;
                sfx_play(SFX_BARRELROLL, 1, 255);
            }

            last_l_roll = frame_count;
        }
        if(key_hit(KEY_R))
        {
            if(frame_count - last_r_roll <= BARREL_ROLL_FRAMES)
            {
                barrel_roll = -1;
                sfx_play(SFX_BARRELROLL, 1, 255);
            }

            last_r_roll = frame_count;
        }
    }
    else
    {
        if(barrel_roll_wait > 0)
            --barrel_roll_wait;
        shld = 0;
    }
    if(barrel_roll == 0)
    {
        if(shld == 1) 
            smooth_min(&player->roll, ROLL_MIN);
        else if(shld == -1) 
            smooth_max(&player->roll, ROLL_MAX);

        if(rolling && shld == 0)
        {
            smooth_zero(&player->roll);
            if(horz == 0)
                rolling = (player->roll != 0);
            else
                rolling = (player->roll > PITCH_MAX || player->roll < PITCH_MIN);
        }
        else if(banking && horz == 0)
        {
            smooth_zero(&player->roll);
            banking = (player->roll != 0);
        }
        else if(!banking && !rolling)
        {
            fx rock = fxmul(sin_tab[frame_count & 255], ROCK_LIMIT);
            player->roll = rock;
        }
    }
    else
    {
        if(barrel_roll > 0)
            smooth_max(&player->roll, BARREL_ROLL_MIN);
        else
            smooth_min(&player->roll, BARREL_ROLL_MAX);

        if(player->roll < BARREL_ROLL_MAX || player->roll > BARREL_ROLL_MIN)
        {
            barrel_roll = 0;
            barrel_roll_wait = 45;
        }
    }

    if(horz > 0)
    {
        if(key_is_down(KEY_R) || barrel_roll < 0)
            smooth_max(&speed_x, MAX_ROLL_H_SPEED);
        else
            smooth_max(&speed_x, MAX_H_SPEED);
    }
    else if(horz < 0)
    {
        if(key_is_down(KEY_L) || barrel_roll > 0)
            smooth_min(&speed_x, MIN_ROLL_H_SPEED);
        else
            smooth_min(&speed_x, MIN_H_SPEED);
    }
    if(horz == 0)
    {
        smooth_zero_limit(&player->yaw);
        smooth_zero_limit_slow(&camera.roll);
        //smooth_zero_limit(&camera.x);
        smooth_zero_limit(&speed_x);
    }
    else
    {
        player->x += fxmul(speed_x, FX(1) >> 6);
        if(player->x > level.max_player_check_x)
        {
            //if(level.max_camera_x != 0)
                //smooth_max_limit(&camera.x, level.max_camera_x);
            if(player->x > level.max_player_x)
                player->x = level.max_player_x;
        }
        else if(player->x < level.min_player_check_x)
        {
            //if(level.min_camera_x != 0)
                //smooth_min_limit(&camera.x, level.min_camera_x);
            if(player->x < level.min_player_x)
                player->x = level.min_player_x;
        }
        /*else
        {
            smooth_zero_limit_slow(&camera.x);
        }*/
    }
    
    if(vert > 0)
        smooth_max(&speed_y, MAX_V_SPEED);
    else if(vert < 0)
        smooth_min(&speed_y, MIN_V_SPEED);
    if(vert == 0)
    {
        smooth_zero_limit(&player->pitch);
        //smooth_zero_limit_slow(&camera.pitch);
        //smooth_zero_limit_slow(&camera.y);
        smooth_zero_limit(&speed_y);
    }
    else
    {
        player->y += fxmul(speed_y, FX(1) >> 7);
        if(player->y < level.min_player_check_y)
        {
            /*if(level.min_camera_y != 0)
            {
                smooth_min(&camera.y, level.min_camera_y);
            }
            else
            {
                if(vert < 0)
                    player->pitch = 0;
                smooth_zero_limit(&camera.pitch);
                smooth_zero_limit(&camera.y);
            }*/
            if(player->y < level.min_player_y)
            {
                if(vert < 0)
                    player->pitch = 0;
                player->y = level.min_player_y;
            }
        }
        else if(player->y > level.max_player_check_y)
        {
            /*if(level.max_camera_y != 0)
            {
                smooth_max(&camera.y, level.max_camera_y);
            }
            else
            {
                if(vert < 0)
                    player->pitch = 0;
                smooth_zero_limit(&camera.pitch);
                smooth_zero_limit(&camera.y);
            }*/
            if(player->y > level.max_player_y)
            {
                if(vert > 0)
                    player->pitch = 0;
                player->y = level.max_player_y;
            }
        }
        /*else
        {
            smooth_zero_limit_slow(&camera.y);
            smooth_zero_limit_slow(&camera.pitch);
        }*/
    }
}

IWRAM_CODE static void draw_horizontal_line_3d(fx z, fx height, fx x0, fx x1, fx turn, int color)
{
    if(turn > 0)
    {
        x0 += fxmul(z, turn);
        x1 += x1 + fxmul(z, turn);
    }
    else if(turn < 0)
    {
        x1 += fxmul(z, turn);
        x0 += x0 + fxmul(z, turn);
    }

    fx vy = height - camera.y;
    fx vz = z - camera.z;

    fx rz_base = fxmul(vy, cam_mat.m[2][1]) + fxmul(vz, cam_mat.m[2][2]);
    fx ry_base = fxmul(vy, cam_mat.m[1][1]) + fxmul(vz, cam_mat.m[1][2]);
    fx rx_base = fxmul(vy, cam_mat.m[0][1]) + fxmul(vz, cam_mat.m[0][2]);

    fx vx0 = x0 - camera.x;
    fx vx1 = x1 - camera.x;

    fx rz0 = fxmul(vx0, cam_mat.m[2][0]) + rz_base;
    fx rz1 = fxmul(vx1, cam_mat.m[2][0]) + rz_base;

    if(rz0 < NEAR && rz1 < NEAR) return;
    if(rz0 < NEAR) rz0 = NEAR;
    if(rz1 < NEAR) rz1 = NEAR;

    fx rx0 = fxmul(vx0, cam_mat.m[0][0]) + rx_base;
    fx rx1 = fxmul(vx1, cam_mat.m[0][0]) + rx_base;
    fx ry0 = fxmul(vx0, cam_mat.m[1][0]) + ry_base;
    fx ry1 = fxmul(vx1, cam_mat.m[1][0]) + ry_base;

    int shift0 = 0; fx den0 = rz0;
    if(den0 < 1) den0 = 1;
    while(den0 >= 512) { den0 >>= 1; shift0++; }
    fx inv0 = recip_tab[den0];

    int shift1 = 0; fx den1 = rz1;
    if(den1 < 1) den1 = 1;
    while(den1 >= 512) { den1 >>= 1; shift1++; }
    fx inv1 = recip_tab[den1];

    int sx0 = fx2int((fxmul(rx0, inv0) >> shift0) * PROJ_SCALE) + 120;
    int sy0 = fx2int((fxmul(ry0, inv0) >> shift0) * -PROJ_SCALE) + 80;
    int sx1 = fx2int((fxmul(rx1, inv1) >> shift1) * PROJ_SCALE) + 120;
    int sy1 = fx2int((fxmul(ry1, inv1) >> shift1) * -PROJ_SCALE) + 80;

    line(sx0, sy0, sx1, sy1, color);
}

IWRAM_CODE static void draw_vertical_line_3d(fx z, fx x, fx turn, int color)
{
    fx vx = x - camera.x;
    fx vz = z - camera.z;

    fx rz_base = fxmul(vx, cam_mat.m[2][0]) + fxmul(vz, cam_mat.m[2][2]);
    fx ry_base = fxmul(vx, cam_mat.m[1][0]) + fxmul(vz, cam_mat.m[1][2]);
    fx rx_base = fxmul(vx, cam_mat.m[0][0]) + fxmul(vz, cam_mat.m[0][2]);

    fx vy0 = level.floor_height - camera.y;
    fx vy1 = level.ceiling_height - camera.y;

    fx rz0 = fxmul(vy0, cam_mat.m[2][1]) + rz_base;
    fx rz1 = fxmul(vy1, cam_mat.m[2][1]) + rz_base;

    if(rz0 < NEAR && rz1 < NEAR) return;
    if(rz0 < NEAR) rz0 = NEAR;
    if(rz1 < NEAR) rz1 = NEAR;

    fx rx0 = fxmul(vy0, cam_mat.m[0][1]) + rx_base;
    fx rx1 = fxmul(vy1, cam_mat.m[0][1]) + rx_base;
    fx ry0 = fxmul(vy0, cam_mat.m[1][1]) + ry_base;
    fx ry1 = fxmul(vy1, cam_mat.m[1][1]) + ry_base;

    int shift0 = 0; fx den0 = rz0;
    if(den0 < 1) den0 = 1;
    while(den0 >= 512) { den0 >>= 1; shift0++; }
    fx inv0 = recip_tab[den0];

    int shift1 = 0; fx den1 = rz1;
    if(den1 < 1) den1 = 1;
    while(den1 >= 512) { den1 >>= 1; shift1++; }
    fx inv1 = recip_tab[den1];

    int sx0 = fx2int((fxmul(rx0, inv0) >> shift0) * PROJ_SCALE) + 120;
    int sy0 = fx2int((fxmul(ry0, inv0) >> shift0) * -PROJ_SCALE) + 80;
    int sx1 = fx2int((fxmul(rx1, inv1) >> shift1) * PROJ_SCALE) + 120;
    int sy1 = fx2int((fxmul(ry1, inv1) >> shift1) * -PROJ_SCALE) + 80;

    line(sx0, sy0, sx1, sy1, color);
}

static inline void draw_space_dust(fx z, int color)
{
    fx x = rand() & 0xFF * ((rand() & 1) ? -1 : 1);
    fx y = rand() & 0xFF * ((rand() & 1) ? -1 : 1);

    putpixel_size(x, y, z >> 10, color);
}

IWRAM_CODE void draw_level_floor(fx (*z_array)[3], fx height, fx turn, int color)
{
    for(int i = 0; i < NUM_FLOOR_LINES; ++i)
    {
        if(!paused)
        {
            z_array[i][0] -= level.speed;

            if(z_array[i][0] < NEAR)
            {
                z_array[i][0] += FX(NUM_FLOOR_LINES * 2);
                z_array[i][1] = (int)level.type;
                z_array[i][2] = height;
            }
        }

        enum LevelType t = z_array[i][1];
        fx h = z_array[i][2];
        if(t != LT_Space)
        {
            if(((t == LT_Ground || t == LT_Valley) && h < 0) || t == LT_Sky_Ground || t == LT_Tunnel || ((t == LT_Sky || t == LT_Inverse_Valley) && h > 0))
            {
                fx z = z_array[i][0];
                fx w1 = level.max_width;
                fx w2 = level.min_width;
                if((t == LT_Valley || t == LT_Inverse_Valley || t == LT_Tunnel) && turn == 0)
                {
                    if(w1 > FX(6))
                        w1 = FX(6);
                    if(w2 < FX(-6))
                        w2 = FX(-6);
                }
                draw_horizontal_line_3d(z, h, w1, w2, turn, color);
            }
        }
        else if(level.type == LT_Space)
        {
            int c = 21 + ((rand() & 3) * 3);
            fx z = z_array[i][0];

            int brightness;
            if(z < FX(2))           brightness = 2; // lightest
            else if(z < (FAR >> 1)) brightness = 1; // medium
            else                    brightness = 0; // darkest

            draw_space_dust(z, c + brightness);
        }
    }
}

IWRAM_CODE void draw_level_wall(fx (*z_array)[3], fx width, fx turn, int color)
{
    for(int i = 0; i < NUM_FLOOR_LINES; ++i)
    {
        if(!paused)
        {
            z_array[i][0] -= level.speed;

            if(z_array[i][0] < NEAR)
            {
                z_array[i][0] += FX(NUM_FLOOR_LINES * 2);
                z_array[i][1] = (int)level.type;
                z_array[i][2] = width;
            }
        }
        enum LevelType t = z_array[i][1];
        fx w = z_array[i][2];
        if(t != LT_Space)
        {
            if((t == LT_Wall_L && w < 0) || (t == LT_Wall_R && w > 0) || t == LT_Walls || t == LT_Tunnel || t == LT_Inverse_Valley || t == LT_Valley)
            {
                fx z = z_array[i][0];
                
                fx x = 0;
                if(turn != 0)
                {
                    x = fxmul(w, FX(2)) + fxmul(z, turn);
                }
                else
                {
                    if(w < FX(-6))
                        w = FX(-6);
                    else if(w > FX(6))
                        w = FX(6);
                    x = w + fxmul(z, turn);
                }

                draw_vertical_line_3d(z, x, turn, color);
            }
        }
    }
}

void init3d(void)
{
    REG_DISPCNT = DCNT_MODE4 | DCNT_BG2 | DCNT_OBJ | DCNT_OBJ_1D;

    // environment / entities
    pal_bg_mem[0] = RGB8(0,0,0); // black 
    pal_bg_mem[1] = RGB8(255,255,255); // white
    pal_bg_mem[2] = RGB8(90,90,90); // grey
    pal_bg_mem[3] = RGB8(176,176,176); // light grey
    pal_bg_mem[4] = RGB8(0,0,255); // blue
    pal_bg_mem[5] = RGB8(255,0,0); // red
    pal_bg_mem[6] = RGB8(255,99,0); // orange
    pal_bg_mem[7] = RGB8(255,238,0); // yellow
    pal_bg_mem[8] = RGB8(0,255,153); // teal
    pal_bg_mem[9] = RGB8(151,92,255); // purple
    pal_bg_mem[10] = RGB8(207,171,81); // gold

    // player
    pal_bg_mem[11] = RGB8(77, 155, 230); // player
    pal_bg_mem[12] = RGB8(48, 225, 185); // laser 1
    pal_bg_mem[13] = RGB8(143, 248, 226); // laser 2

    // health bar
    pal_bg_mem[16] = RGB8(0,255,0); // green
    pal_bg_mem[17] = RGB8(191,204,0); // green yellow
    pal_bg_mem[18] = RGB8(221,162,0); // orange
    pal_bg_mem[19] = RGB8(237,117,0); // red orange
    pal_bg_mem[20] = RGB8(255,0,0); // red
    
    // space dust
    pal_bg_mem[21] = RGB8(24,41,24); // dark green
    pal_bg_mem[22] = RGB8(66,99,74); // greenish
    pal_bg_mem[23] = RGB8(173,189,173); // light greenish

    pal_bg_mem[24] = RGB8(214,74,41); // light red
    pal_bg_mem[25] = RGB8(239,173,66); // goldish
    pal_bg_mem[26] = RGB8(255,222,107); // yellowish

    pal_bg_mem[27] = RGB8(90,74,222); // purpleish
    pal_bg_mem[28] = RGB8(123,165,255); // blueish
    pal_bg_mem[29] = RGB8(181,239,255); // light blueish

    pal_bg_mem[30] = RGB8(90, 105, 136); // grey
    pal_bg_mem[31] = RGB8(139,155,180); // light grey
    pal_bg_mem[32] = RGB8(192,203,220); // lighter grey

    bucket = 0;
    current_state = GS_attract3d;

    for(int i = 0; i < ENTITY_ARRAY_SIZE; ++i)
    {
        obj3d[i] = ENTITY3D_DEFAULT;
        pro3d[i] = ENTITY3D_DEFAULT;
    }
    
    apply_entity_template(&obj3d[ENTITY_ARRAY_SIZE - 1], &ship_template);
    obj3d[ENTITY_ARRAY_SIZE - 1].health = 100;
    obj3d[ENTITY_ARRAY_SIZE - 1].old_color = obj3d[ENTITY_ARRAY_SIZE - 1].color;

    camera = CAMERA_DEFAULT;
    level = LEVEL3D_DEFAULT;
    apply_level_preset(&level, LT_Space);

    for(int i = 0; i < NUM_FLOOR_LINES; ++i)
    {
        floor_lines[i][0] = FX(i * 2);   // spacing between stripes
        floor_lines[i][1] = (int)level.type;
        ceiling_lines[i][0] = FX(i * 2);
        ceiling_lines[i][1] = (int)level.type;
        wall_lines_r[i][0] = FX(i * 2);
        wall_lines_r[i][1] = (int)level.type;
        wall_lines_l[i][0] = FX(i * 2);
        wall_lines_l[i][1] = (int)level.type;
    }
}

static inline void start_explosion(Entity3d* ent)
{
    ent->state = ES_Exploding;
    ent->timer = 60;

    for (int v = 0; v < ent->num_vertices; v++)
    {
        fx x = ent->model_vertices[v][0];
        fx y = ent->model_vertices[v][1];
        fx z = ent->model_vertices[v][2];

        ent->explodeVerts[v].vx = x;
        ent->explodeVerts[v].vy = y;
        ent->explodeVerts[v].vz = z;

        fx speed = (FX(1) >> 8);

        fx len = fxmul(x,x) + fxmul(y,y) + fxmul(z,z);
        if (len < (FX(1) >> 8)) len = (FX(1) >> 8);
        fx inv = fast_fxdiv(FX(1), len);

        fx nx = fxmul(x, inv);
        fx ny = fxmul(y, inv);
        fx nz = fxmul(z, inv);

        ent->explodeVerts[v].dx = fxmul(nx, speed);
        ent->explodeVerts[v].dy = fxmul(ny, speed);
        ent->explodeVerts[v].dz = fxmul(nz, speed);

        ent->explodeVerts[v].active = 1;
    }
    sfx_play(SFX_EXPLOSION, 0, 255);
}

IWRAM_CODE int check_entity_bounds(Entity3d* ent)
{
    if(ent->x > BOUND_RIGHT || ent->x < BOUND_LEFT
    || ent->y > BOUND_UP || ent->y < BOUND_DOWN
    || ent->z > FAR || ent->z < -NEAR - ent->bounding_radius
    || ent->scale <= 0)
    {
        ent->id = -1;
        return 0;
    }
    else if(ent->z < NEAR - ent->bounding_radius)
    {
        return 0;
    }
    return 1;
}

IWRAM_CODE int handle_entity_logic(Entity3d* ent)
{
    if(ent->state != ES_Exploding)
    {
        if(ent->i_frames)
        {
            ent->color = 16 + (ent->i_frames & 5);
            --ent->i_frames;
            if(!ent->i_frames)
                ent->color = ent->old_color;
        }

        if(ent->health > 0)
        {
            Entity3d* player = &obj3d[ENTITY_ARRAY_SIZE - 1];
            switch(ent->id)
            {
                case MODEL_POWERUP:
                case MODEL_RING:
                    if(ent->owner == ENTITY_ARRAY_SIZE - 1)
                    {
                        ent->x = player->x;
                        ent->y = player->y;
                        ent->z = player->z;
                        ent->scale -= FX(1) >> 4;
                        ent->yaw += FX(32);
                    }
                    else
                        ent->z -= level.speed >> 1;
                    break;
                case MODEL_PILLAR2:
                case MODEL_MINE:
                case MODEL_PILLAR: {
                    if(ent->owner == -1)
                    {
                        fx bottom = level.floor_height;
                        fx h = ent->hitbox_max_y;
                        if(!bottom)
                        {
                            h = -ent->hitbox_min_y;
                            bottom = level.ceiling_height;
                        }
                        ent->y = (bottom >> 1) + h;
                    }
                }
                case MODEL_BARRIER:
                case MODEL_WALL:
                case MODEL_ARCHWAY:
                    ent->z -= level.speed >> 1;
                    if(level.turn != 0)
                        ent->x += fxmul(ent->z, level.turn);
                    break;
                case MODEL_SPHERE:
                case MODEL_CUBOID:
                    ent->z -= level.speed >> 1;
                    if(level.turn != 0)
                        ent->x += fxmul(ent->z, level.turn);
                    ent->yaw += ent->target[0];
                    ent->roll += ent->target[1];
                    ent->pitch += ent->target[2];
                    break;
                case MODEL_ENEMY_CRYSTAL:
                case MODEL_ENEMY_BASIC:
                    handle_enemy_logic(ent, player, pro3d);
                    break;
                default:
                    break;
            }
        }
        else
        {
            switch(ent->id)
            {
                case MODEL_SHIP:
                    sfx_stop(SFX_ENGINE);
                    break;
                default:
                    break;
            }
            start_explosion(ent);
        }
    }
    else
    {
        for(int v = 0; v < ent->num_vertices; v++) 
        {
            if (!ent->explodeVerts[v].active) continue;

            ent->explodeVerts[v].vx += ent->explodeVerts[v].dx;
            ent->explodeVerts[v].vy += ent->explodeVerts[v].dy;
            ent->explodeVerts[v].vz += ent->explodeVerts[v].dz;
        }
        if(--ent->timer <= 0) 
        {
            switch(ent->id)
            {
                case MODEL_SHIP:
                    dead = 1;
                    break;
                default:
                    break;
            }
            ent->id = -1;
            return 0;
        }
    }
    return 1;
}

IWRAM_CODE void draw_entity(Entity3d* ent)
{
    fx dx = ent->x - camera.x;
    fx dy = ent->y - camera.y;
    fx dz = ent->z - camera.z;

    fx ev_x = fxmul(dx, cam_mat.m[0][0]) +
                fxmul(dy, cam_mat.m[0][1]) +
                fxmul(dz, cam_mat.m[0][2]);

    fx ev_y = fxmul(dx, cam_mat.m[1][0]) +
                fxmul(dy, cam_mat.m[1][1]) +
                fxmul(dz, cam_mat.m[1][2]);

    fx ev_z = fxmul(dx, cam_mat.m[2][0]) +
                fxmul(dy, cam_mat.m[2][1]) +
                fxmul(dz, cam_mat.m[2][2]);
    
    u32 proj_x[64];
    u32 proj_y[64];
    
    Mat3 ent_mat;

    int er = fx2int(ent->roll) & 255;
    int ep = fx2int(ent->pitch) & 255;
    int ey = (-fx2int(ent->yaw)) & 255;

    fx esr = sin_tab[er], ecR = cos_tab[er];
    fx esp = sin_tab[ep], ecP = cos_tab[ep];
    fx esy = sin_tab[ey], ecY = cos_tab[ey];

    ent_mat.m[0][0] =  fxmul(ecY, ecR) + fxmul(fxmul(esy, esp), esr);
    ent_mat.m[0][1] =  fxmul(-ecY, esr) + fxmul(fxmul(esy, esp), ecR);
    ent_mat.m[0][2] =  fxmul(esy, ecP);

    ent_mat.m[1][0] =  fxmul(ecP, esr);
    ent_mat.m[1][1] =  fxmul(ecP, ecR);
    ent_mat.m[1][2] = -esp;

    ent_mat.m[2][0] = -fxmul(esy, ecR) + fxmul(fxmul(ecY, esp), esr);
    ent_mat.m[2][1] =  fxmul(esy, esr) + fxmul(fxmul(ecY, esp), ecR);
    ent_mat.m[2][2] =  fxmul(ecY, ecP);

    Mat3 mv_mat;
    for(int r = 0; r < 3; ++r)
    {
        for(int c = 0; c < 3; ++c)
        {
            mv_mat.m[r][c] = fxmul(cam_mat.m[r][0], ent_mat.m[0][c]) +
                                fxmul(cam_mat.m[r][1], ent_mat.m[1][c]) +
                                fxmul(cam_mat.m[r][2], ent_mat.m[2][c]);
        }
    }
    
    for(int v = 0; v < ent->num_vertices; ++v)
    {
        fx vx, vy, vz;

        if(ent->state != ES_Exploding) 
        {
            fx sx = ent->scale;
            vx = fxmul(ent->model_vertices[v][0], sx);
            vy = fxmul(ent->model_vertices[v][1], sx);
            vz = fxmul(ent->model_vertices[v][2], sx);
        } 
        else 
        {
            vx = ent->explodeVerts[v].vx;
            vy = ent->explodeVerts[v].vy;
            vz = ent->explodeVerts[v].vz;
        }

        fx rx = fxmul(vx, mv_mat.m[0][0]) +
                fxmul(vy, mv_mat.m[0][1]) +
                fxmul(vz, mv_mat.m[0][2]);

        fx ry = fxmul(vx, mv_mat.m[1][0]) +
                fxmul(vy, mv_mat.m[1][1]) +
                fxmul(vz, mv_mat.m[1][2]);

        fx rz = fxmul(vx, mv_mat.m[2][0]) +
                fxmul(vy, mv_mat.m[2][1]) +
                fxmul(vz, mv_mat.m[2][2]);

        fx wx = rx + ev_x;
        fx wy = ry + ev_y;
        fx wz = rz + ev_z;

        if(wz < NEAR) wz = NEAR;

        int shift = 0;
        fx den = wz;
        if(den < 1) den = 1;
        while(den >= 512) 
        { 
            den >>= 1; 
            ++shift; 
        }
        fx inv = recip_tab[den];

        proj_x[v] = fx2int((fxmul(wx, inv) >> shift) * PROJ_SCALE) + 120;
        proj_y[v] = fx2int((fxmul(wy, inv) >> shift) * -PROJ_SCALE) + 80;
    }
    if(ent->state != ES_Exploding) 
    {
        for(int e = 0; e < ent->num_edges; ++e) 
        {
            int a = ent->model_edges[e][0];
            int b = ent->model_edges[e][1];
            line(proj_x[a], proj_y[a], proj_x[b], proj_y[b], ent->color);
        }
    } 
    else 
    {
        for(int v = 0; v < ent->num_vertices; ++v) 
        {
            if (ent->explodeVerts[v].active)
                putpixel_size(proj_x[v], proj_y[v], 2, ent->color);
        }
    }
}

IWRAM_CODE void process_entities(int start_i, int end_i)
{
    Entity3d* player = &obj3d[ENTITY_ARRAY_SIZE - 1];
    for(int i = start_i; i < end_i; ++i)
    {
        Entity3d* ent = &obj3d[i];

        if(ent->id == -1) continue;

        if(!paused)
        {
            if(!handle_entity_logic(ent)) continue;
        }
        if(!check_entity_bounds(ent)) continue;
        draw_entity(ent);
    }

    for(int i = start_i; i < end_i; ++i)
    {
        Entity3d* ent = &obj3d[i];
        if(ent->id == -1) continue;
        if(ent->state == ES_Exploding) continue;
        if(ent->z < NEAR - ent->bounding_radius)
        if(i == ENTITY_ARRAY_SIZE - 1) continue;

        if(!player->i_frames)
        {
            if(ent->owner != ENTITY_ARRAY_SIZE - 1)
            {
                fx dx = ent->x - player->x;
                //fx dy = ent->y - player->y;
                fx dz = ent->z - player->z;

                fx dist2 = fxmul(dx,dx) + fxmul(dz,dz);
                fx rad_ent = ent->bounding_radius >> 1;
                fx rad_player = player->bounding_radius >> 1;
                if(player->roll > FX(63) || player->roll < FX(-63))
                {
                    rad_player = rad_player >> 1;
                }
                rad_player -= 10;

                if(rad_ent < 1) rad_ent = 1;
                if(rad_player < 1) rad_player = 1;

                fx rad = rad_ent + rad_player;

                fx ent_top = ent->y + ent->hitbox_max_y;
                fx ent_bottom = ent->y + ent->hitbox_min_y;

                fx player_top = player->y + player->hitbox_max_y;
                fx player_bottom = player->y + player->hitbox_min_y;
                if(dist2 < fxmul(rad,rad) && ent_bottom <= player_top && ent_top >= player_bottom)
                {
                    if(ent->damage > 0)
                    {
                        player->health -= ent->damage;
                        powerup_health -= ent->damage;
                        sfx_play(SFX_HIT, 1, 255);
                        if(powerup_health <= 0)
                        {
                            powerup_health = 25;
                            sfx_play(SFX_POWERDOWN, 1, 255);
                            powerup_state = 0;
                        }
                        if(player->health <= 0)
                            player->health = 0;
                        else
                        {
                            player->i_frames = PLAYER_I_FRAMES;
                            player->old_color = player->color;
                        }
                    }
                    else
                    {
                        switch(ent->id)
                        {
                            case MODEL_RING:
                                player->health -= ent->damage;
                                if(player->health > 100)
                                    player->health = 100;
                                ent->owner = ENTITY_ARRAY_SIZE - 1;
                                sfx_play(SFX_HPUP, 255, 255);
                                break;
                            case MODEL_POWERUP:
                                powerup_state = 1;
                                powerup_health = 25;
                                ent->owner = ENTITY_ARRAY_SIZE - 1;
                                sfx_play(SFX_POWERUP, 255, 255);
                                break;
                            default:
                                break;
                        }
                    }
                    continue;
                }
            }
        }

        for(int j = 0; j < ENTITY_ARRAY_SIZE; ++j)
        {
            Entity3d* laser = &pro3d[j];
            if(laser->id == -1) continue;
            if(laser->owner != ENTITY_ARRAY_SIZE - 1) continue;

            fx dx = ent->x - laser->x;
            //fx dy = ent->y - laser->y;
            fx dz = ent->z - laser->z;

            fx dist2 = fxmul(dx,dx) + fxmul(dz,dz);
            fx rad_ent = ent->bounding_radius;
            fx rad_laser = laser->bounding_radius;

            if(rad_ent < 1) rad_ent = 1;
            if(rad_laser < 1) rad_laser = 1;

            fx ent_top = ent->y + ent->hitbox_max_y;
            fx ent_bottom = ent->y + ent->hitbox_min_y;

            fx laser_top = laser->y + laser->hitbox_max_y;
            fx laser_bottom = laser->y + laser->hitbox_min_y;

            fx rad = rad_ent + rad_laser;
            if(dist2 < fxmul(rad,rad) && ent_bottom <= laser_top && ent_top >= laser_bottom)
            {
                laser->id = -1;
                int p_laser = laser->owner == ENTITY_ARRAY_SIZE - 1;
                if(p_laser)
                    --laser_count;
                sfx_play(SFX_HIT, 1, 255);
                
                if(ent->health != INT_MAX  && !ent->i_frames)
                {
                    ent->old_color = ent->color;
                    ent->health -= laser->damage;
                    if(ent->health <= 0 && p_laser)
                    {
                        ++score;
                    }
                }
                break;
            }
        }
    }
}

IWRAM_CODE void process_projectiles(int start_i, int end_i)
{
    Entity3d* player = &obj3d[ENTITY_ARRAY_SIZE - 1];
    for(int i = start_i; i < end_i; ++i)
    {
        Entity3d* ent = &pro3d[i];

        if(ent->id == -1) continue;

        if(!paused)
        {
            ent->x += ent->dx;
            ent->y += ent->dy;
            ent->z -= ent->dz;
            if(!check_entity_bounds(ent)) 
            {
                if(ent->owner == ENTITY_ARRAY_SIZE - 1)
                    --laser_count;
                continue;
            }
        }
        draw_entity(ent);
        
        if(!paused && !player->i_frames && ent->owner != ENTITY_ARRAY_SIZE - 1 && ent->id != -1)
        {
            fx dx = ent->x - player->x;
            fx dz = ent->z - player->z;

            fx dist2 = fxmul(dx,dx) + fxmul(dz,dz);
            fx rad_ent = ent->bounding_radius >> 1;
            fx rad_player = player->bounding_radius >> 1;
            if(player->roll > FX(63) || player->roll < FX(-63))
            {
                rad_player = rad_player >> 1;
            }
            rad_player -= 10;

            if(rad_ent < 1) rad_ent = 1;
            if(rad_player < 1) rad_player = 1;

            fx rad = rad_ent + rad_player;

            fx ent_top = ent->y + ent->hitbox_max_y;
            fx ent_bottom = ent->y + ent->hitbox_min_y;

            fx player_top = player->y + player->hitbox_max_y;
            fx player_bottom = player->y + player->hitbox_min_y;

            if(dist2 < fxmul(rad,rad) && ent_bottom <= player_top && ent_top >= player_bottom)
            {
                ent->id = -1;
                if(barrel_roll_wait)
                {
                    sfx_play(SFX_SELECT, 1, 255);
                }
                else
                {
                    player->health -= ent->damage;
                    powerup_health -= ent->damage;
                    if(powerup_health <= 0)
                    {
                        sfx_play(SFX_POWERDOWN, 1, 255);
                        powerup_state = 0;
                    }
                    player->i_frames = PLAYER_I_FRAMES;
                    player->old_color = player->color;
                    sfx_play(SFX_HIT, 1, 255);
                }
            }
        }
    }
}

IWRAM_CODE int run3d(void)
{
    int start_i = 0;
    int end_i   = 0;
    
    handle_player_input();

    switch(bucket)
    {
        case 0:
            build_camera_matrix();
            draw_level_floor(floor_lines, level.floor_height, level.turn, level.floor_color);
            draw_level_wall(wall_lines_r, level.max_width, level.turn, level.wall_r_color);
            draw_level_wall(wall_lines_l, level.min_width, level.turn, level.wall_l_color);
            break;
        case 1:
            draw_level_floor(ceiling_lines, level.ceiling_height, level.turn, level.ceiling_color);
            start_i = 0;
            end_i   = 8;
            break;
        case 2:
            start_i = 8;
            end_i   = ENTITY_ARRAY_SIZE;
            break;
    }

    ++bucket;
    if(bucket == 3) bucket = 0;

    process_entities(start_i, end_i);
    process_projectiles(start_i, end_i);
    int dma = bucket == 0;
    if(dma)
    {
        if(paused && !dead)
        {
            display_message(backbuf, 64, 10, 1, 3);
            display_message(backbuf, 64, 87, 1, 5 + invert);
            display_message(backbuf, 64, 98, 1, 7 + invert);
            display_message(backbuf, 64, 109, 1, 9);
            display_message(backbuf, 64, 120, 1, 10);
        }
        else if(!dead)
        {
            Entity3d* player = &obj3d[ENTITY_ARRAY_SIZE - 1];
            int color = 16;
            if(player->health < 80)
                ++color;
            if(player->health < 60)
                ++color;
            if(player->health < 40)
                ++color;
            if(player->health < 20)
                ++color;
            display_message(backbuf, 0, 0, color, 4);
            display_number(backbuf, 7, 0, player->health, color);
            display_score(backbuf, 0, 10, score, 1);
            run_endless3d(&level, obj3d);
            level.current_position += level.speed;
        }
        else if(dead)
        {
            current_state = GS_init3d;
        }
    }
    return dma;
}

static inline void smooth_to_target(u32 *value, int target, fx speed)
{
    int diff = target - *value;
    int step = fxmul(FX(diff), speed) >> 8;

    if(step == 0 && diff != 0)
        step = (diff > 0) ? 1 : -1;

    *value += step;

    if(abs(target - *value) < 2)
        *value = target;
}

int model_viewer3d(void)
{
    Entity3d* ent = &obj3d[ENTITY_ARRAY_SIZE - 1];

    static int model = MODEL_SHIP;
    static int old = 0;
    static int start = 0;

    if(key_is_down(KEY_LEFT))
    {
        ent->yaw += FX(1);
    }
    else if(key_is_down(KEY_RIGHT))
    {
        ent->yaw -= FX(1);
    }
    if(key_is_down(KEY_UP))
    {
        ent->pitch += FX(1);
    }
    else if(key_is_down(KEY_DOWN))
    {
        ent->pitch -= FX(1);
    }
    if(key_hit(KEY_L))
    {
        --model;
        camera = CAMERA_DEFAULT;
        if(model < 0)
            model = MODEL_MAX - 1;
        old = 1;
    }
    else if(key_hit(KEY_R))
    {
        ++model;
        camera = CAMERA_DEFAULT;
        if(model == MODEL_MAX)
            model = 0;
        old = 1;
    }
    if(key_is_down(KEY_B))
    {
        camera.z -= FX(1) >> 2;
    }
    else if(key_is_down(KEY_A))
    {
        camera.z += FX(1) >> 2;
        if(camera.z > 0)
            camera.z = 0;
    }
    if(key_hit(KEY_START))
    {
        camera = CAMERA_DEFAULT;
    }
    start |= key_hit(KEY_SELECT);
    switch(bucket)
    {
        case 0:
            dma3_fill(backbuf, 0, sizeof(backbuf));
            build_camera_matrix();
            draw_level_floor(floor_lines, level.floor_height, 0, 0);
            draw_level_wall(wall_lines_r, level.max_width, 0, 0);
            draw_level_wall(wall_lines_l, level.min_width, 0, 0);
            draw_level_floor(ceiling_lines, level.ceiling_height, 0, 0);
            break;
        case 1:
            if(old)
            {
                old = 0;
                switch(model)
                {
                    default: break;
                    case MODEL_ARCHWAY: apply_entity_template(ent, &archway_template); break;
                    case MODEL_BARRIER: apply_entity_template(ent, &barrier_template); break;
                    case MODEL_PILLAR:  apply_entity_template(ent, &pillar_template); break;
                    case MODEL_WALL:    apply_entity_template(ent, &wall_template); break;
                    case MODEL_PILLAR2: apply_entity_template(ent, &pillar2_template); break;
                    case MODEL_MINE:    apply_entity_template(ent, &mine_template); break;
                    case MODEL_RING:    apply_entity_template(ent, &ring_template); break;
                    case MODEL_POWERUP: apply_entity_template(ent, &powerup_template); break;
                    case MODEL_SPHERE:  apply_entity_template(ent, &sphere_template); break;
                    case MODEL_CUBOID:  apply_entity_template(ent, &cuboid_template);break;
                    case MODEL_ENEMY_BASIC:   apply_entity_template(ent, &enemy_basic_template); break;
                    case MODEL_ENEMY_CRYSTAL: apply_entity_template(ent, &enemy_crystal_template); break;
                    case MODEL_SHIP: apply_entity_template(ent, &ship_template); break;
                    case MODEL_LASER: apply_entity_template(ent, &laser_template); break;
                    case MODEL_BEAM:  apply_entity_template(ent, &beam_template); break;
                }
            }
            draw_entity(ent);
            display_message(backbuf, 100, 0, ent->color, model + 16);
            display_message(backbuf, 7, 0, ent->color, MODEL_MAX + 16);
            display_message(backbuf, 230, 0, ent->color, MODEL_MAX + 17);
            break;
        case 2:
            display_message(backbuf, 0, 130, ent->color, 33);
            if(start)
            {
                current_state = GS_init3d;
            }
            break;
    }

    ++bucket;
    if(bucket == 3) bucket = 0;

    return bucket == 0;
}

int attract3d(void)
{
    static int start = 0;
    static int left = 0;
    static int right = 0;
    static u32 orbit_angle = 64;
    static int flash = 0;
    static int message_x = 77;
    static int msg_idx = 14;
    Entity3d* player = &obj3d[ENTITY_ARRAY_SIZE - 1];
    fx rock = fxmul(sin_tab[frame_count & 255], ROCK_LIMIT);
    player->roll = rock;

    if(!left && !right)
    {
        left |= key_hit(KEY_LEFT);
        right |= key_hit(KEY_RIGHT);
        start |= key_is_down(KEY_START);
        if(start)
        {
            if(!flash)
                sfx_play(SFX_START, 1, 255);
            smooth_to_target(&orbit_angle, 64, FX(1) >> 6);
            ++flash;
        }
        else
        {
            display_message(backbuf, 67, 120, 16, 12);
            display_message(backbuf, 164, 120, 16, 13);
        }
    }
    else
    {
        if(right)
        {
            message_x += 7;
            if(message_x > 240)
            {
                if(msg_idx == 14)
                    ++msg_idx;
                else
                    --msg_idx;
                message_x = 0;
            }
            else if(message_x == 77)
            {
                right = 0;
                left = 0;
            }
        }
        else
        {
            message_x -= 7;
            if(message_x < 0)
            {
                if(msg_idx == 14)
                    ++msg_idx;
                else
                    --msg_idx;
                message_x = 238;
            }
            else if(message_x == 77)
            {
                right = 0;
                left = 0;
            }
        }
    }

    switch(bucket)
    {
        case 0:
            dma3_fill(backbuf, 0, sizeof(backbuf));
            build_camera_matrix();
            draw_level_floor(floor_lines, level.floor_height, 0, 0);
            draw_level_wall(wall_lines_r, level.max_width, 0, 0);
            draw_level_wall(wall_lines_l, level.min_width, 0, 0);
            draw_level_floor(ceiling_lines, level.ceiling_height, 0, 0);
            break;
        case 1:
            if(!start)
                orbit_angle = (orbit_angle + 1) & 255;
            camera.x = player->x + fxmul(player->bounding_radius, cos_tab[orbit_angle & 255]);
            camera.z = player->z + fxmul(player->bounding_radius, sin_tab[orbit_angle & 255]);
            camera.yaw = FX((orbit_angle + 64) & 255);
            sfx_play(SFX_ENGINE, 0, 127);
            draw_entity(player);
            display_message(backbuf, 1, 140, 6, 1);
            display_message(backbuf, 157, 140, 9, 2);
            display_message(backbuf, 0, 0, 1, 11);
            display_score(backbuf, 0, 10, score, 1);
            display_message(backbuf, message_x, 120, 16, msg_idx);
            break;
        case 2:
            display_message(backbuf, 83, 20, 16, 34);
            display_message(backbuf, 83, 29, 16, 35);
            display_message(backbuf, 83, 38, 16, 36);
            if(!(start && (flash & 2) == 0))
            {
                display_message(backbuf, 80, 110, 16, 0);
            }
            if(start && orbit_angle == 64 && !sfx_check(SFX_START))
            {
                if(msg_idx == 14)
                {
                    camera = CAMERA_DEFAULT;
                    apply_level_preset(&level, LT_Ground);
                    current_state = GS_run3d;
                    start = 0;
                    flash = 0;
                    score = 0;
                }
                else 
                {
                    camera = CAMERA_DEFAULT;
                    current_state = GS_model_view3d;
                    obj3d[ENTITY_ARRAY_SIZE - 1].roll = 0;
                    sfx_stop(SFX_ENGINE);
                    level.floor_height = 1;
                    level.ceiling_height = 1;
                }
                start = 0;
                left = 0;
                right = 0;
                orbit_angle = 64;
                flash = 0;
                message_x = 77;
                msg_idx = 14;
            }
            break;
    }

    ++bucket;
    if(bucket == 3) bucket = 0;

    return bucket == 0;
}

IWRAM_CODE void dma3d(void)
{
    dma3_cpy((u8*)0x6000000, backbuf, sizeof(backbuf));
    dma3_fill(backbuf, level.bg_color, sizeof(backbuf));
}