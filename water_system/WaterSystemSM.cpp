#include <Arduino.h>

#include "DebugWS.h"

#include "ButtonWS.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"


WaterSystemSM::WaterSystemSM(ulong current_milli)
    : WaterSystemSM(
        current_milli,
        new ButtonWS(nextButPin, nextButISR),
        new ButtonWS(okButPin, okButISR)
        ) {}

WaterSystemSM::WaterSystemSM(
    ulong current_milli,
    ButtonWS *okBut,
    ButtonWS *nextBut
    )
    :
    okBut(okBut),
    nextBut(nextBut)
{
    _state = wss_start;
    _last_transition_milli = current_milli;
    _last_reason = reason_init;

    _timeout = _state_to[_state];

}

wss_type WaterSystemSM::stateAfterOKButton()
{
    wss_type ret_state = WaterSystemSM::_okBut_next_state[_state];

    return ret_state;
}

wss_type WaterSystemSM::stateAfterNextButton()
{
    return WaterSystemSM::_nextBut_next_state[_state];
}

wss_type WaterSystemSM::stateAfterTimeout()
{
    return WaterSystemSM::_to_next_state[_state];
}

bool WaterSystemSM::stateUpdated(ulong current_milli) {

    if (okBut->isPressed(current_milli)) {

        DEBUG("OK pressed");

        noInterrupts();
        _state = stateAfterOKButton();
        _last_transition_milli = current_milli;
        _last_reason = reason_ok_button;
        _timeout = _state_to[_state];
        interrupts();

        return true;
    }

    if (nextBut->isPressed(current_milli)) {

        DEBUG("Next pressed");

        noInterrupts();
        _state = stateAfterNextButton();
        _last_transition_milli = current_milli;
        _last_reason = reason_next_button;
        _timeout = _state_to[_state];
        interrupts();

        return true;
    }

    ulong time_delta = timedelta(_last_transition_milli, current_milli);
    if (time_delta >= _timeout) {

        DEBUG("Timeout from state %u", _state);

        noInterrupts();
        wss_type _next_state = stateAfterTimeout();
        bool state_changed = (_next_state != _state);

        if (state_changed) {
            DEBUG("state changes via timeout (old = %d, new = %d, delta = %ul, _to = %ul", _state, _next_state, time_delta, _timeout);
            _state = _next_state;
            _last_transition_milli = current_milli;
            _last_reason = reason_timeout;
            _timeout = WaterSystemSM::_state_to[_state];
        } else {
            DEBUG("same state");
        }
        interrupts();

        return state_changed;
    }

    DEBUG("No state change");

    return false;
}

void WaterSystemSM::setPanicState() {
    _state = wss_panic;
}

wss_type WaterSystemSM::State() {
    return _state;
};

transition_reason WaterSystemSM::lastTransitionReason()
{
    return _last_reason;
}

//private:
