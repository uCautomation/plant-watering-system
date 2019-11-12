#ifndef WATERSYSTEM_H
#define WATERSYSTEM_H

#include <limits.h>

#define DEBUG_ON
#include "DebugWS.h"

#include "ws_types.h"



class WaterSystem
{
private:
    /* data */
    bool _some_module_selected;
    byte _selected_module = 0;

    byte saneModuleIndex();
public:
    WaterSystem();
    void activateSelection();
    void deactivateSelection();

    bool selectNextModule();
    bool hasActiveModule(byte *pModuleIdx);
};


ulong timedelta(ulong ref_timestamp, ulong now);


#endif // WATERSYSTEM_H
