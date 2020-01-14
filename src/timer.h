
#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>

void timer0_isr() __interrupt(1);

void initTimer0(void);
void delay3(uint8_t ticks);

#endif
