#include "inventory.h"
#include "battle.h"
#include "quest.h"
#include "save.h"
#include "ui.h"

#define INVENTORY_BOX_X 0
#define INVENTORY_BOX_Y 3
#define INVENTORY_BOX_W 40
#define INVENTORY_BOX_H 22
#define INVENTORY_FIRST_ROW 11
#define INVENTORY_LAST_ROW 16

const InventoryItem inventory_items[ITEM_COUNT] = {
    {"EMPTY", ITEM_KIND_KEY, 0, 0, 0, 0, 0},
    {"POTION", ITEM_KIND_CONSUMABLE, 0, 0, 15, 99, 5},
    {"BRONZE SWORD", ITEM_KIND_WEAPON, 2, 0, 0, 1, 20},
    {"IRON SWORD", ITEM_KIND_WEAPON, 4, 0, 0, 1, 50},
    {"LEATHER ARMOR", ITEM_KIND_ARMOR, 0, 1, 0, 1, 15},
    {"CHAIN ARMOR", ITEM_KIND_ARMOR, 0, 3, 0, 1, 45},
    {"LANTERN RELIC", ITEM_KIND_KEY, 0, 0, 0, 1, 0},
    {"RUNE KEY", ITEM_KIND_KEY, 0, 0, 0, 1, 0}
};

static const char *equipped_name(u8 item_id) {
    const InventoryItem *item = inventory_get_item(item_id);
    return item == NULL || item_id == ITEM_NONE ? "NONE" : item->name;
}

void inventory_reset(void) {
    for (u8 i = 0; i < SAVE_INVENTORY_SLOTS; i++) {
        game_save_data.inventory[i] = 0;
    }

    game_save_data.inventory[ITEM_POTION] = 2;
    game_save_data.inventory[ITEM_BRONZE_SWORD] = 1;
    game_save_data.inventory[ITEM_LEATHER_ARMOR] = 1;
    game_save_data.equipped_weapon = ITEM_BRONZE_SWORD;
    game_save_data.equipped_armor = ITEM_LEATHER_ARMOR;
    player_stats.potions = game_save_data.inventory[ITEM_POTION];
    inventory_recalculate_stats();
}

void inventory_recalculate_stats(void) {
    u32 attack = player_stats.base_atk;
    u32 defense = player_stats.base_def;
    const InventoryItem *weapon = inventory_get_item(game_save_data.equipped_weapon);
    const InventoryItem *armor = inventory_get_item(game_save_data.equipped_armor);

    if (weapon != NULL && weapon->kind == ITEM_KIND_WEAPON) {
        attack += weapon->attack_bonus;
    }
    if (armor != NULL && armor->kind == ITEM_KIND_ARMOR) {
        defense += armor->defense_bonus;
    }

    player_stats.atk = attack > 65535 ? 65535 : (u16) attack;
    player_stats.def = defense > 65535 ? 65535 : (u16) defense;
    player_stats.potions = game_save_data.inventory[ITEM_POTION];
}

const InventoryItem *inventory_get_item(u8 item_id) {
    if (item_id >= ITEM_COUNT) return NULL;
    return &inventory_items[item_id];
}

u8 inventory_get_count(u8 item_id) {
    if (item_id >= SAVE_INVENTORY_SLOTS || item_id >= ITEM_COUNT) return 0;
    return game_save_data.inventory[item_id];
}

bool inventory_add_item(u8 item_id, u8 amount) {
    const InventoryItem *item = inventory_get_item(item_id);
    if (item == NULL || item_id == ITEM_NONE || amount == 0) return FALSE;
    if ((u16) game_save_data.inventory[item_id] + amount > item->max_count) {
        return FALSE;
    }

    game_save_data.inventory[item_id] += amount;
    inventory_recalculate_stats();
    return TRUE;
}

bool inventory_remove_item(u8 item_id, u8 amount) {
    if (item_id == ITEM_NONE || item_id >= ITEM_COUNT || amount == 0 ||
        game_save_data.inventory[item_id] < amount) {
        return FALSE;
    }

    game_save_data.inventory[item_id] -= amount;
    if (game_save_data.inventory[item_id] == 0) {
        if (game_save_data.equipped_weapon == item_id) {
            game_save_data.equipped_weapon = ITEM_NONE;
        }
        if (game_save_data.equipped_armor == item_id) {
            game_save_data.equipped_armor = ITEM_NONE;
        }
    }
    inventory_recalculate_stats();
    return TRUE;
}

bool inventory_toggle_equipment(u8 item_id) {
    const InventoryItem *item = inventory_get_item(item_id);
    if (item == NULL || game_save_data.inventory[item_id] == 0) return FALSE;

    if (item->kind == ITEM_KIND_WEAPON) {
        game_save_data.equipped_weapon =
            game_save_data.equipped_weapon == item_id ? ITEM_NONE : item_id;
    } else if (item->kind == ITEM_KIND_ARMOR) {
        game_save_data.equipped_armor =
            game_save_data.equipped_armor == item_id ? ITEM_NONE : item_id;
    } else {
        return FALSE;
    }

    inventory_recalculate_stats();
    return TRUE;
}

