#include <stdint.h>
#include <stdio.h>

#include "ant.h"

uint8_t stop = 0;


void *ant_behaviour(void *p) {

	while(!stop) {
		printf("Executing thread: %d\n", pthread_self());
	}
}


int spawn_ants(unsigned int n_ants) {

	int ret;

	ret = pthread_create(&ant1, NULL, ant_behaviour, NULL);
	ret = pthread_create(&ant2, NULL, ant_behaviour, NULL);

}

void kill_ants() {

	stop = 1;

	pthread_join(ant1, NULL);
	pthread_join(ant2, NULL);
}