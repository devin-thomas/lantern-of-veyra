#ifndef BATTLE_H
#define BATTLE_H

#include <genesis.h>

#define BATTLE_ENEMY_DRAGON 2

typedef struct {
    u16 level;
    s16 hp;
    s16 max_hp;
    s16 mp;
    s16 max_mp;
    u16 atk;
    u16 def;
    u16 base_atk;
    u16 base_def;
    u32 exp;
    u32 gold;
    u8 potions;
} Character;

typedef enum {
    ENEMY_TRAIT_NONE = 0,
    ENEMY_TRAIT_REGENERATE,
    ENEMY_TRAIT_POISON,
    ENEMY_TRAIT_BURN
} EnemyTrait;

typedef struct {
    char name[20];
    u16 level;
    s16 hp;
    s16 max_hp;
    u16 atk;
    u16 def;
    u32 exp_reward;
    u32 gold_reward;
    u8 min_level;
    u8 encounter_weight;
    EnemyTrait trait;
} Enemy;

typedef enum {
    BATTLE_ACTION_FIGHT = 1,
    BATTLE_ACTION_SKILL,
    BATTLE_ACTION_ITEM,
    BATTLE_ACTION_GUARD,
    BATTLE_ACTION_RUN
} BattleAction;

typedef enum {
    BATTLE_SKILL_HEAL = 1,
    BATTLE_SKILL_POWER_STRIKE
} BattleSkill;

typedef enum {
    BATTLE_STATUS_NONE = 0,
    BATTLE_STATUS_POISON,
    BATTLE_STATUS_BURN
} BattleStatus;

extern Character player_stats;
extern const Enemy enemy_database[];

void player_reset(void);
void player_restore_at_town(void);
int battle_choose_enemy(bool in_dungeon);
int battle_choose_enemy_in_area(bool in_dungeon, u8 area);

void battle_start(int enemy_id);
void battle_start_boss(void);
int battle_update(void);
void level_up(void);
BattleStatus battle_get_player_status(void);
u8 battle_get_player_status_turns(void);
bool battle_boss_in_enraged_phase(void);

#endif // BATTLE_H
