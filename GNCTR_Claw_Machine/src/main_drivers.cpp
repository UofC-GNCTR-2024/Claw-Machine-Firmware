
#include "main.h"
#include <AccelStepper.h>
#include <MultiStepper.h>
#include "I2CScanner.h"
#include <HT16K33.h>
#include "game_config.h"

uint32_t global_last_user_input_time_ms = millis(); // global extern

AccelStepper x1Stepper, x2Stepper, yStepper;
MultiStepper xSteppers, xxySteppers;

HT16K33 display(I2C_SCREEN_ADDR);

const float stepper_speed = STEPPER_MICROSTEPS * 16000;
const float max_stepper_speed = STEPPER_MICROSTEPS * 17000;

long yStepperMinPos = -yAxisLength; // updated during homing
long yStepperMaxPos = 80000; // updated during homing

// for X1 and X2, both values should be the same
// when X1 and X2 are zeroed, min is set to 0
long stepperMinMulti[] = {-80000, -80000}; // updated during homing
long xStepperMaxMulti[] = {80000, 80000}; // updated during homing

unsigned long prevClawTransTime = 0;
const unsigned long clawToggleDebounceTime = 150;
bool currentlyTogglingClaw = false;
claw_mode_t clawState = CLAW_RELEASE;

// either -1 or +1 to indicate the previous direction in each axis
int8_t prevXdir = 0, prevYdir = 0;

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
    clawState = state;
    if (state == CLAW_ENGAGE) {
        digitalWrite(PIN_CLAW_EN, HIGH);
        // Serial.println("DEBUG: Claw enabled.");
    }
    else if (state == CLAW_RELEASE) {
        digitalWrite(PIN_CLAW_EN, LOW);
        // Serial.println("DEBUG: Claw disabled.");
    }
}

void toggle_claw_state()
{
    if (clawState == CLAW_RELEASE) {
        set_claw_state(CLAW_ENGAGE);
        set_start_button_led(true);
    }
    else {
        set_claw_state(CLAW_RELEASE);
        set_start_button_led(false);
    }

    debug_print_axes_info();
}

void debug_print_axes_info() {
    // debug print the current stepper positions (useful for configuring the limits/homing)
    Serial.print("DEBUG: x1Stepper=");
    Serial.print(x1Stepper.currentPosition());
    Serial.print(", x2Stepper=");
    Serial.print(x2Stepper.currentPosition());
    Serial.print(", yStepper=");
    Serial.print(yStepper.currentPosition());
    Serial.print(", xMin=");
    Serial.print(stepperMinMulti[0]);
    Serial.print(", xMax=");
    Serial.print(xStepperMaxMulti[0]);
    Serial.print(", yMin=");
    Serial.print(yStepperMinPos);
    Serial.print(", yMax=");
    Serial.print(yStepperMaxPos);
    Serial.println();
}

/* True=Triggered, False=Not Triggered. */
bool get_switch_state(limit_switch_t limit_switch)
{
    bool sw_state;
    switch (limit_switch) {
        case LIMIT_X1:
            sw_state = digitalRead(PIN_LIMIT_X1);
            break;
        case LIMIT_X2:
            sw_state = digitalRead(PIN_LIMIT_X2);
            break;
        case LIMIT_Y:
            sw_state = digitalRead(PIN_LIMIT_Y);
            break;
        case CLAW_UP_BTN:
            sw_state = digitalRead(PIN_INPUT_BTN_1);
            break;
        case CLAW_DOWN_BTN:
            sw_state = digitalRead(PIN_INPUT_BTN_2);
            break;
        case CLAW_GRAB_BTN:
            sw_state = digitalRead(PIN_INPUT_BTN_3);
            break;
        case START_BTN:
            sw_state = digitalRead(PIN_AUX_SW_1);
            break;
        case AUX_SW_2:
            sw_state = digitalRead(PIN_AUX_SW_2);
            break;
        case STICK_NORTH:
            sw_state = digitalRead(PIN_STICK_NORTH_BTN);
            break;
        case STICK_EAST:
            sw_state = digitalRead(PIN_STICK_EAST_BTN);
            break;
        case STICK_SOUTH:
            sw_state = digitalRead(PIN_STICK_SOUTH_BTN);
            break;
        case STICK_WEST:
            sw_state = digitalRead(PIN_STICK_WEST_BTN);
            break;
        default:
            Serial.println("ERROR: Coding error. Invalid limit_switch_t value.");
            return false;
    }
    if (sw_state) {
        global_last_user_input_time_ms = millis();
    }
    return sw_state;
}

