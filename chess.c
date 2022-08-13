//
// Created by idan alperin on 07/08/2022.
//

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "chess.h"

#define BUF_SIZE 50
#define COMMAND_SIZE 5
#define NUM_OF_PIECES 7
//#define USE_UNICODE
//#define GOD_MODE

typedef enum BoardColor {
    B_BLACK = 250, B_WHITE = 15
} BoardColor;

//typedef Piece TeamArray[16];

typedef struct SaveGame {
    TeamPieces teams[2];
    Team turn: 1;
    GameState state: 2;
} SaveGame;

const SaveGame new_game_template = {{{KING, WHITE, false, false, true, 4, 7, 0,
                                      QUEEN, WHITE, false, false, true, 3, 7, 0,
                                      BISHOP, WHITE, false, false, true, 2, 7, 0,
                                      BISHOP, WHITE, false, false, true, 5, 7, 0,
                                      KNIGHT, WHITE, false, false, true, 1, 7, 0,
                                      KNIGHT, WHITE, false, false, true, 6, 7, 0,
                                      ROOK, WHITE, false, false, true, 0, 7, 0,
                                      ROOK, WHITE, false, false, true, 7, 7, 0,
                                      PAWN, WHITE, false, false, true, 0, 6, 0,
                                      PAWN, WHITE, false, false, true, 1, 6, 0,
                                      PAWN, WHITE, false, false, true, 2, 6, 0,
                                      PAWN, WHITE, false, false, true, 3, 6, 0,
                                      PAWN, WHITE, false, false, true, 4, 6, 0,
                                      PAWN, WHITE, false, false, true, 5, 6, 0,
                                      PAWN, WHITE, false, false, true, 6, 6, 0,
                                      PAWN, WHITE, false, false, true, 7, 6, 0},
                                     {KING, BLACK, false, false, true, 4, 0, 0,
                                      QUEEN, BLACK, false, false, true, 3, 0, 0,
                                      BISHOP, BLACK, false, false, true, 2, 0, 0,
                                      BISHOP, BLACK, false, false, true, 5, 0, 0,
                                      KNIGHT, BLACK, false, false, true, 1, 0, 0,
                                      KNIGHT, BLACK, false, false, true, 6, 0, 0,
                                      ROOK, BLACK, false, false, true, 0, 0, 0,
                                      ROOK, BLACK, false, false, true, 7, 0, 0,
                                      PAWN, BLACK, false, false, true, 0, 1, 0,
                                      PAWN, BLACK, false, false, true, 1, 1, 0,
                                      PAWN, BLACK, false, false, true, 2, 1, 0,
                                      PAWN, BLACK, false, false, true, 3, 1, 0,
                                      PAWN, BLACK, false, false, true, 4, 1, 0,
                                      PAWN, BLACK, false, false, true, 5, 1, 0,
                                      PAWN, BLACK, false, false, true, 6, 1, 0,
                                      PAWN, BLACK, false, false, true, 7, 1, 0}}};

const char *white_name = "white";
const char *black_name = "black";

#ifdef USE_UNICODE
const char * piece_str[] = {NULL, "♔", "♕", "♖", "♗", "♘", "♙", NULL, "♚", "♛", "♜", "♝", "♞", "♟"};
#else
const char *piece_str[] = {NULL, "K", "Q", "R", "B", "N", "p", NULL, "K", "Q", "R", "B", "N", "p"};
#endif

const char *piece_to_str(Piece *piece) {
    return piece_str[piece->type + NUM_OF_PIECES * piece->team];
}

int team_to_color(Team t) {
#ifdef USE_UNICODE
    (void) t;
    return 16;
#else
    return t == WHITE ? 26 : 196;
#endif
}

#ifdef DEBUG
char *coords_to_str(int i, int j, char *out) {
    out[1] = '8' - i;
    out[0] = 'a' + j;
    out[2] = '\0';
    return out;
}
#endif

void draw_board(Board board) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%d \x1b[1m", BOARD_SIZE - i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("\x1b[48;5;%um", (i + j) & 1 ? B_BLACK : B_WHITE);
            if (board[i][j]) {
                printf("\x1b[38;5;%um", team_to_color(board[i][j]->team));
                printf(" %s ", piece_to_str(board[i][j]));
            } else {
                printf("   ");
            }
        }
        printf("\x1b[0;0m\n");
    }
    printf("   a  b  c  d  e  f  g  h\n");
}

void str_to_board(uint8_t *col, uint8_t *row) {
    *row = BOARD_SIZE - (*row - '0');
    *col -= 'a';
}

