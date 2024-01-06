// main_drivers.h

#ifndef __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__

#include "main.h"
#include <AccelStepper.h>

#define STEPPER_MICROSTEPS 16.0

const long xAxisLength = 46000;
const long yAxisLength = 50000;

#define Z_UP_TO_DOWN_RATIO 1.25
#define Z_BRAKE_DURATION_MS 50

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
bool run_z_motor_for_duration_and_watch_start(z_motor_direction_t direction, uint32_t duration_ms);

void set_start_button_led(bool state);
void set_enclosure_led(bool state);
void loop_update_start_button_blinking();

void debug_print_all_limit_switch_states();
void debug_print_all_limit_switch_states(bool verbose);
void debug_print_axes_info();

AccelStepper& init_stepper(AccelStepper &stepper, uint8_t enablePin, uint8_t stepPin, uint8_t dirPin, bool reverse);


void loop_moveMotorsBasedOnButtons();
void loop_dropOrRaiseClaw();
void home_x_axis();
void home_y_axis();
void home_z_motor(uint16_t max_up_duration_ms);
void endgame_move_to_bin();
void move_claw_to_absolute_xy(long x, long y);
bool move_to_absolute_xy_and_watch_for_start_press(long x, long y);

void i2c_scan();

void display_duration_sec(uint32_t duration_sec);
void display_duration_ms(uint32_t duration_ms);
void display_blinking_zeros();
void display_scrolling_press_start();
void display_int(uint16_t int_val);
void display_int_no_leading_zeros(uint16_t int_val);
void display_raw_message(uint8_t *message);

void set_stepper_enable(bool enable);

#define SEG_A   0b00000001
#define SEG_B   0b00000010
#define SEG_C   0b00000100
#define SEG_D   0b00001000
#define SEG_E   0b00010000
#define SEG_F   0b00100000
#define SEG_G   0b01000000
#define SEG_DP  0b10000000

#endif // __INCLUDE_GUARD_MAIN_DRIVERS_H_CLAW_MACHINE__
