#include "battle.h"
#include "audio.h"
#include "inventory.h"
#include "quest.h"
#include "save.h"
#include "settings.h"
#include "ui.h"

#define MAX_LEVEL 10
#define HEAL_MP_COST 3
#define HEAL_BASE_AMOUNT 8
#define POWER_STRIKE_MP_COST 4

static const Character initial_player_stats = {
    .level = 1,
    .hp = 20,
    .max_hp = 20,
    .mp = 8,
    .max_mp = 8,
    .atk = 5,
    .def = 3,
    .base_atk = 5,
    .base_def = 3,
    .exp = 0,
    .gold = 0,
    .potions = 2
};

Character player_stats = {
    .level = 1,
    .hp = 20,
    .max_hp = 20,
    .mp = 8,
    .max_mp = 8,
    .atk = 5,
    .def = 3,
    .base_atk = 5,
    .base_def = 3,
    .exp = 0,
    .gold = 0,
    .potions = 2
};

const Enemy enemy_database[] = {
    {
        .name = "MOON SLIME",
        .level = 1,
        .hp = 10,
        .max_hp = 10,
        .atk = 2,
        .def = 1,
        .exp_reward = 5,
        .gold_reward = 2,
        .min_level = 1,
        .encounter_weight = 70,
        .trait = ENEMY_TRAIT_REGENERATE
    },
    {
        .name = "BONE SENTRY",
        .level = 2,
        .hp = 25,
        .max_hp = 25,
        .atk = 6,
        .def = 4,
        .exp_reward = 15,
        .gold_reward = 10,
        .min_level = 2,
        .encounter_weight = 25,
        .trait = ENEMY_TRAIT_POISON
    },
    {
        .name = "ASH DRAGON",
        .level = 4,
        .hp = 60,
        .max_hp = 60,
        .atk = 12,
        .def = 7,
        .exp_reward = 100,
        .gold_reward = 50,
        .min_level = 4,
        .encounter_weight = 0,
        .trait = ENEMY_TRAIT_BURN
    }
};

static Enemy current_enemy;

typedef enum {
    BATTLE_STATE_INACTIVE,
    BATTLE_STATE_INTRO,
    BATTLE_STATE_PLAYER_TURN,
    BATTLE_STATE_ENEMY_TURN
} BattleState;

static BattleState battle_state = BATTLE_STATE_INACTIVE;
static BattleStatus player_status = BATTLE_STATUS_NONE;
static u8 player_status_turns = 0;
static bool player_guarding = FALSE;
static bool current_battle_is_boss = FALSE;
static bool boss_phase_two = FALSE;

static u32 experience_for_level(u16 level) {
    return (u32) level * level * 10;
}

static void clear_battle_line(u16 y) {
    VDP_drawText("                                        ", 0, y);
}

static void battle_draw_status(void) {
    char line[41];

    clear_battle_line(0);
    clear_battle_line(3);
    clear_battle_line(5);
    clear_battle_line(6);
    clear_battle_line(7);
    VDP_drawText(current_battle_is_boss ? "BOSS BATTLE" : "BATTLE",
                 current_battle_is_boss ? 14 : 17, 0);

    sprintf(line, "ENEMY %s Lv%d HP %d/%d", current_enemy.name,
            (int) current_enemy.level, (int) current_enemy.hp,
            (int) current_enemy.max_hp);
    VDP_drawText(line, 1, 3);

    sprintf(line, "HERO Lv%d HP %d/%d MP %d/%d P:%d", (int) player_stats.level,
            (int) player_stats.hp, (int) player_stats.max_hp,
            (int) player_stats.mp, (int) player_stats.max_mp,
            (int) player_stats.potions);
    VDP_drawText(line, 1, 5);
    sprintf(line, "ATK %d DEF %d POTIONS %d", (int) player_stats.atk,
            (int) player_stats.def, (int) player_stats.potions);
    VDP_drawText(line, 1, 6);
    if (player_status == BATTLE_STATUS_POISON) {
        sprintf(line, "STATUS POISON %dT", (int) player_status_turns);
    } else if (player_status == BATTLE_STATUS_BURN) {
        sprintf(line, "STATUS BURN %dT", (int) player_status_turns);
    } else if (player_guarding) {
        sprintf(line, "STATUS GUARD");
    } else {
        sprintf(line, "STATUS READY");
    }
    VDP_drawText(line, 1, 7);
}

