#ifndef WS_DEFS_H
#define WS_DEFS_H

#define MAX_MODULE_COUNT        4U
#define MIN_REWATER_INTERVAL_MS 1000U
#define HUMAN_PERCEPTIBLE_MS    500U
#define MAX_ADC_VALUE           1023

// after some calibration, when the sensor was dipped in water,
// didn't see values below this limit
#define SEEN_MIN_LIMIT   250
// actually it was around 1002, but this is practically the same
#define SEEN_MAX_LIMIT   MAX_ADC_VALUE

// #define SENSOR_USES_DIRECT_PROPORTION


#if defined SENSOR_USES_DIRECT_PROPORTION
    #define SEEN_SENSOR_DRY_LIMIT   SEEN_MIN_LIMIT
    #define SEEN_SENSOR_WET_LIMIT   SEEN_MAX_LIMIT

    #define SENSOR_SIGN 1

#else
    #define SEEN_SENSOR_DRY_LIMIT   SEEN_MAX_LIMIT
    #define SEEN_SENSOR_WET_LIMIT   SEEN_MIN_LIMIT

    #define SENSOR_SIGN -1

#endif

#endif // WS_DEFS_H
