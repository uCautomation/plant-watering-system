#include <Arduino.h>

#include "DebugWS.h"

#include "ButtonWS.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"


WaterSystemSM::WaterSystemSM(ulong current_milli) {
    _state = wss_start;
    _last_transition_milli = current_milli;

    nextBut = new ButtonWS(nextButPin, nextButISR);
    okBut = new ButtonWS(okButPin, okButISR);

    _timeout = _state_to[_state];
}

bool WaterSystemSM::stateUpdated(ulong current_milli) {

    if (okBut->isPressed(current_milli)) {

        DEBUG("OK pressed");

        noInterrupts();
        _state = _okBut_next_state[_state];
        _last_transition_milli = current_milli;
        interrupts();

        return true;
    }

    if (nextBut->isPressed(current_milli)) {

        DEBUG("Next pressed");

        noInterrupts();
        _state = _nextBut_next_state[_state];
        _last_transition_milli = current_milli;
        interrupts();

        return true;
    }

    ulong time_delta = timedelta(_last_transition_milli, current_milli);
    if (time_delta >= _timeout) {

        DEBUG("Timeout from state %u", _state);

        noInterrupts();
        wss_type _next_state = _to_next_state[_state];
        bool state_changed = (_next_state != _state);

        if (state_changed) {
            DEBUG("state changes via timeout");
            _state = _next_state;
            _last_transition_milli = current_milli;
        } else {
            DEBUG("same state");
        }
        interrupts();

        return state_changed;
    }

    DEBUG("No state change");

    return false;
}

wss_type WaterSystemSM::State() {
    return _state;
};

//private:

