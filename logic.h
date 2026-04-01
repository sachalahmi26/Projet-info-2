#ifndef LOGIC_H
#define LOGIC_H

#include <stddef.h>
#include "types.h"

void layout_compute(Layout *layout, int screen_w, int screen_h);

void game_init(GameState *game, const char *pseudo, int start_stage, int screen_w, int screen_h);
void game_reset_stage(GameState *game, int stage);
void game_destroy(GameState *game);
void game_update(GameState *game, const InputState *input, float dt);

const char *weapon_name(WeaponType weapon);
const char *bonus_name(BonusType bonus);
const char *stage_display_name(int stage);

int game_stage_is_boss(int stage);
int game_next_stage_after_win(const GameState *game);
int game_should_show_end_menu(const GameState *game);
int game_player_active_projectiles(const GameState *game);

#endif
