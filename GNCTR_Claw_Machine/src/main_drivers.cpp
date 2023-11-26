
#include "main.h"
#include <AccelStepper.h>

void init_pin_modes()
{
    // Switch Input Pins
    pinMode(PIN_LIMIT_X1, INPUT);
    pinMode(PIN_LIMIT_X2, INPUT);
    pinMode(PIN_LIMIT_Y, INPUT);
    pinMode(PIN_INPUT_BTN_1, INPUT);
    pinMode(PIN_INPUT_BTN_2, INPUT);
    pinMode(PIN_INPUT_BTN_3, INPUT);
    pinMode(PIN_AUX_SW_1, INPUT);
    pinMode(PIN_AUX_SW_2, INPUT);

    // Control Stick Input Pins
    pinMode(PIN_STICK_NORTH_BTN, INPUT);
    pinMode(PIN_STICK_EAST_BTN, INPUT);
    pinMode(PIN_STICK_SOUTH_BTN, INPUT);
    pinMode(PIN_STICK_WEST_BTN, INPUT);

    // Assorted Output Pins
    pinMode(PIN_ONBOARD_LED, OUTPUT);

    // Stepper Motor Output Pins
    pinMode(PIN_X1_NEN, OUTPUT);
    pinMode(PIN_X1_STEP, OUTPUT);
    pinMode(PIN_X1_DIR, OUTPUT);
    pinMode(PIN_X2_NEN, OUTPUT);
    pinMode(PIN_X2_STEP, OUTPUT);
    pinMode(PIN_X2_DIR, OUTPUT);
    pinMode(PIN_Y_NEN, OUTPUT);
    pinMode(PIN_Y_STEP, OUTPUT);
    pinMode(PIN_Y_DIR, OUTPUT);

    // DC Motor Output Pins
    pinMode(PIN_Z_DC_IN1, OUTPUT);
    pinMode(PIN_Z_DC_IN2, OUTPUT);
    pinMode(PIN_Z_DC_IN3, OUTPUT);
    pinMode(PIN_Z_DC_IN4, OUTPUT);
    pinMode(PIN_Z_DC_EN_A, OUTPUT);
    pinMode(PIN_Z_DC_EN_B, OUTPUT);

    // DC Motor Encoder Pins
    pinMode(PIN_Z_ENC_SIG_1, INPUT);
    pinMode(PIN_Z_ENC_SIG_2, INPUT);

    // General Power Output Pins
    pinMode(PIN_GENERAL_PWR_EN_0, OUTPUT);
    pinMode(PIN_GENERAL_PWR_EN_1, OUTPUT);
    pinMode(PIN_GENERAL_PWR_EN_2, OUTPUT);
    pinMode(PIN_GENERAL_PWR_EN_3, OUTPUT);
    pinMode(PIN_CLAW_EN, OUTPUT);
}

/* True=Enable/Grab, False=Disable/Release. */
void set_claw_state(bool state)
{
    if (state) {
        digitalWrite(PIN_CLAW_EN, HIGH);
        Serial.println("DEBUG: Claw enabled.");
    }
    else {
        digitalWrite(PIN_CLAW_EN, LOW);
        Serial.println("DEBUG: Claw disabled.");
    }
}

/* True=Triggered, False=Not Triggered. */
bool get_switch_state(limit_switch_t limit_switch)
{
    if (limit_switch == LIMIT_X1) {
        return digitalRead(PIN_LIMIT_X1);
    }
    else if (limit_switch == LIMIT_X2) {
        return digitalRead(PIN_LIMIT_X2);
    }
    else if (limit_switch == LIMIT_Y) {
        return digitalRead(PIN_LIMIT_Y);
    }
    else if (limit_switch == INPUT_BTN_1) {
        return digitalRead(PIN_INPUT_BTN_1);
    }
    else if (limit_switch == INPUT_BTN_2) {
        return digitalRead(PIN_INPUT_BTN_2);
    }
    else if (limit_switch == INPUT_BTN_3) {
        return digitalRead(PIN_INPUT_BTN_3);
    }
    else if (limit_switch == AUX_SW_1) {
        return digitalRead(PIN_AUX_SW_1);
    }
    else if (limit_switch == AUX_SW_2) {
        return digitalRead(PIN_AUX_SW_2);
    }
    else if (limit_switch == STICK_NORTH) {
        return digitalRead(PIN_STICK_NORTH_BTN);
    }
    else if (limit_switch == STICK_EAST) {
        return digitalRead(PIN_STICK_EAST_BTN);
    }
    else if (limit_switch == STICK_SOUTH) {
        return digitalRead(PIN_STICK_SOUTH_BTN);
    }
    else if (limit_switch == STICK_WEST) {
        return digitalRead(PIN_STICK_WEST_BTN);
    }
    else {
        Serial.println("ERROR: Coding error. Invalid limit_switch_t value.");
        return false;
    }
}

