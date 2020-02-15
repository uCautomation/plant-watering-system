#ifndef WSRUNTIME_H
#define WSRUNTIME_H

#include "ws_types.h"


void panicLEDToggle();
void system_panic_no_return();
void system_panic_wo_lcd_no_return();
ulong timedelta(ulong ref_timestamp, ulong now);

#ifdef __AVR
#define u16PgmRead(var) pgm_read_word(&(var))
#else
#define u16PgmRead(var) (var)
#endif

inline byte systemAnalogReadBits()
{
    // TODO: Should we set the read resolution via analogReadResolution() to 8 bits? for EEPROM efficiency?
    return 10;
}

#define ZERO_INIT_ARRAY(ARR) memset(&(ARR), 0, sizeof(ARR))

void assert_or_panic(bool condition);
#endif // WSRUNTIME_H
