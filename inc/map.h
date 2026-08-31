#ifndef MAP_H
#define MAP_H

#include <genesis.h>

typedef struct {
    s16 x;
    s16 y;
    s16 map_x;
    s16 map_y;
    s16 w;
    s16 h;
} Player;

extern const u16 town_map_data[16384];
extern const u16 dungeon_map_data[16384];

extern const u16 *current_map_data;
extern s16 map_width;
extern s16 map_height;

extern Player player;
extern s16 camera_x;
extern s16 camera_y;

void map_init(void);
void map_update_player(void);
void map_load(const u16 *map_data, s16 w, s16 h);
void map_redraw(void);
bool map_is_solid(s16 x, s16 y);
u16 map_get_tile(s16 x, s16 y);
bool map_take_encounter_trigger(void);
void map_reset_encounter_meter(void);
u8 map_get_encounter_steps(void);
u8 map_get_encounter_threshold(void);

#endif // MAP_H
