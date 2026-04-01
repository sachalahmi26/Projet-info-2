#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "persistence.h"

static int read_save_records(SaveRecord *records, int max_records) {
    FILE *f;
    int count = 0;
    char pseudo[MAX_PSEUDO_LEN + 1];
    int next_stage;
    long score;

    f = fopen(SAVE_FILE, "r");
    if (f == NULL) {
        return 0;
    }

    while (count < max_records && fscanf(f, " %31[^;];%d;%ld", pseudo, &next_stage, &score) == 3) {
        strcpy(records[count].pseudo, pseudo);
        records[count].next_stage = next_stage;
        records[count].score = score;
        count++;
    }

    fclose(f);
    return count;
}

int save_progress(const char *pseudo, int next_stage, long score) {
    SaveRecord records[64];
    int i;
    int count;
    int found = 0;
    FILE *f;

    if (pseudo == NULL || pseudo[0] == '\0') {
        return 0;
    }

    count = read_save_records(records, 64);

    for (i = 0; i < count; ++i) {
        if (strcmp(records[i].pseudo, pseudo) == 0) {
            if (next_stage > records[i].next_stage) {
                records[i].next_stage = next_stage;
            }
            if (score > records[i].score) {
                records[i].score = score;
            }
            found = 1;
            break;
        }
    }

    if (!found && count < 64) {
        strcpy(records[count].pseudo, pseudo);
        records[count].next_stage = next_stage;
        records[count].score = score;
        count++;
    }

    f = fopen(SAVE_FILE, "w");
    if (f == NULL) {
        return 0;
    }

    for (i = 0; i < count; ++i) {
        fprintf(f, "%s;%d;%ld\n", records[i].pseudo, records[i].next_stage, records[i].score);
    }

    fclose(f);
    return 1;
}

int load_progress(const char *pseudo, SaveRecord *out_record) {
    SaveRecord records[64];
    int i;
    int count;

    if (pseudo == NULL || out_record == NULL) {
        return 0;
    }

    count = read_save_records(records, 64);
    for (i = 0; i < count; ++i) {
        if (strcmp(records[i].pseudo, pseudo) == 0) {
            *out_record = records[i];
            return 1;
        }
    }

    return 0;
}

static int compare_scores_desc(const void *a, const void *b) {
    const ScoreRecord *sa = (const ScoreRecord *)a;
    const ScoreRecord *sb = (const ScoreRecord *)b;
    if (sa->score < sb->score) return 1;
    if (sa->score > sb->score) return -1;
    return strcmp(sa->pseudo, sb->pseudo);
}

int load_highscores(ScoreRecord *scores, int max_scores) {
    FILE *f;
    int count = 0;
    char pseudo[MAX_PSEUDO_LEN + 1];
    long score;

    if (scores == NULL || max_scores <= 0) {
        return 0;
    }

    f = fopen(SCORE_FILE, "r");
    if (f == NULL) {
        return 0;
    }

    while (count < max_scores && fscanf(f, " %31[^;];%ld", pseudo, &score) == 2) {
        strcpy(scores[count].pseudo, pseudo);
        scores[count].score = score;
        count++;
    }

    fclose(f);
    return count;
}

int update_highscores(const char *pseudo, long score) {
    ScoreRecord scores[128];
    int count = 0;
    int i;
    FILE *f;

    if (pseudo == NULL || pseudo[0] == '\0') {
        return 0;
    }

    count = load_highscores(scores, 128);
    if (count < 128) {
        strcpy(scores[count].pseudo, pseudo);
        scores[count].score = score;
        count++;
    }

    qsort(scores, count, sizeof(ScoreRecord), compare_scores_desc);

    if (count > MAX_HIGHSCORES) {
        count = MAX_HIGHSCORES;
    }

    f = fopen(SCORE_FILE, "w");
    if (f == NULL) {
        return 0;
    }

    for (i = 0; i < count; ++i) {
        fprintf(f, "%s;%ld\n", scores[i].pseudo, scores[i].score);
    }
    fclose(f);
    return 1;
}
