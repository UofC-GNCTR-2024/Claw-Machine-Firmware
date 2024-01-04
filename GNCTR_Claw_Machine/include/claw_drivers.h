
#ifndef __INCLUDE_GUARD_CLAW_DRIVERS_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_CLAW_DRIVERS_H_CLAW_MACHINE__

#include "main.h"

// #define DEBUG_CLAW 1

typedef enum {
    Z_MOTOR_DIRECTION_DROP,
    Z_MOTOR_DIRECTION_RAISE,
    Z_MOTOR_DIRECTION_STOP
} z_motor_direction_t;

typedef enum {
    CLAW_ENGAGE,
    CLAW_RELEASE
} claw_mode_t;


/* True=Enable/Grab, False=Disable/Release. */
void set_claw_state(claw_mode_t state);


void set_z_motor_state(z_motor_direction_t direction);
void set_z_motor_state_raw(z_motor_direction_t direction);
void home_z_axis();


void loop_dropOrRaiseClaw();


#endif // __INCLUDE_GUARD_CLAW_DRIVERS_H_CLAW_MACHINE__
