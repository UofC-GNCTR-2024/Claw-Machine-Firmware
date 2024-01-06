#include "states.h"
#include "main_drivers.h"

// persistent vars for GAME_STATE_IDLE
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

    Serial.println("Starting xy-axis homing because it's past the start button waiting.");
    home_z_motor(3000);
    home_x_axis();
    home_y_axis();

    Serial.println("INFO: Homing complete. Move to center.");
    // move to middle
    move_claw_to_absolute_xy(xAxisLength/2, yAxisLength/2);

    return GAME_STATE_IDLE;
}

game_state_t idle_state(game_state_t prev)
{
    if (prev != GAME_STATE_IDLE) {
        Serial.println("Starting GAME_STATE_IDLE state");
        idle_start_time_ms = millis();
    }

    // display scrolling text: "Press Start" (non-blocking)
    display_scrolling_press_start();

    // flash the START_BTN LED
    loop_update_start_button_blinking();

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
    else if (get_switch_state(CLAW_GRAB_BTN)) { // cheat code to start demo
        return GAME_STATE_DEMO;
    }

    // start the demo if it's been idle for a while
    if (millis() - idle_start_time_ms > (idle_time_to_start_demo_sec * 1000)) {
        Serial.println("INFO: Starting demo because it's been idle for a while");
        return GAME_STATE_DEMO;
    }
    
    return GAME_STATE_IDLE;
}


game_state_t demo_state(game_state_t prev) {
    if (prev != GAME_STATE_DEMO) {
        Serial.println("Starting GAME_STATE_DEMO state");
        set_start_button_led(false);
        set_stepper_enable(1);
    }
    else {
        Serial.println("Continuing GAME_STATE_DEMO state (should not happen)");
    }

    // assume claw is z-homed
    // assume claw is in the middle
    // ensure claw is released
    set_claw_state(CLAW_RELEASE);

    set_start_button_led(false);

    for (uint8_t i = 0; i < 5; i++) { // number of times to play itself
        long claw_down_dist_ms = random(2000, 3500);

        Serial.print("INFO: demo iteration ");
        Serial.print(i);
        Serial.print(", claw_down_dist_ms = ");
        Serial.print(claw_down_dist_ms);
        Serial.println();

        // move to random location
        if (move_to_absolute_xy_and_watch_for_start_press(random(0, xAxisLength*0.65), random(0, yAxisLength*0.65))) {
            finish_demo();
            return GAME_STATE_PLAY;
        }
        delay(150);

        // move down
        if (run_z_motor_for_duration_and_watch_start(Z_MOTOR_DIRECTION_DROP, claw_down_dist_ms)) {
            finish_demo();
            return GAME_STATE_PLAY;
        }

        // grab
        set_claw_state(CLAW_ENGAGE);
        delay(random(10, 800));

        // move up
        if (run_z_motor_for_duration_and_watch_start(Z_MOTOR_DIRECTION_RAISE, claw_down_dist_ms * Z_UP_TO_DOWN_RATIO)) {
            finish_demo();
            return GAME_STATE_PLAY;
        }

        // move to stash
        if (move_to_absolute_xy_and_watch_for_start_press(xAxisLength*0.9, yAxisLength*0.9)) {
            finish_demo();
            return GAME_STATE_PLAY;
        }
        delay(150);

        // move down (bow down to stash)
        if (run_z_motor_for_duration_and_watch_start(Z_MOTOR_DIRECTION_DROP, 500)) {
            finish_demo();
            return GAME_STATE_PLAY;
        }

        // release to stash
        set_claw_state(CLAW_RELEASE);
        delay(400);

        // move back up
        //home_z_motor(2000);
        if (run_z_motor_for_duration_and_watch_start(Z_MOTOR_DIRECTION_RAISE, 500*Z_UP_TO_DOWN_RATIO)) {
            finish_demo();
            return GAME_STATE_PLAY;
        }
    }

    Serial.println("INFO: demo over");

    // release claw
    finish_demo();
    
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

    if (approx_homing_showoff_chance > 0 && (millis() % approx_homing_showoff_chance) == 0) {
        Serial.println("INFO: doing homing showoff, by random chance");
        run_homing_showoff();
    }

    // do a countdown (4444, 3333, 2222, 1111) [LAME]
    // for (uint16_t i = 3333; i >= 1111; i -= 1111) {
    //     display_int(i);
    //     delay(600);
    // }

    // do a countdown: 4321 removing the numbers
    set_enclosure_led(false);
    uint16_t countdown_nums[] = {4321, 321, 21, 1};
    for (uint16_t i = 0; i < 4; i++) {
        display_int_no_leading_zeros(countdown_nums[i]);
        
        delay(500);
        set_enclosure_led(true);
        delay(200);
        set_enclosure_led(false);

    }
    set_enclosure_led(true);

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

    home_z_motor(2000);

    // move claw over bin in front
    endgame_move_to_bin();

    set_claw_state(CLAW_RELEASE);

    delay(1000); // wait a sec showing that it's over

    return GAME_STATE_IDLE;
}

void finish_demo() {
    Serial.println("INFO: finish_demo()");
    
    // release claw
    set_claw_state(CLAW_RELEASE);

    // move claw up
    home_z_motor(2000);

    // move claw to middle
    move_claw_to_absolute_xy(xAxisLength/2, yAxisLength/2);

    // pause before it starts the countdown to play (in case it's playing next)
    delay(750);
}

