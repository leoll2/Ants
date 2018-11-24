#ifndef ANT_H
#define ANT_H

#include <pthread.h>

#include "conf/ants.h"
#include "rt_thread.h"

typedef struct position {
	uint16_t x;
	uint16_t y;
	float angle;
} position;


typedef struct ant {
	position pos;
	unsigned int id;	// TODO: useful to be killed
} ant;

ant ants[POP_SIZE_MAX];



void *ant_behaviour(void *arg);


int spawn_ants(unsigned int n_ants);

void kill_ants(void);


#endif