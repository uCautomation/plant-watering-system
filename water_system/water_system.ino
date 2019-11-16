#include <limits.h>
#include <stdio.h>

#include "ws_defs.h"
#include "ws_types.h"

#include "ButtonWS.h"
#include "WSMenu.h"
#include "WaterSystem.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"


WaterSystem *pWaterSystem;
WaterSystemSM *pWSSM;

#define LIST_ALL_MENU_STEP 3
#define LIST_ALL_MENU_ENTRIES 6
WSMenu list_all_menu(LIST_ALL_MENU_STEP, LIST_ALL_MENU_ENTRIES);
// Next states on OK on menu entry
// wss_list_one, wss_list_one, wss_list_one, wss_list_one, wss_logs, wss_sleep

#define LIST_ONE_MENU_STEP 2
#define LIST_ONE_MENU_ENTRIES 3
#define LIST_ONE_MENU_LINE 1
#define LIST_ONE_MENU_START_COLUMN 0xb
WSMenu list_one_menu(LIST_ONE_MENU_STEP, LIST_ONE_MENU_ENTRIES, LIST_ONE_MENU_LINE, LIST_ONE_MENU_START_COLUMN);
// Next states on OK on menu entry
// wss_manualwater, wss_logs (reset calibration?), wss_sleep




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
            pWaterSystem->system_list();
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
                pWaterSystem->sp[idx].ManualGiveWaterAndAdjustDry();
            }
            delay(MIN_REWATER_INTERVAL_MS);
            break;
            ;;

        case wss_logs:
            {
                transition_reason reason = pWSSM->lastTransitionReason();

                lcd.clear();
                lcd.setBacklight(255);
                lcd.setCursor(0,0);
                char msg[17] = {0};
                byte module_index;
                if (pWaterSystem->hasActiveModule(&module_index)) {
                    snprintf(msg, 16, "Logs: module %d", module_index);
                } else {
                    snprintf(msg, 16, "Logs: [NoSelect]");
                }
                lcd.print(msg);
                lcd.setCursor(1,0);
                lcd.print("TODO:Sys/Mod Log");
            };
            break;
            ;;

        default:
            /* nothing to do */
            ;;
    }
}
