/*=============================================================================
    Name    : stubs.c
    Purpose : Temporary stubs for unimplemented functions.
              These allow the project to link while modules are being decompiled.
              Each stub should be removed as the real implementation is added.
=============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "debug.h"
#include "file.h"
#include "memory.h"
#include "MultiplayerGame.h"
#include "MultiplayerLANGame.h"
#include "subtitle.h"
#include "mainswitches.h"
#include "TitanInterfaceC.h"
#include "camera.h"
#include "CommandLayer.h"
#include "DefenseFighter.h"
#include "etg.h"
#include "fastmath.h"
#include "glinc.h"
#include "gun.h"
#include "shipselect.h"
#include "universe.h"

/*=============================================================================
    Memory stubs
=============================================================================*/

#ifdef MEM_USE_STDLIB
// Stub out memory stats variables/functions when using stdlib allocator
#if MEM_STATISTICS
char memStatString[256] = {0};
bool32 memStatsLogging = FALSE;
taskhandle memStatsTaskHandle = 0;
void memStatsTaskFunction(void) {}
memcookiename memStatsCookieNames[MS_NumberCookieNames] = {0};
#endif
void memAnalysisCreate(void) {}
#endif

/*=============================================================================
    Misc globals with no identified owner yet
=============================================================================*/

ubyte byte_88F454 = 0;

void telldir(void) {}

/*=============================================================================
    Ship header stubs - CustShipHeader for each ship type not yet reconciled
=============================================================================*/
#include "spaceobj.h"

CustShipHeader CloakGeneratorHeader = {0};
CustShipHeader DDDFrigateHeader = {0};
CustShipHeader DefenseFighterHeader = {0};
CustShipHeader DroneHeader = {0};
CustShipHeader LightCorvetteHeader = {0};
CustShipHeader P2AdvanceSwarmerHeader = {0};
CustShipHeader P2FuelPodHeader = {0};
CustShipHeader P2MothershipHeader = {0};
CustShipHeader P2SwarmerHeader = {0};
CustShipHeader P3StandardShipHeader = {0};
CustShipHeader ProbeHeader = {0};
CustShipHeader ResearchShipHeader = {0};
CustShipHeader ResourceControllerHeader = {0};
CustShipHeader SensorArrayHeader = {0};

/*=============================================================================
    Function stubs for ship types not yet reconciled
=============================================================================*/

void CloakGeneratorJustDisabled(Ship *ship) { (void)ship; }
void DDDFrigateDockAllDronesInstantly(Ship *ship) { (void)ship; }
void DDDFrigateJustDisabled(Ship *ship) { (void)ship; }
void DDDFrigateMakeReadyForHyperspace(Ship *ship) { (void)ship; }
void DDDFrigateMakeSureItCanGuard(Ship *ship) { (void)ship; }
void DDDFrigateSwitchSides(Ship *dddf, sdword player) { (void)dddf; (void)player; }
bool32 isThereAnotherTargetForMe(Ship *ship, SelectAnyCommand *targets) { (void)ship; (void)targets; return FALSE; }
void DefenseFighterBulletRemoval(Bullet *bullettogoByeBye) { (void)bullettogoByeBye; }
void DefenseFighterReportBullet(Ship *ship, Bullet *bullet) { (void)ship; (void)bullet; }
void defenseFighterAdjustLaser(Bullet *laser)
{
    DefenseFighterSpec *spec;
    Node *bulletnode;
    DefenseStruct *defensestruct;
    Ship *ship;
    vector tempvec;
    GunStatic *gunstatic;
    Gun *gun;
    vector positionInWorldCoordSys;

    if (laser->owner == NULL)
    {
        return;
    }

    ship = laser->owner;
    gun = &ship->cataclysmGunInfo->guns[0];
    gunstatic = gun->gunstatic;
    spec = (DefenseFighterSpec *)ship->ShipSpecifics;
    bulletnode = spec->DefenseList.head;

    while (bulletnode != NULL)
    {
        defensestruct = (DefenseStruct *)listGetStructOfNode(bulletnode);
        if (defensestruct->LaserDead)
        {
            bulletnode = bulletnode->next;
            continue;
        }
        if (laser == defensestruct->laser)
        {
            dbgAssert(defensestruct->bullet != NULL);
            matMultiplyMatByVec(&positionInWorldCoordSys, &ship->rotinfo.coordsys, &gunstatic->position);
            vecAdd(laser->posinfo.position, positionInWorldCoordSys, ship->posinfo.position);
            vecSub(laser->lengthvec, defensestruct->bullet->posinfo.position, laser->posinfo.position);

            tempvec = defensestruct->laser->lengthvec;
            vecNormalize(&tempvec);

            defensestruct->laser->bulletheading = tempvec;
            matCreateCoordSysFromHeading(&defensestruct->laser->rotinfo.coordsys, &tempvec);
            if (defensestruct->laser->effect != NULL)
            {
                ((real32 *)defensestruct->laser->effect->variable)[ETG_LengthVariable] =
                    fsqrt(vecMagnitudeSquared(defensestruct->laser->lengthvec));
            }
            return;
        }
        bulletnode = bulletnode->next;
    }
}
void dockDrawSalvageInfo(SpaceObjRotImpTargGuidanceShipDerelict *obj) { (void)obj; }
int aviGetSamples(void *pBuf, long *pNumSamples, long nBufSize) { (void)pBuf; (void)pNumSamples; (void)nBufSize; return 0; }

