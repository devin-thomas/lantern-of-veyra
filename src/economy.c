#include "economy.h"
#include "audio.h"
#include "battle.h"
#include "inventory.h"
#include "ui.h"

#define SHOP_OFFER_COUNT 3
#define SHOP_BOX_X 0
#define SHOP_BOX_Y 4
#define SHOP_BOX_W 40
#define SHOP_BOX_H 21
#define SHOP_FIRST_ROW 10

static const u8 shop_offers[SHOP_OFFER_COUNT] = {
    ITEM_POTION,
    ITEM_IRON_SWORD,
    ITEM_CHAIN_ARMOR
};

static bool shop_has_offer(u8 item_id) {
    for (u8 i = 0; i < SHOP_OFFER_COUNT; i++) {
        if (shop_offers[i] == item_id) return TRUE;
    }
    return FALSE;
}

bool shop_purchase(u8 item_id) {
    const InventoryItem *item = inventory_get_item(item_id);
    if (item == NULL || !shop_has_offer(item_id) ||
        player_stats.gold < item->price ||
        !inventory_add_item(item_id, 1)) {
        return FALSE;
    }

    player_stats.gold -= item->price;
    return TRUE;
}

bool inn_rest(void) {
    if (player_stats.hp == player_stats.max_hp &&
        player_stats.mp == player_stats.max_mp) {
        return FALSE;
    }
    if (player_stats.gold < INN_PRICE) return FALSE;

    player_stats.gold -= INN_PRICE;
    player_restore_at_town();
    return TRUE;
}

static u16 wait_for_action(void) {
    const u16 buttons = BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START;
    u16 joy;

    do {
        joy = JOY_readJoypad(JOY_1);
        SYS_doVBlankProcess();
    } while (joy & buttons);

    do {
        joy = JOY_readJoypad(JOY_1);
        SYS_doVBlankProcess();
    } while (!(joy & buttons));
    return joy;
}

static bool confirm_prompt(const char *prompt) {
    ui_draw_box(2, 9, 36, 8);
    ui_show_text(prompt);
    VDP_drawText("A YES  B NO", 14, 15);
    const u16 joy = wait_for_action();
    const bool confirmed = (joy & BUTTON_A) != 0;
    ui_clear_box();
    return confirmed;
}

static void show_inn_result(const char *message) {
    ui_draw_box(2, 9, 36, 8);
    VDP_drawText("VEYRA INN", 15, 9);
    ui_show_text(message);
    VDP_drawText("A/B CLOSE", 15, 15);
    wait_for_action();
    ui_clear_box();
}

static void draw_shop_screen(u8 selection, const char *message) {
    char line[41];

    ui_draw_box(SHOP_BOX_X, SHOP_BOX_Y, SHOP_BOX_W, SHOP_BOX_H);
    VDP_drawText("VEYRA SHOP", 16, 4);
    sprintf(line, "GOLD %lu", (unsigned long) player_stats.gold);
    VDP_drawText(line, 2, 6);
    VDP_drawText("CATALOG", 2, 8);

    for (u8 i = 0; i < SHOP_OFFER_COUNT; i++) {
        const InventoryItem *item = inventory_get_item(shop_offers[i]);
        const u16 row = SHOP_FIRST_ROW + i;
        VDP_drawText(i == selection ? ">" : " ", 2, row);
        sprintf(line, "%s  %uG  x%d", item->name, (unsigned) item->price,
                (int) inventory_get_count(shop_offers[i]));
        VDP_drawText(line, 4, row);
    }

    if (message != NULL) VDP_drawText(message, 2, 19);
    VDP_drawText("A BUY  B CLOSE", 2, 22);
}

bool shop_open(void) {
    u8 selection = 0;
    bool changed = FALSE;
    const char *message = NULL;
    u16 previous_joy = JOY_readJoypad(JOY_1);

    draw_shop_screen(selection, message);
    while (1) {
        const u16 joy = JOY_readJoypad(JOY_1);
        const u16 pressed = joy & (u16) ~previous_joy;
        previous_joy = joy;

        if (pressed & BUTTON_UP) {
            selection = selection == 0 ? SHOP_OFFER_COUNT - 1 : selection - 1;
            message = NULL;
            draw_shop_screen(selection, message);
        } else if (pressed & BUTTON_DOWN) {
            selection = (selection + 1) % SHOP_OFFER_COUNT;
            message = NULL;
            draw_shop_screen(selection, message);
        }

        if (pressed & (BUTTON_B | BUTTON_C | BUTTON_START)) break;

        if (pressed & BUTTON_A) {
            const u8 item_id = shop_offers[selection];
            const InventoryItem *item = inventory_get_item(item_id);
            char prompt[64];
            sprintf(prompt, "Buy %s for %u gold?", item->name,
                    (unsigned) item->price);
            message = "CANCELED";

            if (confirm_prompt(prompt)) {
                if (shop_purchase(item_id)) {
                    message = "PURCHASED";
                    play_sfx_select();
                    changed = TRUE;
                } else if (player_stats.gold < item->price) {
                    message = "NOT ENOUGH GOLD";
                } else {
                    message = "ALREADY OWNED";
                }
            }
            draw_shop_screen(selection, message);
        }

        SYS_doVBlankProcess();
    }

    ui_clear_box();
    return changed;
}

bool inn_open(void) {
    char line[41];
    bool changed = FALSE;

    ui_draw_box(2, 9, 36, 8);
    VDP_drawText("VEYRA INN", 15, 9);
    sprintf(line, "Rest and restore HP/MP for %d gold?", INN_PRICE);
    ui_show_text(line);
    VDP_drawText("A REST  B LEAVE", 13, 15);

    const u16 joy = wait_for_action();
    if (joy & BUTTON_A) {
        if (player_stats.hp == player_stats.max_hp &&
            player_stats.mp == player_stats.max_mp) {
            show_inn_result("HP and MP are already full.");
        } else if (player_stats.gold < INN_PRICE) {
            show_inn_result("You do not have enough gold.");
        } else if (inn_rest()) {
            play_sfx_select();
            changed = TRUE;
        } else {
            show_inn_result("The inn cannot restore you right now.");
        }
    }

    ui_clear_box();
    return changed;
}
