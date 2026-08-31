"""Prepare the public-authored indexed art set used by Lantern of Veyra.

The default sprites are drawn from simple pixel primitives so the public
repository has no dependency on an untracked source image. Pass a local path
with --hero-source only when testing an alternate hero sprite.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parent
RES = ROOT / "res"


def force_corner_transparent(img: Image.Image) -> Image.Image:
    """Put the corner color at palette index 0 for SGDK sprites."""
    transparent_index = img.getpixel((0, 0))
    if transparent_index == 0:
        img.info["transparency"] = 0
        return img

    palette = img.getpalette()
    order = [transparent_index] + [index for index in range(16)
                                   if index != transparent_index]
    remap = [0] * 256
    for new_index, old_index in enumerate(order):
        remap[old_index] = new_index

    pixels = [
        img.getpixel((x, y))
        for y in range(img.height)
        for x in range(img.width)
    ]
    img.putdata([remap[pixel] for pixel in pixels])
    new_palette = []
    for old_index in order:
        new_palette.extend(palette[old_index * 3:old_index * 3 + 3])
    new_palette.extend(palette[48:])
    img.putpalette(new_palette)
    img.info["transparency"] = 0
    return img


def save_transparent_sprite(img: Image.Image, dst_path: Path) -> None:
    """Write palette index 0 as actual PNG transparency for browser previews."""
    img.save(dst_path, transparency=0)


def finish_sprite(img: Image.Image, dst_path: Path) -> None:
    """Quantize a sprite and reserve palette index 0 for transparency."""
    indexed = img.convert("P", palette=Image.ADAPTIVE, colors=16)
    save_transparent_sprite(force_corner_transparent(indexed), dst_path)


def create_hero(dst_path: Path) -> None:
    """Draw the lantern bearer as a compact 16x16 public-authored sprite."""
    img = Image.new("RGB", (16, 16), color=(255, 0, 255))
    draw = ImageDraw.Draw(img)
    night = (12, 24, 42)
    blue = (26, 62, 83)
    teal = (42, 102, 99)
    orange = (215, 94, 45)
    gold = (240, 200, 100)
    skin = (214, 158, 112)
    cream = (246, 230, 178)

    draw.rectangle([5, 2, 10, 4], fill=night)
    draw.rectangle([6, 4, 9, 6], fill=skin)
    draw.point((9, 5), fill=night)
    draw.rectangle([4, 7, 7, 11], fill=orange)
    draw.rectangle([6, 7, 10, 12], fill=blue)
    draw.rectangle([7, 8, 9, 10], fill=teal)
    draw.rectangle([3, 9, 5, 12], fill=orange)
    draw.rectangle([5, 12, 7, 14], fill=night)
    draw.rectangle([9, 12, 11, 14], fill=night)
    draw.point((8, 13), fill=gold)
    draw.rectangle([10, 8, 12, 9], fill=skin)
    draw.rectangle([12, 8, 14, 11], fill=gold)
    draw.rectangle([13, 9, 13, 10], fill=cream)
    draw.line([12, 7, 14, 7], fill=gold, width=1)
    draw.point((14, 6), fill=orange)

    finish_sprite(img, dst_path)


def create_slime(dst_path: Path) -> None:
    """Draw the Moon Slime from blocky, low-color primitives."""
    img = Image.new("RGB", (32, 32), color=(255, 0, 255))
    draw = ImageDraw.Draw(img)
    deep = (13, 52, 89)
    blue = (37, 161, 197)
    light = (104, 218, 217)
    cream = (246, 230, 178)

    draw.polygon(
        [(4, 22), (4, 16), (7, 11), (12, 7), (20, 7),
         (25, 10), (28, 16), (28, 23), (23, 27), (9, 27)],
        fill=deep,
    )
    draw.rectangle([7, 15, 25, 23], fill=blue)
    draw.rectangle([10, 11, 22, 15], fill=blue)
    draw.rectangle([11, 10, 16, 12], fill=light)
    draw.rectangle([9, 16, 11, 18], fill=cream)
    draw.rectangle([20, 16, 22, 18], fill=cream)
    draw.rectangle([12, 21, 19, 23], fill=deep)
    draw.rectangle([8, 24, 23, 25], fill=deep)

    finish_sprite(img, dst_path)


def process_image(src_path: Path, dst_path: Path,
                  size: tuple[int, int]) -> None:
    img = Image.open(src_path)

    if img.mode in ("RGBA", "LA") or (
        img.mode == "P" and "transparency" in img.info
    ):
        background = Image.new("RGB", img.size, (255, 0, 255))
        background.paste(img, mask=img.convert("RGBA").split()[3])
        img = background

    img = img.resize(size, Image.NEAREST).convert(
        "P", palette=Image.ADAPTIVE, colors=16
    )
    img = force_corner_transparent(img)
    save_transparent_sprite(img, dst_path)


def create_enemy(dst_path: Path, size: tuple[int, int],
                 main_color: tuple[int, int, int],
                 eye_color: tuple[int, int, int],
                 accent_color: tuple[int, int, int],
                 shadow_color: tuple[int, int, int]) -> None:
    img = Image.new("RGB", size, color=(255, 0, 255))
    draw = ImageDraw.Draw(img)

    if size[0] <= 32:
        # A small bone sentinel with a readable skull, ribs, and separated limbs.
        draw.rectangle([8, 3, 23, 13], fill=main_color)
        draw.rectangle([5, 6, 7, 12], fill=shadow_color)
        draw.rectangle([24, 6, 26, 12], fill=shadow_color)
        draw.rectangle([10, 7, 12, 9], fill=eye_color)
        draw.rectangle([19, 7, 21, 9], fill=eye_color)
        draw.rectangle([11, 12, 20, 14], fill=shadow_color)
        draw.rectangle([10, 16, 21, 24], fill=main_color)
        draw.rectangle([12, 18, 19, 20], fill=shadow_color)
        draw.rectangle([13, 22, 18, 23], fill=shadow_color)
        draw.rectangle([4, 16, 9, 18], fill=main_color)
        draw.rectangle([22, 16, 27, 18], fill=main_color)
        draw.rectangle([7, 25, 12, 28], fill=main_color)
        draw.rectangle([19, 25, 24, 28], fill=main_color)
        draw.point((15, 15), fill=accent_color)
        draw.point((17, 15), fill=accent_color)
    else:
        # The Ash Dragon is built from hard-edged blocks so it stays legible at 8x scale.
        draw.polygon(
            [(7, 24), (15, 14), (25, 18), (22, 31), (10, 34)],
            fill=shadow_color,
        )
        draw.polygon(
            [(39, 20), (49, 11), (57, 20), (57, 31), (45, 32)],
            fill=shadow_color,
        )
        draw.rectangle([20, 22, 45, 44], fill=main_color)
        draw.rectangle([29, 14, 49, 30], fill=main_color)
        draw.rectangle([44, 18, 59, 26], fill=main_color)
        draw.rectangle([52, 24, 62, 29], fill=accent_color)
        draw.rectangle([24, 39, 31, 53], fill=main_color)
        draw.rectangle([37, 39, 44, 53], fill=main_color)
        draw.rectangle([17, 49, 27, 57], fill=shadow_color)
        draw.rectangle([41, 49, 51, 57], fill=shadow_color)
        draw.polygon(
            [(20, 42), (10, 49), (4, 56), (10, 58), (24, 51)],
            fill=main_color,
        )
        draw.rectangle([35, 16, 38, 19], fill=eye_color)
        draw.rectangle([47, 16, 50, 19], fill=eye_color)
        draw.rectangle([34, 31, 38, 34], fill=accent_color)
        draw.rectangle([40, 31, 44, 34], fill=accent_color)

    finish_sprite(img, dst_path)


def create_tileset(dst_path: Path) -> None:
    img = Image.new("RGB", (32, 32), color=(22, 88, 42))
    draw = ImageDraw.Draw(img)
    grass = (28, 120, 45)
    grass_dark = (16, 83, 39)
    grass_light = (62, 153, 61)
    path = (167, 111, 51)
    path_dark = (105, 67, 37)
    path_light = (205, 151, 78)
    stone = (82, 95, 109)
    stone_light = (124, 133, 143)
    stone_dark = (45, 53, 66)
    portal = (35, 52, 99)
    portal_light = (80, 156, 219)
    gold = (223, 179, 67)
    wall = (20, 71, 39)
    wall_light = (35, 105, 50)
    house = (146, 46, 42)
    house_dark = (87, 31, 34)

    def tile_rect(index: int) -> list[int]:
        tx = (index % 4) * 8
        ty = (index // 4) * 8
        return [tx, ty, tx + 7, ty + 7]

    def tile_point(index: int, x: int, y: int,
                  color: tuple[int, int, int]) -> None:
        tx = (index % 4) * 8
        ty = (index // 4) * 8
        draw.point((tx + x, ty + y), fill=color)

    def tile_line(index: int, start: tuple[int, int], end: tuple[int, int],
                  color: tuple[int, int, int]) -> None:
        tx = (index % 4) * 8
        ty = (index // 4) * 8
        draw.line((tx + start[0], ty + start[1],
                   tx + end[0], ty + end[1]), fill=color)

    # Tile 0 is a safe fallback; map data uses the explicit source indices.
    draw.rectangle(tile_rect(0), fill=(22, 88, 42))
    tile_point(0, 2, 5, grass_dark)
    tile_point(0, 6, 2, grass_light)

    # Town grass and path.
    draw.rectangle(tile_rect(1), fill=grass)
    for x, y, color in ((1, 1, grass_light), (5, 2, grass_dark),
                        (3, 6, grass_dark), (7, 5, grass_light)):
        tile_point(1, x, y, color)
    draw.rectangle(tile_rect(2), fill=path)
    tile_line(2, (0, 1), (7, 1), path_light)
    tile_line(2, (0, 6), (7, 6), path_dark)
    tile_point(2, 3, 4, path_dark)
    tile_point(2, 6, 3, path_light)

    # Dungeon floor and portal.
    draw.rectangle(tile_rect(3), fill=stone)
    tile_line(3, (0, 0), (7, 0), stone_light)
    tile_line(3, (0, 7), (7, 7), stone_dark)
    tile_point(3, 2, 3, stone_dark)
    tile_point(3, 6, 5, stone_light)
    draw.rectangle(tile_rect(4), fill=grass)
    tile_line(4, (0, 0), (7, 0), grass_light)
    tile_line(4, (0, 7), (7, 7), grass_dark)
    draw.rectangle(tile_rect(5), fill=portal)
    draw.rectangle([42, 10, 45, 13], fill=portal_light)
    tile_point(5, 3, 3, gold)

    # Alternate readable variants keep unused slots safe for future maps.
    draw.rectangle(tile_rect(6), fill=path)
    tile_line(6, (1, 0), (1, 7), path_dark)
    tile_line(6, (6, 0), (6, 7), path_light)
    draw.rectangle(tile_rect(7), fill=stone)
    tile_line(7, (1, 1), (6, 1), stone_light)
    tile_line(7, (1, 6), (6, 6), stone_dark)
    draw.rectangle(tile_rect(8), fill=grass)
    tile_line(8, (0, 3), (7, 3), grass_dark)
    tile_line(8, (0, 5), (7, 5), grass_light)
    draw.rectangle(tile_rect(9), fill=portal)
    tile_point(9, 2, 2, portal_light)
    tile_point(9, 5, 5, gold)
    draw.rectangle(tile_rect(10), fill=wall)
    tile_line(10, (0, 1), (7, 1), wall_light)
    tile_line(10, (0, 5), (7, 5), stone_dark)

    # Town boundary, buildings, and dungeon walls.
    draw.rectangle(tile_rect(11), fill=wall)
    tile_line(11, (0, 1), (7, 1), wall_light)
    tile_line(11, (1, 6), (6, 6), grass_dark)
    draw.rectangle(tile_rect(12), fill=house)
    tile_line(12, (0, 1), (7, 1), house_dark)
    tile_line(12, (0, 5), (7, 5), house_dark)
    tile_point(12, 2, 3, path_light)
    tile_point(12, 5, 3, path_light)
    draw.rectangle(tile_rect(13), fill=stone_dark)
    tile_line(13, (0, 1), (7, 1), stone_light)
    tile_line(13, (0, 5), (7, 5), stone)
    tile_point(13, 3, 3, stone_light)
    draw.rectangle(tile_rect(14), fill=wall)
    tile_line(14, (0, 0), (7, 7), wall_light)
    draw.rectangle(tile_rect(15), fill=house_dark)
    tile_line(15, (0, 2), (7, 2), house)
    tile_line(15, (0, 5), (7, 5), house)

    img = img.convert("P", palette=Image.ADAPTIVE, colors=16)
    img.save(dst_path)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--hero-source",
        type=Path,
        help="optional local image used to regenerate res/hero.png",
    )
    args = parser.parse_args()

    RES.mkdir(parents=True, exist_ok=True)
    if args.hero_source is not None:
        process_image(args.hero_source, RES / "hero.png", (16, 16))
    else:
        create_hero(RES / "hero.png")

    create_slime(RES / "slime.png")

    create_enemy(
        RES / "skeleton.png", (32, 32), (200, 200, 200),
        (255, 0, 0), (225, 176, 58), (86, 96, 110)
    )
    create_enemy(
        RES / "dragon.png", (64, 64), (0, 150, 0),
        (255, 255, 0), (225, 91, 43), (0, 69, 61)
    )
    create_tileset(RES / "tileset.png")


if __name__ == "__main__":
    main()
