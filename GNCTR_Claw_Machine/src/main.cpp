#include "main.h"
#include "states.h"
#include <Arduino.h>

game_state_t game_state = IDLE;

void setup()
{
    Serial.begin(115200);
    Serial.println("DEBUG: Start of setup() ===============================");
    init_pin_modes();
    init_steppers();
}

void loop()
{
    switch (game_state) {
        case IDLE:
            game_state = idle_state(game_state);
            break;
        case PLAY:
            game_state = play_state(game_state);
            break;
        case RESET:
            game_state = reset_state(game_state);
            break;
    }
}
