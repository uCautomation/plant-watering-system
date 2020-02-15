# plant-watering-system

[![Build Status](https://travis-ci.com/uCautomation/plant-watering-system.svg?branch=master)](https://travis-ci.com/uCautomation/plant-watering-system)

An automatic plant watering system with 4 watering modules (sensor + valve/pump).

The user can interact with the system via the 16x2 LCD display and the user buttons.

## TODO

- BUG: Check why plant 0 is marked as not used, while others are used
- Disable/Enable module
- Reset/Forget module calibration (i.e. change plant)
- Save power by going to sleep on timeout
- Test the WaterSystem state machine
- Persistent calibration data after reset each module; save the calibration data
  into the EEPROM
  - Save the calibration data into a sort-of flash/EEPROM filesystem to prevent
    early death of specific EEPROM memory cells - see Generational EEPROM storage
- Persistent log system (using EEPROM)
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

All screens' menus are activated with OK.
When the menu is active, the current active menu position is highlighted with
the cursor, and selected with OK.

#### List all / main screen

      0123456789abcdef
     +----------------+
    0|P1 P2 P3 P4 == X|
    1|+2 -3 +5 -9    S|
     +----------------+

In this menu, when active:

- OK: selects the active item
- Next: moves 3 positions(characters on LCD) at a time (wrap around jumps to 0)
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

When this menu is active:

- OK: Selects active menu item
- Next: moves 2 positions at a time (wrap around jumps to b)
- Cursor default position: (1, f).
- Menu items:
  - Water Now: manual watering of the selected plant
  - Calibration details and control (one plant)
  - Exit / Sleep


#### Confirm watering one plant

      0123456789abcdef
     +----------------+
    0|P1 Now:52 Ref:50|
    1|Water now? Y/N/X|
     +----------------+

Do we need this? Maybe later? Probably we're over-engineering at this point.

#### Calibration details and control (one plant) screen

      0123456789abcdef
     +----------------+
    0|P1 Refs 47 53 51|
    1|>  NoUse Reset X|
     +----------------+

- **Menu is active immediately (no display mode)**
- Cursor: default (1,f), step=6, wrap around jumps to 3
- Menu items:
  - NoUse - disables module (no plant exists for this module)
    - this entry will need to be "Enable/Use" if currently the module has no plant, i.e. was previously disabled
  - Reset - forget all calibration data and set threshold to default (middle of the range?)
    - could this be better named instead a "new plant" menu item? What is the string here? "New (plant_icon)"?


How do we get to this screen?
Show this for single-plant status '>' menu item?

## Generational EEPROM storage (GeneROMst)

### Rationale

We want to have the most recent calibration data and logs even in case of reset,
power on, or after recovering from system failures.

In case of system failures, internal error logs and regular logs can help identify and debug the runtime issue.

Even if the EEPROM on ATmega48/88/168/328 has 100.000 write/erase cycles, if we
plan a life of 10 years for the system, we only get a rough maximum of 1.14
writes/hour per cell. But if we want to log the system state and sensor read
values on each wake up, we are forced have at most 1 wake up per hour to reach
our desired lifetime, which is very constrained.

In case of a system detecting various asynchronous errors or events, logging and
state saving to EEPROM could easily jump above that average. During the test
phase several write/erase cycles will take place, also.

We want to guarantee the long life of the system. Because one limiting factor
is the EEPROM, we need a way to spread out the wear level of EEPROM cells,
instead of simply writing over the same locations.

As a consequence, we want to split the EEPROM memory in 2 regions: 1 for
calibration info, one for runtime error logging. Each region can have multiple
generations of data written into them.

Both regions can have a generation index, but the interpretation of the value
can differ between the two regions:

- For the calibration region, the data is redundant, the most recent valid index
  is the data we want to load on system reset
- For the log region, the storage is a circular buffer of entries; the oldest
  log entries will be overwritten in case the memory is full and a new even must
  be saved.

### GeneROMst design details

- a GeneROMst region is split in equally sized entries
- each entry can be valid or invalid (erased or incorrectly/incompletely written)
  - validation of an entry's data is made via a check sum or a correction code
    such as: even/odd parity, CRC, LDPC, Hamming structure, an ECC such as a
    Reed Solomon code etc.
- each entry has one unique (at any given time) generation index (Gi)
- generation indexes can be recycled, provided that any GeneROMst system state
  never contains the same Gi more than once in different valid entries
- each region, if it contains at least one valid entry, has a most-recent
  generation index and a least-recent generation index
- on writing, the entry to be written is selected following these steps:
  - if invalid entries exist, one such entry will be chosen, at random
  - if all entries are valid, the least recent entry will be selected
- notations:
  - S = size of the GeneROMst region (in bytes)
  - E = GeneROMst entry size (in bytes)
  - N = maximum number of generations which can be stored in the region
        This results from S / E
  - Gi = generation index for entry i

Note: It is easier to implement a GeneROMst region with both S and E as exact
powers of 2, or, at least, with N as a power of 2. Other values should work,
but they could complicate some accesses, decodings, identification and correct
handling of corner cases.

TODO:

1. What happens if we select for write an invalid entry which has a broken cell?

With the current algorithm, we would end up hammering on it, without detecting
this faulty situation, effectively killing it?

Possible solutions:

- Add a BAD marker?
- Rely on system log info?
- Randomly choose between 2 oldest ones?
- Log the last tried somewhere (in a consensus-like bad block identification?)
  and avoid often occurring entry numbers? What happens if multiple ones are
  broken? How do we detect that?

### Detecting and calculating newest and oldest entries

For regular operation, depending on the use - "latest is best" or "circular
buffer" - we always need to be able to identify the most recent, oldest and
free/invalid entries.

In case of a log read, the valid entries need to be ordered in chronological
order.

In case of a calibration data read, since last is best, we need to detect
the most recent entry.

To be able to implement these mechanisms the following rules are applied:

- generation indexes of any entry i (Gi) will be in the range [0, 2N-1]
- only the Gi of regions with correct check sums are considered valid
- due to index reuse, indexes will wrap around, so a proposed Gi=0 would be the
  most recent entry in the GeneROMst, newer than any other Gj>0
- to detect wrap around of the Gi indexes, we find the maximum Gi (GM) and
  minimum Gi (Gm)
- if the distance/difference (D) between the indexes
  - (GM - Gm) >= N, then we have wrap around situation.
    Note: not always GM=2N-1 and Gm=0 because some writes might have failed
    - in case of wrap around, to ease up chronological ordering, we need
      to calculate normalized generation indexes of entry i (gi):
      gi = (Gi + N) % N
  - if (GM - Gm < N) then gi = Gi

After calculation of all gi values, the oldest entry i has the smallest
gi value, while the newest entry j will have the biggest gj value.


#### Example for old/new calulation (all sizes in bytes)

Assuming the following GeneROMst region

- entry size E=32
- GeneROMst (EEPROM region) size S=512
- maximum number of stored generations N = S / E = 512 / 32 = 16
  It follows that any Gi is in range [0, 2N), so 0 <= Gi < 2N,
  in our example, 0 <= Gi < 32
- normalization of the following G indexes:
    G: 2,  3,  5, 18
    because GM=18 & Gm=2, D=16 which is >= N (16),
    then we have wrap around, so normalized generations are:
    G:  2,  3,  5, 18
    g: 18, 19, 21,  2
    so G=18 is oldest, and G=5 is newest

In case of non-wrap around cases, the normalized indexes (g) are equal to
non-normalized indexes (G), so any Gi=gi, for any entry i.
