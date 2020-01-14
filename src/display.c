#include "display.h"
#include "adc.h"
#include "ds1302.h"
#include "font.h"
#include "global.h"
#include "timer.h"

volatile __bit set_mode;        // = 1 when setting in progress
volatile uint8_t display_state; // shows which item is shown actually
volatile uint8_t set_item;      // shows what data is set currently
volatile uint8_t bright_level;  // result of LDR and config settings

volatile uint8_t __xdata dis0[24] = { // buffer #0 (with initial content)
0x00, 0x7F, 0x08, 0x08, 0x7F, 0x00, 0x38, 0x54, 0x54, 0x48, 0x00, 0x3F,
0x40, 0x00, 0x3F, 0x40, 0x00, 0x38, 0x44, 0x44, 0x38, 0x00, 0x5F, 0x00};
volatile uint8_t __xdata dis1[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // buffer #1
volatile uint8_t __xdata mode[24] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // applied effects (no double buffering)

volatile uint8_t* current_buffer; // points the currently presented buffer
volatile uint8_t* free_buffer;    // points the not presented buffer
volatile uint8_t* show_now;       // points the actual column value
volatile __bit has_new_to_show;   // TRUE if the free buffer has new picture
volatile uint8_t aPos;            // cycles through 0-23 (columns)
volatile uint8_t aTicks;          // on and off cycle counter
volatile __bit refresh_temp;      // helper to slow down temperature reading

extern __bit _1hzToggle; // from timer.c
extern __bit _5hzToggle; // from timer.c

extern __idata struct Clock clockRam;  // from ds1302.c
extern uint8_t __at 0x2F configBitReg; // from ds1302.c
extern __bit AlarmOn;                  // from ds1302.c
extern __bit TempOn;                   // from ds1302.c
extern __bit DateOn;                   // from ds1302.c
extern __bit DowOn;                    // from ds1302.c
extern __bit ShowZero;                 // from ds1302.c
extern __bit Select_FC;                // from ds1302.c
extern __bit Select_12;                // from ds1302.c
extern __bit KeyBeep;                  // from ds1302.c

extern unsigned char __xdata font_4x7[]; // from font.c

extern __bit non_time_changed; // = 1 when something (not clock) changed

__code const uint8_t* __code setting_name[] = {"ALRM", "TEMP", "DATE", "DOW ", "ZERO", "F/C ", "12HR", "BEEP"};

// text for 12/24 hour display setting
__code const uint8_t mode_F_C[] = {0x1f, 'F', '/', 0x1f, 'C'};
// text for settings
__code const uint8_t* setting_text = "FLAGS";


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Arrays of callback function pointers
//

__code const fptr column_on_callback[24] = {
column_on_00, column_on_01, column_on_02, column_on_03,
column_on_04, column_on_05, column_on_06, column_on_07,
column_on_08, column_on_09, column_on_10, column_on_11,
column_on_12, column_on_13, column_on_14, column_on_15,
column_on_16, column_on_17, column_on_18, column_on_19,
column_on_20, column_on_21, column_on_22, column_on_23};

__code const fptr column_off_callback[24] = {
column_off_00, column_off_01, column_off_02, column_off_03,
column_off_04, column_off_05, column_off_06, column_off_07,
column_off_08, column_off_09, column_off_10, column_off_11,
column_off_12, column_off_13, column_off_14, column_off_15,
column_off_16, column_off_17, column_off_18, column_off_19,
column_off_20, column_off_21, column_off_22, column_off_23};

__code const fptr show_callback[] = {
show_clock, show_second, show_date, show_year, show_dow, show_alarm,
#if HAS_THERMISTOR
show_temp,
#endif
show_setting_flags};

__code const fptr change_callback[] = {
change_clock, change_second, change_date, change_year, change_dow, change_alarm,
#if HAS_THERMISTOR
change_temp,
#endif
change_setting_flags};


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Initialize display related variables

void init_display()
{
    display_state = ST_BEGIN;

    current_buffer = dis0;
    free_buffer = dis1;
    has_new_to_show = FALSE;

    bright_level = MAX_BRIGHT >> 1;

    aTicks = 0;
    aPos = 0;
    show_now = current_buffer;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Update the LED display based on global variables.
//
// These are:
//
//      aTicks        = Counts from 0 to MAX_BRIGHT
//                        * column on from 0 to <bright_level>
//                        * colimn off from <bright_level> to MAX_BRIGHT
//      aPos          = The number of the actual column (0 = leftmost column)
//      show_now      = Points to the value of the actual column
//      mode          = An array with the column highlight modes
//
// This routine is called from inside the ISR so be quick!!
//

void update_display(void)
{

    if (aTicks == 0)
    {
        // new cycle begins, turn on column
        //
        uint8_t column_mode = mode[aPos];
        __bit apply_effect = (column_mode & MODE_FREQUENCY) ? _5hzToggle : _1hzToggle;
        uint8_t column_value = *show_now;

        switch (column_mode & MODE_TYPE)
        {
            case MODE_NORMAL:
            default:
                break;

            case MODE_BLINK:
                if (apply_effect)
                    column_value &= 0x00;
                break;

            case MODE_INVERT:
                column_value ^= 0xff;
                break;

            case MODE_UNDERLINE:
                column_value |= 0x80;
                break;

            case MODE_INVERT | MODE_BLINK:
                if (apply_effect)
                    column_value ^= 0xff;
                break;

            case MODE_UNDERLINE | MODE_BLINK:
                if (apply_effect)
                    column_value |= 0x80;
                break;

            case MODE_INVERT | MODE_UNDERLINE:
                column_value |= 0x80;
                column_value ^= 0xff;
                break;

            case MODE_INVERT | MODE_UNDERLINE | MODE_BLINK:
                if (apply_effect)
                {
                    column_value |= 0x80;
                    column_value ^= 0xff;
                }
                break;
        }

        column_on_callback[aPos]();
        ROW = column_value;
    }
    else if (aTicks == bright_level)
    {
        // on time ran out, now switch off
        //
        ROW = ALL_OFF;
        column_off_callback[aPos]();
    }
    else if (aTicks >= MAX_BRIGHT)
    {
        // cycle finished
        //
        aTicks = 0xff;
        ++aPos;
        ++show_now;

        if (aPos >= 24)
        {
            // finished showing one full picture
            //

            if (has_new_to_show)
            {
                // if there is new picture, change buffers
                //
                unsigned char* tmp;
                tmp = current_buffer;
                current_buffer = free_buffer;
                free_buffer = tmp;

                has_new_to_show = FALSE;
            }

            // start over
            //
            aPos = 0;
            show_now = current_buffer;
        }
    }

    ++aTicks;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes text into buffer
//

void write_text(unsigned char* text, uint8_t len)
{
    uint8_t __idata column = 0;  // nth colum data is being handled
    uint8_t __idata counter = 0; // nth character is being processed

    while (column < 24)
    {
        uint8_t width;
        unsigned char c;

        c = (counter < len) ? text[counter++] : 0x20;
        width = get_width(c);

        if (width)
        {
            uint8_t* data = get_data_ptr(c);
            uint8_t char_column = 0;

            while (char_column < width && column < 24)
            {
                mode[column] = 0;
                free_buffer[column++] = data[char_column++];
            }
        }
    }

    has_new_to_show = TRUE;
    while (has_new_to_show)
        ; // wait until buffer change happens
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Writes text into buffer
//

void write_text_mode(unsigned char* text, uint8_t len, uint8_t* text_mode)
{
    uint8_t __idata column = 0;  // nth colum data is being handled
    uint8_t __idata counter = 0; // nth character is being processed

    while (column < 24)
    {
        uint8_t width;
        uint8_t char_mode;
        unsigned char c;

        if (counter < len)
        {
            c = text[counter];
            if (c > 0x7f)
                c = 0x7f; // only the first 128 char codes handled

            char_mode = text_mode[counter];
            counter++;
        }
        else
        {
            c = 0x20;
            char_mode = 0x00;
        }
        width = get_width(c);

        if (width)
        {
            uint8_t* data = get_data_ptr(c);
            uint8_t char_column = 0;

            while (char_column < width && column < 24)
            {
                mode[column] = char_mode;
                free_buffer[column++] = data[char_column++];
            }

            // if there is room for potential padding
            if (char_column == width && column < 24)
            {
                switch (char_mode & MODE_PADDING)
                {
                    case MODE_PAD: // add an empty column aftervards
                        mode[column] = char_mode;
                        free_buffer[column++] = 0x00;
                        break;

                    case MODE_NO_PAD: // no padding
                    default:
                        break;
                }
            }
        }
    }

    has_new_to_show = TRUE;
    while (has_new_to_show)
        ; // wait until buffer change happens
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Creates string from hex or BCD number
//

void write_hex8(uint8_t* p, uint8_t hex)
{
    uint8_t __idata digit;

    digit = ((hex & 0xf0) >> 4);
    digit += (digit > 0x09) ? ('A' - 10) : '0';
    *p = digit;

    p++;

    digit = (hex & 0x0f);
    digit += (digit > 0x09) ? ('A' - 10) : '0';
    *p = digit;
}

void write_hex16(uint8_t* p, uint16_t hex)
{
    uint16_t __idata digit;

    digit = ((hex & 0xf000) >> 12);
    digit += (digit > 0x09) ? ('A' - 10) : '0';
    *p = digit;

    p++;

    digit = ((hex & 0x0f00) >> 8);
    digit += (digit > 0x09) ? ('A' - 10) : '0';
    *p = digit;

    p++;

    digit = ((hex & 0xf0) >> 4);
    digit += (digit > 0x09) ? ('A' - 10) : '0';
    *p = digit;

    p++;

    digit = (hex & 0x0f);
    digit += (digit > 0x09) ? ('A' - 10) : '0';
    *p = digit;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Shows display content according to the actual state
//

void show()
{
    show_callback[display_state]();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Changes (increases or inverts) the value of the actul item
//
void change()
{
    change_callback[display_state]();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Switches display to the next state
//
void next_state(uint8_t action)
{
    switch (action)
    {
        case ACT_KEYPRESS:
            display_state++;
            if (display_state == ST_END)
                display_state = ST_BEGIN;

            // skip of some of the screens turned off
            while (
            (!DateOn && (display_state == ST_DATE || display_state == ST_YEAR))
#if HAS_THERMISTOR
            | (!TempOn && display_state == ST_TEMP)
#endif
            | (!DowOn && display_state == ST_DOW))
            {
                display_state++;
                if (display_state == ST_END)
                    display_state = ST_BEGIN;
            }

            break;

        case ACT_TIMER:
#if HAS_THERMISTOR
            if (display_state == ST_CLOCK && TempOn)
                display_state = ST_TEMP;
            else
#endif
                display_state = ST_BEGIN;

        default:
            break;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Reads LDR and presents as hex value
//

#if HAS_LDR

void show_ldr()
{
    uint8_t __xdata text[2];
    uint8_t ldr = getADCResult8(ADC_LDR);

    write_hex8(text, ldr);
    write_text(text, 2);
}

void handle_ldr()
{
    uint16_t __idata ldr_level = 0xff - getADCResult8(ADC_LDR);
    uint8_t new_bright_level;
    // cut too low and too high values
    //
    if (ldr_level < MIN_LDR_LEVEL)
        ldr_level = MIN_LDR_LEVEL;
    if (ldr_level > MAX_LDR_LEVEL)
        ldr_level = MAX_LDR_LEVEL;
    ldr_level -= MIN_LDR_LEVEL;

    // scale read value into MIN..MAX range
    //
    new_bright_level = ldr_level * (MAX_BRIGHT - MIN_BRIGHT) / (MAX_LDR_LEVEL - MIN_LDR_LEVEL) + MIN_BRIGHT;
    if (aTicks < new_bright_level)
        bright_level = new_bright_level;
}

#endif

#if HAS_THERMISTOR

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Reads temp sensor and presents as hex value
//
// Notes:
//   * read value has 10 bits
//   * the higher the number, the lower the temperature
//   * to convert to decimal we can multiply with 6 bits (max 63)
//   * result shall be in 1/10th celsius degrees
//

void show_temp()
{
    uint8_t __xdata text[6];
    uint8_t __xdata text_mode[6] = {MODE_PAD, MODE_PAD, MODE_PAD, MODE_PAD, MODE_NORMAL, MODE_NORMAL};
    uint16_t ldr = getADCResult(ADC_TEMP);

    if (!set_mode)
    {
        if (refresh_temp == _1hzToggle)
        {
            uint16_t degrees;

            if (ldr > TEMP_0)
                ldr = TEMP_0;
            if (ldr < TEMP_50)
                ldr = TEMP_50;

            // scale max..min range to 0..500 range
            degrees = ((TEMP_0 - ldr) * 50) / ((TEMP_0 - TEMP_50) / 10);
            degrees += clockRam.tempOffset;

            if (!Select_FC)
            {
                text[3] = degrees % 10 + 0x30;
                degrees /= 10;
                text[1] = degrees % 10 + 0x30;
                degrees /= 10;
                text[0] = degrees % 10 + 0x30;

                text[2] = '.';
                text[4] = 0x1f; // used as degree symbol
                text[5] = 'C';
            }
            else
            {
                degrees = degrees * 9 / 5 + 32;
                if (degrees < 1000)
                {
                    text[3] = degrees % 10 + 0x30;
                    text[2] = '.';
                }
                else
                {
                    // 100F or above, no fractions
                    text[3] = ' ';
                    degrees /= 10;
                    text[2] = degrees % 10 + 0x30;
                }

                degrees /= 10;
                text[1] = degrees % 10 + 0x30;
                degrees /= 10;
                text[0] = degrees % 10 + 0x30;

                text[2] = '.';
                text[4] = 0x1f; // used as degree symbol
                text[5] = 'F';
            }

            refresh_temp = !refresh_temp;
        }
    }
    else
    {
        switch (set_item % 3)
        {
            case 0:
            {
                uint8_t i;
                const uint8_t* p = mode_F_C;
                for (i = 0; i < 5; ++i)
                    text[i] = *p++;
                text[5] = ' ';

                if (Select_FC)
                {
                    text_mode[0] |= SET_MARKER; // degree F mode
                    text_mode[1] |= SET_MARKER;
                }
                else
                {
                    text_mode[3] |= SET_MARKER; // degree C mode
                    text_mode[4] |= SET_MARKER;
                }
            }
            break;

            case 1:
            {
                int8_t degrees = clockRam.tempOffset;

                if (degrees < 0)
                {
                    degrees = -degrees;
                    text[0] = '-';
                }
                else if (degrees > 0)
                    text[0] = '+';
                else
                    text[0] = 0x7f;

                text[3] = degrees % 10 + 0x30;
                degrees /= 10;
                text[1] = degrees % 10 + 0x30;

                text[2] = '.';
                text[4] = 0x1f; // used as degree symbol
                text[5] = (Select_FC) ? 'F' : 'C';

                text_mode[0] |= SET_MARKER;
                text_mode[1] |= SET_MARKER;
                text_mode[2] |= SET_MARKER;
                text_mode[3] |= SET_MARKER;
            }
            break;

            case 2:
                // in set mode the raw value is printed (may help in tuning the thermometer)
                write_hex16(text, ldr);
                text[4] = ' ';
                text[5] = ' ';
        }
    }

    write_text_mode(text, 6, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Sets the temp related params
//

void change_temp()
{
    switch (set_item % 3)
    {
        case 0:
        default:
            Select_FC = !Select_FC;
            break;

        case 1:
            clockRam.tempOffset++;
            if (clockRam.tempOffset >= 30)
                clockRam.tempOffset = -30;
            break;

        case 2:
            break;
    }
}

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Shows setup parameters as flags
//

void show_setting_flags()
{
    uint8_t __xdata text[6];
    uint8_t __xdata text_mode[6] = {MODE_NO_PAD, MODE_NO_PAD, MODE_NO_PAD, MODE_PAD, MODE_PAD, MODE_NO_PAD};
    uint8_t i;
    uint8_t mask;
    const uint8_t* p;

    if (!set_mode)
    {
        p = setting_text;
        for (i = 0; i < 5; ++i)
            text[i] = *p++;
        text[5] = ' ';
    }
    else
    {
#if !HAS_THERMISTOR
        // if no thermistor, skip related settings
        if (set_item & 0x07 == 1 || set_item & 0x07 == 5)
            ++set_item;
#endif
        p = setting_name[set_item & 0x07];
        for (i = 0; i < 4; ++i)
            text[i] = *p++;

        text[4] = ':';
        mask = 1 << (set_item & 0x07);
        text[5] = (clockRam.statusBits & mask) ? 0x1e : 0x1d;

        text_mode[5] |= SET_MARKER;
    }

    write_text_mode(text, 6, text_mode);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Sets the setting's flags
//

void change_setting_flags()
{
    uint8_t mask;
    mask = 1 << (set_item & 0x07);

    clockRam.statusBits ^= mask;
    configBitReg = clockRam.statusBits;

    non_time_changed = 1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Turn on or off particular columns
//

void column_on_00() { COL_00 = COLUMN_ON; }
void column_on_01() { COL_01 = COLUMN_ON; }
void column_on_02() { COL_02 = COLUMN_ON; }
void column_on_03() { COL_03 = COLUMN_ON; }
void column_on_04() { COL_04 = COLUMN_ON; }
void column_on_05() { COL_05 = COLUMN_ON; }
void column_on_06() { COL_06 = COLUMN_ON; }
void column_on_07() { COL_07 = COLUMN_ON; }
void column_on_08() { COL_08 = COLUMN_ON; }
void column_on_09() { COL_09 = COLUMN_ON; }
void column_on_10() { COL_10 = COLUMN_ON; }
void column_on_11() { COL_11 = COLUMN_ON; }
void column_on_12() { COL_12 = COLUMN_ON; }
void column_on_13() { COL_13 = COLUMN_ON; }
void column_on_14() { COL_14 = COLUMN_ON; }
void column_on_15() { COL_15 = COLUMN_ON; }
void column_on_16() { COL_16 = COLUMN_ON; }
void column_on_17() { COL_17 = COLUMN_ON; }
void column_on_18() { COL_18 = COLUMN_ON; }
void column_on_19() { COL_19 = COLUMN_ON; }
void column_on_20() { COL_20 = COLUMN_ON; }
void column_on_21() { COL_21 = COLUMN_ON; }
void column_on_22() { COL_22 = COLUMN_ON; }
void column_on_23() { COL_23 = COLUMN_ON; }

void column_off_00() { COL_00 = COLUMN_OFF; }
void column_off_01() { COL_01 = COLUMN_OFF; }
void column_off_02() { COL_02 = COLUMN_OFF; }
void column_off_03() { COL_03 = COLUMN_OFF; }
void column_off_04() { COL_04 = COLUMN_OFF; }
void column_off_05() { COL_05 = COLUMN_OFF; }
void column_off_06() { COL_06 = COLUMN_OFF; }
void column_off_07() { COL_07 = COLUMN_OFF; }
void column_off_08() { COL_08 = COLUMN_OFF; }
void column_off_09() { COL_09 = COLUMN_OFF; }
void column_off_10() { COL_10 = COLUMN_OFF; }
void column_off_11() { COL_11 = COLUMN_OFF; }
void column_off_12() { COL_12 = COLUMN_OFF; }
void column_off_13() { COL_13 = COLUMN_OFF; }
void column_off_14() { COL_14 = COLUMN_OFF; }
void column_off_15() { COL_15 = COLUMN_OFF; }
void column_off_16() { COL_16 = COLUMN_OFF; }
void column_off_17() { COL_17 = COLUMN_OFF; }
void column_off_18() { COL_18 = COLUMN_OFF; }
void column_off_19() { COL_19 = COLUMN_OFF; }
void column_off_20() { COL_20 = COLUMN_OFF; }
void column_off_21() { COL_21 = COLUMN_OFF; }
void column_off_22() { COL_22 = COLUMN_OFF; }
void column_off_23() { COL_23 = COLUMN_OFF; }
