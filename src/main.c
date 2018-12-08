#include <allegro.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "ant.h"
#include "field.h"
#include "multimedia.h"
#include "rt_thread.h"




int main(int argc, char **argv) {

	srand(time(NULL));
    printf("Inizio main\n");

    init_rt_thread_manager();
    printf("Thread manager successfully initialized.\n");
    init_ants_manager();
    printf("Ant manager successfully initialized.\n");
    init_foods();
    printf("Food manager successfully initialized.\n");
    start_pheromones();
    printf("Pheromone decay successfully enabled.\n");

    unsigned int spawned = spawn_ants(DEFAULT_POP);
    printf("Successfully spawned %d ants.\n", spawned);

    if (start_graphics())
    	return 1;
    if (start_keyboard())
        return 1;
    if (start_mouse())
        return 1;

    wait_for_termination();

    kill_ants();
    printf("Ants killed.\n");

    stop_keyboard();
    stop_graphics();
    stop_pheromones();

    return 0;
}
END_OF_MAIN()
