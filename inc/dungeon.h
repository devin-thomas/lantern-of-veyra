#ifndef DUNGEON_H
#define DUNGEON_H

#include <genesis.h>

#define DUNGEON_KEY_TILE_X 80
#define DUNGEON_KEY_TILE_Y 64
#define DUNGEON_GATE_TILE_X 64
#define DUNGEON_GATE_TILE_Y 72
#define DUNGEON_TREASURE_TILE_X 48
#define DUNGEON_TREASURE_TILE_Y 80
#define DUNGEON_CHECKPOINT_TILE_X 80
#define DUNGEON_CHECKPOINT_TILE_Y 80
#define DUNGEON_GOAL_TILE_X 64
#define DUNGEON_GOAL_TILE_Y 104

#define DUNGEON_KEY_X (DUNGEON_KEY_TILE_X * 8)
#define DUNGEON_KEY_Y (DUNGEON_KEY_TILE_Y * 8)
#define DUNGEON_GATE_X (DUNGEON_GATE_TILE_X * 8)
#define DUNGEON_GATE_Y (DUNGEON_GATE_TILE_Y * 8)
#define DUNGEON_TREASURE_X (DUNGEON_TREASURE_TILE_X * 8)
#define DUNGEON_TREASURE_Y (DUNGEON_TREASURE_TILE_Y * 8)
#define DUNGEON_CHECKPOINT_X (DUNGEON_CHECKPOINT_TILE_X * 8)
#define DUNGEON_CHECKPOINT_Y (DUNGEON_CHECKPOINT_TILE_Y * 8)
#define DUNGEON_GOAL_X (DUNGEON_GOAL_TILE_X * 8)
#define DUNGEON_GOAL_Y (DUNGEON_GOAL_TILE_Y * 8)

#define DUNGEON_AREA_UPPER 0
#define DUNGEON_AREA_LOWER 1

typedef enum {
    DUNGEON_INTERACTION_NONE = 0,
    DUNGEON_INTERACTION_HANDLED,
    DUNGEON_INTERACTION_CHANGED,
    DUNGEON_INTERACTION_BOSS
} DungeonInteractionResult;

void dungeon_init(void);
void dungeon_update(void);
DungeonInteractionResult dungeon_interact(void);
u16 dungeon_tile_override(s16 tile_x, s16 tile_y, u16 base_tile);
bool dungeon_is_safe_zone(void);
u8 dungeon_get_area(void);
u8 dungeon_get_encounter_chance_percent(void);

#endif
