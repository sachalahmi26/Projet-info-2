#include <stdio.h>
#include <math.h>
#include <string.h>
#include <allegro.h>

#include "renderer.h"
#include "logic.h"
#include "input.h"

static int color_rgb(int r, int g, int b) {
    return makecol(r, g, b);
}

static RectF local_player_rect(const Player *player) {
    RectF r;
    r.x = player->pos.x - player->w * 0.5f;
    r.y = player->pos.y - player->h * 0.5f;
    r.w = player->w;
    r.h = player->h;
    return r;
}

static void draw_background(BITMAP *buffer, BITMAP *background, const Layout *layout, int fallback_color) {
    clear_to_color(buffer, fallback_color);
    if (background != NULL) {
        stretch_blit(background, buffer,
                     0, 0, background->w, background->h,
                     0, 0, layout->screen_w, layout->screen_h);
    }
}

static void draw_panel(BITMAP *buffer, RectF panel_rect, int fill_color, int outline_color) {
    rectfill(buffer,
             (int)panel_rect.x,
             (int)panel_rect.y,
             (int)(panel_rect.x + panel_rect.w),
             (int)(panel_rect.y + panel_rect.h),
             fill_color);

    rect(buffer,
         (int)panel_rect.x,
         (int)panel_rect.y,
         (int)(panel_rect.x + panel_rect.w),
         (int)(panel_rect.y + panel_rect.h),
         outline_color);
}

static void draw_centered_label(BITMAP *buffer, const char *text, RectF rect, int color) {
    int x = (int)(rect.x + rect.w * 0.5f);
    int y = (int)(rect.y + rect.h * 0.5f - text_height(font) * 0.5f);
    textprintf_centre_ex(buffer, font, x, y, color, -1, "%s", text);
}

void renderer_build_button_stack(const Layout *layout, int item_count, float start_y_ratio, RectF *out_rects) {
    float button_w = layout->screen_w * 0.30f;
    float button_h = layout->screen_h * 0.075f;
    float gap = layout->screen_h * 0.018f;
    float start_y = layout->screen_h * start_y_ratio;
    int i;

    for (i = 0; i < item_count; ++i) {
        out_rects[i].x = layout->screen_w * 0.5f - button_w * 0.5f;
        out_rects[i].y = start_y + i * (button_h + gap);
        out_rects[i].w = button_w;
        out_rects[i].h = button_h;
    }
}

static void draw_button_stack(BITMAP *buffer,
                              const Layout *layout,
                              const char *const *items,
                              int item_count,
                              int selected,
                              int mouse_x,
                              int mouse_y,
                              float start_y_ratio) {
    RectF rects[12];
    int i;

    renderer_build_button_stack(layout, item_count, start_y_ratio, rects);

    for (i = 0; i < item_count; ++i) {
        int hovered = input_point_in_rect(mouse_x, mouse_y, rects[i]);
        int fill = color_rgb(25, 32, 70);
        int outline = color_rgb(180, 200, 255);
        int text_col = color_rgb(230, 238, 255);

        if (i == selected) {
            fill = color_rgb(55, 90, 180);
            outline = color_rgb(255, 255, 255);
        } else if (hovered) {
            fill = color_rgb(40, 64, 120);
        }

        draw_panel(buffer, rects[i], fill, outline);
        draw_centered_label(buffer, items[i], rects[i], text_col);
    }
}

void renderer_draw_menu(BITMAP *buffer,
                        const Assets *assets,
                        const Layout *layout,
                        const char *title,
                        const char *subtitle,
                        const char *const *items,
                        int item_count,
                        int selected,
                        int mouse_x,
                        int mouse_y) {
    int title_y;
    int subtitle_y;

    draw_background(buffer, assets->menu_bg, layout, color_rgb(9, 12, 22));

    title_y = (int)(layout->screen_h * 0.11f);
    subtitle_y = (int)(layout->screen_h * 0.18f);

    textprintf_centre_ex(buffer, font, layout->screen_w / 2, title_y, color_rgb(255, 245, 180), -1, "%s", title);
    textprintf_centre_ex(buffer, font, layout->screen_w / 2, subtitle_y, color_rgb(210, 220, 255), -1, "%s", subtitle);

    draw_button_stack(buffer, layout, items, item_count, selected, mouse_x, mouse_y, 0.29f);

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(layout->screen_h * 0.93f),
                         color_rgb(170, 180, 210),
                         -1,
                         "Clavier: fleches + Entree | Souris: clic gauche");
}

