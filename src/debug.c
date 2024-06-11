#include "debug.h"

void debug_init(uint32_t baudrate) {
    // Enable clocks for USART1 and GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Configure PA9 (TX) and PA10 (RX) in alternate function mode
    GPIOA->MODER |= GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1;
    GPIOA->AFR[1] |= (0x1 << 4) | (0x1 << 8);

    // Configure USART1
    USART1->BRR = SystemCoreClock / baudrate;
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART1->CR1 |= USART_CR1_UE;
}

void debug_send_char(char c) {
    while (!(USART1->ISR & USART_ISR_TXE));
    USART1->TDR = c;
}

void debug_send_string(const char* str) {
    while (*str) {
        debug_send_char(*str++);
    }
}

void debug_printf(const char* fmt, ...) {
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    debug_send_string(buffer);
}
