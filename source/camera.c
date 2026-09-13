#include "camera.h"

EWRAM_DATA Mat3 cam_mat;

EWRAM_DATA Camera camera = CAMERA_DEFAULT;

IWRAM_CODE void build_camera_matrix(void)
{
    int cr = fx2int(camera.roll) & 255;
    int cp = fx2int(camera.pitch) & 255;
    int cy = fx2int(camera.yaw) & 255;

    fx sr = sin_tab[cr], cR = cos_tab[cr];
    fx sp = sin_tab[cp], cP = cos_tab[cp];
    fx sy = sin_tab[cy], cY = cos_tab[cy];

    cam_mat.m[0][0] =  fxmul(cY, cR) + fxmul(fxmul(sy, sp), sr);
    cam_mat.m[0][1] =  fxmul(-cY, sr) + fxmul(fxmul(sy, sp), cR);
    cam_mat.m[0][2] =  fxmul(sy, cP);

    cam_mat.m[1][0] =  fxmul(cP, sr);
    cam_mat.m[1][1] =  fxmul(cP, cR);
    cam_mat.m[1][2] = -sp;

    cam_mat.m[2][0] = -fxmul(sy, cR) + fxmul(fxmul(cY, sp), sr);
    cam_mat.m[2][1] =  fxmul(sy, sr) + fxmul(fxmul(cY, sp), cR);
    cam_mat.m[2][2] =  fxmul(cY, cP);
}