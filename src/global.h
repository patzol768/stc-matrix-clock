
#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "stc15.h"

#define TRUE 1 // for general use, bit setters too
#define FALSE 0

#define FOSC 22118400L // clock speed in mhz
#define T0TICKS 20000  // Timer 0 tick rate (t) (1/t = 50us)

#define TICKS_MS 20   // This is set by Timer 1 tick rate
#define MAX_BRIGHT 16 // maximum tick count for brightness
#define MIN_BRIGHT 1  // minimum tick count for brightness

//---------------------------------------------------------------------------
// Begin Hardware Option configuration
//---------------------------------------------------------------------------

#define HAS_LDR TRUE
#define HAS_THERMISTOR TRUE

#define TEMP_0 0x02cf
#define TEMP_50 0x00f6

#define MIN_LDR_LEVEL 0x20
#define MAX_LDR_LEVEL 0xa0

//---------------------------------------------------------------------------
// End Hardware Option configuration
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin Software Option configuration
//---------------------------------------------------------------------------

#define SET_12HR_FORMAT FALSE
#define SET_DEGF_FORMAT FALSE
#define SET_KEY_BEEP FALSE
#define SET_SHOW_ZERO FALSE

//---------------------------------------------------------------------------
// End Software Option configuration
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin board hardware defines
//---------------------------------------------------------------------------

// Pushbutton port pins

#define S2 P3_3 // push button input pin aliases
#define S1 P3_1

// DS1302 pin to port mapping

#define CE_HI P4_0 = 1; // pin 5 <--17 P4.0
#define CE_LO P4_0 = 0;
#define IO P1_5   // pin 6 <--10 P1.5
#define SCLK P5_5 // pin 7 <--15 P5.5

// adc channels for sensors
#if HAS_LDR
#define ADC_LDR 7
#define SET_LDR_PORT        \
    P1M1 |= (1 << ADC_LDR); \
    P1M0 |= (1 << ADC_LDR);
#else
#define SET_LDR_PORT ;
#endif

#if HAS_THERMISTOR
#define ADC_TEMP 6
#define SET_THERMISTOR_PORT  \
    P1M1 |= (1 << ADC_TEMP); \
    P1M0 |= (1 << ADC_TEMP);
#endif

// buzzer port pins and active state set

#define BZR_ON P5_4 = 0  // direct port assigments
#define BZR_OFF P5_4 = 1 // with logic


//---------------------------------------------------------------------------
// End board hardware defines
//---------------------------------------------------------------------------

// Columans left to right:
//  P2_0, P2_1, P2_2, P2_3, P2_4, P2_5, P2_6, P2_7,
//  P3_4, P3_5, P3_6, P3_7, P4_1, P4_2, P4_3, P4_4,
//  P1_0, P1_1, P4_7, P1_2, P1_3, P3_2, P4_5, P4_6

// Rows up to down:
//  P0_0, P0_1, P0_2, P0_3, P0_4, P0_5, P0_6, P0_7

#define COL_00 P2_0
#define COL_01 P2_1
#define COL_02 P2_2
#define COL_03 P2_3
#define COL_04 P2_4
#define COL_05 P2_5
#define COL_06 P2_6
#define COL_07 P2_7
#define COL_08 P3_4
#define COL_09 P3_5
#define COL_10 P3_6
#define COL_11 P3_7
#define COL_12 P4_1
#define COL_13 P4_2
#define COL_14 P4_3
#define COL_15 P4_4
#define COL_16 P1_0
#define COL_17 P1_1
#define COL_18 P4_7
#define COL_19 P1_2
#define COL_20 P1_3
#define COL_21 P3_2
#define COL_22 P4_5
#define COL_23 P4_6

//     0 1 2 3 4 5 6 7
// p0: R R R R R R R R --> R: row values
// P1: C C C C     A A --> C: column selector, A: analog input
// P2: C C C C C C C C
// P3:   S C S C C C C --> S: switch input
// P4: R C C C C C C C --> R: RTC
// P5: - - - - B R - - --> B: beep, -: not available

#endif
