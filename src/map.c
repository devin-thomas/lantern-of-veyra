#include "map.h"

#include "dungeon.h"
#include "map_data.h"

#define MAP_TILE_BASE TILE_USER_INDEX
#define MAP_PLANE_WIDTH 64
#define MAP_PLANE_HEIGHT 32
#define PLAYER_SPEED 2
#define PORTAL_TILE 5
#define ENCOUNTER_MIN_STEPS 4
#define ENCOUNTER_MAX_STEPS 8

const u16 *current_map_data;
s16 map_width;
s16 map_height;

Player player;
s16 camera_x = 0;
s16 camera_y = 0;
static s16 drawn_start_col = -1;
static s16 drawn_start_row = -1;
static u8 encounter_steps = 0;
static bool encounter_triggered = FALSE;

static bool map_can_move_to(s16 x, s16 y) {
    if (x < 0 || y < 0 || x + player.w > map_width * 8 || y + player.h > map_height * 8) {
        return FALSE;
    }

    return !map_is_solid(x, y) &&
           !map_is_solid(x + player.w - 1, y) &&
           !map_is_solid(x, y + player.h - 1) &&
           !map_is_solid(x + player.w - 1, y + player.h - 1);
}

static void map_set_camera(void) {
    const s16 max_camera_x = (map_width * 8 > 320) ? (map_width * 8) - 320 : 0;
    const s16 max_camera_y = (map_height * 8 > 224) ? (map_height * 8) - 224 : 0;

    camera_x = player.x - 160;
    camera_y = player.y - 112;
    if (camera_x < 0) camera_x = 0;
    if (camera_y < 0) camera_y = 0;
    if (camera_x > max_camera_x) camera_x = max_camera_x;
    if (camera_y > max_camera_y) camera_y = max_camera_y;

    VDP_setHorizontalScroll(BG_B, -camera_x);
    // Positive V scroll moves the plane up, revealing the world below it.
    VDP_setVerticalScroll(BG_B, camera_y);
}

static void map_draw_visible(void) {
    const s16 start_col = camera_x >> 3;
    const s16 start_row = camera_y >> 3;

    for (s16 y = 0; y < 29; y++) {
        for (s16 x = 0; x < 41; x++) {
            const s16 mx = start_col + x;
            const s16 my = start_row + y;
            if (mx >= 0 && mx < map_width && my >= 0 && my < map_height) {
                const u16 tile = map_get_tile(mx * 8, my * 8);
                VDP_setTileMapXY(BG_B,
                                 TILE_ATTR_FULL(PAL0, 0, 0, 0, MAP_TILE_BASE + tile),
                                 mx % MAP_PLANE_WIDTH, my % MAP_PLANE_HEIGHT);
            }
        }
    }

    drawn_start_col = start_col;
    drawn_start_row = start_row;
}

void map_load(const u16 *map_data, s16 w, s16 h) {
    encounter_steps = 0;
    encounter_triggered = FALSE;

    current_map_data = map_data;
    map_width = w;
    map_height = h;

    drawn_start_col = -1;
    drawn_start_row = -1;
    map_set_camera();
    map_draw_visible();
}

void map_redraw(void) {
    drawn_start_col = -1;
    drawn_start_row = -1;
    map_draw_visible();
}

void map_init(void) {
    player.x = 64;
    player.y = 64;
    player.w = 8;
    player.h = 8;
    player.map_x = player.x / 8;
    player.map_y = player.y / 8;
    encounter_steps = 0;
    encounter_triggered = FALSE;

    map_load(town_map_data, 128, 128);
}

u16 map_get_tile(s16 x, s16 y) {
    if (x < 0 || x >= map_width * 8 || y < 0 || y >= map_height * 8) return 0;
    const s16 tile_x = x / 8;
    const s16 tile_y = y / 8;
    u16 tile = current_map_data[tile_y * map_width + tile_x];
    if (current_map_data == dungeon_map_data) {
        tile = dungeon_tile_override(tile_x, tile_y, tile);
    }
    return tile;
}

bool map_is_solid(s16 x, s16 y) {
    const u16 tile = map_get_tile(x, y);
    return tile == 11 || tile == 12 || tile == 13;
}

void map_update_player(void) {
    const u16 joy = JOY_readJoypad(JOY_1);
    const s16 old_x = player.x;
    const s16 old_y = player.y;
    const s16 old_map_x = player.map_x;
    const s16 old_map_y = player.map_y;
    s16 dx = 0;
    s16 dy = 0;

    if (joy & BUTTON_LEFT) dx -= PLAYER_SPEED;
    if (joy & BUTTON_RIGHT) dx += PLAYER_SPEED;
    if (joy & BUTTON_UP) dy -= PLAYER_SPEED;
    if (joy & BUTTON_DOWN) dy += PLAYER_SPEED;

    if (dx != 0 && map_can_move_to(player.x + dx, player.y)) {
        player.x += dx;
    }
    if (dy != 0 && map_can_move_to(player.x, player.y + dy)) {
        player.y += dy;
    }

    player.map_x = player.x / 8;
    player.map_y = player.y / 8;

    // Check the tile under the player's center so a two-pixel step cannot skip portals.
    const u16 current_tile = map_get_tile(player.x + player.w / 2, player.y + player.h / 2);
    if (current_tile == PORTAL_TILE) {
        const bool entering_dungeon = (current_map_data == town_map_data);
        const u16 *next_map = entering_dungeon ? dungeon_map_data : town_map_data;

        map_load(next_map, 128, 128);

        // Start just beyond the portal. Search outward in case a generated tile is solid.
        const s16 direction = entering_dungeon ? 1 : -1;
        bool placed = FALSE;
        for (s16 distance = 1; distance <= 8 && !placed; distance++) {
            const s16 candidate_x = (64 * 8) + (direction * distance * 8);
            if (map_can_move_to(candidate_x, 64 * 8)) {
                player.x = candidate_x;
                player.y = 64 * 8;
                placed = TRUE;
            }
        }
        if (!placed) {
            player.x = 64 * 8;
            player.y = 64 * 8;
        }
        player.map_x = player.x / 8;
        player.map_y = player.y / 8;
        map_load(current_map_data, map_width, map_height);
        return;
    }

    if (current_map_data == dungeon_map_data &&
        (player.map_x != old_map_x || player.map_y != old_map_y) &&
        (player.x != old_x || player.y != old_y) &&
        !dungeon_is_safe_zone()) {
        if (encounter_steps < 255) encounter_steps++;
        const bool forced_encounter = encounter_steps >= ENCOUNTER_MAX_STEPS;
        const bool chance_encounter = encounter_steps >= ENCOUNTER_MIN_STEPS &&
                                      (random() % 100) <
                                      dungeon_get_encounter_chance_percent();
        if (forced_encounter || chance_encounter) {
            encounter_steps = 0;
            encounter_triggered = TRUE;
        }
    }

    map_set_camera();
    const s16 start_col = camera_x >> 3;
    const s16 start_row = camera_y >> 3;
    if (start_col != drawn_start_col || start_row != drawn_start_row) {
        map_draw_visible();
    }
}

bool map_take_encounter_trigger(void) {
    const bool triggered = encounter_triggered;
    encounter_triggered = FALSE;
    return triggered;
}

void map_reset_encounter_meter(void) {
    encounter_steps = 0;
    encounter_triggered = FALSE;
}

u8 map_get_encounter_steps(void) {
    return encounter_steps;
}

u8 map_get_encounter_threshold(void) {
    return ENCOUNTER_MAX_STEPS;
}
