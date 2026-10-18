/*=============================================================================
    Name    : SaveGame.c
    Purpose : Code for saving and loading games (saves entire contents of universe, etc.)

    Created 98/07/12 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <intrin.h>
#include "types.h"

#include "LinkedList.h"
#include "spaceobj.h"
#include "blobs.h"
#include "shipselect.h"
#include "universe.h"
#include "univupdate.h"
#include "singleplayer.h"
#include "file.h"
#include "CameraCommand.h"
#include "clouds.h"
#include "Star3d.h"
#include "SalCapCorvette.h"
#include "ACV.h"
#include "Ship.h"
#include "RepairDroid.h"
#include "objtypes.h"

#include "tactics.h"
#include "consMgr.h"
#include "AIPlayer.h"
#include "SaveGame.h"
#include "Objectives.h"

#include "utility.h"
#include "levelload.h"
#include "BeastMothership.h"
#include "damage.h"
#include "tutor.h"
#include "TradeMgr.h"
#include "ping.h"
#include "teams.h"
#include "btg.h"

#include "light.h"
#include "commandwrap.h"
#include "clamp.h"
#include "strings.h"
#include "sensors.h"
#include "randy.h"
#include "soundevent.h"
#include "multiplayergame.h"
#include "memory.h"

void telldir(void);
#include "InfoOverlay.h"
#include "main.h"
#include "CrashLog.h"
#include "LOD.h"

void SaveConsMgr();
void LoadConsMgr();
void shipModuleSlotSave(void);
void shipModuleSlotLoad(void);
void *shipGetBeastWeaponInfo(Ship *ship);
static DockPointActivity *ShipCataDockPointActivity(Ship *ship);
static void ShipSetCataDockPointActivity(Ship *ship, DockPointActivity *dockPointActivity);

#pragma warning( 4 : 4047)      // turns off "different levels of indirection warning"

GrowSelection SpaceObjRegistry;
GrowSelection BlobRegistry;
filehandle savefile = 0;
sdword savefilestatus = 0;
static long lastLoadChunkOffset = -1;
static sdword loadSpaceObjOrdinal = -1;
extern ShipStaticInfo shipStaticInfos[TOTAL_NUM_SHIPS + 1];


#define SAVEGAME_SHIPS_INSIDE_ME_HELPER_SIZE 16

static_assert(sizeof(ShipsInsideMe) == SAVEGAME_SHIPS_INSIDE_ME_HELPER_SIZE, "ShipsInsideMe size mismatch");

typedef struct SaveCameraCommand
{
    LinkedList camerastack;
    CameraStackEntry *currentCameraStack;
    bool32 ViewEntireMissphere;
    Camera actualcamera;
    TransitionType transition;
    ActionType action;
    real32 transTimeStamp;
    sdword UserControlled;
    bool32 zoomInCloseAsPossible;
    real32 lockCameraTimeStamp;
    bool32 dontUseVelocityPredInChase;
    udword ccMode;
    CameraStackEntry *dontFocusOnMe;
} SaveCameraCommand;

static_assert(offsetof(SaveCameraCommand, dontFocusOnMe) == 0x90, "SaveCameraCommand struct layout mismatch");
static_assert(sizeof(SaveCameraCommand) == 0x94, "SaveCameraCommand struct layout mismatch");

static void SaveGameSaveSpaceObjRefAtOffset(Ship *ship, udword offset)
{
    *(sdword *)((ubyte *)ship + offset) = SpaceObjRegistryGetID(*(SpaceObj **)((ubyte *)ship + offset));
}

static void SaveGameFixSpaceObjRefAtOffset(Ship *ship, udword offset)
{
    *(void **)((ubyte *)ship + offset) = SpaceObjRegistryGetObj(*(sdword *)((ubyte *)ship + offset));
}

static void SaveGameSavePlayerRefAtOffset(Ship *ship, udword offset)
{
    Player *player = *(Player **)((ubyte *)ship + offset);

    if (player == NULL)
    {
        *(sdword *)((ubyte *)ship + offset) = -1;
    }
    else
    {
        *(sdword *)((ubyte *)ship + offset) = SavePlayerToPlayerIndex(player);
    }
}

static void SaveGameFixPlayerRefAtOffset(Ship *ship, udword offset)
{
    Player *player = *(Player **)((ubyte *)ship + offset);

    if (player == (Player *)-1)
    {
        player = NULL;
    }
    else
    {
        player = SavePlayerIndexToPlayer(player);
    }
    *(Player **)((ubyte *)ship + offset) = player;
}

static udword SaveGameSerializedShipPrefixSize(ShipStaticInfo *shipstaticinfo)
{
    udword sizespec = shipstaticinfo->custshipheader.sizeofShipSpecifics;
    udword sizeship = offsetof(Ship, ShipSpecifics) + sizeof(udword);

    /* Retail SaveShip/LoadShip use the serialized ShipSpecifics size directly
       (LoadShip 0x50053f-0x500556 starts from 0x63C and adds the static size
       field only), without the live-allocation Beast padding or the module-slot
       floor that ShipRuntimeAllocSize applies for runtime safety. */
    if (sizespec != 0)
    {
        sizeship += sizespec - sizeof(udword);
    }
    return sizeship;
}

#define CATA_SAVE_UNIVERSE_NUMPLAYERS_OFFSET offsetof(Universe, numPlayers)
#define CATA_SAVE_UNIVERSE_SERIALIZED_SIZE sizeof(Universe)

static_assert(CATA_SAVE_UNIVERSE_SERIALIZED_SIZE == 0x3D9BC, "Serialized Universe size mismatch");
static_assert(offsetof(CaptainGameInfo, unk268) == 0x268, "CaptainGameInfo serialized prefix mismatch");

// Retail save files serialize only the CaptainGameInfo prefix through playerInfo.
#define CATA_SAVE_CAPTAINGAMEINFO_SERIALIZED_SIZE offsetof(CaptainGameInfo, unk268)

static void SaveGameCopyUniverseRuntimeToSerialized(void *destination, const Universe *source)
{
    (void)destination;
    memcpy(destination, source, sizeof(Universe));
}

static void SaveGameCopyUniverseSerializedToRuntime(Universe *destination, const void *source)
{
    memcpy(destination, source, sizeof(Universe));
}

static bool32 SaveGameShouldCountLoadedShip(Ship *ship)
{
    if (ship == NULL || ship->playerowner == NULL || ship->staticinfo == NULL)
    {
        return FALSE;
    }

    return
        !ShipTypeIsTargetDroneType(ship->shiptype) &&
        ship->shiptype != HeadShotAsteroid &&
        ship->shiptype != CryoTray;
}

static void SaveGameClearLoadedPosRuntimePad(PosInfo *posinfo)
{
    // Retail LoadSpaceObj zeroes the first two Cataclysm-added PosInfo dwords
    // after memcpy for generic loaded space objects.
    memset(posinfo->posPad, 0, 8);
}


static void SaveGameRebuildLoadedPlayerShipTotals(void)
{
    Node *node;
    sdword i;

    for (i = 0; i <= MAX_MULTIPLAYER_PLAYERS; i++)
    {
        universe.players[i].totalships = 0;
        memset(universe.players[i].shiptotals, 0, sizeof(universe.players[i].shiptotals));
        memset(universe.players[i].classtotals, 0, sizeof(universe.players[i].classtotals));
    }

    for (node = universe.ShipList.head; node != NULL; node = node->next)
    {
        Ship *ship = listGetStructOfNode(node);

        bitClear(ship->specialFlags1C8, CATA_RUNTIME_IN_UNIT_CAPS);

        if (!SaveGameShouldCountLoadedShip(ship))
        {
            continue;
        }

        ship->playerowner->totalships++;
        ship->playerowner->shiptotals[ship->shiptype]++;
        ship->playerowner->classtotals[ship->staticinfo->shipclass]++;
        bitSet(ship->specialFlags1C8, CATA_RUNTIME_IN_UNIT_CAPS);
    }
}

/*=============================================================================
    Private Function prototypes
=============================================================================*/

void RegisterAllBlobs(void);
void RegisterAllSpaceObjs(void);
void LoadAllBlobs();
void SaveAllBlobs();
void FixAllBlobs();
void SaveBlob(blob *tblob);
blob *LoadBlob(void);
void FixBlob(blob *tblob);
void SaveAllSpaceObjs();
void LoadAllSpaceObjs();
void FixAllSpaceObjs();
SpaceObj *LoadSpaceObj();
void FixSpaceObj(SpaceObj *obj);
void FixShip(Ship *ship);
void FixBullet(Bullet *bullet);
void FixAsteroid(Asteroid *asteroid);
void FixNebula(Nebula *nebula);
void FixDustCloud(DustCloud *dustcloud);
void FixDerelict(Derelict *derelict);
void FixMissile(Missile *missile);
void SaveSpaceObj(SpaceObj *obj);
void LoadUniverse();
void SaveUniverse();
sdword SaveShipNamePointers(void);
void SaveShipNames(void);
void LoadShipNames(void);
void FixShipNamePointers(void);
void SaveShockwaves(void);
void LoadShockwaves(void);
void univSaveCountTables(void);
void univLoadCountTables(void);
void Load_StringToAddress(char *addr);

typedef void (*ListAddCB)(LinkedList *list,SpaceObj *obj);

void SaveLinkedListOfSpaceObjs(LinkedList *list);
void LoadLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB);
void JustLoadLinkedListOfSpaceObjs(LinkedList *list);
void FixLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB);

extern sdword actorFlagsEnabled;

#define CATA_MEMGROWTH_SAVE_SIZE   156
#define INFO_NEEDED_FLAGS_DELIMITER 0xf1a5deed

static void *CataMemGrowthSaveData(void)
{
    return mainDerelictTextureSaveData();
}

#define CATA_SAVE_STATICINFO_DYNAMIC_FLAG 0x00040000

static bool32 SaveGameHasSerializedDynamicStaticInfo(udword flags)
{
    return ((flags & CATA_SAVE_STATICINFO_DYNAMIC_FLAG) != 0);
}

static void SaveGameSetSerializedDynamicStaticInfoFlag(udword *flags, bool32 isDynamic)
{
    *flags &= ~SOF_StaticInfoIsDynamicCata;
    if (isDynamic)
    {
        *flags |= CATA_SAVE_STATICINFO_DYNAMIC_FLAG;
    }
    else
    {
        *flags &= ~CATA_SAVE_STATICINFO_DYNAMIC_FLAG;
    }
}

static void SaveGameTranslateSerializedDynamicStaticInfoFlag(udword *flags)
{
    bool32 isDynamic = SaveGameHasSerializedDynamicStaticInfo(*flags);

    *flags &= ~CATA_SAVE_STATICINFO_DYNAMIC_FLAG;
    *flags &= ~SOF_StaticInfoIsDynamicCata;
    if (isDynamic)
    {
        *flags |= SOF_StaticInfoIsDynamicCata;
    }
}

static void SaveResourceStaticInfoChunk(const void *staticinfo, udword size)
{
    SaveChunk *chunk;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_STATICINFO, size, (void *)staticinfo);
    SaveThisChunk(chunk);
    memFree(chunk);
}

static ResourceStaticInfo *LoadResourceStaticInfoChunk(const ResourceStaticInfo *baseStaticInfo, udword size)
{
    ResourceStaticInfo *staticinfo;
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunk(chunk, BASIC_STRUCTURE|SAVE_STATICINFO, size);

    staticinfo = memAlloc(size, "ResourceStaticInfo", NonVolatile);
    memcpy(staticinfo, chunkContents(chunk), size);
    memFree(chunk);

    staticinfo->staticheader.LOD = baseStaticInfo->staticheader.LOD;
    staticinfo->staticheader.pMexData = baseStaticInfo->staticheader.pMexData;

    return staticinfo;
}

static bool32 SaveGameShipHasRuntimeDynamicStaticInfo(Ship *ship)
{
    ShipStaticInfo *realstaticinfo = GetShipStaticInfo(ship->shiptype, ship->shiprace);

    if (ship->flags & SOF_StaticInfoIsDynamicCata)
    {
        return TRUE;
    }

    return (ship->staticinfo != realstaticinfo);
}

bool32 PeekNextChunkHeader(SaveChunk *header)
{
    FILE *fp;
    long offset;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);
    offset = ftell(fp);
    if (offset < 0)
    {
        return FALSE;
    }
    if (fread(header, sizeof(*header), 1, fp) != 1)
    {
        fseek(fp, offset, SEEK_SET);
        return FALSE;
    }
    fseek(fp, offset, SEEK_SET);
    return TRUE;
}

static bool32 PeekNextInfoNumber(sdword *info)
{
    SaveChunk header;
    FILE *fp;
    long offset;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);
    offset = ftell(fp);
    if (offset < 0)
    {
        return FALSE;
    }
    if (fread(&header, sizeof(header), 1, fp) != 1
     || header.type != INFO_CHUNK
     || header.contentsSize != sizeof(InfoChunkContents)
     || fread(info, sizeof(*info), 1, fp) != 1)
    {
        fseek(fp, offset, SEEK_SET);
        return FALSE;
    }
    fseek(fp, offset, SEEK_SET);
    return TRUE;
}

static void SaveCataMemGrowthState(void)
{
    SaveStructureOfSize(CataMemGrowthSaveData(), CATA_MEMGROWTH_SAVE_SIZE);
}

typedef struct CataGlobalStateSaveData
{
    real32 rawTaskTimeElapsed;
    real32 gameTaskTimeElapsed;
    sdword SongNumber;
} CataGlobalStateSaveData;

static void SaveCataGlobalState(void)
{
    CataGlobalStateSaveData saveState;
    SaveChunk *chunk;

    saveState.rawTaskTimeElapsed = taskTimeElapsed;
    saveState.gameTaskTimeElapsed = taskTimeElapsedGame;
    saveState.SongNumber = SongNumber;

    chunk = CreateChunk(BASIC_STRUCTURE | SAVE_GLOBAL_STATE, sizeof(saveState), &saveState);
    SaveThisChunk(chunk);
    memFree(chunk);
}

static void LoadCataMemGrowthStateIfPresent(void)
{
    SaveChunk header;

    if (!PeekNextChunkHeader(&header))
    {
        return;
    }
    if (header.type != BASIC_STRUCTURE || header.contentsSize != CATA_MEMGROWTH_SAVE_SIZE)
    {
        return;
    }
    LoadStructureOfSizeToAddress(CataMemGrowthSaveData(), CATA_MEMGROWTH_SAVE_SIZE);
}

static void LoadCataGlobalState(void)
{
    SaveChunk header;
    SaveChunk *chunk;
    CataGlobalStateSaveData saveState;

    if (!PeekNextChunkHeader(&header))
    {
        return;
    }
    if (header.type != (BASIC_STRUCTURE | SAVE_GLOBAL_STATE) ||
        header.contentsSize != sizeof(saveState))
    {
        return;
    }

    chunk = LoadNextChunk();
    VerifyChunk(chunk, BASIC_STRUCTURE | SAVE_GLOBAL_STATE, sizeof(saveState));
    memcpy(&saveState, chunkContents(chunk), sizeof(saveState));
    memFree(chunk);

    taskTimeElapsed = saveState.rawTaskTimeElapsed;
    taskTimeElapsedGame = saveState.gameTaskTimeElapsed;
    SongNumber = saveState.SongNumber;
}

static void LoadAlignTopLevelSelections(void)
{
    SaveChunk header;
    char skippedString[256];

    if (!PeekNextChunkHeader(&header)
     || header.type != (VARIABLE_STRUCTURE | SAVESTRING))
    {
        return;
    }

    Load_StringToAddress(skippedString);
}

static void LoadRefreshShipStaticHealths(void)
{
    sdword i;

    for (i = 0; i < TOTAL_NUM_SHIPS + 1; i++)
    {
        ShipStaticInfo *staticInfo = &shipStaticInfos[i];

        if (!bitTest(staticInfo->staticheader.infoFlags, IF_InfoLoaded))
        {
            continue;
        }

        if (!singlePlayerGame || staticInfo->shiprace != RACE_Sect)
        {
            staticInfo->maxhealth = TreatAsReal32(staticInfo->unk10);
        }
        else
        {
            staticInfo->maxhealth = GameDifficultyPrimaryModifier * TreatAsReal32(staticInfo->unk10);
        }

        staticInfo->oneOverMaxHealth = 1.0f / staticInfo->maxhealth;
    }
}

// Fixed serialized prefix size (Ship header through end of serialized
// ShipSpecifics). Retail SaveShip/LoadShip do not include the live
// Beast/Leech padding or module-slot allocation floor here.

typedef struct
{
    udword state;
    udword unk04[32];
    GunStatic *gunStatic;
} SaveGameShipUpgradeSlotStateEntry;

typedef struct
{
    sdword count;
    SaveGameShipUpgradeSlotStateEntry entries[1];
} SaveGameShipUpgradeSlotStateBlock;

static_assert(sizeof(SaveGameShipUpgradeSlotStateEntry) == 0x88, "SaveGameShipUpgradeSlotStateEntry size mismatch");

static DockPointActivity *ShipCataDockPointActivity(Ship *ship)
{
    return ship->dockvars.dockPointActivity;
}

static void ShipSetCataDockPointActivity(Ship *ship, DockPointActivity *dockPointActivity)
{
    ship->dockvars.dockPointActivity = dockPointActivity;
}

static sdword ShipCataDockPointActivitySize(Ship *ship)
{
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    dbgAssert(dockStaticInfo != NULL);
    return offsetof(DockPointActivity, reserved) + sizeof(((DockPointActivity *)0)->reserved[0]) * dockStaticInfo->numDockPoints;
}

static ShipsInsideMe *ShipCataShipsInsideMe(Ship *ship)
{
    return ship->shipsInsideMe;
}

static void ShipSetCataShipsInsideMe(Ship *ship, ShipsInsideMe *shipsInsideMe)
{
    ship->shipsInsideMe = shipsInsideMe;
}

static SpaceObjSelection *ShipCataSelection22C(Ship *ship)
{
    return ship->selection22C;
}

static void ShipSetCataSelection22C(Ship *ship, SpaceObjSelection *selection)
{
    ship->selection22C = selection;
}

static madanim *ShipCataMadBindingsBody(Ship *ship)
{
    return ship->madBindings;
}

static void ShipSetCataMadBindingsBody(Ship *ship, madanim *madBindings)
{
    ship->madBindings = madBindings;
}

static GunInfo *ShipCataGunInfo(Ship *ship)
{
    return ship->cataclysmGunInfo;
}

static void ShipSetCataGunInfo(Ship *ship, GunInfo *gunInfo)
{
    ship->cataclysmGunInfo = gunInfo;
}

static GunInfo *ShipSaveGunInfo(Ship *ship)
{
    return ShipCataGunInfo(ship);
}

static void ShipRefreshRuntimeGunBindings(Ship *ship)
{
    GunInfo *gunInfo;
    GunStaticInfo *gunStaticInfo;
    sdword i;

    if (ship == NULL || ship->staticinfo == NULL)
    {
        return;
    }

    gunInfo = ShipSaveGunInfo(ship);
    gunStaticInfo = ship->staticinfo->gunStaticInfo;
    if (gunInfo == NULL || gunStaticInfo == NULL)
    {
        return;
    }

    dbgAssert(gunInfo->numGuns == gunStaticInfo->numGuns);
    for (i = 0; i < gunInfo->numGuns; i++)
    {
        Gun *gun = &gunInfo->guns[i];
        GunStatic *gunstatic = &gunStaticInfo->gunstatics[i];

        gun->gunstatic = gunstatic;
    }
}
static ShipsInsideMe *ShipSaveShipsInsideMe(Ship *ship)
{
    return ShipCataShipsInsideMe(ship);
}

static DockInfo *BuildDockInfoFromDockPointActivity(Ship *ship, DockPointActivity *dockPointActivity)
{
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    sdword numDockPoints;
    DockInfo *dockInfo;
    sdword i;

    if (dockPointActivity == NULL || dockStaticInfo == NULL)
    {
        return NULL;
    }

    numDockPoints = dockStaticInfo->numDockPoints;
    dockInfo = memAlloc(sizeofDockInfo(numDockPoints), "dockInfo", NonVolatile);
    dockInfo->busyness = 0;
    dockInfo->numDockPoints = numDockPoints;

    for (i = 0; i < numDockPoints; i++)
    {
        sdword thisDockBusy = dockPointActivity->reserved[i];
        dockInfo->dockpoints[i].dockstaticpoint = &dockStaticInfo->dockstaticpoints[i];
        dockInfo->dockpoints[i].thisDockBusy = thisDockBusy;
        dockInfo->busyness += thisDockBusy;
    }

    return dockInfo;
}

