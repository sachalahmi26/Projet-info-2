#ifndef TYPES_H
#define TYPES_H

#include "config.h"

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} RectF;

typedef enum {
    WEAPON_BASIC = 0,
    WEAPON_RAPID,
    WEAPON_TRIPLE,
    WEAPON_EXPLOSIVE
} WeaponType;

typedef enum {
    BONUS_NONE = 0,
    BONUS_RAPID,
    BONUS_TRIPLE,
    BONUS_EXPLOSIVE
} BonusType;

typedef enum {
    EFFECT_HIT = 0,
    EFFECT_EXPLOSION
} EffectType;

typedef enum {
    GAME_STATUS_COUNTDOWN = 0,
    GAME_STATUS_RUNNING,
    GAME_STATUS_WON,
    GAME_STATUS_LOST,
    GAME_STATUS_COMPLETED
} GameStatus;

typedef struct {
    RectF info_zone;
    RectF game_zone;
    int screen_w;
    int screen_h;
} Layout;

typedef struct {
    RectF rect;
} Obstacle;

typedef struct Projectile {
    Vec2 pos;
    Vec2 vel;
    float radius;
    WeaponType weapon_type;
    float ttl;
    struct Projectile *next;
} Projectile;

typedef struct Bubble {
    Vec2 pos;
    Vec2 vel;
    float radius;
    int tier;
    int hit_points;
    int can_shoot_lightning;
    float lightning_cooldown;
    int can_drop_bonus;
    BonusType hidden_bonus;
    struct Bubble *next;
} Bubble;

typedef struct Bonus {
    Vec2 pos;
    Vec2 vel;
    float size;
    BonusType type;
    float ttl;
    struct Bonus *next;
} Bonus;

typedef struct Lightning {
    Vec2 pos;
    float width;
    float length;
    float speed;
    float ttl;
    struct Lightning *next;
} Lightning;

typedef struct Effect {
    Vec2 pos;
    float radius;
    float timer;
    float duration;
    EffectType type;
    struct Effect *next;
} Effect;

typedef struct {
    int active;
    Vec2 pos;
    Vec2 vel;
    float w;
    float h;
    int hp;
    int max_hp;
    float spawn_cooldown;
    float invuln_timer;
} Boss;

typedef struct {
    char pseudo[MAX_PSEUDO_LEN + 1];
    Vec2 pos;
    float w;
    float h;
    float speed;
    int alive;
    WeaponType weapon;
    float weapon_timer;
    float shoot_cooldown;
} Player;

typedef struct {
    char pseudo[MAX_PSEUDO_LEN + 1];
    int next_stage;
    long score;
} SaveRecord;

typedef struct {
    char pseudo[MAX_PSEUDO_LEN + 1];
    long score;
} ScoreRecord;

typedef struct {
    int left;
    int right;
    int fire;
    int up_pressed;
    int down_pressed;
    int confirm_pressed;
    int back_pressed;
    int esc_pressed;
    int mouse_x;
    int mouse_y;
    int mouse_left_down;
    int mouse_left_pressed;
    int typed_count;
    char typed_chars[MAX_TYPED_CHARS];
    int backspace_pressed;
    int delete_pressed;
    int enter_pressed;
} InputState;

typedef struct {
    Layout layout;
    Player player;
    Bubble *bubbles;
    Projectile *projectiles;
    Bonus *bonuses;
    Lightning *lightnings;
    Effect *effects;
    Boss boss;
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacle_count;
    int current_stage;
    int pending_spawn_after_countdown;
    float countdown;
    float time_limit;
    float time_left;
    long score;
    long stage_bonus_score;
    GameStatus status;
    char status_message[MAX_MESSAGE_LEN + 1];
} GameState;

#endif
