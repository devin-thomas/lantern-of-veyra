#include "ui.h"
#include "battle.h"
#include <string.h>

static u16 box_x = 0;
static u16 box_y = 0;
static u16 box_w = 0;
static u16 box_h = 0;
static u8 text_speed = UI_TEXT_SPEED_NORMAL;

void ui_init(void) {
    box_x = 0;
    box_y = 0;
    box_w = 0;
    box_h = 0;
    VDP_clearPlane(BG_A, TRUE);
}

void ui_set_text_speed(u8 speed) {
    text_speed = speed > UI_TEXT_SPEED_FAST ? UI_TEXT_SPEED_FAST : speed;
}

u8 ui_get_text_speed(void) {
    return text_speed;
}

void ui_draw_box(u16 x, u16 y, u16 w, u16 h) {
    box_x = x;
    box_y = y;
    box_w = w;
    box_h = h;

    // Draw corners
    VDP_drawText("+", x, y);
    VDP_drawText("+", x + w - 1, y);
    VDP_drawText("+", x, y + h - 1);
    VDP_drawText("+", x + w - 1, y + h - 1);

    // Draw top and bottom borders
    for(u16 i = 1; i < w - 1; i++) {
        VDP_drawText("-", x + i, y);
        VDP_drawText("-", x + i, y + h - 1);
    }

    // Draw left and right borders
    for(u16 i = 1; i < h - 1; i++) {
        VDP_drawText("|", x, y + i);
        VDP_drawText("|", x + w - 1, y + i);
    }

    // Clear interior
    for(u16 j = 1; j < h - 1; j++) {
        for(u16 i = 1; i < w - 1; i++) {
            VDP_drawText(" ", x + i, y + j);
        }
    }
}

void ui_clear_box(void) {
    if(box_w == 0 || box_h == 0) return;
    for(u16 j = 0; j < box_h; j++) {
        for(u16 i = 0; i < box_w; i++) {
            VDP_drawText(" ", box_x + i, box_y + j);
        }
    }
    box_x = 0; box_y = 0; box_w = 0; box_h = 0;
}

void ui_show_text(const char* text) {
    if(text == NULL || box_w <= 2 || box_h <= 2) return;

    u16 max_chars_per_line = box_w - 2;
    u16 max_lines = box_h - 2;

    u16 cx = box_x + 1;
    u16 cy = box_y + 1;

    // Clear interior first
    for(u16 j = 1; j < box_h - 1; j++) {
        for(u16 i = 1; i < box_w - 1; i++) {
            VDP_drawText(" ", box_x + i, box_y + j);
        }
    }

    const char* ptr = text;
    char line[40];
    u16 line_idx = 0;

    while(*ptr != '\0' && line_idx < max_lines) {
        const char* line_start = ptr;
        u16 length = 0;
        u16 last_space = 0;

        while (ptr[length] != '\0' && ptr[length] != '\n' &&
               length < max_chars_per_line) {
            if (ptr[length] == ' ') last_space = length + 1;
            length++;
        }

        if (ptr[length] != '\0' && ptr[length] != '\n' &&
            length == max_chars_per_line && last_space > 0) {
            // Keep words intact when a message is longer than the box.
            length = last_space - 1;
            ptr += last_space;
        } else {
            ptr += length;
            if (*ptr == '\n') ptr++;
        }

        memcpy(line, line_start, length);
        line[length] = '\0';
        VDP_drawText(line, cx, cy + line_idx);
        const u8 delay_frames = text_speed == UI_TEXT_SPEED_SLOW ? 3 :
                                text_speed == UI_TEXT_SPEED_NORMAL ? 1 : 0;
        for (u8 frame = 0; frame < delay_frames; frame++) {
            SYS_doVBlankProcess();
        }
        line_idx++;
    }
}

int ui_battle_menu(void) {
    static const char* labels[] = {"FIGHT", "SKILL", "ITEM", "GUARD", "RUN"};
    static const u16 option_x[] = {3, 11, 19, 3, 11};
    static const u16 option_y[] = {9, 9, 9, 11, 11};
    int selection = 0;

    ui_draw_box(1, 8, 28, 7);
    for (u16 i = 0; i < 5; i++) {
        VDP_drawText(labels[i], option_x[i], option_y[i]);
    }
    VDP_drawText(">", option_x[selection] - 1, option_y[selection]);

    u16 prev_state = JOY_readJoypad(JOY_1);

    while(1) {
        u16 state = JOY_readJoypad(JOY_1);
        u16 changed = state ^ prev_state;
        u16 pressed = state & changed;
        prev_state = state;

        int previous_selection = selection;
        if (pressed & BUTTON_UP) {
            if (selection == 3) selection = 0;
            else if (selection == 4) selection = 1;
        } else if (pressed & BUTTON_DOWN) {
            if (selection == 0) selection = 3;
            else if (selection == 1) selection = 4;
        } else if (pressed & BUTTON_LEFT) {
            if (selection == 1) selection = 0;
            else if (selection == 2) selection = 1;
            else if (selection == 4) selection = 3;
        } else if (pressed & BUTTON_RIGHT) {
            if (selection == 0) selection = 1;
            else if (selection == 1) selection = 2;
            else if (selection == 3) selection = 4;
        }

        if (selection != previous_selection) {
            VDP_drawText(" ", option_x[previous_selection] - 1,
                         option_y[previous_selection]);
            VDP_drawText(">", option_x[selection] - 1, option_y[selection]);
        }

        if (pressed & (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START)) {
            break;
        }

        SYS_doVBlankProcess();
    }

    ui_clear_box();

    // Keep public battle actions distinct from the zero-based cursor index.
    return selection + 1;
}

int ui_skill_menu(void) {
    static const char* labels[] = {"HEAL 3MP", "POWER 4MP"};
    static const u16 option_x[] = {10, 10};
    static const u16 option_y[] = {11, 13};
    int selection = 0;

    ui_draw_box(7, 9, 26, 7);
    for (u16 i = 0; i < 2; i++) {
        VDP_drawText(labels[i], option_x[i], option_y[i]);
    }
    VDP_drawText(">", option_x[selection] - 1, option_y[selection]);

    u16 prev_state = JOY_readJoypad(JOY_1);
    while (1) {
        const u16 state = JOY_readJoypad(JOY_1);
        const u16 changed = state ^ prev_state;
        const u16 pressed = state & changed;
        prev_state = state;

        if (pressed & (BUTTON_UP | BUTTON_DOWN)) {
            const int previous_selection = selection;
            selection = selection == 0 ? 1 : 0;
            VDP_drawText(" ", option_x[previous_selection] - 1,
                         option_y[previous_selection]);
            VDP_drawText(">", option_x[selection] - 1, option_y[selection]);
        }

        if (pressed & BUTTON_A) {
            ui_clear_box();
            return selection + 1;
        }
        if (pressed & (BUTTON_B | BUTTON_C | BUTTON_START)) {
            ui_clear_box();
            return 0;
        }

        SYS_doVBlankProcess();
    }
}
