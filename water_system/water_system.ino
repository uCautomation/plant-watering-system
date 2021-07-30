#include <limits.h>
#include <stdio.h>

#include <LowPower.h>

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

    // TODO for lower power consumption:
    //  - Set all unconnected pins to input pull up - see 14.2.6 Unconnected Pins in ATmega48A/PA/88A/PA/168A/PA/328/P megaAVR Data sheet
    //  - set power reduction bits (PRR - power reduction register)
    //      - ADC bit - PRADC - ADC should be disabled before ADC shutdown
    //      - PRSPI
    //      - PRTIM0/1/2 ? - are any of these used for the internal clocks or millis()?
    //      Note: can't stop TWI/I2C as it will lose LCD glyphs, USART0, as we would lose serial output debugging clues
    //  - disable brown-out-detector (BOD) - BODS bit in MCUCR - see programming sequence in 10.11.2 MCUCR – MCU Control Register

    Serial.begin(9600);
    panicLEDToggle();
    delay(500);


    ulong last = allMillis();

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

    // turn off the LED
    panicLEDOff();
}

#if defined(__AVR_ATmega2560__)
    #define HAS_TIMER5
    #define HAS_TIMERs43
    #define HAS_TIMER2

    #define HAS_USARTs32
    #define HAS_USART1
    #define HAS_USART0

#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega88__)
    #define HAS_TIMER2

    #define HAS_USART0

#elif defined(__AVR_ATmega32U4__)
    #define HAS_TIMERs43

    #define HAS_USART1
    #define UART1_IS_SERIAL

    #define HAS_USB

#else
    #warning "low power not defined for this HW"
    #define NO_LOW_POWER

#endif

volatile short sleep_period = (short)SLEEP_8S;
static_assert(sizeof(sleep_period) == sizeof(period_t), "nope, not big enough");

void goLowPower() {
    // DEBUG_P("LP\n");

    #if defined(NO_LOW_POWER)
        DEBUG_P("Low power not defined for this HW");
        panicLEDToggle();

    #else

        // DEBUG("S%u", sleep_period);
        panicLEDToggle();
        delay(10); // TODO: LED settle timeout
        panicLEDToggle();
        // delay(10); // TODO: LED settle timeout

        LowPower.idle((period_t)sleep_period, ADC_OFF,
            #if defined(HAS_TIMER5)
                    TIMER5_OFF,
            #endif
            #if defined(HAS_TIMERs43)
                    TIMER4_OFF, TIMER3_OFF,
            #endif
            #if defined(HAS_TIMER2)
                    TIMER2_OFF,
            #endif
                    TIMER1_OFF, TIMER0_OFF,
                    SPI_OFF,
            #if defined(HAS_USARTs32)
                    USART3_OFF, USART2_OFF,
            #endif
            #if defined(HAS_USART1)
                #if defined(UART1_IS_SERIAL)
                    USART1_ON,
                #else
                    USART1_OFF,
                #endif
            #endif
            #if defined(HAS_USART0)
                    USART0_ON,
            #endif
                    TWI_ON
            #if defined(HAS_USB)
                    , USB_OFF
            #endif
        );

        // assume sleep has occurred and was completed, even if
        // incorrect, it makes sense to have a roughly correct
        // delta added to the millis;
        // even if we would add these only when a complete sleep
        // cycle is completed, the WDT timer is inaccurate and the
        // error would still exist but it will undersestimate
        // instead of overestimate the real millis
        //
        // Overestimating is simpler, has fewer moving parts
        // and we don't have to modify the LowPower library to have
        // our slightly modified ISR handler.
        //
        // Consider the more intrusive approach if button debouncing
        // is broken by this and using millis() in the debounce still
        // doesn't fix it
        addSleepMillis(sleep_period);

        noInterrupts();
        // slowly increasing sleep duration, unless a button ISR resets it
        sleep_period = sleep_period == SLEEP_8S ? SLEEP_8S : sleep_period + 1;
        interrupts();

    #endif
}


void loop() {
    ulong now = allMillis();

    if (pWSSM->stateUpdated(now))
    {
        set_system_state(pWSSM->State());
    }

    if (pWSSM->State() == wss_sleep)
    {
        goLowPower();
    }
}

void set_system_state(wss_type nextstate)
{
    // DEBUG_P("Set system state:"); DEBUG("%d", nextstate);
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
            static_assert((wss_list_one_p2 - wss_list_one_p1) == 1,
                          "Inconsistent one_p1 and one_p2 values");
            static_assert((wss_list_one_p2 - wss_list_one_p1)
                          == (wss_list_one_p3 - wss_list_one_p2),
                          "Inconsistent one_p1, one_p2, one_p3 values");
            static_assert((wss_list_one_p2 - wss_list_one_p1)
                          == (wss_list_one_p4 - wss_list_one_p3),
                          "Inconsistent one_p1, one_p2, one_p3, one_p4 values");
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

        case wss_reset_current_calibration:
            pWaterSystem->resetCalibrationForCurrentModule();
            break;

        default:
            pWaterSystem->showState(nextstate);
    }
}
