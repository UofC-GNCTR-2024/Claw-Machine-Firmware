
#include "main.h"
#include <AccelStepper.h>
#include <MultiStepper.h>
#include "I2CScanner.h"
#include <HT16K33.h>

AccelStepper x1Stepper, x2Stepper, yStepper;
MultiStepper xSteppers;

HT16K33 display(I2C_SCREEN_ADDR);

const float stepper_speed = STEPPER_MICROSTEPS * 8000;
const long stepperMinPos = -80000;
const long stepperMaxPos = 80000;
long stepperMinMulti[] = {stepperMinPos, stepperMinPos};
long stepperMaxMulti[] = {stepperMaxPos, stepperMaxPos};

// either -1 or +1 to indicate the previous direction in each axis
int prevXdir = 0, prevYdir = 0;

z_motor_direction_t prev_z_motor_direction = Z_MOTOR_DIRECTION_STOP;


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

void init_display() {
    // Guide: https://registry.platformio.org/libraries/robtillaart/HT16K33/examples/demo_displayInt/demo_displayInt.ino
    
    Wire.begin();
    Wire.setClock(100000);
    display.begin();

    display.displayOn();
    display.setBrightness(0x0F); // 0x00 to 0x0F
    display.displayClear();
    display.setBlink(0);

    display.setDigits(4); // 4 leading zeros, I think
    display.displayInt(0);
}

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

/* True=Triggered, False=Not Triggered. */
bool get_switch_state(limit_switch_t limit_switch)
{
    switch (limit_switch) {
        case LIMIT_X1:
            return digitalRead(PIN_LIMIT_X1);
        case LIMIT_X2:
            return digitalRead(PIN_LIMIT_X2);
        case LIMIT_Y:
            return digitalRead(PIN_LIMIT_Y);
        case CLAW_UP_BTN:
            return digitalRead(PIN_INPUT_BTN_1);
        case CLAW_DOWN_BTN:
            return digitalRead(PIN_INPUT_BTN_2);
        case CLAW_GRAB_BTN:
            return digitalRead(PIN_INPUT_BTN_3);
        case START_BTN:
            return digitalRead(PIN_AUX_SW_1);
        case AUX_SW_2:
            return digitalRead(PIN_AUX_SW_2);
        case STICK_NORTH:
            return digitalRead(PIN_STICK_NORTH_BTN);
        case STICK_EAST:
            return digitalRead(PIN_STICK_EAST_BTN);
        case STICK_SOUTH:
            return digitalRead(PIN_STICK_SOUTH_BTN);
        case STICK_WEST:
            return digitalRead(PIN_STICK_WEST_BTN);
        default:
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
        // Serial.println("DEBUG: Z motor dropping.");
    }
    else if (direction == Z_MOTOR_DIRECTION_RAISE) {
        digitalWrite(PIN_Z_DC_EN_A, HIGH);
        digitalWrite(PIN_Z_DC_IN1, LOW);
        digitalWrite(PIN_Z_DC_IN2, HIGH);
        // Serial.println("DEBUG: Z motor raising.");
    }
    else if (direction == Z_MOTOR_DIRECTION_STOP) {
        digitalWrite(PIN_Z_DC_EN_A, LOW);
        digitalWrite(PIN_Z_DC_IN1, LOW);
        digitalWrite(PIN_Z_DC_IN2, LOW);
        // Serial.println("DEBUG: Z motor stopped.");
    }
    else {
        Serial.println(
            "ERROR: Coding error. Invalid z_motor_direction_t value.");
    }
}

