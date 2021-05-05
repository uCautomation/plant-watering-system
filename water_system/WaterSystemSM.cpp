#include <Arduino.h>

#include "DebugWS.h"

#include "ButtonWS.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"

constexpr const wss_type WaterSystemSM::_okBut_next_state[WSS_NOSTATE] PROGMEM;
constexpr const wss_type WaterSystemSM::_nextBut_next_state[WSS_NOSTATE] PROGMEM;
constexpr const wss_type WaterSystemSM::_to_next_state[WSS_NOSTATE] PROGMEM;
constexpr const uint16_t WaterSystemSM::_state_to[WSS_NOSTATE] PROGMEM;

ulong WaterSystemSM::_timeoutForState(wss_type state)
{
    if (state >= WSS_NOSTATE) {
        DEBUG_P("_timeoutForState:PANIC: Out of range state");
        system_panic_no_return();
    };

    uint16_t state_to = u16PgmRead(_state_to[state]);
    // 0 is the default initializer, but for TO we want by default SleepTimeOut
    if (state_to == 0U) {
        state_to = SleepTimeOut;
    }

    return timeInMilli(state_to);
}


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
    nextBut(nextBut),
    // _state(wss_start),
    _last_transition_milli(current_milli)
{

    _timeout = _timeoutForState(_state);

}

wss_type WaterSystemSM::stateAfterOKButton()
{
    return u16PgmRead(_okBut_next_state[_state]);
}

wss_type WaterSystemSM::stateAfterNextButton()
{
    return u16PgmRead(_nextBut_next_state[_state]);
}

wss_type WaterSystemSM::stateAfterTimeout()
{
    return u16PgmRead(_to_next_state[_state]);
}

bool WaterSystemSM::stateUpdated(ulong current_milli) {

    if (okBut->isPressed(current_milli)) {

        DEBUG_P("OK pressed");

        noInterrupts();
        _state = stateAfterOKButton();
        _last_transition_milli = current_milli;
        _timeout = _timeoutForState(_state);
        interrupts();

        return true;
    }

    if (nextBut->isPressed(current_milli)) {

        DEBUG_P("Next pressed");

        noInterrupts();
        _state = stateAfterNextButton();
        _last_transition_milli = current_milli;
        _timeout = _timeoutForState(_state);
        interrupts();

        return true;
    }

    ulong time_delta = timedelta(_last_transition_milli, current_milli);
    if (time_delta >= _timeout) {

        DEBUG_P("Time out from state:"); DEBUG("%u", _state);

        noInterrupts();
        wss_type _next_state = stateAfterTimeout();
        bool state_changed = (_next_state != _state);

        if (state_changed) {
            DEBUG_P("state change via timeout: old\tnew\tdelta\t_to");
            DEBUG(":%d\t%d\t%lu\t%lu", _state, _next_state, time_delta, _timeout);
            _state = _next_state;
            _last_transition_milli = current_milli;
            _timeout = _timeoutForState(_state);
        } else {
            DEBUG_P("same state");
        }
        interrupts();

        return state_changed;
    }

    DEBUG_P("No state chg");

    return false;
}

void WaterSystemSM::setPanicState() {
    _state = wss_panic;
}

wss_type WaterSystemSM::State() {
    return _state;
}

//private:
