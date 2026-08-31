# LANTERN OF VEYRA

## Player's Field Manual

**The Last Light**

For use with the Sega Genesis / Mega Drive version of the game.

> The night has a lock. Find the key. Carry the light home.

---

## 1. The Long Dusk

Veyra was built where three old roads meet. For generations, its watchfires
were fed by a lantern recovered from the ruins beneath the hill. Then the
lantern went dark.

The darkness did not pass with the morning. It stayed. Crops stopped growing,
travelers stopped coming, and the town learned to measure time by the coldness
of the sky.

The old Guide believes the lantern's heart is still below the sealed Stonewake
dungeon. The ruin is guarded by an Ash Dragon and closed behind a rune gate.
You are the one traveler willing to take the road down, find the lantern relic,
and bring its light back to Veyra.

The Guide will give you the first direction. The Watcher will tell you when you
are ready for deeper halls. The rest is written in stone, steel, and the
choices you make in battle.

## 2. Begin the Adventure

On the title screen:

- If no save exists, press any face button or START to begin **NEW GAME**.
- If a save exists, use UP and DOWN to choose **CONTINUE** or **NEW GAME**,
  then press A, B, C, or START.

Every new hero begins with:

| Measure | Starting value |
| --- | ---: |
| Level | 1 |
| HP | 20 / 20 |
| MP | 8 / 8 |
| Attack | 7 with the Bronze Sword |
| Defense | 4 with Leather Armor |
| Gold | 0 |
| Potions | 2 |

You begin in Veyra near the center road. The Guide is marked `G GUIDE`.
Walk close and press A to speak.

## 3. Controls

### Field

| Input | Action |
| --- | --- |
| D-pad | Walk in four directions |
| A | Talk, inspect, open, or enter a battle action |
| B | Open the pause/status screen |
| START | Save your adventure |

The field is built on an 8x8 tile grid. Your hero moves two pixels at a time;
the map scrolls as you approach the edge of the screen.

### Pause / Status

| Input | Action |
| --- | --- |
| A | Open the inventory |
| C | Open settings |
| B | Resume the field |
| START | Close the pause screen |

The status screen shows your level, current HP and MP, attack, defense, gold,
equipment, quest objective, area, position, and the next navigation landmark.

### Inventory

Use UP and DOWN to select an item. Press A to use a potion or equip/unequip a
weapon or armor. Press B, C, or START to close.

### Settings

Use UP and DOWN to select text speed, music, or sound effects. Use LEFT and
RIGHT to change the selected setting. Press A, B, or START to return.

### Battle

Use the D-pad to move the cursor. Press A, B, C, or START to confirm the
highlighted battle action. The five actions are:

| Action | Result |
| --- | --- |
| FIGHT | Strike with your current attack |
| SKILL | Choose Heal or Power Strike |
| ITEM | Use one Potion |
| GUARD | Halve the next enemy hit |
| RUN | Attempt to escape from a normal encounter |

Inside the skill menu, use UP and DOWN, press A to choose, or press B, C, or
START to cancel.

## 4. The Road Through Stonewake

This section contains the complete route. Read it after your first attempt if
you want to preserve the discovery of the dungeon.

1. Speak with the Guide in Veyra. Your objective becomes **FIND THE LANTERN
   RELIC**.
2. Follow the town roads to the blue portal at the crossroads. The portal is
   at map position `(64,64)`.
3. In Stonewake, find the **RUNE KEY** in the eastern chamber at `(80,64)`.
4. Return to the sealed gate at `(64,72)` and press A. The rune key opens the
   way to the lower halls.
5. Visit the **TREASURE** at `(48,80)`. The chest gives its best remaining
   reward: an Iron Sword, Chain Armor, or three Potions, plus 25 gold.
6. Touch the **CHECKPOINT** at `(80,80)`. It restores HP and MP and records a
   save. This is the safest place to prepare for the altar.
7. Reach the **RELIC ALTAR** at `(64,104)`. It will not awaken until the rune
   gate is open, the treasure is claimed, and the checkpoint is touched.
8. Press A at the awakened altar, then face the Ash Dragon. After victory,
   return north through the open gate and step onto the town portal.
9. Speak with the Guide in Veyra to complete the quest. The ending sequence
   will preserve your completed adventure before returning to the title screen.

### Stonewake landmark chart

```text
                             N

                 RUNE KEY (80,64)
                         |
             PORTAL (64,64) --- SEALED GATE (64,72)
                                  |
        TREASURE (48,80) --- CHECKPOINT (80,80)
                                  |
                         RELIC ALTAR (64,104)

                             S
```

The chart is a navigation aid, not a scale map. Use the pause screen's
position readout and destination line when the corridors look alike.

## 5. Battlecraft

Normal encounters occur only in Stonewake. The upper halls are calmer than the
lower halls. After four movement steps, an encounter may occur; if luck stays
quiet, an encounter is forced by the eighth step. Landmark areas are safe and
reset the danger meter.

