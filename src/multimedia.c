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
#define ICON_IDLE       "img/icon/idle.bmp"
#define ICON_ADD_FOOD   "img/icon/addfood.bmp"
#define ICON_ADD_ANT    "img/icon/addant.bmp"
#define ICON_KILL_ANT   "img/icon/killant.bmp"
#define ICON_EXIT       "img/icon/exit.bmp"

#define IMG_FOOD_SIZE       40
#define IMG_ANTHILL_SIZE    60


pthread_cond_t terminate = PTHREAD_COND_INITIALIZER;
pthread_mutex_t terminate_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t action_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t selected_ant_mtx = PTHREAD_MUTEX_INITIALIZER;

unsigned int graphics_tid, keyboard_tid, mouse_tid;
BITMAP* surface;
BITMAP* fieldbmp;
BITMAP *antbmp;
BITMAP *foodbmp;
BITMAP *anthillbmp;

typedef enum action {IDLE, ADD_FOOD, ADD_ANT, KILL_ANT, EXIT, N_ACTIONS} action;

action current_action = IDLE;

char* action_keybind[N_ACTIONS] = {
    "Q",
    "W",
    "E",
    "R",
    "ESC"
};

char* action_desc[N_ACTIONS] = {
    "None",
    "Add food",
    "Add ant",
    "Kill ant",
    "Quit"
};

int selected_ant = -1;

char current_message[80];

typedef struct icon {
    int x;
    int y;
    BITMAP* bmp;
} icon;

char* icon_bmp_paths[N_ACTIONS] = {
    ICON_IDLE,
    ICON_ADD_FOOD,
    ICON_ADD_ANT,
    ICON_KILL_ANT,
    ICON_EXIT
};

icon icons[N_ACTIONS];

/* ======================================
*  ============== UTILITY ===============
*  ====================================== */

/* Converts the format of an angle.:
   From: [0, 2*PI) float counterclockwise origin positive x
   To:   [0, 256)  uint  clockwise        origin positive y */
unsigned int angle_float_to_256(float angle) {

    return (64 - (unsigned int)round(128 * (angle / M_PI))) % 256;
}


int get_selected(void) {

    int sel_ant;
    pthread_mutex_lock(&selected_ant_mtx);
    sel_ant = selected_ant;
    pthread_mutex_unlock(&selected_ant_mtx);
    return sel_ant;
}


void set_selected(int id) {

    pthread_mutex_lock(&selected_ant_mtx);
    selected_ant = id;
    pthread_mutex_unlock(&selected_ant_mtx);
}


action get_action(void) {

    action a;
    pthread_mutex_lock(&action_mtx);
    a = current_action;
    pthread_mutex_unlock(&action_mtx);
    return a;
}

void set_action(action a) {

    pthread_mutex_lock(&action_mtx);
    current_action = a;
    pthread_mutex_unlock(&action_mtx);
}



/* ======================================
*  ============== GRAPHICS ==============
*  ====================================== */


void init_icons() {

    int left_padding = (FIELD_WIDTH - (2 * N_ACTIONS - 1) * ICON_SIZE) / 2;

    for (int i = 0; i < N_ACTIONS; i++) {
        icons[i].x = left_padding + 2 * i * ICON_SIZE - ICON_SIZE / 2;
        icons[i].y = FIELD_HEIGHT + TOOLBAR_H / 2 - ICON_SIZE / 2;
        icons[i].bmp = load_bitmap(icon_bmp_paths[i], NULL);
    }
}

unsigned int init_graphics() {

	if (allegro_init()!=0)
		return 1;

    set_color_depth(32);
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, FIELD_WIDTH + STATS_PANEL_W, 
                FIELD_HEIGHT + TOOLBAR_H , 0, 0))
        return 2;

    surface = create_bitmap(SCREEN_W, SCREEN_H);
    fieldbmp = load_bitmap(BG_PATH, NULL);
    foodbmp = load_bitmap(FOOD_PATH, NULL);
    antbmp = load_bitmap(ANT_PATH, NULL);
    anthillbmp = load_bitmap(HOME_PATH, NULL);

    init_icons();
    snprintf(current_message, 80, "%s", "Left-click on an ant to view info about it");

    clear_to_color(surface, COLOR_GREEN);
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

    return 0;
}


