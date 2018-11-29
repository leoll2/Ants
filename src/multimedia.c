#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include "field.h"
#include "multimedia.h"


BITMAP* surface = NULL;
unsigned int graphics_tid;


/* Converts the format of an angle.:
   From: [0, 2*PI) float counterclockwise origin positive x
   To:   [0, 256)  uint  clockwise        origin positive y
 */
unsigned int angle_float_to_256(float angle) {

    return (64 - (unsigned int)round(128 * (angle / M_PI))) % 256;
}


unsigned int init_allegro() {

	if (allegro_init()!=0)
		return 1;

    install_keyboard();

    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, FIELD_WIDTH, FIELD_HEIGHT, 0, 0)) 
        return 2;
    surface = create_bitmap(SCREEN_W, SCREEN_H);
    
    clear_to_color(surface, COLOR_GREEN);
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    return 0;
}


void clear_to_background() {

    surface = load_bitmap("img/dirt.bmp", NULL);
}


static inline unsigned int phero_radius(float value) {

    return (unsigned int)ceil((CELL_SIZE / 2) * (value / SMELL_UB));
}


static inline void draw_pheromone(int i, int j) {

    pthread_mutex_lock(&ph[i][j].mtx);

    if (ph[i][j].food > 0) {
        circle(surface, 
                i * CELL_SIZE + CELL_SIZE / 2, 
                j * CELL_SIZE + CELL_SIZE / 2, 
                phero_radius(ph[i][j].food), 
                COLOR_RED);
    }
    if (ph[i][j].home > 0) {
        circle(surface, 
                i * CELL_SIZE + CELL_SIZE / 2, 
                j * CELL_SIZE + CELL_SIZE / 2, 
                phero_radius(ph[i][j].home), 
                COLOR_BLUE);
    }
    pthread_mutex_unlock(&ph[i][j].mtx);
}


static inline void draw_ant(BITMAP *antbmp, int i) {

    ant *a = &ants[i];
    pthread_mutex_lock(&a->mtx);

    if (a->alive) {    
        if (antbmp != NULL) {
            rotate_sprite(surface, antbmp, (a->pos.x - antbmp->w/2), 
                    (a->pos.y - antbmp->h/2), itofix(angle_float_to_256(a->pos.angle)));
        } else 
            circlefill(surface, a->pos.x, a->pos.y, CELL_SIZE / 2, COLOR_RED);  // fallback
    }

    pthread_mutex_unlock(&a->mtx);

}

void *graphics_behaviour(void *arg) {

    BITMAP *antbmp;
    antbmp = load_bitmap("img/ant.bmp", NULL);  

    clear_to_background();

    // Draw ants
    for (int i = 0; i < POP_SIZE_MAX; ++i)
        draw_ant(antbmp, i);

    // Draw pheromones
    for (int i = 0; i < PH_SIZE_H; ++i)
        for (int j = 0; j < PH_SIZE_V; ++j)
            draw_pheromone(i, j);
  
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
        printf("Initialized the graphics thread with id #%d.\n", graphics_tid);
    }

    return 0;
}


void stop_graphics() {

    stop_thread(graphics_tid);
    printf("Graphics thread stopped.\n");
}