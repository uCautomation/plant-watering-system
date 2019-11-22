#ifndef BUTTONWS_H
#define BUTTONWS_H

#include <Arduino.h>

#include "ws_types.h"

class ButtonWS {
    public:
        ButtonWS(int pin, isr butChISR);

        void changed(void);

        bool isPressed(ulong now);

    private:
        const byte _pin;
        byte _state;
        ulong _lastmilli;
        volatile ulong _milli;
        const ulong _debounceDelay;
};

#endif // BUTTONWS_H
