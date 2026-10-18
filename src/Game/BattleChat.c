/*=============================================================================
    Name    : BattleChat.c
    Purpose : Decompiled functions from cataclysm.exe (BattleChat module)
=============================================================================*/

#include "BattleChat.h"

#include "debug.h"
#include "Globals.h"
#include "MultiplayerGame.h"
#include "SoundEvent.h"
#include "StringsOnly.h"

#include <string.h>

sdword battleChatMessageCount = 0;
sdword battleChatVisibleCount = 0;
sdword battleChatCurrentIndex = 0;
BattleChatEntry battleChatEntries[BATTLE_CHAT_MAX_MESSAGES];

void battleChatLock(void)
{
    dbgAssert(gMessageMutex != NULL);
    LockMutex(gMessageMutex);
}

void battleChatUnlock(void)
{
    if (gMessageMutex != NULL)
    {
        UnLockMutex(gMessageMutex);
    }
}

static void battleChatDiscardOldestMessageNoLock(void)
{
    sdword index;

    if (battleChatMessageCount <= 1)
    {
        if (battleChatMessageCount > 0)
        {
            battleChatMessageCount--;
        }
        battleChatCurrentIndex = 0;
        battleChatVisibleCount = 0;
        return;
    }

    for (index = 0; index < battleChatMessageCount - 1; index++)
    {
        strcpy(battleChatEntries[index].message, battleChatEntries[index + 1].message);
        battleChatEntries[index].priority = battleChatEntries[index + 1].priority;
        battleChatEntries[index].sourceShip = battleChatEntries[index + 1].sourceShip;
        battleChatEntries[index].color = battleChatEntries[index + 1].color;
        battleChatEntries[index].highlighted = FALSE;
        battleChatEntries[index].flashStartTime = 0.0f;
    }

    memset(&battleChatEntries[battleChatMessageCount - 1], 0, sizeof(BattleChatEntry));
    battleChatMessageCount--;
    if (battleChatCurrentIndex > 0)
    {
        battleChatCurrentIndex--;
    }
    if (battleChatVisibleCount > battleChatMessageCount)
    {
        battleChatVisibleCount = battleChatMessageCount;
    }
}

void __cdecl battleChatDiscardOldestMessage(void)
{
    battleChatLock();
    battleChatDiscardOldestMessageNoLock();
    battleChatUnlock();
}

void __cdecl battleChatClearShipMessages(Ship *ship)
{
    sdword index;

    battleChatLock();
    for (index = 0; index < battleChatMessageCount; index++)
    {
        if (battleChatEntries[index].priority == 1 && battleChatEntries[index].sourceShip == ship)
        {
            battleChatEntries[index].priority = 0;
            battleChatEntries[index].sourceShip = NULL;
        }
    }
    battleChatUnlock();
}

int __cdecl battleChatAddMessage(const char *message, int color, int priority, int sourceShip)
{
    sdword entryIdx;
    bool32 isDuplicate;
    BattleChatEntry *entry;

    battleChatLock();

    entryIdx = battleChatMessageCount;
    if (entryIdx >= BATTLE_CHAT_MAX_MESSAGES)
    {
        battleChatDiscardOldestMessageNoLock();
        entryIdx = battleChatMessageCount;
    }

    entry = &battleChatEntries[entryIdx];
    strcpy(entry->message, message);
    entry->highlighted = FALSE;
    entry->flashStartTime = 0.0f;
    entry->color = color;
    entry->priority = priority;
    entry->sourceShip = (Ship *)sourceShip;

    isDuplicate = (battleChatCurrentIndex == entryIdx - 1);
    battleChatCurrentIndex = entryIdx;
    if (isDuplicate)
    {
        if (++battleChatVisibleCount > BATTLE_CHAT_MAX_VISIBLE)
        {
            battleChatVisibleCount = BATTLE_CHAT_MAX_VISIBLE;
        }
    }
    else
    {
        battleChatVisibleCount = entryIdx + 1;
        if (battleChatVisibleCount > BATTLE_CHAT_MAX_VISIBLE)
        {
            battleChatVisibleCount = BATTLE_CHAT_MAX_VISIBLE;
        }
    }

    battleChatMessageCount = entryIdx + 1;

    if (strcmp(strGetString(strSavedGame), message))
    {
        soundEventPlay(NULL, 298, NULL);
    }

    battleChatUnlock();
    return (int)entry;
}
