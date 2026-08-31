#ifndef UI_H
#define UI_H

#include <genesis.h>
#include "settings.h"

#define UI_TEXT_SPEED_SLOW SETTINGS_TEXT_SPEED_SLOW
#define UI_TEXT_SPEED_NORMAL SETTINGS_TEXT_SPEED_NORMAL
#define UI_TEXT_SPEED_FAST SETTINGS_TEXT_SPEED_FAST

void ui_init(void);
void ui_draw_box(u16 x, u16 y, u16 w, u16 h);
void ui_show_text(const char* text);
void ui_clear_box(void);
void ui_set_text_speed(u8 speed);
u8 ui_get_text_speed(void);
int ui_battle_menu(void);
int ui_skill_menu(void);

#endif // UI_H
