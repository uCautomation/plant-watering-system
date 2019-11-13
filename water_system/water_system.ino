#include <limits.h>
#include <stdio.h>

#include "ws_defs.h"
#include "ws_types.h"

#include "ButtonWS.h"
#include "SensorAndPump.h"
#include "WaterSystem.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"


WaterSystem *pWaterSystem;
WaterSystemSM *pWSSM;


void nextButISR(void)
{
    pWSSM->nextBut->changed();
}

void okButISR(void)
{
    pWSSM->okBut->changed();
}

void setup() {
    Serial.begin(9600);
    panicLEDToggle();
    delay(500);

    ulong last = millis();

    pWaterSystem = new WaterSystem();
    pWSSM = new WaterSystemSM(last);

    if (pWSSM->State() == wss_panic) {
        system_panic_no_return();
    }

}

void loop() {
  ulong now = millis();

  if (pWSSM->stateUpdated(now)) {
    set_system_state(pWSSM->State());
  }
}

void set_system_state(wss_type nextstate)
{
    switch (nextstate) {
        case wss_panic:
            system_panic_no_return(); // never returns
            break;
            ;;

        case wss_list_all:
            system_list();
            break;
            ;;

        case wss_sleep:
            lcd.setBacklight(0);
            lcd.noDisplay();
            break;
            ;;

        case wss_manualwater:
            byte idx;
            if (pWaterSystem->hasActiveModule(&idx)) {
                sp[idx].ManualGiveWaterAndAdjustDry();
            }
            delay(MIN_REWATER_INTERVAL_MS);
            break;
            ;;

        default:
            /* nothing to do */
            ;;
    }
}

void system_list()
{
    lcd.display();
    lcd.setBacklight(255);lcd.home(); lcd.clear();

    char buf[51] = ".    .    |    .    |    .    |    .    ";
    for( int i=0; i<MAX_MODULE_COUNT; i++) {
        int moist = sp[i].GetCurrentMoisture();
        sprintf(buf, "S%.1d=%d  ", i, moist);
        lcd.setCursor((i&0x1) * 8, i>>1);
        lcd.print(buf);
        DEBUG("system_list(): %s", buf);
    }
}