void clear_field() {

    blit(fieldbmp, surface, 0, 0, 0, 0, fieldbmp->w, fieldbmp->h);
}



void draw_toolbar(void) {

    int x0 = 0;
    int y0 = FIELD_HEIGHT;

    action a = get_action();

    // Draw background and border
    rectfill(surface, 0, FIELD_HEIGHT, FIELD_WIDTH, SCREEN_H - 1, COLOR_TOOLBAR_BORDER);
    rectfill(surface, 2, FIELD_HEIGHT + 2, FIELD_WIDTH - 2, SCREEN_H - 3, COLOR_TOOLBAR);

    // Display toolbar elements
    for (int i = 0; i < N_ACTIONS; i++) {
        // Draw icon
        if (a == i) {
            rectfill(surface, icons[i].x - 3, icons[i].y - 3, 
                    icons[i].x + ICON_SIZE + 3, icons[i].y + ICON_SIZE + 3, COLOR_ICON_BORDER);
            rectfill(surface, icons[i].x, icons[i].y, 
                    icons[i].x + ICON_SIZE, icons[i].y + ICON_SIZE, COLOR_TOOLBAR);
        } else {
            rect(surface, icons[i].x - 1, icons[i].y - 1, 
                    icons[i].x + ICON_SIZE + 1, icons[i].y + ICON_SIZE + 1, COLOR_ICON_BORDER);
        }
        draw_sprite(surface, icons[i].bmp, icons[i].x, icons[i].y);

        // Draw keybind
        textout_centre_ex(surface, font, action_keybind[i], 
            icons[i].x + ICON_SIZE / 2, icons[i].y - 12, COLOR_TEXT, COLOR_TOOLBAR);

        // Draw description
        textout_centre_ex(surface, font, action_desc[i], 
            icons[i].x + ICON_SIZE / 2, icons[i].y + ICON_SIZE + 8, COLOR_TEXT, COLOR_TOOLBAR);
    }

    // Print the current message
    textout_centre_ex(surface, font, current_message, 
            x0 + FIELD_WIDTH / 2, y0 + 7, COLOR_TEXT, COLOR_TOOLBAR);
}


void draw_selected_ant_stats(int x0, int y0) {

    char buf[60];
    int sel_ant;
    unsigned int sel_tid;
    position sel_pos;
    float sel_aud;
    float sel_exc;
    phero_type sel_int;
    behaviour sel_beh;

    hline(surface, FIELD_WIDTH, STATS_ANT_OFF_H, SCREEN_W - 1, COLOR_TOOLBAR_BORDER);
    textout_centre_ex(surface, font, "ANT", 
            x0 + (SCREEN_W - FIELD_WIDTH) / 2, STATS_ANT_OFF_H + 10, COLOR_TEXT, COLOR_STATS_PANEL);

    sel_ant = get_selected();
    if (sel_ant >= 0)
        sprintf(buf, "%-16s %d", "Selected: ", sel_ant);
    else
        sprintf(buf, "%-16s %s", "Selected: ", "none");
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 40, COLOR_TEXT, COLOR_STATS_PANEL);

    if (sel_ant >= 0 && sel_ant < POP_SIZE_MAX) {
        pthread_mutex_lock(&ants[sel_ant].mtx);

        if (!ants[sel_ant].alive) {
            pthread_mutex_unlock(&ants[sel_ant].mtx);
            set_selected(-1);
            return;
        }
            
        sel_tid = ants[sel_ant].tid;
        sel_pos = ants[sel_ant].pos;
        sel_int = ants[sel_ant].interest;
        sel_beh = ants[sel_ant].behaviour;
        sel_aud = ants[sel_ant].audacity;
        sel_exc = ants[sel_ant].excitement;
        pthread_mutex_unlock(&ants[sel_ant].mtx);
    } else
        return;

    sprintf(buf, "%-16s (%d %d)", "Pos (x,y):", sel_pos.x, sel_pos.y);
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 60, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %f", "Angle (deg):", 360 / TWO_PI * sel_pos.angle);
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 80, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %s", "Activity:", ((sel_beh == EXPLORING) || (sel_beh == EXPLOITING)) ? "Searching" : 
            (sel_beh == EATING ? "Eating" : "Resting"));
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 100, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %s", "Looking for:", (sel_int == FOOD) ? "Food" : "Home");
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 120, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %f", "Audacity:", sel_aud);
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 140, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %f", "Excitement:", sel_exc);
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 160, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %d", "Deadline misses:", how_many_dl_missed(sel_tid));
    textout_ex(surface, font, buf, x0 + 10, STATS_ANT_OFF_H + 180, COLOR_TEXT, COLOR_STATS_PANEL);
}


