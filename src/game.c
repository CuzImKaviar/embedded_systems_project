#include "game.h"
#include "adc.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

void init_game_board(GameBoard *board) {
    // Initialisieren des Spielfeldes mit Wasser
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            board->grid[i][j] = WATER;
        }
    }
}

bool is_valid_position(GameBoard *board, int x, int y, int length, bool horizontal) {
    for (int i = 0; i < length; i++) {
        int new_x = x + (horizontal ? i : 0);
        int new_y = y + (horizontal ? 0 : i);
        
        if (new_x >= GRID_SIZE || new_y >= GRID_SIZE)
            return false;
        
        if (board->grid[new_x][new_y] != WATER)
            return false;
        
        // Check surrounding cells to ensure no adjacent ships
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int adj_x = new_x + dx;
                int adj_y = new_y + dy;
                if (adj_x >= 0 && adj_x < GRID_SIZE && adj_y >= 0 && adj_y < GRID_SIZE) {
                    if (board->grid[adj_x][adj_y] != WATER)
                        return false;
                }
            }
        }
    }
    return true;
}

void place_ship(GameBoard *board, int length) {
    ADC_init();
    bool placed = false;
    while (!placed) {
        int x = ADC_read() % GRID_SIZE;
        int y = ADC_read() % GRID_SIZE;
        bool horizontal = ADC_read() % 2;

        if (is_valid_position(board, x, y, length, horizontal)) {
            for (int i = 0; i < length; i++) {
                int new_x = x + (horizontal ? i : 0);
                int new_y = y + (horizontal ? 0 : i);
                board->grid[new_x][new_y] = SHIP;
            }
            placed = true;
        }
    }
}

void place_ships(GameBoard *board) {

    // Liste der Schiffslängen
    int ships[] = {5, 4, 4, 3, 3, 3, 2, 2, 2, 2};

    for (int i = 0; i < sizeof(ships)/sizeof(ships[0]); i++) {
        place_ship(board, ships[i]);
    }
}

void calculate_checksum(GameBoard *board) {
    for (int i = 0; i < GRID_SIZE; i++) {
        uint8_t sum = 0;
        for (int j = 0; j < GRID_SIZE; j++) {
            sum += board->grid[i][j];
        }
        board->checksum[i] = sum;
    }
}

int receive_checksum(const char *msg, uint8_t *checksum) {
    for (int i = 0; i < GRID_SIZE; i++) {
        if (sscanf(msg + 2 * i, "%2hhx", &checksum[i]) != 1) {
            return -1; // Fehler beim Parsen
        }
    }
    return 0; // Erfolgreich
}

void send_checksum(GameBoard *board) {
    int ship_count[GRID_SIZE] = {0};
    char checksum[GRID_SIZE] = {0};

    // Zähle die Schiffe in jeder Spalte
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board->grid[j][i] == SHIP) {
                ship_count[i]++;
            }
        }
    }
    for (int i = 0; i < GRID_SIZE; i++) {
        sprintf(checksum + strlen(checksum), "%d", ship_count[i]);
    }
    printf("CS%s\n", checksum);
}
void handle_shot(GameBoard *board, int x, int y, char *response) {
    if (board->grid[x][y] == SHIP) {
        board->grid[x][y] = HIT;
        strcpy(response, "HIT");
    } else if (board->grid[x][y] == WATER) {
        board->grid[x][y] = MISS;
        strcpy(response, "MISS");
    }
}

void print_board(GameBoard *board) {
    printf("Spielfeld:\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            switch (board->grid[i][j]) {
                case WATER:
                    printf("~ ");
                    break;
                case SHIP:
                    printf("S ");
                    break;
                case HIT:
                    printf("X ");
                    break;
                case MISS:
                    printf("O ");
                    break;
                default:
                    break;
            }
        }
        printf("\n");
    }
}