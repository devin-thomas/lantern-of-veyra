#include "dungeon.h"

#include "audio.h"
#include "battle.h"
#include "inventory.h"
#include "map.h"
#include "quest.h"
#include "save.h"
#include "ui.h"

#define DUNGEON_MARKER_COUNT 5
#define DUNGEON_DIALOGUE_X 1
#define DUNGEON_DIALOGUE_Y 17
#define DUNGEON_DIALOGUE_W 38
#define DUNGEON_DIALOGUE_H 9
#define DUNGEON_DIALOGUE_TEXT_Y 24
#define DUNGEON_SCREEN_MIN_X 1
#define DUNGEON_SCREEN_MAX_X 38
#define DUNGEON_SCREEN_MIN_Y 5
#define DUNGEON_SCREEN_MAX_Y 23
#define DUNGEON_INTERACTION_RANGE 16
#define DUNGEON_STONE_TILE 3
#define DUNGEON_TREASURE_GOLD 25

typedef struct {
    s16 x;
    s16 y;
    const char *label;
    u8 label_width;
    bool visible;
} DungeonMarker;

static s16 drawn_x[DUNGEON_MARKER_COUNT];
static s16 drawn_y[DUNGEON_MARKER_COUNT];

static s16 world_to_screen(s16 world, s16 camera) {
    return (world - camera) / 8;
}

static void clear_drawn_markers(void) {
    for (u8 i = 0; i < DUNGEON_MARKER_COUNT; i++) {
        if (drawn_x[i] < 0) continue;
        VDP_drawText("              ", drawn_x[i], drawn_y[i]);
        drawn_x[i] = -1;
        drawn_y[i] = -1;
    }
}

static bool is_near_player(s16 x, s16 y) {
    s16 dx = player.x - x;
    s16 dy = player.y - y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx <= DUNGEON_INTERACTION_RANGE &&
           dy <= DUNGEON_INTERACTION_RANGE;
}

static void wait_for_dialogue_input(void) {
    const u16 buttons = BUTTON_UP | BUTTON_DOWN | BUTTON_LEFT | BUTTON_RIGHT |
                        BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START;
    u16 joy;

    do {
        joy = JOY_readJoypad(JOY_1);
        SYS_doVBlankProcess();
    } while (joy & buttons);

    do {
        joy = JOY_readJoypad(JOY_1);
        SYS_doVBlankProcess();
    } while (!(joy & buttons));
}

static void show_dialogue(const char *speaker,
                          const char *const *pages,
                          u8 page_count) {
    ui_draw_box(DUNGEON_DIALOGUE_X, DUNGEON_DIALOGUE_Y,
                DUNGEON_DIALOGUE_W, DUNGEON_DIALOGUE_H);
    VDP_drawText(speaker, DUNGEON_DIALOGUE_X + 2, DUNGEON_DIALOGUE_Y);

    for (u8 page = 0; page < page_count; page++) {
        ui_show_text(pages[page]);
        VDP_drawText(page + 1 < page_count ? "A NEXT" : "A CLOSE",
                     29, DUNGEON_DIALOGUE_TEXT_Y);
        wait_for_dialogue_input();
    }

    ui_clear_box();
}

