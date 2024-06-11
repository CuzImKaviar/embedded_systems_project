#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#define GRID_SIZE 10

int hit_counter;

typedef enum {
    WATER,
    HIT,
    MISS,
    SHIP
} CellState;

typedef struct {
    CellState grid[GRID_SIZE][GRID_SIZE];
    int checksum[GRID_SIZE];
    int ship_lengths[GRID_SIZE][GRID_SIZE];
} GameBoard;

void init_game_board(GameBoard *board);
void calculate_checksum(GameBoard *board);
void place_ships(GameBoard *board);
int receive_checksum(const char *msg, uint8_t *checksum);
void send_checksum(GameBoard *board);


void stupid_fire_solution(void);

void print_board(GameBoard *board);

#endif