static DockInfo *BuildDefaultDockInfo(Ship *ship)
{
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    sdword numDockPoints;
    DockInfo *dockInfo;
    sdword i;

    if (dockStaticInfo == NULL)
    {
        return NULL;
    }

    numDockPoints = dockStaticInfo->numDockPoints;
    dockInfo = memAlloc(sizeofDockInfo(numDockPoints), "dockInfo", NonVolatile);
    dockInfo->busyness = 0;
    dockInfo->numDockPoints = numDockPoints;

    for (i = 0; i < numDockPoints; i++)
    {
        dockInfo->dockpoints[i].dockstaticpoint = &dockStaticInfo->dockstaticpoints[i];
        dockInfo->dockpoints[i].thisDockBusy = 0;
    }

    return dockInfo;
}

/*=============================================================================
    Functions:
=============================================================================*/

void SpaceObjRegistryInit()
{
    Node* objnode = universe.ShipList.head;
    while (objnode != NULL)
    {
        SpaceObj* spaceobj = (SpaceObj*)listGetStructOfNode(objnode);
        dmgStopEffect((Ship*)spaceobj, DMG_All);
        objnode = objnode->next;
    }
    growSelectInit(&SpaceObjRegistry);
}

void SpaceObjRegistryClose()
{
    growSelectClose(&SpaceObjRegistry);
}

sdword SpaceObjRegistryObjPresent(SpaceObj *obj)
{
    sdword i;
    sdword numObjs = SpaceObjRegistry.selection->numShips;

    for (i=0;i<numObjs;i++)
    {
        if (obj == (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i])
        {
            return i;
        }
    }

    return -1;
}

void SpaceObjRegistryRegister(SpaceObj *obj)
{
    sdword index = SpaceObjRegistryObjPresent(obj);
    if (index == -1)        // object not present, so add
    {
        growSelectAddShip(&SpaceObjRegistry,(Ship *)obj);
    }
}

void SpaceObjRegistryRegisterNoCheck(SpaceObj *obj)
{
    growSelectAddShip(&SpaceObjRegistry,(Ship *)obj);
}

static blob **SaveGameBlobRefForObj(SpaceObj *obj)
{
    if (obj == NULL)
    {
        return NULL;
    }

    switch (obj->objtype)
    {
        case OBJ_ShipType:
            return &((Ship *)obj)->collMyBlob;
        case OBJ_BulletType:
            return &((Bullet *)obj)->collMyBlob;
        case OBJ_AsteroidType:
            return &((Asteroid *)obj)->collMyBlob;
        case OBJ_NebulaType:
            return &((Nebula *)obj)->collMyBlob;
        case OBJ_DustType:
            return &((DustCloud *)obj)->collMyBlob;
        case OBJ_DerelictType:
            return &((Derelict *)obj)->collMyBlob;
        case OBJ_MissileType:
            return &((Missile *)obj)->collMyBlob;
        default:
            return NULL;
    }
}

static void SaveGameClearBlobRefForObj(SpaceObj *obj)
{
    blob **collMyBlob;

    if (obj == NULL)
    {
        return;
    }

    collMyBlob = SaveGameBlobRefForObj(obj);
    if (collMyBlob != NULL)
    {
        *collMyBlob = NULL;
    }
    bitClear(obj->flags, SOF_DontCreateBlob);
}

static void SaveGameClearBlobRefsInsideMe(ShipsInsideMe *shipsInsideMe)
{
    Node *insideNode;

    if (shipsInsideMe == NULL)
    {
        return;
    }

    for (insideNode = shipsInsideMe->insideList.head; insideNode != NULL; insideNode = insideNode->next)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(insideNode);
        Ship *ship = insideShip->ship;

        SaveGameClearBlobRefForObj((SpaceObj *)ship);
        SaveGameClearBlobRefsInsideMe(ShipCataShipsInsideMe(ship));
    }
}

void SaveGameClearRegisteredBlobRefs(void)
{
    Node *node;
    sdword i;

    for (i = 0; i < SpaceObjRegistry.selection->numShips; i++)
    {
        SpaceObj *obj = (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i];

        SaveGameClearBlobRefForObj(obj);
    }

    for (node = universe.SpaceObjList.head; node != NULL; node = node->next)
    {
        SpaceObj *obj = (SpaceObj *)listGetStructOfNode(node);

        SaveGameClearBlobRefForObj(obj);
        if (obj != NULL && obj->objtype == OBJ_ShipType)
        {
            SaveGameClearBlobRefsInsideMe(ShipCataShipsInsideMe((Ship *)obj));
        }
    }

    for (node = universe.MinorSpaceObjList.head; node != NULL; node = node->next)
    {
        SaveGameClearBlobRefForObj((SpaceObj *)listGetStructOfNode(node));
    }

    for (node = singlePlayerGameInfo.ShipsInHyperspace.head; node != NULL; node = node->next)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
        Ship *ship = insideShip->ship;

        SaveGameClearBlobRefForObj((SpaceObj *)ship);
        SaveGameClearBlobRefsInsideMe(ShipCataShipsInsideMe(ship));
    }
}

sdword SpaceObjRegistryGetID(SpaceObj *obj)
{
    if (obj != NULL)
    {
        return SpaceObjRegistryObjPresent(obj);
    }
    else
    {
        return -1;
    }
}

SpaceObj *SpaceObjRegistryGetObj(sdword ID)
{
    if (ID == -1)
    {
        return NULL;
    }

    if (ID < 0 || ID >= SpaceObjRegistry.selection->numShips)
    {
        dbgMessagef(
            "SpaceObjRegistryGetObj: nulling out-of-range id=%d count=%d ordinal=%d offset=0x%x",
            ID,
            SpaceObjRegistry.selection->numShips,
            loadSpaceObjOrdinal,
            lastLoadChunkOffset);
        return NULL;
    }

    return (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[ID];
}

Ship *SpaceObjRegistryGetShip(sdword ID)
{
    if (ID == -1)
    {
        return NULL;
    }

    return SpaceObjRegistry.selection->ShipPtr[ID];
}

Resource *SpaceObjRegistryGetResource(sdword ID)
{
    Resource *resource;

    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);
    resource = (Resource *)SpaceObjRegistry.selection->ShipPtr[ID];
    dbgAssert(resource->objtype >= OBJ_AsteroidType && resource->objtype <= OBJ_DustType);
    return resource;
}

Bullet *SpaceObjRegistryGetBullet(sdword ID)
{
    Bullet *bullet;

    if (ID == -1)
    {
        return NULL;
    }

    dbgAssert(ID < SpaceObjRegistry.selection->numShips);
    bullet = (Bullet *)SpaceObjRegistry.selection->ShipPtr[ID];
    dbgAssert(bullet->objtype == OBJ_BulletType);
    return bullet;
}

TargetPtr SpaceObjRegistryGetTarget(sdword ID)
{
    TargetPtr target;

    if (ID == -1)
    {
        return NULL;
    }

    if (ID < 0 || ID >= SpaceObjRegistry.selection->numShips)
    {
        dbgMessagef(
            "SpaceObjRegistryGetTarget: nulling out-of-range id=%d count=%d ordinal=%d offset=0x%x",
            ID,
            SpaceObjRegistry.selection->numShips,
            loadSpaceObjOrdinal,
            lastLoadChunkOffset);
        return NULL;
    }
    target = (TargetPtr)SpaceObjRegistry.selection->ShipPtr[ID];
    if (target == NULL || !(target->flags & SOF_Targetable))
    {
        dbgMessagef(
            "SpaceObjRegistryGetTarget: nulling bad lookup id=%d ordinal=%d offset=0x%x ptr=0x%x flags=0x%x",
            ID,
            loadSpaceObjOrdinal,
            lastLoadChunkOffset,
            target,
            target ? target->flags : 0xffffffff);
        return NULL;
    }
    return target;
}

void BlobRegistryInit()
{
    growSelectInit(&BlobRegistry);
}

void BlobRegistryClose()
{
    growSelectClose(&BlobRegistry);
}

sdword BlobRegistryBlobPresent(blob *tblob)
{
    sdword i;
    sdword numObjs = BlobRegistry.selection->numShips;

    for (i=0;i<numObjs;i++)
    {
        if (tblob == (blob *)BlobRegistry.selection->ShipPtr[i])
        {
            return i;
        }
    }

    return -1;
}

void BlobRegistryRegister(blob *tblob)
{
    dbgAssert(BlobRegistryBlobPresent(tblob) == -1);
    growSelectAddShip(&BlobRegistry,(Ship *)tblob);
}

sdword BlobRegistryGetID(blob *tblob)
{
    return BlobRegistryBlobPresent(tblob);
}

sdword BlobRegistryGetIDNoBlobOkay(blob *tblob)
{
    if (tblob == NULL)
    {
        return -1;
    }
    else
    {
        sdword ID = BlobRegistryBlobPresent(tblob);
        return ID;
    }
}

blob *BlobRegistryGetBlob(sdword ID)
{
    if (ID == -1)
    {
        return NULL;
    }
    else
    {
        if (ID < 0 || ID >= BlobRegistry.selection->numShips)
        {
            dbgMessagef(
                "BlobRegistryGetBlob: nulling out-of-range id=%d count=%d ordinal=%d offset=0x%x",
                ID,
                BlobRegistry.selection->numShips,
                loadSpaceObjOrdinal,
                lastLoadChunkOffset);
            return NULL;
        }
        return (blob *)BlobRegistry.selection->ShipPtr[ID];
    }
}

/*-----------------------------------------------------------------------------
    Name        : SaveThisChunk
    Description : saves thischunk
    Inputs      : thischunk
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void SaveThisChunk(SaveChunk *thischunk)
{
    FILE *fp;
    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    if (fwrite(thischunk, sizeofSaveChunk(thischunk->contentsSize), 1, fp) != 1)
    {
        savefilestatus = 1;
    }
}

SaveChunk *CreateChunk(TypeOfSaveChunk type,sdword contentsSize,void *contents)
{
    SaveChunk *chunk = memAlloc(sizeofSaveChunk(contentsSize),"savechunk",Pyrophoric);

    chunk->type = type;
    chunk->contentsSize = contentsSize;

    dbgAssert(contentsSize >= 0);
    if ((contentsSize > 0) && (contents != NULL))
    {
        memcpy(chunkContents(chunk),contents,contentsSize);
    }

    return chunk;
}

SaveChunk *LoadNextChunk()
{
    SaveChunk readchunk;
    SaveChunk *returnchunk;
    FILE *fp;

    fp = fileStream(savefile);
    fread(&readchunk,sizeof(SaveChunk),1,fp);

    returnchunk = memAlloc(sizeofSaveChunk(readchunk.contentsSize),"readchunk",Pyrophoric);

    memcpy(returnchunk,&readchunk,sizeof(SaveChunk));

    if (readchunk.contentsSize > 0)
    {
        fread(((ubyte *)returnchunk) + sizeof(SaveChunk),readchunk.contentsSize,1,fp);
    }

    return returnchunk;
}

SaveChunk *LoadNextChunkSafe()
{
    SaveChunk readchunk;
    SaveChunk *returnchunk;
    FILE *fp;

    fp = fileStream(savefile);
    if (fread(&readchunk,sizeof(SaveChunk),1,fp) == 0)
    {
        return NULL;
    }

    if (readchunk.contentsSize < 0)
    {
        return NULL;
    }

    returnchunk = memAlloc(sizeofSaveChunk(readchunk.contentsSize),"readchunk",Pyrophoric);

    memcpy(returnchunk,&readchunk,sizeof(SaveChunk));

    if (readchunk.contentsSize > 0)
    {
        if (fread(((ubyte *)returnchunk) + sizeof(SaveChunk),readchunk.contentsSize,1,fp) == 0)
        {
            memFree(returnchunk);
            return NULL;
        }
    }

    return returnchunk;
}

void SaveInfoNumber(sdword info)
{
    InfoChunkContents infocontents;
    SaveChunk *chunk;

    infocontents.info = info;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    memFree(chunk);
}

sdword LoadInfoNumber()
{
    SaveChunk *chunk;
    sdword num;

    chunk = LoadNextChunk();
    num = ((InfoChunkContents *)chunkContents(chunk))->info;
    memFree(chunk);

    return num;
}

bool32 LoadInfoNumberOptional(sdword *info)
{
    SaveChunk *chunk;

    chunk = LoadNextChunkSafe();
    if (chunk == NULL)
    {
        return FALSE;
    }

    *info = ((InfoChunkContents *)chunkContents(chunk))->info;
    memFree(chunk);

    return TRUE;
}

void SaveBackground(void)
{
    real32 theta = btgGetTheta();
    real32 phi = btgGetPhi();

    SaveInfoNumber(TreatAsUdword(theta));
    SaveInfoNumber(TreatAsUdword(phi));
    Save_String(btgLastBackground);
}

void LoadBackground(void)
{
    sdword datatheta = LoadInfoNumber();
    sdword dataphi = LoadInfoNumber();

    btgSetTheta(TreatAsReal32(datatheta));
    btgSetPhi(TreatAsReal32(dataphi));
    Load_StringToAddress(btgLastBackground);

    btgLoadTextures();
}

void SaveLighting(void)
{
    Save_String(lightCurrentLighting);
}

void LoadLighting(void)
{
    Load_StringToAddress(lightCurrentLighting);

    if (strlen(lightCurrentLighting) > 1)
    {
        lightParseHSF(lightCurrentLighting);
    }
    else
    {
        lightDefaultLightSet();
    }
}

void SaveVersionInfo(void)
{
    sdword version = SAVE_VERSION_NUMBER;
    FILE *fp;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    if (fwrite(&version, sizeof(sdword), 1, fp) != 1)
    {
        savefilestatus = 1;
    }
}

sdword LoadVersionInfo(void)
{
    sdword num;
    sdword version;
    FILE *fp;

    dbgAssert(!fileUsingBigfile(savefile));
    fp = fileStream(savefile);

    num = fread(&version,sizeof(sdword),1,fp);
    if (num == 0)
    {
        return VERIFYSAVEFILE_ERROROPENING;
    }

    if (version != SAVE_VERSION_NUMBER)
    {
        return VERIFYSAVEFILE_BADVERSION;
    }

    return VERIFYSAVEFILE_OK;
}

void SavePreGameInfo(void)
{
    sdword i;

    SaveInfoNumber(singlePlayerGame);
    SaveInfoNumber(musicEventCurrentTrack());
    SaveInfoNumber(universe.numPlayers);
    SaveStructureOfSize(&playerNames[0][0],MAX_MULTIPLAYER_PLAYERS*MAX_PERSONAL_NAME_LEN);
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        SaveInfoNumber(universe.players[i].race);
        SaveInfoNumber(teColorSchemes[i].textureColor.base);
        SaveInfoNumber(teColorSchemes[i].textureColor.detail);
    }

    if (singlePlayerGame)
    {
        SaveInfoNumber(singlePlayerGameInfo.currentMission);
        SaveInfoNumber(TreatAsUdword(GameDifficultyPrimaryModifier));
    }
    else
    {
        SaveStructureOfSize(&tpGameCreated, CATA_SAVE_CAPTAINGAMEINFO_SERIALIZED_SIZE);
    }

    if (!(tutorial==TUTORIAL_ONLY))
    {
        universeSaveEverythingNeeded();
    }
}

void LoadPreGameInfo(void)
{
    sdword i;
    sdword loadedModifier;
    sdword sp;

    sp = LoadInfoNumber();
    if (sp)
    {
        dbgAssert(singlePlayerGame);
    }
    else
    {
        dbgAssert(!singlePlayerGame);
    }
    
    SongNumber = LoadInfoNumber();
    numPlayers = (uword)LoadInfoNumber();
    LoadStructureOfSizeToAddress(&playerNames[0][0],MAX_MULTIPLAYER_PLAYERS*MAX_PERSONAL_NAME_LEN);
    for (i=0;i<MAX_MULTIPLAYER_PLAYERS;i++)
    {
        universe.players[i].race = LoadInfoNumber();
        teColorSchemes[i].textureColor.base = LoadInfoNumber();
        teColorSchemes[i].textureColor.detail = LoadInfoNumber();
    }

    if (sp)
    {
        singlePlayerGameInfo.currentMission = LoadInfoNumber();
        loadedModifier = LoadInfoNumber();
        GameDifficultyPrimaryModifier = TreatAsReal32(loadedModifier);
    }
    else
    {
        LoadStructureOfSizeToAddress(&tpGameCreated, CATA_SAVE_CAPTAINGAMEINFO_SERIALIZED_SIZE);
    }

    if (!(tutorial==TUTORIAL_ONLY))
    {
        universeLoadEverythingNeeded();
    }

}

/*-----------------------------------------------------------------------------
    Name        : SaveGame
    Description :
    Inputs      :
    Outputs     :
    Return      : TRUE on success
----------------------------------------------------------------------------*/
bool32 SaveGame(char *filename)
{
    sdword i;

    savefile = fileOpen(filename,FF_WriteMode|FF_ReturnNULLOnFail);
    if (savefile == NULL)
    {
        return FALSE;
    }
    savefilestatus = 0;

    SaveVersionInfo();
    SavePreGameInfo();
    SaveCataMemGrowthState();

    SpaceObjRegistryInit();
    BlobRegistryInit();

    RegisterAllSpaceObjs();
    RegisterAllBlobs();
    SaveAllSpaceObjs();
    SaveAllBlobs();
    SaveCataGlobalState();
    SaveUniverse();
    SaveIDToPtrTable(&ShipIDToPtr);
    SaveIDToPtrTable(&ResourceIDToPtr);
    SaveIDToPtrTable(&DerelictIDToPtr);
    SaveIDToPtrTable(&MissileIDToPtr);
    nebSave_Nebula();
    SaveConsMgr();
    aiplayerSave();

    SaveSinglePlayerGame();

    shipModuleSlotSave();
    ioOverlayElapsedTimeSave();

    if (singlePlayerGame)
    {
        kasSave();
        objectiveSave();
        tmSave();
        pingSave();
        Save_String(CurrentLevelName);
    }

    if( tutorial==TUTORIAL_ONLY )
    {
        tutSaveTutorialGame();
    }

    SaveMaxSelection(&selSelected);
    for (i=0;i<SEL_NumberHotKeyGroups;i++)
    {
        SaveMaxSelection(&selHotKeyGroup[i]);
    }

    SaveBackground();
    SaveLighting();

    smSave();
    ranSave();

    SaveInfoNumber(actorFlagsEnabled);
    SaveInfoNumber(smGhostMode);

    SaveConsMgrDetermOptional();                // added for V1.04 patch

    fileClose(savefile);
    savefile = 0;

    if (savefilestatus)
    {
        fileDelete(filename);
        savefilestatus = 0;
        return FALSE;
    }
    else
    {
        return TRUE;        // save successful
    }
}

