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
    char line2[17] = { 0U };
    for( int i=0; i<MAX_MODULE_COUNT; i++) {

        const int x = i * 3;
        lcd.setCursor(x, 0);
        // TODO: use the WaterSystem::_plant glyph
        sprintf(buf, "P%.1d ", i);
        lcd.print(buf);

        lcd.setCursor(x, 1);
        int delta = sp[i].GetNormalizedDeltaToThreshold();
        sprintf(line2, "%.2d ", delta);
        lcd.print(line2);


        DEBUG("system_list(): %s %s", buf, line2);
    }

    // the menu item
    lcd.setCursor(12, 0);
    sprintf(buf, "== X");
    lcd.print(buf);

    lcd.setCursor(15, 1);
    sprintf(line2, "%c", pWaterSystem->hasInternalError() ? '!' : ' ');
    lcd.print(line2);

}
