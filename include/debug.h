#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include "stm32f0xx.h"

void debug_init(uint32_t baudrate);
void debug_send_char(char c);
void debug_send_string(const char* str);
void debug_printf(const char* fmt, ...);

#endif // DEBUG_H
