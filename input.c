#include <string.h>
#include <ctype.h>
#include <allegro.h>
#include "input.h"

static int g_prev_up = 0;
static int g_prev_down = 0;
static int g_prev_enter = 0;
static int g_prev_esc = 0;
static int g_prev_backspace = 0;
static int g_prev_delete = 0;
static int g_prev_mouse_left = 0;

void input_init(void) {
    g_prev_up = 0;
    g_prev_down = 0;
    g_prev_enter = 0;
    g_prev_esc = 0;
    g_prev_backspace = 0;
    g_prev_delete = 0;
    g_prev_mouse_left = 0;
}

static int read_scancode_char(int keycode) {
    int c = keycode & 0xFF;
    if (c >= 32 && c <= 126) {
        return c;
    }
    return 0;
}

void input_poll(InputState *input) {
    int current_up;
    int current_down;
    int current_enter;
    int current_esc;
    int current_backspace;
    int current_delete;
    int current_mouse_left;

    memset(input, 0, sizeof(*input));

    input->left = key[KEY_LEFT];
    input->right = key[KEY_RIGHT];
    input->fire = key[KEY_SPACE];
    input->mouse_x = mouse_x;
    input->mouse_y = mouse_y;
    input->mouse_left_down = (mouse_b & 1) ? 1 : 0;

    current_up = key[KEY_UP];
    current_down = key[KEY_DOWN];
    current_enter = key[KEY_ENTER];
    current_esc = key[KEY_ESC];
    current_backspace = key[KEY_BACKSPACE];
    current_delete = key[KEY_DEL];
    current_mouse_left = input->mouse_left_down;

    input->up_pressed = current_up && !g_prev_up;
    input->down_pressed = current_down && !g_prev_down;
    input->confirm_pressed = current_enter && !g_prev_enter;
    input->enter_pressed = input->confirm_pressed;
    input->back_pressed = current_esc && !g_prev_esc;
    input->esc_pressed = input->back_pressed;
    input->backspace_pressed = current_backspace && !g_prev_backspace;
    input->delete_pressed = current_delete && !g_prev_delete;
    input->mouse_left_pressed = current_mouse_left && !g_prev_mouse_left;

    while (keypressed() && input->typed_count < MAX_TYPED_CHARS) {
        int value = readkey();
        int c = read_scancode_char(value);
        if (c != 0) {
            if (isprint(c)) {
                input->typed_chars[input->typed_count++] = (char)c;
            }
        }
    }

    g_prev_up = current_up;
    g_prev_down = current_down;
    g_prev_enter = current_enter;
    g_prev_esc = current_esc;
    g_prev_backspace = current_backspace;
    g_prev_delete = current_delete;
    g_prev_mouse_left = current_mouse_left;
}

int input_point_in_rect(int x, int y, RectF rect) {
    return x >= (int)rect.x &&
           x <= (int)(rect.x + rect.w) &&
           y >= (int)rect.y &&
           y <= (int)(rect.y + rect.h);
}
