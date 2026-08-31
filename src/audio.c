#include "audio.h"

static AudioTheme current_theme = THEME_NONE;
static u16 note_frames_remaining = 0;
static u8 note_index = 0;
static bool note_started = FALSE;
static u8 select_sfx_frames = 0;
static u8 hit_sfx_frames = 0;
static bool music_enabled = TRUE;
static bool sfx_enabled = TRUE;

typedef struct {
    u16 tone;
    u8 frames;
} MusicNote;

// Tone values for the SN76489 divider; larger values are lower pitches.
static const MusicNote town_seq[] = {
    {477, 12}, {379, 12}, {318, 12}, {238, 12}
};
static const MusicNote dungeon_seq[] = {
    {955, 24}, {803, 24}, {637, 24}, {477, 24}
};
static const MusicNote battle_seq[] = {
    {238, 6}, {318, 6}, {379, 6}, {318, 6},
    {189, 6}, {238, 6}, {318, 12}, {159, 12}
};

static const MusicNote *audio_get_sequence(u8 *length) {
    switch (current_theme) {
        case THEME_DUNGEON:
            *length = sizeof(dungeon_seq) / sizeof(dungeon_seq[0]);
            return dungeon_seq;
        case THEME_BATTLE:
            *length = sizeof(battle_seq) / sizeof(battle_seq[0]);
            return battle_seq;
        case THEME_TOWN:
        default:
            *length = sizeof(town_seq) / sizeof(town_seq[0]);
            return town_seq;
    }
}

static void audio_play_music_note(void) {
    u8 sequence_length;
    const MusicNote *sequence = audio_get_sequence(&sequence_length);
    if (note_index >= sequence_length) note_index = 0;
    const MusicNote note = sequence[note_index];

    PSG_setTone(0, note.tone);
    PSG_setEnvelope(0, 8);
    note_frames_remaining = note.frames;
    note_started = TRUE;
}

void audio_init(void) {
    PSG_reset();
    current_theme = THEME_TOWN;
    note_frames_remaining = 0;
    note_index = 0;
    note_started = FALSE;
    select_sfx_frames = 0;
    hit_sfx_frames = 0;
    music_enabled = TRUE;
    sfx_enabled = TRUE;
}

void audio_set_theme(AudioTheme theme) {
    current_theme = theme;
    note_frames_remaining = 0;
    note_index = 0;
    note_started = FALSE;
    PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
}

void audio_set_music_enabled(bool enabled) {
    music_enabled = enabled;
    note_frames_remaining = 0;
    note_started = FALSE;
    if (!music_enabled) PSG_setEnvelope(0, PSG_ENVELOPE_MIN);
}

void audio_set_sfx_enabled(bool enabled) {
    sfx_enabled = enabled;
    select_sfx_frames = 0;
    hit_sfx_frames = 0;
    if (!sfx_enabled) {
        PSG_setEnvelope(1, PSG_ENVELOPE_MIN);
        PSG_setEnvelope(3, PSG_ENVELOPE_MIN);
    }
}

bool audio_get_music_enabled(void) {
    return music_enabled;
}

bool audio_get_sfx_enabled(void) {
    return sfx_enabled;
}

void audio_update(void) {
    if (select_sfx_frames > 0) {
        select_sfx_frames--;
        if (select_sfx_frames == 0) PSG_setEnvelope(1, PSG_ENVELOPE_MIN);
    }
    if (hit_sfx_frames > 0) {
        hit_sfx_frames--;
        if (hit_sfx_frames == 0) PSG_setEnvelope(3, PSG_ENVELOPE_MIN);
    }

    if (!music_enabled || current_theme == THEME_NONE) return;

    if (note_frames_remaining > 0) note_frames_remaining--;
    if (note_frames_remaining == 0) {
        if (note_started) {
            u8 sequence_length;
            audio_get_sequence(&sequence_length);
            note_index = (note_index + 1) % sequence_length;
        }
        audio_play_music_note();
    }
}

void play_sfx_hit(void) {
    if (!sfx_enabled) return;
    PSG_setNoise(PSG_NOISE_TYPE_WHITE, PSG_NOISE_FREQ_CLOCK8);
    PSG_setEnvelope(3, 2);
    hit_sfx_frames = 5;
}

void play_sfx_select(void) {
    if (!sfx_enabled) return;
    PSG_setTone(1, 150);
    PSG_setEnvelope(1, 2);
    select_sfx_frames = 4;
}
