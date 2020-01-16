#ifndef WSMENU_H
#define WSMENU_H

#include "ws_types.h"
#include "WSRuntime.h"

class WSMenu {
    private:
        int _cursor_columns_step; // Each menu has items are at constant offset
        int _no_of_menu_entries; // note, last item is default
        int _0th_item_column; // the column number on which the first menu item is on
        int _active_lcd_line; // Only one LCD line can contain menu items

        byte _selected_item; // the currently selected item in menu

        inline byte _setSaneSelectedItem(byte itemIndex)
        {
            byte saneIndex = itemIndex % _no_of_menu_entries;
            noInterrupts();
            _selected_item = saneIndex;
            interrupts();

            return saneIndex;
        }

    public:
        WSMenu
        (
            int MenuColumnStep,
            int NoOfMenuItems,
            int MenuStartsAtColumn=0,
            int MenuLine=0
        ) :
            _cursor_columns_step(MenuColumnStep),
            _no_of_menu_entries(NoOfMenuItems),
            _0th_item_column(MenuStartsAtColumn),
            _active_lcd_line(MenuLine)
        {
            _selected_item = _no_of_menu_entries - 1; // default is rightmost on screen (typically 'X'/Exit/Sleep)
            // TODO: map each menu item with a next state (with parameter)
            // we need to get the OK button functions to execute for each

            // Well constructed menu can't run out of the screen
            if (_selected_item * _cursor_columns_step + _0th_item_column > 0xf) {
                system_panic_no_return();
            }
        }

        void openMenu()
        {
            (void)_setSaneSelectedItem(_no_of_menu_entries - 1);
        }

        byte nextMenuEntry()
        {
            return _setSaneSelectedItem(_selected_item + 1);
        }

        int getSelectedMenuEntry()
        {
            return _selected_item;
        }

        int getLcdCursorColumn()
        {
            return _0th_item_column + _selected_item * _cursor_columns_step;
        }

        inline int getLcdCursorLine()
        {
            return _active_lcd_line;
        }

};

#endif // WSMENU_H
