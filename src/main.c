#include <allegro.h>
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
    start_pheromones();
    printf("Pheromone decay successfully enabled.\n");

    unsigned int spawned = spawn_ants(DEFAULT_POP);
    printf("Successfully spawned %d ants\n", spawned);

    if (start_graphics())
    	return 1;

    int k = readkey();      // TODO: rimpiazza con una wait_termination(), la quale join il thread tastiera

    kill_ants();
    printf("Ants killate\n");

    stop_graphics();
    stop_pheromones();

    return 0;
}
END_OF_MAIN()
