#ifndef DEBUGWS_H
#define DEBUGWS_H

#ifndef DEBUG

#ifdef DEBUG_ON
#define MAX_DEBUG_MSG_LEN 80

extern char dbgbuf[MAX_DEBUG_MSG_LEN + 1];

#define DEBUG(fmt, args ...)                                               \
    do {                                                                   \
        int ret = snprintf(dbgbuf, MAX_DEBUG_MSG_LEN, "D: " fmt, ## args); \
        dbgbuf[MAX_DEBUG_MSG_LEN] = 0;                                     \
        Serial.println(dbgbuf);                                            \
        if ( (ret < 0) || (ret >= MAX_DEBUG_MSG_LEN)) {                    \
            Serial.println(F(" Debug message buffer overflow !"));         \
            system_panic_no_return();                                      \
        };                                                                 \
    } while (0)

#define DEBUG_P(msg) Serial.print(F(msg))

#else
#define DEBUG(fmt, args ...)
#define DEBUG_P(msg)

#endif // #ifdef DEBUG_ON

#endif // #ifndef DEBUG


#endif // DEBUGWS_H
