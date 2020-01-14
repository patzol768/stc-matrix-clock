
#include "clock.h"
#include "display.h"
#include "ds1302.h"
#include "global.h"
//#include "timer.h"
#include "utility.h"

extern __idata struct Clock clockRam; // from ds1302.c
extern __bit AlarmOn;                 // from ds1302.c
extern __bit TempOn;                  // from ds1302.c
extern __bit DateOn;                  // from ds1302.c
extern __bit DowOn;                   // from ds1302.c
extern __bit ShowZero;                // from ds1302.c
extern __bit Select_FC;               // from ds1302.c
extern __bit Select_12;               // from ds1302.c
extern __bit KeyBeep;                 // from ds1302.c
extern __bit time_changed;            // from ds1302.c
extern __bit non_time_changed;        // from ds1302.c

extern __bit volatile set_mode;   // from display.c
extern uint8_t volatile set_item; // from display.c

extern __bit alarm_on;     // from alarm.c
extern __bit alarm_snooze; // from alarm.c

// names of days
__code const uint8_t* __code dow_data[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

// names of months
__code const uint8_t* __code month_data[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

// lookup table for day of week calculation
__code const uint8_t months_shift_lookup[] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};

// lookup table for days of months (BCD coded values)
__code const uint8_t days_lookup[] = {0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31};

// text for 12/24 hour display setting
__code const uint8_t* mode_12_24 = "12/24";

// text for alarm on/off setting
__code const uint8_t* alarm_txt = "ALM:";

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes the clock value into the free buffer and flags to show it
//

void show_clock()
{
    uint8_t __xdata text[6];
    uint8_t __xdata text_mode[6] = {MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_BLINK | MODE_1HZ | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_NO_PAD};


    if (Select_12)
    {
        uint8_t hour = bcd_to_dec(clockRam.hr);
        if (AlarmOn)
        {
            text[5] = (hour > 11) ? 0x1b : 0x1a;
        }
        else
        {
            text[5] = (hour > 11) ? 0x1c : 0x20;
        }

        if (hour > 11)
            hour -= 11;
        if (!hour)
            hour += 12;

        write_hex8(text, dec_to_bcd(hour));
    }
    else
    {
        text[5] = (AlarmOn) ? 0x1a : ' ';
        write_hex8(text, clockRam.hr);
    }

    if (!ShowZero && text[0] == 0x30)
        text[0] = 0x7f;
    write_hex8(text + 3, clockRam.min);

    text[2] = ':';

    if (set_mode)
    {
        text_mode[2] &= (MODE_BLINK ^ 0xff); // don't blink colon while setting

        switch (set_item % 3)
        {
            case 0:
                text_mode[0] |= SET_MARKER; // hour setting
                text_mode[1] |= SET_MARKER;
                break;

            case 1:
                text_mode[3] |= SET_MARKER; // minute setting
                text_mode[4] |= SET_MARKER;
                break;

            case 2:
            {
                uint8_t i;
                const uint8_t* p = mode_12_24;
                for (i = 0; i < 5; ++i)
                    text[i] = *p++;

                if (Select_12)
                {
                    text_mode[0] |= SET_MARKER; // 12 hour mode
                    text_mode[1] |= SET_MARKER;
                }
                else
                {
                    text_mode[3] |= SET_MARKER; // 24 hour mode
                    text_mode[4] |= SET_MARKER;
                }
            }
        }
    }
    else
    {
        if (alarm_on)
        {
            uint8_t i;
            for (i = 0; i < 6; ++i)
                text_mode[i] |= (alarm_snooze) ? MODE_INVERT : MODE_INVERT | MODE_BLINK;
        }
    }

    write_text_mode(text, 6, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Sets the clock value, through increasing hour or minute
//

void change_clock()
{
    switch (set_item % 3)
    {
        case 0:
            clockRam.hr++;
            if ((clockRam.hr & 0x0f) == 0x0a)
                clockRam.hr += 6;
            if (clockRam.hr > 0x23)
                clockRam.hr = 0;

            clockRam.sec = 0; // if clock changes, set seconds to zero
            time_changed = 1;
            break;

        case 1:
            clockRam.min++;
            if ((clockRam.min & 0x0f) == 0x0a)
                clockRam.min += 6;
            if (clockRam.min > 0x59)
                clockRam.min = 0;

            clockRam.sec = 0; // if clock changes, set seconds to zero
            time_changed = 1;
            break;

        case 2:
            Select_12 = !Select_12;
            non_time_changed = 1;
            break;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes the seconds value into the free buffer and flags to show it
//

void show_second()
{
    uint8_t __xdata text[5];
    uint8_t __xdata text_mode[5] = {MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_BLINK | MODE_1HZ | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD};

    write_hex8(text + 3, clockRam.sec);

    text[0] = 0x7f; // empty char with 4 columns width
    text[1] = 0x7f;
    text[2] = ':';

    if (set_mode)
    {
        text_mode[2] &= (MODE_BLINK ^ 0xff); // don't blink colon while setting
        text_mode[3] |= SET_MARKER;          // second setting
        text_mode[4] |= SET_MARKER;
    }

    write_text_mode(text, 5, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Clock setting sets seconds to zero, no separate setting provided
//

void change_second()
{
    set_mode = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes the date value into the free buffer and flags to show it
//

void show_date()
{
    uint8_t __xdata text[5];
    uint8_t __xdata text_mode[5] = {MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD};
    uint8_t i;
    const uint8_t* p;
    //  uint8_t mon;

    //  mon = (mon & 0x10) ? mon - 7 : mon - 1; // BCD to real value
    p = month_data[bcd_to_dec(clockRam.mon) - 1];
    for (i = 0; i < 3; ++i)
        text[i] = *p++;

    write_hex8(text + 3, clockRam.date);

    if (set_mode)
    {
        if (set_item & 0x01)
        {
            text_mode[3] |= SET_MARKER; // day of month setting
            text_mode[4] |= SET_MARKER;
        }
        else
        {
            text_mode[0] |= SET_MARKER; // month setting
            text_mode[1] |= SET_MARKER;
            text_mode[2] |= SET_MARKER;
        }
    }

    write_text_mode(text, 5, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Sets the date value, through increasing months or days
//

void change_date()
{
    uint8_t max_days = days_lookup[bcd_to_dec(clockRam.mon) - 1];
    if (clockRam.mon == 0x02 && !(bcd_to_dec(clockRam.yr) % 4)) // % 100 rule not required
    {
        max_days++;
    }

    if (set_item & 0x01)
    {
        clockRam.date++;
        if ((clockRam.date & 0x0f) == 0x0a)
            clockRam.date += 6;
        if (clockRam.date > max_days)
            clockRam.date = 1;
    }
    else
    {
        clockRam.mon++;
        if ((clockRam.mon & 0x0f) == 0x0a)
            clockRam.mon += 6;
        if (clockRam.mon >= 0x13)
            clockRam.mon = 1;
        if (clockRam.date > max_days)
            clockRam.date = max_days;
    }

    calculate_dow();
    non_time_changed = 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes the year value into the free buffer and flags to show it
//

void show_year()
{
    uint8_t __xdata text[5];
    uint8_t __xdata text_mode[5] = {MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD};

    write_hex8(text + 2, clockRam.yr);

    text[0] = '2';
    text[1] = '0';
    text[4] = '.';

    if (set_mode)
    {
        text_mode[0] |= SET_MARKER; // year setting
        text_mode[1] |= SET_MARKER;
        text_mode[2] |= SET_MARKER;
        text_mode[3] |= SET_MARKER;
    }

    write_text_mode(text, 5, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Sets the year value, through increasing years (20)
//
// Note: 2020-2099 only
//

void change_year()
{
    clockRam.yr++;
    if ((clockRam.yr & 0x0f) == 0x0a)
        clockRam.yr += 6;
    if (clockRam.yr > 0x99)
        clockRam.yr = 0x20;
    if (clockRam.yr < 0x20)
        clockRam.yr = 0x20;

    calculate_dow();
    non_time_changed = 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Shows the day fo week (no set oprion...)
//

void show_dow()
{
    uint8_t __xdata text[3];
    uint8_t __xdata text_mode[3] = {MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD, MODE_NORMAL | MODE_PAD};
    uint8_t i;
    const uint8_t* p;

    p = dow_data[clockRam.day - 1];
    for (i = 0; i < 3; ++i)
        text[i] = *p++;

    write_text_mode(text, 3, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Day of week is calculated, so no setting required
//

void change_dow()
{
    set_mode = 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Calculates day of week (constrained algorithm, works 2020 - 2099)
//
//

void calculate_dow()
{
    uint16_t d;
    uint8_t year;

    year = bcd_to_dec(clockRam.yr);

    d = 5;
    d += year;
    d += (year + 3) >> 2;

    d += months_shift_lookup[bcd_to_dec(clockRam.mon) - 1];
    if (clockRam.mon > 0x02 && year % 4 == 0) // % 100 rule not required
    {
        d++;
    }

    d += bcd_to_dec(clockRam.date);

    clockRam.day = d % 7 + 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes the alarm value into the free buffer and flags to show it
//

void show_alarm()
{
    uint8_t __xdata text[6];
    uint8_t __xdata text_mode[6] = {MODE_UNDERLINE | MODE_PAD, MODE_UNDERLINE | MODE_PAD, MODE_UNDERLINE | MODE_PAD, MODE_UNDERLINE | MODE_PAD, MODE_UNDERLINE | MODE_PAD, MODE_UNDERLINE | MODE_NO_PAD};

    if (Select_12)
    {
        uint8_t hour = bcd_to_dec(clockRam.almHour);
        text[5] = (hour > 11) ? 0x1c : 0x20;

        if (hour > 11)
            hour -= 12;
        if (!hour)
            hour += 12;

        write_hex8(text, dec_to_bcd(hour));
    }
    else
    {
        text[5] = ' ';
        write_hex8(text, clockRam.almHour);
    }

    if (!ShowZero && text[0] == 0x30)
        text[0] = 0x7f;
    write_hex8(text + 3, clockRam.almMin);

    text[2] = ':';

    if (set_mode)
    {
        switch (set_item % 10)
        {
            case 0:
                text_mode[0] |= SET_MARKER; // hour setting
                text_mode[1] |= SET_MARKER;
                break;

            case 1:
                text_mode[3] |= SET_MARKER; // minute setting
                text_mode[4] |= SET_MARKER;
                break;

            case 2:
            {
                uint8_t i;
                const uint8_t* p = alarm_txt;
                for (i = 0; i < 4; ++i)
                    text[i] = *p++;
                text[4] = AlarmOn ? 0x1e : 0x1d;
                text[5] = 0xff;

                text_mode[4] |= SET_MARKER;
            }
            break;

            default:
            {
                uint8_t i;
                const uint8_t* p = dow_data[set_item % 10 - 3];
                uint8_t mask = 1 << (set_item % 10 - 3);

                for (i = 0; i < 3; ++i)
                    text[i] = *p++;
                text[3] = ':';
                text[4] = (clockRam.almDays & mask) ? 0x1e : 0x1d;
                text[5] = 0xff;

                text_mode[4] |= SET_MARKER;
            }
            break;
        }
    }

    write_text_mode(text, 6, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Sets the clock value, through increasing hour or minute
//

void change_alarm()
{
    switch (set_item % 10)
    {
        case 0:
            clockRam.almHour++;
            if ((clockRam.almHour & 0x0f) == 0x0a)
                clockRam.almHour += 6;
            if (clockRam.almHour > 0x23)
                clockRam.almHour = 0;

            non_time_changed = 1;
            break;

        case 1:
            clockRam.almMin++;
            if ((clockRam.almMin & 0x0f) == 0x0a)
                clockRam.almMin += 6;
            if (clockRam.almMin > 0x59)
                clockRam.almMin = 0;

            non_time_changed = 1;
            break;

        case 2:
            AlarmOn = !AlarmOn;
            non_time_changed = 1;
            break;

        default:
        {
            uint8_t mask = 1 << (set_item % 10 - 3);
            clockRam.almDays ^= mask;
        }
        break;
    }

    non_time_changed = 1;
}
