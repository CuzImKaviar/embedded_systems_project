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
GamePhase phase = STATE_START;

//global variable to store the player number
int player = 0;

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
        char opponent_id[10];
        strncpy(opponent_id, data + 5, 8);
        opponent_id[9] = '\0';
        reset_data();
    } else if (strncmp(data, "CS", 2) == 0 && strlen(data) == 13) {
        strncpy(opponent_board.checksum, data + 2, 10);
        //printf("Empfangener Checksum: %s", opponent_board.checksum);
        reset_data();
    } else if (strncmp(data, "BOOM", 4) == 0 && strlen(data) == 6) {
        char boom[3];
        strncpy(boom, data + 4, 2);
        boom[2] = '\0';
        reset_data();
    } else if ((data[0] == 'W' || data[0] == 'T' || data[0] == 'V') && strlen(data) == 2) {
        switch(data[0]) {
            case 'W':
                printf("W");
                break;
            case 'T':
                printf("T");
                break;
            case 'V':
                printf("V");
                break;
        }
        reset_data();
    } else if (strncmp(data, "SF", 2) == 0 && data[3] == 'D' && strlen(data) == 15) {
        char opponent_ship_field[12];
        strncpy(opponent_ship_field, data + 4, 10);
        opponent_ship_field[10] = '\0';
        printf("Gültiger Command SF: %s", opponent_ship_field);
        reset_data();
    } else {
        printf("Ungültige Daten: %s", data);
        reset_data();
    }
}

void start_loop(void) {
    switch (state) {
        case STATE_INIT:
            // Initial state, wait for someone to start the game
            break;
        case STATE_PLAYER1:
            // Initial state p1, send START message
            printf("START52215874\n");
            state = STATE_WAIT_CHECKSUM;
            break;
        case STATE_PLAYER2:
            // Initial state p2, process START message
            process_received_data((char*)data);
            state = STATE_SEND_CHECKSUM;
            break;
        case STATE_WAIT_START:
            // Wait for START message
            if(newline_rcvd){
                process_received_data((char*)data);
                phase = STATE_GAME;
            }
            break;
        case STATE_SEND_CHECKSUM:
            // Send checksum to opponent
            send_checksum(&player_board);
            if(player == 1){
                state = STATE_WAIT_START;
            } else {
                state = STATE_WAIT_CHECKSUM;
            }
            break;
        case STATE_WAIT_CHECKSUM:
            // Wait for checksum from opponent
            if (newline_rcvd) {
                process_received_data((char*)data);
                if (player == 1) {
                    state = STATE_SEND_CHECKSUM;
                } else {
                    phase = STATE_GAME;
                }
            }
            break;
        default:
            break;
    }
}
void game_loop(void){
    char msg[20];
    char response[5];
    int x, y;

    switch (state) {
        case STATE_INIT:
            // Initial state, set player state
            if(player == 1){
                state = STATE_SEND_SHOT;
            } else {
                state = STATE_WAIT_SHOT;
            }
            break;
        case STATE_SEND_SHOT:
            // Send shot message
            printf("BOOM%d%d\n", x, y);
            state = STATE_WAIT_RESULT;
            break;
        case STATE_SEND_RESULT:
            // Send shot result
            handle_shot(&player_board, x, y, response);
            EPL_usart_write_n_bytes((uint8_t*)response, 3);
            state = STATE_WAIT_SHOT;
            break;
        case STATE_WAIT_SHOT:
            // Wait for opponent's shot
            if (newline_rcvd && sscanf((char*)data, "BOOM%d%d\n", &x, &y) == 2) {
                handle_shot(&player_board, x, y, response);
                EPL_usart_write_n_bytes((uint8_t*)response, 3);
                //reset_data();
                state = STATE_SEND_RESULT;
            }
            break;
        case STATE_WAIT_RESULT:
            // Wait for shot result
            if (newline_rcvd && sscanf((char*)data, "BOOM%d%d\n", &x, &y) == 2) {
                handle_shot(&player_board, x, y, response);
                EPL_usart_write_n_bytes((uint8_t*)response, 3);
                //reset_data();
                state = STATE_SEND_SHOT;
            }
            break;
        case STATE_END:
            // End state, do nothing
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
    //print_board(&player_board);

    while (1) {
///////////////////////////////////////////START PHASE///////////////////////////////////////////

        if (button && player == 0) {
            player = 1;
            state = STATE_PLAYER1;
        }
        else if (newline_rcvd && player == 0) {
            player = 2;
            state = STATE_PLAYER2;
        }

///////////////////////////////////////////GAME PHASE///////////////////////////////////////////
        if(player != 0){
            if(phase == STATE_START){
                start_loop();
            }
            else if(phase == STATE_GAME){
                game_loop();
            }
        }
    }

    return 0;
}
