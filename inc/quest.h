#ifndef QUEST_H
#define QUEST_H

#include <genesis.h>

const char *quest_current_objective(void);
bool quest_on_guide_interaction(void);
bool quest_on_enemy_defeated(u16 enemy_level);
bool quest_on_dungeon_goal(void);
bool quest_on_boss_defeated(void);

#endif
