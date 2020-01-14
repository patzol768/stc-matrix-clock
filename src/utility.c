
#include "utility.h"
#include <stdint.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Returns the decoded decimal value from a binary-coded decimal (BCD) byte.
// Assumes 'bcd' is coded with 4-bits per digit, with the tens place digit in
// the upper 4 MSBs.
//

uint8_t bcd_to_dec(uint8_t bcd)
{
    return (10 * ((bcd & 0xF0) >> 4) + (bcd & 0x0F));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Returns the binary-coded decimal of 'dec'. Inverse of bcd_to_dec.
//

uint8_t dec_to_bcd(uint8_t dec)
{
    uint8_t tens = dec / 10;
    uint8_t ones = dec % 10;
    return (tens << 4) | ones;
}