void renderer_draw_rules(BITMAP *buffer, const Assets *assets, const Layout *layout) {
    const char *lines[] = {
        "But: eliminer toutes les bulles avant la fin du temps sans etre touche.",
        "Le joueur se deplace horizontalement en bas de l'ecran.",
        "Tir simple au depart: un seul projectile actif en meme temps.",
        "A partir du niveau 2, certains bonus d'arme tombent au sol.",
        "Niveau 3 et 4: certaines bulles lancent des eclairs verticaux.",
        "Fin du niveau 4: combat contre le boss final.",
        "Echec si une bulle, un eclair ou le boss vous touche,",
        "ou si le temps est ecoule.",
        "Sauvegarde disponible entre les niveaux.",
        "Retour: Echap ou clic sur le bouton."
    };
    int i;
    RectF panel;

    draw_background(buffer, assets->menu_bg, layout, color_rgb(12, 14, 28));

    panel.x = layout->screen_w * 0.10f;
    panel.y = layout->screen_h * 0.12f;
    panel.w = layout->screen_w * 0.80f;
    panel.h = layout->screen_h * 0.72f;

    draw_panel(buffer, panel, color_rgb(18, 24, 52), color_rgb(190, 210, 255));

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(panel.y + 18),
                         color_rgb(255, 242, 180),
                         -1,
                         "REGLES DU JEU");

    for (i = 0; i < 10; ++i) {
        textprintf_ex(buffer,
                      font,
                      (int)(panel.x + 28),
                      (int)(panel.y + 58 + i * 34),
                      color_rgb(235, 240, 255),
                      -1,
                      "%s",
                      lines[i]);
    }

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(layout->screen_h * 0.90f),
                         color_rgb(190, 210, 255),
                         -1,
                         "Echap pour revenir au menu");
}

void renderer_draw_text_input(BITMAP *buffer,
                              const Assets *assets,
                              const Layout *layout,
                              const char *title,
                              const char *prompt,
                              const char *value,
                              const char *hint,
                              const char *message) {
    RectF panel;
    RectF field;

    draw_background(buffer, assets->menu_bg, layout, color_rgb(8, 10, 20));

    panel.x = layout->screen_w * 0.18f;
    panel.y = layout->screen_h * 0.20f;
    panel.w = layout->screen_w * 0.64f;
    panel.h = layout->screen_h * 0.42f;

    field.x = panel.x + panel.w * 0.08f;
    field.y = panel.y + panel.h * 0.45f;
    field.w = panel.w * 0.84f;
    field.h = panel.h * 0.18f;

    draw_panel(buffer, panel, color_rgb(20, 26, 54), color_rgb(190, 210, 255));
    draw_panel(buffer, field, color_rgb(10, 14, 28), color_rgb(255, 255, 255));

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(panel.y + 24),
                         color_rgb(255, 242, 180),
                         -1,
                         "%s",
                         title);

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(panel.y + 80),
                         color_rgb(220, 230, 255),
                         -1,
                         "%s",
                         prompt);

    textprintf_ex(buffer,
                  font,
                  (int)(field.x + 16),
                  (int)(field.y + field.h * 0.5f - text_height(font) * 0.5f),
                  color_rgb(255, 255, 255),
                  -1,
                  "%s_",
                  value);

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(panel.y + panel.h - 48),
                         color_rgb(190, 210, 255),
                         -1,
                         "%s",
                         hint);

    if (message != NULL && message[0] != '\0') {
        textprintf_centre_ex(buffer,
                             font,
                             layout->screen_w / 2,
                             (int)(panel.y + panel.h + 28),
                             color_rgb(255, 190, 190),
                             -1,
                             "%s",
                             message);
    }
}

static void draw_obstacles(BITMAP *buffer, const GameState *game) {
    int i;
    for (i = 0; i < game->obstacle_count; ++i) {
        const RectF r = game->obstacles[i].rect;
        draw_panel(buffer, r, color_rgb(60, 80, 120), color_rgb(210, 220, 255));
    }
}