static DungeonMarker marker_for_index(u8 index) {
    DungeonMarker marker = {0};
    marker.visible = TRUE;

    if (index == 0) {
        marker.x = DUNGEON_KEY_X;
        marker.y = DUNGEON_KEY_Y;
        marker.label = "K RUNE KEY";
        marker.label_width = 10;
        marker.visible = !(game_save_data.story_flags &
                           STORY_FLAG_DUNGEON_KEY_FOUND);
    } else if (index == 1) {
        marker.x = DUNGEON_GATE_X;
        marker.y = DUNGEON_GATE_Y;
        marker.label = (game_save_data.story_flags & STORY_FLAG_DUNGEON_GATE_OPEN) ?
                       "O OPEN GATE" : "X SEALED GATE";
        marker.label_width = 13;
    } else if (index == 2) {
        marker.x = DUNGEON_TREASURE_X;
        marker.y = DUNGEON_TREASURE_Y;
        marker.label = "T TREASURE";
        marker.label_width = 10;
        marker.visible = !(game_save_data.story_flags &
                           STORY_FLAG_DUNGEON_TREASURE_OPEN);
    } else if (index == 3) {
        marker.x = DUNGEON_CHECKPOINT_X;
        marker.y = DUNGEON_CHECKPOINT_Y;
        marker.label = "C CHECKPOINT";
        marker.label_width = 12;
    } else {
        marker.x = DUNGEON_GOAL_X;
        marker.y = DUNGEON_GOAL_Y;
        if (game_save_data.story_flags & STORY_FLAG_DUNGEON_BOSS_DEFEATED) {
            marker.label = "A ALTAR CLEAR";
        } else if (game_save_data.story_flags & STORY_FLAG_DUNGEON_GOAL_REACHED) {
            marker.label = "A DRAGON ALTAR";
        } else {
            marker.label = "A RELIC ALTAR";
        }
        marker.label_width = 13;
    }

    return marker;
}

void dungeon_init(void) {
    for (u8 i = 0; i < DUNGEON_MARKER_COUNT; i++) {
        drawn_x[i] = -1;
        drawn_y[i] = -1;
    }
}

void dungeon_update(void) {
    clear_drawn_markers();
    if (current_map_data != dungeon_map_data) return;

    for (u8 i = 0; i < DUNGEON_MARKER_COUNT; i++) {
        const DungeonMarker marker = marker_for_index(i);
        if (!marker.visible) continue;

        const s16 screen_x = world_to_screen(marker.x, camera_x);
        const s16 screen_y = world_to_screen(marker.y, camera_y);
        if (screen_x < DUNGEON_SCREEN_MIN_X ||
            screen_x > DUNGEON_SCREEN_MAX_X ||
            screen_y < DUNGEON_SCREEN_MIN_Y ||
            screen_y > DUNGEON_SCREEN_MAX_Y ||
            screen_x + marker.label_width > 40) {
            continue;
        }

        VDP_drawText(marker.label, screen_x, screen_y);
        drawn_x[i] = screen_x;
        drawn_y[i] = screen_y;
    }
}

u16 dungeon_tile_override(s16 tile_x, s16 tile_y, u16 base_tile) {
    if (tile_x == DUNGEON_GATE_TILE_X && tile_y == DUNGEON_GATE_TILE_Y &&
        (game_save_data.story_flags & STORY_FLAG_DUNGEON_GATE_OPEN)) {
        return DUNGEON_STONE_TILE;
    }
    return base_tile;
}

bool dungeon_is_safe_zone(void) {
    if (current_map_data != dungeon_map_data) return FALSE;
    return is_near_player(DUNGEON_KEY_X, DUNGEON_KEY_Y) ||
           is_near_player(DUNGEON_GATE_X, DUNGEON_GATE_Y) ||
           is_near_player(DUNGEON_TREASURE_X, DUNGEON_TREASURE_Y) ||
           is_near_player(DUNGEON_CHECKPOINT_X, DUNGEON_CHECKPOINT_Y) ||
           is_near_player(DUNGEON_GOAL_X, DUNGEON_GOAL_Y);
}

u8 dungeon_get_area(void) {
    return player.map_y >= DUNGEON_GATE_TILE_Y ? DUNGEON_AREA_LOWER :
                                                 DUNGEON_AREA_UPPER;
}

u8 dungeon_get_encounter_chance_percent(void) {
    return dungeon_get_area() == DUNGEON_AREA_LOWER ? 18 : 10;
}

