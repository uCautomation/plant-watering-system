#ifndef WATERSYSTEMSM_H
#define WATERSYSTEMSM_H

#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>

typedef enum {
    wss_start = 0,
    wss_sleep,
    wss_list_all,
    wss_menusel,
    wss_manualwater,
    wss_list_one,
    wss_probe,
    wss_autowater,
    wss_panic,
    wss_logs,

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


#define MAX_MODULE_COUNT 4

ulong timedelta(ulong ref_timestamp, ulong now);

class WaterSystemSM {
    public:
        class ButtonWS *nextBut, *okBut;

        WaterSystemSM(ulong current_milli);

        wss_type State();
        transition_reason lastTransitionReason();
        bool stateUpdated(ulong current_milli);

    private:

        volatile ulong _last_transition_milli;
        wss_type _state;
        transition_reason _last_reason = reason_init;

        wss_type _okBut_next_state[WSS_NOSTATE] = {
            wss_sleep, //  wss_start = 0,
            wss_list_all, //  wss_sleep,
            wss_sleep, //  wss_list_all,
            wss_menusel, //  wss_menusel,
            wss_manualwater, //  wss_manualwater,
            wss_probe, //  wss_list_one,
            wss_list_one, //  wss_probe,
            wss_sleep, //  wss_autowater,
            wss_panic, //  wss_panic,
            wss_list_all, // wss_logs
        };

        wss_type _nextBut_next_state[WSS_NOSTATE] = {
            wss_sleep, //  wss_start = 0,
            wss_list_all, //  wss_sleep,
            wss_sleep, //  wss_list_all,
            wss_list_one, //  wss_menusel,
            wss_manualwater, //  wss_manualwater,
            wss_probe, //  wss_list_one,
            wss_list_one, //  wss_probe,
            wss_sleep, //  wss_autowater,
            wss_panic, //  wss_panic,
            wss_sleep, // wss_logs
        };

        wss_type _to_next_state[WSS_NOSTATE] = {
            wss_list_all, //  wss_start = 0,
            wss_sleep, //  wss_sleep,
            wss_sleep, //  wss_list_all,
            wss_sleep, //  wss_menusel,
            wss_manualwater, //  wss_manualwater,
            wss_sleep, //  wss_list_one,
            wss_sleep, //  wss_probe,
            wss_sleep, //  wss_autowater,
            wss_panic, //  wss_panic,
            wss_sleep, // wss_logs
        };
        ulong _timeout = 1000;

        ulong _state_to[WSS_NOSTATE] = {
            5000, //  wss_start = 0,
            30000, // wss_sleep,
            2000, //  wss_list_all,
            5000, //  wss_menusel,
            5000, //  wss_manualwater,
            2000, //  wss_list_one,
            1000, //  wss_probe,
            1000, //  wss_autowater,
            1000, //  wss_panic,
            5000, //  wss_logs
        };

        wss_type stateAfterOKButton();
        wss_type stateAfterNextButton();
        wss_type stateAfterTimeout();

};

#endif
