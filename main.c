
#include "chess.h"
#include <stdio.h>

int main(int argc, char **argv) {

    Game game;

    printf("%lu\n", sizeof(Piece));
    printf("%lu\n", sizeof(Game));


    switch (argc) {
        case 1:
            new_game(&game);
            break;
        case 2:
            load_game(&game, argv[1]);
            break;
        default:
            printf("Usage: chess {+savefile}\n");
            return 1;
    }

    play_game(&game);

}
