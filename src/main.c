#include <allegro.h>
#include <stdio.h>

#include "ant.h"
#include "multimedia.h"

int main(int argc, char **argv) {

    if (init_allegro())
        return 1;

    spawn_ants(8);

    while (!keypressed()) {
        printf("Resting\n");
        rest(50);
    }

    kill_ants();

    return 0;
}
END_OF_MAIN()