/*-----------------------------------------------------------------------------
    Name        : VerifySaveFile
    Description : verifies filename is valid save game file
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword VerifySaveFile(char *filename)
{
    sdword verify;

    savefile = fileOpen(filename,FF_ReturnNULLOnFail);
    if (savefile == NULL)
    {
        return VERIFYSAVEFILE_ERROROPENING;
    }
    verify = LoadVersionInfo();

    if (savefile)
    {
        fileClose(savefile);
        savefile = 0;
    }

    return verify;
}

sdword SaveFileGetSinglePlayerFlag(char *filename, bool32 *singlePlayer)
{
    sdword verify;
    sdword sp;

    dbgAssert(singlePlayer != NULL);
    *singlePlayer = TRUE;

    savefile = fileOpen(filename, FF_ReturnNULLOnFail);
    if (savefile == NULL)
    {
        return VERIFYSAVEFILE_ERROROPENING;
    }

    verify = LoadVersionInfo();
    if (verify == VERIFYSAVEFILE_OK)
    {
        sp = LoadInfoNumber();
        *singlePlayer = (sp != 0);
    }

    if (savefile)
    {
        fileClose(savefile);
        savefile = 0;
    }

    return verify;
}

/*-----------------------------------------------------------------------------
    Name        : PreLoadGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PreLoadGame(char *filename)
{
    sdword verify;

    savefile = fileOpen(filename,0);
    verify = LoadVersionInfo();
    if (verify != VERIFYSAVEFILE_OK)
    {
        dbgFatalf(DBG_Loc,"Error %d loading game %s",verify,filename);
        return;
    }
    LoadPreGameInfo();
}

/*-----------------------------------------------------------------------------
    Name        : LoadGame
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void LoadGame(char *filename)
{
    sdword i;

    dbgAssert(savefile);


    SpaceObjRegistryInit();
    BlobRegistryInit();
    LoadCataMemGrowthStateIfPresent();
    mainLoadDerelictTexture();

    LoadAllSpaceObjs();
    LoadAllBlobs();
    LoadCataGlobalState();
    LoadUniverse();
    LoadIDToPtrTable(&ShipIDToPtr);
    LoadIDToPtrTable(&ResourceIDToPtr);
    LoadIDToPtrTable(&DerelictIDToPtr);
    LoadIDToPtrTable(&MissileIDToPtr);
    nebLoad_Nebula();
    LoadConsMgr();
    aiplayerLoad();

    LoadSinglePlayerGame();

    shipModuleSlotLoad();
    ioOverlayElapsedTimeLoad();

    if (singlePlayerGame)
    {
        kasLoad();
        objectiveLoad();
        tmLoad();
        pingLoad();
        Load_StringToAddress(&CurrentLevelName[0]);
    }

    if(tutorial==TUTORIAL_ONLY)
    {
        tutLoadTutorialGame();
    }

    LoadMaxSelectionAndFix(&selSelected);
    for (i=0;i<SEL_NumberHotKeyGroups;i++)
    {
        LoadMaxSelectionAndFix(&selHotKeyGroup[i]);
    }

    LoadBackground();
    LoadLighting();

    smLoad();
    ranLoad();

    LoadInfoNumberOptional(&actorFlagsEnabled);
    LoadInfoNumberOptional(&smGhostMode);

    LoadConsMgrDetermOptional();            // optional, won't crash if not there, added for V1.04 patch

    FixAllSpaceObjs();
    FixAllBlobs();
    FixShipNamePointers();
    listInit(&universe.MinorSpaceObjList);

    if (singlePlayerGame)
    {
        spPostLoadAutosaveSave(filename);
    }

    growSelectClose(&SpaceObjRegistry);
    growSelectClose(&BlobRegistry);
    LoadRefreshShipStaticHealths();

    fileClose(savefile);
    savefile = 0;
}

/*=============================================================================
    Actual Save routines for various key data structures:
=============================================================================*/

void RegisterAllBlobs(void)
{
    Node *blobnode = universe.collBlobList.head;
    blob *thisBlob;

    // for each blob, check all collisions within that blob
    while (blobnode != NULL)
    {
        thisBlob = (blob *)listGetStructOfNode(blobnode);

        BlobRegistryRegister(thisBlob);

        blobnode = blobnode->next;
    }
}

void RegisterAllSpaceObjsInsideMe(ShipsInsideMe *shipsInsideMe)
{
    Node *insidenode = shipsInsideMe->insideList.head;
    InsideShip *insideship;
    Ship *ship;

    while (insidenode != NULL)
    {
        insideship = (InsideShip *)listGetStructOfNode(insidenode);
        dbgAssert(insideship->ship->objtype == OBJ_ShipType);
        ship = insideship->ship;
        if ((ship->flags & SOF_Dead) == 0)
        {
            ShipsInsideMe *nestedShipsInsideMe;

            SpaceObjRegistryRegister((SpaceObj *)ship);
            nestedShipsInsideMe = ship->shipsInsideMe;
            if (nestedShipsInsideMe != NULL)
            {
                RegisterAllSpaceObjsInsideMe(nestedShipsInsideMe);
            }
        }

        insidenode = insidenode->next;
    }
}

void RegisterAllSpaceObjs(void)
{
    Node *node = universe.SpaceObjList.head;
    SpaceObj *obj;

    while (node != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);

        if (obj->objtype == OBJ_EffectType)
        {
            goto nextnode;
        }
        if (obj->flags & SOF_Dead)
        {
            if (obj->objtype == OBJ_ShipType)
            {
                univRemoveShipReferences((Ship *)obj);
            }
            goto nextnode;
        }

        SpaceObjRegistryRegister(obj);
        if (obj->objtype == OBJ_ShipType)
        {
            Ship *ship = (Ship *)obj;
            ShipsInsideMe *shipsInsideMe = ((Ship *)obj)->shipsInsideMe;

            if (shipsInsideMe != NULL)
            {
                RegisterAllSpaceObjsInsideMe(shipsInsideMe);
            }

            if ((ship->flags2 & BIT3) != 0)
            {
                Effect *moduleSlotEffect = (Effect *)shipModuleSlotRuntime(ship)->activeEffect;

                if (moduleSlotEffect != NULL)
                {
                    SpaceObjRegistryRegister((SpaceObj *)moduleSlotEffect);
                }
            }
        }

nextnode:
        node = node->next;
    }

    node = universe.MinorSpaceObjList.head;
    while (node != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);

        if ((obj->flags & SOF_Dead) == 0)
        {
            SpaceObjRegistryRegister(obj);
        }

        node = node->next;
    }

    node = singlePlayerGameInfo.ShipsInHyperspace.head;
    while (node != NULL)
    {
        InsideShip *insideShip = (InsideShip *)listGetStructOfNode(node);
        Ship *ship = insideShip->ship;

        if ((ship->flags & SOF_Dead) == 0)
        {
            Node *midlevelNode;
            LinkedList *midlevelHyperspaceIn;

            dbgAssert(ship->objtype == OBJ_ShipType);
            dbgAssert((ship->flags & SOF_Dead) == 0);
            SpaceObjRegistryRegister((SpaceObj *)ship);
            if (ship->shipSinglePlayerGameInfo != NULL)
            {
                midlevelHyperspaceIn = (LinkedList *)ship->shipSinglePlayerGameInfo;
                for (midlevelNode = midlevelHyperspaceIn->head; midlevelNode != NULL; midlevelNode = midlevelNode->next)
                {
                    InsideShip *midlevelInsideShip = (InsideShip *)listGetStructOfNode(midlevelNode);
                    Ship *midlevelShip = midlevelInsideShip->ship;

                    if ((midlevelShip->flags & SOF_Dead) == 0)
                    {
                        SpaceObjRegistryRegister((SpaceObj *)midlevelShip);
                        if (midlevelShip->shipsInsideMe != NULL)
                        {
                            RegisterAllSpaceObjsInsideMe(midlevelShip->shipsInsideMe);
                        }
                    }
                }
            }
        }

        node = node->next;
    }
}

void SaveAllBlobs()
{
    sdword numBlobs = BlobRegistry.selection->numShips;
    sdword i;
    InfoChunkContents infocontents;
    SaveChunk *chunk;

    infocontents.info = numBlobs;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));

    for (i=0;i<numBlobs;i++)
    {
        SaveBlob((blob *)BlobRegistry.selection->ShipPtr[i]);
    }

    memFree(chunk);
}

void LoadAllBlobs()
{
    SaveChunk *infochunk;
    sdword numBlobs;
    blob *tblob;
    sdword i;

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    numBlobs = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    for (i=0;i<numBlobs;i++)
    {
        tblob = LoadBlob();
        BlobRegistryRegister(tblob);
    }
}

void FixAllBlobs()
{
    sdword numBlobs = BlobRegistry.selection->numShips;
    sdword i;

    for (i=0;i<numBlobs;i++)
    {
        FixBlob((blob *)BlobRegistry.selection->ShipPtr[i]);
    }
}

void SaveBlob(blob *tblob)
{
    blob *sblob;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_BLOB,sizeof(blob),tblob);

    sblob = chunkContents(chunk);

    sblob->subBlobs.num = BIT31;        // flag subblobs as nonexistent

    SaveThisChunk(chunk);

    dbgAssert(bitTest(sblob->flags, BTF_FreeThisBlob));
    dbgAssert(bitTest(sblob->flags, BTF_FreeBlobObjects));

    if (sblob->blobShips != NULL) SaveSelection((SpaceObjSelection *)sblob->blobShips);
    if (sblob->blobSmallShips != NULL) SaveSelection((SpaceObjSelection *)sblob->blobSmallShips);
    if (sblob->blobBigShips != NULL) SaveSelection((SpaceObjSelection *)sblob->blobBigShips);
    if (sblob->blobSmallTargets != NULL) SaveSelection((SpaceObjSelection *)sblob->blobSmallTargets);
    if (sblob->blobBigTargets != NULL) SaveSelection((SpaceObjSelection *)sblob->blobBigTargets);
    if (sblob->blobResources != NULL) SaveSelection((SpaceObjSelection *)sblob->blobResources);
    if (sblob->blobDerelicts != NULL) SaveSelection((SpaceObjSelection *)sblob->blobDerelicts);
    if (sblob->blobBullets != NULL) SaveSelection((SpaceObjSelection *)sblob->blobBullets);
    if (sblob->blobMissileMissiles != NULL) SaveSelection((SpaceObjSelection *)sblob->blobMissileMissiles);
    if (sblob->blobMissileMines != NULL) SaveSelection((SpaceObjSelection *)sblob->blobMissileMines);

    if (sblob->blobObjects != NULL) SaveSelection((SpaceObjSelection *)sblob->blobObjects);

    memFree(chunk);
}

blob *LoadBlob(void)
{
    SaveChunk *chunk;
    blob *lblob;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_BLOB,sizeof(blob));

    lblob = memAlloc(sizeof(blob),"loadblob",0);
    memcpy(lblob,chunkContents(chunk),sizeof(blob));

    lblob->subBlobs.num = BIT31;            // flag subblobs as nonexistent

    memFree(chunk);

    if (lblob->blobShips != NULL) { lblob->blobShips = (SelectCommand *)LoadSelection(); lblob->maxNumShips = lblob->blobShips->numShips; }
    if (lblob->blobSmallShips != NULL) { lblob->blobSmallShips = (SelectCommand *)LoadSelection(); lblob->maxNumSmallShips = lblob->blobSmallShips->numShips; }
    if (lblob->blobBigShips != NULL) { lblob->blobBigShips = (SelectCommand *)LoadSelection(); lblob->maxNumBigShips = lblob->blobBigShips->numShips; }
    if (lblob->blobSmallTargets != NULL) { lblob->blobSmallTargets = (SelectAnyCommand *)LoadSelection(); lblob->maxNumSmallTargets = lblob->blobSmallTargets->numTargets; };
    if (lblob->blobBigTargets != NULL) { lblob->blobBigTargets = (SelectAnyCommand *)LoadSelection(); lblob->maxNumBigTargets = lblob->blobBigTargets->numTargets; }
    if (lblob->blobResources != NULL) { lblob->blobResources = (ResourceSelection *)LoadSelection(); lblob->maxNumResources = lblob->blobResources->numResources; }
    if (lblob->blobDerelicts != NULL) { lblob->blobDerelicts = (DerelictSelection *)LoadSelection(); lblob->maxNumDerelicts = lblob->blobDerelicts->numDerelicts; }
    if (lblob->blobBullets != NULL) { lblob->blobBullets = (BulletSelection *)LoadSelection(); lblob->maxNumBullets = lblob->blobBullets->numBullets; }
    if (lblob->blobMissileMissiles != NULL) { lblob->blobMissileMissiles = (MissileSelection *)LoadSelection(); lblob->maxNumMissileMissiles = lblob->blobMissileMissiles->numMissiles; }
    if (lblob->blobMissileMines != NULL) { lblob->blobMissileMines = (MissileSelection *)LoadSelection(); lblob->maxNumMissileMines = lblob->blobMissileMines->numMissiles; }

    if (lblob->blobObjects != NULL) { lblob->blobObjects = LoadSelection(); lblob->maxNumObjects = lblob->blobObjects->numSpaceObjs; }

    return lblob;
}

void FixBlob(blob *tblob)
{
    if (tblob->blobShips != NULL) FixSelection((SpaceObjSelection *)tblob->blobShips);
    if (tblob->blobSmallShips != NULL) FixSelection((SpaceObjSelection *)tblob->blobSmallShips);
    if (tblob->blobBigShips != NULL) FixSelection((SpaceObjSelection *)tblob->blobBigShips);
    if (tblob->blobSmallTargets != NULL) FixSelection((SpaceObjSelection *)tblob->blobSmallTargets);
    if (tblob->blobBigTargets != NULL) FixSelection((SpaceObjSelection *)tblob->blobBigTargets);
    if (tblob->blobResources != NULL) FixSelection((SpaceObjSelection *)tblob->blobResources);
    if (tblob->blobDerelicts != NULL) FixSelection((SpaceObjSelection *)tblob->blobDerelicts);
    if (tblob->blobBullets != NULL) FixSelection((SpaceObjSelection *)tblob->blobBullets);
    if (tblob->blobMissileMissiles != NULL) FixSelection((SpaceObjSelection *)tblob->blobMissileMissiles);
    if (tblob->blobMissileMines != NULL) FixSelection((SpaceObjSelection *)tblob->blobMissileMines);

    if (tblob->blobObjects != NULL) FixSelection((SpaceObjSelection *)tblob->blobObjects);
}

void SaveAllSpaceObjs()
{
    sdword numObjs = SpaceObjRegistry.selection->numShips;
    sdword i;
    InfoChunkContents infocontents;
    SaveChunk *chunk;

    infocontents.info = numObjs;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    for (i=0;i<numObjs;i++)
    {
        SaveSpaceObj((SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i]);
    }

    memFree(chunk);
}

void LoadAllSpaceObjs()
{
    SaveChunk *infochunk;
    sdword numObjs;
    SpaceObj *obj;
    sdword i;

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    numObjs = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);
    loadSpaceObjOrdinal = -1;

    for (i=0;i<numObjs;i++)
    {
        obj = LoadSpaceObj();
        SpaceObjRegistryRegisterNoCheck(obj);
    }
}

void FixAllSpaceObjs()
{
    sdword numObjs = SpaceObjRegistry.selection->numShips;
    sdword i;

    for (i=0;i<numObjs;i++)
    {
        SpaceObj *obj = (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[i];
        switch (obj->objtype)
        {
            case OBJ_ShipType:
                FixShip((Ship *)obj);
                break;

            case OBJ_BulletType:
                FixBullet((Bullet *)obj);
                break;

            case OBJ_AsteroidType:
                FixAsteroid((Asteroid *)obj);
                break;

            case OBJ_NebulaType:
                FixNebula((Nebula *)obj);
                break;

            case OBJ_GasType:
                dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
                return;

            case OBJ_DustType:
                FixDustCloud((DustCloud *)obj);
                break;

            case OBJ_DerelictType:
                FixDerelict((Derelict *)obj);
                break;

            case OBJ_MissileType:
                FixMissile((Missile *)obj);
                break;

            default:
                break;
        }

        if (obj->flags & SOF_Targetable)
        {
            ((SpaceObjRotImpTarg *)obj)->flashtimer = 0.0f;
        }
    }
}

void SaveAttackTargets(AttackTargets *multipleAttackTargets)
{
    AttackTargets *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_ATTACKTARGETS,sizeofAttackTargets(multipleAttackTargets->numAttackTargets),multipleAttackTargets);
    savecontents = chunkContents(chunk);

    for (i=0;i<multipleAttackTargets->numAttackTargets;i++)
    {
        savecontents->TargetPtr[i] = (sdword)SpaceObjRegistryGetID((SpaceObj *)multipleAttackTargets->TargetPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

AttackTargets *LoadAttackTargets()
{
    AttackTargets *loadedAttackTargets;
    AttackTargets *originalAttackTargets;
    SaveChunk *chunk;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_ATTACKTARGETS);

    originalAttackTargets = (AttackTargets *)chunkContents(chunk);
    size = sizeofAttackTargets(originalAttackTargets->numAttackTargets);
    loadedAttackTargets = memAlloc(size,"AttackTargets",0);
    memcpy(loadedAttackTargets,originalAttackTargets,size);

    memFree(chunk);

    return loadedAttackTargets;
}

void FixAttackTargets(AttackTargets *multipleAttackTargets)
{
    sdword i;

    for (i=0;i<multipleAttackTargets->numAttackTargets;i++)
    {
        multipleAttackTargets->TargetPtr[i] = SpaceObjRegistryGetTarget((sdword)multipleAttackTargets->TargetPtr[i]);
    }
}

void SaveGunInfo(Ship *ship)
{
    GunInfo *originalGunInfo = ShipSaveGunInfo(ship);
    GunInfo *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_GUNINFO,sizeofGunInfo(originalGunInfo->numGuns),originalGunInfo);
    savecontents = chunkContents(chunk);

    for (i=0;i<originalGunInfo->numGuns;i++)
    {
        savecontents->guns[i].gunstatic = NULL;
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadGunInfo(Ship *ship)
{
    GunInfo *loadedGunInfo;
    GunInfo *originalGunInfo;
    SaveChunk *chunk;
    sdword size;
    sdword i;
    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_GUNINFO);

    originalGunInfo = (GunInfo *)chunkContents(chunk);
    size = sizeofGunInfo(originalGunInfo->numGuns);
    dbgAssert(size == chunk->contentsSize);
    loadedGunInfo = memAlloc(size,"gunInfo",NonVolatile);
    memcpy(loadedGunInfo,originalGunInfo,size);

    memFree(chunk);

    ShipSetCataGunInfo(ship, loadedGunInfo);
    ShipRefreshRuntimeGunBindings(ship);

    for (i=0;i<loadedGunInfo->numGuns;i++)
    {
        Gun *gun = &loadedGunInfo->guns[i];

        switch (gun->gunstatic->guntype)
        {
            case GUN_MissileLauncher:
            case GUN_MineLauncher:
            case GUN_Gimble:
            case GUN_Fixed:
                gun->gunheading = gun->gunstatic->gunnormal;
                break;

            case GUN_NewGimble:
                univResetNewGimbleGun(gun);
                break;
        }

        gun->lasttimefired = 0.0f;
        gun->burstStartTime = 0.0f;
        gun->useBurstFireTime = 0.0f;
        gun->useBurstWaitTime = 0.0f;
        gun->burstState = 0;
        gun->gimblehandle = SOUND_NOTINITED;
    }
}

void SaveDockInfo(Ship *ship)
{
    DockPointActivity *dockPointActivity = ShipCataDockPointActivity(ship);
    SaveChunk *chunk;
    sdword size = ShipCataDockPointActivitySize(ship);
    dbgAssert(dockPointActivity != NULL);

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_DOCKINFO,size,dockPointActivity);

    SaveThisChunk(chunk);
    memFree(chunk);
}

static void LoadDockInfoChunk(Ship *ship, SaveChunk *chunk)
{
    DockPointActivity *loadedDockPointActivity;
    sdword size;

    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_DOCKINFO);

    if (ship->staticinfo->dockStaticInfo == NULL)
    {
        dbgMessagef("LoadDockInfo: dropping dock activity ordinal=%d offset=0x%lx type=%d race=%d size=%d",
                    loadSpaceObjOrdinal,
                    lastLoadChunkOffset,
                    ship->shiptype,
                    ship->shiprace,
                    chunk->contentsSize);
        memFree(chunk);
        ShipSetCataDockPointActivity(ship, NULL);
        return;
    }

    size = ShipCataDockPointActivitySize(ship);
    dbgAssert(size == chunk->contentsSize);
    loadedDockPointActivity = memAlloc(size,"dockPointActivity",NonVolatile);
    memcpy(loadedDockPointActivity,chunkContents(chunk),size);
    memFree(chunk);
    ShipSetCataDockPointActivity(ship, loadedDockPointActivity);
}
void LoadDockInfo(Ship *ship)
{
    LoadDockInfoChunk(ship, LoadNextChunk());
}

