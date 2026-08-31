#include "town.h"
#include "audio.h"
#include "battle.h"
#include "economy.h"
#include "map.h"
#include "quest.h"
#include "save.h"
#include "ui.h"

#define TOWN_NPC_COUNT 4
#define TOWN_DIALOGUE_X 1
#define TOWN_DIALOGUE_Y 17
#define TOWN_DIALOGUE_W 38
#define TOWN_DIALOGUE_H 9
#define TOWN_DIALOGUE_TEXT_Y 24
#define TOWN_SCREEN_MIN_X 1
#define TOWN_SCREEN_MAX_X 38
#define TOWN_SCREEN_MIN_Y 5
#define TOWN_SCREEN_MAX_Y 23
#define TOWN_INTERACTION_RANGE 16

typedef struct {
    s16 x;
    s16 y;
    const char *label;
    u8 label_width;
} TownNpc;

static const TownNpc town_npcs[TOWN_NPC_COUNT] = {
    {TOWN_GUIDE_X, TOWN_GUIDE_Y, "G GUIDE", 7},
    {TOWN_WATCHER_X, TOWN_WATCHER_Y, "W WATCHER", 9},
    {TOWN_SHOP_X, TOWN_SHOP_Y, "S SHOP", 6},
    {TOWN_INN_X, TOWN_INN_Y, "I INN", 5}
};

static s16 drawn_x[TOWN_NPC_COUNT];
static s16 drawn_y[TOWN_NPC_COUNT];

static void clear_drawn_markers(void) {
    for (u8 i = 0; i < TOWN_NPC_COUNT; i++) {
        if (drawn_x[i] < 0) continue;
        VDP_drawText("          ", drawn_x[i], drawn_y[i]);
        drawn_x[i] = -1;
        drawn_y[i] = -1;
    }
}

static s16 world_to_screen(s16 world, s16 camera) {
    return (world - camera) / 8;
}

static bool is_near_player(const TownNpc *npc) {
    s16 dx = player.x - npc->x;
    s16 dy = player.y - npc->y;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx <= TOWN_INTERACTION_RANGE && dy <= TOWN_INTERACTION_RANGE;
}

static void wait_for_dialogue_input(void) {
    const u16 buttons = BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START;
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
    ui_draw_box(TOWN_DIALOGUE_X, TOWN_DIALOGUE_Y,
                TOWN_DIALOGUE_W, TOWN_DIALOGUE_H);
    VDP_drawText(speaker, TOWN_DIALOGUE_X + 2, TOWN_DIALOGUE_Y);

    for (u8 page = 0; page < page_count; page++) {
        ui_show_text(pages[page]);
        VDP_drawText(page + 1 < page_count ? "A NEXT" : "A CLOSE",
                     29, TOWN_DIALOGUE_TEXT_Y);
        wait_for_dialogue_input();
    }

    ui_clear_box();
}

static void show_guide_dialogue(void) {
    static const char *const first_visit[] = {
        "The old Stonewake dungeon lies beyond the stone gate at the crossroads.",
        "Find the lantern relic below, then return here alive. Press A to fight and START to save."
    };
    static const char *const returning[] = {
        "Stonewake is dangerous, but every battle makes you stronger.",
        "Rest in Veyra when your HP or MP runs low. The gate will still be here."
    };
    static const char *const completion[] = {
        "You found the lantern relic! Veyra's future is no longer sealed.",
        "Keep it safe. There are deeper ruins beyond Stonewake."
    };

    if ((game_save_data.story_flags & STORY_FLAG_RELIC_FOUND) &&
        !(game_save_data.story_flags & STORY_FLAG_QUEST_COMPLETE)) {
        show_dialogue("GUIDE", completion, 2);
    } else if (!(game_save_data.story_flags & STORY_FLAG_GUIDE_MET)) {
        show_dialogue("GUIDE", first_visit, 2);
    } else {
        show_dialogue("GUIDE", returning, 2);
    }
    quest_on_guide_interaction();
}

static void show_watcher_dialogue(void) {
    static const char *const before_goal[] = {
        "The watcher's post is quiet. Ask the guide about the relic.",
        "No one should enter the dungeon without knowing what they seek."
    };
    static const char *const early_adventure[] = {
        "The lantern relic is somewhere in Stonewake. Do not push past your limits.",
        "Bring potions, watch your MP, and return to Veyra when you need rest."
    };
    static const char *const ready_for_depths[] = {
        "You have grown strong enough to search Stonewake's lower halls.",
        "Bring back the lantern relic, and Veyra may finally have a future."
    };

    if (!(game_save_data.story_flags & STORY_FLAG_DUNGEON_GOAL_KNOWN)) {
        show_dialogue("WATCHER", before_goal, 2);
    } else if (player_stats.level < 3) {
        show_dialogue("WATCHER", early_adventure, 2);
    } else {
        show_dialogue("WATCHER", ready_for_depths, 2);
    }
    game_save_data.story_flags |= STORY_FLAG_WATCHER_MET;
}

static bool interact_with_npc(u8 npc_id) {
    if (npc_id == 0) {
        const u16 previous_flags = game_save_data.story_flags;
        show_guide_dialogue();
        return previous_flags != game_save_data.story_flags;
    }
    if (npc_id == 1) {
        const u16 previous_flags = game_save_data.story_flags;
        show_watcher_dialogue();
        return previous_flags != game_save_data.story_flags;
    }
    if (npc_id == 2) return shop_open();
    return inn_open();
}

void town_init(void) {
    for (u8 i = 0; i < TOWN_NPC_COUNT; i++) {
        drawn_x[i] = -1;
        drawn_y[i] = -1;
    }
}

void town_update(void) {
    clear_drawn_markers();
    if (current_map_data != town_map_data) return;

    for (u8 i = 0; i < TOWN_NPC_COUNT; i++) {
        const s16 screen_x = world_to_screen(town_npcs[i].x, camera_x);
        const s16 screen_y = world_to_screen(town_npcs[i].y, camera_y);
        if (screen_x < TOWN_SCREEN_MIN_X ||
            screen_x > TOWN_SCREEN_MAX_X ||
            screen_y < TOWN_SCREEN_MIN_Y ||
            screen_y > TOWN_SCREEN_MAX_Y ||
            screen_x + town_npcs[i].label_width > 40) {
            continue;
        }

        VDP_drawText(town_npcs[i].label, screen_x, screen_y);
        drawn_x[i] = screen_x;
        drawn_y[i] = screen_y;
    }
}

bool town_interact(void) {
    if (current_map_data != town_map_data) return FALSE;

    for (u8 i = 0; i < TOWN_NPC_COUNT; i++) {
        if (!is_near_player(&town_npcs[i])) continue;

        play_sfx_select();
        return interact_with_npc(i);
    }

    return FALSE;
}
