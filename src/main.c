#include <genesis.h>
#include "map.h"
#include "battle.h"
#include "ui.h"
#include "audio.h"
#include "inventory.h"
#include "quest.h"
#include "save.h"
#include "town.h"
#include "dungeon.h"
#include "pause.h"
#include "settings.h"
#include "resources.h"

enum GameState {
    STATE_TITLE,
    STATE_OVERWORLD,
    STATE_BATTLE,
    STATE_GAMEOVER,
    STATE_ENDING
};

enum GameState current_state;
Sprite* hero_sprite = NULL;
static const u16 *displayed_map_data = NULL;
static u16 previous_overworld_joy = 0;
static u16 save_notice_frames = 0;

static void clear_overworld_line(u16 y) {
    VDP_drawText("                                        ", 0, y);
}

static void draw_overworld_hud(void) {
    char status[41];
    char progress[41];
    char combat[41];
    const bool in_dungeon = current_map_data == dungeon_map_data;
    const char *area = in_dungeon ? "STONEWAKE" : "VEYRA";

    clear_overworld_line(1);
    clear_overworld_line(2);
    clear_overworld_line(3);
    clear_overworld_line(4);
    clear_overworld_line(24);
    clear_overworld_line(26);

    sprintf(status, "%s Lv%d HP %d/%d MP %d/%d", area,
            (int) player_stats.level, (int) player_stats.hp,
            (int) player_stats.max_hp, (int) player_stats.mp,
            (int) player_stats.max_mp);
    sprintf(progress, "EXP %lu  GOLD %lu  POTIONS %d",
            (unsigned long) player_stats.exp, (unsigned long) player_stats.gold,
            (int) player_stats.potions);
    VDP_drawText(status, 1, 1);
    VDP_drawText(progress, 1, 2);
    if (in_dungeon) {
        sprintf(combat, "ATK %d  DEF %d  DANGER %d/%d",
                (int) player_stats.atk, (int) player_stats.def,
                (int) map_get_encounter_steps(),
                (int) map_get_encounter_threshold());
    } else {
        sprintf(combat, "ATK %d  DEF %d", (int) player_stats.atk,
                (int) player_stats.def);
    }
    VDP_drawText(combat, 1, 3);
    VDP_drawText(quest_current_objective(), 1, 4);

    if (in_dungeon) {
        VDP_drawText("B STATUS  A ACTION  START SAVE", 1, 26);
    } else {
        VDP_drawText("B STATUS  A TALK  START SAVE", 1, 26);
    }
}

static void show_save_notice(bool succeeded) {
    clear_overworld_line(24);
    VDP_drawText(succeeded ? "GAME SAVED" : "SAVE FAILED", 15, 24);
    save_notice_frames = 90;
}

static void update_save_notice(void) {
    if (save_notice_frames == 0) return;

    save_notice_frames--;
    if (save_notice_frames == 0) clear_overworld_line(24);
}

static bool save_current_game(void) {
    game_save_data.map_id = current_map_data == dungeon_map_data ?
                            SAVE_MAP_DUNGEON : SAVE_MAP_TOWN;
    game_save_data.player_x = player.x;
    game_save_data.player_y = player.y;
    game_save_data.player = player_stats;

    const bool saved = save_write(&game_save_data);
    show_save_notice(saved);
    return saved;
}