void FixDockInfo(Ship *ship)
{
    DockPointActivity *loadedDockPointActivity = ShipCataDockPointActivity(ship);

    if (loadedDockPointActivity == NULL || ship->staticinfo->dockStaticInfo == NULL)
    {
        ship->dockInfo = NULL;
        return;
    }

    ship->dockInfo = BuildDockInfoFromDockPointActivity(ship, loadedDockPointActivity);
}

void SaveSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *ship)
{
    Derelict *derelict = (Derelict *)ship;
    SalvageInfo *originalSalvageInfo = derelict->salvageInfo;
    SalvageInfo *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SALVAGEINFO,sizeofSalvageInfo(originalSalvageInfo->numSalvagePoints),originalSalvageInfo);
    savecontents = chunkContents(chunk);

    for (i=0;i<originalSalvageInfo->numSalvagePoints;i++)
    {
        if (savecontents->salvagePoints[i].salvageStaticPoint == NULL)
        {
            savecontents->salvagePoints[i].salvageStaticPoint = -1;
        }
        else
        {
            savecontents->salvagePoints[i].salvageStaticPoint = savecontents->salvagePoints[i].salvageStaticPoint->number;
        }
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *ship)
{
    SalvageInfo *loadedSalvageInfo;
    SalvageInfo *originalSalvageInfo;
    SaveChunk *chunk;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_SALVAGEINFO);

    originalSalvageInfo = (SalvageInfo *)chunkContents(chunk);
    size = sizeofSalvageInfo(originalSalvageInfo->numSalvagePoints);
    dbgAssert(size == chunk->contentsSize);
    loadedSalvageInfo = memAlloc(size,"salvageInfo",NonVolatile);
    memcpy(loadedSalvageInfo,originalSalvageInfo,size);

    memFree(chunk);

    ((Derelict *)ship)->salvageInfo = loadedSalvageInfo;
}

void FixSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *ship)
{
    SalvageInfo *salvageInfo = ((Derelict *)ship)->salvageInfo;
    SalvagePoint *salvagePoint;
    sdword i;

    for (i=0;i<salvageInfo->numSalvagePoints;i++)
    {
        salvagePoint = &salvageInfo->salvagePoints[i];
        if (salvagePoint->salvageStaticPoint == -1)
        {
            salvagePoint->salvageStaticPoint = NULL;
        }
        else
        {
            salvagePoint->salvageStaticPoint = &ship->staticinfo->salvageStaticInfo->salvageStaticPoints[(sdword)salvagePoint->salvageStaticPoint];
        }
    }
}

void SaveShipSinglePlayerGameInfo(Ship *ship)
{
    SaveChunk *chunk;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO,sizeof(ShipSinglePlayerGameInfo),ship->shipSinglePlayerGameInfo);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void LoadShipSinglePlayerGameInfoChunk(Ship *ship, SaveChunk *chunk)
{
    ShipSinglePlayerGameInfo *loadedShipSinglePlayerInfo;
    sdword copySize;

    dbgAssert(
        (chunk->type == (BASIC_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO)) ||
        (chunk->type == (VARIABLE_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO)));
    dbgAssert((chunk->contentsSize == 56) || (chunk->contentsSize == sizeof(ShipSinglePlayerGameInfo)));

    loadedShipSinglePlayerInfo = memAlloc(sizeof(ShipSinglePlayerGameInfo),"shipsingleGame",0);
    memset(loadedShipSinglePlayerInfo, 0, sizeof(ShipSinglePlayerGameInfo));

    copySize = (chunk->contentsSize < sizeof(ShipSinglePlayerGameInfo)) ? chunk->contentsSize : sizeof(ShipSinglePlayerGameInfo);
    memcpy(loadedShipSinglePlayerInfo, chunkContents(chunk), copySize);

    ship->shipSinglePlayerGameInfo = loadedShipSinglePlayerInfo;

    ship->shipSinglePlayerGameInfo->hyperspaceEffect = NULL;

    memFree(chunk);
}

void LoadShipSinglePlayerGameInfo(Ship *ship)
{
    LoadShipSinglePlayerGameInfoChunk(ship, LoadNextChunk());
}

void Save_String(char *string)
{
    SaveChunk *chunk;

    if (string == NULL)
    {
        chunk = CreateChunk(VARIABLE_STRUCTURE|SAVESTRING,0,string);
    }
    else
    {
        chunk = CreateChunk(VARIABLE_STRUCTURE|SAVESTRING,strlen(string)+1,string);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

char *Load_String(void)
{
    SaveChunk *chunk;
    char *str;
    char *contents;

    chunk = LoadNextChunk();

    if (chunk->contentsSize == 0)
    {
        memFree(chunk);
        return NULL;
    }

    contents = (char *)chunkContents(chunk);
    str = memAlloc(strlen(contents) + 1,"loadedstring",0);
    strcpy(str, contents);

    memFree(chunk);
    return str;
}

void Load_StringToAddress(char *addr)
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    strcpy(addr, (char *)chunkContents(chunk));

    memFree(chunk);
}

void SaveStructureOfSize(void *structure,sdword size)
{
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE,size,structure);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void *LoadStructureOfSize(sdword size)
{
    SaveChunk *chunk;
    void *structure;

    chunk = LoadNextChunk();
    if (chunk->type != BASIC_STRUCTURE || chunk->contentsSize != size)
    {
        dbgFatalf(DBG_Loc,
                  "LoadStructureOfSize mismatch: request=%d actual=%d type=0x%x offset=0x%lx ret=%p",
                  size,
                  chunk->contentsSize,
                  chunk->type,
                  lastLoadChunkOffset,
                  _ReturnAddress());
    }
    structure = memAlloc(size,"loadedstructure",0);
    memcpy(structure,chunkContents(chunk),size);

    memFree(chunk);
    return structure;
}

void LoadStructureOfSizeToAddress(void *address,sdword size)
{
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    memcpy(address,chunkContents(chunk),size);

    memFree(chunk);
}

void SaveStructureWithSize(void *structure)
{
    SaveChunk *chunk;
    chunk = CreateChunk(VARIABLE_STRUCTURE,*((sdword *)structure),structure);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void *LoadStructureWithSize()
{
    SaveChunk *chunk;
    void *chunkcontents;
    void *loadedstructure;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE);

    chunkcontents = chunkContents(chunk);
    dbgAssert( (*((sdword *)chunkcontents)) == chunk->contentsSize);

    loadedstructure = memAlloc(chunk->contentsSize,"loadedvarstr",0);
    memcpy(loadedstructure,chunkcontents,chunk->contentsSize);

    memFree(chunk);

    return loadedstructure;
}

void SaveShipsInsideMe(Ship *ship)
{
    ShipsInsideMe *shipsInsideMe = ShipSaveShipsInsideMe(ship);
    ShipsInsideMe saveShipsInsideMe;
    SaveChunk *chunk;

    saveShipsInsideMe = *shipsInsideMe;
    saveShipsInsideMe.insideList.head = NULL;
    saveShipsInsideMe.insideList.tail = NULL;
    saveShipsInsideMe.insideList.num = 0;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SHIPSINSIDEME, sizeof(saveShipsInsideMe), &saveShipsInsideMe);

    SaveThisChunk(chunk);
    memFree(chunk);

    SaveLinkedListOfInsideShips(&shipsInsideMe->insideList);
}

void LoadShipsInsideMe(Ship *ship)
{
    SaveChunk *chunk;
    ShipsInsideMe *shipsInsideMe;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk, BASIC_STRUCTURE|SAVE_SHIPSINSIDEME);
    dbgAssert(chunk->contentsSize == sizeof(ShipsInsideMe));

    shipsInsideMe = memAlloc(sizeof(ShipsInsideMe),"ShipsInsideMe",NonVolatile);
    memset(shipsInsideMe, 0, sizeof(ShipsInsideMe));
    memcpy(shipsInsideMe, chunkContents(chunk), sizeof(ShipsInsideMe));
    ShipSetCataShipsInsideMe(ship, shipsInsideMe);
    memFree(chunk);

    LoadLinkedListOfInsideShips(&shipsInsideMe->insideList);
}

void FixShipsInsideMe(Ship *ship)
{
    ShipsInsideMe *shipsInsideMe = ShipCataShipsInsideMe(ship);
    dbgAssert(shipsInsideMe != NULL);
    FixLinkedListOfInsideShips(&shipsInsideMe->insideList);
}

void CopyPointersFromRealStaticHeader(StaticHeader *dynamicstaticheader,StaticHeader *realstaticheader)
{
    dynamicstaticheader->LOD = realstaticheader->LOD;
    dynamicstaticheader->pMexData = realstaticheader->pMexData;
}

void CopyPointersFromRealAsteroidStaticInfo(AsteroidStaticInfo *dynamicstaticinfo,AsteroidStaticInfo *realstaticinfo)
{
    CopyPointersFromRealStaticHeader(&dynamicstaticinfo->staticheader,&realstaticinfo->staticheader);
}

void CopyPointersFromRealDustCloudStaticInfo(DustCloudStaticInfo *dynamicstaticinfo,DustCloudStaticInfo *realstaticinfo)
{
    CopyPointersFromRealStaticHeader(&dynamicstaticinfo->staticheader,&realstaticinfo->staticheader);
}

void CopyPointersFromRealShipStaticInfo(ShipStaticInfo *dynamicstaticinfo,ShipStaticInfo *realstaticinfo)
{
    sdword i;

    CopyPointersFromRealStaticHeader(&dynamicstaticinfo->staticheader,&realstaticinfo->staticheader);

    dynamicstaticinfo->custshipheader = realstaticinfo->custshipheader;
    dynamicstaticinfo->custstatinfo = realstaticinfo->custstatinfo;

    for (i=0;i<MAX_NUM_TRAILS;i++)
    {
        dynamicstaticinfo->engineNozzleOffset[i] = realstaticinfo->engineNozzleOffset[i];
        dynamicstaticinfo->trailStatic[i] = realstaticinfo->trailStatic[i];
    }

    dynamicstaticinfo->multipleEngines = realstaticinfo->multipleEngines;
    memcpy(dynamicstaticinfo->unk6, realstaticinfo->unk6, sizeof(dynamicstaticinfo->unk6));

    dynamicstaticinfo->madStatic = realstaticinfo->madStatic;

    dynamicstaticinfo->gunStaticInfo = realstaticinfo->gunStaticInfo;
    dynamicstaticinfo->dockStaticInfo = realstaticinfo->dockStaticInfo;
    dynamicstaticinfo->dockOverideInfo = realstaticinfo->dockOverideInfo;
    dynamicstaticinfo->navlightStaticInfo = realstaticinfo->navlightStaticInfo;
    dynamicstaticinfo->resNozzleStatic = realstaticinfo->resNozzleStatic;
    dynamicstaticinfo->tractorEmitterStatic = realstaticinfo->tractorEmitterStatic;
    ShipStaticInfoSetDockProcessCallback(dynamicstaticinfo,
                                         ShipStaticInfoGetDockProcessCallback(realstaticinfo));
}

void FixShipStaticInfoAfterLoad(ShipStaticInfo *dynamicstaticinfo, ShipStaticInfo *realstaticinfo)
{
    CopyPointersFromRealShipStaticInfo(dynamicstaticinfo, realstaticinfo);
}

void SaveClampInfo(ClampInfo *clampInfo)
{
    SaveChunk *chunk;
    ClampInfo *sc;

    chunk = CreateChunk(BASIC_STRUCTURE,sizeof(ClampInfo),clampInfo);
    sc = chunkContents(chunk);

    sc->host = SpaceObjRegistryGetID((SpaceObj *)sc->host);

    SaveThisChunk(chunk);

    if (sc->selection != NULL)
    {
        SaveSelection((SpaceObjSelection *)sc->selection);
    }

    memFree(chunk);
}

ClampInfo *LoadClampInfoChunk(SaveChunk *chunk)
{
    ClampInfo *clampInfo;
    sdword copySize;
    VerifyChunkNoSize(chunk, BASIC_STRUCTURE);
    dbgAssert(chunk->contentsSize >= 52);

    clampInfo = memAlloc(sizeof(ClampInfo),"ClampInfo",0);
    memset(clampInfo, 0, sizeof(ClampInfo));

    copySize = (chunk->contentsSize < sizeof(ClampInfo)) ? chunk->contentsSize : sizeof(ClampInfo);
    memcpy(clampInfo, chunkContents(chunk), copySize);

    memFree(chunk);

    if (clampInfo->host == (SpaceObjRotImpTarg *)-1)
    {
        SpaceObjSelection *selection = LoadSelection();
        SelectCommand *selectionCopy = memAlloc(sizeofSelectCommand(TOTAL_NUM_SHIPS), "ClampSelection", 0);

        memset(selectionCopy, 0, sizeofSelectCommand(TOTAL_NUM_SHIPS));
        memcpy(selectionCopy, selection, sizeofSelectCommand(selection->numSpaceObjs));
        memFree(selection);
        clampInfo->selection = selectionCopy;
    }

    return clampInfo;
}

ClampInfo *LoadClampInfo(void)
{
    return LoadClampInfoChunk(LoadNextChunk());
}

void SlaveAddToSlaveListCB(LinkedList *list,SpaceObj *obj);

void FixClampInfo(ClampInfo *clampInfo)
{
    sdword rawHost = (sdword)clampInfo->host;

    clampInfo->host = (SpaceObjRotImpTarg *)SpaceObjRegistryGetObj(rawHost);
    if (clampInfo->host == NULL && clampInfo->selection != NULL)
    {
        FixSelection((SpaceObjSelection *)clampInfo->selection);
    }
}

void FixSlaveInfo(SlaveInfo *slaveinfo)
{
    slaveinfo->Master = SpaceObjRegistryGetShip((sdword)slaveinfo->Master);

    if (slaveinfo->flags & SF_MASTER)
    {
        FixLinkedListOfSpaceObjs(&slaveinfo->slaves, SlaveAddToSlaveListCB);
    }
}

void SaveShip(Ship *ship)
{
    udword sizeship;
    udword totalsave;
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;
    bool32 saveDynamicStaticInfo;
    sdword i,j;

    Ship *savecontents;
    SaveChunk *chunk;

    // Retail SaveShip uses the serialized prefix size, not the larger live
    // allocation size. PreFix_madBindings relies on this exact serialized base.
    sizeship = SaveGameSerializedShipPrefixSize(shipstaticinfo);
    totalsave = sizeship;
    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
        totalsave += sizeof(PreciseSelection);
    }

    if (ship->madBindings != NULL)
    {
        totalsave += ship->madBindings->totalSize;
    }

    savecontents = memAlloc(totalsave,"ShipSave",Pyrophoric);
    memcpy(savecontents, ship, totalsave);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(ship->collMyBlob);
    savecontents->collInfo.precise = NULL;
    savecontents->playerowner = SavePlayerToPlayerIndex(savecontents->playerowner);
    SaveGameSaveSpaceObjRefAtOffset(savecontents, offsetof(Ship, dockingCollisionTarget));

    savecontents->rowGetOutOfWay = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->rowGetOutOfWay);
    savecontents->gettingrocked = NULL;
    savecontents->recentAttacker = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->recentAttacker);
    savecontents->firingAtUs = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->firingAtUs);

    // ship->flightmanInfo saved in chunk if non-NULL
    savecontents->command = ConvertPointerInListToNum(&universe.mainCommandLayer.todolist,savecontents->command);

    //ship->attackvars.multipleAttackTarget saved in chunk if non-NULL

    savecontents->attackvars.attacktarget = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->attackvars.attacktarget);
    savecontents->attackvars.myLeaderIs = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->attackvars.myLeaderIs);
    savecontents->attackvars.myWingmanIs = (sdword)SpaceObjRegistryGetID((SpaceObj *)ship->attackvars.myWingmanIs);

    savecontents->dockvars.dockship = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->dockvars.dockship);
    ShipSetCataDockPointActivity(savecontents, ShipCataDockPointActivity(ship));
    ShipSetCataShipsInsideMe(savecontents, ShipSaveShipsInsideMe(ship));

    SaveGameSaveSpaceObjRefAtOffset(savecontents, offsetof(Ship, dockvars.dockstaticpoint));
    SaveGameSaveSpaceObjRefAtOffset(savecontents, offsetof(Ship, dockvars.customdockinfo));
    SaveGameSaveSpaceObjRefAtOffset(savecontents, 0x3EC);

    savecontents->rceffect = NULL;
    PreFix_ShipXHarvestsResourceY(savecontents);
    SaveGameSaveSpaceObjRefAtOffset(savecontents, offsetof(Ship, beastAttack.sourceShip));
    SaveGameSavePlayerRefAtOffset(savecontents, offsetof(Ship, beastAttack.sourcePlayer));
    SaveGameSaveSpaceObjRefAtOffset(savecontents, offsetof(Ship, salCapDockApproachHost));

    for (i=0;i<3;i++)
    {
        for (j=0;j<3;j++)
        {
            savecontents->showingDamage[i][j] = NULL;
        }
    }

    SaveGameSavePlayerRefAtOffset(savecontents, offsetof(Ship, tractorPosinfo));

    savecontents->newDockWithTransfer = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents->newDockWithTransfer);

    for (i=0;i<MAX_NUM_TRAILS;i++)
    {
        savecontents->trail[i] = 0;     // on load re-initialize trails
    }
    savecontents->bindings = NULL;      // re-initialized on load by univMeshBindingsDupe

    // savecontents->soundevent will get reinitialized on load

    // gunInfo saved
    // dockInfo saved

    // reinitialize navLightInfo

    // shipsInsideMe saved

    if (ship->madBindings != NULL)
    {
        ShipSetCataMadBindingsBody(savecontents, ship->madBindings);
        //savecontents->madBindings = (madanim *)(((ubyte *)savecontents) + madoffset);
        PreFix_madBindings(ship,savecontents);
    }
    else
    {
        ShipSetCataMadBindingsBody(savecontents, NULL);
    }

    // Retail clears the runtime-only status-window tail before serializing the main ship chunk.
    savecontents->statusWindow.phaseEndTime = 0.0f;
    savecontents->statusWindow.phaseScale = 0.0f;

    // fix later
    for (i=0;i<2;i++)
    {
        savecontents->lightning[i] = 0;
    }

    if (shipstaticinfo->custshipheader.CustShip_PreFix != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_PreFix(savecontents);
    }

    saveDynamicStaticInfo = SaveGameShipHasRuntimeDynamicStaticInfo(ship);
    SaveGameSetSerializedDynamicStaticInfoFlag(&savecontents->flags, saveDynamicStaticInfo);

    chunk = CreateChunk(VARIABLE_STRUCTURE|SAVE_SPACEOBJ|OBJ_ShipType,totalsave,NULL);
    memcpy(chunkContents(chunk), savecontents, totalsave);
    SaveThisChunk(chunk);
    memFree(savecontents);
    memFree(chunk);

    if (saveDynamicStaticInfo)
    {
        SaveStructureOfSize(ship->staticinfo, sizeof(ShipStaticInfo));
    }

    if (ship->clampInfo != NULL)
    {
        SaveClampInfo(ship->clampInfo);
    }

    if (ship->flightmanInfo != NULL)
    {
        SaveStructureWithSize(ship->flightmanInfo);
    }

    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        SaveAttackTargets(ship->attackvars.multipleAttackTargets);
    }

    if (ship->damageLights)
    {
        dbgAssert(ship->nDamageLights > 0);
        SaveStructureOfSize(ship->damageLights,ship->nDamageLights*sizeof(DamageLightStatic));
    }

    if (ship->cataclysmGunInfo != NULL)
    {
        SaveGunInfo(ship);
    }

    if (ShipCataDockPointActivity(ship) != NULL)
    {
        SaveDockInfo(ship);
    }

    if (ShipSaveShipsInsideMe(ship) != NULL)
    {
        SaveShipsInsideMe(ship);
    }

    if (ship->madBindings != NULL)
    {
        Save_madBindings(ship);
    }

    if (ship->shipSinglePlayerGameInfo != NULL)
    {
        SaveShipSinglePlayerGameInfo(ship);
    }

    if (shipstaticinfo->custshipheader.CustShip_Save != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_Save(ship);
    }
   return;
}