/*=============================================================================
    Animatic/anim stubs (animatic.c not yet in build)
=============================================================================*/

#define NUM_SP_MISSIONS 19
typedef struct
{
    char filename[32];
} animlst;

static animlst animlisting[NUM_SP_MISSIONS];
static sdword subY1;

sdword animaticJustPlayed = 0;

bool32 animBinkPlay(sdword a, sdword b) { (void)a; (void)b; return FALSE; }

static void animSubtitlesSetup(bool32 on)
{
    static GLint matrixMode;
    static GLfloat projection[16];

    if (on)
    {
        glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
        glGetFloatv(GL_PROJECTION_MATRIX, projection);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
    }
    else
    {
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projection);

        glMatrixMode(matrixMode);
    }
}

void animSubtitlesDraw(void)
{
    sdword index;

    animSubtitlesSetup(TRUE);

    subY1 = 0;

    for (index = 0; index < SUB_NumberRegions; index++)
    {
        if (subRegion[index].bEnabled && subRegion[index].cardIndex > 0)
        {
            subTimeElapsed = &universe.totaltimeelapsed;
            subTitlesDraw(&subRegion[index]);
            if (subRegion[index].rect.y1 > subY1)
            {
                subY1 = subRegion[index].rect.y1;
            }
        }
    }

    animSubtitlesSetup(FALSE);
}

void animShutdown(void)
{
    memset(animlisting, 0, NUM_SP_MISSIONS * sizeof(animlst));
}

void animStartup(void)
{
    filehandle lst;
    char line[128], temp[64];
    sdword level;

    memset(animlisting, 0, NUM_SP_MISSIONS * sizeof(animlst));

    if (!fileExists("Movies\\animatics.lst", 0))
    {
        return;
    }

    lst = fileOpen("Movies\\animatics.lst", FF_TextMode);
    while (fileLineRead(lst, line, 127) != FR_EndOfFile)
    {
        if (strlen(line) < 5)
        {
            continue;
        }
        if (line[0] == ';' || line[0] == '/')
        {
            continue;
        }
        sscanf(line, "%d %s", &level, temp);
        dbgAssert(level >= 0 && level < NUM_SP_MISSIONS);
        memStrncpy(animlisting[level].filename, temp, 31);
    }
    fileClose(lst);
}

bool32 binkInit(sdword rgltype) { (void)rgltype; return TRUE; }
bool32 binkCleanup(void) { return TRUE; }

/*=============================================================================
    KAS helper predicates / selection wrappers (parity with cataclysm.exe)
    These are referenced by Init_Mission05 and other Cataclysm-only mission
    scripts. Their bodies match IDA exactly.
=============================================================================*/

#include "Ship.h"
#include "KASFunc.h"
#include "universe.h"

sdword shipGetSlotStatus(Ship *ship, sdword slotIndex);

sdword kasShipMatchesMissionVolumeOccupantFilter(Ship *ship, sdword arg1, sdword arg2)
{
    ShipType shiptype;
    (void)arg2;
    if (arg1)
    {
        if (ship->playerowner->playerIndex)
        {
            if ((ship->specialFlags4 & 0x2000000) == 0)
            {
                shiptype = ship->shiptype;
                if (shiptype != sSwarmer && shiptype != dCrystal0 && shiptype != dCrystal1)
                    return 1;
            }
        }
    }
    else if (!ship->playerowner->playerIndex
          && shipGetSlotStatus(ship, 1)
          && !shipIsModuleSlotActive(ship, universe.curPlayerPtr)
          && (ship->specialFlags4 & 0x2000000) == 0
          && ship->shiptype != sSwarmer)
    {
        return 1;
    }
    return 0;
}

