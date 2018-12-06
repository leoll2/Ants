#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include "ant.h"
#include "conf/field.h"
#include "conf/multimedia.h"
#include "rt_thread.h"


unsigned int start_graphics(void);

unsigned int start_keyboard(void);

void stop_graphics(void);

void stop_keyboard(void);

void wait_for_termination(void);

#endif