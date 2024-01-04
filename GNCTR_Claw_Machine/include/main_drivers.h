// main_drivers.h

#ifndef __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__

#include "main.h"
#include <AccelStepper.h>

#define STEPPER_MICROSTEPS 16.0

typedef enum {
    LIMIT_X1,
    LIMIT_X2,
    LIMIT_Y,
    CLAW_UP_BTN,
    CLAW_DOWN_BTN,
    CLAW_GRAB_BTN,
    START_BTN,
    AUX_SW_2,
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

typedef enum {
    CLAW_ENGAGE,
    CLAW_RELEASE
} claw_mode_t;


void init_pin_modes();
void init_steppers();
void init_display();

/* True=Enable/Grab, False=Disable/Release. */
void set_claw_state(claw_mode_t state);

/* True=Triggered, False=Not Triggered. */
bool get_switch_state(limit_switch_t limit_switch);

void set_z_motor_state(z_motor_direction_t direction);

void set_start_button_led(bool state);

void debug_print_all_limit_switch_states();
void debug_print_all_limit_switch_states(bool verbose);


AccelStepper& init_stepper(AccelStepper &stepper, uint8_t enablePin, uint8_t stepPin, uint8_t dirPin, bool reverse);


void loop_moveMotorsBasedOnButtons();
void loop_dropOrRaiseClaw();

void i2c_scan();

#endif // __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__
