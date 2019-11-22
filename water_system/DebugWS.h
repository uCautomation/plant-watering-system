#ifndef DEBUGWS_H
#define DEBUGWS_H

#ifndef DEBUG

#ifdef DEBUG_ON
#define DEBUG(fmt, args ...)                   \
    do {                                       \
        char dbgbuf[200];                      \
        sprintf(dbgbuf, "DBG: " fmt, ## args); \
        Serial.println(dbgbuf);                \
    } while(0)

#else
#define DEBUG(fmt, args ...)

#endif // #ifdef DEBUG_ON

#endif // #ifndef DEBUG


#endif // DEBUGWS_H
