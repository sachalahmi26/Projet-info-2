#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#include <allegro.h>

#include "config.h"
#include "types.h"
#include "logic.h"
#include "input.h"
#include "persistence.h"
#include "resources.h"
#include "renderer.h"

typedef enum {
    SCREEN_MENU = 0,
    SCREEN_RULES,
    SCREEN_INPUT_NEW,
    SCREEN_INPUT_LOAD,
    SCREEN_PLAYING,
    SCREEN_STAGE_END,
    SCREEN_HIGHSCORES,
    SCREEN_COMPLETION
} AppScreen;

static void init_allegro_system(void) {
    allegro_init();
    install_keyboard();
    install_mouse();
    install_timer();
    set_uformat(U_ASCII);
    set_color_depth(desktop_color_depth());

    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, WINDOW_W, WINDOW_H, 0, 0) != 0) {
        allegro_message("Impossible d'ouvrir le mode graphique.");
        allegro_exit();
        exit(EXIT_FAILURE);
    }
}

static void reset_text_buffer(char *buffer) {
    buffer[0] = '\0';
}

static void append_typed_chars(char *buffer, const InputState *input) {
    int len = (int)strlen(buffer);
    int i;

    for (i = 0; i < input->typed_count; ++i) {
        char c = input->typed_chars[i];
        if (len >= MAX_PSEUDO_LEN) {
            break;
        }
        if (isalnum((unsigned char)c) || c == '_' || c == '-' || c == ' ') {
            if (c != ';') {
                buffer[len++] = c;
                buffer[len] = '\0';
            }
        }
    }

    if (input->backspace_pressed || input->delete_pressed) {
        if (len > 0) {
            buffer[len - 1] = '\0';
        }
    }
}

static int hover_selected(const Layout *layout,
                          int item_count,
                          float start_y_ratio,
                          int mouse_x,
                          int mouse_y,
                          int current_selected) {
    RectF rects[12];
    int i;

    renderer_build_button_stack(layout, item_count, start_y_ratio, rects);
    for (i = 0; i < item_count; ++i) {
        if (input_point_in_rect(mouse_x, mouse_y, rects[i])) {
            return i;
        }
    }
    return current_selected;
}

static int click_on_button(const Layout *layout,
                           int item_count,
                           float start_y_ratio,
                           int mouse_x,
                           int mouse_y) {
    RectF rects[12];
    int i;

    renderer_build_button_stack(layout, item_count, start_y_ratio, rects);
    for (i = 0; i < item_count; ++i) {
        if (input_point_in_rect(mouse_x, mouse_y, rects[i])) {
            return i;
        }
    }
    return -1;
}

static int clamp_stage(int stage) {
    if (stage < 1) return 1;
    if (stage > BOSS_STAGE) return BOSS_STAGE;
    return stage;
}

