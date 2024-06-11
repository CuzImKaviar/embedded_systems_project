#include "debug.h"

void debug_init(void) {

     // Enable peripheral GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Enable peripheral USART1 clock
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Configure PA2 as USART1_TX using alternate function 1
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1; // Alternate function mode
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3); // Clear alternate function settings
    GPIOA->AFR[0] |= (0b0001 << (4 * 2)) | (0b0001 << (4 * 3)); // Set AF1 for PA2 (USART1 TX) and PA3 (USART1 RX)

    // Configure the UART Baud rate Register
    USART1->BRR = (48000000 / 9600);

    // Enable the UART using the CR1 register
    USART1->CR1 |= (USART_CR1_RE | USART_CR1_TE | USART_CR1_UE);

}

void debug_send_char(char c) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = c;
}

void debug_send_string(char *s) {
    while (*s) {
        debug_send_char(*s++);
    }
}
