#include <allegro.h>
#include <pthread.h>
#include <stdio.h>

#include "multimedia.h"


BITMAP* surface = NULL;
unsigned int graphics_tid;

unsigned int init_allegro() {

	if (allegro_init()!=0)
		return 1;

    install_keyboard();

    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) !=0 ) 
        return 2;
    surface = create_bitmap(SCREEN_W, SCREEN_H);
    
    clear_to_color(surface, COLOR_GREEN);
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    return 0;
}


void *graphics_behaviour(void *arg) {

    BITMAP *antbmp;
    antbmp = load_bitmap("img/ant.bmp", NULL);

    clear_bitmap(surface);
    clear_to_color(surface, COLOR_GREEN);

    pthread_mutex_lock(&ants_mtx);

    for (int i = 0; i < POP_SIZE_MAX; ++i) {
        pthread_mutex_lock(&ants[i].mtx);
        if (ants[i].alive) {    
            if (antbmp != NULL) {
                rotate_sprite(surface, antbmp, (ants[i].pos.x - antbmp->w/2), 
                        (ants[i].pos.y - antbmp->h/2), itofix(0));
                //circle(surface, ants[i].pos.x, ants[i].pos.y, 8, COLOR_RED);
            } else 
                circlefill(surface, ants[i].pos.x, ants[i].pos.y, 5, COLOR_RED);  // fallback
        }
        pthread_mutex_unlock(&ants[i].mtx);
    }

    pthread_mutex_unlock(&ants_mtx);
    
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}


unsigned int start_graphics() {

    unsigned int ret;

    ret = init_allegro();
    if (ret)
        return ret;

    ret = start_thread(graphics_behaviour, NULL, SCHED_FIFO, 
            WCET_GRAPHICS, PRD_GRAPHICS, DL_GRAPHICS, PRIO_GRAPHICS);
    if (graphics_tid < 0) {
        printf("Failed to initialize the graphics thread!\n");
        return 1;
    } else {
        graphics_tid = ret;
        printf("Initialized the graphics thread.\n");
    }

    return 0;
}


void stop_graphics() {

    stop_thread(graphics_tid);
    printf("Graphics thread stopped.\n");
}