int get_move(char *buf, uint8_t *fr, uint8_t *fc, uint8_t *tr, uint8_t *tc) {
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

bool is_threat(Game *game, Piece *from, Piece *to);

void do_move(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
    if (game->board[tr][tc])
        game->board[tr][tc]->alive = false;
    game->board[tr][tc] = game->board[fr][fc];
    game->board[fr][fc] = NULL;
    game->board[tr][tc]->moves++;
    game->board[tr][tc]->row = tr;
    game->board[tr][tc]->col = tc;

    if (is_threat(game, game->board[tr][tc], &game->teams[!game->turn].king)) {
        printf("Check!\n");
        game->teams[!game->turn].king.checked = true;
    }
}

bool is_move_valid_rook(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    int dr = fr - tr, dc = fc - tc;
    int step_c, step_r, steps;

    if (dc == 0) {
        steps = abs(dr);
        step_c = 0;
        step_r = dr == 0 ? 0 : (dr < 0 ? 1 : -1);
    } else if (dr == 0) {
        steps = abs(dc);
        step_r = 0;
        step_c = dc < 0 ? 1 : -1;
    } else {
        return false;
    }

    for (int i = 1; i < steps; i++) {
        if (game->board[fr + i * step_r][fc + i * step_c]) {
            return false;
        }
    }

    return true;
}

bool is_move_valid_bishop(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    int dr = fr - tr, dc = fc - tc;
    int step_r, step_c;

    if (abs(dc) != abs(dr)) return false;

    step_c = dc < 0 ? 1 : -1;
    step_r = dr < 0 ? 1 : -1;

    for (int i = 1; i < abs(dc); i++) {
        if (game->board[fr + i * step_r][fc + i * step_c]) {
            return false;
        }
    }

    return true;
}

bool is_move_valid_knight(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    (void) game;
    int dr = fr - tr, dc = fc - tc;
    return (abs(dc) == 1 && abs(dr) == 2) || (abs(dc) == 2 && abs(dr) == 1);
}

bool is_move_valid_king_helper(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    (void) game;
    int dr = fr - tr, dc = fc - tc;
    return abs(dc) <= 1 && abs(dr) <= 1;
}


bool is_move_valid_king(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    int dr = fr - tr, dc = fc - tc;

    // castling
    if (game->board[fr][fc]->moves == 0 && dr == 0 && !game->board[fr][fc]->checked) {
        if (dc == 2 && game->board[fr][0]->moves == 0 && !game->board[fr][1] && !game->board[fr][2]
            && !game->board[fr][3]) {
            do_move(game, fr, 0, tr, 3);
            return true;
        }
        if (dc == -2 && game->board[fr][7]->moves == 0 && !game->board[fr][6] && !game->board[fr][5]) {
            do_move(game, fr, 7, tr, 5);
            return true;
        }
    }

    return is_move_valid_king_helper(game, fr, fc, tr, tc);
}

bool is_move_valid_queen(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    return is_move_valid_bishop(game, fr, fc, tr, tc) || is_move_valid_rook(game, fr, fc, tr, tc);
}

bool is_move_valid_pawn_helper(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
#ifdef GOD_MODE
    return true;
#endif
    int dr = fr - tr, dc = fc - tc;
    int dir = game->board[fr][fc]->team == BLACK ? -1 : 1;
    //printf("%d %d %d %u %d %d\n", dr, dc, dir, game->board[fr][fc].moves, game->board[fr + dir][fc].type == NONE, game->board[tr][tc].type == NONE);
    if (dr == dir * 2 && dc == 0 && game->board[fr][fc]->moves == 0
        && !game->board[fr - dir][fc] && !game->board[tr][tc]) {
        game->board[fr][fc]->enpass = true;
        return true;
    }
    if (dr != dir) return false;
    if (abs(dc) == 1 && game->board[tr][tc]) return true;
    if (abs(dc) == 1 && game->board[fr][tc] && game->board[fr][tc]->enpass) { // en passant handling
        game->board[fr][tc]->alive = false;
        game->board[fr][tc] = NULL;
        return true;
    }
    if (abs(dc) == 0 && !game->board[tr][tc]) return true;
    return false;
}

bool is_move_valid_pawn(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
    if (!is_move_valid_pawn_helper(game, fr, fc, tr, tc)) {
        return false;
    }
    //printf("--> %u, %d\n", tr, game->board[fr][fc]->team);
    int team_shift = game->board[fr][fc]->team * 7;
    if (tr == team_shift) {
        printf("%s = 0, %s = 1, %s = 2, %s = 3\nPromotion! please choose piece: ", piece_str[2 + team_shift], piece_str[3 + team_shift],
               piece_str[4 + team_shift], piece_str[5 + team_shift]);
        char buf[BUF_SIZE];
        while (true) {
            if (!fgets(buf, BUF_SIZE - 1, stdin)) {
                fputs("input error\n", stderr);
                game->state = ERROR;
                break;
            }
            fflush(stdin);

            if (buf[0] < '0' || buf[0] > '3') {
                puts("Bad input");
                continue;
            }

            game->board[fr][fc]->type = buf[0] - '0' + 2;
            break;
        }

    }
    return true;
}

bool is_threat(Game *game, Piece *from, Piece *to) {
    uint8_t fr = from->row, fc = from->col, tr = to->row, tc = to->col;
    switch (game->board[fr][fc]->type) {
        case PAWN:
            return is_move_valid_pawn_helper(game, fr, fc, tr, tc);
        case ROOK:
            return is_move_valid_rook(game, fr, fc, tr, tc);
        case KNIGHT:
            return is_move_valid_knight(game, fr, fc, tr, tc);
        case BISHOP:
            return is_move_valid_bishop(game, fr, fc, tr, tc);
        case KING:
            return is_move_valid_king_helper(game, fr, fc, tr, tc);
        case QUEEN:
            return is_move_valid_queen(game, fr, fc, tr, tc);
        default:
            return false;
    }
}

bool is_valid_move_helper(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
    switch (game->board[fr][fc]->type) {
        case PAWN:
            return is_move_valid_pawn(game, fr, fc, tr, tc);
        case ROOK:
            return is_move_valid_rook(game, fr, fc, tr, tc);
        case KNIGHT:
            return is_move_valid_knight(game, fr, fc, tr, tc);
        case BISHOP:
            return is_move_valid_bishop(game, fr, fc, tr, tc);
        case KING:
            return is_move_valid_king(game, fr, fc, tr, tc);
        case QUEEN:
            return is_move_valid_queen(game, fr, fc, tr, tc);
        default:
            return false;
    }
}

bool is_valid_move(Game *game, uint8_t fr, uint8_t fc, uint8_t tr, uint8_t tc) {
    Piece *fp = game->board[fr][fc];
    Piece *tp = game->board[tr][tc];
    if (!fp) {
        puts("No piece found!");
        return false;
    }
    if (fp->team != game->turn) {
        puts("Not your piece!");
        return false;
    }
    if (tp && tp->team == game->turn) {
        puts("Overlap!");
        return false;
    }
    if (!is_valid_move_helper(game, fr, fc, tr, tc)) {
        puts("Invalid move!");
        return false;
    }
    return true;
}

void clear_en_passant(Game *game) {
    for (int i = 0; i < 8; ++i) {
        game->teams[game->turn].pawn[i].enpass = false;
    }
}

void loop(Game *game, char *move) {
    uint8_t fr, fc, tr, tc;
    clear_en_passant(game);

    if (!get_move(move, &fr, &fc, &tr, &tc)) return;
    if (!is_valid_move(game, fr, fc, tr, tc)) return;

    do_move(game, fr, fc, tr, tc);
    game->turn = game->turn == WHITE ? BLACK : WHITE;

    draw_board(game->board);


}

void encode_game(SaveGame *to, const Game *from) {
    to->state = from->state;
    to->teams[0] = from->teams[0];
    to->teams[1] = from->teams[1];
    to->turn = from->turn;
}

void decode_game(Game *to, const SaveGame *from) {
    to->state = from->state;
    to->teams[0] = from->teams[0];
    to->teams[1] = from->teams[1];
    to->turn = from->turn;

    memset(to->board, 0, sizeof(Board));

    for (int i = 0; i < 32; ++i) {
        Piece *p = ((Piece *) to->teams) + i;
        if (p->alive)
            to->board[p->row][p->col] = p;
    }
}

void save_game(Game *game, char *filename) {
    if (!game || !filename) return;
    FILE *save_file;
    SaveGame s_game;
    encode_game(&s_game, game);
    if (!(save_file = fopen(filename, "wb"))) {
        return;
    }
    fwrite(&s_game, sizeof(SaveGame), 1, save_file);
    fclose(save_file);
    printf("%s saved!\n", filename);
}

void load_game(Game *game, char *filename) {
    if (!game || !filename) return;
    FILE *save_file;
    SaveGame s_game;
    if (!(save_file = fopen(filename, "rb"))) {
        return;
    }
    fread(&s_game, sizeof(SaveGame), 1, save_file);
    fclose(save_file);
    decode_game(game, &s_game);
    printf("%s loaded!\n", filename);
}

void play_game(Game *game) {
    if (!game) return;
    draw_board(game->board);

    while (game->state == CONTINUE) {

        char buf[BUF_SIZE + 1] = {0};
        printf("%s's turn: ", game->turn == WHITE ? white_name : black_name);
        if (!fgets(buf, BUF_SIZE, stdin)) {
            fprintf(stderr, "An error occurred while reading input.\n");
            game->state = ERROR;
            return;
        }

        fflush(stdin);

        buf[strlen(buf) - 1] = '\0'; // remove newline

        if (strncmp(buf, "save ", COMMAND_SIZE) == 0 && strlen(buf + COMMAND_SIZE) > 0) {
            save_game(game, buf + COMMAND_SIZE);
        } else if (strncmp(buf, "load ", COMMAND_SIZE) == 0 && strlen(buf + COMMAND_SIZE) > 0) {
            load_game(game, buf + COMMAND_SIZE);
            draw_board(game->board);
        } else {
            loop(game, buf);
        }
    }
}

void new_game(Game *game) {
    if (!game) return;

    decode_game(game, &new_game_template);
}