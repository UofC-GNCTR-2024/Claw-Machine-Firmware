#include "states.h"
#include "main_drivers.h"

uint16_t start_btn_led_blink_rate_ms = 400; // half-period

bool start_btn_led_state = false;
uint32_t start_btn_led_last_toggle_time_ms = 0;

game_state_t idle_state(game_state_t prev)
{
    if (prev != GAME_STATE_IDLE) {
        // TODO display a welcome message
        Serial.println("Starting GAME_STATE_IDLE state");
    }

    // flash the START_BTN LED
    if (millis() - start_btn_led_last_toggle_time_ms > start_btn_led_blink_rate_ms) {
        start_btn_led_state = !start_btn_led_state;
        set_start_button_led(start_btn_led_state);
        start_btn_led_last_toggle_time_ms = millis();
    }

    // next-state logic
    if (get_switch_state(START_BTN)) {
        return GAME_STATE_PLAY;
    }
    
    return GAME_STATE_IDLE;
}

game_state_t play_state(game_state_t prev)
{

    if (prev != GAME_STATE_PLAY) {
        Serial.println("Starting GAME_STATE_PLAY state");
        set_start_button_led(false);
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
