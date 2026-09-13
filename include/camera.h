#ifndef CAMERA_H
#define CAMERA_H

#include <tonc.h>
#include "global.h"
#include "trig_lut.h"

#define CAMERA_DEFAULT \
(Camera){ \
    .x = 0, \
    .y = 0, \
    .z = 0, \
    .roll = 0, \
    .pitch = 0, \
    .yaw = 0 \
}

typedef struct Camera {
    fx x;
    fx y;
    fx z;
    fx roll;
    fx pitch;
    fx yaw;
} Camera;

extern Mat3 cam_mat;

extern Camera camera;

IWRAM_CODE void build_camera_matrix(void);

#endif