#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>

#include "ant.h"
#include "field.h"
#include "multimedia.h"
#include "rt_thread.h"


int main(int argc, char **argv) {

	srand(time(NULL));

    init_rt_thread_manager();

    start_pheromones();
    spawn_ants(8);
    if (start_graphics())
    	return 1;

    int k = readkey();

    kill_ants();
    stop_graphics();
    stop_pheromones();

    return 0;
}
END_OF_MAIN()

