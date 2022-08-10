#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BOARD_SIZE 8

const char * white_name = "white";
const char * black_name = "black";

typedef enum PieceType {NONE=0, KING='K', QUEEN='Q', ROOK='R', BISHOP='B', PAWN='p', KNIGHT='N'} PieceType;
typedef enum BoardColor {B_BLACK=250, B_WHITE=15} BoardColor;
typedef enum Team{WHITE=26, BLACK=196} Team;
typedef enum GameState{CONTINUE, END, ERROR, SAVE} GameState;

typedef struct Piece {
    PieceType type : 8;
    Team team : 8;
    unsigned int moves;
} Piece;

typedef Piece Board[BOARD_SIZE][BOARD_SIZE];

typedef struct Game {
    Board board;
    Team turn : 8;
    GameState state : 2;
} Game;

void draw_board(Board board) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d \x1b[1m", BOARD_SIZE - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("\x1b[48;5;%um", (i + j) & 1 ? B_BLACK : B_WHITE);
            if (board[i][j].type) {
                printf("\x1b[38;5;%um", board[i][j].team);
                printf(" %c ", board[i][j].type);
            } else {
                printf("   ");
            }
        }
        printf("\x1b[0;0m\n");
    }
    printf("   a  b  c  d  e  f  g  h\n");
}

void str_to_board(char *col, char *row) {
    *row = BOARD_SIZE - (*row - '0');
    *col -= 'a';
}

int get_move(char* buf, char *fr, char *fc, char *tr, char *tc) {
    if (sscanf(buf, "%c%c:%c%c", fc, fr, tc, tr) != 4) {
        puts("bad format");
        return 0;
    }
    if (*fr > '8' || *fr < '1' || *tr > '8' || *tr < '1' ||
            *fc > 'h' || *fc < 'a' || *tc > 'h' || *tc < 'a') {
        puts("bad coords");
        return 0;
    }
    str_to_board(fc, fr);
    str_to_board(tc, tr);
    //printf("chose: %c at %d, %d", game->board[*fr][*fc]->type, *fr, *fc);
    return 1;
}

bool is_move_valid_rook(Game* game, char fr, char fc, char tr, char tc) {
    int dr = fr - tr, dc = fc - tc;
    int dir, steps;

    if (dc == 0) {
        dir = dr > 0 ? 1 : -1;
        steps = dr;
    } else if(dr == 0) {
        dir = dc > 0 ? 1 : -1;
        steps = dc;
    } else {
        return false;
    }

    return false;
}

int is_valid_delta(Game* game, char fr, char fc, char tr, char tc) {
    int dr = fr - tr, dc = fc - tc;

    switch (game->board[fr][fc].type) {
        case PAWN:
            return true;
        case ROOK:
            return dc == 0 || dr == 0;
        case KNIGHT:
            return (abs(dc) == 1 && abs(dr) == 2) || (abs(dc) == 2 && abs(dr) == 1);
        case BISHOP:
            return abs(dc) == abs(dr);
        case KING:
            return abs(dc) <= 1 && abs(dr) <= 1;
        case QUEEN:
            return abs(dc) == abs(dr) || dc == 0 || dr == 0;
        default:
            return false;
    }
    /*if(p->team == WHITE) {
        return dc == 0 (dr == 1)
    } else {

    }*/
    return 0;
}

int is_valid_move(Game* game, char fr, char fc, char tr, char tc) {
    Piece *fp = &game->board[fr][fc];
    Piece *tp = &game->board[tr][tc];
    if (!fp->type) {
        puts("No piece found!");
        return 0;
    }
    if (fp->team != game->turn) {
        puts("Not your piece!");
        return 0;
    }
    if (tp->type && tp->team == game->turn) {
        puts("Overlap!");
        return 0;
    }
    if (!is_valid_delta(game, fr, fc, tr, tc)) {
        puts("invalid move!");
        return 0;
    }
    return 1;
}

void loop(Game* game) {
    char fr, fc, tr, tc;
    while (1) {
        char buf[10];
        printf("%s's turn: ", game->turn == WHITE ? white_name : black_name);
        if (!fgets(buf, 10, stdin)) {
            puts("input failed");
            game->state = END;
            return;
        }
        if (buf[0] == 'k') {
            game->state = SAVE;
            break;
        }

        if (!get_move(buf, &fr, &fc, &tr, &tc)) continue;
        if (!is_valid_move(game, fr, fc, tr, tc)) continue;

        game->board[tr][tc] = game->board[fr][fc];
        game->board[fr][fc].type = NONE;
        game->board[tr][tc].moves++;
        game->turn = game->turn == WHITE ? BLACK : WHITE;
        break;
    }
}

void save_game(Game * game, char * filename) {
    FILE* save_file;
    if (!(save_file = fopen(filename, "wb"))){
        return;
    }
    fwrite( game, sizeof(Game),1,save_file);
    fclose(save_file);
    printf("saved!\n");
}

void load_game(Game * game, char * filename) {
    FILE* save_file;
    if (!(save_file = fopen(filename, "rb"))){
        return;
    }
    fgets((char *) game , sizeof(Game), save_file);
    fclose(save_file);
    printf("opened!\n");
}

int main() {
    Game game = {{{ROOK, BLACK,0,KNIGHT, BLACK,0,BISHOP, BLACK, 0,QUEEN, BLACK,0,KING, BLACK,0,BISHOP, BLACK,0,KNIGHT, BLACK,0,ROOK, BLACK, 0},
                  {PAWN, BLACK,0,PAWN, BLACK,0,PAWN, BLACK,0,PAWN, BLACK,0,PAWN, BLACK,0,PAWN, BLACK,0,PAWN, BLACK,0,PAWN, BLACK, 0},
                  {},{},{},{},
                  {PAWN, WHITE, 0, PAWN, WHITE,0,PAWN, WHITE,0,PAWN, WHITE,0,PAWN, WHITE,0,PAWN, WHITE,0,PAWN, WHITE,0,PAWN, WHITE, 0},
                  {ROOK, WHITE,0,KNIGHT, WHITE,0,BISHOP, WHITE, 0,QUEEN, WHITE,0,KING, WHITE,0,BISHOP, WHITE,0,KNIGHT, WHITE,0,ROOK, WHITE, 0},
                 },
                 WHITE,
                 CONTINUE};
//    Game game;
//    load_game(&game, "save.sav");

    //printf("%lu\n", sizeof(Game));

    do {
        switch (game.state) {
            case CONTINUE:
                draw_board(game.board);
                loop(&game);
                break;
            case SAVE:
                game.state = CONTINUE;
                save_game(&game, "save.sav");
                break;
            case ERROR:
                fprintf(stderr, "An error occurred!\n");
            case END:
                return 0;
        }

    } while (game.state != END);
}
