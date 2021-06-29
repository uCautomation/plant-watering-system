#!/bin/sh -e

echo "Direct proportional tests..." && echo

sed -i 's@^.*#define SENSOR_USES_DIRECT_PROPORTION@#define SENSOR_USES_DIRECT_PROPORTION@' ../ws_defs.h && make && echo "Inverse proportional tests.." && echo && sed -i 's@#define SENSOR_USES_DIRECT_PROPORTION@// #define SENSOR_USES_DIRECT_PROPORTION@' ../ws_defs.h && make
