
#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "clock.h"
#include "global.h"
#include "local_types.h"
#include <stdint.h>

#define MODE_1HZ 0x00
#define MODE_5HZ 0x01
#define MODE_FREQUENCY (MODE_1HZ | MODE_5HZ)

#define MODE_NORMAL 0x00
#define MODE_BLINK 0x02
#define MODE_INVERT 0x04
#define MODE_UNDERLINE 0x08
#define MODE_TYPE (MODE_NORMAL | MODE_BLINK | MODE_INVERT | MODE_UNDERLINE)

#define MODE_NO_PAD 0x00
#define MODE_PAD 0x10
#define MODE_PADDING (MODE_NO_PAD | MODE_PAD)

//#define SET_MARKER MODE_BLINK|MODE_5HZ
#define SET_MARKER MODE_UNDERLINE | MODE_BLINK

#define COLUMN_ON 0
#define COLUMN_OFF 1
#define ALL_OFF 0
#define ROW P0

enum state
{
    ST_BEGIN,
    ST_CLOCK = 0,
    ST_SECOND,
    ST_DATE,
    ST_YEAR,
    ST_DOW,
    ST_ALRM,
#if HAS_THERMISTOR
    ST_TEMP,
#endif
    ST_SETTING,
    ST_END
}; // same order as function pointers in show_callback[]
enum action
{
    ACT_KEYPRESS,
    ACT_TIMER
};

void init_display(void);
void update_display(void);

uint8_t get_width(unsigned char c);
void write_text(unsigned char* text, uint8_t len);
void write_text_mode(unsigned char* text, uint8_t len, uint8_t* text_mode);

void write_hex8(uint8_t* p, uint8_t bcd);
void write_hex16(uint8_t* p, uint16_t hex);

void show();
void change();
void next_state(uint8_t action);

void show_ldr();
void handle_ldr();

void show_temp();
void change_temp();
void show_setting_flags();
void change_setting_flags();

#endif