Ship *LoadShip(SaveChunk *chunk)
{
    Ship *ship;
    udword sizeship;
    udword totalruntime;
    bool32 hasSerializedDynamicStaticInfo;
    ShipStaticInfo *shipstaticinfo;
    Ship *serializedShip;

   VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVE_SPACEOBJ|OBJ_ShipType);

   if (chunk->contentsSize < (sdword)(offsetof(Ship, ShipSpecifics) + sizeof(udword)))
   {
       dbgFatalf(DBG_Loc,
                 "LoadShip undersized main chunk ordinal=%d offset=0x%lx type=0x%x size=%d",
                 loadSpaceObjOrdinal,
                 lastLoadChunkOffset,
                 chunk->type,
                 chunk->contentsSize);
   }

    serializedShip = (Ship *)chunkContents(chunk);

   hasSerializedDynamicStaticInfo = SaveGameHasSerializedDynamicStaticInfo(serializedShip->flags);

   if (hasSerializedDynamicStaticInfo)
   {
       ShipStaticInfo *realstaticinfo = GetShipStaticInfo(serializedShip->shiptype,serializedShip->shiprace);

       shipstaticinfo = LoadStructureOfSize(sizeof(ShipStaticInfo));
       FixShipStaticInfoAfterLoad(shipstaticinfo, realstaticinfo);
    }
    else
    {
        shipstaticinfo = GetShipStaticInfo(serializedShip->shiptype,serializedShip->shiprace);
    }

    ship = memAlloc(chunk->contentsSize, "Ship", NonVolatile);
    memcpy(ship, chunkContents(chunk), chunk->contentsSize);
    ship->staticinfo = shipstaticinfo;
    SaveGameTranslateSerializedDynamicStaticInfoFlag(&ship->flags);
    ship->cataBusyStateWindow.busyStatePtr1EC = NULL;
    if (shipIsACV(ship))
    {
        acvGetEMPSelection(ship)->selection = NULL;
        acvGetEMPSelection(ship)->maxNumShips = 0;
    }

    // Retail LoadShip reconstructs the serialized prefix size directly from the
    // loaded ShipStaticInfo size field; it does not apply live allocation pads.
    sizeship = SaveGameSerializedShipPrefixSize(shipstaticinfo);
    totalruntime = sizeship;

    if (shipstaticinfo->staticheader.staticCollInfo.preciseSelection != 0xff)
    {
        ship->collInfo.precise = (PreciseSelection *)(((ubyte *)ship) + sizeship);
        totalruntime += sizeof(PreciseSelection);
    }
    else
    {
        ship->collInfo.precise = NULL;
    }

    if (ship->madBindings != NULL)
    {
        ship->madBindings = (madanim *)(((ubyte *)ship) + totalruntime);
        ShipSetCataMadBindingsBody(ship, ship->madBindings);
        totalruntime += ship->madBindings->totalSize;
    }
    else
    {
        ShipSetCataMadBindingsBody(ship, NULL);
    }

    if (totalruntime != (udword)chunk->contentsSize)
    {
        dbgFatalf(DBG_Loc,
                  "LoadShip size mismatch type=%d race=%d ordinal=%d offset=0x%lx chunk=%u computed=%u",
                  ship->shiptype,
                  ship->shiprace,
                  loadSpaceObjOrdinal,
                  lastLoadChunkOffset,
                  chunk->contentsSize,
                  totalruntime);
    }

    memFree(chunk);

    // re-initialize engine trails
    InitializeEngineTrails(ship);

    if (ship->clampInfo != NULL)
    {
        SaveChunk nextHeader;

        if (PeekNextChunkHeader(&nextHeader) &&
            nextHeader.type == BASIC_STRUCTURE &&
            (nextHeader.contentsSize == 52 || nextHeader.contentsSize == sizeof(ClampInfo)))
        {
            SaveChunk *nextChunk = LoadNextChunk();
            ship->clampInfo = LoadClampInfoChunk(nextChunk);
        }
        else
        {
            ship->clampInfo = NULL;
        }
    }

    if (ship->flightmanInfo != NULL)
    {
        ship->flightmanInfo = LoadStructureWithSize();
    }

    if (ship->attackvars.multipleAttackTargets != NULL)
    {
        ship->attackvars.multipleAttackTargets = LoadAttackTargets();
    }

    if (ship->damageLights != NULL)
    {
        dbgAssert(ship->nDamageLights > 0);
        ship->damageLights = LoadStructureOfSize(ship->nDamageLights*sizeof(DamageLightStatic));
    }

    if (ship->cataclysmGunInfo != NULL)
    {
        LoadGunInfo(ship);
    }

    ShipSetCataDockPointActivity(ship, NULL);
    {
        SaveChunk nextHeader;

        if (PeekNextChunkHeader(&nextHeader) &&
            nextHeader.type == (BASIC_STRUCTURE|SAVE_DOCKINFO))
        {
            LoadDockInfoChunk(ship, LoadNextChunk());
        }
    }

    setSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)ship);
    InitializeNavLights(ship);

    if (ShipCataShipsInsideMe(ship) != NULL)
    {
        LoadShipsInsideMe(ship);
    }

    ship->bindings = univMeshBindingsDupe(shipstaticinfo, ship);
    lodMeshBindingsAllocate((SpaceObj *)ship);

    if (ship->madBindings != NULL)
    {
        Load_madBindings(ship);
    }

    {
        SaveChunk nextHeader;

        if (PeekNextChunkHeader(&nextHeader) &&
            ((nextHeader.type == (BASIC_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO)) ||
             (nextHeader.type == (VARIABLE_STRUCTURE|SAVE_SHIPSINGLEPLAYERGAMEINFO))) &&
            ((nextHeader.contentsSize == 56) || (nextHeader.contentsSize == sizeof(ShipSinglePlayerGameInfo))))
        {
            LoadShipSinglePlayerGameInfoChunk(ship, LoadNextChunk());
        }
        else
        {
            ship->shipSinglePlayerGameInfo = NULL;
        }
    }

    if (shipstaticinfo->custshipheader.CustShip_Load != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_Load(ship);
    }
    return ship;
}

void FixShip(Ship *ship)
{
    ShipStaticInfo *shipstaticinfo = ship->staticinfo;

    ship->collMyBlob = BlobRegistryGetBlob((sdword)ship->collMyBlob);

    ship->playerowner = SavePlayerIndexToPlayer(ship->playerowner);
    SaveGameFixSpaceObjRefAtOffset(ship, offsetof(Ship, dockingCollisionTarget));

    ship->rowGetOutOfWay = SpaceObjRegistryGetShip((sdword)ship->rowGetOutOfWay);
    ship->recentAttacker = SpaceObjRegistryGetShip((sdword)ship->recentAttacker);
    ship->firingAtUs = SpaceObjRegistryGetShip((sdword)ship->firingAtUs);

    ship->formationcommand = NULL;
    ship->command = ConvertNumToPointerInList(&universe.mainCommandLayer.todolist,(sdword)ship->command);

    if (ship->attackvars.multipleAttackTargets)
    {
        FixAttackTargets(ship->attackvars.multipleAttackTargets);
    }
    ship->attackvars.attacktarget = SpaceObjRegistryGetTarget((sdword)ship->attackvars.attacktarget);
    ship->attackvars.myLeaderIs = SpaceObjRegistryGetShip((sdword)ship->attackvars.myLeaderIs);
    ship->attackvars.myWingmanIs = SpaceObjRegistryGetShip((sdword)ship->attackvars.myWingmanIs);
    ship->dockvars.dockship = SpaceObjRegistryGetShip((sdword)ship->dockvars.dockship);

    SaveGameFixSpaceObjRefAtOffset(ship, offsetof(Ship, dockvars.dockstaticpoint));
    SaveGameFixSpaceObjRefAtOffset(ship, offsetof(Ship, dockvars.customdockinfo));
    SaveGameFixSpaceObjRefAtOffset(ship, 0x3EC);
    SaveGameFixSpaceObjRefAtOffset(ship, offsetof(Ship, beastAttack.sourceShip));
    SaveGameFixPlayerRefAtOffset(ship, offsetof(Ship, beastAttack.sourcePlayer));
    SaveGameFixSpaceObjRefAtOffset(ship, offsetof(Ship, salCapDockApproachHost));

    if (ship->beastAttack.effect != NULL)
    {
        ShipType replacementType = ship->staticinfo->beastAttackConvertTo;
        ShipStaticInfo *replacementStaticInfo =
            GetShipStaticInfo(replacementType, ship->beastAttack.sourcePlayer->race);

        ship->beastAttack.effect = (Effect *)univMeshBindingsStaticDupe(replacementStaticInfo);
    }

    if (ship->shipSinglePlayerGameInfo != NULL)
    {
        FixLinkedListOfInsideShips((LinkedList *)ship->shipSinglePlayerGameInfo);
    }

    if (ShipCataShipsInsideMe(ship) != NULL)
    {
        FixShipsInsideMe(ship);
    }

    if (ShipCataSelection22C(ship) != NULL)
    {
        FixSelection(ShipCataSelection22C(ship));
    }

    if (ship->clampInfo != NULL)
    {
        FixClampInfo(ship->clampInfo);
    }

    if (ship->madBindings != NULL)
    {
        Fix_madBindings(ship);
    }

    Fix_ShipXHarvestsResourceY(ship);

    SaveGameFixPlayerRefAtOffset(ship, offsetof(Ship, tractorPosinfo));

    ship->newDockWithTransfer = SpaceObjRegistryGetShip((sdword)ship->newDockWithTransfer);

    if (shipstaticinfo->custshipheader.CustShip_Fix != NULL)
    {
        shipstaticinfo->custshipheader.CustShip_Fix(ship);
    }

    if (shipstaticinfo->nRepairDroids != 0 && ship->shiprace == RACE_Sect)
    {
        ship->runtimeAnimationOverlay.runtimeController =
            (udword)CreateRepairDroid(ship, shipstaticinfo->nRepairDroids);
        if (ship->runtimeAnimationOverlay.runtimeController == 0)
        {
            ShipTypeToStr(shipstaticinfo->shiptype);
            telldir();
        }
    }
    else
    {
        ship->runtimeAnimationOverlay.runtimeController = 0;
    }
}

void SaveAsteroid(Asteroid *asteroid)
{
    Asteroid savebody;
    Asteroid *savecontents;
    SaveChunk *chunk;
    bool32 saveDynamicStaticInfo;

    savebody = *asteroid;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_AsteroidType,sizeof(Asteroid),&savebody);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetIDNoBlobOkay(asteroid->collMyBlob);
    saveDynamicStaticInfo = ((asteroid->flags & SOF_StaticInfoIsDynamicCata) != 0);
    SaveGameSetSerializedDynamicStaticInfoFlag(&savecontents->flags, saveDynamicStaticInfo);

    savecontents->resourceVolume = ConvertPointerInListToNum(&universe.ResourceVolumeList,asteroid->resourceVolume);
    savecontents->cataRuntimeState2.linkedDerelict = (struct Derelict *)SpaceObjRegistryGetID((SpaceObj *)asteroid->cataRuntimeState2.linkedDerelict);

    SaveThisChunk(chunk);
    memFree(chunk);

    if (saveDynamicStaticInfo)
    {
        SaveResourceStaticInfoChunk(asteroid->staticinfo, sizeof(AsteroidStaticInfo));
    }
}

Asteroid *LoadAsteroid(SaveChunk *chunk)
{
    Asteroid *asteroid;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_AsteroidType,sizeof(Asteroid));

    asteroid = memAlloc(sizeof(Asteroid),"Asteroid",NonVolatile);

    memcpy(asteroid, chunkContents(chunk), sizeof(Asteroid));
    ClearNode(asteroid->renderlink);
    ClearNode(asteroid->objlink);
    ClearNode(asteroid->impactablelink);
    ClearNode(asteroid->resourcelink);
    asteroid->collInfo.precise = NULL;

    memFree(chunk);

    SaveGameTranslateSerializedDynamicStaticInfoFlag(&asteroid->flags);
    asteroid->flags |= SOF_Resource;
    asteroid->staticinfo = (ResourceStaticInfo *)&asteroidStaticInfos[asteroid->asteroidtype];
    if (asteroid->flags & SOF_StaticInfoIsDynamicCata)
    {
        asteroid->staticinfo = LoadResourceStaticInfoChunk((ResourceStaticInfo *)&asteroidStaticInfos[asteroid->asteroidtype],
                                                           sizeof(AsteroidStaticInfo));
    }

    // asteroid->collMyBlob gets fixed in FixAllSpaceObjs

    return asteroid;
}

void FixAsteroid(Asteroid *asteroid)
{
    asteroid->collMyBlob = BlobRegistryGetBlob((sdword)asteroid->collMyBlob);
    asteroid->resourceVolume = ConvertNumToPointerInList(&universe.ResourceVolumeList,(sdword)asteroid->resourceVolume);
    asteroid->cataRuntimeState2.linkedDerelict = (struct Derelict *)SpaceObjRegistryGetObj((sdword)asteroid->cataRuntimeState2.linkedDerelict);
    asteroid->collInfo.precise = NULL;
}

void SaveCloudSystem(DustCloud *dustcloud)
{
    cloudSystem *savecontents;
    SaveChunk *chunk;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_CLOUDSYSTEM,sizeof(cloudSystem),dustcloud->stub);

    savecontents = chunkContents(chunk);

    savecontents->position = NULL;
    savecontents->rotation = NULL;

    for (i=0;i<MAX_BURSTS;i++)
    {
        savecontents->bursts[i] = 0;        // reset all lightening for now.  Fix later
    }

    SaveThisChunk(chunk);

    memFree(chunk);
}

void LoadCloudSystem(DustCloud *dustcloud)
{
    cloudSystem *stub;
    SaveChunk *chunk;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_CLOUDSYSTEM,sizeof(cloudSystem));

    stub = memAlloc(sizeof(cloudSystem),"cloud system", NonVolatile);

    memcpy(stub,chunkContents(chunk),chunk->contentsSize);

    memFree(chunk);

    dustcloud->stub = stub;
    stub->position = &dustcloud->posinfo.position;
    stub->rotation = &dustcloud->rotinfo.coordsys;
}

void SaveDustCloud(DustCloud *dustcloud)
{
    DustCloud savebody;
    DustCloud *savecontents;
    SaveChunk *chunk;
    bool32 saveDynamicStaticInfo;

    savebody = *dustcloud;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DustType,sizeof(DustCloud),&savebody);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(dustcloud->collMyBlob);
    saveDynamicStaticInfo = ((dustcloud->flags & SOF_StaticInfoIsDynamicCata) != 0);
    SaveGameSetSerializedDynamicStaticInfoFlag(&savecontents->flags, saveDynamicStaticInfo);

    savecontents->resourceVolume = ConvertPointerInListToNum(&universe.ResourceVolumeList,dustcloud->resourceVolume);

    SaveThisChunk(chunk);
    memFree(chunk);

    if (saveDynamicStaticInfo)
    {
        SaveResourceStaticInfoChunk(dustcloud->staticinfo, sizeof(DustCloudStaticInfo));
    }

    if (dustcloud->stub != NULL)
    {
        SaveCloudSystem(dustcloud);
    }
}

DustCloud *LoadDustCloud(SaveChunk *chunk)
{
    DustCloud *dustcloud;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DustType,sizeof(DustCloud));

    dustcloud = memAlloc(sizeof(DustCloud),"DustCloud",NonVolatile);

    memcpy(dustcloud, chunkContents(chunk), chunk->contentsSize);

    memFree(chunk);

    dustcloud->staticinfo = (ResourceStaticInfo *)&dustcloudStaticInfos[dustcloud->dustcloudtype];
    if (dustcloud->flags & SOF_StaticInfoIsDynamicCata)
    {
        dustcloud->staticinfo = LoadResourceStaticInfoChunk((ResourceStaticInfo *)&dustcloudStaticInfos[dustcloud->dustcloudtype],
                                                            sizeof(DustCloudStaticInfo));
    }

    // dustcloud->collMyBlob gets fixed in FixAllSpaceObjs

    if (dustcloud->stub != NULL)
    {
        LoadCloudSystem(dustcloud);
    }

    return dustcloud;
}

void FixDustCloud(DustCloud *dustcloud)
{
    dustcloud->collMyBlob = BlobRegistryGetBlob((sdword)dustcloud->collMyBlob);
    dustcloud->resourceVolume = ConvertNumToPointerInList(&universe.ResourceVolumeList,(sdword)dustcloud->resourceVolume);
    dustcloud->collInfo.precise = NULL;
}

void SaveNebula(Nebula *nebula)
{
    Nebula savebody;
    Nebula *savecontents;
    SaveChunk *chunk;
    nebChunk* nebchunk;
    nebulae_t* neb;

    savebody = *nebula;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_NebulaType,sizeof(Nebula),&savebody);

    dbgAssert((nebula->flags & SOF_StaticInfoIsDynamicCata) == 0);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(nebula->collMyBlob);
    savecontents->resourceVolume = ConvertPointerInListToNum(&universe.ResourceVolumeList,nebula->resourceVolume);
    nebchunk = (nebChunk*)nebula->stub;
    neb = (nebulae_t*)nebchunk->nebulae;
    savecontents->stub = nebChunkPtrToNum(neb, nebula->stub);

    SaveThisChunk(chunk);
    memFree(chunk);
}

Nebula *LoadNebula(SaveChunk *chunk)
{
    Nebula *nebula;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_NebulaType,sizeof(Nebula));

    nebula = memAlloc(sizeof(Nebula),"Nebula",NonVolatile);

    memcpy(nebula, chunkContents(chunk), sizeof(Nebula));
    ClearNode(nebula->renderlink);
    ClearNode(nebula->objlink);
    ClearNode(nebula->impactablelink);
    ClearNode(nebula->resourcelink);
    nebula->collInfo.precise = NULL;

    memFree(chunk);

    nebula->flags |= SOF_Resource;
    nebula->staticinfo = (ResourceStaticInfo *)&nebulaStaticInfos[nebula->nebulatype];

    return nebula;
}

void FixNebula(Nebula *nebula)
{
    nebulae_t* neb;

    nebula->collMyBlob = BlobRegistryGetBlob((sdword)nebula->collMyBlob);
    nebula->resourceVolume = ConvertNumToPointerInList(&universe.ResourceVolumeList,(sdword)nebula->resourceVolume);

    neb = &nebNebulae[nebula->nebNebulaeIndex];
    nebula->stub = nebNumToChunkPtr(neb, (sdword)nebula->stub);
    nebula->collInfo.precise = NULL;
}

void SaveBullet(Bullet *bullet)
{
    Bullet *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_BulletType,sizeof(Bullet),bullet);

    savecontents = chunkContents(chunk);

    dbgAssert(savecontents->staticinfo == NULL);
    savecontents->collMyBlob = BlobRegistryGetID(bullet->collMyBlob);

    savecontents->owner = SpaceObjRegistryGetID((SpaceObj *)bullet->owner);
    if (bullet->gunowner != NULL)
    {
        savecontents->gunowner = bullet->gunowner->gunstatic->gunindex;
    }
    else
    {
        savecontents->gunowner = -1;
    }
    savecontents->target = SpaceObjRegistryGetID((SpaceObj *)bullet->target);

    savecontents->hitEffect = NULL;
    savecontents->effect = NULL;
    savecontents->playerowner = SavePlayerToPlayerIndex(bullet->playerowner);

    SaveThisChunk(chunk);

    memFree(chunk);
}

Bullet *LoadBullet(SaveChunk *chunk)
{
    Bullet *bullet;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_BulletType,sizeof(Bullet));

    bullet = memAlloc(sizeof(Bullet),"Bullet",0);

    memcpy(bullet,chunkContents(chunk),chunk->contentsSize);
    SaveGameClearLoadedPosRuntimePad(&bullet->posinfo);

    memFree(chunk);

    // bullet->collMyBlob gets fixed in FixAllSpaceObjs
    return bullet;
}

