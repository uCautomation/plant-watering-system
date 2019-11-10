#include <Arduino.h>

#include "ws_defs.h"
#include "WaterSystem.h"

WaterSystem::WaterSystem(/* args */)
{
    _some_module_selected = false;
}

inline void WaterSystem::activateSelection()
{
    _selected_module = saneModuleIndex();
    _some_module_selected = true;
}

inline byte WaterSystem::saneModuleIndex()
{
    return _selected_module % MAX_MODULE_COUNT;
}

inline void WaterSystem::deactivateSelection()
{
    _some_module_selected = false;
}

bool WaterSystem::hasActiveModule(byte *pModuleIdx)
{
    *pModuleIdx = saneModuleIndex();
    return _some_module_selected;
}

bool WaterSystem::selectNextModule()
{
    noInterrupts();
    _selected_module++;
    _selected_module = saneModuleIndex();
    interrupts();
    return _some_module_selected;
}


ulong timedelta(ulong refts, ulong now)
{
  if (now >= refts)
    return now - refts;
  else // overflow
    return ULONG_MAX - refts + now;
}
