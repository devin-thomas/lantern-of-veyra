#include "settings.h"

#include "audio.h"
#include "save.h"
#include "ui.h"

#define SETTINGS_BOX_X 2
#define SETTINGS_BOX_Y 5
#define SETTINGS_BOX_W 36
#define SETTINGS_BOX_H 17
#define SETTINGS_FIRST_ROW 9
#define SETTINGS_OPTION_COUNT 3

static const char *text_speed_name(u8 speed) {
    if (speed == SETTINGS_TEXT_SPEED_SLOW) return "SLOW";
    if (speed == SETTINGS_TEXT_SPEED_FAST) return "FAST";
    return "NORMAL";
}

void settings_reset_defaults(void) {
    game_save_data.text_speed = SETTINGS_TEXT_SPEED_NORMAL;
    game_save_data.music_enabled = TRUE;
    game_save_data.sfx_enabled = TRUE;
    settings_apply();
}

void settings_apply(void) {
    ui_set_text_speed(game_save_data.text_speed);
    audio_set_music_enabled(game_save_data.music_enabled != 0);
    audio_set_sfx_enabled(game_save_data.sfx_enabled != 0);
}

static void wait_for_settings_frame(void) {
    SYS_doVBlankProcess();
}

static void draw_settings_screen(u8 selection) {
    char line[41];

    ui_draw_box(SETTINGS_BOX_X, SETTINGS_BOX_Y,
                SETTINGS_BOX_W, SETTINGS_BOX_H);
    VDP_drawText("SETTINGS", 16, SETTINGS_BOX_Y);
    VDP_drawText("LEFT/RIGHT CHANGE", 10, 7);

    sprintf(line, "%sTEXT SPEED: %s", selection == 0 ? "> " : "  ",
            text_speed_name(game_save_data.text_speed));
    VDP_drawText(line, 6, SETTINGS_FIRST_ROW);
    sprintf(line, "%sMUSIC: %s", selection == 1 ? "> " : "  ",
            game_save_data.music_enabled ? "ON" : "OFF");
    VDP_drawText(line, 6, SETTINGS_FIRST_ROW + 1);
    sprintf(line, "%sSFX: %s", selection == 2 ? "> " : "  ",
            game_save_data.sfx_enabled ? "ON" : "OFF");
    VDP_drawText(line, 6, SETTINGS_FIRST_ROW + 2);
    VDP_drawText("A/B BACK", 15, 19);
}

bool settings_open(void) {
    u8 selection = 0;
    bool changed = FALSE;
    u16 previous_joy = JOY_readJoypad(JOY_1);

    draw_settings_screen(selection);
    while (1) {
        const u16 joy = JOY_readJoypad(JOY_1);
        const u16 pressed = joy & (u16) ~previous_joy;
        previous_joy = joy;

        if (pressed & BUTTON_UP) {
            selection = selection == 0 ? SETTINGS_OPTION_COUNT - 1 :
                        selection - 1;
            draw_settings_screen(selection);
        } else if (pressed & BUTTON_DOWN) {
            selection = (selection + 1) % SETTINGS_OPTION_COUNT;
            draw_settings_screen(selection);
        }

        if (pressed & (BUTTON_A | BUTTON_B | BUTTON_START)) break;

        if (pressed & (BUTTON_LEFT | BUTTON_RIGHT)) {
            const bool increase = (pressed & BUTTON_RIGHT) != 0;
            if (selection == 0) {
                if (increase) {
                    game_save_data.text_speed =
                        (game_save_data.text_speed + 1) % 3;
                } else {
                    game_save_data.text_speed =
                        game_save_data.text_speed == 0 ? 2 :
                        game_save_data.text_speed - 1;
                }
            } else if (selection == 1) {
                game_save_data.music_enabled =
                    game_save_data.music_enabled ? 0 : 1;
            } else {
                game_save_data.sfx_enabled = game_save_data.sfx_enabled ? 0 : 1;
            }
            settings_apply();
            changed = TRUE;
            draw_settings_screen(selection);
        }

        wait_for_settings_frame();
    }

    ui_clear_box();
    return changed;
}
