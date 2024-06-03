#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#define GRID_SIZE 10

typedef enum {
    WATER,
    HIT,
    MISS,
    SHIP
} CellState;

typedef struct {
    CellState grid[GRID_SIZE][GRID_SIZE];
    uint8_t checksum[GRID_SIZE];
} GameBoard;

void init_game_board(GameBoard *board);
void calculate_checksum(GameBoard *board);
void place_ships(GameBoard *board);
int receive_checksum(const char *msg, uint8_t *checksum);
void send_checksum(GameBoard *board);
void handle_shot(GameBoard *board, int x, int y, char *response);

void print_board(GameBoard *board);

#endif