#ifndef MULTIMEDIA_H
#define MULTIMEDIA_H

#include "ant.h"
#include "conf/field.h"
#include "conf/multimedia.h"
#include "rt_thread.h"


unsigned int init_multimedia(void);

void stop_multimedia(void);

void wait_for_termination(void);

#endif