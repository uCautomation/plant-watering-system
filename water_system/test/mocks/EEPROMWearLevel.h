#ifndef EEPROM_WEAR_LEVEL_H
#define EEPROM_WEAR_LEVEL_H

// #include <EEPROM.h>

class EEPROMClass {
};

class EEPROMWearLevel {
  public:
    void begin(const byte layoutVersion, const int amountOfIndexes) {};

    template< typename T > T &get(const int idx, T &t) {
      return t;
    }
    template< typename T > T &put(const int idx, T &t) {
      return t;
    }
};


/**
   the instance of EEPROMWearLevel
*/
extern EEPROMWearLevel EEPROMwl;
extern EEPROMClass EEPROM;

#endif /* EEPROM_WEAR_LEVEL_H */