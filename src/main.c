#include "adc.h"
#include "alarm.h"
#include "clock.h"
#include "display.h"
#include "ds1302.h"
#include "global.h"
#include "key_handler.h"
#include "stc15.h"
#include "timer.h"

extern __bit _1hzToggle;     // from timer.c
extern __bit _5hzToggle;     // from timer.c
extern uint8_t userTimer100; // from timer.c

extern uint8_t stateQueueS1; // from key_handler.c
extern __bit pressedS1;      // from key_handler.c
extern uint8_t stateQueueS2; // from key_handler.c
extern __bit pressedS2;      // from key_handler.c

extern __bit has_new_to_show;   // from display.c
extern __bit set_mode;          // from display.c
extern uint8_t set_item;        // from display.c
extern uint8_t* current_buffer; // from display.c
extern uint8_t* free_buffer;    // from display.c
extern uint8_t display_state;   // from display.c

extern __bit alarm_on; // from alarm.c

void shift_buffer()
{
    uint8_t i;
    uint8_t* s = current_buffer;
    uint8_t* d = free_buffer;

    s++;

    for (i = 0; i < 23; ++i)
    {
        *d++ = *s++;
    }
    *d = 0;
}

void clear_initial_screen()
{
    uint8_t j;
    delay3(300);

    for (j = 0; j < 24; ++j)
    {
        shift_buffer();
        has_new_to_show = TRUE;
        while (has_new_to_show)
            ;

        delay3(10);
    }
}

int main()
{
    init_display(); // initialize display
    init_alarm();
    initRtc(); // setup DS1302 and read config ram
#if HAS_LDR
    InitADC(ADC_LDR); // init light sensor AD input
#endif
#if HAS_THERMISTOR
    InitADC(ADC_TEMP); // init temp sensor AD input
#endif
    initTimer0(); // start timers and display scan

    delay3(33); // wait 100ms - else the reset doesn't work
    if (checkForReset())
    {
        initColdStart(); // reset clock when both switches down @ powerup
        while (checkForRelease())
            ;
    }

    clear_initial_screen();
    userTimer100 = 200;

    while (1)
    {
#if HAS_LDR
        handle_ldr();
#endif

        if (!set_mode)
        {
            getClock();
            check_alarm();

            if (alarm_on)
            {
                // when alarm, show the clock always;
                display_state == ST_CLOCK;
                userTimer100 = 200;
            }

            if (!userTimer100)
            {
                next_state(ACT_TIMER);
                userTimer100 = 200;
            }

            if (checkAndClearS2())
            {
                if (!alarm_on)
                {
                    next_state(ACT_KEYPRESS);
                    userTimer100 = 200;
                }
                else
                {
                    stop_alarm();
                }
            }

            if (checkAndClearS1())
            {
                if (!alarm_on)
                {
                    userTimer100 = 2;
                    while (userTimer100)
                    {
                    };

                    if (stateQueueS1 == K_HELD)
                    {
                        while (stateQueueS1 == K_HELD)
                        {
                        }; // wait for release

                        set_mode = 1;
                        set_item = 0;
                        userTimer100 = 200;
                    }
                    else
                    {
                        set_item++;
                        stateQueueS1 = 0xff; // clear key state queue to avoid false held identification
                        userTimer100 = 200;
                        show();
                    }
                }
                else
                {
                    snooze_alarm();
                }
            }

            show();
        }
        else
        {
            if (!userTimer100)
            {
                write_changes();
                set_mode = 0;
                userTimer100 = 200;
            }

            if (checkAndClearS2())
            {
                change();
                userTimer100 = 200;
                show();
            }

            if (checkAndClearS1())
            {
                userTimer100 = 3;
                while (userTimer100)
                {
                };

                if (stateQueueS1 == K_HELD)
                {
                    write_changes();

                    while (stateQueueS1 == K_HELD)
                    {
                    }; // wait for release

                    set_mode = 0;
                    userTimer100 = 200;
                }
                else
                {
                    set_item++;
                    stateQueueS1 = 0xff; // clear key state queue to avoid false held identification
                    userTimer100 = 200;
                    show();
                }
            }
        }

        delay3(10); // a small delay to slow down things
    }
}