static void draw_player(BITMAP *buffer, const Assets *assets, const GameState *game) {
    RectF pr = local_player_rect(&game->player);
    if (assets->player != NULL) {
        stretch_sprite(buffer, assets->player, (int)pr.x, (int)pr.y, (int)pr.w, (int)pr.h);
    } else {
        rectfill(buffer, (int)pr.x, (int)pr.y, (int)(pr.x + pr.w), (int)(pr.y + pr.h), color_rgb(80, 200, 255));
        rect(buffer, (int)pr.x, (int)pr.y, (int)(pr.x + pr.w), (int)(pr.y + pr.h), color_rgb(255, 255, 255));
    }
}

static void draw_bubbles(BITMAP *buffer, const Assets *assets, const GameState *game) {
    const Bubble *bubble = game->bubbles;
    while (bubble != NULL) {
        BITMAP *sprite = NULL;
        int color = color_rgb(230, 70, 70);
        int r = (int)bubble->radius;

        if (bubble->tier >= 0 && bubble->tier < 4) {
            sprite = assets->bubbles[bubble->tier];
        }

        if (bubble->tier == 2) color = color_rgb(235, 95, 95);
        if (bubble->tier == 1) color = color_rgb(245, 130, 130);
        if (bubble->tier == 0) color = color_rgb(255, 180, 180);

        if (sprite != NULL) {
            stretch_sprite(buffer, sprite,
                           (int)(bubble->pos.x - bubble->radius),
                           (int)(bubble->pos.y - bubble->radius),
                           2 * r,
                           2 * r);
        } else {
            circlefill(buffer, (int)bubble->pos.x, (int)bubble->pos.y, r, color);
            circle(buffer, (int)bubble->pos.x, (int)bubble->pos.y, r, color_rgb(255, 255, 255));
        }

        bubble = bubble->next;
    }
}

static void draw_projectiles(BITMAP *buffer, const Assets *assets, const GameState *game) {
    const Projectile *p = game->projectiles;
    while (p != NULL) {
        if (assets->projectile != NULL) {
            stretch_sprite(buffer,
                           assets->projectile,
                           (int)(p->pos.x - 4),
                           (int)(p->pos.y - 18),
                           8,
                           24);
        } else {
            line(buffer,
                 (int)p->pos.x,
                 (int)p->pos.y,
                 (int)p->pos.x,
                 (int)(p->pos.y - 18),
                 color_rgb(255, 255, 90));
        }
        p = p->next;
    }
}

static void draw_bonuses(BITMAP *buffer, const Assets *assets, const GameState *game) {
    const Bonus *bonus = game->bonuses;
    while (bonus != NULL) {
        BITMAP *sprite = NULL;
        int fill = color_rgb(100, 240, 120);

        if (bonus->type == BONUS_RAPID) {
            sprite = assets->bonus_rapid;
            fill = color_rgb(255, 180, 0);
        } else if (bonus->type == BONUS_TRIPLE) {
            sprite = assets->bonus_triple;
            fill = color_rgb(110, 210, 255);
        } else if (bonus->type == BONUS_EXPLOSIVE) {
            sprite = assets->bonus_explosive;
            fill = color_rgb(255, 90, 70);
        }

        if (sprite != NULL) {
            stretch_sprite(buffer,
                           sprite,
                           (int)(bonus->pos.x - bonus->size * 0.5f),
                           (int)(bonus->pos.y - bonus->size * 0.5f),
                           (int)bonus->size,
                           (int)bonus->size);
        } else {
            rectfill(buffer,
                     (int)(bonus->pos.x - bonus->size * 0.5f),
                     (int)(bonus->pos.y - bonus->size * 0.5f),
                     (int)(bonus->pos.x + bonus->size * 0.5f),
                     (int)(bonus->pos.y + bonus->size * 0.5f),
                     fill);
            rect(buffer,
                 (int)(bonus->pos.x - bonus->size * 0.5f),
                 (int)(bonus->pos.y - bonus->size * 0.5f),
                 (int)(bonus->pos.x + bonus->size * 0.5f),
                 (int)(bonus->pos.y + bonus->size * 0.5f),
                 color_rgb(255, 255, 255));
        }
        bonus = bonus->next;
    }
}

static void draw_lightnings(BITMAP *buffer, const Assets *assets, const GameState *game) {
    const Lightning *l = game->lightnings;
    while (l != NULL) {
        if (assets->lightning != NULL) {
            stretch_sprite(buffer,
                           assets->lightning,
                           (int)(l->pos.x - l->width * 0.5f),
                           (int)l->pos.y,
                           (int)l->width,
                           (int)l->length);
        } else {
            vline(buffer,
                  (int)l->pos.x,
                  (int)l->pos.y,
                  (int)(l->pos.y + l->length),
                  color_rgb(255, 255, 0));
            vline(buffer,
                  (int)l->pos.x + 2,
                  (int)l->pos.y,
                  (int)(l->pos.y + l->length),
                  color_rgb(255, 255, 255));
        }
        l = l->next;
    }
}

