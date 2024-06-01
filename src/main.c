/*
----------------------------------------------------------------------------
 * Notes:
 * This project is currently under construction and has not yet been fully tested.
 * Use with caution.
 * ----------------------------------------------------------------------------
 */

#include <stm32f0xx.h>
#include "epl_clock.h"
#include "epl_usart.h"
#include <stdbool.h>

#define DEBUG

uint8_t data[20];
uint8_t data_idx = 0;
bool newline_rcvd = false;

/**
 * @brief Delays the program execution for a specified amount of time.
 * @param time The amount of time to delay in number of cycles.
 * @return 0 when the delay is completed.
 */
int delay(uint32_t time){
    for(uint32_t i = 0; i < time; i++ ){
        asm("nop"); // No operation, used for delaying
    }
    return 0;
}


void USART2_IRQHandler(){
    if ( USART2->ISR & USART_ISR_RXNE ) {
        uint8_t c = USART2->RDR;

        if (data_idx >= sizeof(data)) {
            return;
        }

        data[data_idx] = c;
        data_idx ++;
        if (c == '\n') {
            newline_rcvd = true;
        }
    }
}

void reset_data(){
    memset(data, 0, sizeof(data));
    data_idx = 0;
    newline_rcvd = false;
}

int main(void){


    // configure usart
    epl_usart_t myusart;
    myusart.baudrate = 9600;

    // Configure the system clock to 48MHz
    EPL_SystemClock_Config();
    EPL_init_usart(&myusart);

    bool running = true;
    printf("Hello, World \n");
    while (running){
        if (newline_rcvd)
        {
        uint8_t msg[] = "Hello, World \r\n";
        EPL_usart_write_n_bytes(msg, sizeof(data));
        reset_data();
        }
        
    }
}