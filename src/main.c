#include <allegro.h>

int main()
{
    if (allegro_init()!=0) return 1;
    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) !=0 ) 
        return 2;
    BITMAP* surface = create_bitmap(SCREEN_W, SCREEN_H);
    install_keyboard();
    clear_to_color(surface, makecol(255,255,255));
    circlefill(surface, 100, 100, 50, makecol(128,128,128));
    blit(surface, screen, 0,0,0,0,SCREEN_W,SCREEN_H);
    
    while (!keypressed())
        rest(20);
    return 0;
}
END_OF_MAIN()

