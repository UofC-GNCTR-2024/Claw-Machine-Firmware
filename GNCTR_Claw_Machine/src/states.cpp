#include "states.h"
#include "main_drivers.h"

uint16_t start_btn_led_blink_rate_ms = 400; // half-period
uint16_t game_play_max_time_sec = 10; // FIXME: set to 45 for deployment

// persistent vars for GAME_STATE_IDLE
bool start_btn_led_state = false;
uint32_t start_btn_led_last_toggle_time_ms = 0;
uint32_t idle_start_time_ms = 0;

// persistent vars for GAME_STATE_PLAY
// uint32_t play_state_start_time_ms = 0;

// persistent vars for GAME_STATE_RESET


game_state_t homing_state(game_state_t prev)
{
    if (prev != GAME_STATE_HOMING) {
        Serial.println("Starting GAME_STATE_HOMING state");
        set_start_button_led(false);
        set_claw_state(CLAW_RELEASE);
    }
    
    Serial.println("Press the start button to do the x-axis homing.");
    while (!get_switch_state(START_BTN)) {
        delay(25); // lol save power
    }

    // write the homing message to the LCD
    uint8_t seg_zero_message_for_display[] = {
        SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,  // Z
        SEG_A | SEG_F | SEG_E | SEG_G | SEG_D,  // E
        SEG_E | SEG_G,                          // r
        // SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F, // 0
        SEG_C | SEG_D | SEG_E | SEG_G, // 0
    };
    //display_int(0); // hmm
    display_raw_message(seg_zero_message_for_display);

    Serial.println("Starting x-axis homing because it's past the start button waiting.");
    home_x_axis();

    return GAME_STATE_IDLE;
}

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

    // set the motors idle
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);
    set_stepper_enable(0);

    // set lights on
    set_enclosure_led(true);
    
    // do a semi-sentient "clip-clip" with the claw like a crab
    uint32_t millis_mod_10sec = millis() % 10000;
    if (millis_mod_10sec < 200) {
        set_claw_state(CLAW_ENGAGE);
    }
    else if (millis_mod_10sec < 400) {
        set_claw_state(CLAW_RELEASE);
    }
    else if (millis_mod_10sec < 600) {
        set_claw_state(CLAW_ENGAGE);
    }
    else if (millis_mod_10sec < 800) {
        set_claw_state(CLAW_RELEASE);
    }
    else if (millis_mod_10sec < 1000) {
        set_claw_state(CLAW_ENGAGE);
    }
    else {
        set_claw_state(CLAW_RELEASE);
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
        set_stepper_enable(1);
    }
    else {
        Serial.println("Continuing GAME_STATE_PLAY state (should not happen)");
    }

    set_start_button_led(false);

    // do a countdown (4444, 3333, 2222, 1111) [LAME]
    // for (uint16_t i = 3333; i >= 1111; i -= 1111) {
    //     display_int(i);
    //     delay(600);
    // }

    // do a countdown: 4321 removing the numbers
    uint16_t countdown_nums[] = {4321, 321, 21, 1};
    for (uint16_t i = 0; i < 4; i++) {
        display_int_no_leading_zeros(countdown_nums[i]);
        delay(700);
    }

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

    Serial.println("INFO: game over");
    
    return GAME_STATE_RESET;
}

// reset_state blocks
game_state_t reset_state(game_state_t prev)
{
    if (prev != GAME_STATE_RESET) {
        Serial.println("Starting GAME_STATE_RESET state");
    }

    display_blinking_zeros();
    // TODO: "game over" message, maybe

    // set known state
    set_start_button_led(false);
    set_claw_state(CLAW_ENGAGE);
    delay(250); // wait for claw to close before lifting
    set_z_motor_state(Z_MOTOR_DIRECTION_RAISE);

    // wait for claw to raise all the way, and then drop it a touch
    delay(1000); // TODO: make it only raise a little bit, if it's aware of how far down it is
    set_z_motor_state(Z_MOTOR_DIRECTION_DROP);
    delay(50);
    set_z_motor_state(Z_MOTOR_DIRECTION_STOP);

    // move claw over bin in front
    endgame_move_to_bin();

    set_claw_state(CLAW_RELEASE);

    delay(1000); // wait a sec showing that it's over

    return GAME_STATE_IDLE;
}
