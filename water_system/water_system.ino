#include <limits.h>
#include <stdio.h>

#include "ws_defs.h"
#include "ws_types.h"

#include "ButtonWS.h"
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
    panicLEDToggle();
    pWSSM->okBut->changed();
}

void printHex(byte b)
{
    Serial.print(b, HEX);
    Serial.print(' ');
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
void dumpWSTables(WaterSystemSM *pWSSM)
{
    // DEBUG_P("Sizeof(wss_type) = ");
    // DEBUG("     >>>> %d", sizeof(wss_type));

    // DEBUG_P("Sizeof(*pWSSM) =");
    // DEBUG("     >>>> %d", sizeof(*pWSSM));

    // byte *pStateByte = (byte *)&(WaterSystemSM::_okBut_next_state);
    // for (uintptr_t i = 0; i < sizeof(WaterSystemSM::_okBut_next_state); i++)
    // {
    //     if (i % 16 == 0) {
    //         Serial.println(' ');
    //         Serial.print(i, HEX);
    //         Serial.print(": ");
    //     }
    //     printHex(*(pStateByte + i));
    // }
}
#pragma GCC diagnostic pop

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
    dumpWSTables(pWSSM);

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
    DEBUG_P("Set system state:"); DEBUG("%d", nextstate);
    // lcd.setCursor(13, 1);
    // lcd.print(nextstate);
    switch (nextstate) {
        case wss_panic:
            system_panic_no_return(); // never returns
            break;

        case wss_list_all:
            pWaterSystem->listAll();
            break;

        case wss_menu_all_x:
            pWaterSystem->openMenu(&list_all_menu);
            break;

        case wss_menu_all_p1:
        case wss_menu_all_p2:
        case wss_menu_all_p3:
        case wss_menu_all_p4:
        case wss_menu_all_ctrl:
            pWaterSystem->selectNextMenuEntry();
            break;

        case wss_list_one_p1:
        case wss_list_one_p2:
        case wss_list_one_p3:
        case wss_list_one_p4:
            static_assert((wss_list_one_p2 - wss_list_one_p1) == 1);
            static_assert((wss_list_one_p2 - wss_list_one_p1)
                          == (wss_list_one_p3 - wss_list_one_p2));
            static_assert((wss_list_one_p2 - wss_list_one_p1)
                          == (wss_list_one_p4 - wss_list_one_p3));
            if (!pWaterSystem->statusOne((byte)(nextstate - wss_list_one_p1))) {
                DEBUG_P("Unexpected failure of statusOne");
                pWaterSystem->setSystemInternalError();
            };
            break;

        case wss_menu_one_x:
            pWaterSystem->showStatusCurrentOne();
            pWaterSystem->openMenu(&list_one_menu);
            break;

        case wss_menu_one_ctrl:
        case wss_menu_one_water:
            pWaterSystem->selectNextMenuEntry();
            break;

        case wss_menu_ctrl_current_x:
            pWaterSystem->showCtrlCurrentOne();
            pWaterSystem->openMenu(&ctrl_one_menu);
            break;

        case wss_menu_ctrl_current_reset:
        case wss_menu_ctrl_current_toggleuse:
            pWaterSystem->selectNextMenuEntry();
            break;

        case wss_toggle_use_current:
            pWaterSystem->toggleUsageForCurrent();
            break;

        case wss_sleep:
            lcd.setBacklight(0);
            lcd.noDisplay();
            break;

        case wss_manualwater:
            pWaterSystem->manualWaterCurrent();
            break;

        case wss_sys_status:
            pWaterSystem->showSysStatus();
            break;

        case wss_autowater:
            pWaterSystem->autoWater();
            break;

        default:
            pWaterSystem->showState(nextstate);
    }
}
