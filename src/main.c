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
    printf("Loading...\n");

    /* Initialize data structures to support the creation of real-time threads */
    init_rt_thread_manager();
    printf("Thread manager successfully initialized.\n");

    /* Initialize data structures needed to spawn/kill ants */
    init_ants_manager();
    printf("Ant manager successfully initialized.\n");

    /* Initialize data structures needed to spawn food */
    init_foods();
    printf("Food manager successfully initialized.\n");

    /* Initialize the thread which handles pheromones decay */
    start_pheromones();
    printf("Pheromone decay successfully enabled.\n");

    /* Spawn a few ants */
    unsigned int spawned = spawn_ants(DEFAULT_POP);
    printf("Successfully spawned %d ants.\n", spawned);

    /* Initialize graphics, mouse and keyboard */
    if (init_multimedia())
        return 1;

    /* Wait for the end of simulation (ESC key pressed by user) */
    wait_for_termination();

    /* Kill all the ants */
    kill_ants();
    printf("Ants killed.\n");

    /* Halt graphics, mouse and keyboard */
    stop_multimedia();

    /* Stop the thread for pheromones decay */
    stop_pheromones();

    return 0;
}
END_OF_MAIN()
