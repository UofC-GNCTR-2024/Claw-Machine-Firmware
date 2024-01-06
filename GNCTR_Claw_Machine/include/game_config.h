
#ifndef __INCLUDE_GUARD_GAME_CONFIG_H_CLAW_MACHINE__
#define __INCLUDE_GUARD_GAME_CONFIG_H_CLAW_MACHINE__

#include <stdint.h>

const uint16_t start_btn_led_blink_rate_ms = 400; // half-period
const uint16_t game_play_max_time_sec = 120; // FIXME: set to 45 for deployment
const uint32_t idle_time_to_start_demo_sec = 69; // FIXME: set to about a minute for deployment


// 1 in n chance of this being at the start of your play, or 0 to disable, or 1 to force
const uint8_t approx_homing_showoff_chance = 0; // this is lame anyway


#endif // __INCLUDE_GUARD_GAME_CONFIG_H_CLAW_MACHINE__
