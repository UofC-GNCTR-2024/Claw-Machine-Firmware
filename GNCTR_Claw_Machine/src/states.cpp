#include "states.h"
#include "main_drivers.h"


game_state_t idle_state(game_state_t prev)
{
    if (prev != GAME_STATE_IDLE) {
        // TODO display a welcome message
        Serial.println("Starting GAME_STATE_IDLE state");
        return GAME_STATE_IDLE;
    }
    
    if (get_switch_state(START_BTN))  // start button
        return GAME_STATE_PLAY;
    
    return GAME_STATE_IDLE;
}

game_state_t play_state(game_state_t prev)
{
    if (prev != GAME_STATE_PLAY) {
        Serial.println("Starting GAME_STATE_PLAY state");
        ; // TODO screen message
    }
    loop_moveMotorsBasedOnButtons();
    loop_dropOrRaiseClaw();
    return GAME_STATE_PLAY;
}

// This function blocks
game_state_t reset_state(game_state_t prev)
{
    if (prev != GAME_STATE_IDLE) {
        Serial.println("Starting GAME_STATE_IDLE state");
        ; // TODO screen message
    }

    // TODO "game over" message
    // TODO release the claw

    // Use blocking functions here?
    // TODO raise the claw
    // TODO return the claw to the home position

    return GAME_STATE_IDLE;
}