static void draw_effects(BITMAP *buffer, const GameState *game) {
    const Effect *e = game->effects;
    while (e != NULL) {
        float phase = 1.0f - (e->timer / e->duration);
        int radius = (int)(e->radius * (0.35f + phase));
        int color = (e->type == EFFECT_EXPLOSION) ? color_rgb(255, 140, 50) : color_rgb(255, 255, 255);
        circle(buffer, (int)e->pos.x, (int)e->pos.y, radius, color);
        e = e->next;
    }
}

static void draw_boss(BITMAP *buffer, const Assets *assets, const GameState *game) {
    RectF br;
    int hp_w;
    int hp_fill;

    if (!game->boss.active) {
        return;
    }

    br.x = game->boss.pos.x - game->boss.w * 0.5f;
    br.y = game->boss.pos.y - game->boss.h * 0.5f;
    br.w = game->boss.w;
    br.h = game->boss.h;

    if (assets->boss != NULL) {
        stretch_sprite(buffer, assets->boss, (int)br.x, (int)br.y, (int)br.w, (int)br.h);
    } else {
        rectfill(buffer, (int)br.x, (int)br.y, (int)(br.x + br.w), (int)(br.y + br.h), color_rgb(120, 240, 140));
        rect(buffer, (int)br.x, (int)br.y, (int)(br.x + br.w), (int)(br.y + br.h), color_rgb(255, 255, 255));
    }

    hp_w = (int)(br.w * ((float)game->boss.hp / (float)game->boss.max_hp));
    hp_fill = color_rgb(255, 60, 60);

    rectfill(buffer,
             (int)br.x,
             (int)(br.y - 18),
             (int)(br.x + br.w),
             (int)(br.y - 10),
             color_rgb(60, 20, 20));
    rectfill(buffer,
             (int)br.x,
             (int)(br.y - 18),
             (int)(br.x + hp_w),
             (int)(br.y - 10),
             hp_fill);
    rect(buffer,
         (int)br.x,
         (int)(br.y - 18),
         (int)(br.x + br.w),
         (int)(br.y - 10),
         color_rgb(255, 255, 255));
}

static void draw_info_zone(BITMAP *buffer, const GameState *game) {
    RectF info = game->layout.info_zone;
    draw_panel(buffer, info, color_rgb(10, 14, 34), color_rgb(190, 210, 255));

    textprintf_ex(buffer, font, (int)(info.x + 16), (int)(info.y + 14), color_rgb(255, 245, 180), -1, "Pseudo : %s", game->player.pseudo);
    textprintf_ex(buffer, font, (int)(info.x + 16), (int)(info.y + 38), color_rgb(220, 235, 255), -1, "Stage : %s", stage_display_name(game->current_stage));

    textprintf_ex(buffer,
                  font,
                  (int)(info.x + info.w * 0.42f),
                  (int)(info.y + 14),
                  color_rgb(230, 255, 220),
                  -1,
                  "Score : %ld",
                  game->score);
    textprintf_ex(buffer,
                  font,
                  (int)(info.x + info.w * 0.42f),
                  (int)(info.y + 38),
                  color_rgb(230, 255, 220),
                  -1,
                  "Arme : %s",
                  weapon_name(game->player.weapon));

    textprintf_ex(buffer,
                  font,
                  (int)(info.x + info.w * 0.77f),
                  (int)(info.y + 14),
                  color_rgb(255, 220, 220),
                  -1,
                  "Temps : %d",
                  (int)ceil(game->time_left));

    if (game->countdown > 0.0f) {
        textprintf_ex(buffer,
                      font,
                      (int)(info.x + info.w * 0.77f),
                      (int)(info.y + 38),
                      color_rgb(255, 230, 160),
                      -1,
                      "Depart : %d",
                      (int)ceil(game->countdown));
    } else if (game->player.weapon_timer > 0.0f) {
        textprintf_ex(buffer,
                      font,
                      (int)(info.x + info.w * 0.77f),
                      (int)(info.y + 38),
                      color_rgb(255, 230, 160),
                      -1,
                      "Bonus : %.1fs",
                      game->player.weapon_timer);
    } else {
        textprintf_ex(buffer,
                      font,
                      (int)(info.x + info.w * 0.72f),
                      (int)(info.y + 38),
                      color_rgb(180, 190, 220),
                      -1,
                      "Simple / 1 tir");
    }
}

