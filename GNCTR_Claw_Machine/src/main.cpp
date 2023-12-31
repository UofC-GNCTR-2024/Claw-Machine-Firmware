#include "main.h"
#include "states.h"
#include <Arduino.h>

game_state_t prev_game_state = GAME_STATE_BOOT; 
game_state_t game_state = GAME_STATE_BOOT; 

void setup()
{
    Serial.begin(115200);
    Serial.println("DEBUG: Start of setup() ===============================");
    init_pin_modes();
    init_steppers();
    init_display();

    i2c_scan();

    // DEBUG: print all limit switch states
    // while (1) {
    //     debug_print_all_limit_switch_states();
    // }

    Serial.println("DEBUG: End of setup() ===============================");
}

void loop()
{
    game_state_t game_state_at_start_of_loop = game_state;
    switch (game_state) {
        case GAME_STATE_BOOT:
            // easter_egg_gnctr_theme();  // FIXME move this
            game_state = GAME_STATE_HOMING; // THIS IS THE MAIN DEPLOYMENT ONE
            // game_state = GAME_STATE_PLAY; // shortcut for dev
            break;
        case GAME_STATE_HOMING:
            game_state = homing_state(prev_game_state);
            break;
        case GAME_STATE_IDLE:
            game_state = idle_state(prev_game_state);
            break;
        case GAME_STATE_DEMO:
            game_state = demo_state(prev_game_state);
            break;
        case GAME_STATE_GNCTR_EASTER_EGG:
            game_state = gnctr_easter_egg_state(prev_game_state);
            break;
        case GAME_STATE_PLAY:
            game_state = play_state(prev_game_state);
            break;
        case GAME_STATE_RESET:
            game_state = reset_state(prev_game_state);
            break;
        default:
            Serial.println("ERROR: Invalid game state");
            break;
    }
    prev_game_state = game_state_at_start_of_loop;
}