static DungeonInteractionResult interact_with_key(void) {
    static const char *const pages[] = {
        "A rune key rests in Stonewake's eastern chamber.",
        "The sealed gate below may yield to it."
    };
    static const char *const unusable[] = {
        "The rune key is here, but it cannot be carried.",
        "This save cannot hold the key forward."
    };

    if (game_save_data.story_flags & STORY_FLAG_DUNGEON_KEY_FOUND) {
        return DUNGEON_INTERACTION_HANDLED;
    }

    if (!inventory_add_item(ITEM_RUNE_KEY, 1)) {
        show_dialogue("DUNGEON", unusable, 2);
        return DUNGEON_INTERACTION_HANDLED;
    }

    game_save_data.story_flags |= STORY_FLAG_DUNGEON_KEY_FOUND;
    play_sfx_select();
    show_dialogue("DUNGEON", pages, 2);
    return DUNGEON_INTERACTION_CHANGED;
}

static DungeonInteractionResult interact_with_gate(void) {
    static const char *const sealed_pages[] = {
        "The gate is sealed by an ancient rune.",
        "Search the eastern chamber for something that fits."
    };
    static const char *const open_pages[] = {
        "The rune key turns inside the lock.",
        "The lower halls are open. Keep the route back to town in mind."
    };

    if (game_save_data.story_flags & STORY_FLAG_DUNGEON_GATE_OPEN) {
        return DUNGEON_INTERACTION_HANDLED;
    }
    if (!(game_save_data.story_flags & STORY_FLAG_DUNGEON_KEY_FOUND)) {
        show_dialogue("GATE", sealed_pages, 2);
        return DUNGEON_INTERACTION_HANDLED;
    }

    game_save_data.story_flags |= STORY_FLAG_DUNGEON_GATE_OPEN;
    play_sfx_select();
    show_dialogue("GATE", open_pages, 2);
    return DUNGEON_INTERACTION_CHANGED;
}

static DungeonInteractionResult interact_with_treasure(void) {
    static const char *const already_open[] = {
        "The treasure chest is empty.",
        "Its supplies helped you reach the lower halls."
    };
    static const char *const iron_sword[] = {
        "The chest contains an iron sword and 25 gold.",
        "Equip the sword from the status screen when you are ready."
    };
    static const char *const chain_armor[] = {
        "The chest contains chain armor and 25 gold.",
        "Equip it from the status screen when you are ready."
    };
    static const char *const supplies[] = {
        "The chest contains three potions and 25 gold.",
        "The lower halls will not be kind, so use them wisely."
    };
    static const char *const gold_only[] = {
        "The chest contains 25 gold.",
        "Your pack is full, but the gold will still help."
    };

    if (game_save_data.story_flags & STORY_FLAG_DUNGEON_TREASURE_OPEN) {
        show_dialogue("CHEST", already_open, 2);
        return DUNGEON_INTERACTION_HANDLED;
    }

    const char *const *pages = gold_only;
    if (inventory_get_count(ITEM_IRON_SWORD) == 0) {
        if (!inventory_add_item(ITEM_IRON_SWORD, 1)) {
            return DUNGEON_INTERACTION_HANDLED;
        }
        pages = iron_sword;
    } else if (inventory_get_count(ITEM_CHAIN_ARMOR) == 0) {
        if (!inventory_add_item(ITEM_CHAIN_ARMOR, 1)) {
            return DUNGEON_INTERACTION_HANDLED;
        }
        pages = chain_armor;
    } else {
        const u8 potion_count = inventory_get_count(ITEM_POTION);
        const u8 potion_room = potion_count < 99 ? 99 - potion_count : 0;
        const u8 potion_reward = potion_room < 3 ? potion_room : 3;
        if (potion_reward > 0) {
            inventory_add_item(ITEM_POTION, potion_reward);
            pages = supplies;
        }
    }

    if (player_stats.gold <= 0xFFFFFFFFUL - DUNGEON_TREASURE_GOLD) {
        player_stats.gold += DUNGEON_TREASURE_GOLD;
    } else {
        player_stats.gold = 0xFFFFFFFFUL;
    }
    game_save_data.story_flags |= STORY_FLAG_DUNGEON_TREASURE_OPEN;
    play_sfx_select();
    show_dialogue("CHEST", pages, 2);
    return DUNGEON_INTERACTION_CHANGED;
}

