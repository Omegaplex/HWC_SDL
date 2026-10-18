#ifndef BATTLECHAT_H
#define BATTLECHAT_H

#include "types.h"
#include "prim2d.h"
#include "spaceobj.h"

#define BATTLE_CHAT_MAX_MESSAGES 25
#define BATTLE_CHAT_MAX_VISIBLE 5

typedef struct BattleChatEntry
{
    char      message[512];
    rectangle rect;
    bool32    highlighted;
    real32    flashStartTime;
    sdword    priority;
    Ship     *sourceShip;
    color     color;
} BattleChatEntry;

extern sdword battleChatMessageCount;
extern sdword battleChatVisibleCount;
extern sdword battleChatCurrentIndex;
extern BattleChatEntry battleChatEntries[BATTLE_CHAT_MAX_MESSAGES];

int __cdecl battleChatAddMessage(const char *message, int color, int priority, int sourceShip);
void __cdecl battleChatClearShipMessages(Ship *ship);
void __cdecl battleChatDiscardOldestMessage(void);
void battleChatLock(void);
void battleChatUnlock(void);

#endif
