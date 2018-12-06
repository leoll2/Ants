#include <allegro.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <allegro/keyboard.h>

#include "field.h"
#include "multimedia.h"


#define BG_PATH         "img/dirt.bmp"
#define ANT_PATH        "img/ant.bmp"
#define FOOD_PATH       "img/apple.bmp"
#define HOME_PATH       "img/anthill.bmp"

#define IMG_FOOD_SIZE       40
#define IMG_ANTHILL_SIZE    60


pthread_cond_t terminate = PTHREAD_COND_INITIALIZER;
pthread_mutex_t terminate_mtx = PTHREAD_MUTEX_INITIALIZER;

unsigned int graphics_tid, keyboard_tid;
BITMAP* surface = NULL;


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
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, FIELD_WIDTH + STATS_PANEL_W, 
                FIELD_HEIGHT + TOOLBAR_H , 0, 0))
        return 2;
    surface = create_bitmap(SCREEN_W, SCREEN_H);

    clear_to_color(surface, COLOR_GREEN);
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    return 0;
}


void clear_to_background() {

    surface = load_bitmap(BG_PATH, NULL);
}



void draw_toolbar(void) {

    rectfill(surface, 0, FIELD_HEIGHT, FIELD_WIDTH, SCREEN_H, COLOR_BLACK);
}



void draw_stats_panelbox(void) {

    rectfill(surface, FIELD_WIDTH, 0, SCREEN_W, SCREEN_H, COLOR_WHITE);
}



static inline unsigned int phero_radius(float value) {

    return (unsigned int)ceil((CELL_SIZE / 2) * (value / SMELL_UNIT));
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



void draw_anthill(BITMAP *anthillbmp) {

    draw_sprite(surface, anthillbmp, HOME_X - IMG_ANTHILL_SIZE / 2, HOME_Y - IMG_ANTHILL_SIZE / 2);
}



void draw_food(BITMAP *foodbmp) {

    draw_sprite(surface, foodbmp, FOOD_X - IMG_FOOD_SIZE / 2, FOOD_Y - IMG_FOOD_SIZE / 2);
}



void *graphics_behaviour(void *arg) {

    char s[20];

    BITMAP *antbmp;
    antbmp = load_bitmap(ANT_PATH, NULL);
    BITMAP *foodbmp;
    foodbmp = load_bitmap(FOOD_PATH, NULL);
    BITMAP *anthillbmp;
    anthillbmp = load_bitmap(HOME_PATH, NULL);

    clear_to_background();
    draw_toolbar();
    draw_stats_panelbox();

    // Draw anthill
    draw_anthill(anthillbmp);

    // Draw food
    draw_food(foodbmp);

    // Draw ants
    for (int i = 0; i < POP_SIZE_MAX; ++i)
        draw_ant(antbmp, i);

    // Draw pheromones
    for (int i = 0; i < PH_SIZE_H; ++i)
        for (int j = 0; j < PH_SIZE_V; ++j)
            draw_pheromone(i, j);
    textout_ex ( screen, font, "tasto aggiungi formica: A ; uccidi formica: K; arresta simulazione: ESC", 
                0, FIELD_HEIGHT + TOOLBAR_H/2, COLOR_RED, 1  );
    sprintf(s,"%d", n_ants);
    textout_ex ( screen, font, "numero ants:" , TOOLBAR_W + 10 , FIELD_HEIGHT, COLOR_RED, 1  );
    textout_ex ( screen, font, s , TOOLBAR_W + 110 , FIELD_HEIGHT, COLOR_RED, 1  );
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}


unsigned int start_graphics(void) {

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

void stop_graphics(void) {

    stop_thread(graphics_tid);
    printf("Graphics thread stopped.\n");
}



void get_keycodes(char *scan, char *ascii) {

    int k;
    k = readkey();  // read the key
    *ascii = k;     // extract ascii code
    *scan = k >> 8; // extract scan code
}




void *keyboard_behaviour(void *arg) {

    int key;
    char ascii, scan;

    while (keypressed()) {
        get_keycodes(&scan, &ascii);
        // TODO: FARE COSE IN BASE AI TASTI PREMUTI
        switch (scan) {
            case KEY_ESC:
                pthread_mutex_lock(&terminate_mtx);
                pthread_cond_signal(&terminate);
                pthread_mutex_unlock(&terminate_mtx);
                break;
            case KEY_SPACE:
                printf("Pressed spacebar\n");
                break;
            default:
                printf("Press ESC to quit!\n");
        }
    }
}


unsigned int start_keyboard(void) {

    unsigned int ret;

    ret = start_thread(keyboard_behaviour, NULL, SCHED_FIFO,
            WCET_KEYBOARD, PRD_KEYBOARD, DL_KEYBOARD, PRIO_KEYBOARD);
    if (keyboard_tid < 0) {
        printf("Failed to initialize the keyboard thread!\n");
        return 1;
    } else {
        keyboard_tid = ret;
        printf("Initialized the keyboard thread with id #%d.\n", keyboard_tid);
    }
    return 0;
}



void stop_keyboard(void) {

    stop_thread(keyboard_tid);
    printf("Keyboard thread stopped.\n");
}


void wait_for_termination(void) {

    pthread_mutex_lock(&terminate_mtx);
    pthread_cond_wait(&terminate, &terminate_mtx);
    pthread_mutex_unlock(&terminate_mtx);
}

void increment_ants_command(int k){
    if ( (k >> 8)  == KEY_LEFT)
        spawn_ant();
}