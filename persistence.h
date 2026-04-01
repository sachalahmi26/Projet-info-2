#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "types.h"

int save_progress(const char *pseudo, int next_stage, long score);
int load_progress(const char *pseudo, SaveRecord *out_record);

int update_highscores(const char *pseudo, long score);
int load_highscores(ScoreRecord *scores, int max_scores);

#endif
