#include "main.h"
#include <Arduino.h>

void setup()
{
    init_pin_modes();

    Serial.begin(115200);
}

void loop()
{
    Serial.println("DEBUG: Start of loop() ===============================");

    digitalWrite(PIN_ONBOARD_LED, HIGH);
    delay(200);
    digitalWrite(PIN_ONBOARD_LED, LOW);
    delay(200);

    debug_print_all_limit_switch_states();

    set_claw_state(true);
    digitalWrite(PIN_GENERAL_PWR_EN_1, HIGH);
    set_z_motor_state(Z_MOTOR_DIRECTION_DROP);
    digitalWrite(PIN_ONBOARD_LED, HIGH);
    delay(2500);

    set_claw_state(false);
    digitalWrite(PIN_GENERAL_PWR_EN_1, LOW);
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
    digitalWrite(PIN_ONBOARD_LED, LOW);
    delay(2500);
}
