#ifndef AUDIO_H
#define AUDIO_H

#include <genesis.h>

// Theme types
typedef enum {
    THEME_NONE = 0,
    THEME_TOWN,
    THEME_DUNGEON,
    THEME_BATTLE
} AudioTheme;

void audio_init(void);
void audio_update(void);
void audio_set_theme(AudioTheme theme);
void audio_set_music_enabled(bool enabled);
void audio_set_sfx_enabled(bool enabled);
bool audio_get_music_enabled(void);
bool audio_get_sfx_enabled(void);

void play_sfx_hit(void);
void play_sfx_select(void);

#endif // AUDIO_H