static void battle_message(const char *text) {
    battle_draw_status();
    ui_draw_box(1, 18, 38, 5);
    ui_show_text(text);
}

static void battle_wait_for_button(void) {
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

void player_reset(void) {
    player_stats = initial_player_stats;
    settings_reset_defaults();
    inventory_reset();
}

void player_restore_at_town(void) {
    player_stats.hp = player_stats.max_hp;
    player_stats.mp = player_stats.max_mp;
}

static u8 encounter_weight_for_area(u16 enemy_id, u8 area) {
    if (enemy_id == 0) return area == 0 ? 70 : 35;
    if (enemy_id == 1) return area == 0 ? 25 : 60;
    return 0;
}

int battle_choose_enemy_in_area(bool in_dungeon, u8 area) {
    const u16 enemy_count = sizeof(enemy_database) / sizeof(enemy_database[0]);
    u16 total_weight = 0;

    if (!in_dungeon) return 0;

    for (u16 i = 0; i < enemy_count; i++) {
        if (player_stats.level >= enemy_database[i].min_level) {
            const u8 weight = encounter_weight_for_area(i, area);
            if (weight > 0) {
                total_weight += weight;
            }
        }
    }

    if (total_weight == 0) return 0;

    u16 roll = (u16) (random() % total_weight);
    for (u16 i = 0; i < enemy_count; i++) {
        if (player_stats.level < enemy_database[i].min_level) continue;
        const u8 weight = encounter_weight_for_area(i, area);
        if (weight == 0) continue;
        if (roll < weight) return i;
        roll -= weight;
    }

    return 0;
}

int battle_choose_enemy(bool in_dungeon) {
    return battle_choose_enemy_in_area(in_dungeon, 0);
}

void battle_start(int enemy_id) {
    const u16 enemy_count = sizeof(enemy_database) / sizeof(enemy_database[0]);
    if (enemy_id < 0 || enemy_id >= enemy_count) enemy_id = 0;

    current_enemy = enemy_database[enemy_id];
    battle_state = BATTLE_STATE_INTRO;
    player_status = BATTLE_STATUS_NONE;
    player_status_turns = 0;
    player_guarding = FALSE;
    current_battle_is_boss = FALSE;
    boss_phase_two = FALSE;

    char buffer[64];
    sprintf(buffer, "A wild %s appears!", current_enemy.name);
    battle_message(buffer);
}

void battle_start_boss(void) {
    current_enemy = enemy_database[BATTLE_ENEMY_DRAGON];
    current_enemy.level = 5;
    current_enemy.hp = 80;
    current_enemy.max_hp = 80;
    current_enemy.atk = 14;
    current_enemy.def = 8;
    current_enemy.exp_reward = 150;
    current_enemy.gold_reward = 100;
    battle_state = BATTLE_STATE_INTRO;
    player_status = BATTLE_STATUS_NONE;
    player_status_turns = 0;
    player_guarding = FALSE;
    current_battle_is_boss = TRUE;
    boss_phase_two = FALSE;
    battle_message("The Ash Dragon guards the lantern relic!");
}

BattleStatus battle_get_player_status(void) {
    return player_status;
}

u8 battle_get_player_status_turns(void) {
    return player_status_turns;
}

bool battle_boss_in_enraged_phase(void) {
    return current_battle_is_boss && boss_phase_two;
}

void level_up(void) {
    u16 levels_gained = 0;

    while (player_stats.level < MAX_LEVEL &&
           player_stats.exp >= experience_for_level(player_stats.level)) {
        player_stats.level++;
        player_stats.max_hp += 5;
        player_stats.max_mp += 2;
        player_stats.base_atk += 2;
        player_stats.base_def += 1;
        inventory_recalculate_stats();
        player_stats.hp = player_stats.max_hp;
        player_stats.mp = player_stats.max_mp;
        levels_gained++;
    }

    if (levels_gained > 0) {
        char buffer[64];
        sprintf(buffer, "Level up! Reached level %d!", (int) player_stats.level);
        battle_message(buffer);
    }
}

static s16 calculate_damage(u16 attack, u16 defense, bool *critical) {
    s16 damage = (s16) attack - (s16) defense;
    if (damage < 1) damage = 1;
    damage += (s16) (random() % 3);

    *critical = (random() % 16) == 0;
    if (*critical) damage *= 2;
    return damage;
}

static s16 enemy_regenerate(void) {
    if (current_enemy.trait != ENEMY_TRAIT_REGENERATE ||
        current_enemy.hp >= current_enemy.max_hp) {
        return 0;
    }

    const s16 restored = current_enemy.max_hp - current_enemy.hp < 3 ?
                         current_enemy.max_hp - current_enemy.hp : 3;
    current_enemy.hp += restored;
    return restored;
}

static bool enemy_apply_status(void) {
    if (player_status != BATTLE_STATUS_NONE ||
        current_enemy.trait == ENEMY_TRAIT_REGENERATE ||
        current_enemy.trait == ENEMY_TRAIT_NONE) {
        return FALSE;
    }

    if (current_enemy.trait == ENEMY_TRAIT_POISON) {
        player_status = BATTLE_STATUS_POISON;
        player_status_turns = 3;
        return TRUE;
    }
    if (current_enemy.trait == ENEMY_TRAIT_BURN) {
        player_status = BATTLE_STATUS_BURN;
        player_status_turns = current_battle_is_boss && boss_phase_two ? 3 : 2;
        return TRUE;
    }
    return FALSE;
}

static bool battle_tick_status(void) {
    if (player_status == BATTLE_STATUS_NONE || player_status_turns == 0) {
        return TRUE;
    }

    const BattleStatus status = player_status;
    const s16 damage = status == BATTLE_STATUS_POISON ? 2 : 3;
    player_stats.hp -= damage;
    if (player_stats.hp < 0) player_stats.hp = 0;
    player_status_turns--;
    if (player_status_turns == 0) player_status = BATTLE_STATUS_NONE;

    battle_message(status == BATTLE_STATUS_POISON ?
                   "Poison deals 2 damage." : "Burn deals 3 damage.");
    battle_wait_for_button();
    if (player_stats.hp > 0) return TRUE;

    battle_message("The status effect defeated you...");
    battle_wait_for_button();
    battle_state = BATTLE_STATE_INACTIVE;
    return FALSE;
}

static int battle_win(void) {
    char buffer[64];
    const u32 exp_reward = current_enemy.exp_reward;
    const u32 gold_reward = current_enemy.gold_reward;
    const u16 old_level = player_stats.level;

    player_stats.exp += exp_reward;
    player_stats.gold += gold_reward;
    sprintf(buffer, "%s defeated! +%lu EXP +%lu G", current_enemy.name,
            (unsigned long) exp_reward, (unsigned long) gold_reward);
    battle_message(buffer);
    battle_wait_for_button();

    const bool relic_found = current_battle_is_boss ?
                             quest_on_boss_defeated() :
                             quest_on_enemy_defeated(current_enemy.level);
    if (relic_found) {
        battle_message("The lantern relic was found!");
        battle_wait_for_button();
    }

    if (current_battle_is_boss) {
        game_save_data.story_flags |= STORY_FLAG_DUNGEON_BOSS_DEFEATED |
                                      STORY_FLAG_GAME_COMPLETE;
        battle_message("The Ash Dragon is defeated. Veyra is safe!");
        battle_wait_for_button();
    }

    level_up();
    if (player_stats.level != old_level) battle_wait_for_button();

    battle_state = BATTLE_STATE_INACTIVE;
    return 1;
}

static bool battle_use_heal(void) {
    if (player_stats.mp < HEAL_MP_COST) {
        battle_message("Not enough MP for Heal.");
        battle_wait_for_button();
        return FALSE;
    }
    if (player_stats.hp >= player_stats.max_hp) {
        battle_message("HP is already full.");
        battle_wait_for_button();
        return FALSE;
    }

    const s16 old_hp = player_stats.hp;
    player_stats.mp -= HEAL_MP_COST;
    player_stats.hp += HEAL_BASE_AMOUNT + (s16) player_stats.level * 2;
    if (player_stats.hp > player_stats.max_hp) player_stats.hp = player_stats.max_hp;

    char buffer[64];
    sprintf(buffer, "Heal restored %d HP.", (int) (player_stats.hp - old_hp));
    battle_message(buffer);
    battle_wait_for_button();
    return TRUE;
}

static int battle_use_power_strike(void) {
    if (player_stats.mp < POWER_STRIKE_MP_COST) {
        battle_message("Not enough MP for Power Strike.");
        battle_wait_for_button();
        return 0;
    }

    player_stats.mp -= POWER_STRIKE_MP_COST;
    u32 power_attack = (u32) player_stats.atk * 2 + player_stats.level;
    if (power_attack > 65535) power_attack = 65535;

    bool critical;
    const s16 damage = calculate_damage((u16) power_attack,
                                        current_enemy.def, &critical);
    current_enemy.hp -= damage;
    if (current_enemy.hp < 0) current_enemy.hp = 0;

    play_sfx_hit();
    char buffer[64];
    if (critical) {
        sprintf(buffer, "Power Strike crits for %d damage!", (int) damage);
    } else {
        sprintf(buffer, "Power Strike deals %d damage.", (int) damage);
    }
    battle_message(buffer);

    if (current_enemy.hp <= 0) {
        battle_wait_for_button();
        battle_win();
        return 2;
    }

    battle_wait_for_button();
    return 1;
}

static bool battle_use_potion(void) {
    if (inventory_get_count(ITEM_POTION) == 0) {
        battle_message("You have no potions.");
        battle_wait_for_button();
        return FALSE;
    }
    if (player_stats.hp >= player_stats.max_hp) {
        battle_message("HP is already full.");
        battle_wait_for_button();
        return FALSE;
    }

    const s16 old_hp = player_stats.hp;
    inventory_use_potion();

    char buffer[64];
    sprintf(buffer, "Potion restored %d HP.", (int) (player_stats.hp - old_hp));
    battle_message(buffer);
    battle_wait_for_button();
    return TRUE;
}

static bool battle_try_run(void) {
    s16 escape_chance = 50 +
        ((s16) player_stats.level - (s16) current_enemy.level) * 10;
    if (escape_chance < 25) escape_chance = 25;
    if (escape_chance > 90) escape_chance = 90;

    if ((random() % 100) < escape_chance) {
        battle_message("You escaped safely.");
        battle_wait_for_button();
        battle_state = BATTLE_STATE_INACTIVE;
        return TRUE;
    }

    battle_message("Couldn't escape!");
    battle_wait_for_button();
    return FALSE;
}

int battle_update(void) {
    if (battle_state == BATTLE_STATE_INACTIVE) return 1;

    if (battle_state == BATTLE_STATE_INTRO) {
        battle_wait_for_button();
        battle_state = BATTLE_STATE_PLAYER_TURN;
        battle_message("Choose an action.");
        return 0;
    }

    if (battle_state == BATTLE_STATE_PLAYER_TURN) {
        const BattleAction action = (BattleAction) ui_battle_menu();

        if (action == BATTLE_ACTION_FIGHT) {
            bool critical;
            const s16 damage = calculate_damage(player_stats.atk,
                                                current_enemy.def, &critical);
            current_enemy.hp -= damage;
            if (current_enemy.hp < 0) current_enemy.hp = 0;

            play_sfx_hit();
            char buffer[64];
            if (critical) {
                sprintf(buffer, "Critical hit! %s takes %d damage.",
                        current_enemy.name, (int) damage);
            } else {
                sprintf(buffer, "You hit %s for %d damage.",
                        current_enemy.name, (int) damage);
            }
            battle_message(buffer);

            if (current_enemy.hp <= 0) {
                battle_wait_for_button();
                return battle_win();
            }

            battle_wait_for_button();
            battle_state = BATTLE_STATE_ENEMY_TURN;
            return 0;
        }

        if (action == BATTLE_ACTION_SKILL) {
            const BattleSkill skill = (BattleSkill) ui_skill_menu();
            if (skill == BATTLE_SKILL_HEAL) {
                if (battle_use_heal()) battle_state = BATTLE_STATE_ENEMY_TURN;
            } else if (skill == BATTLE_SKILL_POWER_STRIKE) {
                const int result = battle_use_power_strike();
                if (result == 1) battle_state = BATTLE_STATE_ENEMY_TURN;
                else if (result == 2) return 1;
            }
            return 0;
        }

        if (action == BATTLE_ACTION_ITEM) {
            if (battle_use_potion()) battle_state = BATTLE_STATE_ENEMY_TURN;
            return 0;
        }

        if (action == BATTLE_ACTION_GUARD) {
            player_guarding = TRUE;
            battle_message("Guard raised. Enemy damage is halved.");
            battle_wait_for_button();
            battle_state = BATTLE_STATE_ENEMY_TURN;
            return 0;
        }

        if (action == BATTLE_ACTION_RUN) {
            if (battle_try_run()) return 1;
            battle_state = BATTLE_STATE_ENEMY_TURN;
            return 0;
        }

        return 0;
    }

    if (battle_state == BATTLE_STATE_ENEMY_TURN) {
        if (current_battle_is_boss && !boss_phase_two &&
            current_enemy.hp <= current_enemy.max_hp / 2) {
            boss_phase_two = TRUE;
            current_enemy.atk += 4;
            current_enemy.def += 1;
            battle_message("The Ash Dragon enters its enraged phase!");
            battle_wait_for_button();
        }

        const s16 regeneration = enemy_regenerate();
        const bool inflicted_status = enemy_apply_status();
        bool critical;
        s16 damage = calculate_damage(current_enemy.atk,
                                      player_stats.def, &critical);
        if (player_guarding) damage = (damage + 1) / 2;
        player_stats.hp -= damage;
        if (player_stats.hp < 0) player_stats.hp = 0;

        play_sfx_hit();
        char buffer[64];
        if (regeneration > 0) {
            sprintf(buffer, "%s regenerates %d HP, then hits for %d.",
                    current_enemy.name, (int) regeneration, (int) damage);
        } else if (inflicted_status) {
            sprintf(buffer, "%s hits for %d damage. You are %s!",
                    current_enemy.name, (int) damage,
                    current_enemy.trait == ENEMY_TRAIT_POISON ?
                    "poisoned" : "burning");
        } else if (player_guarding) {
            sprintf(buffer, "Guard halves %s's hit to %d damage.",
                    current_enemy.name, (int) damage);
        } else if (critical) {
            sprintf(buffer, "Critical! %s hits for %d damage.",
                    current_enemy.name, (int) damage);
        } else {
            sprintf(buffer, "%s hits you for %d damage.",
                    current_enemy.name, (int) damage);
        }
        battle_message(buffer);

        if (player_stats.hp <= 0) {
            battle_message("You have fallen in battle...");
            battle_wait_for_button();
            battle_state = BATTLE_STATE_INACTIVE;
            return 1;
        }

        battle_wait_for_button();
        player_guarding = FALSE;
        if (!battle_tick_status()) return 1;
        battle_state = BATTLE_STATE_PLAYER_TURN;
        return 0;
    }

    battle_state = BATTLE_STATE_INACTIVE;
    return 1;
}
