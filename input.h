#ifndef INPUT_H
#define INPUT_H

#include "types.h"

void input_init(void);
void input_poll(InputState *input);
int input_point_in_rect(int x, int y, RectF rect);

#endif