bool inventory_use_potion(void) {
    const InventoryItem *potion = inventory_get_item(ITEM_POTION);
    if (potion == NULL || player_stats.hp >= player_stats.max_hp ||
        !inventory_remove_item(ITEM_POTION, 1)) {
        return FALSE;
    }

    player_stats.hp += potion->heal_amount;
    if (player_stats.hp > player_stats.max_hp) {
        player_stats.hp = player_stats.max_hp;
    }
    return TRUE;
}

static void draw_inventory_screen(u8 selection, const char *message) {
    char line[41];
    const u8 item_id = (u8) (selection + ITEM_POTION);

    ui_draw_box(INVENTORY_BOX_X, INVENTORY_BOX_Y,
                INVENTORY_BOX_W, INVENTORY_BOX_H);
    VDP_drawText("STATUS / INVENTORY", 2, 3);
    sprintf(line, "LV%d HP %d/%d MP %d/%d", (int) player_stats.level,
            (int) player_stats.hp, (int) player_stats.max_hp,
            (int) player_stats.mp, (int) player_stats.max_mp);
    VDP_drawText(line, 2, 5);
    sprintf(line, "ATK %d  DEF %d  EXP %lu  GOLD %lu",
            (int) player_stats.atk, (int) player_stats.def,
            (unsigned long) player_stats.exp,
            (unsigned long) player_stats.gold);
    VDP_drawText(line, 2, 6);
    VDP_drawText(quest_current_objective(), 2, 7);
    sprintf(line, "WEAPON: %s", equipped_name(game_save_data.equipped_weapon));
    VDP_drawText(line, 2, 8);
    sprintf(line, "ARMOR:  %s", equipped_name(game_save_data.equipped_armor));
    VDP_drawText(line, 2, 9);
    VDP_drawText("ITEMS / KEY ITEMS", 2, 10);

    for (u8 i = 0; i < ITEM_COUNT - ITEM_POTION; i++) {
        const u8 listed_id = (u8) (i + ITEM_POTION);
        const InventoryItem *item = inventory_get_item(listed_id);
        const u16 row = INVENTORY_FIRST_ROW + i;
        VDP_drawText(listed_id == item_id ? ">" : " ", 2, row);
        sprintf(line, "%s x%d", item->name,
                (int) inventory_get_count(listed_id));
        VDP_drawText(line, 4, row);
    }

    if (message != NULL) VDP_drawText(message, 2, 22);
    VDP_drawText("A USE/EQUIP  B CLOSE", 2, 23);
}

bool inventory_open(void) {
    u8 selection = 0;
    bool changed = FALSE;
    const char *message = NULL;
    u16 previous_joy = JOY_readJoypad(JOY_1);

    inventory_recalculate_stats();
    draw_inventory_screen(selection, message);

    while (1) {
        const u16 joy = JOY_readJoypad(JOY_1);
        const u16 pressed = joy & (u16) ~previous_joy;
        previous_joy = joy;

        if (pressed & BUTTON_UP) {
            selection = selection == 0 ? (ITEM_COUNT - ITEM_POTION) - 1 :
                        selection - 1;
            message = NULL;
            draw_inventory_screen(selection, message);
        } else if (pressed & BUTTON_DOWN) {
            selection = (selection + 1) % (ITEM_COUNT - ITEM_POTION);
            message = NULL;
            draw_inventory_screen(selection, message);
        }

        if (pressed & (BUTTON_B | BUTTON_C | BUTTON_START)) break;

        if (pressed & BUTTON_A) {
            const u8 item_id = (u8) (selection + ITEM_POTION);
            const InventoryItem *item = inventory_get_item(item_id);
            message = "CANNOT USE";

            if (item->kind == ITEM_KIND_CONSUMABLE) {
                if (inventory_use_potion()) {
                    message = "POTION USED";
                    changed = TRUE;
                } else if (inventory_get_count(ITEM_POTION) == 0) {
                    message = "NO POTIONS";
                } else {
                    message = "HP IS FULL";
                }
            } else if (item->kind == ITEM_KIND_WEAPON ||
                       item->kind == ITEM_KIND_ARMOR) {
                if (inventory_toggle_equipment(item_id)) {
                    const bool equipped =
                        item->kind == ITEM_KIND_WEAPON ?
                        game_save_data.equipped_weapon == item_id :
                        game_save_data.equipped_armor == item_id;
                    message = equipped ? "EQUIPPED" : "UNEQUIPPED";
                    changed = TRUE;
                } else {
                    message = "NOT IN INVENTORY";
                }
            }
            draw_inventory_screen(selection, message);
        }

        SYS_doVBlankProcess();
    }

    ui_clear_box();
    return changed;
}
