
#include "timer.h"
#include "display.h"
#include "global.h"
#include "key_handler.h"
#include <stdint.h>

volatile uint8_t _3msTimer;    // triggers events in every 3ms
volatile uint8_t _10msTimer;   // triggers events in every 10ms
volatile uint8_t _100msTimer;  // triggers events in every 100ms
volatile uint8_t _500msTimer;  // triggers events in every 500ms
volatile uint8_t _1minTimer;   // triggers events in every 1min
volatile uint8_t userTimer100; // user delay in 100ms ticks (25.5 seconds max)
volatile uint8_t userTimer3;   // user delay in 3ms ticks 765ms max (3*255)
volatile uint8_t beepTimer;    // time period until key beeps
volatile uint8_t alarmTimer;   // time period until alarm is running (minutes)
volatile uint8_t snoozeTimer;  // snooze period (minutes)

volatile uint8_t keyRepeatTimer; // sets repeat time (in 10ms ticks)

volatile __bit _1hzToggle; // 500 ms pulse width (blinks : in display)
volatile __bit _5hzToggle; // 100 ms used to blink digits being set

void initTimer0(void) // 50us @ 22.1184mhz
{
    AUXR |= 0x80; // set to clock div 1 without trashing other bits
    TH0 = (65536 - (FOSC / T0TICKS)) >> 8;
    TL0 = (65536 - (FOSC / T0TICKS)) & 0xFF;
    TF0 = 0; // Clear TF0 flag
    TR0 = 1; // Timer0 start run
    ET0 = 1; // enable timer0 interrupt
    EA = 1;  // global interrupt enable
}

void timer0_isr() __interrupt(1)
{
    // Here on every timer roll-over (50us).

    // Tell display code a timer tick has occurred.
    // Response is:
    // Just increment counter and return if during anode ON time
    // If time on period has expired, turn off anode driver and contiune counting
    // unitl at end of cycle (MaxOnTime)

    update_display();

    // handle various time based tasks
    // User key press handler, call every 3ms

    if (!_3msTimer--)
    {
        _3msTimer = 3 * TICKS_MS;
        if (userTimer3)
            userTimer3--;
    }
    if (!_10msTimer--)
    {
        _10msTimer = 10 * TICKS_MS;
        debounceSwitches();
        if (keyRepeatTimer)
            keyRepeatTimer--;
        if (!_100msTimer--)
        {
            _100msTimer = 10;
            _5hzToggle = !_5hzToggle;
            if (userTimer100)
                userTimer100--;
        }
        if (!_500msTimer--)
        {
            _500msTimer = 50;
            _1hzToggle = !_1hzToggle;

            if (!_1minTimer--)
            {
                _1minTimer = 120;
                if (snoozeTimer)
                {
                    snoozeTimer--;
                }
                else
                {
                    if (alarmTimer)
                        alarmTimer--;
                }
            }
        }
        if (beepTimer)
        {
            beepTimer--;
            if (!beepTimer)
                beep_off();
        }
    }
#if HAS_NY3P_SPEECH
    if (soundTimer)
        soundTimer--;
#endif
}

// User delay routine
// Call with number of 10ms ticks to wait
// Max is 3ms * 255 = 0.765 seconds delay

void delay3(uint8_t ticks)
{
    userTimer3 = ticks;
    while (userTimer3)
        ;
}
