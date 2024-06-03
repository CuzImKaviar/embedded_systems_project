#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f0xx.h"
#include <string.h>

typedef enum {
    STATE_INIT,
    STATE_PLAYER1,
    STATE_PLAYER2,
    STATE_WAIT_START,
    STATE_SEND_CHECKSUM,
    STATE_WAIT_CHECKSUM,
    STATE_SEND_SHOT,
    STATE_WAIT_SHOT,
    STATE_SEND_RESULT,
    STATE_WAIT_RESULT,
    STATE_END
} GameState;

typedef enum {
    STATE_START,
    STATE_GAME,
} GamePhase;

#endif // MAIN_H_