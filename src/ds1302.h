
#ifndef _DS1302_H
#define _DS1302_H

#include <stdint.h>

enum Register
{
    kSecondReg = 0,
    kMinuteReg = 1,
    kHourReg = 2,
    kDateReg = 3,
    kMonthReg = 4,
    kDayReg = 5,
    kYearReg = 6,
    kWriteProtectReg = 7,
};

enum Command
{
    kClockBurstWrite = 0xBE,
    kClockBurstRead = 0xBF,
    kRamBurstWrite = 0xFE,
    kRamBurstRead = 0xFF
};

// Time structure as used by DS1302
// These values are BCD!

struct Clock
{
    uint8_t sec;        // seconds. Range: {0...59}
    uint8_t min;        // minutes. Range: {0...59}
    uint8_t hr;         // hours.   Range: {0...23} or {1...12}
    uint8_t date;       // dom.     Range: {1...31}
    uint8_t mon;        // month.   Range: {1...12}
    uint8_t day;        // dow      Range: {1...7}  Sunday = 1;
    uint8_t yr;         // year.    Range: {00...99}
    uint8_t check0;     // 0xAA     check0 xor check1 = 0xFF
    uint8_t check1;     // 0x55
    uint8_t statusBits; // Pack all into one byte (no bool's allowed)
    uint8_t almHour;    // {0...23} or {1...12} in bcd
    uint8_t almMin;     // {0...59} in bcd
    uint8_t almDays;    // Bits representing weekdays, 1 = alarm on day, LSB = SUN
    uint8_t reserved0;  //
    uint8_t reserved1;  //
    uint8_t reserved2;  //
    int8_t tempOffset;  // constrain range to +/- 15
};

#define clockSize 7
#define configSize 10
#define totalSize clockSize + configSize

// bit definitions used in status bits
//
#define kAlarmOn 0x01   // alarm On/Off status
#define kTempOn 0x02    // Temperature Display On/Off status
#define kDateOn 0x04    // Date display On/Off status
#define kDowOn 0x08     // Day of Week On/Off status
#define kShowZero 0x08  // = 1 when adding leading zero in clock's hour
#define kSelect_FC 0x20 // = 1 when degrees F, = 0 if degrees C
#define kSelect_12 0x40 // = 1 when 12 hr mode
#define kKeyBeep 0x80   // beep keys on press

// External module usage:

void getClock();
void putClock();
void refreshTime();
void initRtc();
void initColdStart();
void write_changes();

// Internal module use only

void wait500();
void reset_3w();
void wbyte_3w(uint8_t W_Byte);
uint8_t rbyte_3w();
void getConfigRam();
void putConfigRam();

#endif
