#ifndef WATERSYSTEMSM_H
#define WATERSYSTEMSM_H

#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>
#include "ws_defs.h"

typedef enum {
    wss_sleep = 0,
    wss_start,
    wss_panic, // TBD: is this necessary?

    // states that should be ignoring inputs
    // and timeout immediately after completion
    wss_toggle_use_current,
    wss_manualwater, // indirectly reached via wss_menu_one_pN menu
    wss_reset_current_calibration, // forget calibration data for one module
    wss_autowater, // Used by automatic watering

    // states accepting transitioning-out via button presses
    wss_list_all, // summary of all modules

    // states of the ListAll Menu
    wss_menu_all_x,
    wss_menu_all_p1,
    wss_menu_all_p2,
    wss_menu_all_p3,
    wss_menu_all_p4,
    wss_menu_all_ctrl,

    wss_list_one_p1,
    wss_list_one_p2,
    wss_list_one_p3,
    wss_list_one_p4,

    wss_menu_one_x,
    wss_menu_one_ctrl,
    wss_menu_one_water,

    wss_menu_ctrl_current_x,
    wss_menu_ctrl_current_reset,
    wss_menu_ctrl_current_toggleuse,

    // wss_probe __attribute__((deprecated)), // check the current sensor reading (on demand)

    wss_sys_status, // reservoir water level, dump logs, battery level, error/panic/watchdog reset count
    wss_ctrl_all, // system level control menu (forget calibration, forget+recalibrate, clear system logs, etc.)


    WSS_NOSTATE
} __attribute__ ((__packed__)) wss_type;


const byte nextButPin = 3;
const byte okButPin = 2;

void nextButISR(void);
void okButISR(void);


constexpr ulong timeInMilli(uint16_t seconds)
{
    return 1000UL * seconds;
}

constexpr const ulong SleepTimeOut = 5U;

ulong timedelta(ulong ref_timestamp, ulong now);

class WaterSystemSM {
    public:
        class ButtonWS  *okBut, *nextBut;

        WaterSystemSM(ulong current_milli);
        WaterSystemSM(ulong current_milli, ButtonWS *okBut, ButtonWS *nextBut);

        wss_type State();
        bool stateUpdated(ulong current_milli);

    private:

        friend void dumpWSTables();

        volatile ulong _last_transition_milli;
        wss_type _state = wss_start;

        constexpr static const wss_type _okBut_next_state[WSS_NOSTATE] PROGMEM {
            [wss_sleep] = wss_list_all,
            [wss_start] = wss_sleep,
            [wss_panic] = wss_panic,

            [wss_toggle_use_current] = wss_sleep, // TODO: wss_menu_ctrl_current_x on TO and ignore this?
            [wss_manualwater] = wss_manualwater,
            [wss_reset_current_calibration] = wss_reset_current_calibration, // ignore keypresss, redo has no effect
            [wss_autowater] = wss_sleep, // ignore key press

            [wss_list_all] = wss_menu_all_x,

            [wss_menu_all_x] = wss_list_all,
            [wss_menu_all_p1] = wss_list_one_p1,
            [wss_menu_all_p2] = wss_list_one_p2,
            [wss_menu_all_p3] = wss_list_one_p3,
            [wss_menu_all_p4] = wss_list_one_p4,
            [wss_menu_all_ctrl] = wss_ctrl_all,

            [wss_list_one_p1] = wss_menu_one_x,
            [wss_list_one_p2] = wss_menu_one_x,
            [wss_list_one_p3] = wss_menu_one_x,
            [wss_list_one_p4] = wss_menu_one_x,

            [wss_menu_one_x] = wss_list_all, // TODO: wss_list_one_pN?
            [wss_menu_one_ctrl] = wss_menu_ctrl_current_x,
            [wss_menu_one_water] = wss_manualwater,

            [wss_menu_ctrl_current_x] = wss_menu_one_x,
            [wss_menu_ctrl_current_reset] = wss_reset_current_calibration,
            [wss_menu_ctrl_current_toggleuse] = wss_toggle_use_current,

            // [wss_probe] = wss_list_one,

        };

        constexpr static const wss_type _nextBut_next_state[WSS_NOSTATE] PROGMEM {
            [wss_sleep] = wss_list_all,
            [wss_start] = wss_sleep,
            [wss_panic] = wss_panic,

            [wss_toggle_use_current] = wss_menu_ctrl_current_x, // or sleep?
            [wss_manualwater] = wss_menu_one_x,
            [wss_reset_current_calibration] = wss_reset_current_calibration, // ignore keypresss, redo has no effect
            [wss_autowater] = wss_autowater, // ignore key press

            [wss_list_all] = wss_sys_status,

            [wss_menu_all_x] = wss_menu_all_p1,
            [wss_menu_all_p1] = wss_menu_all_p2,
            [wss_menu_all_p2] = wss_menu_all_p3,
            [wss_menu_all_p3] = wss_menu_all_p4,
            [wss_menu_all_p4] = wss_menu_all_ctrl,
            [wss_menu_all_ctrl] = wss_menu_all_x,

            [wss_list_one_p1] = wss_list_one_p2,
            [wss_list_one_p2] = wss_list_one_p3,
            [wss_list_one_p3] = wss_list_one_p4,
            [wss_list_one_p4] = wss_list_one_p1,

            [wss_menu_one_x] = wss_menu_one_water, // TODO: one_pN?
            [wss_menu_one_ctrl] = wss_menu_one_x,
            [wss_menu_one_water] = wss_menu_one_ctrl,

            [wss_menu_ctrl_current_x] = wss_menu_ctrl_current_toggleuse,
            [wss_menu_ctrl_current_reset] = wss_menu_ctrl_current_x,
            [wss_menu_ctrl_current_toggleuse] = wss_menu_ctrl_current_reset,


            // [wss_probe] = wss_list_one,

            [wss_sys_status] = wss_ctrl_all,
            [wss_ctrl_all] = wss_list_all,
        };

        constexpr static const wss_type _to_next_state[WSS_NOSTATE] PROGMEM {
            [wss_sleep] = wss_autowater,
            [wss_start] = wss_list_all,
            [wss_panic] = wss_panic,

            [wss_toggle_use_current] = wss_menu_ctrl_current_x, // or sleep?
            [wss_manualwater] = wss_menu_one_x, // go back to parent menu
            [wss_reset_current_calibration] = wss_menu_ctrl_current_x, // reprint menu

        };
        ulong _timeout = 1000;

        // TODO: represent on a single byte
        //
        // rangeof valid values 1s - 3 hours (maybe more? 12h?)

        // values in seconds
        constexpr static const uint16_t _state_to[WSS_NOSTATE] PROGMEM {
            [wss_sleep] = 60U,
            [wss_start] = 1U,
            [wss_panic] = 1U,

            [wss_toggle_use_current] = 1U,
            [wss_manualwater] = 1U, // go back to parent menu when done
            [wss_reset_current_calibration] = 1U, // back to sleep when done
            [wss_autowater] = 1U, // back to sleep when done

            [wss_list_all] = 5U,

            // 0 is the same as SleepTimeOut, see _timeoutForState
        };

        wss_type stateAfterOKButton();
        wss_type stateAfterNextButton();
        wss_type stateAfterTimeout();

        static ulong _timeoutForState(wss_type);

        void setPanicState();

        wss_type _current_menu_of_state = WSS_NOSTATE;
};

#endif
