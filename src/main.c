#include "stm32f0xx.h"
#include "epl_clock.h"
#include "epl_usart.h"
#include "adc.h"
#include "button.h"
#include "game.h"
#include "debug.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


uint8_t data[20];
uint8_t data_idx = 0;
bool newline_rcvd = false;
GameBoard player_board, opponent_board;
char opponent_id[10] = {0};
GameState state = STATE_INIT;
GamePhase phase = PHASE_START;
int iterations = 0;
int schiffe_getroffen = 0;

bool won = false;

int hits[GRID_SIZE][GRID_SIZE] = {0};

//global variable to store the player number
int player = 0;
int x, y;

void set_default_values(void){
    uint8_t data[20];
    uint8_t data_idx = 0;
    bool newline_rcvd = false;
    GameBoard player_board, opponent_board;
    GameState state = STATE_INIT;
    GamePhase phase = PHASE_START;
    int counter = 0;
    int schiffe_getroffen = 0;

    //global variable to store the player number
    int player = 0;
    // 2D-Array zur Verfolgung der bereits beschossenen Felder
    int hits[GRID_SIZE][GRID_SIZE] = {0};
}



// Funktion, um die Spalte mit den meisten Schiffen zu ermitteln
int find_best_column() {
    int max_ships = -1;
    int best_column = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        if (opponent_board.checksum[i] > max_ships) {
            max_ships = opponent_board.checksum[i];
            best_column = i;
        }
    }
    return best_column;
}

// Funktion, um die beste Zeile in der gegebenen Spalte zu finden
int find_best_row(int column) {
    for (int i = 0; i < GRID_SIZE; i++) {
        if (hits[i][column] == 0) { // Wähle die erste nicht beschossene Zeile
            return i;
        }
    }
    // Falls alle Zeilen in der Spalte bereits beschossen wurden, gebe -1 zurück
    return -1;
}

// Funktion, um eine zufällige nicht beschossene Position zu finden
void find_random_position(int *x, int *y) {
    while (1) {
        *x = ADC_read() % GRID_SIZE;
        *y = ADC_read() % GRID_SIZE;
        if (hits[*y][*x] == 0) {
            return;
        }
    }
}

// Smart fire solution
void smart_fire_solution(void) {
    int x, y;
    int best_column = find_best_column();
    int best_row = find_best_row(best_column);
    
    if (best_row != -1) {
        // Wenn eine nicht beschossene Zeile in der besten Spalte gefunden wurde
        x = best_column;
        y = best_row;
    } else {
        // Wenn alle Zeilen in der besten Spalte beschossen wurden, wähle eine zufällige Position
        find_random_position(&x, &y);
    }

    // Schuss ausgeben
    printf("BOOM%d%d\n", x, y);

    // Markiere das Feld als beschossen
    hits[y][x] = 1;
}

void USART2_IRQHandler() {
    if (USART2->ISR & USART_ISR_RXNE) {
        uint8_t c = USART2->RDR;
        if (data_idx >= sizeof(data)) {
            return;
        }
        data[data_idx] = c;
        data_idx++;
        if (c == '\n') {
            newline_rcvd = true;
        }
    }
}

void reset_data() {
    memset(data, 0, sizeof(data));
    data_idx = 0;
    newline_rcvd = false;
}

void process_sf(const char *data) {
    int col_1 = 0;
    int sf_data[GRID_SIZE] = {0};

    // Parse the input data into columns
    while(col_1 < GRID_SIZE) {
        if(newline_rcvd){
            for(int i = 0; i < 10; i++){
                if(data[i+4] != '0'){
                    sf_data[col_1]++;
                }
            }
            col_1++;
            reset_data();
        }
    }

    // Compare the calculated checksum with the opponent's checksum
    int valid = 1;
    for (int col = 0; col < GRID_SIZE; col++) {
        if (sf_data[col] != opponent_board.checksum[col]) {
            valid = 0;
            printf("eMismatch in column %d: expected %d, got %d\n", col, opponent_board.checksum[col], sf_data[col]);
        }
    }

    if (valid) {
        //printf("eSF data matches the opponent's checksum.\n");
    } else {
        printf("eSF data does not match the opponent's checksum.\n");
    }
}

void process_received_data(const char *data) {
    if (strncmp(data, "START", 5) == 0) {
        strncpy(opponent_id, data + 5, 8);
    } else if (strncmp(data, "CS", 2) == 0 && strlen(data) == 13) {
        char temp[11];
        strncpy(temp, data + 2, 10);
        for(int i = 0; i < 10; i++){
            opponent_board.checksum[i] = temp[i] - '0';
        }
    } else if (strncmp(data, "BOOM", 4) == 0 && strlen(data) == 7) {
        x = data[4] - '0';
        y = data[5] - '0';
    } else if ((strncmp(data, "T", 1) == 0 || strncmp(data, "W", 1) == 0) && strlen(data) == 2) {
        if(strncmp(data, "T", 1) == 0){
            schiffe_getroffen++;
            state = STATE_HANDLE_SHOT;
        }
        if(strncmp(data, "W", 1) == 0){
            state = STATE_HANDLE_SHOT;
        }
        iterations++;
        if(iterations > 100){
            printf("#Schüsse>100\n");
            player = 3;
            phase = PHASE_END;
            state = STATE_RESET;
        }
    } else if (strncmp(data, "SF", 2) == 0) {
        process_sf(data);
        player = 3;
        phase = PHASE_END;
        state = STATE_SEND_SF;
    } /*else {
        printf("Ungueltige Daten: %s", data);
    }*/
    //reset_data();
}