The upper halls have a 10 percent chance after the minimum step count. The
lower halls have an 18 percent chance. The danger meter on the field HUD shows
the current count from `0/8` to `8/8`.

### Damage and defense

Basic damage is based on attack minus defense, with a small 0-2 damage swing.
Damage never falls below 1. A critical hit has a 1 in 16 chance and doubles the
result. Guard halves the next enemy hit, rounding up.

### Skills and recovery

| Skill / item | Cost | Effect |
| --- | ---: | --- |
| Heal | 3 MP | Restores 8 + 2 per level HP |
| Power Strike | 4 MP | Attacks with twice your ATK plus your level before defense |
| Potion | 0 | Restores 15 HP; maximum stock is 99 |
| Inn | 10 gold | Restores all HP and MP in Veyra |
| Checkpoint | 0 | Restores all HP and MP in Stonewake and saves |

Heal and Potions cannot be used at full HP. Run is never guaranteed; your
chance is based on the level difference and is clamped between 25 and 90
percent. The Ash Dragon cannot be escaped.

### Enemies

| Enemy | Level | HP | ATK | DEF | Reward | Trait |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| Moon Slime | 1 | 10 | 2 | 1 | 5 EXP, 2 G | Regenerates up to 3 HP before its turn |
| Bone Sentry | 2 | 25 | 6 | 4 | 15 EXP, 10 G | Can inflict Poison for 3 turns |
| Ash Dragon | 4 | 60 | 12 | 7 | 100 EXP, 50 G | Can inflict Burn for 2 turns |

The dragon in the altar battle is stronger than the normal encounter profile:

| Boss phase | Level | HP | ATK | DEF | Reward |
| --- | ---: | ---: | ---: | ---: | --- |
| Ash Dragon | 5 | 80 | 14 | 8 | 150 EXP, 100 G |
| Enraged | 5 | 80 max | 18 | 9 | Same |

When the boss reaches half HP, it enters the enraged phase before its next
attack. Its Burn lasts three turns in that phase. Poison deals 2 damage each
turn; Burn deals 3.

## 6. Steel, Supplies, and Relics

The Veyra shop is marked `S SHOP`. It sells one copy of each weapon or armor
and up to 99 Potions.

| Item | Kind | Effect | Price |
| --- | --- | --- | ---: |
| Potion | Consumable | Restore 15 HP | 5 G |
| Bronze Sword | Weapon | +2 ATK; equipped at the start | 20 G |
| Iron Sword | Weapon | +4 ATK | 50 G |
| Leather Armor | Armor | +1 DEF; equipped at the start | 15 G |
| Chain Armor | Armor | +3 DEF | 45 G |
| Lantern Relic | Key item | The light Veyra needs | - |
| Rune Key | Key item | Opens the Stonewake gate | - |

The `I INN` marker identifies the inn. Buying a stronger weapon or armor does
not equip it automatically. Open the inventory and press A on the item you
want. Press A again to remove it. You can have one weapon and one armor
equipped at a time.

## 7. Saving and Game Over

Press START on the field to write your progress to cartridge SRAM. The game
also saves after important interactions, battles, the Stonewake checkpoint,
returning to Veyra, and changes made in the pause menus.

If the hero falls in battle, the adventure ends with **ALL PROGRESS LOST**.
Press START to return to the title screen. Save before entering the lower halls
and use the checkpoint before the altar.

If an emulator asks whether to enable save RAM, enable it. A valid save stores
the current map, position, stats, inventory, equipment, story flags, and audio
settings. The save includes a version and checksum so invalid data is rejected
instead of being treated as a real adventure.

## 8. Level Growth

The level cap is 10. The total EXP needed to reach each level is:

| Reach level | Total EXP |
| ---: | ---: |
| 2 | 10 |
| 3 | 40 |
| 4 | 90 |
| 5 | 160 |
| 6 | 250 |
| 7 | 360 |
| 8 | 490 |
| 9 | 640 |
| 10 | 810 |

Each level adds 5 maximum HP, 2 maximum MP, 2 base ATK, and 1 base DEF. A
level-up restores HP and MP completely. Equipment bonuses are recalculated
after every level-up.

## 9. Production Notes

Lantern of Veyra is a small original SGDK project designed around the limits
and habits of a 16-bit cartridge RPG: 8x8 indexed tiles, short text windows,
four-channel PSG-style musical phrases, compact SRAM saves, and a map that can
be read at a glance. The digital field guide is the modern companion; this
manual is the artifact meant to be printed, folded, and kept beside the
controller.

Game code, documentation, and project-authored assets are released under the
MIT License. SGDK and its startup source retain their own upstream attribution;
see `THIRD_PARTY_NOTICES.md`.

---

**LANTERN OF VEYRA**

**The Long Dusk ends where your road begins.**