void FixBullet(Bullet *bullet)
{
    Ship *ownerShip = NULL;
    bullet->collMyBlob = BlobRegistryGetBlob((sdword)bullet->collMyBlob);

    bullet->owner = SpaceObjRegistryGetShip((sdword)bullet->owner);
    ownerShip = (Ship *)bullet->owner;
    if (bullet->gunowner != -1 && ownerShip != NULL && ownerShip->cataclysmGunInfo != NULL)
    {
        if ((sdword)bullet->gunowner < ownerShip->cataclysmGunInfo->numGuns)
        {
            bullet->gunowner = &ownerShip->cataclysmGunInfo->guns[(sdword)bullet->gunowner];
        }
        else
        {
            dbgMessagef(
                "FixBullet: nulling bad gunowner index=%d numGuns=%d owner=0x%x",
                (sdword)bullet->gunowner,
                ownerShip->cataclysmGunInfo->numGuns,
                ownerShip);
            bullet->gunowner = NULL;
        }
    }
    else
    {
        bullet->gunowner = NULL;
    }

    bullet->target = (SpaceObjRotImpTarg *)SpaceObjRegistryGetShip((sdword)bullet->target);

    bullet->playerowner = SavePlayerIndexToPlayer(bullet->playerowner);
}

void SaveDerelict(Derelict *derelict)
{
    CataDerelictSaveBody savebody;
    CataDerelictSaveBody *savecontents;
    SaveChunk *chunk;

    dbgAssert(sizeof(CataDerelictSaveBody) == 660);

    memset(&savebody, 0, sizeof(savebody));

    memcpy(&savebody, derelict, offsetof(Derelict, cameraDistanceSquared));
    memcpy(&savebody.cameraDistanceSquared,
           &derelict->cameraDistanceSquared,
           offsetof(Derelict, nonstatvars) - offsetof(Derelict, cameraDistanceSquared));
    savebody.nonstatvars = derelict->nonstatvars;
    savebody.clampInfo = derelict->clampInfo;
    savebody.aistate = derelict->aistate;
    memcpy(savebody.pad1, derelict->pad1, sizeof(savebody.pad1));
    savebody.putOnDoor = derelict->putOnDoor;
    savebody.dockingship = derelict->dockingship;
    savebody.specialFlags = derelict->specialFlags;
    savebody.specialFlags2 = derelict->specialFlags2;
    memcpy(&savebody.specialFlags3, &derelict->specialFlags3, sizeof(savebody.specialFlags3) * MAX_MULTIPLAYER_PLAYERS);
    savebody.navLightInfo = derelict->navLightInfo;
    memcpy(&savebody.derelictlink, &derelict->derelictlink, sizeof(savebody.derelictlink));
    savebody.derelictID = derelict->derelictID;
    savebody.colorScheme = derelict->colorScheme;
    memcpy(savebody.pad, derelict->pad, sizeof(savebody.pad));
    savebody.ambientSoundHandle = derelict->ambientSoundHandle;
    savebody.randomSoundHandle = derelict->randomSoundHandle;
    savebody.nextRandomTime = derelict->nextRandomTime;
    savebody.TractorTrue = derelict->TractorTrue;
    savebody.tractorbeaminfo = derelict->tractorbeaminfo;
    savebody.tractorvector = derelict->tractorvector;
    savebody.tractorbeam_playerowner = derelict->tractorbeam_playerowner;
    savebody.tractorstate = derelict->tractorstate;
    memcpy(savebody.tractorPosinfo, derelict->tractorPosinfo, sizeof(savebody.tractorPosinfo));
    savebody.needNewLeader = derelict->needNewLeader;
    savebody.newDockWithTransfer = derelict->newDockWithTransfer;
    savebody.newDockIndexTransfer = derelict->newDockIndexTransfer;
    savebody.creationTime = derelict->creationTime;
    savebody.derelicttype = derelict->derelicttype;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DerelictType,sizeof(CataDerelictSaveBody),&savebody);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetIDNoBlobOkay(derelict->collMyBlob);       // planets aren't in blobs

    savecontents->dockingship = SpaceObjRegistryGetID((SpaceObj *)derelict->dockingship);
    savecontents->tractorbeam_playerowner = SavePlayerToPlayerIndex(derelict->tractorbeam_playerowner);

    savecontents->newDockWithTransfer = SpaceObjRegistryGetID((SpaceObj *)derelict->newDockWithTransfer);

    SaveThisChunk(chunk);

    memFree(chunk);

    if (derelict->clampInfo != NULL)
    {
        SaveClampInfo(derelict->clampInfo);
    }
}

Derelict *LoadDerelict(SaveChunk *chunk)
{
    Derelict *derelict;
    CataDerelictSaveBody *loadcontents;

    dbgAssert(sizeof(CataDerelictSaveBody) == 660);
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_DerelictType,sizeof(CataDerelictSaveBody));

    derelict = memAlloc(sizeof(Derelict),"Derelict",NonVolatile);
    memset(derelict, 0, sizeof(*derelict));

    loadcontents = chunkContents(chunk);

    memcpy(derelict, loadcontents, offsetof(Derelict, cameraDistanceSquared));
    memcpy(&derelict->cameraDistanceSquared,
           &loadcontents->cameraDistanceSquared,
           offsetof(Derelict, nonstatvars) - offsetof(Derelict, cameraDistanceSquared));
    derelict->nonstatvars = loadcontents->nonstatvars;
    derelict->clampInfo = loadcontents->clampInfo;
    derelict->aistate = loadcontents->aistate;
    memcpy(derelict->pad1, loadcontents->pad1, sizeof(derelict->pad1));
    derelict->putOnDoor = loadcontents->putOnDoor;
    derelict->dockingship = loadcontents->dockingship;
    derelict->specialFlags = loadcontents->specialFlags;
    derelict->specialFlags2 = loadcontents->specialFlags2;
    memcpy(&derelict->specialFlags3, &loadcontents->specialFlags3, sizeof(loadcontents->specialFlags3) * MAX_MULTIPLAYER_PLAYERS);
    derelict->navLightInfo = loadcontents->navLightInfo;
    memcpy(&derelict->derelictlink, &loadcontents->derelictlink, sizeof(derelict->derelictlink));
    derelict->derelictID = loadcontents->derelictID;
    derelict->colorScheme = loadcontents->colorScheme;
    memcpy(derelict->pad, loadcontents->pad, sizeof(derelict->pad));
    derelict->ambientSoundHandle = loadcontents->ambientSoundHandle;
    derelict->randomSoundHandle = loadcontents->randomSoundHandle;
    derelict->nextRandomTime = loadcontents->nextRandomTime;
    derelict->TractorTrue = loadcontents->TractorTrue;
    derelict->tractorbeaminfo = loadcontents->tractorbeaminfo;
    derelict->tractorvector = loadcontents->tractorvector;
    derelict->tractorbeam_playerowner = loadcontents->tractorbeam_playerowner;
    derelict->tractorstate = loadcontents->tractorstate;
    memcpy(derelict->tractorPosinfo, loadcontents->tractorPosinfo, sizeof(derelict->tractorPosinfo));
    derelict->needNewLeader = loadcontents->needNewLeader;
    derelict->newDockWithTransfer = loadcontents->newDockWithTransfer;
    derelict->newDockIndexTransfer = loadcontents->newDockIndexTransfer;
    derelict->creationTime = loadcontents->creationTime;
    derelict->derelicttype = loadcontents->derelicttype;
    derelict->salvageInfo = NULL;
    derelict->collInfo.precise = NULL;
    SaveGameClearLoadedPosRuntimePad(&derelict->posinfo);

    memFree(chunk);

    dbgAssert(derelict->derelicttype < NUM_DERELICTTYPES);
    derelict->staticinfo = (DerelictStaticInfo *)&derelictStaticInfos[derelict->derelicttype];

    InitializeDerelictNavLights(derelict);

    if (derelict->clampInfo != NULL)
    {
        derelict->clampInfo = LoadClampInfo();
    }
    setSalvageInfo((SpaceObjRotImpTargGuidanceShipDerelict *)derelict);

    return derelict;
}

void FixDerelict(Derelict *derelict)
{
    derelict->collMyBlob = BlobRegistryGetBlob((sdword)derelict->collMyBlob);

    derelict->dockingship = SpaceObjRegistryGetShip((sdword)derelict->dockingship);
    derelict->tractorbeam_playerowner = SavePlayerIndexToPlayer(derelict->tractorbeam_playerowner);

    derelict->newDockWithTransfer = SpaceObjRegistryGetShip((sdword)derelict->newDockWithTransfer);

    if (derelict->clampInfo)
    {
        FixClampInfo(derelict->clampInfo);
    }
    derelict->collInfo.precise = NULL;
}

void SaveMissile(Missile *missile)
{
    Missile *savecontents;
    SaveChunk *chunk;
    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_MissileType,sizeof(Missile),missile);

    savecontents = chunkContents(chunk);

    savecontents->staticinfo = NULL;
    savecontents->collMyBlob = BlobRegistryGetID(missile->collMyBlob);

    savecontents->trail = NULL;

    savecontents->playerowner = SavePlayerToPlayerIndex(missile->playerowner);
    savecontents->owner = SpaceObjRegistryGetID((SpaceObj *)missile->owner);

    savecontents->target = SpaceObjRegistryGetID((SpaceObj *)missile->target);

    savecontents->hitEffect = saveEtglodGunEventToIndex(missile->hitEffect);

    savecontents->formationinfo = ConvertPointerInListToNum(&universe.MineFormationList,savecontents->formationinfo);

    SaveThisChunk(chunk);

    memFree(chunk);

    if (missile->clampInfo != NULL)
    {
        SaveClampInfo(missile->clampInfo);
    }
}

Missile *LoadMissile(SaveChunk *chunk)
{
    Missile *missile;

    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_SPACEOBJ|OBJ_MissileType,sizeof(Missile));

    missile = memAlloc(sizeof(Missile),"Missile",0);

    memcpy(missile,chunkContents(chunk),chunk->contentsSize);
    SaveGameClearLoadedPosRuntimePad(&missile->posinfo);
    missile->collInfo.precise = NULL;

    memFree(chunk);

    if(missile->missileType == MISSILE_Regular)
    {
        missile->staticinfo = &missileStaticInfos[missile->race];
    }
    else
    {
        dbgAssert(missile->missileType == MISSILE_Mine);
        missile->staticinfo = &mineStaticInfos[missile->race];
    }

    if (missile->clampInfo != NULL)
    {
        missile->clampInfo = LoadClampInfo();
    }

    return missile;
}

void FixMissile(Missile *missile)
{
    missile->collMyBlob = BlobRegistryGetBlob((sdword)missile->collMyBlob);

    missile->playerowner = SavePlayerIndexToPlayer(missile->playerowner);

    missile->owner = SpaceObjRegistryGetShip((sdword)missile->owner);
    missile->target = SpaceObjRegistryGetTarget((sdword)missile->target);

    missile->hitEffect = saveIndexToEtglodGunEvent((sdword)missile->hitEffect);

    missile->formationinfo = ConvertNumToPointerInList(&universe.MineFormationList,(sdword)missile->formationinfo);

    if (missile->clampInfo)
    {
        FixClampInfo(missile->clampInfo);
    }
}