void renderer_draw_game(BITMAP *buffer, const Assets *assets, const GameState *game) {
    BITMAP *bg = assets->backgrounds[game->current_stage];

    draw_background(buffer, bg, &game->layout, color_rgb(14, 18, 40));
    draw_info_zone(buffer, game);
    draw_obstacles(buffer, game);
    draw_bubbles(buffer, assets, game);
    draw_bonuses(buffer, assets, game);
    draw_lightnings(buffer, assets, game);
    draw_projectiles(buffer, assets, game);
    draw_effects(buffer, game);
    draw_boss(buffer, assets, game);
    draw_player(buffer, assets, game);

    if (game->countdown > 0.0f) {
        textprintf_centre_ex(buffer,
                             font,
                             game->layout.screen_w / 2,
                             (int)(game->layout.game_zone.y + game->layout.game_zone.h * 0.32f),
                             color_rgb(255, 245, 180),
                             -1,
                             "%d",
                             (int)ceil(game->countdown));
    }

    if (game->status_message[0] != '\0') {
        textprintf_centre_ex(buffer,
                             font,
                             game->layout.screen_w / 2,
                             (int)(game->layout.game_zone.y + 18),
                             color_rgb(255, 250, 210),
                             -1,
                             "%s",
                             game->status_message);
    }
}

void renderer_draw_stage_end(BITMAP *buffer,
                             const Assets *assets,
                             const GameState *game,
                             const char *const *items,
                             int item_count,
                             int selected,
                             int mouse_x,
                             int mouse_y) {
    RectF panel;
    draw_background(buffer, assets->menu_bg, &game->layout, color_rgb(10, 10, 20));

    panel.x = game->layout.screen_w * 0.18f;
    panel.y = game->layout.screen_h * 0.16f;
    panel.w = game->layout.screen_w * 0.64f;
    panel.h = game->layout.screen_h * 0.56f;

    draw_panel(buffer, panel, color_rgb(18, 24, 52), color_rgb(190, 210, 255));

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(panel.y + 24),
                         (game->status == GAME_STATUS_WON) ? color_rgb(190, 255, 190) : color_rgb(255, 180, 180),
                         -1,
                         "%s",
                         (game->status == GAME_STATUS_WON) ? "NIVEAU TERMINE" : "NIVEAU PERDU");

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(panel.y + 64),
                         color_rgb(240, 244, 255),
                         -1,
                         "%s",
                         game->status_message);

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(panel.y + 102),
                         color_rgb(230, 235, 255),
                         -1,
                         "Score courant : %ld",
                         game->score);

    if (game->stage_bonus_score > 0) {
        textprintf_centre_ex(buffer,
                             font,
                             game->layout.screen_w / 2,
                             (int)(panel.y + 126),
                             color_rgb(255, 238, 180),
                             -1,
                             "Bonus de temps : %ld",
                             game->stage_bonus_score);
    }

    draw_button_stack(buffer, &game->layout, items, item_count, selected, mouse_x, mouse_y, 0.43f);
}

