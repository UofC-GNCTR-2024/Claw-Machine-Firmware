// main_drivers.h

#ifndef __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__

#include "main.h"

typedef enum {
    LIMIT_X1,
    LIMIT_X2,
    LIMIT_Y,
    INPUT_BTN_1,
    INPUT_BTN_2,
    INPUT_BTN_3,
    AUX_SW_1,
    AUX_SW_2,
    AUX_SW_3,
    STICK_NORTH,
    STICK_EAST,
    STICK_SOUTH,
    STICK_WEST
} limit_switch_t;

typedef enum {
    Z_MOTOR_DIRECTION_DROP,
    Z_MOTOR_DIRECTION_RAISE,
    Z_MOTOR_DIRECTION_STOP
} z_motor_direction_t;


void init_pin_modes();


/* True=Enable/Grab, False=Disable/Release. */
void set_claw_state(bool state);

/* True=Triggered, False=Not Triggered. */
bool get_switch_state(limit_switch_t limit_switch);

void set_z_motor_state(z_motor_direction_t direction);


void debug_print_all_limit_switch_states();
void debug_print_all_limit_switch_states(bool verbose);

#endif // __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__
