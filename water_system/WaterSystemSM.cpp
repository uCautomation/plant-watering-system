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
    NON_INTERACTIVEMENU_STEP,
    NON_INTERACTIVEMENU_ENTRIES,
    NON_INTERACTIVEMENU_START_COLUMN
    );
// on OK next state is always the current state or wss_sleep(?)

#define LIST_ALL_MENU_STEP    3
#define LIST_ALL_MENU_ENTRIES 6
WSMenu list_all_menu(
    LIST_ALL_MENU_STEP,
    LIST_ALL_MENU_ENTRIES
    );
// Next states on OK on menu entry
// wss_list_one, wss_list_one, wss_list_one, wss_list_one, wss_logs, wss_sleep

#define LIST_ONE_MENU_STEP         2
#define LIST_ONE_MENU_ENTRIES      3
#define LIST_ONE_MENU_START_COLUMN 0xb
#define LIST_ONE_MENU_LINE         1
WSMenu list_one_menu(
    LIST_ONE_MENU_STEP,
    LIST_ONE_MENU_ENTRIES,
    LIST_ONE_MENU_START_COLUMN,
    LIST_ONE_MENU_LINE
    );
// Next states on OK on menu entry
// wss_manualwater, wss_logs (reset calibration?), wss_sleep


WaterSystemSM::WaterSystemSM(ulong current_milli) {
    _state = wss_start;
    _last_transition_milli = current_milli;
    _last_reason = reason_init;

    nextBut = new ButtonWS(nextButPin, nextButISR);
    okBut = new ButtonWS(okButPin, okButISR);
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
        interrupts();

        return true;
    }

    if (nextBut->isPressed(current_milli)) {

        DEBUG("Next pressed");

        noInterrupts();
        _state = stateAfterNextButton();
        _last_transition_milli = current_milli;
        _last_reason = reason_next_button;
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
            DEBUG("state changes via timeout");
            _state = _next_state;
            _last_transition_milli = current_milli;
            _last_reason = reason_timeout;
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
