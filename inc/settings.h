#ifndef SETTINGS_H
#define SETTINGS_H

#include <genesis.h>

#define SETTINGS_TEXT_SPEED_SLOW 0
#define SETTINGS_TEXT_SPEED_NORMAL 1
#define SETTINGS_TEXT_SPEED_FAST 2

void settings_reset_defaults(void);
void settings_apply(void);
bool settings_open(void);

#endif
