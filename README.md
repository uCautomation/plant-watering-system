# plant-watering-system

[![Build Status](https://travis-ci.com/uCautomation/plant-watering-system.svg?branch=master)](https://travis-ci.com/uCautomation/plant-watering-system)

An automatic plant watering system with 4 watering modules (sensor+valve).

The user can interact with the system via the 16x2 LCD display and the user buttons.

TODO: Save power by going to sleep on timeout.

## watering modules

Each watering modules needs has 3 conenctions to the Arduino:
- Vsens - power supply for the moisture sensor; connected to 1 digital output to prolong the life of the sensor by powering only when needed
- Dpump - control pin for the valve; connected to 1 digital output pin of the Arduino
- Asens - moisture sensor read value; conenctedt to 1 analog input pin

Power:
Conenctions to V12 and GND are also needed to control. The valve operates at 12V, so each module also needs V12 supply to work.

## Interface

TODO: Next and OK buttons allow selecting the menu options:

* Status - display last read sensors' values, dry and wet thresholds and other status info
* Calibrate sensor - Allow selection and dry/wet threshold module calibration
* Water now(?) - select a module and water it under manual command - maybe should use to auto-calibrate dry,wet and watering duration values?
* Exit/Sleep
