#include <tonc.h>
#include <maxmod.h>

#include "soundbank.h"
#include "global.h"
#include "mode3d.h"
#include "sound.h"

int main(void)
{
    irq_init(NULL);
    irq_add(II_VBLANK, mmVBlank);
    irq_enable(II_VBLANK);
    mus_init();
    //mus_change(MOD_FF1_BATT);
    
    int dma = 0;
    while(1)
    {
        sfx_update();
        mmFrame();
        key_poll();
        switch(current_state)
        {
            case GS_Menu:
                break;
            case GS_init2d:
                break;
            case GS_run2d: 
                break;
            case GS_init3d:
                init3d();
                break;
            case GS_attract3d:
                dma = attract3d();
                break;
            case GS_run3d:
                dma = run3d();
                break;
            case GS_model_view3d:
                dma = model_viewer3d();
                break;
        }
        ++frame_count;
        
        VBlankIntrWait();
        if(dma)
            dma3d();
    }
    return 0;
}