void handle_shot(GameBoard *board, int x, int y) {
    if (board->grid[x][y] == SHIP || board->grid[x][y] == HIT) {
        board->grid[x][y] = HIT;
        hit_counter++;
        if(hit_counter == 30){
            //printf("Lost!!!!!!!\n"); //DEBUG MESSAGE
            player = 3;
            state = STATE_SEND_SF;
            phase = PHASE_END;
        }
        else{
            printf("T\n");
            smart_fire_solution();
            state = STATE_CHECK_SHOT;
        }
    } else if (board->grid[x][y] == WATER) {
        board->grid[x][y] = MISS;
        printf("W\n");
        smart_fire_solution();
        state = STATE_CHECK_SHOT;
    }
}

void send_sf(GameBoard *board){
    for (int col = 0; col < GRID_SIZE; col++) {
        printf("SF%dD", col);
        for (int row = 0; row < GRID_SIZE; row++) {
            printf("%d", board->ship_lengths[row][col]);
        }
        printf("\n");
    }
}


void start_loop(void) {
    switch (state) {
        case STATE_SEND_START:
            // Initial state p1, send START message
            printf("START52215874\n");
            if(player == 1){
                state = STATE_WAIT_CHECKSUM;
            } else if(player == 2){
                phase = PHASE_GAME;
                state = STATE_INIT_PLAYER;
            }
            break;
        case STATE_PROCESS_START:
            // Initial state p2, process START message
            if(newline_rcvd){
                process_received_data((char*)data);
                reset_data();
                state = STATE_SEND_CHECKSUM;
            }
            break;
        case STATE_WAIT_START:
            // Wait for START message
            if(newline_rcvd){
                process_received_data((char*)data);
                reset_data();
                phase = PHASE_GAME;
                state = STATE_INIT_PLAYER;
            }
            break;
        case STATE_SEND_CHECKSUM:
            // Send checksum to opponent
            send_checksum(&player_board);
            if(player == 1){
                state = STATE_WAIT_START;
            } else if(player ==2){
                state = STATE_WAIT_CHECKSUM;
            }
            break;
        case STATE_WAIT_CHECKSUM:
            // Wait for checksum from opponent
            if(newline_rcvd){
                process_received_data((char*)data);
                reset_data();
                if(player == 1){
                    state = STATE_SEND_CHECKSUM;
                } else if(player==2){
                    state = STATE_SEND_START;
                }
            }
            
            break;
        default:
            break;
    }
}
void game_loop(void){
    switch (state) {
        case STATE_INIT_PLAYER:
            // Initial state, set player state
            if(player == 1){
                state = STATE_SEND_SHOT;
            } else if(player==2){
                state = STATE_HANDLE_SHOT;
            }
            break;
        case STATE_SEND_SHOT:
            // Send shot message
            smart_fire_solution();
            state = STATE_CHECK_SHOT;
            break;
        case STATE_CHECK_SHOT:
            // Check result of shot
            if(newline_rcvd){
                process_received_data((char*)data);
                reset_data();
            }
            break;
        case STATE_HANDLE_SHOT:
            // Wait for opponent's shot
            if (newline_rcvd) {
                process_received_data((char*)data);
                reset_data();
                handle_shot(&player_board, x, y);
            }
            break;
        default:
            break;
    }
}

void end_loop(void){

    switch(state){
        case STATE_SEND_SF:
            // Send SF message
            //printf("SF\n");
            send_sf(&player_board);
            state = STATE_RESET;
            break;
        case STATE_RESET:
            set_default_values();
            reset_data();
            break;
        default:
            break;
    }

}

int main(void) {
    epl_usart_t myusart;
    myusart.baudrate = 9600;

    EPL_SystemClock_Config();
    EPL_init_usart(&myusart);
    button_init();
    exti_init();

    init_game_board(&player_board);
    place_ships(&player_board);

    debug_init(9600);
    
    debug_printf("Hello World\n");
    
    //print_board(&player_board);
    
    
    while (1) {
///////////////////////////////////////////WAIT FOR START///////////////////////////////////////////

        if (button && player == 0) {
            player = 1;
            state = STATE_SEND_START;
        }
        else if (newline_rcvd && player == 0) {
            player = 2;
            state = STATE_PROCESS_START;
        }

///////////////////////////////////////////GAME PHASE///////////////////////////////////////////
        if(player != 0 && player != 3){
            if(phase == PHASE_START){
                start_loop();
            }
            else if(phase == PHASE_GAME){
                game_loop();
            }
        }
        
///////////////////////////////////////////END PHASE///////////////////////////////////////////
        if(player == 3){
            if(phase == PHASE_END){
                state = STATE_SEND_SF;
                end_loop();
            }
        }
    }

    return 0;
}
