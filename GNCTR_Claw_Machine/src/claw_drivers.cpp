
#include "claw_drivers.h"

int32_t z_axis_net_duration_down_ms = 0; // positive ms for down
const float z_axis_speed_ratio_up_to_down = 1.25; // claw raises 1.25x faster than it drops
const int32_t z_axis_max_net_duration_down_ms = 2800;


void set_claw_state(claw_mode_t state)
{
    if (state == CLAW_ENGAGE) {
        digitalWrite(PIN_CLAW_EN, HIGH);
        // Serial.println("DEBUG: Claw enabled.");
    }
    else if (state == CLAW_RELEASE) {
        digitalWrite(PIN_CLAW_EN, LOW);
        // Serial.println("DEBUG: Claw disabled.");
    }
}

z_motor_direction_t last_z_motor_direction = Z_MOTOR_DIRECTION_STOP;
uint32_t last_z_motor_direction_update_time_ms = 0;
void set_z_motor_state(z_motor_direction_t direction) {
    // drives the z motor, also tracking the net duration that it has been down for
    if (last_z_motor_direction != Z_MOTOR_DIRECTION_STOP) {
        // add the amount of time it moved to the net duration
        int32_t net_dir_down_ms = millis() - last_z_motor_direction_update_time_ms;
        if (last_z_motor_direction == Z_MOTOR_DIRECTION_RAISE) {
            // raising is negative, and has a derated speed
            net_dir_down_ms /= -z_axis_speed_ratio_up_to_down;
        }
        z_axis_net_duration_down_ms += net_dir_down_ms;
    }
    last_z_motor_direction_update_time_ms = millis();

    #ifdef DEBUG_CLAW
    Serial.print("DEBUG: z_axis_net_duration_down_ms=");
    Serial.print(z_axis_net_duration_down_ms);
    Serial.print(", direction=");
    if (direction == Z_MOTOR_DIRECTION_DROP) {
        Serial.print("Z_MOTOR_DIRECTION_DROP");
    }
    else if (direction == Z_MOTOR_DIRECTION_RAISE) {
        Serial.print("Z_MOTOR_DIRECTION_RAISE");
    }
    else if (direction == Z_MOTOR_DIRECTION_STOP) {
        Serial.print("Z_MOTOR_DIRECTION_STOP");
    }
    Serial.println();
    #endif


    // limit the net duration at the top
    if (z_axis_net_duration_down_ms < -500) {
        z_axis_net_duration_down_ms = -500;
    }

    if (direction == Z_MOTOR_DIRECTION_STOP && last_z_motor_direction == Z_MOTOR_DIRECTION_DROP) {
        // special case to make the claw stop better, when it transitions from down to stop
        set_z_motor_state_raw(Z_MOTOR_DIRECTION_RAISE);
        delay(40);
        set_z_motor_state_raw(Z_MOTOR_DIRECTION_STOP);
    }
    else {
        set_z_motor_state_raw(direction);
    }

    last_z_motor_direction = direction;
}

void set_z_motor_state_raw(z_motor_direction_t direction)
{
    // This function should only be called by set_z_motor_state() and home_z_axis().
    if (direction == Z_MOTOR_DIRECTION_DROP) {
        digitalWrite(PIN_Z_DC_EN_A, HIGH);
        digitalWrite(PIN_Z_DC_IN1, HIGH);
        digitalWrite(PIN_Z_DC_IN2, LOW);
        #ifdef DEBUG_CLAW
        Serial.println("DEBUG: Raw: Z motor dropping.");
        #endif
    }
    else if (direction == Z_MOTOR_DIRECTION_RAISE) {
        digitalWrite(PIN_Z_DC_EN_A, HIGH);
        digitalWrite(PIN_Z_DC_IN1, LOW);
        digitalWrite(PIN_Z_DC_IN2, HIGH);
        #ifdef DEBUG_CLAW
        Serial.println("DEBUG: Raw: Z motor raising.");
        #endif
    }
    else if (direction == Z_MOTOR_DIRECTION_STOP) {
        digitalWrite(PIN_Z_DC_EN_A, LOW);
        digitalWrite(PIN_Z_DC_IN1, LOW);
        digitalWrite(PIN_Z_DC_IN2, LOW);
        #ifdef DEBUG_CLAW
        Serial.println("DEBUG: Raw: Z motor stopped.");
        #endif
    }
    else {
        Serial.println(
            "ERROR: Coding error. Invalid z_motor_direction_t value.");
    }
}



void home_z_axis() {
    Serial.println("INFO: starting z-axis homing");
    set_z_motor_state_raw(Z_MOTOR_DIRECTION_RAISE);
    delay(3000); // 2 seconds to get to the top, plus a bit of buffer

    // drop it a tad
    set_z_motor_state_raw(Z_MOTOR_DIRECTION_DROP);
    delay(150);

    // do the special case to make the claw stop better, when it transitions from down to stop
    set_z_motor_state_raw(Z_MOTOR_DIRECTION_RAISE);
    delay(40);
    set_z_motor_state_raw(Z_MOTOR_DIRECTION_STOP);

    z_axis_net_duration_down_ms = 0;

    // write these directly
    last_z_motor_direction_update_time_ms = millis();
    last_z_motor_direction = Z_MOTOR_DIRECTION_STOP;

    Serial.println("INFO: z-axis homing complete");
}


void loop_dropOrRaiseClaw()
{
    bool upButton = get_switch_state(CLAW_UP_BTN);
    bool downButton = get_switch_state(CLAW_DOWN_BTN);

    z_motor_direction_t z_motor_direction = Z_MOTOR_DIRECTION_STOP;

    if (upButton && downButton) {
        // Serial.println("INFO: Both up and down buttons pressed.");
        z_motor_direction = Z_MOTOR_DIRECTION_STOP;
    }
    else if (upButton) {
        // Serial.println("INFO: Up button.");
        z_motor_direction = Z_MOTOR_DIRECTION_RAISE;
    }
    else if (downButton) {
        // Serial.println("INFO: Moving claw down");
        // track a maximum net duration that the claw can move down for
        if (z_axis_net_duration_down_ms < z_axis_max_net_duration_down_ms) {
            z_motor_direction = Z_MOTOR_DIRECTION_DROP;
        }
    }
    set_z_motor_state(z_motor_direction);

    // do claw
    if (get_switch_state(CLAW_GRAB_BTN)) {
        set_claw_state(CLAW_ENGAGE);
    }
    else {
        set_claw_state(CLAW_RELEASE);
    }

}