sdword kasfShipsSelectMissionVolumeEnemyCombatShips(int *result, int source)
{
    return kasfShipsSelectByPredicate((GrowSelection *)result, (GrowSelection *)source,
                                      kasShipMatchesMissionVolumeOccupantFilter, 1, 0);
}

sdword kasfShipsSelectMissionVolumeInactivePlayerModules(int *result, int source)
{
    return kasfShipsSelectByPredicate((GrowSelection *)result, (GrowSelection *)source,
                                      kasShipMatchesMissionVolumeOccupantFilter, 0, 0);
}

// PlugScreen not being reconstructed; leave stubs
void psModeBegin(char *directory, udword modeFlags) { (void)directory; (void)modeFlags; }
void psScreenStart(char *name) { (void)name; }

/*=============================================================================
    Titan network stubs (TitanInterface not yet wired)
=============================================================================*/

unsigned long titanStart(unsigned long isLan, unsigned long isIP) { (void)isLan; (void)isIP; return 0; }
unsigned long titanCheckCanNetwork(unsigned long isLan, unsigned long isIP) { (void)isLan; (void)isIP; return 0; }
void titanLeaveGameNotify(void) {}
void titanShutdown(void) {}
void titanRefreshRequest(char *theDir) { (void)theDir; }
unsigned long titanReadyToStartGame(unsigned char *routingaddress) { (void)routingaddress; return 0; }
unsigned long titanBehindFirewall(void) { return 0; }
void titanCreateGame(wchar_t *str, DirectoryCustomInfo *myInfo) { (void)str; (void)myInfo; }
void titanSendPacketTo(Address *address, unsigned char titanMsgType, const void *thePacket, unsigned short theLen) { (void)address; (void)titanMsgType; (void)thePacket; (void)theLen; }
void titanBroadcastPacket(unsigned char titanMsgType, const void *thePacket, unsigned short theLen) { (void)titanMsgType; (void)thePacket; (void)theLen; }
void titanSendLanBroadcast(const void *thePacket, unsigned short theLen) { (void)thePacket; (void)theLen; }
void titanConnectToClient(Address *address) { (void)address; }
void titanSendPing(Address *address, unsigned int pingsizebytes) { (void)address; (void)pingsizebytes; }
void titanSetGameKey(unsigned char *key) { (void)key; }
int titanGetPatch(char *filename, char *saveFileName) { (void)filename; (void)saveFileName; return 0; }
void titanReplaceGameInfo(wchar_t *str, DirectoryCustomInfo *myInfo, unsigned long replaceTimeout) { (void)str; (void)myInfo; (void)replaceTimeout; }
void BroadcastChatMessage(unsigned short size, const void *chatData) { (void)size; (void)chatData; }
void SendPrivateChatMessage(unsigned long *userIDList, unsigned short numUsersInList, unsigned short size, const void *chatData) { (void)userIDList; (void)numUsersInList; (void)size; (void)chatData; }
void authAuthenticate(char *loginName, char *password) { (void)loginName; (void)password; }
void authCreateUser(char *loginName, char *password) { (void)loginName; (void)password; }
void authChangePassword(char *loginName, char *oldpassword, char *newpassword) { (void)loginName; (void)oldpassword; (void)newpassword; }
void titanWaitShutdown(void) {}
void titanConnectingCancelHit(void) {}
void chatClose(void) {}
int titanSaveWonstuff(void) { return 0; }
void titanPumpEngine(void) {}
void titanRemoveGame(wchar_t *str) { (void)str; }
int titanStartChatServer(wchar_t *password) { (void)password; return 0; }
void chatConnect(wchar_t *password) { (void)password; }
void titanStartShutdown(unsigned long titanMsgType, const void *thePacket, unsigned short theLen) { (void)titanMsgType; (void)thePacket; (void)theLen; }
void titanAnyoneSendPacketTo(Address *address, unsigned char titanMsgType, const void *thePacket, unsigned short theLen) { (void)address; (void)titanMsgType; (void)thePacket; (void)theLen; }
void titanAnyoneBroadcastPacket(unsigned char titanMsgType, const void *thePacket, unsigned short theLen) { (void)titanMsgType; (void)thePacket; (void)theLen; }
const unsigned char *titanGetGameKey(void) { return NULL; }
Address titanGetMyPingAddress(void) { Address addr = {0}; return addr; }
