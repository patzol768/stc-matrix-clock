#include "alarm.h"
#include "ds1302.h"
#include "global.h"
#include "timer.h"

__bit alarm_on;     // 1 = alarm is running
__bit alarm_snooze; // 1 = alarm is snoozed
uint8_t phase;
uint8_t sample_position;

extern __idata struct Clock clockRam; // from ds1302.c
extern uint8_t alarmTimer;            // from timer.c
extern uint8_t snoozeTimer;           // from timer.c

__code const uint8_t alarm_sample[] = {0x01, 0x03, 0x07, 0x0f};

void init_alarm()
{
    alarm_on = 0;
}

void check_alarm()
{
    if (!alarm_on)
    {
        // check if alarm time arrived
        if (
        clockRam.sec == 3 && clockRam.hr == clockRam.almHour && clockRam.min == clockRam.almMin && clockRam.almDays & (1 << (clockRam.day - 1)))
        {
            phase = 0;
            sample_position = 0;
            alarmTimer = 5; // minutes
            snoozeTimer = 0;
            alarm_on = 1;
        }
    }
    else
    {
        if (!snoozeTimer)
        {
            uint8_t sample = (phase < 4) ? phase : 3;

            if (alarm_sample[sample] & (1 << (sample_position & 0x07)))
            {
                BZR_ON;
            }

            delay3(90);
            BZR_OFF;
            delay3(30);

            sample_position++;

            if (sample_position > 0x50) // bits 3..7 show the repeat time inside a phase
            {
                sample_position = 0;
                phase++;
                if (phase > 60)
                    alarm_on = 0;
            }
        }
    }
}

void snooze_alarm()
{
    if (!snoozeTimer)
    {
        snoozeTimer = 5; // 5 mintues
        alarmTimer = 5;  // reset to 5 minutes
    }
}

void stop_alarm()
{
    alarm_on = 0;
}
