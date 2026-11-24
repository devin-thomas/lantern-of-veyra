#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genesis.h"
#include "sgdkstub/sgdkstub.h"
#include "map.h"
#include "battle.h"
#include "economy.h"
#include "ui.h"
#include "audio.h"
#include "inventory.h"
#include "quest.h"
#include "save.h"
#include "town.h"
#include "dungeon.h"
#include "pause.h"
#include "settings.h"

/* The Genesis calls are the sgdk-host-stub package's recorder (sgs). */
static void reset_render_trace(void) {
    sgs.hscroll[BG_B] = 0;
    sgs.vscroll[BG_B] = 0;
    sgs.tile_writes = 0;
    sgs.max_tile_y = 0;
}

static void reset_psg_trace(void) {
    sgs.psg_log_length = 0;
}

static void set_joy_sequence(const u16 *sequence, size_t length) {
    sgs_joy_set(JOY_1, 0);
    sgs_joy_script(JOY_1, sequence, (u16) length);
}

static int expect_true(const char *name, int condition) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", name);
        return 1;
    }
    printf("PASS: %s\n", name);
    return 0;
}

int main(void) {
    int failures = 0;
    const u16 right[] = {BUTTON_RIGHT};
    const u16 walk_right[] = {BUTTON_RIGHT, BUTTON_RIGHT,
                              BUTTON_RIGHT, BUTTON_RIGHT};
    const u16 confirm[] = {0, BUTTON_A};
    const u16 attack_and_ack[] = {0, BUTTON_A, 0, BUTTON_B};
    const u16 enemy_action_ack[] = {0, BUTTON_B, 0, BUTTON_B};
    const u16 skill_and_ack[] = {0, BUTTON_RIGHT, 0, BUTTON_A,
                                  0, BUTTON_A, 0, BUTTON_B};
    const u16 item_and_ack[] = {0, BUTTON_RIGHT, 0, BUTTON_RIGHT,
                                0, BUTTON_A, 0, BUTTON_B};
    const u16 power_no_mp[] = {0, BUTTON_RIGHT, 0, BUTTON_A,
                               0, BUTTON_DOWN, 0, BUTTON_A,
                               0, BUTTON_B};
    const u16 power_victory[] = {0, BUTTON_RIGHT, 0, BUTTON_A,
                                 0, BUTTON_DOWN, 0, BUTTON_A,
                                 0, BUTTON_B, 0, BUTTON_B};
    const u16 boss_phase[] = {0, BUTTON_B, 0, BUTTON_B, 0, BUTTON_B};
    const u16 boss_victory[] = {0, BUTTON_RIGHT, 0, BUTTON_A,
                                0, BUTTON_DOWN, 0, BUTTON_A,
                                0, BUTTON_B, 0, BUTTON_B,
                                0, BUTTON_B, 0, BUTTON_B,
                                0, BUTTON_B, 0, BUTTON_B};
    const u16 guard_and_ack[] = {0, BUTTON_DOWN, 0, BUTTON_A, 0, BUTTON_B};
    const u16 run_menu[] = {0, BUTTON_DOWN, 0, BUTTON_RIGHT, 0, BUTTON_A};
    const u16 pause_close[] = {0, BUTTON_B};
    const u16 settings_speed[] = {0, BUTTON_RIGHT, 0, BUTTON_B};
    const u16 settings_music[] = {0, BUTTON_DOWN, 0, BUTTON_RIGHT,
                                  0, BUTTON_B};
    const u16 victory[] = {0, BUTTON_A, 0, BUTTON_B, 0, BUTTON_B};
    const u16 dialogue_pages[] = {0, BUTTON_A, 0, BUTTON_A};
    const u16 inventory_menu[] = {0, BUTTON_DOWN, 0, BUTTON_A, 0, BUTTON_B};
    const u16 shop_menu[] = {0, BUTTON_A, 0, BUTTON_A, 0, BUTTON_B};

    sgs.tiles[BG_B][0][0] = 0;
    map_init();
    set_joy_sequence(right, 1);
    map_update_player();
    failures += expect_true("directional input moves the player", player.x > 64);
    failures += expect_true("map tiles use the reserved user VRAM base",
                            sgs.tiles[BG_B][0][0] == TILE_USER_INDEX + 11);
    failures += expect_true("town movement never triggers encounters",
                            !map_take_encounter_trigger());

    town_init();
    game_save_data = (SaveData) {0};
    player.x = TOWN_GUIDE_X;
    player.y = TOWN_GUIDE_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("guide interaction opens paged dialogue and sets goal",
                            town_interact() &&
                            (game_save_data.story_flags & STORY_FLAG_GUIDE_MET) &&
                            (game_save_data.story_flags & STORY_FLAG_DUNGEON_GOAL_KNOWN) &&
                            (game_save_data.story_flags & STORY_FLAG_QUEST_STARTED) &&
                            strcmp(quest_current_objective(),
                                   "QUEST: FIND THE LANTERN RELIC") == 0);
    player.x = TOWN_WATCHER_X;
    player.y = TOWN_WATCHER_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("watcher interaction exposes progression state",
                            town_interact() &&
                            (game_save_data.story_flags & STORY_FLAG_WATCHER_MET));

    player_reset();
    failures += expect_true("defeating the relic guardian advances the quest",
                            quest_on_enemy_defeated(4) &&
                            (game_save_data.story_flags & STORY_FLAG_RELIC_FOUND) &&
                            inventory_get_count(ITEM_ANCIENT_RELIC) == 1 &&
                            strcmp(quest_current_objective(),
                                   "QUEST: RETURN TO THE GUIDE") == 0);
    player.x = TOWN_GUIDE_X;
    player.y = TOWN_GUIDE_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("returning to the guide completes the quest",
                            town_interact() &&
                            (game_save_data.story_flags & STORY_FLAG_QUEST_COMPLETE) &&
                            strcmp(quest_current_objective(),
                                   "QUEST COMPLETE - VEYRA IS SAFE") == 0);

    player_reset();
    failures += expect_true("new players receive usable starter equipment",
                            inventory_get_count(ITEM_POTION) == 2 &&
                            inventory_get_count(ITEM_BRONZE_SWORD) == 1 &&
                            inventory_get_count(ITEM_LEATHER_ARMOR) == 1 &&
                            player_stats.atk > player_stats.base_atk &&
                            player_stats.def > player_stats.base_def);
    const u16 stats_before_invalid_gear = player_stats.atk;
    failures += expect_true("invalid equipment leaves stats unchanged",
                            !inventory_toggle_equipment(ITEM_IRON_SWORD) &&
                            player_stats.atk == stats_before_invalid_gear);
    failures += expect_true("equipping a found weapon recalculates attack",
                            inventory_add_item(ITEM_IRON_SWORD, 1) &&
                            inventory_toggle_equipment(ITEM_IRON_SWORD) &&
                            player_stats.atk == player_stats.base_atk + 4);
    player_stats.hp = 5;
    failures += expect_true("using a potion updates health and count",
                            inventory_use_potion() &&
                            player_stats.hp > 5 &&
                            inventory_get_count(ITEM_POTION) == 1);
    player_reset();
    set_joy_sequence(inventory_menu, 6);
    failures += expect_true("status menu can unequip selected gear",
                            inventory_open() &&
                            game_save_data.equipped_weapon == ITEM_NONE &&
                            player_stats.atk == player_stats.base_atk);

    player_reset();
    player_stats.gold = 4;
    const u8 potions_before_purchase = inventory_get_count(ITEM_POTION);
    failures += expect_true("shop rejects insufficient gold without mutation",
                            !shop_purchase(ITEM_POTION) &&
                            player_stats.gold == 4 &&
                            inventory_get_count(ITEM_POTION) ==
                                potions_before_purchase);
    player_stats.gold = 100;
    player.x = TOWN_SHOP_X;
    player.y = TOWN_SHOP_Y;
    set_joy_sequence(shop_menu, 6);
    failures += expect_true("shop confirmation buys a potion and charges gold",
                            town_interact() &&
                            player_stats.gold == 95 &&
                            inventory_get_count(ITEM_POTION) == 3);
    const u32 gold_before_invalid_offer = player_stats.gold;
    failures += expect_true("shop rejects non-catalog items without mutation",
                            !shop_purchase(ITEM_ANCIENT_RELIC) &&
                            player_stats.gold == gold_before_invalid_offer &&
                            inventory_get_count(ITEM_ANCIENT_RELIC) == 0);
    player_stats.gold = INN_PRICE;
    player_stats.hp = 1;
    player_stats.mp = 0;
    failures += expect_true("inn restores both resources for its price",
                            inn_rest() &&
                            player_stats.gold == 0 &&
                            player_stats.hp == player_stats.max_hp &&
                            player_stats.mp == player_stats.max_mp);
    player_stats.gold = INN_PRICE - 1;
    player_stats.hp = 1;
    failures += expect_true("inn rejects insufficient gold without mutation",
                            !inn_rest() &&
                            player_stats.gold == INN_PRICE - 1 &&
                            player_stats.hp == 1);

    player.x = 320;
    player.y = 320;
    player.map_x = player.x / 8;
    player.map_y = player.y / 8;
    reset_render_trace();
    set_joy_sequence(NULL, 0);
    map_update_player();
    failures += expect_true("vertical scroll follows the camera",
                            sgs.vscroll[BG_B] == camera_y);
    failures += expect_true("horizontal scroll follows the camera",
                            sgs.hscroll[BG_B] == -camera_x);
    failures += expect_true("map rendering stays inside the 64x32 plane",
                            sgs.max_tile_y < 32);
    reset_render_trace();
    map_update_player();
    failures += expect_true("map tiles are not redrawn every frame",
                            sgs.tile_writes == 0);
    const s16 previous_y = player.y;
    const u16 down[] = {BUTTON_DOWN};
    set_joy_sequence(down, 1);
    map_update_player();
    failures += expect_true("downward input moves the player",
                            player.y > previous_y);

    player.x = 64 * 8;
    player.y = 64 * 8;
    player.map_x = 64;
    player.map_y = 64;
    set_joy_sequence(NULL, 0);
    map_update_player();
    failures += expect_true("portal entry switches to the dungeon",
                            current_map_data == dungeon_map_data);
    failures += expect_true("portal entry clears the encounter meter",
                            !map_take_encounter_trigger());
    set_joy_sequence(NULL, 0);
    map_update_player();
    failures += expect_true("portal exit does not immediately bounce back",
                            current_map_data == dungeon_map_data);

    player_reset();
    game_save_data.story_flags = STORY_FLAG_QUEST_STARTED;
    dungeon_init();
    map_load(dungeon_map_data, 128, 128);
    player.x = DUNGEON_CHECKPOINT_X;
    player.y = DUNGEON_CHECKPOINT_Y;
    player.map_x = DUNGEON_CHECKPOINT_TILE_X;
    player.map_y = DUNGEON_CHECKPOINT_TILE_Y;
    map_reset_encounter_meter();
    set_joy_sequence(walk_right, 4);
    for (u8 i = 0; i < 4; i++) map_update_player();
    failures += expect_true("checkpoint area is a safe encounter zone",
                            dungeon_is_safe_zone() &&
                            map_get_encounter_steps() == 0);
    player.x = 40 * 8;
    player.y = 64 * 8;
    player.map_x = 40;
    player.map_y = 64;
    map_reset_encounter_meter();
    set_joy_sequence(walk_right, 4);
    for (u8 i = 0; i < 4; i++) map_update_player();
    failures += expect_true("dungeon danger exposes step progress",
                            !dungeon_is_safe_zone() &&
                            map_get_encounter_steps() == 1 &&
                            map_get_encounter_steps() <
                            map_get_encounter_threshold());
    const u8 upper_chance = dungeon_get_encounter_chance_percent();
    player.map_y = 80;
    failures += expect_true("lower dungeon area raises encounter pressure",
                            dungeon_get_area() == DUNGEON_AREA_LOWER &&
                            dungeon_get_encounter_chance_percent() > upper_chance &&
                            battle_choose_enemy_in_area(TRUE,
                                                        DUNGEON_AREA_LOWER) !=
                                BATTLE_ENEMY_DRAGON);
    map_load(dungeon_map_data, 128, 128);
    failures += expect_true("loading a dungeon resets encounter danger",
                            map_get_encounter_steps() == 0 &&
                            !map_take_encounter_trigger());
    failures += expect_true("dungeon starts with a solid locked gate",
                            map_get_tile(DUNGEON_GATE_X + 1,
                                         DUNGEON_GATE_Y + 1) == 13 &&
                            map_is_solid(DUNGEON_GATE_X + 1,
                                         DUNGEON_GATE_Y + 1));
    player.x = DUNGEON_GOAL_X;
    player.y = DUNGEON_GOAL_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("dungeon boss stays locked before progression",
                            dungeon_interact() == DUNGEON_INTERACTION_HANDLED &&
                            !(game_save_data.story_flags &
                              STORY_FLAG_DUNGEON_GOAL_REACHED));
    player.x = DUNGEON_KEY_X;
    player.y = DUNGEON_KEY_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("rune key interaction persists the key",
                            dungeon_interact() == DUNGEON_INTERACTION_CHANGED &&
                            (game_save_data.story_flags &
                             STORY_FLAG_DUNGEON_KEY_FOUND) &&
                            inventory_get_count(ITEM_RUNE_KEY) == 1);
    player.x = DUNGEON_GATE_X;
    player.y = DUNGEON_GATE_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("rune key opens the previously blocked route",
                            dungeon_interact() == DUNGEON_INTERACTION_CHANGED &&
                            (game_save_data.story_flags &
                             STORY_FLAG_DUNGEON_GATE_OPEN) &&
                            !map_is_solid(DUNGEON_GATE_X + 1,
                                          DUNGEON_GATE_Y + 1) &&
                            map_get_tile(DUNGEON_GATE_X + 1,
                                         DUNGEON_GATE_Y + 1) == 3);
    player.x = DUNGEON_TREASURE_X;
    player.y = DUNGEON_TREASURE_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("dungeon treasure grants a useful reward",
                            dungeon_interact() == DUNGEON_INTERACTION_CHANGED &&
                            (game_save_data.story_flags &
                             STORY_FLAG_DUNGEON_TREASURE_OPEN) &&
                            inventory_get_count(ITEM_IRON_SWORD) == 1 &&
                            player_stats.gold == 25);
    player_stats.hp = 1;
    player_stats.mp = 0;
    player.x = DUNGEON_CHECKPOINT_X;
    player.y = DUNGEON_CHECKPOINT_Y;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("dungeon checkpoint restores and persists recovery",
                            dungeon_interact() == DUNGEON_INTERACTION_CHANGED &&
                            (game_save_data.story_flags &
                             STORY_FLAG_DUNGEON_CHECKPOINT) &&
                            player_stats.hp == player_stats.max_hp &&
                            player_stats.mp == player_stats.max_mp);
    player.x = DUNGEON_GOAL_X;
    player.y = DUNGEON_GOAL_Y;
    game_save_data.story_flags |= STORY_FLAG_DUNGEON_TREASURE_OPEN |
                                  STORY_FLAG_DUNGEON_CHECKPOINT;
    set_joy_sequence(dialogue_pages, 4);
    failures += expect_true("dungeon goal awakens a gated boss encounter",
                            dungeon_interact() == DUNGEON_INTERACTION_BOSS &&
                            (game_save_data.story_flags &
                             STORY_FLAG_DUNGEON_GOAL_REACHED) &&
                            inventory_get_count(ITEM_ANCIENT_RELIC) == 0);
    player_stats.atk = 50;
    battle_start_boss();
    set_joy_sequence(confirm, 2);
    battle_update();
    set_joy_sequence(attack_and_ack, 4);
    battle_update();
    set_joy_sequence(boss_phase, 6);
    failures += expect_true("boss changes behavior at its enraged phase",
                            battle_update() == 0 &&
                            battle_boss_in_enraged_phase() &&
                            battle_get_player_status() == BATTLE_STATUS_BURN &&
                            battle_get_player_status_turns() == 2);
    player_stats.atk = 100;
    player_stats.mp = player_stats.max_mp;
    battle_start_boss();
    set_joy_sequence(confirm, 2);
    battle_update();
    set_joy_sequence(boss_victory, 18);
    failures += expect_true("boss victory records completion and relic reward",
                            battle_update() == 1 &&
                            (game_save_data.story_flags &
                             STORY_FLAG_DUNGEON_BOSS_DEFEATED) &&
                            (game_save_data.story_flags & STORY_FLAG_GAME_COMPLETE) &&
                            (game_save_data.story_flags & STORY_FLAG_RELIC_FOUND) &&
                            inventory_get_count(ITEM_ANCIENT_RELIC) == 1);
    player.x = 64 * 8;
    player.y = 64 * 8;
    set_joy_sequence(NULL, 0);
    map_update_player();
    failures += expect_true("opened dungeon keeps a return portal to town",
                            current_map_data == town_map_data);

    player_reset();
    srand(1);
    failures += expect_true("town does not select a dungeon enemy",
                            battle_choose_enemy(FALSE) == 0);
    failures += expect_true("level one dungeon encounters start with slimes",
                            battle_choose_enemy(TRUE) == 0);

    audio_init();
    reset_psg_trace();
    audio_update();
    failures += expect_true("music starts with a note",
                            sgs_psg_has_tone(0));
    for (int i = 0; i < 50; i++) audio_update();
    failures += expect_true("music advances to another note",
                            sgs_psg_tone_changed(0));
    audio_set_theme(THEME_BATTLE);
    reset_psg_trace();
    audio_update();
    failures += expect_true("battle theme starts with a note",
                            sgs_psg_has_tone(0));
    settings_reset_defaults();
    set_joy_sequence(settings_speed, 4);
    failures += expect_true("settings adjust and apply text speed",
                            settings_open() &&
                            game_save_data.text_speed ==
                                SETTINGS_TEXT_SPEED_FAST &&
                            ui_get_text_speed() == SETTINGS_TEXT_SPEED_FAST);
    set_joy_sequence(settings_music, 6);
    failures += expect_true("settings toggle and apply music",
                            settings_open() &&
                            game_save_data.music_enabled == 0 &&
                            !audio_get_music_enabled());
    const s16 player_x_before_pause = player.x;
    set_joy_sequence(pause_close, 2);
    failures += expect_true("pause screen closes without moving the player",
                            !pause_open() && player.x == player_x_before_pause);
    play_sfx_select();
    for (int i = 0; i < 4; i++) audio_update();
    failures += expect_true("select sound fades out",
                            sgs_psg_has_envelope(1, PSG_ENVELOPE_MIN));
    play_sfx_hit();
    for (int i = 0; i < 5; i++) audio_update();
    failures += expect_true("hit sound fades out",
                            sgs_psg_has_envelope(3, PSG_ENVELOPE_MIN));

    battle_start(1);
    set_joy_sequence(confirm, 2);
    failures += expect_true("battle intro advances on confirmation",
                            battle_update() == 0);
    set_joy_sequence(attack_and_ack, 4);
    failures += expect_true("Fight consumes the player turn",
                            battle_update() == 0);
    set_joy_sequence(enemy_action_ack, 4);
    const s16 hp_before_enemy_turn = player_stats.hp;
    failures += expect_true("enemy turn follows a player attack",
                            battle_update() == 0 &&
                            player_stats.hp < hp_before_enemy_turn);

    player_stats.hp = 8;
    player_stats.mp = player_stats.max_mp;
    const s16 hp_before_skill = player_stats.hp;
    const s16 mp_before_skill = player_stats.mp;
    set_joy_sequence(skill_and_ack, 8);
    failures += expect_true("Heal restores HP and spends MP",
                            battle_update() == 0 &&
                            player_stats.hp > hp_before_skill &&
                            player_stats.mp == mp_before_skill - 3);
    set_joy_sequence(enemy_action_ack, 4);
    battle_update();

    player_stats.mp = 0;
    set_joy_sequence(power_no_mp, 10);
    failures += expect_true("Power Strike rejects missing MP",
                            battle_update() == 0 &&
                            player_stats.mp == 0 &&
                            battle_get_player_status() == BATTLE_STATUS_POISON);

    player_stats.hp = 5;
    game_save_data.inventory[ITEM_POTION] = 2;
    inventory_recalculate_stats();
    const s16 hp_before_item = player_stats.hp;
    set_joy_sequence(item_and_ack, 8);
    failures += expect_true("Potion restores HP and is consumed",
                            battle_update() == 0 &&
                            player_stats.hp > hp_before_item &&
                            player_stats.potions == 1);
    set_joy_sequence(enemy_action_ack, 4);
    battle_update();

    set_joy_sequence(run_menu, 6);
    failures += expect_true("Run is available as a battle action",
                            ui_battle_menu() == BATTLE_ACTION_RUN);

    player_reset();
    player_stats.atk = 100;
    battle_start(0);
    set_joy_sequence(confirm, 2);
    battle_update();
    set_joy_sequence(power_victory, 12);
    failures += expect_true("Power Strike deals a separate high-damage action",
                            battle_update() == 1 &&
                            player_stats.mp == player_stats.max_mp - 4);

    player_reset();
    battle_start(1);
    set_joy_sequence(confirm, 2);
    battle_update();
    set_joy_sequence(guard_and_ack, 6);
    failures += expect_true("Guard consumes a turn",
                            battle_update() == 0);
    set_joy_sequence(enemy_action_ack, 4);
    failures += expect_true("Skeleton poison has a readable duration",
                            battle_update() == 0 &&
                            battle_get_player_status() == BATTLE_STATUS_POISON &&
                            battle_get_player_status_turns() == 2);

    player_reset();
    battle_start(2);
    set_joy_sequence(confirm, 2);
    battle_update();
    set_joy_sequence(guard_and_ack, 6);
    battle_update();
    set_joy_sequence(enemy_action_ack, 4);
    battle_update();
    failures += expect_true("Dragon burn is a distinct short status",
                            battle_get_player_status() == BATTLE_STATUS_BURN &&
                            battle_get_player_status_turns() == 1);

    player_reset();
    battle_start(0);
    set_joy_sequence(confirm, 2);
    battle_update();
    player_stats.atk = 100;
    set_joy_sequence(victory, 6);
    failures += expect_true("winning a battle grants rewards",
                            battle_update() == 1 &&
                            player_stats.exp == 5 &&
                            player_stats.gold == 2);

    player_stats.exp = 100;
    level_up();
    failures += expect_true("level up processes every earned level",
                            player_stats.level == 4 &&
                            player_stats.hp == player_stats.max_hp &&
                            player_stats.mp == player_stats.max_mp);
    player_stats.hp = 1;
    player_stats.mp = 0;
    player_restore_at_town();
    failures += expect_true("town recovery restores HP and MP",
                            player_stats.hp == player_stats.max_hp &&
                            player_stats.mp == player_stats.max_mp);

    SaveData save_original = {0};
    save_original.map_id = SAVE_MAP_DUNGEON;
    save_original.player_x = 520;
    save_original.player_y = 512;
    save_original.player = player_stats;
    save_original.player.level = 4;
    save_original.player.max_hp = 35;
    save_original.player.hp = 31;
    save_original.player.max_mp = 14;
    save_original.player.mp = 9;
    save_original.inventory[ITEM_POTION] = 3;
    save_original.inventory[ITEM_BRONZE_SWORD] = 1;
    save_original.inventory[ITEM_LEATHER_ARMOR] = 1;
    save_original.equipped_weapon = ITEM_BRONZE_SWORD;
    save_original.equipped_armor = ITEM_LEATHER_ARMOR;
    save_original.story_flags = STORY_FLAG_QUEST_STARTED |
                                STORY_FLAG_DUNGEON_KEY_FOUND |
                                STORY_FLAG_DUNGEON_GATE_OPEN |
                                STORY_FLAG_DUNGEON_TREASURE_OPEN |
                                STORY_FLAG_DUNGEON_CHECKPOINT |
                                STORY_FLAG_DUNGEON_GOAL_REACHED |
                                STORY_FLAG_DUNGEON_BOSS_DEFEATED |
                                STORY_FLAG_GAME_COMPLETE;
    save_original.text_speed = SETTINGS_TEXT_SPEED_FAST;
    save_original.music_enabled = 0;
    save_original.sfx_enabled = 1;
    failures += expect_true("save writes and reads progression",
                            save_write(&save_original));

    SaveData save_loaded = {0};
    failures += expect_true("save restores location and future state",
                            save_read(&save_loaded) &&
                            save_loaded.map_id == SAVE_MAP_DUNGEON &&
                            save_loaded.player_x == 520 &&
                            save_loaded.player_y == 512 &&
                            save_loaded.player.level == 4 &&
                            save_loaded.inventory[ITEM_POTION] == 3 &&
                            save_loaded.equipped_weapon == ITEM_BRONZE_SWORD &&
                            save_loaded.equipped_armor == ITEM_LEATHER_ARMOR &&
                            save_loaded.story_flags == save_original.story_flags &&
                            save_loaded.text_speed == save_original.text_speed &&
                            save_loaded.music_enabled == save_original.music_enabled &&
                            save_loaded.sfx_enabled == save_original.sfx_enabled);
    settings_apply();
    failures += expect_true("continued save reapplies accessibility settings",
                            ui_get_text_speed() == SETTINGS_TEXT_SPEED_FAST &&
                            !audio_get_music_enabled() &&
                            audio_get_sfx_enabled());
    sgs.sram[0] ^= 0x01;
    failures += expect_true("corrupt save is rejected",
                            !save_read(&save_loaded));
    save_clear();
    failures += expect_true("clearing save removes Continue data",
                            !save_has_valid_data());

    return failures == 0 ? 0 : 1;
}
