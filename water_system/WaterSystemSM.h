#ifndef WATERSYSTEMSM_H
#define WATERSYSTEMSM_H

#include <Arduino.h>
#include <LiquidCrystal_PCF8574.h>

typedef enum {
  wss_start = 0,
  wss_sleep,
  wss_listing,
  wss_menusel,
  wss_manualwater,
  wss_list,
  wss_probe,
  wss_autowater,
  wss_panic,

  WSS_NOSTATE
} wss_type;

const byte nextButPin = 3;
const byte okButPin = 2;

void nextButISR(void);
void okButISR(void);

void panicLEDToggle(void);

extern LiquidCrystal_PCF8574 lcd;

#define MAX_MODULE_COUNT 4

class WaterSystemSM {
  public:
    class ButtonWS *nextBut, *okBut;

    WaterSystemSM(ulong current_milli);

    wss_type State();
    bool stateUpadated(ulong current_milli);

  private:

    bool okChangedTransition();


    volatile ulong _last_transition_milli;
    wss_type _state;
    wss_type _to_next_state[WSS_NOSTATE] = {
      wss_listing, //  wss_start = 0,
      wss_listing, //  wss_sleep,
      wss_sleep, //  wss_listing,
      wss_sleep, //  wss_menusel,
      wss_manualwater, //  wss_manualwater,
      wss_sleep, //  wss_list,
      wss_sleep, //  wss_probe,
      wss_sleep, //  wss_autowater,
      wss_panic //  wss_panic,
    };
    ulong _timeout = 1000;

    ulong _state_to[WSS_NOSTATE] = {
      2000, //  wss_start = 0,
      30000, //  wss_sleep,
      2000, //  wss_listing,
      5000, //  wss_menusel,
      5000, //  wss_manualwater,
      2000, //  wss_list,
      1000, //  wss_probe,
      1000, //  wss_autowater,
      1000  //  wss_panic,
    };

    bool _lcd = false;

};

#endif
