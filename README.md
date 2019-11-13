# plant-watering-system

[![Build Status](https://travis-ci.com/uCautomation/plant-watering-system.svg?branch=master)](https://travis-ci.com/uCautomation/plant-watering-system)

An automatic plant watering system with 4 watering modules (sensor + valve/pump).

The user can interact with the system via the 16x2 LCD display and the user buttons.

## TODO

- Save power by going to sleep on timeout
- Test the WaterSystem state machine
- Allow the user to calibrate each module
  - Allow the user to save the calibration data into the EEPROM
  - Save the calibration data into a sort-of flash/EEPROM filesystem to prevent
    early death of specific EEPROM memory cells
    - we probably want to split the EEPROM memory in 2 regions: 1 for
      calibration info, one for runtime error logging
  - is this still necessary if calibration happens automatically when plants
    are manually watered?
- Proportional watering - the system will keep the valve/pump open for a time
  proportional to the difference from the reference value (to allow checking
  up with the reference value)

## watering modules

Each watering modules needs 3 connections to the Arduino:

- Vsens - power supply for the moisture sensor; connected to 1 digital output
  to prolong the life of the sensor by powering only when needed
- Dpump - control pin for the valve; connected to 1 digital output pin of the
  Arduino
- Asens - moisture sensor read value; connected to 1 analog input pin

Power:
Connections to 12V and GND are also needed to control the valves, if used. The
valve operates at 12V, so each module also needs 12V supply to work.
If 5V pumps are used instead, the uC will control the relay that closes the pump
5V supply circuit.

## Interface

TODO: Next and OK buttons allow selecting the menu options:

- Global status - Summary of all plant's status
  - probably it makes sense to re-read actual values only if more than 5-10
    minutes passed since the last reading
- Status - display last read sensors' values, dry and wet thresholds and other
  status info
- Calibrate sensor - Allow selection and dry/wet threshold module calibration
  - this might be unnecessary, with auto-learning on manual watering
- Water now(?) - select a module and water it under manual command - maybe should use to auto-calibrate dry, wet(?) and watering duration values?
- Exit/Sleep

### Screens

#### List all / main screen

      0123456789abcdef
     +----------------+
    0|P1 P2 P3 P4 == X|
    1|+2 -3 +5 -9    S|
     +----------------+

- Next: moves 3 positions at a time (wrap around jumps to 0)
- Cursor default position: (0, f).
- Menu items:
  - = (burger menu) - accesses menu (what's in the menu? logs? 'Reset all'
    function?)
  - X (close) - puts the system in sleep mode
- Display items:
  - 'Pn' and '+/-a' - plant ID and latest relative moisture
  - S (status) - lists the system status. In case of internal errors, a skull
    will be shown there

#### Status (one plant)

      0123456789abcdef
     +----------------+
    0|P1 Now:52 Ref:50|
    1|WET(d:+2)  ☔ > X|
     +----------------+

Next: moves 2 positions at a time (wrap around jumps to b)
Cursor default position: (1, f).


#### Confirm watering one plant

      0123456789abcdef
     +----------------+
    0|P1 Now:52 Ref:50|
    1|Water now? Y/N/X|
     +----------------+

#### References screen

      0123456789abcdef
     +----------------+
    0|P1 Refs 47 53 51|
    1|ResetAll?    Y/X|
     +----------------+
