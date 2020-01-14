
#ifndef _KEY_HANDLER_H
#define _KEY_HANDLER_H

#include <stdint.h>

#define K_HELD 0xE0
#define K_PRESSED 0xF0
#define K_RELEASED 0xFF

#define KEY_REPEAT 18;

void debounceSwitches();

void stateSwitchWithS1(uint8_t);
void stateSwitchExtendedWithS1(uint8_t, uint8_t, __bit);
void stateSwitchWithS2(uint8_t);
void stateSwitchExtendedWithS2(uint8_t, uint8_t, __bit);

__bit checkAndClearS1();
__bit checkAndClearS2();
__bit checkAndClearS3();

__bit checkForReset();
__bit checkForRelease();
__bit getStateS2Flasher();

uint8_t getStateS1();
uint8_t getStateS2();

void beep_on(uint8_t);
void beep_off();

#endif
