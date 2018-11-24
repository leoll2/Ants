#include <allegro.h>
#include <pthread.h>
#include <stdio.h>

#include "multimedia.h"


BITMAP* surface = NULL;
unsigned int graphics_tid;

//int x=20, y=20, r=10;

unsigned int init_allegro() {

	if (allegro_init()!=0)
		return 1;

    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) !=0 ) 
        return 2;
    surface = create_bitmap(SCREEN_W, SCREEN_H);

    install_keyboard();
    
    clear_to_color(surface, COLOR_GREEN);
    //circlefill(surface, 100, 100, 50, COLOR_RED);
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    return 0;
}


void *graphics_behaviour(void *arg) {

    clear_bitmap(surface);
    clear_to_color(surface, COLOR_GREEN);

    //TODO: LOCKARE STRUTTURA ants

    for (int i = 0; i < POP_SIZE_MAX; ++i) {
        pthread_mutex_lock(&ants[i].mtx);
        if (ants[i].alive) {
            circlefill(surface, ants[i].pos.x, ants[i].pos.y, 5, COLOR_RED);
        }
        pthread_mutex_unlock(&ants[i].mtx);
    }

    //TODO: UNLOCKARE STRUTTURA ants
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}


unsigned int start_graphics() {

    unsigned int ret;

    ret = init_allegro();
    if (ret)
        return ret;

    ret = start_thread(graphics_behaviour, NULL, SCHED_FIFO, WCET_ANTS, PRD_ANTS, DL_ANTS, PRIO_ANTS);
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