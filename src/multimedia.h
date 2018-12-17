#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include "ant.h"
#include "conf/field.h"
#include "conf/multimedia.h"
#include "rt_thread.h"

/* ======================== 
*  ======= GRAPHICS =======
*  ======================== */
//unsigned int start_graphics(void);
//void stop_graphics(void);

/* ======================== 
*  ======= KEYBOARD =======
*  ======================== */
//unsigned int start_keyboard(void);
//void stop_keyboard(void);

/* ======================== 
*  ======== MOUSE =========
*  ======================== */
//unsigned int start_mouse(void);
//void stop_mouse(void);

unsigned int init_multimedia(void);

void stop_multimedia(void);

void wait_for_termination(void);

#endif