void SaveSpaceObj(SpaceObj *obj)
{
    switch (obj->objtype)
    {
        case OBJ_ShipType:
            SaveShip((Ship *)obj);
            break;

        case OBJ_BulletType:
            SaveBullet((Bullet *)obj);
            break;

        case OBJ_AsteroidType:
            SaveAsteroid((Asteroid *)obj);
            break;

        case OBJ_NebulaType:
            SaveNebula((Nebula *)obj);
            break;

        case OBJ_GasType:
            dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
            break;

        case OBJ_DustType:
            SaveDustCloud((DustCloud *)obj);
            break;

        case OBJ_DerelictType:
            SaveDerelict((Derelict *)obj);
            break;

        case OBJ_EffectType:
            dbgAssert(FALSE);
            break;

        case OBJ_MissileType:
            SaveMissile((Missile *)obj);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

void FixSpaceObj(SpaceObj *obj)
{
    lodinfo *lodInfo;

    bitClear(obj->flags, SOF_DontCreateBlob);

    switch (obj->objtype)
    {
        case OBJ_ShipType:
            FixShip((Ship *)obj);
            break;

        case OBJ_BulletType:
            FixBullet((Bullet *)obj);
            break;

        case OBJ_AsteroidType:
            FixAsteroid((Asteroid *)obj);
            break;

        case OBJ_NebulaType:
            FixNebula((Nebula *)obj);
            break;

        case OBJ_GasType:
            dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
            break;

        case OBJ_DustType:
            FixDustCloud((DustCloud *)obj);
            break;

        case OBJ_DerelictType:
            FixDerelict((Derelict *)obj);
            break;

        case OBJ_EffectType:
            dbgAssert(FALSE);
            break;

        case OBJ_MissileType:
            FixMissile((Missile *)obj);
            break;

        default:
            dbgAssert(FALSE);
            break;
    }

    if (obj->flags & SOF_Targetable)
    {
        ((SpaceObjRotImpTarg *)obj)->flashtimer = 0.0f;
    }

    lodInfo = (obj->staticinfo != NULL) ? obj->staticinfo->staticheader.LOD : NULL;
    if (lodInfo != NULL)
    {
        if (obj->currentLOD >= lodInfo->nLevels)
        {
            obj->currentLOD = (ubyte)((lodInfo->nLevels > 0) ? (lodInfo->nLevels - 1) : 0);
        }
        obj->renderedLODs = 0;
        obj->cameraDistanceSquared = REALlyBig;
    }
}

SpaceObj *LoadSpaceObj()
{
    SaveChunk *chunk;

    loadSpaceObjOrdinal++;
    chunk = LoadNextChunk();
    if ((chunk->type & SAVE_SPACEOBJ) == 0)
    {
        dbgFatalf(DBG_Loc,
                  "LoadSpaceObj desync ordinal=%d offset=0x%lx type=0x%x size=%d",
                  loadSpaceObjOrdinal,
                  lastLoadChunkOffset,
                  chunk->type,
                  chunk->contentsSize);
    }

    switch (chunk->type & SAVE_OBJTYPEMASK)
    {
        case OBJ_ShipType:
            return (SpaceObj *)LoadShip(chunk);

        case OBJ_BulletType:
            return (SpaceObj *)LoadBullet(chunk);

        case OBJ_AsteroidType:
            return (SpaceObj *)LoadAsteroid(chunk);

        case OBJ_NebulaType:
            return (SpaceObj *)LoadNebula(chunk);

        case OBJ_GasType:
            dbgFatal(DBG_Loc,"Saving Gas clouds not supported yet");
            return NULL;

        case OBJ_DustType:
            return (SpaceObj *)LoadDustCloud(chunk);

        case OBJ_DerelictType:
            return (SpaceObj *)LoadDerelict(chunk);

        case OBJ_EffectType:
            dbgAssert(FALSE);
            return NULL;

        case OBJ_MissileType:
            return (SpaceObj *)LoadMissile(chunk);

        default:
            dbgAssert(FALSE);
            return NULL;
    }
}

typedef struct SaveLLSpaceObj {
    sdword num;
    sdword ID[1];
} SaveLLSpaceObj;

#define sizeofSaveLLSpaceObj(n) (sizeof(SaveLLSpaceObj) + (n-1)*sizeof(sdword))

void SaveLinkedListOfSpaceObjs(LinkedList *list)
{
    sdword num = list->num;
    SaveChunk *chunk;
    SaveLLSpaceObj *savecontents;
    Node *node = list->head;
    SpaceObj *obj;
    sdword cur = 0;

    chunk = CreateChunk(LINKEDLISTSPACEOBJS,sizeofSaveLLSpaceObj(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    while (node != NULL)
    {
        obj = (SpaceObj *)listGetStructOfNode(node);
        dbgAssert(obj);

        savecontents->ID[cur++] = SpaceObjRegistryGetID(obj);

        node = node->next;
    }

    dbgAssert(cur == num);

    SaveThisChunk(chunk);

    memFree(chunk);
}

void JustLoadLinkedListOfSpaceObjs(LinkedList *list)
{
    SaveChunk *chunk;
    SaveLLSpaceObj *loadcontents;
    SaveLLSpaceObj *returncontents;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,LINKEDLISTSPACEOBJS);

    loadcontents = (struct SaveLLSpaceObj *)chunkContents(chunk);

    dbgAssert(sizeofSaveLLSpaceObj(loadcontents->num) == chunk->contentsSize);

    returncontents = memAlloc(chunk->contentsSize,"unfixedlistobj",0);
    memcpy(returncontents,loadcontents,chunk->contentsSize);
    memFree(chunk);

    list->head = (void *)returncontents;
}

void FixLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB)
{
    SaveLLSpaceObj *objs;
    SpaceObj *obj;
    sdword num;
    sdword cur = 0;

    objs = (SaveLLSpaceObj *)list->head;
    dbgAssert(objs);

    listInit(list);

    num = objs->num;

    while (cur < num)
    {
        obj = SpaceObjRegistryGetObj(objs->ID[cur++]);
        if (obj != NULL)
        {
            listAddCB(list,obj);
        }
    }

    memFree(objs);
}

void LoadLinkedListOfSpaceObjs(LinkedList *list,ListAddCB listAddCB)
{
    SaveChunk *chunk;
    SaveLLSpaceObj *loadcontents;
    sdword num;
    sdword cur = 0;
    SpaceObj *obj;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,LINKEDLISTSPACEOBJS);

    loadcontents = (struct SaveLLSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(sizeofSaveLLSpaceObj(num) == chunk->contentsSize);

    listInit(list);


    while (cur < num)
    {
        obj = SpaceObjRegistryGetObj(loadcontents->ID[cur++]);
        if (obj != NULL)
        {
            listAddCB(list,obj);
        }
    }

    memFree(chunk);
}

void RenderListAddCB(LinkedList *list,SpaceObj *obj)
{
    listAddNode(list,&obj->renderlink,obj);
}

void SpaceObjListAddCB(LinkedList *list,SpaceObj *obj)
{
    listAddNode(list,&obj->objlink,obj);
}

#define MinorRenderListAddCB RenderListAddCB
#define MinorSpaceObjListAddCB SpaceObjListAddCB

void EffectListAddCB(LinkedList *list, SpaceObj *obj)
{
#define effect ((Effect *)obj)
    dbgAssert(effect->objtype == OBJ_EffectType);
    listAddNode(list, &effect->effectLink, obj);
#undef effect
}

void ShipListAddCB(LinkedList *list,SpaceObj *obj)
{
#define ship ((Ship *)obj)
    dbgAssert(ship->objtype == OBJ_ShipType);
    listAddNode(list,&ship->shiplink,obj);
#undef ship
}

void BulletListAddCB(LinkedList *list,SpaceObj *obj)
{
#define bullet ((Bullet *)obj)
    dbgAssert(bullet->objtype == OBJ_BulletType);
    listAddNode(list,&bullet->bulletlink,obj);
#undef bullet
}

void ResourceListAddCB(LinkedList *list,SpaceObj *obj)
{
#define resource ((Resource *)obj)
    dbgAssert(resource->objtype >= OBJ_AsteroidType && resource->objtype <= OBJ_DustType);
    listAddNode(list,&resource->resourcelink,obj);
#undef resource
}

void DerelictListAddCB(LinkedList *list,SpaceObj *obj)
{
#define derelict ((Derelict *)obj)
    listAddNode(list,&derelict->derelictlink,obj);
#undef derelict
}

void ImpactableListAddCB(LinkedList *list,SpaceObj *obj)
{
#define impactable ((SpaceObjRotImp *)obj)
    listAddNode(list,&impactable->impactablelink,obj);
#undef impactable
}

void MissileListAddCB(LinkedList *list,SpaceObj *obj)
{
#define missile ((Missile *)obj)
    listAddNode(list,&missile->missilelink,obj);
#undef missile
}

void MineAddToMineListCB(LinkedList *list,SpaceObj *obj)
{
#define mine ((Missile *)obj)
    dbgAssert(mine->objtype == OBJ_MissileType);
    listAddNode(list,&mine->formationLink,obj);
#undef mine
}

void SlaveAddToSlaveListCB(LinkedList *list,SpaceObj *obj)
{
#define ship ((Ship *)obj)
    dbgAssert(ship->objtype == OBJ_ShipType);
    listAddNode(list,&ship->slavelink,obj);
#undef ship
}

typedef struct SaveSelSpaceObj {
    sdword num;
    sdword ID[1];
} SaveSelSpaceObj;

#define sizeofSaveSelSpaceObj(n) (sizeof(SaveSelSpaceObj) + (n-1)*sizeof(sdword))


void SaveSelection(SpaceObjSelection *selection)
{
    sdword num = selection->numSpaceObjs;
    SaveChunk *chunk;
    SaveSelSpaceObj *savecontents;
    sdword i;

    if (num < 0 || num > COMMAND_MAX_SHIPS)
    {
        dbgFatalf(DBG_Loc, "SaveSelection: numSpaceObjs=%d out of range (ptr=%p)", num, (void *)selection);
    }

    chunk = CreateChunk(SELECTION,sizeofSaveSelSpaceObj(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    for (i=0;i<num;i++)
    {
        dbgAssert(selection->SpaceObjPtr[i] != NULL);
        savecontents->ID[i] = SpaceObjRegistryGetID(selection->SpaceObjPtr[i]);
        dbgAssert(savecontents->ID[i] != -1);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveMaxSelection(MaxSelection *maxselection)
{
    SaveSelection((SpaceObjSelection *)maxselection);
}

void SaveGrowSelection(GrowSelection *grow)
{
    SaveSelection((SpaceObjSelection *)grow->selection);
}

static void SaveSelectionAllowNullEntries(SpaceObjSelection *selection)
{
    sdword num = selection->numSpaceObjs;
    SaveChunk *chunk;
    SaveSelSpaceObj *savecontents;
    sdword i;

    if (num < 0 || num > COMMAND_MAX_SHIPS)
    {
        dbgFatalf(DBG_Loc, "SaveSelectionAllowNullEntries: numSpaceObjs=%d out of range (ptr=%p)", num, (void *)selection);
    }

    chunk = CreateChunk(SELECTION, sizeofSaveSelSpaceObj(num), NULL);
    savecontents = chunkContents(chunk);
    savecontents->num = num;

    for (i = 0; i < num; i++)
    {
        savecontents->ID[i] = SpaceObjRegistryGetID(selection->SpaceObjPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

SpaceObjSelection *LoadSelection(void)
{
    SpaceObjSelection *selection;
    SaveChunk *chunk;
    SaveSelSpaceObj *loadcontents;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    loadcontents = (struct SaveSelSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    selection = memAlloc(sizeofSelectCommand(num),"loadselect",0);
    selection->numSpaceObjs = num;

    for (i=0;i<num;i++)
    {
        dbgAssert(loadcontents->ID[i] != -1);
        selection->SpaceObjPtr[i] = loadcontents->ID[i];
    }

    memFree(chunk);

    return selection;
}

void LoadGrowSelection(GrowSelection *grow)
{
    grow->selection = (SelectCommand *)LoadSelection();
    grow->maxNumShips = grow->selection->numShips;
}

void FixSelection(SpaceObjSelection *selection)
{
    sdword num = selection->numSpaceObjs;
    sdword i;

    for (i=0;i<num;i++)
    {
        if (selection->SpaceObjPtr[i] == (SpaceObj *)-1)
        {
            selection->SpaceObjPtr[i] = NULL;
        }
        else
        {
            selection->SpaceObjPtr[i] = (SpaceObj *)SpaceObjRegistry.selection->ShipPtr[(sdword)selection->SpaceObjPtr[i]];
            dbgAssert(selection->SpaceObjPtr[i]);
        }
    }
}

void FixGrowSelection(GrowSelection *grow)
{
    FixSelection((SpaceObjSelection *)grow->selection);
}

SpaceObjSelection *LoadSelectionAndFix(void)
{
    SpaceObjSelection *selection;
    SaveChunk *chunk;
    SaveSelSpaceObj *loadcontents;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    loadcontents = (SaveSelSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    dbgAssert(num >= 0);

    selection = memAlloc(sizeofSelectCommand(num),"loadselect",0);
    selection->numSpaceObjs = num;

    for (i=0;i<num;i++)
    {
        if (loadcontents->ID[i] == -1)
        {
            selection->SpaceObjPtr[i] = NULL;
        }
        else
        {
            selection->SpaceObjPtr[i] = SpaceObjRegistryGetObj(loadcontents->ID[i]);
            dbgAssert(selection->SpaceObjPtr[i]);
        }
    }

    memFree(chunk);

    return selection;
}

void LoadMaxSelectionAndFix(MaxSelection *maxselection)
{
    SelectCommand *selection = (SelectCommand *)LoadSelectionAndFix();
    memcpy(maxselection,selection,sizeofSelectCommand(selection->numShips));
    memFree(selection);
}

void LoadMaxSelection(MaxSelection *maxselection)
{
    SelectCommand *selection = (SelectCommand *)LoadSelection();
    if (selection->numShips < 0 || selection->numShips > COMMAND_MAX_SHIPS)
    {
        dbgFatalf(DBG_Loc, "LoadMaxSelection: numShips=%d out of range", selection->numShips);
    }
    memcpy(maxselection, selection, sizeofSelectCommand(selection->numShips));
    memFree(selection);
}

void LoadGrowSelectionAndFix(GrowSelection *grow)
{
    grow->selection = (SelectCommand *)LoadSelectionAndFix();
    grow->maxNumShips = grow->selection->numShips;
}

void SaveLinkedListOfStuff(LinkedList *list,SaveStuffInLinkedListCB savestuffCB)
{
    Node *node = list->head;

    SaveInfoNumber(list->num);

    while (node != NULL)
    {
        savestuffCB(listGetStructOfNode(node));

        node = node->next;
    }
}

void LoadLinkedListOfStuff(LinkedList *list,LoadStuffInLinkedListCB loadstuffCB)
{
    sdword num = LoadInfoNumber();
    sdword i;

    listInit(list);

    for (i=0;i<num;i++)
    {
        loadstuffCB(list);
    }

    dbgAssert(num == list->num);
}

void FixLinkedListOfStuff(LinkedList *list,FixStuffInLinkedListCB fixstuffCB)
{
    Node *node = list->head;

    while (node != NULL)
    {
        fixstuffCB(listGetStructOfNode(node));

        node = node->next;
    }
}

void SaveFormation(FormationCommand *formcommand)
{
    if (formcommand->formationSpecificInfo != NULL)
    {
        SaveStructureWithSize(formcommand->formationSpecificInfo);
    }
}

void LoadFormation(FormationCommand *formcommand)
{
    if (formcommand->formationSpecificInfo != NULL)
    {
        formcommand->formationSpecificInfo = LoadStructureWithSize();
    }
}

void SaveMilitarySlot(MilitarySlot *milslot)
{
    SaveChunk *chunk;
    MilitarySlot *savecontents;
    sdword i;

    chunk = CreateChunk(VARIABLE_STRUCTURE|SAVE_MILITARYSLOT,sizeofMilitarySlot(milslot->numSubslots),milslot);
    savecontents = (MilitarySlot *)chunkContents(chunk);

    for (i=0;i<savecontents->numSubslots;i++)
    {
        savecontents->subslots[i].ship = SpaceObjRegistryGetID((SpaceObj *)savecontents->subslots[i].ship);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveMilitaryParade(MilitaryParadeCommand *militaryParade)
{
    SaveChunk *chunk;
    MilitaryParadeCommand *savecontents;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_MILITARYPARADE,sizeof(MilitaryParadeCommand),militaryParade);
    savecontents = (MilitaryParadeCommand *)chunkContents(chunk);

    savecontents->aroundShip = SpaceObjRegistryGetID((SpaceObj *)militaryParade->aroundShip);

    SaveThisChunk(chunk);
    memFree(chunk);

    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        if (militaryParade->militarySlots[i] != NULL)
        {
            SaveMilitarySlot(militaryParade->militarySlots[i]);
        }
    }
}

MilitarySlot *LoadMilitarySlot(void)
{
    SaveChunk *chunk;
    MilitarySlot *newmilslot;
    sdword i;
    sdword size;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,VARIABLE_STRUCTURE|SAVE_MILITARYSLOT);
    size = sizeofMilitarySlot(((MilitarySlot *)chunkContents(chunk))->numSubslots);

    dbgAssert(size == chunk->contentsSize);

    newmilslot = memAlloc(size,"milslot",0);
    memcpy(newmilslot,chunkContents(chunk),size);

    memFree(chunk);

    for (i=0;i<newmilslot->numSubslots;i++)
    {
        newmilslot->subslots[i].ship = SpaceObjRegistryGetShip((sdword)newmilslot->subslots[i].ship);
    }

    return newmilslot;
}

MilitaryParadeCommand *LoadMilitaryParade(void)
{
    SaveChunk *chunk;
    MilitaryParadeCommand *newmilparade;
    sdword i;

    chunk = LoadNextChunk();
    dbgAssert(
        ((chunk->type == (BASIC_STRUCTURE|SAVE_MILITARYPARADE)) ||
         (chunk->type == (VARIABLE_STRUCTURE|SAVE_MILITARYPARADE))) &&
        (chunk->contentsSize == sizeof(MilitaryParadeCommand)));

    newmilparade = memAlloc(sizeof(MilitaryParadeCommand),"milparadecom",0);
    memcpy(newmilparade,chunkContents(chunk),sizeof(MilitaryParadeCommand));

    memFree(chunk);

    newmilparade->aroundShip = SpaceObjRegistryGetShip((sdword)newmilparade->aroundShip);

    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        if (newmilparade->militarySlots[i] != NULL)
        {
            newmilparade->militarySlots[i] = LoadMilitarySlot();
        }
    }

    return newmilparade;
}

void Save_CommandToDo(CommandToDo *command)
{
    SaveChunk *chunk;
    CommandToDo *savecommand;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_COMMANDTODO,sizeof(CommandToDo),command);
    savecommand = (CommandToDo *)chunkContents(chunk);

    // fixing
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // save
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            savecommand->launchship.receiverShip = (sdword)SpaceObjRegistryGetID((SpaceObj *)command->launchship.receiverShip);
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            savecommand->collect.resource = (sdword)SpaceObjRegistryGetID((SpaceObj *)command->collect.resource);
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            savecommand->buildingship.creator = (sdword)SpaceObjRegistryGetID((SpaceObj *)command->buildingship.creator);
            break;

        case COMMAND_SPECIAL:
            // save
            break;

        case COMMAND_MILITARYPARADE:
            // save
            break;
    }

    SaveThisChunk(chunk);

    dbgAssert(command->selection);
    SaveSelectionAllowNullEntries((SpaceObjSelection *)command->selection);

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        SaveFormation(&command->formation);
    }

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        dbgAssert(command->protect);
        SaveSelectionAllowNullEntries((SpaceObjSelection *)command->protect);
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        dbgAssert(command->ordertype.order != COMMAND_ATTACK);
        dbgAssert(command->attack);
        SaveSelectionAllowNullEntries((SpaceObjSelection *)command->attack);
    }

    // saving
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // save
            dbgAssert((command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) == NULL);
            dbgAssert(command->attack);
            SaveSelectionAllowNullEntries((SpaceObjSelection *)command->attack);
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            break;

        case COMMAND_SPECIAL:
            // save
            if (command->specialtargets != NULL)
            {
                SaveSelectionAllowNullEntries((SpaceObjSelection *)command->specialtargets);
            }
            break;

        case COMMAND_MILITARYPARADE:
            // save
            dbgAssert(command->militaryParade);
            SaveMilitaryParade(command->militaryParade);
            break;
    }

    memFree(chunk);
}

void Load_CommandToDo(LinkedList *list)
{
    SaveChunk *chunk;
    CommandToDo *loadedcommand;
    CommandToDo *command;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_COMMANDTODO,sizeof(CommandToDo));

    loadedcommand = (CommandToDo *)chunkContents(chunk);

    command = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    memcpy(command,loadedcommand,sizeof(CommandToDo));

    memFree(chunk);

    // fixing
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // load
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            command->launchship.receiverShip = SpaceObjRegistryGetShip((sdword)command->launchship.receiverShip);
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            command->collect.resource = SpaceObjRegistryGetResource((sdword)command->collect.resource);
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            command->buildingship.creator = SpaceObjRegistryGetShip((sdword)command->buildingship.creator);
            break;

        case COMMAND_SPECIAL:
            // load
            break;

        case COMMAND_MILITARYPARADE:
            // load
            break;
    }

    command->selection = (SelectCommand *)LoadSelectionAndFix();

    if (command->ordertype.attributes & COMMAND_IS_FORMATION)
    {
        LoadFormation(&command->formation);
    }

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        command->protect = (ProtectCommand *)LoadSelectionAndFix();
    }

    if (command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING)
    {
        dbgAssert(command->ordertype.order != COMMAND_ATTACK);
        command->attack = (AttackCommand *)LoadSelectionAndFix();
    }

    // loading
    switch (command->ordertype.order)
    {
        case COMMAND_MOVE:
            // none needed
            break;

        case COMMAND_ATTACK:
            // load
            dbgAssert((command->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) == NULL);
            command->attack = (AttackCommand *)LoadSelectionAndFix();
            break;

        case COMMAND_DOCK:
            // none needed;
            break;

        case COMMAND_LAUNCHSHIP:
            // fixing
            break;

        case COMMAND_COLLECTRESOURCE:
            // fixing
            break;

        case COMMAND_BUILDINGSHIP:
            // fixing
            break;

        case COMMAND_SPECIAL:
            // load
            if (command->specialtargets != NULL)
            {
                command->specialtargets = (SpecialCommand *)LoadSelectionAndFix();
            }
            break;

        case COMMAND_MILITARYPARADE:
            // load
            dbgAssert(command->militaryParade);
            command->militaryParade = LoadMilitaryParade();
            break;
    }

    listAddNode(list,&command->todonode,command);
}


void Save_CommandLayer(CommandLayer *comlayer)
{
    sdword num = comlayer->todolist.num;
    sdword cur = 0;
    InfoChunkContents infocontents;
    Node *node = comlayer->todolist.head;
    CommandToDo *command;
    SaveChunk *chunk;

    infocontents.info = num;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));

    while (node != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(node);

        Save_CommandToDo(command);
        cur++;

        node = node->next;
    }

    dbgAssert(cur == num);

    memFree(chunk);
}

void Load_CommandLayer(CommandLayer *comlayer)
{
    SaveChunk *infochunk;
    sdword num;
    sdword i;

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    listInit(&comlayer->todolist);
    for (i=0;i<num;i++)
    {
        Load_CommandToDo(&comlayer->todolist);
    }
}

void Save_CameraStackEntry(CameraStackEntry *currententry)
{
    SaveChunk *chunk;
    CameraStackEntry *savecse;
    sdword numShips = currententry->focus.numShips;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_CAMERASTACKENTRY,sizeofCameraStackEntry(numShips),currententry);
    savecse = (CameraStackEntry *)chunkContents(chunk);

    // fix savecse

    savecse->remembercam.playerowner = SavePlayerToPlayerIndex(savecse->remembercam.playerowner);

    for (i=0;i<numShips;i++)
    {
        savecse->focus.ShipPtr[i] = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecse->focus.ShipPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void Load_CameraStackEntry(LinkedList *list)
{
    SaveChunk *chunk;
    CameraStackEntry *loadedcse;
    CameraStackEntry *cse;
    sdword numShips;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_CAMERASTACKENTRY);

    loadedcse = (CameraStackEntry *)chunkContents(chunk);
    numShips = loadedcse->focus.numShips;

    dbgAssert(sizeofCameraStackEntry(numShips) == chunk->contentsSize);

    cse = memAlloc(chunk->contentsSize,"CamStackEntry",NonVolatile);
    memcpy(cse,loadedcse,chunk->contentsSize);

    memFree(chunk);

    // fix cse

    cse->remembercam.playerowner = SavePlayerIndexToPlayer(cse->remembercam.playerowner);

    for (i=0;i<numShips;i++)
    {
        cse->focus.ShipPtr[i] = (Ship *)SpaceObjRegistryGetObj((sdword)cse->focus.ShipPtr[i]);  // don't use SpaceObjRegistryGetShip because it might not actually be a ship
    }

    listAddNode(list,&cse->stacklink,cse);
}

sdword ConvertPointerInListToNum(LinkedList *list,void *entry)
{
    Node *node = list->head;
    sdword cur = -1;

    if (entry == NULL)
    {
        return -1;
    }

    while (node != NULL)
    {
        cur++;
        if (entry == listGetStructOfNode(node))
        {
            return cur;
        }

        node = node->next;
    }

    return -1;
}

void *ConvertNumToPointerInList(LinkedList *list,sdword num)
{
    Node *node;
    sdword cur = 0;

    if (num == -1)
    {
        return NULL;
    }

    dbgAssert((udword)num < list->num);
    node = list->head;

    while (cur < num)
    {
        node = node->next;
        cur++;
    }
    dbgAssert(cur == num);

    return listGetStructOfNode(node);
}

void Save_CameraCommand(CameraCommand *cameracommand)
{
    SaveChunk *chunk;
    SaveCameraCommand *savecc;
    sdword num = cameracommand->camerastack.num;
    sdword cur = 0;
    InfoChunkContents infocontents;
    Node *node = cameracommand->camerastack.head;
    CameraStackEntry *currententry;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_CAMERACOMMAND,sizeof(SaveCameraCommand),NULL);
    savecc = (SaveCameraCommand *)chunkContents(chunk);
    memset(savecc, 0, sizeof(*savecc));
    savecc->camerastack = cameracommand->camerastack;
    savecc->currentCameraStack = cameracommand->currentCameraStack;
    savecc->ViewEntireMissphere = cameracommand->ViewEntireMissphere;
    savecc->actualcamera = cameracommand->actualcamera;
    savecc->transition = cameracommand->transition;
    savecc->action = cameracommand->action;
    savecc->transTimeStamp = cameracommand->transTimeStamp;
    savecc->UserControlled = cameracommand->UserControlled;
    savecc->zoomInCloseAsPossible = cameracommand->zoomInCloseAsPossible;
    savecc->lockCameraTimeStamp = cameracommand->lockCameraTimeStamp;
    savecc->dontUseVelocityPredInChase = cameracommand->dontUseVelocityPredInChase;
    savecc->ccMode = cameracommand->ccMode;
    savecc->dontFocusOnMe = cameracommand->dontFocusOnMe;

    // fix savecc
    savecc->currentCameraStack = ConvertPointerInListToNum(&savecc->camerastack,savecc->currentCameraStack);
    savecc->actualcamera.playerowner = SavePlayerToPlayerIndex(savecc->actualcamera.playerowner);

    savecc->dontFocusOnMe = ConvertPointerInListToNum(&savecc->camerastack,savecc->dontFocusOnMe);

    SaveThisChunk(chunk);

    infocontents.info = num;
    SaveThisChunk(CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));

    while (node != NULL)
    {
        currententry = (CameraStackEntry *)listGetStructOfNode(node);

        Save_CameraStackEntry(currententry);
        cur++;

        node = node->next;
    }

    dbgAssert(cur == num);

    memFree(chunk);
}

void Load_CameraCommand(CameraCommand *cameracommand)
{
    SaveChunk *chunk;
    SaveCameraCommand *loadcc;
    SaveChunk *infochunk;
    sdword num;
    sdword i;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_CAMERACOMMAND,sizeof(SaveCameraCommand));

    loadcc = (SaveCameraCommand *)chunkContents(chunk);
    cameracommand->camerastack = loadcc->camerastack;
    cameracommand->currentCameraStack = loadcc->currentCameraStack;
    cameracommand->ViewEntireMissphere = loadcc->ViewEntireMissphere;
    cameracommand->actualcamera = loadcc->actualcamera;
    cameracommand->transition = loadcc->transition;
    cameracommand->action = loadcc->action;
    cameracommand->transTimeStamp = loadcc->transTimeStamp;
    cameracommand->UserControlled = loadcc->UserControlled;
    cameracommand->zoomInCloseAsPossible = loadcc->zoomInCloseAsPossible;
    cameracommand->lockCameraTimeStamp = loadcc->lockCameraTimeStamp;
    cameracommand->dontUseVelocityPredInChase = loadcc->dontUseVelocityPredInChase;
    cameracommand->ccMode = loadcc->ccMode;
    cameracommand->dontFocusOnMe = loadcc->dontFocusOnMe;

    memFree(chunk);

    infochunk = LoadNextChunk();
    VerifyChunk(infochunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(infochunk))->info;

    memFree(infochunk);

    listInit(&cameracommand->camerastack);
    for (i=0;i<num;i++)
    {
        Load_CameraStackEntry(&cameracommand->camerastack);
    }

    // fix cameracommand

    cameracommand->currentCameraStack = ConvertNumToPointerInList(&cameracommand->camerastack,(sdword)cameracommand->currentCameraStack);
    cameracommand->actualcamera.playerowner = SavePlayerIndexToPlayer(cameracommand->actualcamera.playerowner);

    cameracommand->dontFocusOnMe = ConvertNumToPointerInList(&cameracommand->camerastack,(sdword)cameracommand->dontFocusOnMe);
}

