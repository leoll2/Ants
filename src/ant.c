#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ant.h"

uint8_t stop = 0;
uint8_t n_ants = 0;


void *ant_behaviour(void *arg) {

	struct timespec dt;

	ant *a = (ant *)arg;

	printf("Running ant %d\n", a->id);
}



int spawn_ants(unsigned int n) {

	int id;

	for (int i = 0; i < n; ++i) {
		id = start_thread(ant_behaviour, &ants[i], SCHED_FIFO, WCET_ANTS, PRD_ANTS, DL_ANTS, PRIO_ANTS);
		if (id < 0) {
			printf("Failed to instantiate ant #%d\n", i);
			return 1;
		} else {
			ants[i].id = (unsigned int)id;
			printf("Created ant #%d with id %d\n", i, id);
		}
		++n_ants;	// TODO: ADD LOCK
	}

	return 0;
}


void kill_ants(void) {


	printf("Start killing \n");

	int iter = n_ants;

	for (int i = 0; i < iter; ++i) {
		stop_thread(ants[i].id);
		--n_ants;	// TODO: ADD LOCK
		printf("One stopped!\n");
	}

	if (n_ants > 0)
		printf("For some reason %d ants remained alive\n", n_ants);

	printf("Finished killing \n");
}