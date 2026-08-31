#include "quest.h"
#include "inventory.h"
#include "save.h"

const char *quest_current_objective(void) {
    if (game_save_data.story_flags & STORY_FLAG_QUEST_COMPLETE) {
        return "QUEST COMPLETE - VEYRA IS SAFE";
    }
    if (game_save_data.story_flags & STORY_FLAG_GAME_COMPLETE) {
        return "GAME COMPLETE - RETURN TO TOWN";
    }
    if (!(game_save_data.story_flags & STORY_FLAG_QUEST_STARTED)) {
        return "QUEST: TALK TO THE GUIDE";
    }
    if (!(game_save_data.story_flags & STORY_FLAG_RELIC_FOUND)) {
        return "QUEST: FIND THE LANTERN RELIC";
    }
    return "QUEST: RETURN TO THE GUIDE";
}

bool quest_on_guide_interaction(void) {
    const u16 previous_flags = game_save_data.story_flags;

    if (!(game_save_data.story_flags & STORY_FLAG_GUIDE_MET)) {
        game_save_data.story_flags |= STORY_FLAG_GUIDE_MET |
                                      STORY_FLAG_DUNGEON_GOAL_KNOWN |
                                      STORY_FLAG_QUEST_STARTED;
    }
    if ((game_save_data.story_flags & STORY_FLAG_RELIC_FOUND) &&
        !(game_save_data.story_flags & STORY_FLAG_QUEST_COMPLETE)) {
        game_save_data.story_flags |= STORY_FLAG_QUEST_COMPLETE;
    }

    return previous_flags != game_save_data.story_flags;
}

bool quest_on_enemy_defeated(u16 enemy_level) {
    if (enemy_level < 4 ||
        !(game_save_data.story_flags & STORY_FLAG_QUEST_STARTED) ||
        (game_save_data.story_flags & STORY_FLAG_RELIC_FOUND)) {
        return FALSE;
    }

    game_save_data.story_flags |= STORY_FLAG_RELIC_FOUND;
    inventory_add_item(ITEM_ANCIENT_RELIC, 1);
    return TRUE;
}

bool quest_on_dungeon_goal(void) {
    if (!(game_save_data.story_flags & STORY_FLAG_QUEST_STARTED) ||
        (game_save_data.story_flags & STORY_FLAG_RELIC_FOUND)) {
        return FALSE;
    }

    if (inventory_get_count(ITEM_ANCIENT_RELIC) == 0 &&
        !inventory_add_item(ITEM_ANCIENT_RELIC, 1)) {
        return FALSE;
    }

    game_save_data.story_flags |= STORY_FLAG_RELIC_FOUND;
    return TRUE;
}

bool quest_on_boss_defeated(void) {
    return quest_on_dungeon_goal();
}