void SaveLinkedListOfInsideShips(LinkedList *list)
{
    sdword num = list->num;
    SaveChunk *chunk;
    SaveLLSpaceObj *savecontents;
    Node *node = list->head;
    Ship *ship;
    sdword cur = 0;

    chunk = CreateChunk(LINKEDLISTSPACEOBJS,sizeofSaveLLSpaceObj(num),NULL);

    savecontents = chunkContents(chunk);
    savecontents->num = num;

    while (node != NULL)
    {
        ship = ((InsideShip *)listGetStructOfNode(node))->ship;
        dbgAssert(ship->objtype == OBJ_ShipType);

        savecontents->ID[cur] = SpaceObjRegistryGetID((SpaceObj *)ship);
        cur++;

        node = node->next;
    }

    dbgAssert(cur == num);

    SaveThisChunk(chunk);

    memFree(chunk);
}

void LoadLinkedListOfInsideShips(LinkedList *list)
{
    SaveChunk *chunk;
    SaveLLSpaceObj *loadcontents;
    sdword num;
    sdword cur = 0;
    InsideShip *insideShip;

    chunk = LoadNextChunk();
    loadcontents = (struct SaveLLSpaceObj *)chunkContents(chunk);
    num = loadcontents->num;

    listInit(list);

    while (cur < num)
    {
        insideShip = memAlloc(sizeof(InsideShip),"InsideShip",0);
        insideShip->ship = loadcontents->ID[cur++];
        if (insideShip->ship != -1)
        {
            listAddNode(list,&insideShip->node,insideShip);
        }
    }

    memFree(chunk);
}

void FixLinkedListOfInsideShips(LinkedList *list)
{
    Node *node = list->head;
    InsideShip *insideShip;

    while (node != NULL)
    {
        insideShip = (InsideShip *)listGetStructOfNode(node);
        insideShip->ship = SpaceObjRegistryGetShip((sdword)insideShip->ship);
        dbgAssert(insideShip->ship != NULL);
        dbgAssert(insideShip->ship->objtype == OBJ_ShipType);
        node = node->next;
    }
}

void SaveLinkedListOfResearchTopics(LinkedList *list)
{
    InfoChunkContents infocontents;
    SaveChunk *chunk;
    Node *node = list->head;
    sdword cur = 0;

    infocontents.info = list->num;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    memFree(chunk);

    while (node != NULL)
    {
        cur++;
        chunk = CreateChunk(BASIC_STRUCTURE|SAVE_RESEARCHTOPIC,sizeof(ResearchTopic),listGetStructOfNode(node));
        SaveThisChunk(chunk);
        memFree(chunk);

        node = node->next;
    }

    dbgAssert(cur == list->num);
}

void LoadLinkedListOfResearchTopics(LinkedList *list)
{
    SaveChunk *chunk;
    sdword num;
    sdword i;
    ResearchTopic *newtopic;

    listInit(list);

    chunk = LoadNextChunk();
    VerifyChunk(chunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(chunk))->info;

    memFree(chunk);

    for (i=0;i<num;i++)
    {
        chunk = LoadNextChunk();
        VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_RESEARCHTOPIC,sizeof(ResearchTopic));
        newtopic = (ResearchTopic *)memAlloc(sizeof(ResearchTopic),"ResearchTopic",NonVolatile);
        memcpy(newtopic,chunkContents(chunk),sizeof(ResearchTopic));
        listAddNode(list, &newtopic->link, newtopic);
        memFree(chunk);
    }
}

void Prefix_PlayerResearchInfo(Player *player)
{
    sdword i;
    LinkedList *topicList = rmPlayerResearchTopicList(player);
    sdword labCount = rmPlayerResearchLabCount(player);

    for (i=0;i<labCount;i++)
    {
        ResearchLab *lab = rmPlayerResearchLab(player, i);
        if (lab->labstatus == LS_RESEARCHITEM)
        {
            lab->topic = ConvertPointerInListToNum(topicList, lab->topic);
            dbgAssert(lab->topic != (ResearchTopic *)-1);
        }
        else
        {
            lab->topic = (ResearchTopic *)-1;
        }
    }
}

void Save_PlayerResearchInfo(Player *player)
{
    SaveLinkedListOfResearchTopics(rmPlayerResearchTopicList(player));
}

void Load_PlayerResearchInfo(Player *player,PlayerResearchInfo *researchinfo)
{
    sdword i;
    LinkedList *topicList;
    sdword labCount;
    (void)researchinfo;

    rmInitializeResearchStatics(player);

    topicList = rmPlayerResearchTopicList(player);
    LoadLinkedListOfResearchTopics(topicList);
    labCount = rmPlayerResearchLabCount(player);

    for (i=0;i<labCount;i++)
    {
        ResearchLab *lab = rmPlayerResearchLab(player, i);
        if (lab->labstatus == LS_RESEARCHITEM)
        {
            lab->topic = ConvertNumToPointerInList(topicList, (sdword)lab->topic);
            dbgAssert(lab->topic != NULL);
        }
        else
        {
            lab->topic = NULL;
        }
    }

    rmUpdateBuildCapabilities(player);
}

void SaveMineFormationInfo(MineFormationInfo *mineFormationInfo)
{
    SaveChunk *chunk;
    MineFormationInfo *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_MINEFORMATIONINFO,sizeof(MineFormationInfo),mineFormationInfo);
    savecontents = (MineFormationInfo *)chunkContents(chunk);

    savecontents->playerowner = SavePlayerToPlayerIndex(savecontents->playerowner);
    savecontents->effect = NULL;

    SaveThisChunk(chunk);
    memFree(chunk);

    SaveLinkedListOfSpaceObjs(&mineFormationInfo->MineList);
}

MineFormationInfo *LoadMineFormationInfo(void)
{
    SaveChunk *chunk;
    MineFormationInfo *mineFormationInfo;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_MINEFORMATIONINFO,sizeof(MineFormationInfo));

    mineFormationInfo = memAlloc(sizeof(MineFormationInfo),"MineFormationInfo",NonVolatile);
    memcpy(mineFormationInfo,chunkContents(chunk),sizeof(MineFormationInfo));
    memFree(chunk);

    mineFormationInfo->playerowner = SavePlayerIndexToPlayer(mineFormationInfo->playerowner);

    LoadLinkedListOfSpaceObjs(&mineFormationInfo->MineList,MineAddToMineListCB);

    return mineFormationInfo;
}

void SaveMineFormationList(LinkedList *list)
{
    InfoChunkContents infocontents;
    SaveChunk *chunk;
    Node *node = list->head;
    sdword cur = 0;
    MineFormationInfo *mineFormationInfo;

    infocontents.info = list->num;

    SaveThisChunk(chunk = CreateChunk(INFO_CHUNK,sizeof(InfoChunkContents),&infocontents));
    memFree(chunk);

    while (node != NULL)
    {
        cur++;
        mineFormationInfo = (MineFormationInfo *)listGetStructOfNode(node);
        SaveMineFormationInfo(mineFormationInfo);
        node = node->next;
    }

    dbgAssert(cur == list->num);
}

void LoadMineFormationList(LinkedList *list)
{
    SaveChunk *chunk;
    sdword num;
    sdword i;
    MineFormationInfo *mineFormationInfo;

    listInit(list);

    chunk = LoadNextChunk();
    VerifyChunk(chunk,INFO_CHUNK,sizeof(InfoChunkContents));

    num = ((InfoChunkContents *)chunkContents(chunk))->info;

    memFree(chunk);

    for (i=0;i<num;i++)
    {
        mineFormationInfo = LoadMineFormationInfo();
        listAddNode(list, &mineFormationInfo->FormLink, mineFormationInfo);
    }
}

void SaveRetreatAtom(RetreatAtom *retreat)
{
    SaveChunk *chunk;
    RetreatAtom *savecontents;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_RETREATATOM,sizeof(RetreatAtom),retreat);
    savecontents = (RetreatAtom *)chunkContents(chunk);

    savecontents->retreater = SpaceObjRegistryGetID((SpaceObj *)savecontents->retreater);
    savecontents->fleeingfrom = ConvertPointerInListToNum(&universe.mainCommandLayer.todolist,savecontents->fleeingfrom);

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveRetreatList(LinkedList *list)
{
    sdword num = list->num;
    sdword cur = 0;
    Node *node = list->head;

    SaveInfoNumber(num);

    while (node != NULL)
    {
        cur++;
        SaveRetreatAtom((RetreatAtom *)listGetStructOfNode(node));

        node = node->next;
    }

    dbgAssert(cur == num);
}

RetreatAtom *LoadRetreatAtom()
{
    SaveChunk *chunk;
    RetreatAtom *retreat;

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_RETREATATOM,sizeof(RetreatAtom));
    retreat = (RetreatAtom *)memAlloc(sizeof(RetreatAtom),"Retreat Data", NonVolatile);
    memcpy(retreat,chunkContents(chunk),sizeof(RetreatAtom));
    memFree(chunk);

    retreat->retreater = SpaceObjRegistryGetShip((sdword)retreat->retreater);
    retreat->fleeingfrom = ConvertNumToPointerInList(&universe.mainCommandLayer.todolist,(sdword)retreat->fleeingfrom);

    return retreat;
}

void LoadRetreatList(LinkedList *list)
{
    sdword num;
    sdword i;
    RetreatAtom *retreat;

    num = LoadInfoNumber();

    listInit(list);

    for (i=0;i<num;i++)
    {
        retreat = LoadRetreatAtom();
        listAddNode(list,&retreat->retreatLink,retreat);
    }
}

void SaveAttackAtom(AttackAtom *attackAtom)
{
    SaveChunk *chunk;
    AttackAtom *savecontents;
    sdword size;
    sdword i;
    sdword numShips;

    size = sizeof(AttackAtom) + sizeof(ShipPtr)*(attackAtom->attackerList.numShips-1);

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_ATTACKATOM,size,attackAtom);
    savecontents = (AttackAtom *)chunkContents(chunk);

    savecontents->retreater = SpaceObjRegistryGetID((SpaceObj *)savecontents->retreater);

    numShips = savecontents->attackerList.numShips;
    for (i=0;i<numShips;i++)
    {
        savecontents->attackerList.ShipPtr[i] = SpaceObjRegistryGetID((SpaceObj *)savecontents->attackerList.ShipPtr[i]);
    }

    SaveThisChunk(chunk);
    memFree(chunk);
}

void SaveAttackMemory(LinkedList *list)
{
    sdword num = list->num;
    sdword cur = 0;
    Node *node = list->head;

    SaveInfoNumber(num);

    while (node != NULL)
    {
        cur++;
        SaveAttackAtom((AttackAtom *)listGetStructOfNode(node));

        node = node->next;
    }

    dbgAssert(cur == num);
}

AttackAtom *LoadAttackAtom()
{
    SaveChunk *chunk;
    AttackAtom *attackAtom;
    AttackAtom *loadedAttackAtom;
    sdword size;
    sdword i;
    sdword numShips;

    chunk = LoadNextChunk();
    VerifyChunkNoSize(chunk,BASIC_STRUCTURE|SAVE_ATTACKATOM);
    loadedAttackAtom = (AttackAtom *)chunkContents(chunk);

    numShips = loadedAttackAtom->attackerList.numShips;
    size = sizeof(AttackAtom) + sizeof(ShipPtr)*(numShips-1);

    dbgAssert(size == chunk->contentsSize);

    attackAtom = (AttackAtom *)memAlloc(size,"tacticsattackmemory",NonVolatile);
    memcpy(attackAtom,loadedAttackAtom,size);

    attackAtom->retreater = SpaceObjRegistryGetShip((sdword)attackAtom->retreater);

    for (i=0;i<numShips;i++)
    {
        attackAtom->attackerList.ShipPtr[i] = SpaceObjRegistryGetShip((sdword)attackAtom->attackerList.ShipPtr[i]);
    }

    memFree(chunk);
    return attackAtom;
}

void LoadAttackMemory(LinkedList *list)
{
    sdword num;
    sdword i;
    AttackAtom *attack;

    num = LoadInfoNumber();

    listInit(list);

    for (i=0;i<num;i++)
    {
        attack = LoadAttackAtom();
        listAddNode(list,&attack->attackLink,attack);
    }
}

void SaveResourceVolumeCB(void *stuff)
{
    SaveStructureOfSize(stuff,sizeof(ResourceVolume));
}

void LoadResourceVolumeCB(LinkedList *list)
{
    ResourceVolume *resvol = LoadStructureOfSize(sizeof(ResourceVolume));

    listAddNode(list,&resvol->link,resvol);
}

void SaveUniverse()
{
    SaveChunk *chunk;
    Universe savecontents;
    sdword i;

    chunk = CreateChunk(BASIC_STRUCTURE|SAVE_UNIVERSE,CATA_SAVE_UNIVERSE_SERIALIZED_SIZE,NULL);
    memcpy(&savecontents, &universe, sizeof(savecontents));

    savecontents.curPlayerPtr = SavePlayerToPlayerIndex(universe.curPlayerPtr);

    for (i=0;i<universe.numPlayers;i++)
    {
        savecontents.players[i].PlayerMothership = (sdword)SpaceObjRegistryGetID((SpaceObj *)universe.players[i].PlayerMothership);
        Prefix_PlayerResearchInfo(&savecontents.players[i]);
    }

    for (i=0;i<UNIV_NUMBER_WORLDS;i++)
    {
        savecontents.world[i] = (sdword)SpaceObjRegistryGetID((SpaceObj *)savecontents.world[i]);
    }

    SaveShipNamePointers();
    SaveGameCopyUniverseRuntimeToSerialized(chunkContents(chunk), &savecontents);
    SaveThisChunk(chunk);

    Save_CommandLayer(&universe.mainCommandLayer);
    Save_CameraCommand(&universe.mainCameraCommand);

    SaveLinkedListOfSpaceObjs(&universe.RenderList);
    SaveLinkedListOfSpaceObjs(&universe.MinorRenderList);

    SaveLinkedListOfSpaceObjs(&universe.SpaceObjList);
    SaveLinkedListOfSpaceObjs(&universe.MinorSpaceObjList);
    SaveLinkedListOfSpaceObjs(&universe.ShipList);
    SaveLinkedListOfSpaceObjs(&universe.BulletList);
    SaveLinkedListOfSpaceObjs(&universe.ResourceList);
    SaveLinkedListOfSpaceObjs(&universe.DerelictList);
    SaveLinkedListOfSpaceObjs(&universe.ImpactableList);
    SaveLinkedListOfSpaceObjs(&universe.MissileList);
    SaveMineFormationList(&universe.MineFormationList);

    univDeleteDeadShips();
    univDeleteDeadResources();
    univDeleteDeadDerelicts();
    univDeleteDeadMissiles();

    SaveRetreatList(&universe.RetreatList);
    SaveShockwaves();
    SaveShipNames();

    SaveLinkedListOfStuff(&universe.ResourceVolumeList,SaveResourceVolumeCB);

    SaveGrowSelection(&universe.HousekeepShipList);
    SaveGrowSelection(&universe.unk1);

    for (i=0;i<universe.numPlayers;i++)
    {
        Save_PlayerResearchInfo(&universe.players[i]);
        fogSaveState(&universe.players[i]);
    }

    univSaveCountTables();
    SaveInfoNumber(overlayPauseActive);

    memFree(chunk);
}

void FillOutCollBlobList(LinkedList *list)
{
    sdword numBlobs = BlobRegistry.selection->numShips;
    sdword i;
    blob *tblob;

    listInit(list);
    for (i=0;i<numBlobs;i++)
    {
        tblob = (blob *)BlobRegistry.selection->ShipPtr[i];
        listAddNode(list,&tblob->node,tblob);
    }
}

void LoadUniverse()
{
    SaveChunk *chunk;
    sdword i;

    if (universe.star3dinfo)
    {
        star3dClose(universe.star3dinfo);
        universe.star3dinfo = NULL;
    }

    chunk = LoadNextChunk();
    VerifyChunk(chunk,BASIC_STRUCTURE|SAVE_UNIVERSE,CATA_SAVE_UNIVERSE_SERIALIZED_SIZE);

    SaveGameCopyUniverseSerializedToRuntime(&universe, chunkContents(chunk));

    memFree(chunk);

    Load_CommandLayer(&universe.mainCommandLayer);
    Load_CameraCommand(&universe.mainCameraCommand);

    LoadLinkedListOfSpaceObjs(&universe.RenderList,RenderListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.MinorRenderList,MinorRenderListAddCB);

    LoadLinkedListOfSpaceObjs(&universe.SpaceObjList,SpaceObjListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.MinorSpaceObjList,MinorSpaceObjListAddCB);
    listInit(&universe.effectList);

    LoadLinkedListOfSpaceObjs(&universe.ShipList,ShipListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.BulletList,BulletListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.ResourceList,ResourceListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.DerelictList,DerelictListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.ImpactableList,ImpactableListAddCB);
    LoadLinkedListOfSpaceObjs(&universe.MissileList,MissileListAddCB);
    LoadMineFormationList(&universe.MineFormationList);

    listInit(&universe.DeleteMissileList);
    listInit(&universe.DeleteResourceList);
    listInit(&universe.DeleteDerelictList);
    listInit(&universe.DeleteShipList);

    FillOutCollBlobList(&universe.collBlobList);

    LoadRetreatList(&universe.RetreatList);
    LoadShockwaves();
    LoadShipNames();

    LoadLinkedListOfStuff(&universe.ResourceVolumeList,LoadResourceVolumeCB);

    LoadGrowSelectionAndFix(&universe.HousekeepShipList);
    LoadGrowSelectionAndFix(&universe.unk1);

    universe.star3dinfo = star3dInit(NUM_STARS, universe.radius - 1.0f, universe.radius);

    universe.curPlayerPtr = SavePlayerIndexToPlayer(universe.curPlayerPtr);


    for (i=0;i<universe.numPlayers;i++)
    {
        universe.players[i].PlayerMothership = SpaceObjRegistryGetShip((sdword)universe.players[i].PlayerMothership);
        Load_PlayerResearchInfo(&universe.players[i],&universe.players[i].researchinfo);
        fogLoadState(&universe.players[i]);
    }

    memset(&universe.players[MAX_MULTIPLAYER_PLAYERS],0,sizeof(Player));
    universe.players[MAX_MULTIPLAYER_PLAYERS].playerIndex = MAX_MULTIPLAYER_PLAYERS;

    for (i=0;i<UNIV_NUMBER_WORLDS;i++)
    {
        universe.world[i] = (Derelict *)SpaceObjRegistryGetObj((sdword)universe.world[i]);
    }
    univLoadCountTables();

    overlayPauseActive = LoadInfoNumber();
}

#pragma warning( 2 : 4047)      // turn back on "different levels of indirection warning"



sdword SaveShipNamePointers(void)
{
   sdword i;

   for (i = 0; i < shipNameReservationCount; i++)
   {
       shipNameReservationStorage[i].unk04 = SpaceObjRegistryGetID((SpaceObj *)shipNameReservationStorage[i].ship);
   }

   return shipNameReservationCount;
}

void SaveShipNames(void)
{
   SaveInfoNumber(shipNameReservationCount);
   SaveStructureOfSize(shipNameReservationStorage, sizeof(shipNameReservationStorage));
}

void LoadShipNames(void)
{
   shipNameReservationCount = LoadInfoNumber();
   LoadStructureOfSizeToAddress(shipNameReservationStorage, sizeof(shipNameReservationStorage));
   shipNameReservations = shipNameReservationStorage;
}

void FixShipNamePointers(void)
{
   sdword i;

   for (i = 0; i < shipNameReservationCount; i++)
   {
       shipNameReservationStorage[i].ship = (udword)SpaceObjRegistryGetShip(shipNameReservationStorage[i].unk04);
   }
}