static void wait_for_ending_input(void) {
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

static void show_ending_sequence(void) {
    static const char *const pages[] = {
        "The Ash Dragon falls, and Stonewake's seal breaks.",
        "The lantern relic's light reaches Veyra. The long dusk is over.",
        "Your journey is complete. The saved adventure can still be continued."
    };

    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    SPR_clear();
    hero_sprite = NULL;
    audio_set_theme(THEME_TOWN);
    ui_init();
    ui_draw_box(1, 7, 38, 11);
    VDP_drawText("VICTORY", 16, 7);

    for (u8 page = 0; page < 3; page++) {
        ui_show_text(pages[page]);
        VDP_drawText(page < 2 ? "A NEXT" : "A CONTINUE", 27, 16);
        wait_for_ending_input();
    }

    ui_clear_box();
    VDP_drawText("PROGRESS SAVED", 13, 10);
    VDP_drawText("CONTINUE: RESUME THIS ADVENTURE", 4, 13);
    VDP_drawText("NEW GAME: BEGIN AGAIN", 9, 15);
    VDP_drawText("PRESS START FOR TITLE", 9, 18);
    wait_for_ending_input();
}

static void setup_overworld_display(void) {
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    // Keep SGDK's system tiles intact; map data is offset by this base.
    VDP_loadTileSet(&bg_tileset, TILE_USER_INDEX, DMA);
    PAL_setPalette(PAL0, bg_palette.data, DMA);
    map_load(current_map_data, map_width, map_height);

    displayed_map_data = current_map_data;
    hero_sprite = SPR_addSprite(&spr_hero,
                                player.x - camera_x,
                                player.y - camera_y,
                                TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
    PAL_setPalette(PAL1, spr_hero.palette->data, DMA);

    ui_init();
    town_init();
    dungeon_init();
    draw_overworld_hud();
    audio_set_theme(current_map_data == dungeon_map_data ?
                    THEME_DUNGEON : THEME_TOWN);
    previous_overworld_joy = 0;
}

static void start_new_game(void) {
    save_clear();
    player_reset();
    map_init();
    setup_overworld_display();
    current_state = STATE_OVERWORLD;
}

static bool continue_saved_game(void) {
    SaveData loaded;
    if (!save_read(&loaded)) return FALSE;

    player_stats = loaded.player;
    settings_apply();
    inventory_recalculate_stats();
    game_save_data.player = player_stats;
    map_init();
    player.x = loaded.player_x;
    player.y = loaded.player_y;
    player.map_x = player.x / 8;
    player.map_y = player.y / 8;
    map_load(loaded.map_id == SAVE_MAP_DUNGEON ?
             dungeon_map_data : town_map_data, 128, 128);
    setup_overworld_display();
    current_state = STATE_OVERWORLD;
    return TRUE;
}

static void sync_overworld_context(void) {
    if (displayed_map_data == current_map_data) return;

    displayed_map_data = current_map_data;
    draw_overworld_hud();
    audio_set_theme(current_map_data == dungeon_map_data ?
                    THEME_DUNGEON : THEME_TOWN);
}

static void enter_battle_with_enemy(int enemy_id, bool boss) {
    const SpriteDefinition *enemy_sprite = &spr_slime;
    s16 enemy_x = 144;
    s16 enemy_y = 72;

    current_state = STATE_BATTLE;
    map_reset_encounter_meter();
    audio_set_theme(THEME_BATTLE);

    VDP_clearPlane(BG_B, TRUE);
    SPR_clear();
    hero_sprite = NULL;
    SPR_update();

    if (enemy_id == 1) {
        enemy_sprite = &spr_skeleton;
    } else if (enemy_id == 2) {
        enemy_sprite = &spr_dragon;
        enemy_x = 128;
        enemy_y = 56;
    }

    SPR_addSprite(enemy_sprite, enemy_x, enemy_y,
                  TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
    PAL_setPalette(PAL2, enemy_sprite->palette->data, DMA);

    ui_init();
    if (boss) {
        battle_start_boss();
    } else {
        battle_start(enemy_id);
    }
}

static void enter_battle(void) {
    const bool in_dungeon = current_map_data == dungeon_map_data;
    enter_battle_with_enemy(battle_choose_enemy_in_area(in_dungeon,
                                                        dungeon_get_area()),
                            FALSE);
}

static void enter_boss_battle(void) {
    enter_battle_with_enemy(BATTLE_ENEMY_DRAGON, TRUE);
}

void vblank_cb() {
    audio_update();
}

int main(bool hardReset)
{
    (void) hardReset;

    VDP_init();
    SPR_init();
    JOY_init();

    audio_init();
    SYS_setVIntCallback(vblank_cb);

    current_state = STATE_TITLE;

    while (1) {
        if (current_state == STATE_TITLE) {
            VDP_clearPlane(BG_A, TRUE);
            VDP_clearPlane(BG_B, TRUE);
            SPR_clear();
            audio_set_theme(THEME_TOWN);

            VDP_drawText("THE LAST LIGHT", 14, 8);
            VDP_drawText("LANTERN OF VEYRA", 12, 10);
            const bool continue_available = save_has_valid_data();
            u8 title_selection = 0;

            if (continue_available) {
                VDP_drawText("CONTINUE", 14, 12);
                VDP_drawText("NEW GAME", 14, 15);
                VDP_drawText("A SELECT", 14, 19);
                VDP_drawText(">", 13, 12);
            } else {
                VDP_drawText("NEW GAME", 14, 14);
                VDP_drawText("PRESS START", 14, 18);
            }

            u16 previous_title_joy = JOY_readJoypad(JOY_1);
            while (1) {
                const u16 joy = JOY_readJoypad(JOY_1);
            const u16 pressed = joy & (u16) ~previous_title_joy;
                previous_title_joy = joy;
                if (continue_available &&
                    (pressed & (BUTTON_UP | BUTTON_DOWN))) {
                    title_selection = title_selection == 0 ? 1 : 0;
                    VDP_drawText(" ", 13, title_selection == 0 ? 15 : 12);
                    VDP_drawText(">", 13, title_selection == 0 ? 12 : 15);
                }

                if (pressed & (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START)) {
                    play_sfx_select();
                    if (continue_available && title_selection == 0) {
                        if (continue_saved_game()) {
                            break;
                        }
                        VDP_drawText("SAVE FAILED", 14, 21);
                    } else {
                        start_new_game();
                        break;
                    }
                }
                SYS_doVBlankProcess();
            }
        } else if (current_state == STATE_OVERWORLD) {
            update_save_notice();
            const u16 joy = JOY_readJoypad(JOY_1);
            const bool a_pressed = (joy & BUTTON_A) &&
                                   !(previous_overworld_joy & BUTTON_A);
            const bool b_pressed = (joy & BUTTON_B) &&
                                   !(previous_overworld_joy & BUTTON_B);
            const bool start_pressed = (joy & BUTTON_START) &&
                                       !(previous_overworld_joy & BUTTON_START);
            previous_overworld_joy = joy;

            if (b_pressed) {
                const bool pause_changed = pause_open();
                draw_overworld_hud();
                town_update();
                dungeon_update();
                if (pause_changed) save_current_game();
            } else {
                const u16 *map_before_update = current_map_data;
                map_update_player();

                const bool returned_to_town = map_before_update == dungeon_map_data &&
                                              current_map_data == town_map_data;
                if (returned_to_town) {
                    player_restore_at_town();
                }
                sync_overworld_context();
                if (returned_to_town) save_current_game();

                if (hero_sprite) {
                    SPR_setPosition(hero_sprite, player.x - camera_x,
                                     player.y - camera_y);
                }
                town_update();
                dungeon_update();

                const bool encounter_triggered = map_take_encounter_trigger();
                if (start_pressed) save_current_game();
                if (current_map_data == town_map_data && a_pressed) {
                    if (town_interact()) {
                        draw_overworld_hud();
                        save_current_game();
                    }
                } else if (current_map_data == dungeon_map_data && a_pressed) {
                    const DungeonInteractionResult interaction = dungeon_interact();
                    if (interaction == DUNGEON_INTERACTION_BOSS) {
                        draw_overworld_hud();
                        dungeon_update();
                        enter_boss_battle();
                    } else if (interaction != DUNGEON_INTERACTION_NONE) {
                        if (interaction == DUNGEON_INTERACTION_CHANGED) {
                            map_redraw();
                        }
                        draw_overworld_hud();
                        dungeon_update();
                        if (interaction == DUNGEON_INTERACTION_CHANGED) {
                            save_current_game();
                        }
                    } else {
                        enter_battle();
                    }
                } else if (current_map_data == dungeon_map_data &&
                           encounter_triggered) {
                    enter_battle();
                }
            }
        } else if (current_state == STATE_BATTLE) {
            const int result = battle_update();
            if (result == 1) {
                if (player_stats.hp <= 0) {
                    audio_set_theme(THEME_NONE);
                    current_state = STATE_GAMEOVER;
                } else if (game_save_data.story_flags &
                           STORY_FLAG_DUNGEON_BOSS_DEFEATED) {
                    save_current_game();
                    SPR_clear();
                    hero_sprite = NULL;
                    current_state = STATE_ENDING;
                } else {
                    current_state = STATE_OVERWORLD;
                    SPR_clear();
                    VDP_clearPlane(BG_A, TRUE);
                    hero_sprite = SPR_addSprite(&spr_hero,
                                                player.x - camera_x,
                                                player.y - camera_y,
                                                TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
                    map_load(current_map_data, map_width, map_height);
                    displayed_map_data = current_map_data;
                    draw_overworld_hud();
                    audio_set_theme(current_map_data == dungeon_map_data ?
                                    THEME_DUNGEON : THEME_TOWN);
                    save_current_game();
                    // Do not carry the button used to dismiss battle text into the map.
                    previous_overworld_joy = JOY_readJoypad(JOY_1);
                }
            }
        } else if (current_state == STATE_GAMEOVER) {
            VDP_clearPlane(BG_A, TRUE);
            VDP_clearPlane(BG_B, TRUE);
            SPR_clear();
            audio_set_theme(THEME_NONE);
            VDP_drawText("GAME OVER", 15, 11);
            VDP_drawText("ALL PROGRESS LOST", 11, 13);
            VDP_drawText("PRESS START", 14, 16);

            while (1) {
                const u16 joy = JOY_readJoypad(JOY_1);
                if (joy & BUTTON_START) {
                    player_reset();
                    map_reset_encounter_meter();
                    displayed_map_data = NULL;
                    current_state = STATE_TITLE;
                    break;
                }
                SYS_doVBlankProcess();
            }
        } else if (current_state == STATE_ENDING) {
            show_ending_sequence();
            current_state = STATE_TITLE;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return 0;
}
