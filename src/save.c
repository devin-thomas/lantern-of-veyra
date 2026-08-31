#include "save.h"
#include "settings.h"

#define SAVE_MAGIC 0x4A52
#define SAVE_VERSION 3

typedef struct {
    u16 magic;
    u8 version;
    u8 reserved;
    u16 checksum;
    SaveData data;
} StoredSave;

SaveData game_save_data;

static u16 save_checksum(const StoredSave *stored) {
    const u8 *bytes = (const u8 *) stored;
    u16 checksum = 0xA55A;

    for (u16 i = 0; i < sizeof(StoredSave); i++) {
        if (i == 4 || i == 5) continue;
        checksum = (u16) ((checksum << 1) | (checksum >> 15));
        checksum ^= bytes[i];
    }

    return checksum;
}

static bool save_data_is_sane(const SaveData *data) {
    if (data == NULL) return FALSE;
    if (data->map_id > SAVE_MAP_DUNGEON) return FALSE;
    if (data->player_x < 0 || data->player_x > 1016) return FALSE;
    if (data->player_y < 0 || data->player_y > 1016) return FALSE;
    if (data->player.level == 0 || data->player.level > 99) return FALSE;
    if (data->player.max_hp <= 0 || data->player.hp < 0 ||
        data->player.hp > data->player.max_hp) {
        return FALSE;
    }
    if (data->player.max_mp < 0 || data->player.mp < 0 ||
        data->player.mp > data->player.max_mp) {
        return FALSE;
    }
    if (data->text_speed > SETTINGS_TEXT_SPEED_FAST ||
        data->music_enabled > 1 || data->sfx_enabled > 1) {
        return FALSE;
    }
    return TRUE;
}

bool save_read(SaveData *data) {
    StoredSave stored = {0};
    u8 *bytes = (u8 *) &stored;

    if (data == NULL) return FALSE;

    SRAM_enableRO();
    for (u16 i = 0; i < sizeof(StoredSave); i++) {
        bytes[i] = SRAM_readByte(i);
    }
    SRAM_disable();

    if (stored.magic != SAVE_MAGIC ||
        stored.version != SAVE_VERSION ||
        stored.checksum != save_checksum(&stored) ||
        !save_data_is_sane(&stored.data)) {
        return FALSE;
    }

    *data = stored.data;
    game_save_data = stored.data;
    return TRUE;
}

bool save_has_valid_data(void) {
    SaveData ignored;
    return save_read(&ignored);
}

bool save_write(const SaveData *data) {
    StoredSave stored = {0};
    const u8 *bytes;

    if (!save_data_is_sane(data)) return FALSE;

    stored.magic = SAVE_MAGIC;
    stored.version = SAVE_VERSION;
    stored.data = *data;
    stored.checksum = save_checksum(&stored);
    bytes = (const u8 *) &stored;

    SRAM_enable();
    for (u16 i = 0; i < sizeof(StoredSave); i++) {
        SRAM_writeByte(i, bytes[i]);
    }
    SRAM_disable();

    game_save_data = *data;
    return TRUE;
}

void save_clear(void) {
    SRAM_enable();
    for (u16 i = 0; i < sizeof(StoredSave); i++) {
        SRAM_writeByte(i, 0);
    }
    SRAM_disable();
    game_save_data = (SaveData) {0};
}
