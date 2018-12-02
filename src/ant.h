#ifndef ANT_H
#define ANT_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "conf/ants.h"
#include "conf/field.h"
#include "field.h"
#include "rt_thread.h"

typedef enum behaviour {EXPLORING, TRACKING, EATING, RESTING} behaviour;

typedef struct position {
	int 			x;			// [pixel] x coordinate
	int 			y;			// [pixel] y coordinate
	float 			angle;		// [radians] angle in [0, 2*pi)
} position;


typedef struct ant {
	bool 			alive;		// is ant allocated? is the data meaningful?
	unsigned int 	id;			// unique ant identifier
	position 		pos;		// ant position
	phero_type		interest;	// looking for food or home?
	behaviour		behaviour;	// following a trail or exploring?
	pthread_mutex_t mtx;		// mutex to protect the struct
} ant;


extern ant ants[POP_SIZE_MAX];
extern uint8_t n_ants;
extern pthread_mutex_t ants_mtx;


int spawn_ants(unsigned int n_ants);

void kill_ants(void);


#endif