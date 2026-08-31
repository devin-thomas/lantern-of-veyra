import re
from collections import deque
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
WIDTH = 128
HEIGHT = 128
USED_TILE_IDS = (1, 2, 3, 5, 11, 12, 13)


def read_map(source, name):
    match = re.search(
        rf"const u16 {name}\[128\*128\] = \{{(.*?)\}};",
        source,
        re.DOTALL,
    )
    if match is None:
        raise AssertionError(f"missing {name}")
    values = [int(value) for value in re.findall(r"\b\d+\b", match.group(1))]
    if len(values) != WIDTH * HEIGHT:
        raise AssertionError(f"{name} has {len(values)} cells, expected {WIDTH * HEIGHT}")
    return values


def assert_route(map_data, start, destination):
    queue = deque([start])
    seen = {start}
    while queue:
        x, y = queue.popleft()
        if (x, y) == destination:
            return
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if not (0 <= nx < WIDTH and 0 <= ny < HEIGHT):
                continue
            if (nx, ny) in seen or map_data[ny * WIDTH + nx] > 10:
                continue
            seen.add((nx, ny))
            queue.append((nx, ny))
    raise AssertionError(f"no walkable route from {start} to {destination}")


def assert_no_route(map_data, start, destination):
    try:
        assert_route(map_data, start, destination)
    except AssertionError:
        return
    raise AssertionError(f"unexpected open route from {start} to {destination}")


def main():
    failures = []
    tileset = Image.open(ROOT / "res" / "tileset.png").convert("RGB")
    for tile_id in USED_TILE_IDS:
        left = (tile_id % 4) * 8
        top = (tile_id // 4) * 8
        colors = set(tileset.crop((left, top, left + 8, top + 8)).get_flattened_data())
        if (255, 0, 255) in colors:
            failures.append(f"tile {tile_id} contains the debug magenta color")
        if len(colors) < 2:
            failures.append(f"tile {tile_id} is a flat-color placeholder")

    generator = (ROOT / "generate_maps.py").read_text(encoding="utf-8")
    if "random.random" in generator:
        failures.append("map generation is nondeterministic random noise")

    map_source = (ROOT / "src" / "map_data.c").read_text(encoding="utf-8")
    town = read_map(map_source, "town_map_data")
    dungeon = read_map(map_source, "dungeon_map_data")
    if town[64 * WIDTH + 64] != 5:
        failures.append("town portal is not at the documented location")
    if dungeon[64 * WIDTH + 64] != 5:
        failures.append("dungeon portal is not at the documented location")
    try:
        assert_route(town, (8, 8), (64, 64))
    except AssertionError as error:
        failures.append(str(error))
    if dungeon[72 * WIDTH + 64] != 13:
        failures.append("dungeon gate is not closed in the base map")
    try:
        assert_route(dungeon, (64, 64), (80, 64))
        assert_no_route(dungeon, (64, 64), (64, 104))
        opened_dungeon = list(dungeon)
        opened_dungeon[72 * WIDTH + 64] = 3
        assert_route(opened_dungeon, (64, 64), (64, 104))
    except AssertionError as error:
        failures.append(str(error))

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}")
        return 1

    print("PASS: world tiles and map layout are readable and deterministic")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
