#ifndef BUTTON_H
#define BUTTON_H
#include "stm32f0xx.h"
#include <stdbool.h>

bool button;

void button_init(void);
void exti_init(void);
void EXTI4_15_IRQHandler(void);

#endif // button_h
