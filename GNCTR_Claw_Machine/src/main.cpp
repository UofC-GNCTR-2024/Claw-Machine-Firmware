#include "main.h"
#include "states.h"
#include <Arduino.h>

game_state_t prev_game_state = GAME_STATE_IDLE; 
game_state_t game_state = GAME_STATE_IDLE; 

void setup()
{
    Serial.begin(115200);
    Serial.println("DEBUG: Start of setup() ===============================");
    init_pin_modes();
    init_steppers();
    init_display();

    i2c_scan();

    Serial.println("DEBUG: End of setup() ===============================");
}

void loop()
{
    game_state_t game_state_at_start_of_loop = game_state;
    switch (game_state) {
        case GAME_STATE_IDLE:
            game_state = idle_state(prev_game_state);
            break;
        case GAME_STATE_PLAY:
            game_state = play_state(prev_game_state);
            break;
        case GAME_STATE_RESET:
            game_state = reset_state(prev_game_state);
            break;
    }
    prev_game_state = game_state_at_start_of_loop;
}
