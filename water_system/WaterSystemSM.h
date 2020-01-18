#ifndef WATERSYSTEMSM_H
#define WATERSYSTEMSM_H

#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>
#include "ws_defs.h"

typedef enum {
    wss_sleep = 0,
    wss_start,
    wss_panic, // TBD: is this necessary?

    wss_list_all, // summary of all modules

    // states of the ListAll Menu
    wss_menu_all_x,
    wss_menu_all_p1,
    wss_menu_all_p2,
    wss_menu_all_p3,
    wss_menu_all_p4,
    wss_menu_all_ctrl,

    wss_list_one, // indirectly reached via wss_list_all menu
    wss_manualwater, // indirectly reached via wss_list_one menu
    wss_probe, // check the current sensor reading (on demand)
    wss_autowater, // Used by automatic watering

    wss_sys_status, // reservoir water level, dump logs, battery level, error/panic/watchdog reset count
    wss_ctrl_all, // system level control menu (forget calibration, forget+recalibrate, clear system logs, etc.)

    WSS_NOSTATE
} wss_type;

const byte nextButPin = 3;
const byte okButPin = 2;

void nextButISR(void);
void okButISR(void);

typedef enum {
    reason_init,
    reason_timeout,
    reason_ok_button,
    reason_next_button,
} transition_reason;


const ulong SleepTimeOut = 5000UL;

ulong timedelta(ulong ref_timestamp, ulong now);

class WaterSystemSM {
    public:
        class ButtonWS  *okBut, *nextBut;

        WaterSystemSM(ulong current_milli);
        WaterSystemSM(ulong current_milli, ButtonWS *okBut, ButtonWS *nextBut);

        wss_type State();
        transition_reason lastTransitionReason();
        bool stateUpdated(ulong current_milli);

    private:

        friend void dumpWSTables();

        volatile ulong _last_transition_milli;
        wss_type _state;
        transition_reason _last_reason = reason_init;

        constexpr static wss_type _okBut_next_state[WSS_NOSTATE] {
            [wss_sleep] = wss_list_all,
            [wss_start] = wss_sleep,
            [wss_panic] = wss_panic,
            [wss_list_all] = wss_menu_all_x,

            [wss_menu_all_x] = wss_list_all,
            [wss_menu_all_p1] = wss_list_one,
            [wss_menu_all_p2] = wss_list_one,
            [wss_menu_all_p3] = wss_list_one,
            [wss_menu_all_p4] = wss_list_one,
            [wss_menu_all_ctrl] = wss_ctrl_all,

            [wss_list_one] = wss_probe,
            [wss_manualwater] = wss_manualwater,
            [wss_probe] = wss_list_one,
        };

        constexpr static wss_type _nextBut_next_state[WSS_NOSTATE] {
            [wss_sleep] = wss_list_all,
            [wss_start] = wss_sleep,
            [wss_panic] = wss_panic,

            [wss_list_all] = wss_sys_status,

            [wss_menu_all_x] = wss_menu_all_p1,
            [wss_menu_all_p1] = wss_menu_all_p2,
            [wss_menu_all_p2] = wss_menu_all_p3,
            [wss_menu_all_p3] = wss_menu_all_p4,
            [wss_menu_all_p4] = wss_menu_all_ctrl,
            [wss_menu_all_ctrl] = wss_menu_all_x,

            [wss_list_one] = wss_probe,
            [wss_manualwater] = wss_manualwater,
            [wss_probe] = wss_list_one,
            [wss_autowater] = wss_autowater, // ignore key press

            [wss_sys_status] = wss_ctrl_all,
            [wss_ctrl_all] = wss_list_all,
        };

        constexpr static wss_type _to_next_state[WSS_NOSTATE] {
            [wss_sleep] = wss_sleep,
            [wss_start] = wss_list_all,
            [wss_panic] = wss_panic,
        };
        ulong _timeout = 1000;

        constexpr static ulong _state_to[WSS_NOSTATE] {
            [wss_sleep] = 30000UL,
            [wss_start] = 10000UL,
            [wss_panic] = 1000UL,

            [wss_list_all] = 5000UL,

            [wss_menu_all_x] = SleepTimeOut,
            [wss_menu_all_p1] = SleepTimeOut,
            [wss_menu_all_p2] = SleepTimeOut,
            [wss_menu_all_p3] = SleepTimeOut,
            [wss_menu_all_p4] = SleepTimeOut,
            [wss_menu_all_ctrl] =SleepTimeOut,

            [wss_list_one] = 2000UL,
            [wss_manualwater] = SleepTimeOut,
            [wss_probe] = 1000UL,
            [wss_autowater] = 1000UL,

            [wss_sys_status] = SleepTimeOut,
            [wss_ctrl_all] = SleepTimeOut,
        };

        wss_type stateAfterOKButton();
        wss_type stateAfterNextButton();
        wss_type stateAfterTimeout();

        void setPanicState();

        wss_type _current_menu_of_state = WSS_NOSTATE;
};

#endif