static DungeonInteractionResult interact_with_checkpoint(void) {
    static const char *const first_visit_pages[] = {
        "The checkpoint stone warms beneath your hand.",
        "HP and MP are restored. This discovery is saved."
    };
    static const char *const returning_pages[] = {
        "The checkpoint restores your strength.",
        "The rune gate remains open, so the way home is safe."
    };
    const bool first_visit = !(game_save_data.story_flags &
                               STORY_FLAG_DUNGEON_CHECKPOINT);
    const bool needs_recovery = player_stats.hp != player_stats.max_hp ||
                                player_stats.mp != player_stats.max_mp;

    game_save_data.story_flags |= STORY_FLAG_DUNGEON_CHECKPOINT;
    player_restore_at_town();
    play_sfx_select();
    show_dialogue("CHECKPOINT", first_visit ? first_visit_pages : returning_pages,
                  2);
    return first_visit || needs_recovery ? DUNGEON_INTERACTION_CHANGED :
                                           DUNGEON_INTERACTION_HANDLED;
}

static DungeonInteractionResult interact_with_goal(void) {
    static const char *const locked_pages[] = {
        "The altar is dormant. Search every lower-hall landmark.",
        "Open the gate, claim the treasure, and touch the checkpoint."
    };
    static const char *const awaken_pages[] = {
        "The relic altar awakens. The Ash Dragon guards the lantern relic.",
        "Prepare yourself. Press A to face the Ash Dragon."
    };
    static const char *const clear_pages[] = {
        "The altar is quiet now. The Ash Dragon is gone.",
        "Return through the open gate and bring the lantern relic to Veyra."
    };
    const u16 required_flags = STORY_FLAG_QUEST_STARTED |
                               STORY_FLAG_DUNGEON_GATE_OPEN |
                               STORY_FLAG_DUNGEON_TREASURE_OPEN |
                               STORY_FLAG_DUNGEON_CHECKPOINT;

    if (game_save_data.story_flags & STORY_FLAG_DUNGEON_BOSS_DEFEATED) {
        show_dialogue("ALTAR", clear_pages, 2);
        return DUNGEON_INTERACTION_HANDLED;
    }
    if ((game_save_data.story_flags & required_flags) != required_flags) {
        show_dialogue("ALTAR", locked_pages, 2);
        return DUNGEON_INTERACTION_HANDLED;
    }

    game_save_data.story_flags |= STORY_FLAG_DUNGEON_GOAL_REACHED;
    play_sfx_select();
    show_dialogue("ALTAR", awaken_pages, 2);
    return DUNGEON_INTERACTION_BOSS;
}

DungeonInteractionResult dungeon_interact(void) {
    if (current_map_data != dungeon_map_data) {
        return DUNGEON_INTERACTION_NONE;
    }

    DungeonInteractionResult result = DUNGEON_INTERACTION_NONE;
    if (is_near_player(DUNGEON_KEY_X, DUNGEON_KEY_Y)) {
        result = interact_with_key();
    } else if (is_near_player(DUNGEON_GATE_X, DUNGEON_GATE_Y)) {
        result = interact_with_gate();
    } else if (is_near_player(DUNGEON_TREASURE_X, DUNGEON_TREASURE_Y)) {
        result = interact_with_treasure();
    } else if (is_near_player(DUNGEON_CHECKPOINT_X, DUNGEON_CHECKPOINT_Y)) {
        result = interact_with_checkpoint();
    } else if (is_near_player(DUNGEON_GOAL_X, DUNGEON_GOAL_Y)) {
        result = interact_with_goal();
    }

    if (result != DUNGEON_INTERACTION_NONE) {
        map_reset_encounter_meter();
    }
    return result;
}
