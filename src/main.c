#include <allegro.h>
#include <stdio.h>
#include <stdlib.h>

#include "ant.h"
#include "multimedia.h"
#include "rt_thread.h"

int main(int argc, char **argv) {

	srand(time(NULL));

    init_rt_thread_manager();

    spawn_ants(8);
    if (start_graphics())
    	return 1;

    int k = readkey();

    kill_ants();
    stop_graphics();

    return 0;
}
END_OF_MAIN()

