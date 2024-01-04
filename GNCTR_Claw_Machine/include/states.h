
#ifndef __INCLUDE_GUARD_STATES_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_STATES_H_CLAW_MACHINE__

typedef enum {
    GAME_STATE_IDLE,  // Waiting for someone to press start
    GAME_STATE_PLAY,  // Controlling the claw
    GAME_STATE_RESET  // Return the claw to the home position
} game_state_t;

typedef enum {
    RAISE,
    HOME
} reset_substate_t;


game_state_t idle_state(game_state_t prev);

game_state_t play_state(game_state_t prev);

game_state_t reset_state(game_state_t prev);

#endif
