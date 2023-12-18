#include "states.h"
#include "main_drivers.h"


game_state_t idle_state(game_state_t prev)
{
    if (prev != IDLE) {
        // TODO display a welcome message
        Serial.println("Idle state");
        return IDLE;
    }
    
    if (get_switch_state(START_BTN))  // start button
        return PLAY;
    
    return IDLE;
}

game_state_t play_state(game_state_t prev)
{
    if (prev != PLAY) {
        Serial.println("Play state");
        ; // TODO screen message
    }
    loop_moveMotorsBasedOnButtons();
    loop_dropOrRaiseClaw();
    return PLAY;
}

// This function blocks
game_state_t reset_state(game_state_t prev)
{
    // TODO "game over" message
    // TODO release the claw

    // Use blocking functions here?
    // TODO raise the claw
    // TODO return the claw to the home position

    return IDLE;
}
