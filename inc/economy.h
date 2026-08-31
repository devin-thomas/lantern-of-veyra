#ifndef ECONOMY_H
#define ECONOMY_H

#include <genesis.h>

#define INN_PRICE 10

bool shop_purchase(u8 item_id);
bool shop_open(void);
bool inn_rest(void);
bool inn_open(void);

#endif
