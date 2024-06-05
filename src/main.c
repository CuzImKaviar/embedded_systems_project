#include "stm32f0xx.h"
#include "epl_clock.h"
#include "epl_usart.h"
#include "adc.h"
#include "button.h"
#include "game.h"
#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define DEBUG

uint8_t data[20];
uint8_t data_idx = 0;
bool newline_rcvd = false;
GameBoard player_board, opponent_board;
GameState state = STATE_INIT;
GamePhase phase = PHASE_START;
int iterations = 0;
int schiffe_getroffen = 0;

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

void process_received_data(const char *data) {
    if (strncmp(data, "START", 5) == 0) {
        char opponent_id[10] = {0};
        strncpy(opponent_id, data + 5, 8);
    } else if (strncmp(data, "CS", 2) == 0 && strlen(data) == 13) {
        strncpy(opponent_board.checksum, data + 2, 10);
    } else if (strncmp(data, "BOOM", 4) == 0 && strlen(data) == 6) {
        x = data[4];
        y = data[5];
    } else if ((strncmp(data, "T", 1) == 0 || strncmp(data, "W", 1) == 0) && strlen(data) == 2) {
        if(data[0] == "T"){
            schiffe_getroffen++;
        }
        iterations++;
        if(iterations >= 100){
            printf("Protokollfehler, über 100 Schüsse abgegeben\n");
            phase = PHASE_END;
            state = STATE_RESET;
        }
    } else if (strncmp(data, "SF", 2) == 0 && data[3] == 'D' && strlen(data) == 15) {
        char opponent_ship_field[12];
        strncpy(opponent_ship_field, data + 4, 10);
        opponent_ship_field[10] = '\0';
        printf("Gueltiger Command SF: %s", opponent_ship_field);
    } /*else {
        printf("Ungueltige Daten: %s", data);
    }*/
    reset_data();
}

void handle_shot(GameBoard *board, int x, int y) {
    if (board->grid[x][y] == SHIP || board->grid[x][y] == HIT) {
        board->grid[x][y] = HIT;
        hit_counter++;
        if(hit_counter == 30){
            //printf("Lost!!!!!!!\n"); //DEBUG MESSAGE
            state = STATE_SF;
            phase = PHASE_END;
        }
        else{
            printf("T\n");
        }
    } else if (board->grid[x][y] == WATER) {
        board->grid[x][y] = MISS;
        printf("W\n");
    }
}

void send_sf(GameBoard *board){
    for (int col = 0; col < GRID_SIZE; col++) {
        printf("SF[%d]D", col);
        for (int row = 0; row < GRID_SIZE; row++) {
            printf("[%d]", board->ship_lengths[row][col]);
        }
        printf("\n");
    }
}

void process_sf(const char *data){
    
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
                state = STATE_SEND_CHECKSUM;
            }
            break;
        case STATE_WAIT_START:
            // Wait for START message
            if(newline_rcvd){
                process_received_data((char*)data);
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
            stupid_fire_solution();
            state = STATE_CHECK_SHOT;
            break;
        case STATE_CHECK_SHOT:
            // Check result of shot
            if(newline_rcvd){
                process_received_data((char*)data);
                state = STATE_HANDLE_SHOT;
            }
            break;
        case STATE_HANDLE_SHOT:
            // Wait for opponent's shot
            if (newline_rcvd) {
                process_received_data((char*)data);
                handle_shot(&player_board, x, y);
                state = STATE_SEND_SHOT;
            }
            break;
        default:
            break;
    }
}

void end_loop(void){
    switch(state){
        case STATE_SF:
            // Send SF message
            send_sf(&player_board);
            if(newline_rcvd){
                process_sf((char*)data);
                state = STATE_RESET;
            }
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
    print_board(&player_board);
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
        if(player != 0){
            if(phase == PHASE_START){
                start_loop();
            }
            else if(phase == PHASE_GAME){
                game_loop();
            }
            else if(phase == PHASE_END){
                end_loop();
            }
        }
    }

    return 0;
}
