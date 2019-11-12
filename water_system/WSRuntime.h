#ifndef WSRUNTIME_H
#define WSRUNTIME_H


void panicLEDToggle();
void system_panic_no_return();
ulong timedelta(ulong ref_timestamp, ulong now);

#endif // WSRUNTIME_H