void set_z_motor_state(z_motor_direction_t direction)
{
    if (direction == Z_MOTOR_DIRECTION_DROP) {
        digitalWrite(PIN_Z_DC_EN_A, HIGH);
        digitalWrite(PIN_Z_DC_IN1, HIGH);
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

bool run_z_motor_for_duration_and_watch_start(z_motor_direction_t direction, uint32_t duration_ms)
{
    // runs the z motor for the specified duration, and watches for the start button
    // does the little brake at the end if lowering

    set_z_motor_state(direction);

    uint32_t start_time = millis();
    
    while (millis() - start_time < duration_ms) {
        if (get_switch_state(START_BTN)) {
            // Serial.println("DEBUG: Start button pressed during z motor run.");

            if (direction == Z_MOTOR_DIRECTION_DROP) {
                // do the little brake
                set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
                delay(Z_BRAKE_DURATION_MS);
            }
            set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
            set_start_button_led(false);
            return true;
        }

        // flash the start button
        loop_update_start_button_blinking();
        display_scrolling_press_start();

        delay(20);
    }

    if (direction == Z_MOTOR_DIRECTION_DROP) {
        // do the little brake
        set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
        delay(Z_BRAKE_DURATION_MS);
    }
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
    // set_start_button_led(false);
    return false;
}


void loop_update_start_button_blinking() {
    // set button blinking
    bool target_state = (millis() % (start_btn_led_blink_rate_ms * 2) < start_btn_led_blink_rate_ms);
    set_start_button_led(target_state);
}

void set_start_button_led(bool state) {
    digitalWrite(PIN_GENERAL_PWR_EN_3, state);
}

void set_enclosure_led(bool state) {
    digitalWrite(PIN_GENERAL_PWR_EN_1, state);
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

    stepper.setMaxSpeed(max_stepper_speed);
    stepper.setAcceleration(STEPPER_MICROSTEPS * 5);
    // stepper.setSpeed(500*16);
    stepper.enableOutputs();

    x1Stepper.setSpeed(stepper_speed);
    x2Stepper.setSpeed(stepper_speed);
    yStepper.setSpeed(stepper_speed);

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

    xxySteppers = MultiStepper();
    xxySteppers.addStepper(x1Stepper);
    xxySteppers.addStepper(x2Stepper);
    xxySteppers.addStepper(yStepper);

    // disable on boot
    set_stepper_enable(false);
}

void home_x_axis()
{
    Serial.println("INFO: Starting home_x_axis()");
    set_stepper_enable(1);

    // set direction as positive
    x1Stepper.setSpeed(stepper_speed);
    x2Stepper.setSpeed(stepper_speed);

    // move the claw to the left until it hits the limit switch
    while (!get_switch_state(LIMIT_X1) && !get_switch_state(LIMIT_X2)) {
        x1Stepper.runSpeed();
        x2Stepper.runSpeed();
    }

    x1Stepper.setCurrentPosition(xAxisLength);
    x2Stepper.setCurrentPosition(xAxisLength);
    
    // set the limits
    stepperMinMulti[0] = 0;
    stepperMinMulti[1] = 0;
    xStepperMaxMulti[0] = xAxisLength;
    xStepperMaxMulti[1] = xAxisLength;

    // for beauty, move back a bit from the limit
    delay(150);
    long xPos[] = {xAxisLength-1500, xAxisLength-1500}; // 1500 is about a cm
    xSteppers.moveTo(xPos);
    xSteppers.runSpeedToPosition();

    Serial.println("INFO: X axis homed.");
}

void home_y_axis() {
    Serial.println("INFO: Starting home_y_axis()");
    set_stepper_enable(1);

    // move the claw to the left for a while (relative-ish move)
    yStepper.setCurrentPosition(0);
    yStepper.moveTo(-yAxisLength);
    yStepper.setSpeed(-stepper_speed);
    while (yStepper.runSpeedToPosition());

    yStepper.setCurrentPosition(0);
    // set the limits
    yStepperMinPos = 0;
    yStepperMaxPos = yAxisLength;

    // for beauty, move back a bit from the limit
    delay(150);
    yStepper.moveTo(1500);
    yStepper.setSpeed(stepper_speed);
    while (yStepper.runSpeedToPosition());

    Serial.println("INFO: Y axis homed.");
}

void home_z_motor(uint16_t max_up_duration_ms) {
    // wait for claw to raise all the way, and then drop it a touch
    set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
    delay(max_up_duration_ms); // TODO: make it only raise a little bit, if it's aware of how far down it is
    set_z_motor_state(Z_MOTOR_DIRECTION_DROP);
    delay(Z_BRAKE_DURATION_MS);
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
}

void endgame_move_to_bin() {
    // moves axes over the bin; blocking
    // also zeros the Y axis

    Serial.println("INFO: Starting endgame_move_to_bin()");

    // must run after each full game
    prevXdir = 0;
    prevYdir = 0;

    // move the claw to the bin
    debug_print_axes_info();
    move_claw_to_absolute_xy(0, 0);

    delay(400);

    // drop the claw a touch for fun
    set_z_motor_state(Z_MOTOR_DIRECTION_DROP);
    delay(500);
    set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
    delay(Z_BRAKE_DURATION_MS);
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);


    // release the claw
    set_claw_state(CLAW_RELEASE);
    delay(500);

    // move the claw back up
    home_z_motor(2000);

    // move to middle
    move_claw_to_absolute_xy(xAxisLength/2, yAxisLength/2);
}

void move_claw_to_absolute_xy(long x, long y) {
    // moves the claw to the specified x and y positions
    // blocking
    // system should be homed

    Serial.print("DEBUG: move_claw_to_absolute_xy(x=");
    Serial.print(x);
    Serial.print(", y=");
    Serial.print(y);
    Serial.println(")");

    long xxyPos[] = {x, x, y};
    xxySteppers.moveTo(xxyPos);
    xxySteppers.runSpeedToPosition();
}

bool move_to_absolute_xy_and_watch_for_start_press(long x, long y) {
    // moves the claw to the specified x and y positions
    // blocking
    // system should be homed

    // if start is pressed, returns true
    // if start is not pressed, returns false

    long xxyPos[] = {x, x, y};
    xxySteppers.moveTo(xxyPos);

    uint32_t last_check_time = millis();
    while (1) {
        bool all_done = false; // this tracking is actually probably not necessary
        for (uint16_t i = 0; i < 10; i++) {
            if (!xxySteppers.run()) {
                all_done = true;
                break;
            }
        }
        if (all_done) {
            break;
        }

        if (millis() - last_check_time > 50) { // period of start button check
            last_check_time = millis();
            if (get_switch_state(START_BTN)) {
                set_start_button_led(false);
                return true;
            }
        }

        // flash the start button
        loop_update_start_button_blinking();
        display_scrolling_press_start();
    }
    // set_start_button_led(false);
    return false;
}

void run_homing_showoff() {
    // pointless thing because it's only 8am and Aleks is still an hour away

    move_claw_to_absolute_xy(xAxisLength*0.8, yAxisLength*0.5);

    home_x_axis();

    // move_claw_to_absolute_xy(xAxisLength*0.4, 0);
    // move_claw_to_absolute_xy(xAxisLength*0.6, 0);
    // move_claw_to_absolute_xy(xAxisLength*0.4, 0);
    // move_claw_to_absolute_xy(xAxisLength*0.6, 0);

    // grab
    set_claw_state(CLAW_ENGAGE);
    delay(250);
    set_claw_state(CLAW_RELEASE);

}

void easter_egg_gnctr_theme()
{
    // G-N-CTR, GNCTR

    #define MELODY_Z '1'
    #define MELODY_REST '0'
    
    Serial.println("INFO: Starting easter_egg_gnctr_theme()");

    home_z_motor(3000);

    char melody[] = "X00 x00 1011 0000 1010 1011 0000 x00";
    for (uint8_t rep = 0; rep < 4; rep++) {
        for (uint16_t i = 0; i < strlen(melody); i++) {
            char c = melody[i];
            if (c == '0') {
                set_enclosure_led(false);
                delay(melody_slot_duration);
            }
            else if (c == '1') {
                set_enclosure_led(true);
                set_z_motor_state(Z_MOTOR_DIRECTION_DROP);
                delay(melody_tone_duration);

                // brake
                set_enclosure_led(false);
                set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
                delay(Z_BRAKE_DURATION_MS);

                // rest between notes
                set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
                int16_t remaining_dur_ms = melody_slot_duration-melody_tone_duration-Z_BRAKE_DURATION_MS;
                if (remaining_dur_ms > 0) {
                    delay(remaining_dur_ms);
                }
            }
            else if (c == 'x') { // 2 counts up
                set_enclosure_led(true);

                set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);
                delay(melody_slot_duration + melody_tone_duration);
                set_z_motor_state(Z_MOTOR_DIRECTION_STOP);

            }
            else if (c == 'X') { // 2 counts down
                set_enclosure_led(true);

                set_z_motor_state(Z_MOTOR_DIRECTION_DROP);
                delay(melody_slot_duration + melody_tone_duration);
                set_z_motor_state(Z_MOTOR_DIRECTION_STOP);

            }
            else if (c == ' ') {
                ; // ignore, just a separator
            }
            else {
                Serial.print("ERROR: Invalid character in melody: '");
                Serial.print(c);
                Serial.println("'");
            }
        }
    }

    home_z_motor(3000);
}

