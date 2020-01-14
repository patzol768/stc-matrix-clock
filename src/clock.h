
#ifndef _CLOCK_H
#define _CLOCK_H

#include <stdint.h>

void show_clock();
void show_second();
void show_date();
void show_year();
void show_dow();
void show_alarm();

void change_clock();
void change_second();
void change_date();
void change_year();
void change_dow();
void change_alarm();

// for internal use
void calculate_dow();
uint8_t bcd_to_dec(uint8_t bcd);
uint8_t dec_to_bcd(uint8_t dec);

#endif
