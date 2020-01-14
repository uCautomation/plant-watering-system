#include <limits.h>
#include <stdio.h>

#include "ws_defs.h"
#include "ws_types.h"

#include "ButtonWS.h"
#include "WaterSystem.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"
#include "WSMenu.h"



WaterSystem *pWaterSystem;
WaterSystemSM *pWSSM;



//   0123456789abcdef
//  +----------------+
// 0|P1 P2 P3 P4 == X|
// 1|+2 -3 +5 -9    S|
//  +----------------+
#define LIST_ALL_MENU_STEP    3
#define LIST_ALL_MENU_ENTRIES 6
WSMenu list_all_menu(
    /* .MenuColumnStep = */ LIST_ALL_MENU_STEP,
    /* .NoOfMenuItems = */ LIST_ALL_MENU_ENTRIES
    );


//   0123456789abcdef
//  +----------------+
// 0|P1 Now:52 Ref:50|
// 1|WET(d:+2)  ☔ > X|
//  +----------------+
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

//   0123456789abcdef
//  +----------------+
// 0|P1 Refs 47 53 51|
// 1|>  NoUse Reset X|
//  +----------------+
WSMenu ctrl_one_menu(
    /* .MenuColumnStep = */ 6,
    /* .NoOfMenuItems = */ 3,
    /* .MenuStartsAtColumn = */ 3,
    /* .MenuLine = */ 1
    );

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
    pWSSM = new WaterSystemSM(
                    last,
                    new ButtonWS(okButPin, okButISR),
                    new ButtonWS(nextButPin, nextButISR)
                );

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

        case wss_sys_status:
            {
                /*transition_reason reason = */(void)pWSSM->lastTransitionReason();

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
