#ifndef SAVE_H
#define SAVE_H

#include <genesis.h>
#include "battle.h"

#define SAVE_INVENTORY_SLOTS 8
#define SAVE_MAP_TOWN 0
#define SAVE_MAP_DUNGEON 1

#define STORY_FLAG_GUIDE_MET 0x0001
#define STORY_FLAG_WATCHER_MET 0x0002
#define STORY_FLAG_DUNGEON_GOAL_KNOWN 0x0004
#define STORY_FLAG_QUEST_STARTED 0x0008
#define STORY_FLAG_RELIC_FOUND 0x0010
#define STORY_FLAG_QUEST_COMPLETE 0x0020
#define STORY_FLAG_DUNGEON_KEY_FOUND 0x0040
#define STORY_FLAG_DUNGEON_GATE_OPEN 0x0080
#define STORY_FLAG_DUNGEON_TREASURE_OPEN 0x0100
#define STORY_FLAG_DUNGEON_CHECKPOINT 0x0200
#define STORY_FLAG_DUNGEON_GOAL_REACHED 0x0400
#define STORY_FLAG_DUNGEON_BOSS_DEFEATED 0x0800
#define STORY_FLAG_GAME_COMPLETE 0x1000

typedef struct {
    u16 map_id;
    s16 player_x;
    s16 player_y;
    Character player;
    u8 inventory[SAVE_INVENTORY_SLOTS];
    u8 equipped_weapon;
    u8 equipped_armor;
    u16 story_flags;
    u8 text_speed;
    u8 music_enabled;
    u8 sfx_enabled;
} SaveData;

extern SaveData game_save_data;

bool save_has_valid_data(void);
bool save_read(SaveData *data);
bool save_write(const SaveData *data);
void save_clear(void);

#endif
