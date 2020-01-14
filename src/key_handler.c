#include "key_handler.h"
#include "global.h"
#include "timer.h"

volatile uint8_t stateQueueS1; // holds 5 key states
volatile __bit pressedS1;      // true when pressed, user clears

volatile uint8_t stateQueueS2; // pressed, released, etc.
volatile __bit pressedS2;      // true when pressed, user clears

extern uint8_t keyRepeatTimer; // from timer.c
extern __bit _5hzToggle;       // from timer.c
extern uint8_t beepTimer;      // from timer.c

extern __bit KeyBeep; // from ds1302.c

void debounceSwitches(void)
{
    // Called from Timer ISR code.
    // Update pushbutton state tables every 10ms.
    // Uses negative logic (pressed = 0)
    // State consists of 5 past events and when key is true for 50ms (5*10)
    // the current state will = F0.

    // sw1
    stateQueueS1 = stateQueueS1 << 1 | S1 | K_HELD;
    if (stateQueueS1 == K_PRESSED)
    {
        pressedS1 = TRUE;
        if (KeyBeep)
            beep_on(10);
    }

    // sw2
    stateQueueS2 = stateQueueS2 << 1 | S2 | K_HELD;
    if ((stateQueueS2 == K_HELD) & (!keyRepeatTimer))
    {
        keyRepeatTimer = KEY_REPEAT; // 150ms (1 tick = 10ms)
        pressedS2 = TRUE;
        if (KeyBeep)
            beep_on(10);
    }
}

// Used at power up to detect user reset
// S1 and S2 must be held down to effect reset

__bit checkForReset()
{

    if ((stateQueueS1 == K_HELD) & (stateQueueS2 == K_HELD))
        return TRUE;
    else
        return FALSE;
}

// Used at power up to detect key release after reset

__bit checkForRelease()
{
    pressedS1 = FALSE;
    pressedS2 = FALSE;
    if ((stateQueueS1 == K_RELEASED) & (stateQueueS2 == K_RELEASED))
        return FALSE; // use neg logic to avoid negation of value
    else
        return TRUE; // got it?
}


// return the current queue back to caller
// used for detecting S2 auto-repeat to
// prevent constant flashing of display while changing

__bit getStateS2Flasher()
{
    if (stateQueueS2 == K_HELD)
        return TRUE;
    else
        return _5hzToggle;
}

// Check if S1 pressed. Return TRUE and clear key events if so.
// Return FALSE if no key pressed.

__bit checkAndClearS1()
{
    if (pressedS1)
    {
        pressedS1 = FALSE;
        return TRUE;
    }
    else
        return FALSE;
}

// Check if S2 pressed. Return TRUE and dump key events if so.
// Return FALSE if no key pressed.

__bit checkAndClearS2()
{
    if (pressedS2)
    {
        pressedS2 = FALSE;
        return TRUE;
    }
    else
        return FALSE;
}

// Key beep
//

void beep_on(uint8_t time)
{
    BZR_ON;
    beepTimer = time;
}

void beep_off()
{
    BZR_OFF;
}
