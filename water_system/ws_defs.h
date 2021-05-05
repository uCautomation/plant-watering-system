#ifndef WS_DEFS_H
#define WS_DEFS_H

#define MAX_MODULE_COUNT        4U
#define MIN_REWATER_INTERVAL_MS 1000U
#define HUMAN_PERCEPTIBLE_MS    500U
#define MAX_ADC_VALUE           1023

// after some calibration, when the sensor was dipped in water,
// didn't see values below this limit
#define SEEN_SENSOR_WET_LIMIT   250
// actually it was around 1002, but this is practically the same
#define SEEN_SENSOR_DRY_LIMIT   MAX_ADC_VALUE

// #define SENSOR_USES_DIRECT_PROPORTION

#endif // WS_DEFS_H
