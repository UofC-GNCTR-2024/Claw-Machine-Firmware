#include "main.h"
#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>


AccelStepper x1Stepper, x2Stepper, yStepper;
MultiStepper xSteppers;

const float stepper_speed = STEPPER_MICROSTEPS * 8000;
const long stepperMinPos = -80000;
const long stepperMaxPos = 80000;
long stepperMinMulti[] = {stepperMinPos, stepperMinPos};
long stepperMaxMulti[] = {stepperMaxPos, stepperMaxPos};

// either -1 or +1 to indicate the previous direction in each axis
int prevXdir = 0, prevYdir = 0;

// X is North/South, Y is East/West
int northButton, southButton, eastButton, westButton;

void setup()
{
    Serial.begin(115200);
    Serial.println("DEBUG: Start of setup() ===============================");
    init_pin_modes();

    init_stepper(x1Stepper, PIN_X1_NEN, PIN_X1_STEP, PIN_X1_DIR, true);
    init_stepper(x2Stepper, PIN_X2_NEN, PIN_X2_STEP, PIN_X2_DIR, false);
    init_stepper(yStepper, PIN_Y_NEN, PIN_Y_STEP, PIN_Y_DIR, false);

    xSteppers = MultiStepper();
    xSteppers.addStepper(x1Stepper);
    xSteppers.addStepper(x2Stepper);
}


void loop()
{
    northButton = digitalRead(PIN_STICK_NORTH_BTN);
    southButton = digitalRead(PIN_STICK_SOUTH_BTN);
    eastButton  = digitalRead(PIN_STICK_EAST_BTN);
    westButton  = digitalRead(PIN_STICK_WEST_BTN);
    
    // Positive X direction
    if (northButton == HIGH) {
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
    else if (southButton == HIGH) {
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
    if (eastButton == HIGH) {
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
