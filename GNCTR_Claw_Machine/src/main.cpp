#include "main.h"
#include "states.h"
#include <Arduino.h>

game_state_t game_state = GAME_STATE_IDLE;

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
        case GAME_STATE_IDLE:
            game_state = idle_state(game_state);
            break;
        case GAME_STATE_PLAY:
            game_state = play_state(game_state);
            break;
        case GAME_STATE_RESET:
            game_state = reset_state(game_state);
            break;
    }
}