void set_start_button_led(bool state) {
    digitalWrite(PIN_GENERAL_PWR_EN_3, state);
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
        Serial.print(", CLAW_UP_BTN: ");
        Serial.print(get_switch_state(CLAW_UP_BTN));
        Serial.print(", CLAW_DOWN_BTN: ");
        Serial.print(get_switch_state(CLAW_DOWN_BTN));
        Serial.print(", CLAW_GRAB_BTN: ");
        Serial.print(get_switch_state(CLAW_GRAB_BTN));
        Serial.print(", START_BTN: ");
        Serial.print(get_switch_state(START_BTN));
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
        if (get_switch_state(CLAW_UP_BTN)) {
            Serial.print("CLAW_UP_BTN ");
        }
        if (get_switch_state(CLAW_DOWN_BTN)) {
            Serial.print("CLAW_DOWN_BTN ");
        }
        if (get_switch_state(CLAW_GRAB_BTN)) {
            Serial.print("CLAW_GRAB_BTN ");
        }
        if (get_switch_state(START_BTN)) {
            Serial.print("START_BTN ");
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

void init_steppers()
{
    Serial.println("DEBUG: init_steppers()");
    init_stepper(x1Stepper, PIN_X1_NEN, PIN_X1_STEP, PIN_X1_DIR, true);
    init_stepper(x2Stepper, PIN_X2_NEN, PIN_X2_STEP, PIN_X2_DIR, false);
    init_stepper(yStepper, PIN_Y_NEN, PIN_Y_STEP, PIN_Y_DIR, true);

    xSteppers = MultiStepper();
    xSteppers.addStepper(x1Stepper);
    xSteppers.addStepper(x2Stepper);
}

void loop_moveMotorsBasedOnButtons()
{
    // X is North/South, Y is East/West
    bool northButton = get_switch_state(STICK_NORTH);
    bool southButton = get_switch_state(STICK_SOUTH);
    bool eastButton  = get_switch_state(STICK_EAST);
    bool westButton  = get_switch_state(STICK_WEST);
    
    // Positive X direction
    if (northButton) {
        if (prevXdir != 1) {  // direction change
            Serial.println("Moving claw North");
            prevXdir = 1;
            // x1Stepper.moveTo(stepperMaxPos);
            xSteppers.moveTo(stepperMaxMulti);

            // x1Stepper.setSpeed(stepper_speed);
            // x2Stepper.setSpeed(stepper_speed);
        }
        xSteppers.run();
        // x1Stepper.runSpeedToPosition();
        // xSteppers.runSpeedToPosition();
    }
    // Negative X direction
    else if (southButton) {
        if (prevXdir != -1) {  // direction change
            Serial.println("Moving claw South");
            prevXdir = -1;
            xSteppers.moveTo(stepperMinMulti);

            // x1Stepper.setSpeed(-stepper_speed);
            // x2Stepper.setSpeed(-stepper_speed);
        }
        xSteppers.run();
        // xSteppers.runSpeedToPosition();
    }

    // Positive Y direction
    if (eastButton) {
        if (prevYdir != 1) {  // direction change
            Serial.println("Moving claw East");
            prevYdir = 1;
            // yStepper.moveTo(stepperMaxPos);
            yStepper.setSpeed(stepper_speed);
        }
        // yStepper.run();
        // yStepper.runSpeedToPosition();
        yStepper.runSpeed();
    }
    // Negative Y direction
    else if (westButton == HIGH) {
        if (prevYdir != -1) {  // direction change
            Serial.println("Moving claw West");
            prevYdir = -1;
            // yStepper.moveTo(stepperMinPos);
            yStepper.setSpeed(-stepper_speed);
        }
        // yStepper.run();
        // yStepper.runSpeedToPosition();
        yStepper.runSpeed();
    }

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
    if (upButton) {
        // Serial.println("INFO: Moving claw up");
        z_motor_direction = Z_MOTOR_DIRECTION_RAISE;
    }
    else if (downButton) {
        // Serial.println("INFO: Moving claw down");
        z_motor_direction = Z_MOTOR_DIRECTION_DROP;
    }
    set_z_motor_state(z_motor_direction);

    // special case to make the claw stop better, when it transitions from down to stop
    if (z_motor_direction == Z_MOTOR_DIRECTION_STOP && prev_z_motor_direction == Z_MOTOR_DIRECTION_DROP) {
        set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
        delay(40);
        set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
    }
    prev_z_motor_direction = z_motor_direction;

    // do claw
    if (get_switch_state(CLAW_GRAB_BTN)) {
        set_claw_state(CLAW_ENGAGE);
    }
    else {
        set_claw_state(CLAW_RELEASE);
    }

}

void i2c_scan() {
    Serial.println("INFO: I2C scan starting...");
    I2CScanner scanner;

	scanner.Init();
	scanner.Scan();
    Serial.println("INFO: I2C scan complete.");
}

void display_duration_sec(uint32_t duration_sec) {
    // Display the duration on the screen as mm:ss
    
    uint32_t sec = duration_sec % 60;
    uint32_t min = duration_sec / 60;

    display.setBlink(0);
    display.displayTime(min, sec, true, true);  // args: colon=true, leading_zero=true
    display.displayColon(1);
}

void display_duration_ms(uint32_t duration_ms) {
    // Display the duration on the screen as ss.xx, where xx is the hundredths of a second.

    float dur_sec = duration_ms / 1000.0;
    display.setBlink(0);
    display.displayColon(0);
    display.displayFloat(dur_sec, 1); // 1 decimal place
}

void display_blinking_zeros() {
    display.displayColon(0);
    display.displayInt(0);
    display.setBlink(1);
}

void display_int(uint16_t int_val) {
    display.displayColon(0);
    display.displayInt(int_val);
    display.setBlink(0);
}

void display_scrolling_press_start(uint32_t idle_start_time_ms) {
    display.setBlink(0);
    display.displayColon(0);

    const int msg_len = 14;
    int offset = ((millis() - idle_start_time_ms) / 500) % (msg_len);

    const uint8_t SEG_PRESS_START[] = {
        SEG_A | SEG_B | SEG_F | SEG_E | SEG_G,           // P
        SEG_E | SEG_G,                                   // r
        SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,           // E
        SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,           // S
        SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,           // S
        0,
        SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,           // S
        SEG_F | SEG_E | SEG_G | SEG_D,                   // t
        SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,   // A
        SEG_E | SEG_G,                                   // r
        SEG_F | SEG_E | SEG_G | SEG_D,                   // t
        0,0,0
    };
    uint8_t seg1_data[] = {SEG_D, SEG_D, SEG_D, SEG_D};
    // uint8_t seg2_data[] = {SEG_D, SEG_D, SEG_D, SEG_D};

    for (int i = 0; i < 4; i++) {
        seg1_data[i] = SEG_PRESS_START[(i+offset)%msg_len];
        // seg2_data[i] = SEG_PRESS_START[(i+offset+5)%msg_len];
    }

    display.displayRaw(seg1_data);
}
