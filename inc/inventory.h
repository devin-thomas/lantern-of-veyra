#ifndef INVENTORY_H
#define INVENTORY_H

#include <genesis.h>

typedef enum {
    ITEM_NONE = 0,
    ITEM_POTION,
    ITEM_BRONZE_SWORD,
    ITEM_IRON_SWORD,
    ITEM_LEATHER_ARMOR,
    ITEM_CHAIN_ARMOR,
    ITEM_ANCIENT_RELIC,
    ITEM_RUNE_KEY,
    ITEM_COUNT
} InventoryItemId;

typedef enum {
    ITEM_KIND_CONSUMABLE = 1,
    ITEM_KIND_WEAPON,
    ITEM_KIND_ARMOR,
    ITEM_KIND_KEY
} InventoryItemKind;

typedef struct {
    const char *name;
    InventoryItemKind kind;
    u16 attack_bonus;
    u16 defense_bonus;
    s16 heal_amount;
    u8 max_count;
    u16 price;
} InventoryItem;

extern const InventoryItem inventory_items[ITEM_COUNT];

void inventory_reset(void);
void inventory_recalculate_stats(void);
const InventoryItem *inventory_get_item(u8 item_id);
u8 inventory_get_count(u8 item_id);
bool inventory_add_item(u8 item_id, u8 amount);
bool inventory_remove_item(u8 item_id, u8 amount);
bool inventory_toggle_equipment(u8 item_id);
bool inventory_use_potion(void);
bool inventory_open(void);

#endif
