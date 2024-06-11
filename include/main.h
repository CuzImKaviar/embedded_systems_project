#ifndef MAIN_H_
#define MAIN_H_

#include "stm32f0xx.h"
#include <string.h>

typedef enum {
    STATE_INIT,
    STATE_SEND_START,
    STATE_PROCESS_START,
    STATE_WAIT_START,
    STATE_SEND_CHECKSUM,
    STATE_WAIT_CHECKSUM,
    STATE_SEND_SHOT,
    STATE_CHECK_SHOT,
    STATE_HANDLE_SHOT,
    STATE_INIT_PLAYER,
    STATE_CHECK_RESULT,
    STATE_SEND_SF,
    STATE_RESET,
} GameState;

typedef enum {
    PHASE_START,
    PHASE_GAME,
    PHASE_END,
} GamePhase;

#endif // MAIN_H_