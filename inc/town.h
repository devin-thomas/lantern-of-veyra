#ifndef TOWN_H
#define TOWN_H

#include <genesis.h>

#define TOWN_GUIDE_X 112
#define TOWN_GUIDE_Y 104
#define TOWN_WATCHER_X 264
#define TOWN_WATCHER_Y 264
#define TOWN_SHOP_X 264
#define TOWN_SHOP_Y 104
#define TOWN_INN_X 744
#define TOWN_INN_Y 744

void town_init(void);
void town_update(void);
bool town_interact(void);

#endif
