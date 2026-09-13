#ifndef SOUND_H
#define SOUND_H

#include <maxmod.h>
#include "soundbank_bin.h"
#include "soundbank.h"

extern mm_word current_song;

void mus_init(void);
void mus_change(mm_word song);
void mus_stop(void);
void sfx_play(mm_word sfx, mm_byte priority, mm_byte volume);
int sfx_check(mm_word sfx);
void sfx_update(void);
void sfx_stop(mm_word sfx);
void sfx_stop_all(void);

#endif