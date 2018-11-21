#include <allegro.h>

#include "ant.h"
#include "multimedia.h"

int main() {

    if (init_allegro())
        return 1;

    spawn_ants(8);

    while (!keypressed()) {
        rest(50);
    }

    kill_ants();

    return 0;
}
END_OF_MAIN()

