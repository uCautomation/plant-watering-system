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
void dumpWSTables(WaterSystem *pWS)
{
//     Serial.print("Sizeof(wss_type) = ");
//     Serial.println(sizeof(wss_type));

//     byte *pStateByte = (byte *)&(WaterSystemSM::_okBut_next_state);
//     for (uintptr_t i = 0; i < sizeof(WaterSystemSM::_okBut_next_state); i++)
//     {
//         if (i % 16 == 0) {
//             Serial.println(' ');
//             Serial.print(i, HEX);
//             Serial.print(": ");
//         }
//         printHex(*(pStateByte + i));
//     }
}
#pragma GCC diagnostic pop

void setup() {
    Serial.begin(9600);
    panicLEDToggle();
    delay(500);


    ulong last = millis();

    pWaterSystem = new WaterSystem();
    dumpWSTables(pWaterSystem);

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
    DEBUG("St=%d", nextstate);
    // lcd.setCursor(13, 1);
    // lcd.print(nextstate);
    switch (nextstate) {
        case wss_panic:
            system_panic_no_return(); // never returns
            break;
            ;;

        case wss_list_all:
            pWaterSystem->listAll();
            break;
            ;;

        case wss_menu_all_x:
            pWaterSystem->openMenu(&list_all_menu);
            break;;

        case wss_menu_all_p1:
        case wss_menu_all_p2:
        case wss_menu_all_p3:
        case wss_menu_all_p4:
        case wss_menu_all_ctrl:
            pWaterSystem->selectNextMenuEntry();
            break;;

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
            ;;

        case wss_sleep:
            lcd.setBacklight(0);
            lcd.noDisplay();
            break;
            ;;

        case wss_manualwater:
            pWaterSystem->manualWaterCurrent();
            break;
            ;;

        case wss_sys_status:
            {

                lcd.clear();
                lcd.setBacklight(255);
                lcd.home();
                char msg[lcdLineBufLen] = {0};
                saneModuleIndex_t saneIndex;
                if (pWaterSystem->hasActiveModule(&saneIndex)) {
                    snprintf(msg, lcdLineBufLen, "Logs: module %d", saneIndex.moduleIndex);
                } else {
                    memcpy_P(msg, F("Logs: [NoSelect]"), sizeof(msg));
                }
                lcd.print(msg);
                lcd.setCursor(0, 1);
                lcd.print(F("TODO:Sys/Mod Log"));
            };
            break;
            ;;

        default:
            /* nothing to do */
            ;;
    }
}
