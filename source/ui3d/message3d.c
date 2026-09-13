#include "message3d.h"
#include "text3d.h"

const char *messages[] = {
    "PRESS START",
    "Factor",
    "GBA Jam 2026",
    "     Paused\n\n    Controls\n  \x10|Fire Laser\n  \x12|Roll Left\n  \x13|Roll Right\nSel|Invert Y Axis",
    "\x8F",
    "  \x18|Fly Up",
    "  \x18|Fly Down",
    "  \x17|Fly Down",
    "  \x17|Fly Up",
    "  \x16|Fly Left",
    "  \x15|Fly Right",
    "Last Score",
    "\x16",
    "\x15",
    "Endless Mode",
    "Model Viewer",
    "ARCHWAY",
    "BARRIER",
    "BEAM",
    "METEOR1",
    "ENEMY1",
    "ENEMY2",
    "LASER",
    "MINE",
    "PILLAR1",
    "PILLAR2",
    "POWERUP",
    "RING",
    "SHIP",
    "METEOR2",
    "WALL",
    "\x12",
    "\x13",
    "\x10Zoom In|\x11Zoom Out|\x16-Yaw|\x15+Yaw\n\x17+Pitch|\x18-Pitch|Sel Return",
    "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B",
    "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB",
    "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB"
};

const int messages_count = sizeof(messages) / sizeof(messages[0]);

void display_message(u8* backbuf, int x, int y, u8 color, int id)
{
    if(id > messages_count)
        return;
    
    draw_text(backbuf, x, y, messages[id], color);
}

void display_number(u8* backbuf, int x, int y, int value, u8 color)
{
    char buf[13];
    sprintf(buf, "%d", value);
    draw_text(backbuf, x, y, buf, color);
}

void display_score(u8* backbuf, int x, int y, int value, u8 color)
{
    char buf[13];
    buf[0] = '\x8E';
    sprintf(buf + 1, "%d", value);
    draw_text(backbuf, x, y, buf, color);
}