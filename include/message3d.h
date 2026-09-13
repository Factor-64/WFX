#ifndef MESSAGE3D_H
#define MESSAGE3D_H
#include <tonc.h>

void display_message(u8* backbuf, int x, int y, u8 color, int id);
void display_number(u8* backbuf, int x, int y, int value, u8 color);
void display_score(u8* backbuf, int x, int y, int value, u8 color);

#endif
