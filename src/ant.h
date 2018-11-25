#ifndef ANT_H
#define ANT_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "conf/ants.h"
#include "rt_thread.h"

typedef struct position {
	uint16_t x;
	uint16_t y;
	float angle;
} position;


typedef struct ant {
	bool alive;
	unsigned int id;	// TODO: useful to be killed
	position pos;
	pthread_mutex_t mtx;
} ant;


extern ant ants[POP_SIZE_MAX];
extern uint8_t n_ants;
extern pthread_mutex_t ants_mtx;


void *ant_behaviour(void *arg);

int spawn_ants(unsigned int n_ants);

void kill_ants(void);


#endif