void set_z_motor_state(z_motor_direction_t direction)
{
    if (direction == Z_MOTOR_DIRECTION_DROP) {
        digitalWrite(PIN_Z_DC_EN_A, HIGH);
        digitalWrite(PIN_Z_DC_IN1, HIGH); // TODO: confirm direction
        digitalWrite(PIN_Z_DC_IN2, LOW);
        Serial.println("DEBUG: Z motor dropping.");
    }
    else if (direction == Z_MOTOR_DIRECTION_RAISE) {
        digitalWrite(PIN_Z_DC_EN_A, HIGH);
        digitalWrite(PIN_Z_DC_IN1, LOW);
        digitalWrite(PIN_Z_DC_IN2, HIGH);
        Serial.println("DEBUG: Z motor raising.");
    }
    else if (direction == Z_MOTOR_DIRECTION_STOP) {
        digitalWrite(PIN_Z_DC_EN_A, LOW);
        digitalWrite(PIN_Z_DC_IN1, LOW);
        digitalWrite(PIN_Z_DC_IN2, LOW);
        Serial.println("DEBUG: Z motor stopped.");
    }
    else {
        Serial.println(
            "ERROR: Coding error. Invalid z_motor_direction_t value.");
    }
}

void debug_print_all_limit_switch_states()
{
    debug_print_all_limit_switch_states(false);
}

void debug_print_all_limit_switch_states(bool verbose)
{
    if (verbose) {
        Serial.print("DEBUG: Switches: ");
        Serial.print("LIMIT_X1: ");
        Serial.print(get_switch_state(LIMIT_X1));
        Serial.print(", LIMIT_X2: ");
        Serial.print(get_switch_state(LIMIT_X2));
        Serial.print(", LIMIT_Y: ");
        Serial.print(get_switch_state(LIMIT_Y));
        Serial.print(", INPUT_BTN_1: ");
        Serial.print(get_switch_state(INPUT_BTN_1));
        Serial.print(", INPUT_BTN_2: ");
        Serial.print(get_switch_state(INPUT_BTN_2));
        Serial.print(", INPUT_BTN_3: ");
        Serial.print(get_switch_state(INPUT_BTN_3));
        Serial.print(", AUX_SW_1: ");
        Serial.print(get_switch_state(AUX_SW_1));
        Serial.print(", AUX_SW_2: ");
        Serial.print(get_switch_state(AUX_SW_2));
        Serial.print(", NORTH: ");
        Serial.print(get_switch_state(STICK_NORTH));
        Serial.print(", EAST: ");
        Serial.print(get_switch_state(STICK_EAST));
        Serial.print(", SOUTH: ");
        Serial.print(get_switch_state(STICK_SOUTH));
        Serial.print(", WEST: ");
        Serial.println(get_switch_state(STICK_WEST));
    }
    else {
        Serial.print("DEBUG: Triggered Switches: ");
        if (get_switch_state(LIMIT_X1)) {
            Serial.print("LIMIT_X1 ");
        }
        if (get_switch_state(LIMIT_X2)) {
            Serial.print("LIMIT_X2 ");
        }
        if (get_switch_state(LIMIT_Y)) {
            Serial.print("LIMIT_Y ");
        }
        if (get_switch_state(INPUT_BTN_1)) {
            Serial.print("INPUT_BTN_1 ");
        }
        if (get_switch_state(INPUT_BTN_2)) {
            Serial.print("INPUT_BTN_2 ");
        }
        if (get_switch_state(INPUT_BTN_3)) {
            Serial.print("INPUT_BTN_3 ");
        }
        if (get_switch_state(AUX_SW_1)) {
            Serial.print("AUX_SW_1 ");
        }
        if (get_switch_state(AUX_SW_2)) {
            Serial.print("AUX_SW_2 ");
        }
        if (get_switch_state(STICK_NORTH)) {
            Serial.print("NORTH ");
        }
        if (get_switch_state(STICK_EAST)) {
            Serial.print("EAST ");
        }
        if (get_switch_state(STICK_SOUTH)) {
            Serial.print("SOUTH ");
        }
        if (get_switch_state(STICK_WEST)) {
            Serial.print("WEST ");
        }
        Serial.println();
    }
}

AccelStepper& init_stepper(AccelStepper &stepper, uint8_t enablePin, uint8_t stepPin, uint8_t dirPin, bool reverse)
{
    stepper = AccelStepper(AccelStepper::DRIVER, stepPin, dirPin);
    stepper.setEnablePin(enablePin);
    stepper.setPinsInverted(reverse, false, true);  // Invert enable pin because it's active-low

    stepper.setMaxSpeed(STEPPER_MICROSTEPS * 10000);
    stepper.setAcceleration(STEPPER_MICROSTEPS * 5);
    // stepper.setSpeed(500*16);
    stepper.enableOutputs();

    return stepper;
}