#include <Arduino.h>

#include "DebugWS.h"

#include "ButtonWS.h"
#include "WSMenu.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"

#define NON_INTERACTIVEMENU_STEP         1
#define NON_INTERACTIVEMENU_ENTRIES      1
#define NON_INTERACTIVEMENU_START_COLUMN 0xf
WSMenu non_interactive_menu(
    /* .MenuColumnStep = */ NON_INTERACTIVEMENU_STEP,
    /* .NoOfMenuItems = */ NON_INTERACTIVEMENU_ENTRIES,
    /* .MenuStartsAtColumn = */ NON_INTERACTIVEMENU_START_COLUMN
    );
// on OK next state is always the current state or wss_sleep(?)

#define LIST_ALL_MENU_STEP    3
#define LIST_ALL_MENU_ENTRIES 6
WSMenu list_all_menu(
    /* .MenuColumnStep = */ LIST_ALL_MENU_STEP,
    /* .NoOfMenuItems = */ LIST_ALL_MENU_ENTRIES
    );
// Next states on OK on menu entry
// wss_list_one, wss_list_one, wss_list_one, wss_list_one, wss_sys_status, wss_sleep

#define LIST_ONE_MENU_STEP         2
#define LIST_ONE_MENU_ENTRIES      3
#define LIST_ONE_MENU_START_COLUMN 0xb
#define LIST_ONE_MENU_LINE         1
WSMenu list_one_menu(
    /* .MenuColumnStep = */ LIST_ONE_MENU_STEP,
    /* .NoOfMenuItems = */ LIST_ONE_MENU_ENTRIES,
    /* .MenuStartsAtColumn = */ LIST_ONE_MENU_START_COLUMN,
    /* .MenuLine = */ LIST_ONE_MENU_LINE
    );
// Next states on OK on menu entry
// wss_manualwater, wss_sys_status (reset calibration?), wss_sleep


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

    _pCurrentScreenMenu = &non_interactive_menu;

    _timeout = _state_to[_state];
}

wss_type WaterSystemSM::stateAfterOKButton()
{
    return _okBut_next_state[_state];
}

wss_type WaterSystemSM::stateAfterNextButton()
{
    return _nextBut_next_state[_state];
}

wss_type WaterSystemSM::stateAfterTimeout()
{
    return _to_next_state[_state];
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
            _timeout = _state_to[_state];
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

transition_reason WaterSystemSM::lastTransitionReason()
{
    return _last_reason;
}

//private:
