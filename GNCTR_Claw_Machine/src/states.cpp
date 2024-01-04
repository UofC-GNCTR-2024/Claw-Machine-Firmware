#include "states.h"
#include "main_drivers.h"

uint16_t start_btn_led_blink_rate_ms = 400; // half-period
uint16_t game_play_max_time_sec = 30;

// persistent vars for GAME_STATE_IDLE
bool start_btn_led_state = false;
uint32_t start_btn_led_last_toggle_time_ms = 0;
uint32_t idle_start_time_ms = 0;

// persistent vars for GAME_STATE_PLAY
// uint32_t play_state_start_time_ms = 0;

// persistent vars for GAME_STATE_RESET


game_state_t idle_state(game_state_t prev)
{
    if (prev != GAME_STATE_IDLE) {
        Serial.println("Starting GAME_STATE_IDLE state");
        idle_start_time_ms = millis();
    }

    // display scrolling text: "Press Start" (non-blocking)
    display_scrolling_press_start(idle_start_time_ms);

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

// play_state blocks
game_state_t play_state(game_state_t prev)
{
    if (prev != GAME_STATE_PLAY) {
        Serial.println("Starting GAME_STATE_PLAY state");
        set_start_button_led(false);
    }
    else {
        Serial.println("Continuing GAME_STATE_PLAY state (should not happen)");
    }

    // do a countdown
    set_start_button_led(false);
    for (uint16_t i = 3333; i >= 1111; i -= 1111) {
        display_int(i);
        delay(900);
    }
    set_start_button_led(true);

    uint32_t play_state_start_time_ms = millis();
    uint32_t last_play_state_tenthsec = 0;
    while (millis() - play_state_start_time_ms < (game_play_max_time_sec*1000)) {
        
        // display the time
        uint32_t play_time_remaining_ms = (game_play_max_time_sec*1000) - (millis() - play_state_start_time_ms);
        uint32_t play_time_remaining_tenthsec = play_time_remaining_ms/100;

        if (play_time_remaining_tenthsec != last_play_state_tenthsec) {
            last_play_state_tenthsec = play_time_remaining_tenthsec;
            
            if (play_time_remaining_ms > 10000) {
                display_duration_sec(play_time_remaining_ms/1000);
            }
            else {
                display_duration_ms(play_time_remaining_ms);
            }
        }

        // respond to button presses
        for (int i = 0; i < 10; i++) {
            loop_moveMotorsBasedOnButtons();
        }
        loop_moveMotorsBasedOnButtons();
        loop_dropOrRaiseClaw();
    }

    // TODO: early exit if the claw is dropped over the target

    Serial.println("INFO: game over");

    // set known state
    set_start_button_led(false);
    set_claw_state(CLAW_RELEASE);
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);

    display_blinking_zeros();
    delay(2000); // wait a sec showing that it's over
    
    return GAME_STATE_RESET;
}

// reset_state blocks
game_state_t reset_state(game_state_t prev)
{
    if (prev != GAME_STATE_RESET) {
        Serial.println("Starting GAME_STATE_RESET state");
        ; // TODO screen message
    }

    // TODO "game over" message
    // TODO release the claw

    // Use blocking functions here?
    // TODO raise the claw
    // TODO return the claw to the home position

    return GAME_STATE_IDLE;
}