void renderer_draw_highscores(BITMAP *buffer,
                              const Assets *assets,
                              const Layout *layout,
                              const ScoreRecord *scores,
                              int score_count) {
    RectF panel;
    RectF table_rect;
    int line_h;
    int i;

    draw_background(buffer, assets->menu_bg, layout, color_rgb(7, 9, 18));

    panel.x = layout->screen_w * 0.12f;
    panel.y = layout->screen_h * 0.09f;
    panel.w = layout->screen_w * 0.76f;
    panel.h = layout->screen_h * 0.80f;

    table_rect.x = panel.x + panel.w * 0.05f;
    table_rect.y = panel.y + panel.h * 0.16f;
    table_rect.w = panel.w * 0.90f;
    table_rect.h = panel.h * 0.70f;

    draw_panel(buffer, panel, color_rgb(18, 24, 52), color_rgb(190, 210, 255));
    draw_panel(buffer, table_rect, color_rgb(10, 14, 34), color_rgb(130, 165, 230));

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(panel.y + panel.h * 0.05f),
                         color_rgb(255, 242, 180),
                         -1,
                         "CLASSEMENT");

    textprintf_ex(buffer,
                  font,
                  (int)(table_rect.x + table_rect.w * 0.04f),
                  (int)(table_rect.y + 14),
                  color_rgb(190, 210, 255),
                  -1,
                  "RANG");
    textprintf_ex(buffer,
                  font,
                  (int)(table_rect.x + table_rect.w * 0.23f),
                  (int)(table_rect.y + 14),
                  color_rgb(190, 210, 255),
                  -1,
                  "JOUEUR");
    textprintf_right_ex(buffer,
                        font,
                        (int)(table_rect.x + table_rect.w * 0.96f),
                        (int)(table_rect.y + 14),
                        color_rgb(190, 210, 255),
                        -1,
                        "SCORE");
    hline(buffer,
          (int)(table_rect.x + 10),
          (int)(table_rect.y + 34),
          (int)(table_rect.x + table_rect.w - 10),
          color_rgb(120, 150, 220));

    if (score_count <= 0) {
        textprintf_centre_ex(buffer,
                             font,
                             layout->screen_w / 2,
                             (int)(panel.y + panel.h * 0.5f),
                             color_rgb(220, 230, 255),
                             -1,
                             "Aucun score enregistre.");
    } else {
        line_h = (int)((table_rect.h - 46.0f) / (float)MAX_HIGHSCORES);
        if (line_h < 28) {
            line_h = 28;
        }
        for (i = 0; i < score_count; ++i) {
            int y = (int)(table_rect.y + 44 + i * line_h);
            int text_color = (i < 3) ? color_rgb(255, 240, 170) : color_rgb(230, 235, 255);

            if ((i % 2) == 0) {
                rectfill(buffer,
                         (int)(table_rect.x + 8),
                         y - 2,
                         (int)(table_rect.x + table_rect.w - 8),
                         y + line_h - 6,
                         color_rgb(16, 22, 46));
            }

            textprintf_ex(buffer,
                          font,
                          (int)(table_rect.x + table_rect.w * 0.04f),
                          y,
                          text_color,
                          -1,
                          "%2d",
                          i + 1);
            textprintf_ex(buffer,
                          font,
                          (int)(table_rect.x + table_rect.w * 0.23f),
                          y,
                          text_color,
                          -1,
                          "%-18s",
                          scores[i].pseudo);
            textprintf_right_ex(buffer,
                                font,
                                (int)(table_rect.x + table_rect.w * 0.96f),
                                y,
                                text_color,
                                -1,
                                "%ld",
                                scores[i].score);
        }
    }

    textprintf_centre_ex(buffer,
                         font,
                         layout->screen_w / 2,
                         (int)(panel.y + panel.h * 0.94f),
                         color_rgb(190, 210, 255),
                         -1,
                         "Echap pour revenir");
}

void renderer_draw_completion(BITMAP *buffer,
                              const Assets *assets,
                              const GameState *game,
                              float animation_clock) {
    int i;
    BITMAP *bg = assets->victory;
    draw_background(buffer, bg, &game->layout, color_rgb(6, 10, 24));

    for (i = 0; i < 16; ++i) {
        float angle = animation_clock * 1.5f + i * 0.39f;
        int x = (int)(game->layout.screen_w * 0.5f + cos(angle) * game->layout.screen_w * 0.23f);
        int y = (int)(game->layout.screen_h * 0.38f + sin(angle * 1.3f) * game->layout.screen_h * 0.16f);
        circlefill(buffer, x, y, 4 + (i % 3), color_rgb(255, 220, 90));
    }

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(game->layout.screen_h * 0.22f),
                         color_rgb(255, 245, 180),
                         -1,
                         "VICTOIRE FINALE");

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(game->layout.screen_h * 0.30f),
                         color_rgb(230, 240, 255),
                         -1,
                         "Bravo %s, le boss est vaincu.",
                         game->player.pseudo);

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(game->layout.screen_h * 0.36f),
                         color_rgb(230, 240, 255),
                         -1,
                         "Score final : %ld",
                         game->score);

    textprintf_centre_ex(buffer,
                         font,
                         game->layout.screen_w / 2,
                         (int)(game->layout.screen_h * 0.84f),
                         color_rgb(190, 210, 255),
                         -1,
                         "Entree ou clic gauche pour revenir au menu");
}
