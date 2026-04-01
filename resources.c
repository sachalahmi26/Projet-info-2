#include <string.h>
#include <allegro.h>
#include "config.h"
#include "resources.h"

static BITMAP *load_asset(const char *path) {
    BITMAP *bmp = load_bitmap(path, NULL);
    return bmp;
}

int resources_load(Assets *assets) {
    int i;
    if (assets == NULL) {
        return 0;
    }

    memset(assets, 0, sizeof(*assets));

    assets->menu_bg = load_asset("assets/background_menu.bmp");

    assets->backgrounds[1] = load_asset("assets/background_level1.bmp");
    assets->backgrounds[2] = load_asset("assets/background_level2.bmp");
    assets->backgrounds[3] = load_asset("assets/background_level3.bmp");
    assets->backgrounds[4] = load_asset("assets/background_level4.bmp");
    assets->backgrounds[5] = load_asset("assets/background_boss.bmp");

    assets->player = load_asset("assets/player.bmp");
    assets->bubbles[0] = load_asset("assets/bubble_tiny.bmp");
    assets->bubbles[1] = load_asset("assets/bubble_small.bmp");
    assets->bubbles[2] = load_asset("assets/bubble_medium.bmp");
    assets->bubbles[3] = load_asset("assets/bubble_big.bmp");
    assets->boss = load_asset("assets/boss.bmp");

    assets->bonus_rapid = load_asset("assets/bonus_rapid.bmp");
    assets->bonus_triple = load_asset("assets/bonus_triple.bmp");
    assets->bonus_explosive = load_asset("assets/bonus_explosive.bmp");

    assets->projectile = load_asset("assets/projectile.bmp");
    assets->lightning = load_asset("assets/lightning.bmp");
    assets->victory = load_asset("assets/victory.bmp");
    assets->defeat = load_asset("assets/defeat.bmp");

    for (i = 0; i <= STAGE_COUNT; ++i) {
        /* rien */
    }
    return 1;
}

void resources_free(Assets *assets) {
    int i;
    if (assets == NULL) {
        return;
    }

    if (assets->menu_bg) destroy_bitmap(assets->menu_bg);
    for (i = 0; i <= STAGE_COUNT; ++i) {
        if (assets->backgrounds[i]) destroy_bitmap(assets->backgrounds[i]);
    }
    if (assets->player) destroy_bitmap(assets->player);
    for (i = 0; i < 4; ++i) {
        if (assets->bubbles[i]) destroy_bitmap(assets->bubbles[i]);
    }
    if (assets->boss) destroy_bitmap(assets->boss);
    if (assets->bonus_rapid) destroy_bitmap(assets->bonus_rapid);
    if (assets->bonus_triple) destroy_bitmap(assets->bonus_triple);
    if (assets->bonus_explosive) destroy_bitmap(assets->bonus_explosive);
    if (assets->projectile) destroy_bitmap(assets->projectile);
    if (assets->lightning) destroy_bitmap(assets->lightning);
    if (assets->victory) destroy_bitmap(assets->victory);
    if (assets->defeat) destroy_bitmap(assets->defeat);

    memset(assets, 0, sizeof(*assets));
}
