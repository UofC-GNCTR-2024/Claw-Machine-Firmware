// pin_defs.h

#ifndef __INCLUDE_GUARD_PIN_DEFS_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_PIN_DEFS_H_CLAW_MACHINE__

// Switch Input Pins
#define PIN_LIMIT_X1     2
#define PIN_LIMIT_X2     3
#define PIN_LIMIT_Y      4
#define PIN_INPUT_BTN_1  5
#define PIN_INPUT_BTN_2  6
#define PIN_INPUT_BTN_3  7
#define PIN_AUX_SW_1     8
#define PIN_AUX_SW_2     9

// Control Stick Input Pins
#define PIN_STICK_NORTH_BTN    22
#define PIN_STICK_EAST_BTN     23
#define PIN_STICK_SOUTH_BTN    24
#define PIN_STICK_WEST_BTN     25

// Assorted Output Pins
#define PIN_ONBOARD_LED  13

// Stepper Motor Output Pins
#define PIN_X1_NEN       26
#define PIN_X1_STEP      27
#define PIN_X1_DIR       28
#define PIN_X2_NEN       29
#define PIN_X2_STEP      30
#define PIN_X2_DIR       31
#define PIN_Y_NEN        32
#define PIN_Y_STEP       33
#define PIN_Y_DIR        34

// DC Motor Output Pins
#define PIN_Z_DC_IN1     35
#define PIN_Z_DC_IN2     36
#define PIN_Z_DC_IN3     37
#define PIN_Z_DC_IN4     38
#define PIN_Z_DC_EN_A    39 // EN for IN1 and IN2 // TODO: confirm
#define PIN_Z_DC_EN_B    40 // EN for IN3 and IN4 // TODO: confirm
// DC Motor Encoder Pins
#define PIN_Z_ENC_SIG_1  51
#define PIN_Z_ENC_SIG_2  52

// General Power Output Pins
#define PIN_GENERAL_PWR_EN_0 47 // aka PIN_CLAW_EN
#define PIN_GENERAL_PWR_EN_1 48
#define PIN_GENERAL_PWR_EN_2 49
#define PIN_GENERAL_PWR_EN_3 50
#define PIN_CLAW_EN PIN_GENERAL_PWR_EN_0

#endif // __INCLUDE_GUARD_PIN_DEFS_H_CLAW_MACHINE__
