#include "types.h"
#include "memory.h"
#include "SaveGame.h"
#include "KASFunc.h"

typedef struct KasEntryTableEntry
{
    sdword values[3];
} KasEntryTableEntry;

static KasEntryTableEntry *kasfEntryTable = NULL;
static sdword kasfEntryTableEntries = 0;
static sdword kasfEntryTableCapacity = 0;
static sdword kasfEntryTableNextId = 0;

void kasfEntryTableSave(void)
{
    sdword i;

    SaveInfoNumber(kasfEntryTableEntries);
    SaveInfoNumber(kasfEntryTableCapacity);
    SaveInfoNumber(kasfEntryTableNextId);

    for (i = 0; i < kasfEntryTableEntries; i++)
    {
        SaveStructureOfSize(&kasfEntryTable[i], sizeof(KasEntryTableEntry));
    }
}

void kasfEntryTableLoad(void)
{
    sdword i;

    kasfEntryTableEntries = LoadInfoNumber();
    kasfEntryTableCapacity = LoadInfoNumber();
    kasfEntryTableNextId = LoadInfoNumber();

    if (kasfEntryTable != NULL)
    {
        memFree(kasfEntryTable);
        kasfEntryTable = NULL;
    }

    if (kasfEntryTableCapacity <= 0)
    {
        return;
    }

    dbgAssert(kasfEntryTableEntries <= kasfEntryTableCapacity);
    kasfEntryTable = memAlloc(sizeof(KasEntryTableEntry) * kasfEntryTableCapacity, "kasfEntryTable", 0);
    for (i = 0; i < kasfEntryTableEntries; i++)
    {
        LoadStructureOfSizeToAddress(&kasfEntryTable[i], sizeof(KasEntryTableEntry));
    }
}