void loop_moveMotorsBasedOnButtons()
{
    // X is North/South, Y is East/West
    bool northButton = get_switch_state(STICK_NORTH);
    bool southButton = get_switch_state(STICK_SOUTH);
    bool eastButton  = get_switch_state(STICK_EAST);
    bool westButton  = get_switch_state(STICK_WEST);

    int8_t xDir = 0, yDir = 0;
    long newXPos, newYPos;

    // if no buttons pressed, early exit
    if (!northButton && !southButton && !eastButton && !westButton) {
        return;
    }

    // populate xDir
    if (northButton) {
        xDir = 1;
        newXPos = xAxisLength;
    }
    else if (southButton) {
        xDir = -1;
        newXPos = 0;
    }
    else {
        xDir = 0;
        newXPos = x1Stepper.currentPosition();
    }

    // populate yDir
    if (eastButton) {
        yDir = 1;
        newYPos = yAxisLength;
    }
    else if (westButton) {
        yDir = -1;
        newYPos = 0;
    }
    else {
        yDir = 0;
        newYPos = yStepper.currentPosition();
    }

    // if there's a direction change this time at all
    if (prevXdir != xDir || prevYdir != yDir) {
        Serial.print("DEBUG: Claw direction change. xDir=");
        Serial.print(xDir);
        Serial.print(", yDir=");
        Serial.print(yDir);
        Serial.println();
        prevXdir = xDir;
        prevYdir = yDir;

        // update the target places
        long xxyNewPos[] = {newXPos, newXPos, newYPos}; // placeholders
        xxySteppers.moveTo(xxyNewPos);
    }

    // run the steppers
    for (uint16_t i = 0; i < 50; i++) {
        xxySteppers.run();
    }
    // xxySteppers.run();
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
        // Serial.println("INFO: Moving claw up");
        z_motor_direction = Z_MOTOR_DIRECTION_RAISE;
    }
    else if (downButton) {
        // Serial.println("INFO: Moving claw down");
        // TODO: add a maximum net duration that the claw can move down for
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

    // Debounce the claw grab button
    if (get_switch_state(CLAW_GRAB_BTN)) {
        // enough time has passed since the last transition
        
        if (!currentlyTogglingClaw && millis() >= prevClawTransTime + clawToggleDebounceTime) {
            // Serial.println("DEBUG: Claw grab button pressed.");
            currentlyTogglingClaw = true;
            prevClawTransTime = millis();
            toggle_claw_state();
        }
    }
    else {
        if (currentlyTogglingClaw && millis() >= prevClawTransTime + clawToggleDebounceTime) {
            // Serial.println("DEBUG: Claw grab button released.");
            currentlyTogglingClaw = false;
            prevClawTransTime = millis();
        }
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
    display.displayColon(1);
    display.displayInt(0);
    display.setBlink(1);
}

void display_int(uint16_t int_val) {
    display.displayColon(0);
    display.displayInt(int_val);
    display.setBlink(0);
}

void display_int_no_leading_zeros(uint16_t int_val) {
    display.displayColon(0);
    display.setDigits(0);
    display.displayInt(int_val);
    display.setBlink(0);

    // reset to "normal"
    display.setDigits(4); // enable leading zeros for reliability
}

void display_raw_message(uint8_t *message) {
    // message should be 4 bytes long (array)
    display.displayRaw(message);
}

void display_scrolling_press_start() {
    // displays "Press Start" scrolling across the screen

    display.setBlink(0);
    display.displayColon(0);

    const int msg_len = 14;
    int offset = (millis() / start_btn_led_blink_rate_ms) % (msg_len);

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

    display_raw_message(seg1_data);
}

void set_stepper_enable(bool enable) {
    if (enable) {
        x1Stepper.enableOutputs();
        x2Stepper.enableOutputs();
        yStepper.enableOutputs();
    }
    else {
        x1Stepper.disableOutputs();
        x2Stepper.disableOutputs();
        yStepper.disableOutputs();
    }
}
