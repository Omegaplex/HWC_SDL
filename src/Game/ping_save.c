/*=============================================================================
    Name    : ping_save.c
    Purpose : Save/load support for anomaly pings.
=============================================================================*/

#include <string.h>
#include "types.h"
#include "memory.h"
#include "linkedlist.h"
#include "select.h"
#include "shipselect.h"
#include "SaveGame.h"

typedef bool32 (*pingeval)(struct ping *hellaPing, udword userID, char *userData, bool32 bRemoveReferences);

typedef struct ping
{
    Node link;
    color c;
    real32 size;
    real32 minSize;
    real32 minScreenSize;
    vector centre;
    SpaceObj *owner;
    real32 creationTime;
    real32 pingDuration;
    real32 interPingPause;
    real32 evaluatePeriod;
    real32 lastEvaluateTime;
    pingeval evaluate;
    sdword userDataSize;
    udword userID;
    real32 cameraDistanceSquared;
    udword TOMask;
} ping;

#define sizeofping(p) (sizeof(ping) + (p)->userDataSize)

static_assert(sizeof(ping) == 88, "ping save size mismatch");
static_assert(offsetof(ping, owner) == 0x2C, "ping owner offset mismatch");
static_assert(offsetof(ping, evaluate) == 0x44, "ping evaluate offset mismatch");
static_assert(offsetof(ping, userDataSize) == 0x48, "ping userDataSize offset mismatch");
static_assert(offsetof(ping, userID) == 0x4C, "ping userID offset mismatch");

LinkedList pingList;
SpaceObj *pingDyingObject;

bool32 pingAnomalyPingTimeout(struct ping *hellaPing, udword userID, char *userData, bool32 bRemoveReferences)
{
    SelectCommand *selection;
    real32 dummySize;

    (void)userID;
    (void)userData;

    selection = (SelectCommand *)(hellaPing + 1);
    if (bRemoveReferences)
    {
        if (hellaPing->owner == pingDyingObject)
        {
            return TRUE;
        }
        if (selection->numShips > 0)
        {
            selSelectionRemoveSingleShip((MaxSelection *)selection, (Ship *)pingDyingObject);
            if (selection->numShips == 0)
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    if (selection->numShips > 0)
    {
        hellaPing->centre = selCentrePointComputeGeneral((MaxSelection *)selection, &dummySize);
    }
    return FALSE;
}

void pingLoad(void)
{
    sdword i;
    sdword num = LoadInfoNumber();

    dbgAssert(pingList.num == 0);

    for (i = 0; i < num; i++)
    {
        SaveChunk *chunk = LoadNextChunk();
        ping *loadedPing;
        sdword size;

        VerifyChunkNoSize(chunk, VARIABLE_STRUCTURE | SAVE_PING);

        size = sizeofping(((ping *)chunkContents(chunk)));
        dbgAssert(size == chunk->contentsSize);

        loadedPing = memAlloc(size, "loadedPing", 0);
        memcpy(loadedPing, chunkContents(chunk), size);
        memFree(chunk);

        loadedPing->owner = (SpaceObj *)SpaceObjRegistryGetShip((sdword)loadedPing->owner);
        loadedPing->userID = (udword)SpaceObjRegistryGetShip((sdword)loadedPing->userID);
        loadedPing->evaluate = pingAnomalyPingTimeout;

        if (loadedPing->userDataSize > 0)
        {
            SpaceObjSelection *selection = LoadSelectionAndFix();
            memcpy((ubyte *)(loadedPing + 1), selection, sizeof(sdword) * selection->numSpaceObjs + 8);
            memFree(selection);
        }

        listAddNode(&pingList, &loadedPing->link, loadedPing);
    }
}

void pingSave(void)
{
    Node *node;
    sdword num = 0;

    for (node = pingList.head; node != NULL; node = node->next)
    {
        ping *thisPing = node->structptr;
        if (thisPing->evaluate == pingAnomalyPingTimeout)
        {
            num++;
        }
    }

    SaveInfoNumber(num);

    for (node = pingList.head; node != NULL; node = node->next)
    {
        ping *thisPing = node->structptr;
        if (thisPing->evaluate == pingAnomalyPingTimeout)
        {
            SaveChunk *chunk = CreateChunk(VARIABLE_STRUCTURE | SAVE_PING, sizeofping(thisPing), thisPing);
            ping *savedPing = chunkContents(chunk);

            savedPing->owner = (SpaceObj *)SpaceObjRegistryGetID((SpaceObj *)thisPing->owner);
            savedPing->userID = (udword)SpaceObjRegistryGetID((SpaceObj *)thisPing->userID);

            SaveThisChunk(chunk);
            memFree(chunk);

            if (thisPing->userDataSize > 0)
            {
                SaveSelection((SpaceObjSelection *)(thisPing + 1));
            }
        }
    }
}
