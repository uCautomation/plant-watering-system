#ifndef WIRE_H
#define WIRE_H

#include <stdint.h>

class wire {
  public:
    void begin(void) {};
    void beginTransmission(uint32_t) {};
    int  endTransmission() {return 0;};
};

extern wire Wire;

#endif