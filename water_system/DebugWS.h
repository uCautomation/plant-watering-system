#ifndef DEBUGWS_H
#define DEBUGWS_H

#ifndef DEBUG

#ifdef DEBUG_ON
#define MAX_DEBUG_MSG_LEN 80

#define DEBUG(fmt, args ...)                    \
    do {                                        \
        char dbgbuf[MAX_DEBUG_MSG_LEN + 1];     \
        snprintf(dbgbuf, MAX_DEBUG_MSG_LEN, "DBG: " fmt, ## args); \
        Serial.println(dbgbuf);                 \
    } while(0)

#else
#define DEBUG(fmt, args ...)

#endif // #ifdef DEBUG_ON

#endif // #ifndef DEBUG


#endif // DEBUGWS_H