void draw_stats_panelbox(void) {

    int x0 = FIELD_WIDTH;
    int y0 = 0;
    char buf[60];

    // Draw background and border
    rectfill(surface, FIELD_WIDTH, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_STATS_BORDER);
    rectfill(surface, FIELD_WIDTH + 2, 2, SCREEN_W - 3, SCREEN_H - 3, COLOR_STATS_PANEL);

    textout_centre_ex(surface, font, "STATUS", 
            x0 + (SCREEN_W - FIELD_WIDTH) / 2, y0 + 10, COLOR_TEXT, COLOR_STATS_PANEL);

    // Print stats
    sprintf(buf, "%-16s %d", "Ants:", n_ants);
    textout_ex(surface, font, buf, x0 + 10, y0 + 40, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %d", "Food sources:", n_food_src);
    textout_ex(surface, font, buf, x0 + 10, y0 + 60, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s", "Deadline misses");
    textout_ex(surface, font, buf, x0 + 10, y0 + 80, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %d", " - Graphics:", how_many_dl_missed(graphics_tid));
    textout_ex(surface, font, buf, x0 + 10, y0 + 100, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %d", " - Mouse:", how_many_dl_missed(mouse_tid));
    textout_ex(surface, font, buf, x0 + 10, y0 + 120, COLOR_TEXT, COLOR_STATS_PANEL);
    sprintf(buf, "%-16s %d", " - Keyboard:", how_many_dl_missed(keyboard_tid));
    textout_ex(surface, font, buf, x0 + 10, y0 + 140, COLOR_TEXT, COLOR_STATS_PANEL);

    draw_selected_ant_stats(x0, y0);
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


static inline void draw_ant(int i, bool selected) {

    ant *a = &ants[i];
    pthread_mutex_lock(&a->mtx);

    if (a->alive) {
        if (antbmp != NULL) {
            rotate_sprite(surface, antbmp, (a->pos.x - antbmp->w/2),
                    (a->pos.y - antbmp->h/2), itofix(angle_float_to_256(a->pos.angle)));
            if (selected)
                circle(surface, a->pos.x, a->pos.y, 15, COLOR_RED);
        } else
            circlefill(surface, a->pos.x, a->pos.y, CELL_SIZE / 2, COLOR_RED);  // fallback
    }

    pthread_mutex_unlock(&a->mtx);

}


void draw_anthill(void) {

    draw_sprite(surface, anthillbmp, HOME_X - IMG_ANTHILL_SIZE / 2, HOME_Y - IMG_ANTHILL_SIZE / 2);
}



void draw_food(void) {

    for (int i = 0; i < MAX_FOOD_SRC; ++i) {
        if (foods[i].units > 0)
            draw_sprite(surface, foodbmp, 
                    foods[i].x - IMG_FOOD_SIZE / 2, foods[i].y - IMG_FOOD_SIZE / 2);
    }
}


void *graphics_behaviour(void *arg) {

    clear_field();

    // Draw anthill
    draw_anthill();

    // Draw food
    draw_food();

    // Draw ants
    int sel_ant = get_selected();
    for (int i = 0; i < POP_SIZE_MAX; ++i)
        draw_ant(i, (i == sel_ant));

    // Draw pheromones
    for (int i = 0; i < PH_SIZE_H; ++i)
        for (int j = 0; j < PH_SIZE_V; ++j)
            draw_pheromone(i, j);

    draw_toolbar();
    draw_stats_panelbox();
    
    blit(surface, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    show_mouse(screen);
}


unsigned int start_graphics(void) {

    unsigned int ret;

    ret = start_thread(graphics_behaviour, NULL, SCHED_FIFO,
            WCET_GRAPHICS, PRD_GRAPHICS, DL_GRAPHICS, PRIO_GRAPHICS);
    if (ret < 0) {
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



/* ======================================
*  ============== KEYBOARD ==============
*  ====================================== */


void get_keycodes(char *scan, char *ascii) {

    int k;
    k = readkey();  // read the key
    *ascii = k;     // extract ascii code
    *scan = k >> 8; // extract scan code
}


void *keyboard_behaviour(void *arg) {

    int key;
    char ascii, scan;
    int ret;

    while (keypressed()) {
        action a = get_action();
        get_keycodes(&scan, &ascii);
        switch (scan) {
            case KEY_ESC:
                set_action(EXIT);
                snprintf(current_message, 80, "%s", "Closing...");
                pthread_mutex_lock(&terminate_mtx);
                pthread_cond_signal(&terminate);
                pthread_mutex_unlock(&terminate_mtx);
                break;
            case KEY_SPACE:
                printf("Pressed spacebar\n");
                break;
            case KEY_Q:
                set_action(IDLE);
                snprintf(current_message, 80, "%s", "Left-click on an ant to view info about it");
                break;
            case KEY_W:
                set_action(ADD_FOOD);
                snprintf(current_message, 80, "%s", "Left-click on the field to add food");
                break;
            case KEY_E:
                set_action(ADD_ANT);
                if (spawn_ant() == 0)
                    snprintf(current_message, 80, "%s", "Ant spawned!");
                else
                    snprintf(current_message, 80, "%s", "Failed to spawn a new ant (too many)");
                break;
            case KEY_R:
                set_action(KILL_ANT);
                snprintf(current_message, 80, "%s", "Left-click on the ant to kill");
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
    if (ret < 0) {
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



/* ======================================
*  =============== MOUSE ================
*  ====================================== */


void *mouse_behaviour(void *arg) {

    int x, y;
    int mbutton;

    // Check if any button is being clicked, else return
    mbutton = mouse_b & 3;
    if (mbutton) {
        x = mouse_x;
        y = mouse_y;
    } else
        return NULL;

    action a = get_action();

    switch(mbutton) {
        int ant_id, ret;
        case 1:     // Left-click
            switch(a) {
                case ADD_FOOD:
                    ret = deploy_food(x, y);
                    if (ret == 0) {
                        snprintf(current_message, 80, "%s", "Food placed!");
                        set_action(IDLE);
                    } else if (ret == -1) {
                        snprintf(current_message, 80, "%s", "There is already enough food");
                    }
                    break;
                case KILL_ANT:
                    ant_id = get_ant_id_by_pos(x, y);
                    if (ant_id >= 0 && kill_ant(ant_id) > 0) {
                        snprintf(current_message, 80, "%s", "Killed ant!");
                        set_action(IDLE);
                    }
                    break;
                case IDLE:
                    ant_id = get_ant_id_by_pos(x, y);
                    if (ant_id >= 0) {
                        set_selected(ant_id);
                    } else
                        set_selected(-1);
                    break;
                default:
                    break;
            }
            break;
        case 2:     // Right-click
            printf("Right click at coords(%d, %d)!\n", x, y);
            break;
        case 3:     // Both clicks
            printf("Both clicks at coords(%d, %d)!\n", x, y);
            break;
        default:
            break;
    }
}


unsigned int start_mouse(void) {

    unsigned int ret;

    ret = start_thread(mouse_behaviour, NULL, SCHED_FIFO,
            WCET_MOUSE, PRD_MOUSE, DL_MOUSE, PRIO_MOUSE);
    if (ret < 0) {
        printf("Failed to initialize the mouse thread!\n");
        return 1;
    } else {
        mouse_tid = ret;
        printf("Initialized the mouse thread with id #%d.\n", mouse_tid);
    }

    return 0;
}


void stop_mouse(void) {

    stop_thread(mouse_tid);
    printf("Mouse thread stopped.\n");
}


unsigned int init_multimedia() {

    if (init_graphics())
        return 1;

    install_mouse();
    install_keyboard();

    if (start_graphics() || start_mouse() || start_keyboard())
        return 1;

    return 0;
}


void stop_multimedia() {

    stop_keyboard();
    stop_mouse();
    stop_graphics();
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