W = 128
H = 128


GRASS = 1
PATH = 2
STONE = 3
PORTAL = 5
TOWN_WALL = 11
HOUSE = 12
DUNGEON_WALL = 13


def blank_map(fill):
    return [[fill for _ in range(W)] for _ in range(H)]


def fill_rect(map_data, x0, y0, x1, y1, tile):
    for y in range(y0, y1):
        for x in range(x0, x1):
            map_data[y][x] = tile


def add_border(map_data, tile):
    for x in range(W):
        map_data[0][x] = tile
        map_data[H - 1][x] = tile
    for y in range(H):
        map_data[y][0] = tile
        map_data[y][W - 1] = tile


def generate_town():
    map_data = blank_map(GRASS)
    add_border(map_data, TOWN_WALL)

    # A broad road grid makes the town readable and guarantees a route to the portal.
    for y in (12, 13, 14, 32, 33, 34, 62, 63, 64, 65, 66, 92, 93, 94, 112, 113, 114):
        for x in range(1, W - 1):
            map_data[y][x] = PATH
    for x in (12, 13, 14, 32, 33, 34, 62, 63, 64, 65, 66, 92, 93, 94, 112, 113, 114):
        for y in range(1, H - 1):
            map_data[y][x] = PATH

    # Buildings sit between roads instead of appearing as isolated random blocks.
    for x, y in (
        (18, 18), (42, 18), (78, 18), (102, 18),
        (18, 42), (42, 42), (78, 42), (102, 42),
        (18, 78), (42, 78), (78, 78), (102, 78),
        (18, 102), (42, 102), (78, 102), (102, 102),
    ):
        fill_rect(map_data, x, y, x + 8, y + 6, HOUSE)

    map_data[64][64] = PORTAL
    return map_data


def generate_dungeon():
    map_data = blank_map(STONE)
    add_border(map_data, DUNGEON_WALL)

    # Repeating wall bands create readable rooms and a deliberate return route.
    openings = ((16, 24), (56, 72), (104, 112))
    for y in (24, 48, 72, 96):
        for x in range(1, W - 1):
            # The central wall is a full locked gate until the rune key is used.
            if y == 72:
                map_data[y][x] = DUNGEON_WALL
            elif not any(start <= x < end for start, end in openings):
                map_data[y][x] = DUNGEON_WALL

    for x in (32, 96):
        for y in range(4, H - 4):
            if not any(start <= y < end for start, end in openings):
                map_data[y][x] = DUNGEON_WALL

    # Keep the portal room open and place the return portal at the same coordinate.
    fill_rect(map_data, 61, 61, 68, 68, STONE)
    map_data[64][64] = PORTAL
    return map_data


with open("src/map_data.c", "w", encoding="ascii", newline="\n") as f:
    f.write('#include <genesis.h>\n')
    f.write('const u16 town_map_data[128*128] = {\n')
    town = generate_town()
    for row in town:
        f.write(', '.join(map(str, row)) + ',\n')
    f.write('};\n\n')

    f.write('const u16 dungeon_map_data[128*128] = {\n')
    dungeon = generate_dungeon()
    for row in dungeon:
        f.write(', '.join(map(str, row)) + ',\n')
    f.write('};\n')

with open("inc/map_data.h", "w", encoding="ascii", newline="\n") as f:
    f.write('#ifndef MAP_DATA_H\n#define MAP_DATA_H\n')
    f.write('#include <genesis.h>\n')
    f.write('extern const u16 town_map_data[128*128];\n')
    f.write('extern const u16 dungeon_map_data[128*128];\n')
    f.write('#endif\n')
