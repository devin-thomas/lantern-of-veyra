#include "pause.h"

#include "battle.h"
#include "dungeon.h"
#include "inventory.h"
#include "map.h"
#include "quest.h"
#include "save.h"
#include "settings.h"
#include "ui.h"

#define PAUSE_BOX_X 0
#define PAUSE_BOX_Y 0
#define PAUSE_BOX_W 40
#define PAUSE_BOX_H 26

static const char *equipped_name(u8 item_id) {
    const InventoryItem *item = inventory_get_item(item_id);
    return item == NULL || item_id == ITEM_NONE ? "NONE" : item->name;
}

static const char *navigation_destination(void) {
    const u16 flags = game_save_data.story_flags;

    if (current_map_data != dungeon_map_data) return "STONEWAKE PORTAL";
    if (!(flags & STORY_FLAG_DUNGEON_KEY_FOUND)) return "RUNE KEY EAST";
    if (!(flags & STORY_FLAG_DUNGEON_GATE_OPEN)) return "SEALED GATE SOUTH";
    if (!(flags & STORY_FLAG_DUNGEON_TREASURE_OPEN)) return "TREASURE WEST";
    if (!(flags & STORY_FLAG_DUNGEON_CHECKPOINT)) return "CHECKPOINT EAST";
    if (!(flags & STORY_FLAG_DUNGEON_BOSS_DEFEATED)) return "RELIC ALTAR SOUTH";
    return "TOWN PORTAL NORTH";
}

static void draw_navigation(void) {
    char line[41];
    const bool in_dungeon = current_map_data == dungeon_map_data;

    VDP_drawText("NAVIGATION", 2, 10);
    if (in_dungeon) {
        sprintf(line, "AREA: STONEWAKE - %s HALLS",
                dungeon_get_area() == DUNGEON_AREA_LOWER ? "LOWER" : "UPPER");
        VDP_drawText(line, 2, 11);
        sprintf(line, "POSITION: %d,%d",
                (int) player.map_x, (int) player.map_y);
        VDP_drawText(line, 2, 12);
        VDP_drawText("N ^   P = YOU   V S", 2, 13);
        sprintf(line, "DEST: %s", navigation_destination());
        VDP_drawText(line, 2, 14);
        VDP_drawText("RETURN: OPEN RUNE GATE -> PORTAL", 2, 15);
    } else {
        VDP_drawText("AREA: VEYRA", 2, 11);
        sprintf(line, "POSITION: %d,%d",
                (int) player.map_x, (int) player.map_y);
        VDP_drawText(line, 2, 12);
        VDP_drawText("MAP: P = YOU  D = DUNGEON", 2, 13);
        VDP_drawText("DEST: STONEWAKE PORTAL EAST", 2, 14);
        VDP_drawText("RETURN: PORTAL FROM DUNGEON", 2, 15);
    }
}

static void draw_pause_screen(void) {
    char line[41];
    const bool in_dungeon = current_map_data == dungeon_map_data;

    ui_draw_box(PAUSE_BOX_X, PAUSE_BOX_Y, PAUSE_BOX_W, PAUSE_BOX_H);
    VDP_drawText("PAUSED / STATUS", 2, 1);
    sprintf(line, "LV%d HP %d/%d MP %d/%d",
            (int) player_stats.level, (int) player_stats.hp,
            (int) player_stats.max_hp, (int) player_stats.mp,
            (int) player_stats.max_mp);
    VDP_drawText(line, 2, 3);
    sprintf(line, "GOLD %lu  ATK %d  DEF %d",
            (unsigned long) player_stats.gold, (int) player_stats.atk,
            (int) player_stats.def);
    VDP_drawText(line, 2, 4);
    sprintf(line, "WEAPON: %s", equipped_name(game_save_data.equipped_weapon));
    VDP_drawText(line, 2, 5);
    sprintf(line, "ARMOR:  %s", equipped_name(game_save_data.equipped_armor));
    VDP_drawText(line, 2, 6);
    sprintf(line, "AREA: %s", in_dungeon ? "STONEWAKE" : "VEYRA");
    VDP_drawText(line, 2, 8);
    VDP_drawText("OBJECTIVE:", 2, 17);
    VDP_drawText(quest_current_objective(), 2, 18);
    draw_navigation();
    VDP_drawText("D-PAD MOVE  A ACTION  START SAVE", 2, 20);
    VDP_drawText("A INVENTORY  C SETTINGS  B RESUME", 2, 22);
    VDP_drawText("B OR START CLOSE", 13, 24);
}

bool pause_open(void) {
    bool changed = FALSE;
    u16 previous_joy = JOY_readJoypad(JOY_1);

    ui_init();
    draw_pause_screen();
    while (1) {
        const u16 joy = JOY_readJoypad(JOY_1);
        const u16 pressed = joy & (u16) ~previous_joy;
        previous_joy = joy;

        if (pressed & (BUTTON_B | BUTTON_START)) break;
        if (pressed & BUTTON_A) {
            if (inventory_open()) changed = TRUE;
            draw_pause_screen();
        } else if (pressed & BUTTON_C) {
            if (settings_open()) changed = TRUE;
            draw_pause_screen();
        }
        SYS_doVBlankProcess();
    }

    ui_clear_box();
    return changed;
}
