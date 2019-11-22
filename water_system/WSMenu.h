#ifndef WSMENU_H
#define WSMENU_H

#include "ws_types.h"
#include "WaterSystemSM.h"
#include "WSRuntime.h"

class WSMenu {
    private:
        int _cursor_columns_step; // Each menu has items are at constant offset
        int _no_of_menu_entries; // note, last item is default
        int _active_lcd_line; // Only one LCD line can contain menu items
        int _0th_item_column; // the column number on which the first menu item is on

        int _selected_item; // the currently selected item in menu

    public:
        WSMenu
        (
            int MenuColumnStep,
            int NoOfMenuItems,
            int MenuLine=0,
            int MenuStartsAtColumn=0
        ) :
            _cursor_columns_step(MenuColumnStep),
            _no_of_menu_entries(NoOfMenuItems),
            _active_lcd_line(MenuLine),
            _0th_item_column(MenuStartsAtColumn)
        {
            _selected_item = _no_of_menu_entries - 1; // default is rightmost on screen (typically 'X'/Exit/Sleep)
            // TODO: map each menu item with a next state (with parameter)
            // we need to get the OK button functions to execute for each

            // Well constructed menu can't run out of the screen
            if (_selected_item * _cursor_columns_step + _0th_item_column > 0xf) {
                system_panic_no_return();
            }
        }

        int nextMenuEntry()
        {
            noInterrupts();
            _selected_item = (_selected_item + 1) % _no_of_menu_entries;
            interrupts();
            return _selected_item;
        }

        int getSelectedMenuEntry()
        {
            return _selected_item;
        }

        int getSelectedMenuEntryColumn()
        {
            return _0th_item_column + _selected_item * _cursor_columns_step;
        }

};

// Represents a single menu entry
class WSMenuItem
{
    private:
        /* data */
        wss_type _state_at_ok;

    public:
        WSMenuItem(wss_type onOkState);
        wss_type OnOKState();
        ~WSMenuItem();
};

WSMenuItem::WSMenuItem (wss_type onOkState=wss_sleep)
    : _state_at_ok(onOkState) { }

WSMenuItem::~WSMenuItem() { }

#endif // WSMENU_H