int main(void) {
    BITMAP *buffer;
    Assets assets;
    Layout menu_layout;
    InputState input;
    GameState game;
    ScoreRecord highscores[MAX_HIGHSCORES];
    AppScreen current_screen = SCREEN_MENU;

    const char *menu_items[] = {
        "Lire les regles",
        "Nouvelle partie",
        "Reprendre une partie",
        "Classement",
        "Quitter"
    };

    const char *end_items[] = {
        "Quitter",
        "Sauvegarder",
        "Continuer",
        "Menu"
    };

    int menu_selected = 1;
    int end_selected = 2;
    int highscore_count = 0;
    int completion_recorded = 0;

    char pseudo_buffer[MAX_PSEUDO_LEN + 1];
    char ui_message[MAX_MESSAGE_LEN + 1];

    double previous_time;
    double animation_clock = 0.0;

    init_allegro_system();
    input_init();
    resources_load(&assets);
    layout_compute(&menu_layout, SCREEN_W, SCREEN_H);

    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (buffer == NULL) {
        allegro_message("Impossible de creer le buffer.");
        resources_free(&assets);
        allegro_exit();
        return EXIT_FAILURE;
    }

    reset_text_buffer(pseudo_buffer);
    ui_message[0] = '\0';
    memset(&game, 0, sizeof(game));

    show_mouse(screen);
    previous_time = (double)clock() / (double)CLOCKS_PER_SEC;

    while (1) {
        double now = (double)clock() / (double)CLOCKS_PER_SEC;
        float dt = (float)(now - previous_time);
        previous_time = now;
        animation_clock += dt;

        input_poll(&input);

        if (current_screen == SCREEN_MENU) {
            menu_selected = hover_selected(&menu_layout, 5, 0.29f, input.mouse_x, input.mouse_y, menu_selected);

            if (input.up_pressed) menu_selected = (menu_selected + 4) % 5;
            if (input.down_pressed) menu_selected = (menu_selected + 1) % 5;

            if (input.mouse_left_pressed) {
                int clicked = click_on_button(&menu_layout, 5, 0.29f, input.mouse_x, input.mouse_y);
                if (clicked >= 0) menu_selected = clicked;
            }

            if (input.confirm_pressed || input.mouse_left_pressed) {
                int action = menu_selected;
                if (input.mouse_left_pressed) {
                    int clicked = click_on_button(&menu_layout, 5, 0.29f, input.mouse_x, input.mouse_y);
                    if (clicked >= 0) action = clicked;
                }

                if (action == 0) {
                    current_screen = SCREEN_RULES;
                } else if (action == 1) {
                    reset_text_buffer(pseudo_buffer);
                    ui_message[0] = '\0';
                    current_screen = SCREEN_INPUT_NEW;
                } else if (action == 2) {
                    reset_text_buffer(pseudo_buffer);
                    ui_message[0] = '\0';
                    current_screen = SCREEN_INPUT_LOAD;
                } else if (action == 3) {
                    highscore_count = load_highscores(highscores, MAX_HIGHSCORES);
                    current_screen = SCREEN_HIGHSCORES;
                } else if (action == 4) {
                    break;
                }
            }

            renderer_draw_menu(buffer,
                               &assets,
                               &menu_layout,
                               "SUPER BULLES",
                               "Projet ING1 - Allegro 4 / C",
                               menu_items,
                               5,
                               menu_selected,
                               input.mouse_x,
                               input.mouse_y);
        } else if (current_screen == SCREEN_RULES) {
            if (input.back_pressed || input.confirm_pressed || input.mouse_left_pressed) {
                current_screen = SCREEN_MENU;
            }
            renderer_draw_rules(buffer, &assets, &menu_layout);
        } else if (current_screen == SCREEN_INPUT_NEW) {
            append_typed_chars(pseudo_buffer, &input);

            if (input.back_pressed) {
                current_screen = SCREEN_MENU;
            } else if (input.enter_pressed) {
                if (pseudo_buffer[0] == '\0') {
                    snprintf(ui_message, sizeof(ui_message), "Le pseudo ne peut pas etre vide.");
                } else {
                    game_destroy(&game);
                    game_init(&game, pseudo_buffer, 1, SCREEN_W, SCREEN_H);
                    ui_message[0] = '\0';
                    completion_recorded = 0;
                    current_screen = SCREEN_PLAYING;
                }
            }

            renderer_draw_text_input(buffer,
                                     &assets,
                                     &menu_layout,
                                     "NOUVELLE PARTIE",
                                     "Saisissez votre pseudo",
                                     pseudo_buffer,
                                     "Entree pour valider - Echap pour revenir",
                                     ui_message);
        } else if (current_screen == SCREEN_INPUT_LOAD) {
            append_typed_chars(pseudo_buffer, &input);

            if (input.back_pressed) {
                current_screen = SCREEN_MENU;
            } else if (input.enter_pressed) {
                SaveRecord record;
                if (pseudo_buffer[0] == '\0') {
                    snprintf(ui_message, sizeof(ui_message), "Entrez un pseudo existant.");
                } else if (!load_progress(pseudo_buffer, &record)) {
                    snprintf(ui_message, sizeof(ui_message), "Aucune sauvegarde trouvee pour \"%s\".", pseudo_buffer);
                } else {
                    game_destroy(&game);
                    game_init(&game, record.pseudo, clamp_stage(record.next_stage), SCREEN_W, SCREEN_H);
                    game.score = record.score;
                    snprintf(ui_message, sizeof(ui_message), "Sauvegarde chargee.");
                    completion_recorded = 0;
                    current_screen = SCREEN_PLAYING;
                }
            }

            renderer_draw_text_input(buffer,
                                     &assets,
                                     &menu_layout,
                                     "REPRENDRE UNE PARTIE",
                                     "Entrez le pseudo sauvegarde",
                                     pseudo_buffer,
                                     "Entree pour charger - Echap pour revenir",
                                     ui_message);
        } else if (current_screen == SCREEN_PLAYING) {
            if (input.back_pressed) {
                current_screen = SCREEN_STAGE_END;
                game.status = GAME_STATUS_LOST;
                snprintf(game.status_message, sizeof(game.status_message), "Partie interrompue");
                end_selected = 3;
            } else {
                game_update(&game, &input, dt);
                if (game.status == GAME_STATUS_WON || game.status == GAME_STATUS_LOST) {
                    end_selected = (game.status == GAME_STATUS_WON) ? 2 : 2;
                    current_screen = SCREEN_STAGE_END;
                } else if (game.status == GAME_STATUS_COMPLETED) {
                    completion_recorded = 0;
                    current_screen = SCREEN_COMPLETION;
                }
            }
            renderer_draw_game(buffer, &assets, &game);
        } else if (current_screen == SCREEN_STAGE_END) {
            end_selected = hover_selected(&game.layout, 4, 0.43f, input.mouse_x, input.mouse_y, end_selected);

            if (input.up_pressed) end_selected = (end_selected + 3) % 4;
            if (input.down_pressed) end_selected = (end_selected + 1) % 4;
            if (input.mouse_left_pressed) {
                int clicked = click_on_button(&game.layout, 4, 0.43f, input.mouse_x, input.mouse_y);
                if (clicked >= 0) end_selected = clicked;
            }

            if (input.confirm_pressed || input.mouse_left_pressed) {
                int action = end_selected;
                if (input.mouse_left_pressed) {
                    int clicked = click_on_button(&game.layout, 4, 0.43f, input.mouse_x, input.mouse_y);
                    if (clicked >= 0) action = clicked;
                }

                if (action == 0) {
                    break;
                } else if (action == 1) {
                    int next_stage = (game.status == GAME_STATUS_WON) ? game_next_stage_after_win(&game) : game.current_stage;
                    next_stage = clamp_stage(next_stage);
                    if (save_progress(game.player.pseudo, next_stage, game.score)) {
                        update_highscores(game.player.pseudo, game.score);
                        snprintf(game.status_message, sizeof(game.status_message), "Sauvegarde reussie : %s -> stage %d", game.player.pseudo, next_stage);
                    } else {
                        snprintf(game.status_message, sizeof(game.status_message), "Echec de la sauvegarde.");
                    }
                } else if (action == 2) {
                    if (game.status == GAME_STATUS_WON) {
                        int next_stage = game_next_stage_after_win(&game);
                        if (next_stage > BOSS_STAGE) {
                            current_screen = SCREEN_COMPLETION;
                        } else {
                            game_reset_stage(&game, next_stage);
                            current_screen = SCREEN_PLAYING;
                        }
                    } else {
                        game_reset_stage(&game, game.current_stage);
                        current_screen = SCREEN_PLAYING;
                    }
                } else if (action == 3) {
                    current_screen = SCREEN_MENU;
                }
            }

            renderer_draw_stage_end(buffer,
                                    &assets,
                                    &game,
                                    end_items,
                                    4,
                                    end_selected,
                                    input.mouse_x,
                                    input.mouse_y);
        } else if (current_screen == SCREEN_HIGHSCORES) {
            if (input.back_pressed || input.confirm_pressed || input.mouse_left_pressed) {
                current_screen = SCREEN_MENU;
            }
            renderer_draw_highscores(buffer, &assets, &menu_layout, highscores, highscore_count);
        } else if (current_screen == SCREEN_COMPLETION) {
            if (!completion_recorded) {
                update_highscores(game.player.pseudo, game.score);
                highscore_count = load_highscores(highscores, MAX_HIGHSCORES);
                completion_recorded = 1;
            }

            if (input.confirm_pressed || input.mouse_left_pressed || input.back_pressed) {
                current_screen = SCREEN_MENU;
            }

            renderer_draw_completion(buffer, &assets, &game, (float)animation_clock);
        }

        blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
        rest(FRAME_TIME_MS);
    }

    destroy_bitmap(buffer);
    game_destroy(&game);
    resources_free(&assets);
    allegro_exit();
    return EXIT_SUCCESS;
}
END_OF_MAIN();
