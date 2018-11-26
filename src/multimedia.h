#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include "ant.h"
#include "conf/field.h"
#include "conf/multimedia.h"
#include "rt_thread.h"


unsigned int start_graphics();

void stop_graphics();

void *graphics_behaviour(void *arg);

#endif