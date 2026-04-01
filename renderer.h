#ifndef RENDERER_H
#define RENDERER_H

#include <allegro.h>
#include "types.h"
#include "resources.h"

void renderer_build_button_stack(const Layout *layout, int item_count, float start_y_ratio, RectF *out_rects);

void renderer_draw_menu(BITMAP *buffer,
                        const Assets *assets,
                        const Layout *layout,
                        const char *title,
                        const char *subtitle,
                        const char *const *items,
                        int item_count,
                        int selected,
                        int mouse_x,
                        int mouse_y);

void renderer_draw_rules(BITMAP *buffer, const Assets *assets, const Layout *layout);
void renderer_draw_text_input(BITMAP *buffer,
                              const Assets *assets,
                              const Layout *layout,
                              const char *title,
                              const char *prompt,
                              const char *value,
                              const char *hint,
                              const char *message);

void renderer_draw_game(BITMAP *buffer, const Assets *assets, const GameState *game);

void renderer_draw_stage_end(BITMAP *buffer,
                             const Assets *assets,
                             const GameState *game,
                             const char *const *items,
                             int item_count,
                             int selected,
                             int mouse_x,
                             int mouse_y);

void renderer_draw_highscores(BITMAP *buffer,
                              const Assets *assets,
                              const Layout *layout,
                              const ScoreRecord *scores,
                              int score_count);

void renderer_draw_completion(BITMAP *buffer,
                              const Assets *assets,
                              const GameState *game,
                              float animation_clock);

#endif
