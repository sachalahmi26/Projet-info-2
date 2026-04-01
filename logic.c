#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "logic.h"

#define PI 3.1415926535f

static float clampf(float v, float min_v, float max_v) {
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

static float minf(float a, float b) {
    return (a < b) ? a : b;
}

static float maxf(float a, float b) {
    return (a > b) ? a : b;
}

static float randomf(float a, float b) {
    return a + ((float)rand() / (float)RAND_MAX) * (b - a);
}

static float bubble_radius_for_tier(int tier) {
    switch (tier) {
        case 3: return 60.0f;
        case 2: return 42.0f;
        case 1: return 28.0f;
        default: return 18.0f;
    }
}

static int bubble_score_for_tier(int tier) {
    switch (tier) {
        case 3: return 180;
        case 2: return 130;
        case 1: return 90;
        default: return 55;
    }
}

static void clear_projectiles(Projectile **head) {
    Projectile *node = *head;
    while (node != NULL) {
        Projectile *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

static void clear_bubbles(Bubble **head) {
    Bubble *node = *head;
    while (node != NULL) {
        Bubble *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

static void clear_bonuses(Bonus **head) {
    Bonus *node = *head;
    while (node != NULL) {
        Bonus *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

static void clear_lightnings(Lightning **head) {
    Lightning *node = *head;
    while (node != NULL) {
        Lightning *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

static void clear_effects(Effect **head) {
    Effect *node = *head;
    while (node != NULL) {
        Effect *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

static void game_clear_dynamic_entities(GameState *game) {
    clear_bubbles(&game->bubbles);
    clear_projectiles(&game->projectiles);
    clear_bonuses(&game->bonuses);
    clear_lightnings(&game->lightnings);
    clear_effects(&game->effects);
}

void layout_compute(Layout *layout, int screen_w, int screen_h) {
    float info_h;
    float margin_x;
    float margin_y;

    if (layout == NULL) {
        return;
    }

    info_h = screen_h * INFO_ZONE_RATIO;
    margin_x = screen_w * PLAYFIELD_MARGIN_RATIO;
    margin_y = screen_h * PLAYFIELD_MARGIN_RATIO;

    layout->screen_w = screen_w;
    layout->screen_h = screen_h;

    layout->info_zone.x = margin_x;
    layout->info_zone.y = margin_y;
    layout->info_zone.w = screen_w - 2.0f * margin_x;
    layout->info_zone.h = info_h - margin_y;

    layout->game_zone.x = margin_x;
    layout->game_zone.y = info_h;
    layout->game_zone.w = screen_w - 2.0f * margin_x;
    layout->game_zone.h = screen_h - info_h - margin_y;
}

static void spawn_effect(GameState *game, Vec2 pos, float radius, float duration, EffectType type) {
    Effect *e = (Effect *)malloc(sizeof(Effect));
    if (e == NULL) {
        return;
    }
    e->pos = pos;
    e->radius = radius;
    e->timer = duration;
    e->duration = duration;
    e->type = type;
    e->next = game->effects;
    game->effects = e;
}

static Bubble *spawn_bubble(GameState *game,
                            float x,
                            float y,
                            float vx,
                            float vy,
                            int tier,
                            int can_shoot_lightning,
                            int can_drop_bonus,
                            BonusType hidden_bonus) {
    Bubble *bubble = (Bubble *)malloc(sizeof(Bubble));
    if (bubble == NULL) {
        return NULL;
    }

    bubble->pos.x = x;
    bubble->pos.y = y;
    bubble->vel.x = vx * BUBBLE_SPEED_FACTOR;
    bubble->vel.y = vy * BUBBLE_SPEED_FACTOR;
    bubble->radius = bubble_radius_for_tier(tier);
    bubble->tier = tier;
    bubble->hit_points = (tier == 3) ? 2 : 1;
    bubble->can_shoot_lightning = can_shoot_lightning;
    bubble->lightning_cooldown = randomf(1.0f, 2.5f);
    bubble->can_drop_bonus = can_drop_bonus;
    bubble->hidden_bonus = hidden_bonus;
    bubble->next = game->bubbles;
    game->bubbles = bubble;
    return bubble;
}

static void spawn_bonus(GameState *game, float x, float y, BonusType type) {
    Bonus *bonus;

    if (type == BONUS_NONE) {
        return;
    }

    bonus = (Bonus *)malloc(sizeof(Bonus));
    if (bonus == NULL) {
        return;
    }

    bonus->pos.x = x;
    bonus->pos.y = y;
    bonus->vel.x = randomf(-30.0f, 30.0f);
    bonus->vel.y = 0.0f;
    bonus->size = 28.0f;
    bonus->type = type;
    bonus->ttl = 8.0f;
    bonus->next = game->bonuses;
    game->bonuses = bonus;
}

static void spawn_projectile(GameState *game, float x, float y, float vx, float vy, WeaponType weapon_type) {
    Projectile *p = (Projectile *)malloc(sizeof(Projectile));
    if (p == NULL) {
        return;
    }
    p->pos.x = x;
    p->pos.y = y;
    p->vel.x = vx;
    p->vel.y = vy;
    p->radius = (weapon_type == WEAPON_EXPLOSIVE) ? 10.0f : 5.0f;
    p->weapon_type = weapon_type;
    p->ttl = 1.7f;
    p->next = game->projectiles;
    game->projectiles = p;
}

static void spawn_lightning(GameState *game, float x, float y) {
    Lightning *l = (Lightning *)malloc(sizeof(Lightning));
    if (l == NULL) {
        return;
    }
    l->pos.x = x;
    l->pos.y = y;
    l->width = 10.0f;
    l->length = 65.0f;
    l->speed = game->layout.game_zone.h * LIGHTNING_SPEED_RATIO;
    l->ttl = 2.0f;
    l->next = game->lightnings;
    game->lightnings = l;
}

static int circle_intersects_rect(Vec2 c, float radius, RectF rect) {
    float nearest_x = clampf(c.x, rect.x, rect.x + rect.w);
    float nearest_y = clampf(c.y, rect.y, rect.y + rect.h);
    float dx = c.x - nearest_x;
    float dy = c.y - nearest_y;
    return dx * dx + dy * dy <= radius * radius;
}

static int rects_intersect(RectF a, RectF b) {
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

static RectF player_rect(const Player *player) {
    RectF r;
    r.x = player->pos.x - player->w * 0.5f;
    r.y = player->pos.y - player->h * 0.5f;
    r.w = player->w;
    r.h = player->h;
    return r;
}

static void setup_obstacle(Obstacle *o, float x, float y, float w, float h) {
    o->rect.x = x;
    o->rect.y = y;
    o->rect.w = w;
    o->rect.h = h;
}

static void configure_stage_obstacles(GameState *game) {
    float gx = game->layout.game_zone.x;
    float gy = game->layout.game_zone.y;
    float gw = game->layout.game_zone.w;
    float gh = game->layout.game_zone.h;

    game->obstacle_count = 0;

    switch (game->current_stage) {
        case 2:
            game->obstacle_count = 1;
            setup_obstacle(&game->obstacles[0],
                           gx + gw * 0.43f,
                           gy + gh * 0.46f,
                           gw * 0.14f,
                           gh * 0.04f);
            break;
        case 3:
            game->obstacle_count = 2;
            setup_obstacle(&game->obstacles[0],
                           gx + gw * 0.20f,
                           gy + gh * 0.40f,
                           gw * 0.18f,
                           gh * 0.035f);
            setup_obstacle(&game->obstacles[1],
                           gx + gw * 0.62f,
                           gy + gh * 0.55f,
                           gw * 0.14f,
                           gh * 0.035f);
            break;
        case 4:
            game->obstacle_count = 3;
            setup_obstacle(&game->obstacles[0],
                           gx + gw * 0.17f,
                           gy + gh * 0.34f,
                           gw * 0.17f,
                           gh * 0.035f);
            setup_obstacle(&game->obstacles[1],
                           gx + gw * 0.43f,
                           gy + gh * 0.52f,
                           gw * 0.16f,
                           gh * 0.035f);
            setup_obstacle(&game->obstacles[2],
                           gx + gw * 0.69f,
                           gy + gh * 0.38f,
                           gw * 0.14f,
                           gh * 0.035f);
            break;
        default:
            break;
    }
}

static void configure_stage_entities(GameState *game) {
    float gx = game->layout.game_zone.x;
    float gy = game->layout.game_zone.y;
    float gw = game->layout.game_zone.w;
    float gh = game->layout.game_zone.h;

    if (game->current_stage == 1) {
        spawn_bubble(game, gx + gw * 0.28f, gy + gh * 0.14f, gw * 0.13f, -gh * 0.52f, 3, 0, 0, BONUS_NONE);
        spawn_bubble(game, gx + gw * 0.70f, gy + gh * 0.17f, -gw * 0.11f, -gh * 0.46f, 2, 0, 0, BONUS_NONE);
    } else if (game->current_stage == 2) {
        spawn_bubble(game, gx + gw * 0.22f, gy + gh * 0.16f, gw * 0.13f, -gh * 0.52f, 3, 0, 1, BONUS_RAPID);
        spawn_bubble(game, gx + gw * 0.74f, gy + gh * 0.12f, -gw * 0.16f, -gh * 0.55f, 2, 0, 1, BONUS_TRIPLE);
    } else if (game->current_stage == 3) {
        spawn_bubble(game, gx + gw * 0.24f, gy + gh * 0.11f, gw * 0.14f, -gh * 0.56f, 3, 1, 1, BONUS_EXPLOSIVE);
        spawn_bubble(game, gx + gw * 0.62f, gy + gh * 0.16f, -gw * 0.13f, -gh * 0.58f, 2, 1, 0, BONUS_NONE);
        spawn_bubble(game, gx + gw * 0.80f, gy + gh * 0.09f, -gw * 0.09f, -gh * 0.50f, 1, 0, 0, BONUS_NONE);
    } else if (game->current_stage == 4) {
        spawn_bubble(game, gx + gw * 0.18f, gy + gh * 0.12f, gw * 0.14f, -gh * 0.54f, 3, 1, 1, BONUS_RAPID);
        spawn_bubble(game, gx + gw * 0.42f, gy + gh * 0.18f, -gw * 0.14f, -gh * 0.56f, 2, 1, 1, BONUS_TRIPLE);
        spawn_bubble(game, gx + gw * 0.72f, gy + gh * 0.11f, gw * 0.15f, -gh * 0.58f, 2, 1, 1, BONUS_EXPLOSIVE);
    } else if (game->current_stage == BOSS_STAGE) {
        game->boss.active = 1;
        game->boss.w = gw * 0.13f;
        game->boss.h = gh * 0.16f;
        game->boss.pos.x = gx + gw * 0.50f;
        game->boss.pos.y = gy + gh * 0.16f;
        game->boss.vel.x = gw * 0.14f;
        game->boss.vel.y = gh * 0.06f;
        game->boss.max_hp = 12;
        game->boss.hp = game->boss.max_hp;
        game->boss.spawn_cooldown = 1.0f;
        game->boss.invuln_timer = 0.0f;
    }
}

static void reset_player_for_stage(GameState *game) {
    game->player.pos.x = game->layout.game_zone.x + game->layout.game_zone.w * 0.5f;
    game->player.pos.y = game->layout.game_zone.y + game->layout.game_zone.h - 16.0f;
    game->player.w = minf(game->layout.game_zone.w * 0.05f, 52.0f);
    game->player.h = minf(game->layout.game_zone.h * 0.08f, 56.0f);
    game->player.speed = game->layout.game_zone.w * PLAYER_BASE_SPEED_RATIO;
    game->player.alive = 1;
    game->player.weapon = WEAPON_BASIC;
    game->player.weapon_timer = 0.0f;
    game->player.shoot_cooldown = 0.0f;
}

static float stage_time_limit(int stage) {
    switch (stage) {
        case 1: return 55.0f;
        case 2: return 60.0f;
        case 3: return 65.0f;
        case 4: return 70.0f;
        default: return 80.0f;
    }
}

static void spawn_stage_if_needed(GameState *game) {
    if (game->pending_spawn_after_countdown && game->countdown <= 0.0f) {
        game->pending_spawn_after_countdown = 0;
        configure_stage_entities(game);
    }
}

void game_reset_stage(GameState *game, int stage) {
    if (game == NULL) {
        return;
    }

    game_clear_dynamic_entities(game);
    memset(&game->boss, 0, sizeof(game->boss));

    game->current_stage = stage;
    game->time_limit = stage_time_limit(stage);
    game->time_left = game->time_limit;
    game->countdown = 4.0f;
    game->status = GAME_STATUS_COUNTDOWN;
    game->stage_bonus_score = 0;
    snprintf(game->status_message, sizeof(game->status_message), "Stage %d", stage);

    configure_stage_obstacles(game);
    reset_player_for_stage(game);

    game->pending_spawn_after_countdown = (stage == 2 || stage == 4) ? 1 : 0;
    if (!game->pending_spawn_after_countdown) {
        configure_stage_entities(game);
    }
}

void game_init(GameState *game, const char *pseudo, int start_stage, int screen_w, int screen_h) {
    if (game == NULL) {
        return;
    }

    memset(game, 0, sizeof(*game));
    layout_compute(&game->layout, screen_w, screen_h);
    if (pseudo != NULL) {
        strncpy(game->player.pseudo, pseudo, MAX_PSEUDO_LEN);
        game->player.pseudo[MAX_PSEUDO_LEN] = '\0';
    } else {
        strcpy(game->player.pseudo, "Joueur");
    }

    srand((unsigned int)time(NULL));
    game->score = 0;
    game_reset_stage(game, start_stage);
}

void game_destroy(GameState *game) {
    if (game == NULL) {
        return;
    }
    game_clear_dynamic_entities(game);
}

const char *weapon_name(WeaponType weapon) {
    switch (weapon) {
        case WEAPON_RAPID: return "Rafale";
        case WEAPON_TRIPLE: return "Triple";
        case WEAPON_EXPLOSIVE: return "Explosive";
        default: return "Simple";
    }
}

const char *bonus_name(BonusType bonus) {
    switch (bonus) {
        case BONUS_RAPID: return "Rafale";
        case BONUS_TRIPLE: return "Triple tir";
        case BONUS_EXPLOSIVE: return "Explosion";
        default: return "Aucun";
    }
}

const char *stage_display_name(int stage) {
    switch (stage) {
        case 1: return "Niveau 1";
        case 2: return "Niveau 2";
        case 3: return "Niveau 3";
        case 4: return "Niveau 4";
        case BOSS_STAGE: return "Boss final";
        default: return "Inconnu";
    }
}

int game_stage_is_boss(int stage) {
    return stage == BOSS_STAGE;
}

int game_next_stage_after_win(const GameState *game) {
    if (game == NULL) {
        return 1;
    }
    return game->current_stage + 1;
}

int game_should_show_end_menu(const GameState *game) {
    if (game == NULL) {
        return 0;
    }
    return game->status == GAME_STATUS_WON || game->status == GAME_STATUS_LOST;
}

int game_player_active_projectiles(const GameState *game) {
    int count = 0;
    Projectile *p = game->projectiles;
    while (p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}

static void update_effects(GameState *game, float dt) {
    Effect **it = &game->effects;
    while (*it != NULL) {
        Effect *e = *it;
        e->timer -= dt;
        if (e->timer <= 0.0f) {
            *it = e->next;
            free(e);
        } else {
            it = &e->next;
        }
    }
}

static void give_bonus_to_player(GameState *game, BonusType type) {
    game->player.weapon = (WeaponType)type;
    game->player.weapon_timer = 8.0f;
    switch (type) {
        case BONUS_RAPID:
            game->status_message[0] = '\0';
            snprintf(game->status_message, sizeof(game->status_message), "Bonus acquis : %s", bonus_name(type));
            game->score += 75;
            break;
        case BONUS_TRIPLE:
            snprintf(game->status_message, sizeof(game->status_message), "Bonus acquis : %s", bonus_name(type));
            game->score += 90;
            break;
        case BONUS_EXPLOSIVE:
            snprintf(game->status_message, sizeof(game->status_message), "Bonus acquis : %s", bonus_name(type));
            game->score += 110;
            break;
        default:
            break;
    }
}

static void update_player(GameState *game, const InputState *input, float dt) {
    float left_limit;
    float right_limit;
    int allow_single_projectile;

    left_limit = game->layout.game_zone.x + game->player.w * 0.5f;
    right_limit = game->layout.game_zone.x + game->layout.game_zone.w - game->player.w * 0.5f;

    if (input->left) {
        game->player.pos.x -= game->player.speed * dt;
    }
    if (input->right) {
        game->player.pos.x += game->player.speed * dt;
    }
    game->player.pos.x = clampf(game->player.pos.x, left_limit, right_limit);

    if (game->player.weapon_timer > 0.0f) {
        game->player.weapon_timer -= dt;
        if (game->player.weapon_timer <= 0.0f) {
            game->player.weapon = WEAPON_BASIC;
            game->player.weapon_timer = 0.0f;
        }
    }

    if (game->player.shoot_cooldown > 0.0f) {
        game->player.shoot_cooldown -= dt;
    }

    allow_single_projectile = game_player_active_projectiles(game) == 0;

    if (input->fire && game->player.shoot_cooldown <= 0.0f) {
        float origin_x = game->player.pos.x;
        float origin_y = game->player.pos.y - game->player.h * 0.5f;

        if (game->player.weapon == WEAPON_BASIC) {
            if (allow_single_projectile) {
                spawn_projectile(game, origin_x, origin_y, 0.0f,
                                 -game->layout.game_zone.h * PROJECTILE_SPEED_RATIO,
                                 WEAPON_BASIC);
                game->player.shoot_cooldown = 0.45f;
            }
        } else if (game->player.weapon == WEAPON_RAPID) {
            spawn_projectile(game, origin_x, origin_y, 0.0f,
                             -game->layout.game_zone.h * PROJECTILE_SPEED_RATIO * 1.10f,
                             WEAPON_RAPID);
            game->player.shoot_cooldown = 0.15f;
        } else if (game->player.weapon == WEAPON_TRIPLE) {
            spawn_projectile(game, origin_x - 16.0f, origin_y, -75.0f,
                             -game->layout.game_zone.h * PROJECTILE_SPEED_RATIO,
                             WEAPON_TRIPLE);
            spawn_projectile(game, origin_x, origin_y, 0.0f,
                             -game->layout.game_zone.h * PROJECTILE_SPEED_RATIO,
                             WEAPON_TRIPLE);
            spawn_projectile(game, origin_x + 16.0f, origin_y, 75.0f,
                             -game->layout.game_zone.h * PROJECTILE_SPEED_RATIO,
                             WEAPON_TRIPLE);
            game->player.shoot_cooldown = 0.35f;
        } else if (game->player.weapon == WEAPON_EXPLOSIVE) {
            spawn_projectile(game, origin_x, origin_y, 0.0f,
                             -game->layout.game_zone.h * PROJECTILE_SPEED_RATIO * 0.95f,
                             WEAPON_EXPLOSIVE);
            game->player.shoot_cooldown = 0.55f;
        }
    }
}

static void update_projectiles(GameState *game, float dt) {
    float top_limit = game->layout.game_zone.y - 10.0f;
    Projectile **it = &game->projectiles;

    while (*it != NULL) {
        Projectile *p = *it;
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->ttl -= dt;

        if (p->pos.y < top_limit || p->ttl <= 0.0f ||
            p->pos.x < game->layout.game_zone.x - 16.0f ||
            p->pos.x > game->layout.game_zone.x + game->layout.game_zone.w + 16.0f) {
            *it = p->next;
            free(p);
        } else {
            it = &p->next;
        }
    }
}

static void resolve_bubble_obstacle_collision(Bubble *bubble, const Obstacle *o) {
    if (circle_intersects_rect(bubble->pos, bubble->radius, o->rect)) {
        float left_pen = fabsf((bubble->pos.x + bubble->radius) - o->rect.x);
        float right_pen = fabsf((o->rect.x + o->rect.w) - (bubble->pos.x - bubble->radius));
        float top_pen = fabsf((bubble->pos.y + bubble->radius) - o->rect.y);
        float bottom_pen = fabsf((o->rect.y + o->rect.h) - (bubble->pos.y - bubble->radius));

        float min_pen = left_pen;
        int side = 0;

        if (right_pen < min_pen) { min_pen = right_pen; side = 1; }
        if (top_pen < min_pen) { min_pen = top_pen; side = 2; }
        if (bottom_pen < min_pen) { min_pen = bottom_pen; side = 3; }

        if (side == 0) {
            bubble->pos.x = o->rect.x - bubble->radius - 1.0f;
            bubble->vel.x = -fabsf(bubble->vel.x);
        } else if (side == 1) {
            bubble->pos.x = o->rect.x + o->rect.w + bubble->radius + 1.0f;
            bubble->vel.x = fabsf(bubble->vel.x);
        } else if (side == 2) {
            bubble->pos.y = o->rect.y - bubble->radius - 1.0f;
            bubble->vel.y = -fabsf(bubble->vel.y) * 0.96f;
        } else {
            bubble->pos.y = o->rect.y + o->rect.h + bubble->radius + 1.0f;
            bubble->vel.y = fabsf(bubble->vel.y);
        }
    }
}

static void update_bubbles(GameState *game, float dt) {
    Bubble *bubble = game->bubbles;
    float left = game->layout.game_zone.x;
    float right = game->layout.game_zone.x + game->layout.game_zone.w;
    float floor = game->layout.game_zone.y + game->layout.game_zone.h;
    float gravity = game->layout.game_zone.h * GRAVITY_RATIO * BUBBLE_SPEED_FACTOR;

    while (bubble != NULL) {
        int i;
        bubble->vel.y += gravity * dt;
        bubble->pos.x += bubble->vel.x * dt;
        bubble->pos.y += bubble->vel.y * dt;

        if (bubble->pos.x - bubble->radius < left) {
            bubble->pos.x = left + bubble->radius;
            bubble->vel.x = fabsf(bubble->vel.x);
        }
        if (bubble->pos.x + bubble->radius > right) {
            bubble->pos.x = right - bubble->radius;
            bubble->vel.x = -fabsf(bubble->vel.x);
        }
        if (bubble->pos.y + bubble->radius > floor) {
            bubble->pos.y = floor - bubble->radius;
            bubble->vel.y = -maxf(180.0f, fabsf(bubble->vel.y) * 0.88f);
        }

        for (i = 0; i < game->obstacle_count; ++i) {
            resolve_bubble_obstacle_collision(bubble, &game->obstacles[i]);
        }

        if (bubble->can_shoot_lightning) {
            bubble->lightning_cooldown -= dt;
            if (bubble->lightning_cooldown <= 0.0f) {
                bubble->lightning_cooldown = randomf(1.2f, 3.0f);
                spawn_lightning(game, bubble->pos.x, bubble->pos.y + bubble->radius);
            }
        }

        bubble = bubble->next;
    }
}

static void update_bonuses(GameState *game, float dt) {
    float floor = game->layout.game_zone.y + game->layout.game_zone.h;
    RectF pr = player_rect(&game->player);
    Bonus **it = &game->bonuses;

    while (*it != NULL) {
        Bonus *bonus = *it;
        RectF br;

        bonus->vel.y = game->layout.game_zone.h * BONUS_FALL_SPEED_RATIO;
        bonus->pos.x += bonus->vel.x * dt;
        bonus->pos.y += bonus->vel.y * dt;
        bonus->ttl -= dt;

        if (bonus->pos.y + bonus->size * 0.5f > floor) {
            bonus->pos.y = floor - bonus->size * 0.5f;
            bonus->vel.x *= 0.94f;
        }

        br.x = bonus->pos.x - bonus->size * 0.5f;
        br.y = bonus->pos.y - bonus->size * 0.5f;
        br.w = bonus->size;
        br.h = bonus->size;

        if (rects_intersect(pr, br)) {
            give_bonus_to_player(game, bonus->type);
            *it = bonus->next;
            free(bonus);
        } else if (bonus->ttl <= 0.0f) {
            *it = bonus->next;
            free(bonus);
        } else {
            it = &bonus->next;
        }
    }
}

static void update_lightnings(GameState *game, float dt) {
    RectF pr = player_rect(&game->player);
    Lightning **it = &game->lightnings;

    while (*it != NULL) {
        Lightning *l = *it;
        RectF lr;

        l->pos.y += l->speed * dt;
        l->ttl -= dt;

        lr.x = l->pos.x - l->width * 0.5f;
        lr.y = l->pos.y;
        lr.w = l->width;
        lr.h = l->length;

        if (rects_intersect(pr, lr)) {
            game->player.alive = 0;
            game->status = GAME_STATUS_LOST;
            snprintf(game->status_message, sizeof(game->status_message), "Foudroye !");
        }

        if (l->ttl <= 0.0f || l->pos.y > game->layout.game_zone.y + game->layout.game_zone.h) {
            *it = l->next;
            free(l);
        } else {
            it = &l->next;
        }
    }
}

static void split_bubble(GameState *game, const Bubble *bubble) {
    if (bubble->tier <= 0) {
        return;
    }

    spawn_bubble(game,
                 bubble->pos.x - 8.0f,
                 bubble->pos.y,
                 -maxf(105.0f, fabsf(bubble->vel.x) + 55.0f),
                 -maxf(160.0f, fabsf(bubble->vel.y) + 85.0f),
                 bubble->tier - 1,
                 bubble->tier >= 2 ? bubble->can_shoot_lightning : 0,
                 0,
                 BONUS_NONE);

    spawn_bubble(game,
                 bubble->pos.x + 8.0f,
                 bubble->pos.y,
                 maxf(105.0f, fabsf(bubble->vel.x) + 55.0f),
                 -maxf(160.0f, fabsf(bubble->vel.y) + 85.0f),
                 bubble->tier - 1,
                 bubble->tier >= 2 ? bubble->can_shoot_lightning : 0,
                 0,
                 BONUS_NONE);
}

static void apply_explosion_damage(GameState *game, Vec2 center, float radius) {
    Bubble **it = &game->bubbles;
    while (*it != NULL) {
        Bubble *bubble = *it;
        float dx = bubble->pos.x - center.x;
        float dy = bubble->pos.y - center.y;
        float dist2 = dx * dx + dy * dy;
        float limit = radius + bubble->radius;

        if (dist2 <= limit * limit) {
            Bubble bubble_copy = *bubble;
            game->score += bubble_score_for_tier(bubble->tier);
            spawn_effect(game, bubble->pos, bubble->radius * 1.4f, 0.35f, EFFECT_EXPLOSION);

            if (bubble->can_drop_bonus) {
                spawn_bonus(game, bubble->pos.x, bubble->pos.y, bubble->hidden_bonus);
            }
            if (bubble->tier > 0) {
                split_bubble(game, &bubble_copy);
            }

            *it = bubble->next;
            free(bubble);
        } else {
            it = &bubble->next;
        }
    }
}

static void handle_projectile_hits(GameState *game) {
    Projectile **pit = &game->projectiles;

    while (*pit != NULL) {
        Projectile *p = *pit;
        int projectile_consumed = 0;
        Bubble **bit = &game->bubbles;

        while (*bit != NULL && !projectile_consumed) {
            Bubble *bubble = *bit;
            float dx = bubble->pos.x - p->pos.x;
            float dy = bubble->pos.y - p->pos.y;
            float limit = bubble->radius + p->radius;

            if (dx * dx + dy * dy <= limit * limit) {
                Bubble bubble_copy = *bubble;

                bubble->hit_points -= 1;
                spawn_effect(game, bubble->pos, bubble->radius * 0.8f, 0.20f, EFFECT_HIT);

                if (bubble->hit_points <= 0) {
                    game->score += bubble_score_for_tier(bubble->tier);
                    if (bubble->can_drop_bonus) {
                        spawn_bonus(game, bubble->pos.x, bubble->pos.y, bubble->hidden_bonus);
                    }
                    if (bubble->tier > 0) {
                        split_bubble(game, &bubble_copy);
                    }
                    *bit = bubble->next;
                    free(bubble);
                } else {
                    bit = &bubble->next;
                }

                if (p->weapon_type == WEAPON_EXPLOSIVE) {
                    apply_explosion_damage(game, p->pos, 86.0f);
                    spawn_effect(game, p->pos, 86.0f, 0.32f, EFFECT_EXPLOSION);
                }

                *pit = p->next;
                free(p);
                projectile_consumed = 1;
            } else {
                bit = &bubble->next;
            }
        }

        if (!projectile_consumed && game->boss.active) {
            RectF boss_rect;
            boss_rect.x = game->boss.pos.x - game->boss.w * 0.5f;
            boss_rect.y = game->boss.pos.y - game->boss.h * 0.5f;
            boss_rect.w = game->boss.w;
            boss_rect.h = game->boss.h;
            if (circle_intersects_rect(p->pos, p->radius, boss_rect) && game->boss.invuln_timer <= 0.0f) {
                game->boss.hp -= 1;
                game->boss.invuln_timer = 0.15f;
                game->score += 120;
                spawn_effect(game, p->pos, 30.0f, 0.18f, EFFECT_HIT);
                if (p->weapon_type == WEAPON_EXPLOSIVE) {
                    spawn_effect(game, p->pos, 88.0f, 0.32f, EFFECT_EXPLOSION);
                    apply_explosion_damage(game, p->pos, 74.0f);
                }
                *pit = p->next;
                free(p);
                projectile_consumed = 1;
                if (game->boss.hp <= 0) {
                    game->boss.active = 0;
                    game->stage_bonus_score = (long)(game->time_left * 22.0f);
                    game->score += game->stage_bonus_score + 1000;
                    game->status = GAME_STATUS_COMPLETED;
                    snprintf(game->status_message, sizeof(game->status_message), "Victoire finale !");
                }
            }
        }

        if (!projectile_consumed) {
            pit = &(*pit)->next;
        }
    }
}

static void handle_player_collisions(GameState *game) {
    Bubble *bubble = game->bubbles;
    RectF pr = player_rect(&game->player);

    while (bubble != NULL) {
        if (circle_intersects_rect(bubble->pos, bubble->radius, pr)) {
            game->player.alive = 0;
            game->status = GAME_STATUS_LOST;
            snprintf(game->status_message, sizeof(game->status_message), "Touche par une bulle");
            return;
        }
        bubble = bubble->next;
    }

    if (game->boss.active) {
        RectF boss_rect;
        boss_rect.x = game->boss.pos.x - game->boss.w * 0.5f;
        boss_rect.y = game->boss.pos.y - game->boss.h * 0.5f;
        boss_rect.w = game->boss.w;
        boss_rect.h = game->boss.h;

        if (rects_intersect(pr, boss_rect)) {
            game->player.alive = 0;
            game->status = GAME_STATUS_LOST;
            snprintf(game->status_message, sizeof(game->status_message), "Ecrase par le boss");
        }
    }
}

static void update_boss(GameState *game, float dt) {
    float left;
    float right;
    float top;
    float bottom;

    if (!game->boss.active) {
        return;
    }

    left = game->layout.game_zone.x + game->boss.w * 0.5f;
    right = game->layout.game_zone.x + game->layout.game_zone.w - game->boss.w * 0.5f;
    top = game->layout.game_zone.y + game->boss.h * 0.5f;
    bottom = game->layout.game_zone.y + game->layout.game_zone.h * 0.30f;

    if (game->boss.invuln_timer > 0.0f) {
        game->boss.invuln_timer -= dt;
    }

    game->boss.vel.x = (game->boss.vel.x >= 0.0f ? 1.0f : -1.0f) *
                    (game->layout.game_zone.w * (0.12f + 0.014f * (float)(game->boss.max_hp - game->boss.hp)));

    game->boss.pos.x += game->boss.vel.x * dt;
    game->boss.pos.y += game->boss.vel.y * dt;

    if (game->boss.pos.x < left) {
        game->boss.pos.x = left;
        game->boss.vel.x = fabsf(game->boss.vel.x);
    }
    if (game->boss.pos.x > right) {
        game->boss.pos.x = right;
        game->boss.vel.x = -fabsf(game->boss.vel.x);
    }
    if (game->boss.pos.y < top) {
        game->boss.pos.y = top;
        game->boss.vel.y = fabsf(game->boss.vel.y);
    }
    if (game->boss.pos.y > bottom) {
        game->boss.pos.y = bottom;
        game->boss.vel.y = -fabsf(game->boss.vel.y);
    }

    game->boss.spawn_cooldown -= dt;
    if (game->boss.spawn_cooldown <= 0.0f) {
        int tier = (rand() % 2) + 1;
        float dir = (rand() % 2) ? 1.0f : -1.0f;
        spawn_bubble(game,
                     game->boss.pos.x,
                     game->boss.pos.y + game->boss.h * 0.5f,
                     dir * game->layout.game_zone.w * 0.12f,
                     -game->layout.game_zone.h * 0.46f,
                     tier,
                     1,
                     0,
                     BONUS_NONE);
        game->boss.spawn_cooldown = randomf(1.8f, 2.8f);
    }
}

static int stage_cleared(const GameState *game) {
    return game->bubbles == NULL && !game->boss.active;
}

void game_update(GameState *game, const InputState *input, float dt) {
    if (game == NULL || input == NULL) {
        return;
    }

    dt = clampf(dt, 0.0f, 0.035f);
    update_effects(game, dt);

    if (game->status == GAME_STATUS_WON ||
        game->status == GAME_STATUS_LOST ||
        game->status == GAME_STATUS_COMPLETED) {
        return;
    }

    if (game->status == GAME_STATUS_COUNTDOWN) {
        game->countdown -= dt;
        spawn_stage_if_needed(game);
        if (game->countdown <= 0.0f) {
            game->countdown = 0.0f;
            game->status = GAME_STATUS_RUNNING;
        }
        return;
    }

    if (game->status != GAME_STATUS_RUNNING) {
        return;
    }

    game->time_left -= dt;
    if (game->time_left <= 0.0f) {
        game->time_left = 0.0f;
        game->status = GAME_STATUS_LOST;
        snprintf(game->status_message, sizeof(game->status_message), "Temps ecoule");
        return;
    }

    update_player(game, input, dt);
    update_projectiles(game, dt);
    update_bubbles(game, dt);
    update_bonuses(game, dt);
    update_lightnings(game, dt);
    update_boss(game, dt);
    handle_projectile_hits(game);
    handle_player_collisions(game);

    if (game->status == GAME_STATUS_RUNNING && stage_cleared(game)) {
        game->stage_bonus_score = (long)(game->time_left * 18.0f);
        game->score += game->stage_bonus_score + 250;
        game->status = GAME_STATUS_WON;
        snprintf(game->status_message,
                 sizeof(game->status_message),
                 "%s termine ! Bonus temps : %ld",
                 stage_display_name(game->current_stage),
                 game->stage_bonus_score);
    }
}
