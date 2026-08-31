# Lantern of Veyra

An original Sega Genesis / Mega Drive homebrew RPG built with SGDK.

The night has a lock. Find the key. Carry the light home.

Lantern of Veyra is a compact, one-hero adventure about a town trapped in a
long dusk. Talk to the Guide, cross the Stonewake dungeon, recover the lantern
relic, and return to Veyra alive. It is intentionally small and cartridge-like:
one town, one dungeon, a short upgrade path, readable tile-map exploration, and
turn-based battles driven by the Genesis PSG.

## Read the game

- [Digital field guide](docs/index.html) - responsive, spoiler-aware browser edition
- [Printable player's manual](docs/print-manual.html) - print or save as PDF
- [PDF player's manual](docs/Lantern-of-Veyra-Players-Manual.pdf) - fixed release export
- [Manual source](docs/manual.md) - the canonical release text

## Build

The project targets SGDK 2.11 or a compatible release. SGDK and its toolchain
are not vendored here. Install SGDK separately, then set `GDK` to its folder.
Java is required by SGDK's resource tools.

```powershell
$env:GDK = 'C:\path\to\SGDK'
.\build.ps1
```

The release ROM is written to `out\rom.bin`. That folder is intentionally
ignored by Git; build products belong in a local checkout or a release asset,
not in the source history. A compatible Genesis emulator can open the ROM with
its Genesis / Mega Drive hardware profile.

## Validate

The checks are deliberately split between host logic and source-world data:

```powershell
python -m pip install -r requirements.txt
python tests\validate_world.py
powershell -ExecutionPolicy Bypass -File tests\run_host_smoke.ps1
```

`validate_world.py` checks deterministic maps, tile readability, portals, and
the locked/open dungeon route. `run_host_smoke.ps1` compiles the gameplay
modules against a small SGDK stub and exercises movement, camera scrolling,
map redraw bounds, audio sequencing, portal transitions, saving, menus,
encounters, battle actions, and the dungeon progression path.

## Project shape

```text
inc/                 public C headers
src/                 game code and Genesis boot/header sources
res/                 indexed art and SGDK resource manifest
tests/               host smoke harness and world validator
docs/                print manual and digital field guide
generate_maps.py     deterministic map-data generator
prep_assets.py       public-safe indexed art preparation helper
build.ps1            SGDK build wrapper using the GDK environment variable
```

The checked-in map data is intentional source content, not a transient build
cache. Generated SGDK resource headers, object files, ROMs, Python caches, and
local scratch notes are excluded by `.gitignore`.

## Development notes

- Map tile IDs are kept stable so the 8x8 indexed art and collision rules agree.
- The game uses SRAM save data with a checksum and a versioned payload.
- The supplied boot startup is based on SGDK's startup source; see
  `THIRD_PARTY_NOTICES.md` for attribution boundaries.
- The original project assets are small, indexed, and intentionally free of
  external runtime dependencies. Replace any asset whose source rights do not
  belong to your distribution before publishing a derivative.

## License

Original game code, documentation, and project-authored assets are released
under the MIT License in `LICENSE`. SGDK itself remains under its own license;
it is an external build dependency.
