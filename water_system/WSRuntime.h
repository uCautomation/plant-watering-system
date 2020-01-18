#ifndef WSRUNTIME_H
#define WSRUNTIME_H

#include "ws_types.h"


void panicLEDToggle();
void system_panic_no_return();
ulong timedelta(ulong ref_timestamp, ulong now);

inline byte systemAnalogReadBits()
{
    // TODO: Should we set the read resolution via analogReadResolution() to 8 bits? for EEPROM efficiency?
    return 10;
}
#endif // WSRUNTIME_H
