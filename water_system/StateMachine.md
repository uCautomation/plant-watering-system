# Water System State Machine

## Principles

The Water System State Machine is designed with several principles in mind:

1. There are two buttons, `OK` and `Next`, which should always behave in a consistent manner, no matter the current state or menu
  1. `OK` always confirms or activates a specific item (e.g. menu, menu entry)
  2. `Next` is always used to move between items, be they menus, menu items or screen display information groups
2. The default action on time out is sleep;
  1. In sleep, a timeout should wake up, check status and water the plants which need it
3. If a menu is activated, closing the menu shall bring the state machine into the "display" state from which the menu was activated
4. Pressing 'X' menu entry shall exit he menu active state
5. The switch from one item shall be natural and shall use the `next` button

## State Machine Diagrams

The diagras below actually form a single state machine, but, for clearness and ease of explantion reasons, it is split between several sub-diagrams.

### Timeout means sleep

By default, for most system states, on timeout, the system will go into sleep state


    +--------------+     timeout     +------------+
    | (some state) +---------------->|    sleep   |
    +--------------+                 +------------+

### Main menu cycle

`Next` will jump from one item to the next, as long as `OK` is not pressed, and no timeout occurs


    +-----------+    Next       +------------+
    | list  all +-------------->| sys status |
    +-----------+               +------------+
          ^                           |
          |                           | Next
     Next |       +-----------+       |
          +-------+ ctrl  all |<------+
                  +-----------+

### Menu activation and deactivation

`OK` in menu item activates the menu, if possible.


    +-------------------+
    | (some  screen  S) |
    | (w/ context menu) |<---------+
    +---------+---------+          |
              |                    | OK(X)
              | OK                 |
              |               +----+-----+
              +-------------->| (S menu) |
                              +----------+

### From sleep, on `OK` or `Next` we go to `list all`

When in sleep, pressing on any of the buttons transitions the system to `list all`.

    +-------+   OK/Next   +----------+
    | sleep +------------>| list all |
    +-------+             +----------+
