#include "sound.h"
#include <tonc.h>
#include "sfx_lengths.h"
//#define FRAME_COUNT(x) ((int)((x) / 16000.0f * 59.7275f + 0.5f))
enum MusicState {
    STOPPED,
    PLAYING,
    PAUSED
};

typedef struct {
    mm_sound_effect effect;
    int frames_left;
    mm_bool active;
} SFXTracker;

enum MusicState state = STOPPED;
mm_word current_song = -1;

#define MAX_SFX 8
EWRAM_DATA SFXTracker sfx_list[MAX_SFX];

void mus_init(void)
{
    mmInitDefault((mm_addr)soundbank_bin, 16);
    for(int i = 0; i < MAX_SFX; ++i)
    {
        sfx_list[i].effect.id = -1;
        sfx_list[i].active = 0;
        sfx_list[i].frames_left = 0;
    }
}

void mus_change(mm_word song)
{
    //mmSetModuleTempo((mm_word)768);
    if(song < MSL_NSONGS)
    {
        if(current_song == song)
        {
            switch(state)
            {
                case PAUSED:
                    mmResume();
                    state = PLAYING;
                    break;
                case PLAYING:
                    mmPause();
                    state = PAUSED;
                    break;
                case STOPPED:
                    mmStart(song, MM_PLAY_LOOP);
                    state = PLAYING;
                    break;
            }
        }
        else
        {
            mus_stop();
            mmStart(song, MM_PLAY_LOOP);
            state = PLAYING;
            current_song = song;
        }
    }
}

void mus_stop(void)
{
    if(state != STOPPED)
    {
        state = STOPPED;
        mmStop();
    }
}

void sfx_play(mm_word sfx, mm_byte priority, mm_byte volume)
{
    int special = priority == 255;
    int openspot = MAX_SFX;
    int playing = 0;
    for(int i = 0; i < MAX_SFX; ++i)
    {
        if(!sfx_list[i].active)
            openspot = i;
        else if(sfx_list[i].effect.id == sfx && !special)
        {
            playing = 1;
            break;
        }
    }

    if(!playing && openspot < MAX_SFX)
    {
        mm_sound_effect* effect = &sfx_list[openspot].effect;
        effect->id = sfx;
        effect->volume = volume;
        effect->panning = 127;
        effect->rate = 1024;

        mmEffectEx(effect);

        if(priority < 1)
            mmEffectRelease(effect->handle);
        sfx_list[openspot].active = 1;
        if(sfx < sfx_lengths_count)
            sfx_list[openspot].frames_left = sfx_lengths[sfx];
    }
}

IWRAM_CODE void sfx_update(void)
{
    for(int i = 0 ; i < MAX_SFX; ++i)
    {
        if(sfx_list[i].active)
        {
            if(sfx_list[i].frames_left-- <= 0)
            {
                sfx_list[i].active = 0;
            }
        }
    }
}

IWRAM_CODE int sfx_check(mm_word sfx)
{
    for(int i = 0 ; i < MAX_SFX; ++i)
    {
        if(sfx_list[i].active && sfx_list[i].effect.id == sfx)
        {
            return 1;
        }
    }
    return 0;
}

IWRAM_CODE void sfx_stop(mm_word sfx)
{
    for(int i = 0 ; i < MAX_SFX; ++i)
    {
        if(sfx_list[i].active && sfx_list[i].effect.id == sfx)
        {
            mmEffectCancel(sfx_list[i].effect.handle);
            sfx_list[i].active = 0;
            break;
        }
    }
}

IWRAM_CODE void sfx_stop_all(void)
{
    for(int i = 0 ; i < MAX_SFX; ++i)
    {
        sfx_list[i].active = 0;
    }
    mmEffectCancelAll();
}