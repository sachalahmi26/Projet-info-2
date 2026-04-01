#ifndef RESOURCES_H
#define RESOURCES_H

#include <allegro.h>
#include "config.h"

typedef struct {
    BITMAP *menu_bg;
    BITMAP *backgrounds[STAGE_COUNT + 1];
    BITMAP *player;
    BITMAP *bubbles[4];
    BITMAP *boss;
    BITMAP *bonus_rapid;
    BITMAP *bonus_triple;
    BITMAP *bonus_explosive;
    BITMAP *projectile;
    BITMAP *lightning;
    BITMAP *victory;
    BITMAP *defeat;
} Assets;

int resources_load(Assets *assets);
void resources_free(Assets *assets);

#endif
