//
// Created by idan alperin on 07/08/2022.
//

#ifndef CHESS_CHESS_H
#define CHESS_CHESS_H

#include <stdbool.h>
#define BOARD_SIZE 8

typedef enum PieceType {
    KING=1, QUEEN, ROOK, BISHOP, KNIGHT, PAWN} PieceType;
typedef enum Team{WHITE=0, BLACK=1} Team;
typedef enum GameState{CONTINUE, END, ERROR} GameState;

typedef struct Piece {
    PieceType type : 3;
    Team team : 1;
    bool checked : 1;
    bool enpass : 1;
    bool alive : 1;
    unsigned int col : 3;
    unsigned int row : 3;
    unsigned int moves;
} Piece;

typedef struct TeamPieces {
    Piece king, queen, bishop1, bishop2, knight1, knight2, rook1, rook2, pawn[8];
} TeamPieces;

typedef Piece* Board[BOARD_SIZE][BOARD_SIZE];

typedef struct Game {
    TeamPieces teams[2];
    Team turn : 1;
    GameState state : 2;
    Board board;
} Game;

void play_game(Game *game);

void new_game(Game *game);

void load_game(Game * game, char * filename);

#endif //CHESS_CHESS_H
