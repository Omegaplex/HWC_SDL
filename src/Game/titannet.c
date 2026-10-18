/*=============================================================================
    Name    : titannet.c
    Purpose : All of the logic for the titan game interface stuff.

    Created 6/23/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <io.h>
#include "titannet.h"
#include "types.h"
#include "debug.h"
#include "statscript.h"
#include "TitanInterfaceC.h"
#include "CommandNetwork.h"
#include "multiplayergame.h"
#include "multiplayerlangame.h"
#include "utility.h"
#include "teams.h"
#include "scenpick.h"
#include "colpick.h"
#include "Captaincy.h"
#include "color.h"
#include "file.h"
#include "titan.h"

#define DEBUG_GAME_BW

/*=============================================================================
    Data:
=============================================================================*/

unsigned long TitanActive = FALSE;
unsigned long TitanReadyToShutdown = FALSE;

TPChannelList tpChannelList;
TPServerList  tpServerList;
CaptainGameInfo tpGameCreated;
wchar_t CurrentChannel[MAX_CHANNEL_NAME_LEN] = L"";
wchar_t CurrentChannelDescription[MAX_CHANNEL_DESCRIPTION_LEN] = L"";
unsigned long myIP;
unsigned long myPort;

int ChannelProtectedFlag = 0;
wchar_t ChannelPassword[MAX_PASSWORD_LEN] = L"";

wchar_t RemoveGameRequest[MAX_TITAN_GAME_NAME_LEN] = L"";

long CONNECT_TIMEOUT = 8000;

#define VALIDVERSIONS_MAXLEN 2000

bool32 HaveValidVersions = FALSE;
char ValidVersions[VALIDVERSIONS_MAXLEN] = "";

/*=============================================================================
    Tweakables:
=============================================================================*/

real32 TITAN_PICKER_REFRESH_TIME = 1.0f;
unsigned short TITAN_GAME_EXPIRE_TIME = 3600;
unsigned long TITAN_CHANNEL_EXPIRE_TIME = 300;


unsigned long DIRSERVER_NUM = 1;
unsigned long AUTHSERVER_NUM = 0;       // get from dirserver now
unsigned long PATCHSERVER_NUM = 1;

unsigned long DIRSERVER_PORTS[MAX_PORTS];
unsigned long AUTHSERVER_PORTS[MAX_PORTS];
unsigned long PATCHSERVER_PORTS[MAX_PORTS];

ipString AUTHSERVER_IPSTRINGS[MAX_IPS];
ipString DIRSERVER_IPSTRINGS[MAX_IPS];
ipString PATCHSERVER_IPSTRINGS[MAX_IPS];

unsigned long GAME_PORT = 2500;
unsigned long AD_PORT = 2501;

char PATCHNAME[50] = "HomeworldPatch.exe";

char ROUTING_SERVER_NAME[30] = "routingserv";

real32 LAN_ADVERTISE_USER_TIME = 1.0f;
real32 LAN_ADVERTISE_USER_TIMEOUT = 5.0f;
real32 LAN_ADVERTISE_GAME_TIME = 1.0f;
real32 LAN_ADVERTISE_GAME_TIMEOUT = 5.0f;

real32 KEEPALIVE_SEND_IAMALIVE_TIME = 10.0f;
real32 KEEPALIVE_IAMALIVE_TIMEOUT = 25.0f;
real32 KEEPALIVE_SEND_ALIVESTATUS_TIME = 30.0f;

color PATCHBARCOLOR = colRGB(255, 0, 0);
color PATCHBAROUTLINECOLOR = colWhite;

udword PRINTLAG_IFGREATERTHAN =        10;
udword PRINTLAG_MINFRAMES =            20;

udword ROOM_MIN_THRESHOLD = 1;
udword ROOM_MAX_THRESHOLD = 50;

unsigned long WAIT_SHUTDOWN_MS = 1000;

void scriptSetIPStrings(char *directory,char *field,void *dataToFillIn);
void scriptSetPortNumbers(char *directory,char *field,void *dataToFillIn);

extern sdword TimedOutWaitingForPauseAcksGiveUpAfterNumTimes;

extern real32 HorseRacePlayerDropoutTime;
extern color HorseRaceDropoutColor;

scriptEntry NetTweaks[] =
{
    makeEntry(PATCHBARCOLOR,scriptSetRGBCB),
    makeEntry(PATCHBAROUTLINECOLOR,scriptSetRGBCB),
    makeEntry(TITAN_PICKER_REFRESH_TIME,scriptSetReal32CB),
    makeEntry(TITAN_GAME_EXPIRE_TIME,scriptSetUwordCB),
    makeEntry(TITAN_CHANNEL_EXPIRE_TIME,scriptSetUdwordCB),
    makeEntry(DIRSERVER_NUM, scriptSetUdwordCB),
    makeEntry(PATCHSERVER_NUM, scriptSetUdwordCB),
    makeEntry(DIRSERVER_PORTS,scriptSetPortNumbers),
    makeEntry(PATCHSERVER_PORTS,scriptSetPortNumbers),
    makeEntry(DIRSERVER_IPSTRINGS,scriptSetIPStrings),
    makeEntry(PATCHSERVER_IPSTRINGS,scriptSetIPStrings),
    makeEntry(GAME_PORT,scriptSetUdwordCB),
    makeEntry(T1_Timeout,scriptSetReal32CB),
    makeEntry(T2_Timeout,scriptSetReal32CB),
    makeEntry(TWAITFORPAUSEACKS_Timeout,scriptSetReal32CB),
    makeEntry(TimedOutWaitingForPauseAcksGiveUpAfterNumTimes,scriptSetSdwordCB),
    makeEntry(HorseRacePlayerDropoutTime,scriptSetReal32CB),
    makeEntry(HorseRaceDropoutColor,scriptSetRGBCB),
    makeEntry(ROUTING_SERVER_NAME,scriptSetStringCB),
    makeEntry(PATCHNAME,scriptSetStringCB),
    makeEntry(LAN_ADVERTISE_USER_TIME,scriptSetReal32CB),
    makeEntry(LAN_ADVERTISE_USER_TIMEOUT,scriptSetReal32CB),
    makeEntry(LAN_ADVERTISE_GAME_TIME,scriptSetReal32CB),
    makeEntry(LAN_ADVERTISE_GAME_TIMEOUT,scriptSetReal32CB),
    makeEntry(AD_PORT,scriptSetUdwordCB),
    makeEntry(CONNECT_TIMEOUT,scriptSetSdwordCB),
    makeEntry(KEEPALIVE_SEND_IAMALIVE_TIME,scriptSetReal32CB),
    makeEntry(KEEPALIVE_IAMALIVE_TIMEOUT,scriptSetReal32CB),
    makeEntry(KEEPALIVE_SEND_ALIVESTATUS_TIME,scriptSetReal32CB),
    makeEntry(PRINTLAG_MINFRAMES,scriptSetUdwordCB),
    makeEntry(PRINTLAG_IFGREATERTHAN,scriptSetUdwordCB),
    makeEntry(ROOM_MIN_THRESHOLD,scriptSetUdwordCB),
    makeEntry(ROOM_MAX_THRESHOLD,scriptSetUdwordCB),
    makeEntry(WAIT_SHUTDOWN_MS,scriptSetUdwordCB),
    endEntry
};

/*=============================================================================
    Functions:
=============================================================================*/

void scriptSetPortNumbers(char *directory,char *field,void *dataToFillIn)
{
    char *param;
    sdword i;
    udword *ptr = (udword *)dataToFillIn;

    for (i=0;i<MAX_PORTS;i++)
    {
        param = strtok(field, ", \t");
        field = NULL;
        if (param)
        {
            sscanf(param,"%d",&ptr[i]);
        }
        else
        {
            break;
        }
    }
}

void scriptSetIPStrings(char *directory,char *field,void *dataToFillIn)
{
    char *param;
    sdword i;
    ipString *ipStrings = (ipString *)dataToFillIn;

    for (i=0;i<MAX_IPS;i++)
    {
        param = strtok(field, ", \t");
        field = NULL;
        if (param)
        {
            sscanf(param,"%s",ipStrings[i]);
        }
        else
        {
            break;
        }
    }
}

void ResetChannel(void)
{
    CurrentChannel[0] = 0;
    CurrentChannelDescription[0] = 0;
}

void SetChannel(wchar_t *channel, wchar_t *description)
{
    wcscpy(CurrentChannel,channel);
    wcscpy(CurrentChannelDescription,description);
}

wchar_t *GetCurrentChannel(void)
{
    return CurrentChannel;
}

wchar_t *GetCurrentChannelDescription(void)
{
    return CurrentChannelDescription;
}

/*-----------------------------------------------------------------------------
    Name        : tpLockChannelList
    Description : Locks the tpChannelList for exclusive access
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpLockChannelList()
{
    DWORD result = WaitForSingleObject((HANDLE)tpChannelList.mutex,INFINITE);
    dbgAssert(result != WAIT_FAILED);
}

/*-----------------------------------------------------------------------------
    Name        : tpUnLockChannelList
    Description : Unlocks the tpChannelList, so other tasks can access it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpUnLockChannelList()
{
    bool32 result = ReleaseMutex((HANDLE)tpChannelList.mutex);
    dbgAssert(result);
}

/*-----------------------------------------------------------------------------
    Name        : tpLockServerList
    Description : Locks the tpServerList for exclusive access
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpLockServerList()
{
    DWORD result = WaitForSingleObject((HANDLE)tpServerList.mutex,INFINITE);
    dbgAssert(result != WAIT_FAILED);
}

/*-----------------------------------------------------------------------------
    Name        : tpUnLockServerList
    Description : Unlocks the tpServerList, so other tasks can access it
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void tpUnLockServerList()
{
    bool32 result = ReleaseMutex((HANDLE)tpServerList.mutex);
    dbgAssert(result);
}

/*-----------------------------------------------------------------------------
    Name        : titanGameStartup
    Description : initializes the titan game networking interface variables.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanGameStartup(void)
{
    tpChannelList.numChannels = 0;
    tpChannelList.newDataArrived = FALSE;
    tpChannelList.mutex = (void *)CreateMutex(NULL,FALSE,NULL);
    dbgAssert(tpChannelList.mutex != NULL);
    tpServerList.numServers = 0;
    tpServerList.newDataArrived = FALSE;
    tpServerList.mutex = (void *)CreateMutex(NULL,FALSE,NULL);
    dbgAssert(tpServerList.mutex != NULL);

    titanResetGameCreated();
}

/*-----------------------------------------------------------------------------
    Name        : titanResetGameCreated
    Description : resets the tpGameCreated Structures
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanResetGameCreated(void)
{
    mgResetNamePassword();
    tpGameCreated.MapName[0]        = 0;
    tpGameCreated.DisplayMapName[0] = 0;
    tpGameCreated.numPlayers        = 1;
    tpGameCreated.userBehindFirewall = 0;
}

/*-----------------------------------------------------------------------------
    Name        : titanGameShutdown
    Description : shuts down the titat game interface code and data.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanGameShutdown(void)
{
    CloseHandle((HANDLE)tpChannelList.mutex);
    CloseHandle((HANDLE)tpServerList.mutex);
    tpChannelList.mutex = NULL;
    tpServerList.mutex = NULL;
}

/*-----------------------------------------------------------------------------
    Name        : titanGameEnded
    Description : Function to be called when titan game ended
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void titanGameEnded(void)
{
    signed int IWasCaptain = IAmCaptain;

    mgGameInterestedOff();
    titanLeaveGameNotify();

    if (IWasCaptain)
    {
        if (!LANGame)
        {
            wcscpy(RemoveGameRequest,tpGameCreated.Name);   // don't call titanRemoveGame directly because you must be connected
                                                            // to the room before you call it
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : titanGameDisolved
    Description : Disolves a game created by a captain.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanGameDisolved(bool32 shutdown)
{
    if (shutdown)
    {
        if (!LANGame)
        {
            titanRemoveGame(tpGameCreated.Name);
        }

        titanStartShutdown(TITANMSGTYPE_GAMEDISOLVED, NULL, 0);
    }
    else
    {
        if (!LANGame)
        {
            titanRemoveGame(tpGameCreated.Name);
        }

        titanBroadcastPacket(TITANMSGTYPE_GAMEDISOLVED, NULL, 0);
    }

    mgGameInterestedOff();
    titanLeaveGameNotify();
}

/*-----------------------------------------------------------------------------
    Name        : titanLeaveGame
    Description : Sends a leave game request to the captain.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanLeaveGame(int shutdown)
{
    if (shutdown)
    {
        titanStartShutdown(TITANMSGTYPE_LEAVEGAMEREQUEST,NULL,0);
    }
    else
    {
        titanSendPacketTo(&tpGameCreated.playerInfo[0].address,TITANMSGTYPE_LEAVEGAMEREQUEST,NULL,0);
    }

    mgGameInterestedOff();
    titanLeaveGameNotify();
}

/*-----------------------------------------------------------------------------
    Name        : titanNoClientsCB
    Description : Called when all quees are closed.
    Inputs      : none
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void titanNoClientsCB(void)
{
    TitanReadyToShutdown = TRUE;
}

unsigned long titanConfirmReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
    dbgAssert(bloblen == 0);

    if (LANGame)
        lgJoinGameConfirmed();
    else
        mgJoinGameConfirmed();

    return TRUE;
}

unsigned long titanRejectReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
    dbgAssert(bloblen == 0);

    if (LANGame)
        lgJoinGameDenied();
    else
        mgJoinGameDenied();

    return TRUE;
}

void titanUpdateGameDataCB(const void *blob,unsigned short bloblen)
{
    dbgAssert(bloblen == sizeof(CaptainGameInfo));

    mgBackuptpGameCreated();
    memcpy(&tpGameCreated,blob,bloblen);

    if (LANGame)
        lgUpdateGameInfo();
    else
        mgUpdateGameInfo();
}

bool32 titanKickPlayer(udword i)
{
    unsigned long j;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;

    if (!GameCreator)
    {
        return FALSE;
    }

    if (i < tpGameCreated.numPlayers)
    {
        titanSendPacketTo(&(tpGameCreated.playerInfo[i].address), TITANMSGTYPE_KICKPLAYER, NULL, 0);

        if (i+1>=tpGameCreated.numPlayers)
        {
            tpGameCreated.numPlayers--;
        }
        else
        {
            tpGameCreated.numPlayers--;
            for (j=i;j<tpGameCreated.numPlayers;j++)
            {
                tpGameCreated.playerInfo[j] = tpGameCreated.playerInfo[j+1];
            }
        }

        if (!LANGame)
        {
            generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
            titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);
        }
        else
        {
            generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
            wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        }

        sigsPlayerIndex = 0;
        sigsNumPlayers = tpGameCreated.numPlayers;

        titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

        if (LANGame)
            lgUpdateGameInfo();
        else
            mgUpdateGameInfo();

        return TRUE;

    }
    else
    {
        return FALSE;
    }
}

unsigned long titanLeaveGameReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
    unsigned long i,j;
    bool32       found=FALSE;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;

    if (!GameCreator)
    {
        return FALSE;
    }

    dbgAssert(bloblen == 0);

    // search for the player who has left the game
    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,*address))
        {
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        if (i+1>=tpGameCreated.numPlayers)
        {
            tpGameCreated.numPlayers--;
        }
        else
        {
            tpGameCreated.numPlayers--;
            for (j=i;j<tpGameCreated.numPlayers;j++)
            {
                tpGameCreated.playerInfo[j] = tpGameCreated.playerInfo[j+1];
            }
        }

        if (!LANGame)
        {
            generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
            titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);
        }
        else
        {
            generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
            wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        }

        sigsPlayerIndex = 0;
        sigsNumPlayers = tpGameCreated.numPlayers;

        titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

        if (LANGame)
            lgUpdateGameInfo();
        else
            mgUpdateGameInfo();

        return TRUE;
    }

    return FALSE;
}

void generateDirectoryCustomInfo(DirectoryCustomInfoMax *buildDirectoryCustomInfo)
{
    sdword passwordnamelen;
    sdword mapnamelen = strlen(tpGameCreated.DisplayMapName) + 1;
    int n;

    dbgAssert(mapnamelen <= MAX_MAPNAME_LEN);

    if (bitTest(tpGameCreated.flag,MG_PasswordProtected))
    {
        wcscpy(buildDirectoryCustomInfo->stringdata,tpGameCreated.Password);
        passwordnamelen = wcslen(tpGameCreated.Password) + 1;
    }
    else
    {
        wcscpy(buildDirectoryCustomInfo->stringdata,L"");
        passwordnamelen = wcslen(L"") + 1;
    }

    dbgAssert(passwordnamelen <= MAX_PASSWORD_LEN);

    mbstowcs(buildDirectoryCustomInfo->stringdata + passwordnamelen, tpGameCreated.DisplayMapName, mapnamelen);

    buildDirectoryCustomInfo->stringdatalength = (mapnamelen + passwordnamelen) * 2;

    dbgAssert(tpGameCreated.numPlayers > 0);

    buildDirectoryCustomInfo->userBehindFirewall = 0;
    for(n=0; n<tpGameCreated.numPlayers; n++) {
        if(tpGameCreated.playerInfo[n].behindFirewall) {
            buildDirectoryCustomInfo->userBehindFirewall = 1;
            break;
        }
    }

    tpGameCreated.userBehindFirewall = buildDirectoryCustomInfo->userBehindFirewall;

    buildDirectoryCustomInfo->numPlayers = tpGameCreated.numPlayers;
    buildDirectoryCustomInfo->flag = tpGameCreated.flag;

    buildDirectoryCustomInfo->captainAddress = myAddress;
    buildDirectoryCustomInfo->pingAddress = titanGetMyPingAddress();
    memcpy(buildDirectoryCustomInfo->sessionKey,titanGetGameKey(),GAMEKEY_SIZE);
    memcpy(buildDirectoryCustomInfo->versionInfo,networkVersion,MAX_NETWORKVERSION_STRING_LEN);
}

signed long titanRequestReceivedCB(Address *address,const void *blob,unsigned short bloblen)
{
    sdword i;
    DirectoryCustomInfoMax buildDirectoryCustomInfo;
    const PlayerJoinInfo* pInfo;

    if (AddressesAreEqual(*address,myAddress))
    {
        return REQUEST_RECV_CB_JUSTDENY; // can't join your own game
    }

    // reject if got multiple join requests by the same person
    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,*address))
        {
            return REQUEST_RECV_CB_JUSTDENY;       // already received this one, so return
        }
    }

    if ((tpGameCreated.numPlayers >= MAX_MULTIPLAYER_PLAYERS) || (tpGameCreated.flag & GAME_IN_PROGRESS))
    {
        return REQUEST_RECV_CB_DENYANDCLOSE;
    }

    if ((mgPlayerLimit) && (tpGameCreated.numPlayers >= mgPlayerLimit))
    {
        return REQUEST_RECV_CB_DENYANDCLOSE;
    }

    if (tpGameCreated.numPlayers + tpGameCreated.numComputers >= spScenarios[spCurrentSelected].maxplayers)
    {
        if (tpGameCreated.numPlayers >= spScenarios[spCurrentSelected].maxplayers)
        {
            return REQUEST_RECV_CB_DENYANDCLOSE; // too many people for this map
        }
        else
        {
            // let's bump computers out for humans to join
            tpGameCreated.numComputers = spScenarios[spCurrentSelected].maxplayers - tpGameCreated.numPlayers - 1;
            dbgAssert(tpGameCreated.numComputers >= 0);
            dbgAssert(tpGameCreated.numPlayers + tpGameCreated.numComputers < spScenarios[spCurrentSelected].maxplayers);
        }
    }

    pInfo = (PlayerJoinInfo*)blob;

    if (mgIsUserBanned((char *)pInfo->PersonalName))
    {
        return REQUEST_RECV_CB_JUSTDENY;
    }

    // update directory server
    dbgAssert(bloblen == sizeof(PlayerJoinInfo));

    tpGameCreated.playerInfo[tpGameCreated.numPlayers].address = *address;
    tpGameCreated.playerInfo[tpGameCreated.numPlayers].playerIndex = tpGameCreated.numPlayers;
    tpGameCreated.playerInfo[tpGameCreated.numPlayers].colorsPicked = pInfo->colorsPicked;
    if (!pInfo->colorsPicked)
    {
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].baseColor = teColorSchemes[tpGameCreated.numPlayers].textureColor.base;
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].stripeColor = teColorSchemes[tpGameCreated.numPlayers].textureColor.detail;
    }
    else
    {
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].baseColor = pInfo->baseColor;
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].stripeColor = pInfo->stripeColor;
    }
    tpGameCreated.playerInfo[tpGameCreated.numPlayers].race = pInfo->race;
        tpGameCreated.playerInfo[tpGameCreated.numPlayers].behindFirewall = pInfo->behindFirewall;
    strcpy(tpGameCreated.playerInfo[tpGameCreated.numPlayers].PersonalName, pInfo->PersonalName);
    tpGameCreated.numPlayers++;

    if (!LANGame)
    {
        generateDirectoryCustomInfo(&buildDirectoryCustomInfo);
        titanReplaceGameInfo(tpGameCreated.Name,(DirectoryCustomInfo *)&buildDirectoryCustomInfo, FALSE);
        mgUpdateGameInfo();
    }
    else
    {
        generateDirectoryCustomInfo(&lgMyGameInfo.directoryCustomInfo);
        wcscpy(lgMyGameInfo.Name,tpGameCreated.Name);
        lgUpdateGameInfo();
    }

    return REQUEST_RECV_CB_ACCEPT;
}

void titanGameMsgReceivedCB(const void *blob,unsigned short bloblen)
{
    ReceivedPacketCB((ubyte *)blob,(udword)bloblen);
}

void titanSendBroadcastMessage(ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: broadcast gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif







    titanBroadcastPacket(TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);     // Fix later for non-captain broadcasting stuff
}

void titanSendPointMessage(int playerIndex,ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: pointmsg gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanSendPacketTo(&tpGameCreated.playerInfo[playerIndex].address,TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);
}

void titanAnyoneSendBroadcastMessage(ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: broadcastany gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanAnyoneBroadcastPacket(TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);     // Fix later for non-captain broadcasting stuff
}

void titanAnyoneSendPointMessage(int playerIndex,ubyte *packet,udword sizeofPacket)
{
#ifdef DEBUG_GAME_BW
    titanDebug("bw: pointmsgany gamepkt %x size %d",((HWPacketHeader *)packet)->type,sizeofPacket);
#endif
    titanAnyoneSendPacketTo(&tpGameCreated.playerInfo[playerIndex].address,TITANMSGTYPE_GAME,packet,(uword)sizeofPacket);
}

void captainGameStartCB(void)
{
    sigsPlayerIndex = 0;
    sigsNumPlayers = tpGameCreated.numPlayers;
    sigsPressedStartGame = TRUE;

    if (LANGame)
        lgShutdownMultiPlayerGameScreens();
    else
        mgShutdownMultiPlayerGameScreens();
}

void titanUpdatePlayer(bool32 captain)
{
    PlayerJoinInfo pInfo;

    if (captain)
    {
        tpGameCreated.playerInfo[0].baseColor = utyBaseColor;
        tpGameCreated.playerInfo[0].stripeColor = utyStripeColor;
        tpGameCreated.playerInfo[0].colorsPicked = cpColorsPicked;
        tpGameCreated.playerInfo[0].race = whichRaceSelected;

        titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

        if (LANGame)
            lgUpdateGameInfo();
        else
            mgUpdateGameInfo();
    }
    else
    {
        pInfo.baseColor = utyBaseColor;
        pInfo.stripeColor = utyStripeColor;
        pInfo.colorsPicked = cpColorsPicked;
        pInfo.race = whichRaceSelected;

        titanSendPacketTo(&tpGameCreated.playerInfo[0].address,TITANMSGTYPE_UPDATEPLAYER,&pInfo,sizeof(pInfo));
    }
}

void titanUpdatePlayerCB(Address *address, const void *blob, unsigned short bloblen)
{
    udword i;
    PlayerJoinInfo *pinfo=(PlayerJoinInfo *)blob;

    dbgAssert(bloblen == sizeof(PlayerJoinInfo));

    for (i=0;i<tpGameCreated.numPlayers;i++)
    {
        if (AddressesAreEqual(tpGameCreated.playerInfo[i].address,*address))
        {
            tpGameCreated.playerInfo[i].baseColor   = pinfo->baseColor;
            tpGameCreated.playerInfo[i].stripeColor = pinfo->stripeColor;
            tpGameCreated.playerInfo[i].colorsPicked = pinfo->colorsPicked;
            tpGameCreated.playerInfo[i].race        = pinfo->race;

            titanBroadcastPacket(TITANMSGTYPE_UPDATEGAMEDATA, &tpGameCreated, sizeof(tpGameCreated));

            if (LANGame)
                lgUpdateGameInfo();
            else
                mgUpdateGameInfo();
        }
    }
}

void PrintAddressToString(char *buffer,Address *address)
{
    if (!IPGame)
    {
        sprintf(buffer,"(Ether:%x,%x,%x,%x,%x,%x Port:%x)",address->AddrPart.etherAddr[0],address->AddrPart.etherAddr[1],
                address->AddrPart.etherAddr[2],address->AddrPart.etherAddr[3],address->AddrPart.etherAddr[4],address->AddrPart.etherAddr[5],
                address->Port);
    }
    else
    {
        sprintf(buffer,"(IP:%x Port:%d)",address->AddrPart.IP,address->Port);
    }
}

int titanLoadPublicKey(char *filename,unsigned char** buffer)
{
    sdword fileLength = fileSizeGet(filename, 0);
    (*buffer) = (unsigned char*)malloc(fileLength);
    return fileLoad(filename, *buffer, 0);
}

void titanResetValidVersions()
{
    HaveValidVersions = FALSE;
    ValidVersions[0] = 0;
}

/*-----------------------------------------------------------------------------
    Name        : titanGotValidVersionStrings
    Description : callback for when titan gets valid version strings (one string, multiple version strings seperated by tabs)
    Inputs      : validversions
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void titanGotValidVersionStrings(char *vversions)
{
    strcpy(ValidVersions,vversions);
    HaveValidVersions = TRUE;
}

bool32 VersionFoundInValidVersions(char *myversion)
{
    char copyValidVersions[VALIDVERSIONS_MAXLEN];
    char *strtokptr;
    char *ptr;

    strcpy(copyValidVersions,ValidVersions);        // have to make a copy since strtok converts delimiters to NULLs
    strtokptr = copyValidVersions;

    for (;;)
    {
        ptr = strtok(strtokptr, "\r\n");      // only use "\n", "\t" is contained in the version strings to sepearte language, etc.
        strtokptr = NULL;
        if (!ptr)
            return FALSE;

        if (stricmp(ptr,myversion) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

bool32 titanCheckValidVersion(char *myversion)
{
    if (ShortCircuitWON) return(TRUE);

    if (!HaveValidVersions)
    {
        return TRUE;            // assume we have valid version if we don't have any information yet
    }

    if (VersionFoundInValidVersions(myversion))
    {
        return TRUE;
    }

    return FALSE;
}

bool32 CheckNetworkVersionCompatibility(char *netversion)
{
    if (stricmp(netversion,networkVersion) == 0)
        return TRUE;
    else
        return FALSE;
}

#if 0
void titanCDKeyNotFoundCB(void)
{
    mgDisplayMessage(strGetString(strNoCDKey));
    return;
}

void titanInvalidCDKeyCB(void)
{
    mgDisplayMessage(strGetString(strLightweightBadKey));
    return;
}
#endif



















#if 0  // Decompiled pseudocode reference
/*=== titanGetMyPingAddress (0x58A870) ===*/
Address __cdecl titanGetMyPingAddress(Address *result)
{
  int v1; // esi
  int v3; // ecx
  int v4; // edx
  int v5; // [esp+Ch] [ebp-8h]
  int v6; // [esp+10h] [ebp-4h]

  if ( *(_BYTE *)(*(_DWORD *)titanInterface + 59) )
  {
    v1 = *(_DWORD *)(*(_DWORD *)titanInterface + 2584);
    LOWORD(v6) = ntohs(*(_WORD *)(*(_DWORD *)titanInterface + 2582));
    *result = v1;
    result[1] = v5;
    result[2] = v6;
  }
  else
  {
    v3 = *(_DWORD *)(*(_DWORD *)titanInterface + 848);
    v4 = *(_DWORD *)(*(_DWORD *)titanInterface + 852);
    *result = *(_DWORD *)(*(_DWORD *)titanInterface + 844);
    result[1] = v3;
    result[2] = v4;
  }
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanCreateGame (0x58ADA0) ===*/
void __cdecl titanCreateGame(wchar_t *str, int myInfo)
{
  sub_593EF0(*(char **)titanInterface, str, myInfo);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanConnectToClient (0x58AE00) ===*/
void __cdecl titanConnectToClient(int address)
{
  sub_594A40(*(int *)titanInterface, address);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanSendLanBroadcast (0x58AE70) ===*/
void __cdecl titanSendLanBroadcast(int thePacket, int theLen)
{
  sub_595E30(*(int *)titanInterface, thePacket, theLen);
}
#endif


#if 0  // Decompiled pseudocode reference
/*=== titanAnyoneSendPacketTo (0x58AEE0) ===*/
void __cdecl titanAnyoneSendPacketTo(int *address, char titanMsgType, int thePacket, int theLen)
{
  sub_594A40(*(int *)titanInterface, (int)address);
  sub_595F10(*(_BYTE **)titanInterface, address, titanMsgType, thePacket, theLen, 0, 0);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanAnyoneBroadcastPacket (0x58AF20) ===*/
void __cdecl titanAnyoneBroadcastPacket(char titanMsgType, int thePacket, int theLen)
{
  int v3; // eax
  int *i; // esi
  int v5; // ecx
  int v6; // [esp+0h] [ebp-4h]

  v6 = 0;
  if ( HIWORD(tpGameCreated_flag) )
  {
    v3 = dword_C31F10;
    for ( i = (int *)&dword_8860F8; ; i += 15 )
    {
      v5 = *i;
      if ( !dword_886010 )
        break;
      if ( v5 != v3 )
        goto LABEL_8;
LABEL_9:
      if ( ++v6 >= HIWORD(tpGameCreated_flag) )
        return;
    }
    if ( v5 == v3 && *((_WORD *)i + 2) == (_WORD)dword_C31F14 )
      goto LABEL_9;
LABEL_8:
    sub_594A40(*(int *)titanInterface, (int)i);
    sub_595F10(*(_BYTE **)titanInterface, i, titanMsgType, thePacket, theLen, 0, 0);
    v3 = dword_C31F10;
    goto LABEL_9;
  }
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== chatClose (0x58B320) ===*/
void chatClose()
{
  if ( *(_DWORD *)titanInterface )
    sub_59B400(*(char **)titanInterface, 0);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanSetGameKey (0x58B390) ===*/
int __cdecl titanSetGameKey(int key)
{
  return (*(int (__thiscall **)(int, int, int))(*(_DWORD *)(*(_DWORD *)titanInterface + 2708) + 12))(
           *(_DWORD *)titanInterface + 2708,
           8,
           key);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== authAuthenticate (0x58B440) ===*/
void __cdecl authAuthenticate(const char *loginName, const char *password)
{
  unsigned int v2; // kr08_4
  unsigned int v3; // ebp
  unsigned int v4; // kr0C_4
  unsigned int v5; // ebp
  char *v6; // ecx
  char v7; // al
  _DWORD *v8; // ecx
  char v9; // al
  int v10; // [esp+10h] [ebp-30h] BYREF
  char *v11; // [esp+14h] [ebp-2Ch]
  unsigned int v12; // [esp+18h] [ebp-28h]
  int v13; // [esp+1Ch] [ebp-24h]
  int v14; // [esp+20h] [ebp-20h] BYREF
  char *v15; // [esp+24h] [ebp-1Ch]
  unsigned int v16; // [esp+28h] [ebp-18h]
  int v17; // [esp+30h] [ebp-10h] BYREF
  int v18; // [esp+34h] [ebp-Ch]
  int v19; // [esp+38h] [ebp-8h]
  int v20; // [esp+3Ch] [ebp-4h]

  LOBYTE(v17) = (_BYTE)password;
  v18 = 0;
  v19 = 0;
  v20 = 0;
  stdStringAssignBytes(&v17, &data, strlen((const char *)&data));
  LOBYTE(v10) = (_BYTE)password;
  stdStringReset(&v10, 0);
  v2 = strlen(password) + 1;
  v3 = v2 - 1;
  if ( stdStringResizeOrReserve(&v10, v2 - 1, 1) )
  {
    qmemcpy(v11, password, v3);
    v12 = v2 - 1;
    v11[v3] = 0;
  }
  LOBYTE(v14) = (_BYTE)password;
  stdStringReset(&v14, 0);
  v4 = strlen(loginName) + 1;
  v5 = v4 - 1;
  if ( stdStringResizeOrReserve(&v14, v4 - 1, 1) )
  {
    qmemcpy(v15, loginName, v5);
    v16 = v4 - 1;
    v15[v5] = 0;
  }
  sub_590340(*(char **)titanInterface, &v14, &v10, &v17, 0);
  stdStringReset(&v14, 1);
  if ( v11 )
  {
    v6 = v11 - 1;
    v7 = *(v11 - 1);
    if ( v7 )
      *v6 = v7 - 1;
    else
      operator_delete(v6);
  }
  v11 = 0;
  v12 = 0;
  v13 = 0;
  if ( v18 )
  {
    v8 = (_DWORD *)(v18 - 1);
    v9 = *(_BYTE *)(v18 - 1);
    if ( v9 )
      *(_BYTE *)v8 = v9 - 1;
    else
      operator_delete(v8);
  }
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== authCreateUser (0x58B5B0) ===*/
void __cdecl authCreateUser(const char *loginName, const char *password)
{
  unsigned int v2; // kr08_4
  unsigned int v3; // ebp
  unsigned int v4; // kr0C_4
  unsigned int v5; // ebp
  char *v6; // ecx
  char v7; // al
  _DWORD *v8; // ecx
  char v9; // al
  int v10; // [esp+10h] [ebp-30h] BYREF
  char *v11; // [esp+14h] [ebp-2Ch]
  unsigned int v12; // [esp+18h] [ebp-28h]
  int v13; // [esp+1Ch] [ebp-24h]
  int v14; // [esp+20h] [ebp-20h] BYREF
  char *v15; // [esp+24h] [ebp-1Ch]
  unsigned int v16; // [esp+28h] [ebp-18h]
  int v17; // [esp+30h] [ebp-10h] BYREF
  int v18; // [esp+34h] [ebp-Ch]
  int v19; // [esp+38h] [ebp-8h]
  int v20; // [esp+3Ch] [ebp-4h]

  LOBYTE(v17) = (_BYTE)password;
  v18 = 0;
  v19 = 0;
  v20 = 0;
  stdStringAssignBytes(&v17, &data, strlen((const char *)&data));
  LOBYTE(v10) = (_BYTE)password;
  stdStringReset(&v10, 0);
  v2 = strlen(password) + 1;
  v3 = v2 - 1;
  if ( stdStringResizeOrReserve(&v10, v2 - 1, 1) )
  {
    qmemcpy(v11, password, v3);
    v12 = v2 - 1;
    v11[v3] = 0;
  }
  LOBYTE(v14) = (_BYTE)password;
  stdStringReset(&v14, 0);
  v4 = strlen(loginName) + 1;
  v5 = v4 - 1;
  if ( stdStringResizeOrReserve(&v14, v4 - 1, 1) )
  {
    qmemcpy(v15, loginName, v5);
    v16 = v4 - 1;
    v15[v5] = 0;
  }
  sub_590340(*(char **)titanInterface, &v14, &v10, &v17, 1);
  stdStringReset(&v14, 1);
  if ( v11 )
  {
    v6 = v11 - 1;
    v7 = *(v11 - 1);
    if ( v7 )
      *v6 = v7 - 1;
    else
      operator_delete(v6);
  }
  v11 = 0;
  v12 = 0;
  v13 = 0;
  if ( v18 )
  {
    v8 = (_DWORD *)(v18 - 1);
    v9 = *(_BYTE *)(v18 - 1);
    if ( v9 )
      *(_BYTE *)v8 = v9 - 1;
    else
      operator_delete(v8);
  }
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== authChangePassword (0x58B720) ===*/
void __cdecl authChangePassword(const char *loginName, const char *oldpassword, const void *newpassword)
{
  unsigned int v3; // kr08_4
  unsigned int v4; // ebp
  unsigned int v5; // kr0C_4
  unsigned int v6; // ebp
  char *v7; // ecx
  char v8; // al
  _DWORD *v9; // ecx
  char v10; // al
  int v11; // [esp+10h] [ebp-30h] BYREF
  char *v12; // [esp+14h] [ebp-2Ch]
  unsigned int v13; // [esp+18h] [ebp-28h]
  int v14; // [esp+1Ch] [ebp-24h]
  int v15; // [esp+20h] [ebp-20h] BYREF
  char *v16; // [esp+24h] [ebp-1Ch]
  unsigned int v17; // [esp+28h] [ebp-18h]
  int v18; // [esp+30h] [ebp-10h] BYREF
  int v19; // [esp+34h] [ebp-Ch]
  int v20; // [esp+38h] [ebp-8h]
  int v21; // [esp+3Ch] [ebp-4h]

  LOBYTE(v18) = (_BYTE)newpassword;
  v19 = 0;
  v20 = 0;
  v21 = 0;
  stdStringAssignBytes(&v18, newpassword, strlen((const char *)newpassword));
  LOBYTE(v11) = (_BYTE)newpassword;
  stdStringReset(&v11, 0);
  v3 = strlen(oldpassword) + 1;
  v4 = v3 - 1;
  if ( stdStringResizeOrReserve(&v11, v3 - 1, 1) )
  {
    qmemcpy(v12, oldpassword, v4);
    v13 = v3 - 1;
    v12[v4] = 0;
  }
  LOBYTE(v15) = (_BYTE)newpassword;
  stdStringReset(&v15, 0);
  v5 = strlen(loginName) + 1;
  v6 = v5 - 1;
  if ( stdStringResizeOrReserve(&v15, v5 - 1, 1) )
  {
    qmemcpy(v16, loginName, v6);
    v17 = v5 - 1;
    v16[v6] = 0;
  }
  sub_590340(*(char **)titanInterface, &v15, &v11, &v18, 0);
  stdStringReset(&v15, 1);
  if ( v12 )
  {
    v7 = v12 - 1;
    v8 = *(v12 - 1);
    if ( v8 )
      *v7 = v8 - 1;
    else
      operator_delete(v7);
  }
  v12 = 0;
  v13 = 0;
  v14 = 0;
  if ( v19 )
  {
    v9 = (_DWORD *)(v19 - 1);
    v10 = *(_BYTE *)(v19 - 1);
    if ( v10 )
      *(_BYTE *)v9 = v10 - 1;
    else
      operator_delete(v9);
  }
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== AuthHandleGetPubKeysReply (0x58eb40) ===*/
void __thiscall AuthHandleGetPubKeysReply(char *this, void *a2)
{
__int16 v3; // di
void *v4; // ecx
int *v5; // ecx
int v6; // eax
int v7; // edx
void (__thiscall ***v8)(_DWORD, int); // ecx
int v9; // [esp+0h] [ebp-4Ch] BYREF
void *v10[4]; // [esp+Ch] [ebp-40h] BYREF
unsigned __int16 family[2]; // [esp+1Ch] [ebp-30h]
char v12[4]; // [esp+30h] [ebp-1Ch]
char *v13; // [esp+38h] [ebp-14h]
int *v14; // [esp+3Ch] [ebp-10h]
int v15; // [esp+48h] [ebp-4h]
v14 = &v9;
v13 = this;
titanDebug(aTitaninterface_10);
EnterCriticalSection((LPCRITICAL_SECTION)(this + 2728));
*((_DWORD *)this + 218) = 0;
LeaveCriticalSection((LPCRITICAL_SECTION)(this + 2728));
if ( !*((_DWORD *)this + 246) )
{
titanDebug(aTitaninterface_11);
return;
}
v15 = 0;
sub_6A7E60(v10, (int)a2);
LOBYTE(v15) = 1;
v3 = *(_WORD *)v12;
if ( *(__int16 *)v12 >= 0 )
{
v4 = operator_new(0x30u);
LOBYTE(v15) = 2;
if ( v4 )
v5 = (int *)Auth1PublicKeyBlock_constructor(v4, v10[3], family[0]);
else
v5 = 0;
v6 = *((_DWORD *)this + 246);
*((_DWORD *)this + 248) = v5;
v7 = *v5;
LOBYTE(v15) = 1;
if ( (*(unsigned __int8 (__stdcall **)(int))(v7 + 8))(v6) )
{
titanDebug(aTitaninterface_13);
*((_DWORD *)this + 269) = 0;
sub_58F1C0(this);
goto LABEL_14;
}
v8 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)this + 248);
if ( v8 )
(**v8)(v8, 1);
*((_DWORD *)this + 248) = 0;
goto LABEL_13;
}
titanDebug("TitanInterface::AuthHandleGetPubKeysReply Status = %d", *(_DWORD *)v12);
if ( !authReceiveReply(v3) )
{
LABEL_13:
AuthFailOver(this);
goto LABEL_14;
}
titanDebug(aTitaninterface_13);
*((_DWORD *)this + 269) = 0;
LABEL_14:
LOBYTE(v15) = 0;
sub_6A7EF0(v10);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== ReadLoginKey (0x58ecd0) ===*/
bool __thiscall ReadLoginKey(_DWORD *this, _DWORD *a2)
{
FILE *v3; // eax
FILE *v4; // edi
size_t v6; // esi
char *v7; // ebx
int v8; // esi
int v9; // ecx
char *v10; // eax
unsigned __int8 v11; // [esp+8h] [ebp-4h]
v3 = fopen(off_8BB988, aRb);
v4 = v3;
if ( !v3 )
return 0;
v6 = (unsigned __int8)fgetc(v3);
v7 = (char *)operator_new(24 * v6);
v11 = fread(v7, 0x18u, v6, v4);
fclose(v4);
v8 = 0;
if ( v11 )
{
v9 = this[270];
v10 = v7;
while ( this[4 * v9 + 404] != *(_DWORD *)v10 || HIWORD(this[4 * v9 + 403]) != *((_WORD *)v10 + 2) )
{
++v8;
v10 += 24;
if ( v8 >= v11 )
goto LABEL_10;
}
*a2 = *(_DWORD *)&v7[24 * v8 + 6];
a2[1] = *(_DWORD *)&v7[24 * v8 + 10];
}
LABEL_10:
operator_delete(v7);
return v8 < v11;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== WriteLoginKey (0x58edb0) ===*/
void __thiscall WriteLoginKey(_DWORD *this, _DWORD *a2, int *a3)
{
_DWORD *v3; // esi
FILE *v4; // edi
size_t v5; // ebx
unsigned __int16 *v6; // esi
int v7; // ebx
int v8; // eax
int v9; // eax
_DWORD *v10; // esi
_DWORD *v11; // edi
_DWORD *v12; // ebp
unsigned __int16 *v13; // ebx
int v14; // eax
_DWORD *v15; // eax
int *v16; // eax
unsigned __int8 *v17; // eax
int v18; // eax
_DWORD *v19; // ecx
FILE *v20; // eax
FILE *v21; // edi
int *v22; // esi
int *v23; // eax
int *v24; // edi
unsigned __int8 v25; // [esp+8h] [ebp-50h]
int v26; // [esp+8h] [ebp-50h]
unsigned __int16 *v28; // [esp+10h] [ebp-48h]
char *v29; // [esp+14h] [ebp-44h]
_DWORD *v30; // [esp+18h] [ebp-40h] BYREF
int v31[2]; // [esp+1Ch] [ebp-3Ch] BYREF
unsigned int v32; // [esp+24h] [ebp-34h] BYREF
int v33; // [esp+28h] [ebp-30h] BYREF
int v34[2]; // [esp+2Ch] [ebp-2Ch] BYREF
char v35; // [esp+34h] [ebp-24h]
int v36; // [esp+38h] [ebp-20h] BYREF
void *v37; // [esp+3Ch] [ebp-1Ch]
void *v38; // [esp+40h] [ebp-18h]
char v39; // [esp+44h] [ebp-14h]
int v40; // [esp+48h] [ebp-10h]
int v41; // [esp+54h] [ebp-4h]
v3 = this;
if ( this[397] )
{
v4 = fopen(off_8BB988, aRb);
v28 = 0;
v29 = (char *)operator_new(24 * v3[397]);
LOBYTE(v36) = (_BYTE)a3;
BYTE1(v36) = (_BYTE)a3;
v39 = 0;
sub_5A3250(&v36);
v41 = 0;
if ( v4 )
{
v5 = (unsigned __int8)fgetc(v4);
v6 = (unsigned __int16 *)operator_new(24 * v5);
v28 = v6;
v25 = fread(v6, 0x18u, v5, v4);
v7 = 0;
if ( v25 )
{
do
{
v8 = v6[2];
v34[0] = *(_DWORD *)v6;
v35 = 0;
v34[1] = v8;
sub_5A2F90(&v36, (int)&v32, v34);
sub_6B3120(v31, &v32, &v33);
*(_BYTE *)(v31[0] + 20) = v7++;
v6 += 12;
}
while ( v7 < v25 );
}
fclose(v4);
v3 = this;
}
v9 = v3[397];
v10 = v3 + 397;
v26 = 0;
if ( v9 )
{
v11 = this;
v12 = v29 + 14;
v13 = (unsigned __int16 *)this + 807;
do
{
v14 = *v13;
v32 = *(_DWORD *)(v13 + 1);
v33 = v14;
v15 = sub_5A3C60(&v36, &v32);
v30 = v15;
if ( v15 == v37 || sub_5A4220(&v32, v15 + 3) )
{
v31[0] = (int)v37;
v16 = v31;
}
else
{
v16 = (int *)&v30;
}
v17 = (unsigned __int8 *)*v16;
if ( v17 == v37 )
{
*(_DWORD *)((char *)v12 - 14) = *(_DWORD *)(v13 + 1);
*((_WORD *)v12 - 5) = *v13;
}
else
{
qmemcpy((char *)v12 - 14, &v28[12 * v17[20]], 0x18u);
v11 = this;
}
v18 = v26;
if ( v26 == v11[270] )
{
v19 = a2;
if ( a2 )
{
v11[208] = *v12;
v11[209] = v12[1];
*v12 = *v19;
v12[1] = v19[1];
}
else
{
if ( (_BYTE)a3 )
{
*v12 = v11[208];
v12[1] = v11[209];
}
*(v12 - 2) = *v12;
*(v12 - 1) = v12[1];
}
}
v10 = v11 + 397;
v13 += 8;
v12 += 6;
++v26;
}
while ( (unsigned int)(v18 + 1) < v11[397] );
}
v20 = fopen(off_8BB988, aWb);
v21 = v20;
if ( v20 )
{
fwrite(v10, 1u, 1u, v20);
fwrite(v29, 0x18u, *v10, v21);
fclose(v21);
}
operator_delete(v28);
operator_delete(v29);
v22 = (int *)v37;
v23 = *(int **)v37;
v41 = -1;
a3 = v23;
if ( v40 )
{
sub_5A37A0(&v36, *((void ***)v37 + 1));
*((_DWORD *)v37 + 1) = v38;
v40 = 0;
*(_DWORD *)v37 = v37;
*((_DWORD *)v37 + 2) = v37;
}
else if ( v23 != v37 )
{
do
{
v24 = v23;
sub_5A3380(&a3);
sub_5A33D0(&v36, &a2, v24);
v23 = a3;
}
while ( a3 != v22 );
}
operator_delete(v37);
v37 = 0;
v40 = 0;
operator_delete(v38);
}
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== AuthHandleLogin (0x58f1c0) ===*/
// bad sp value at call has been detected, the output may be wrong!
void __thiscall AuthHandleLogin(char *this)
{
unsigned int v2; // ecx
unsigned int v3; // eax
bool v4; // zf
_DWORD *v5; // eax
_DWORD *v6; // eax
int v7; // ecx
int v8; // edi
char v9; // al
_DWORD *v10; // eax
_DWORD *v11; // eax
_DWORD *v12; // eax
_DWORD *v13; // eax
unsigned int v14; // kr04_4
unsigned int v15; // kr04_4
unsigned __int64 v16; // rdi
_BYTE *v17; // eax
_BYTE *v18; // edi
int v19; // esi
void (__thiscall ***v20)(_DWORD, int); // ecx
_DWORD *v21; // ecx
_DWORD *v22; // eax
char *v23; // [esp-10h] [ebp-164h]
int v24; // [esp-8h] [ebp-15Ch]
int v25; // [esp+0h] [ebp-154h] BYREF
void *theMsgR[3]; // [esp+Ch] [ebp-148h] BYREF
__int16 v27; // [esp+18h] [ebp-13Ch]
bool v28; // [esp+4Ch] [ebp-108h]
char v29; // [esp+4Dh] [ebp-107h]
int v30[4]; // [esp+50h] [ebp-104h] BYREF
int v31[8]; // [esp+60h] [ebp-F4h] BYREF
int v32[4]; // [esp+80h] [ebp-D4h] BYREF
int v33[4]; // [esp+90h] [ebp-C4h] BYREF
int v34; // [esp+A0h] [ebp-B4h] BYREF
int v35; // [esp+A4h] [ebp-B0h]
int v36; // [esp+A8h] [ebp-ACh]
int v37; // [esp+B0h] [ebp-A4h] BYREF
_BYTE *v38; // [esp+B4h] [ebp-A0h]
int v39; // [esp+B8h] [ebp-9Ch]
char v40[80]; // [esp+C8h] [ebp-8Ch] BYREF
_DWORD v41[3]; // [esp+118h] [ebp-3Ch] BYREF
int v42; // [esp+124h] [ebp-30h]
char *v43; // [esp+128h] [ebp-2Ch]
char v44; // [esp+12Fh] [ebp-25h]
int theProductR; // [esp+130h] [ebp-24h] BYREF
CHAR *v46; // [esp+134h] [ebp-20h]
unsigned int v47; // [esp+138h] [ebp-1Ch]
void *v48; // [esp+140h] [ebp-14h]
int *v49; // [esp+144h] [ebp-10h]
int v50; // [esp+150h] [ebp-4h]
v49 = &v25;
v43 = this;
titanDebug(aTitaninterface_17);
v2 = *((_DWORD *)this + 397);
if ( v2 )
{
v3 = *((_DWORD *)this + 270);
if ( v3 >= v2 )
*((_DWORD *)this + 270) = v3 % v2;
v41[2] = this + 2728;
EnterCriticalSection((LPCRITICAL_SECTION)(this + 2728));
v42 = 1;
v4 = *((_DWORD *)this + 218) == 0;
v50 = 0;
if ( !v4 )
{
titanDebug(aNoteTitaninter_1);
v5 = operator_new(0x18u);
v48 = v5;
LOBYTE(v50) = 1;
if ( v5 )
v6 = sub_6D1790(v5, 0, 0, 0, 0);
else
v6 = 0;
v7 = *((_DWORD *)this + 218);
LOBYTE(v50) = 0;
sub_6D08A0(v7, (int)v6);
*((_DWORD *)this + 218) = 0;
}
LeaveCriticalSection((LPCRITICAL_SECTION)(this + 2728));
v4 = *((_DWORD *)this + 248) == 0;
v42 = 0;
if ( v4 )
{
titanDebug(aFailAuthhandle);
}
else
{
sub_6AA840(theMsgR);
v8 = *((_DWORD *)this + 247);
v27 = *(_WORD *)(*((_DWORD *)this + 248) + 32);
v28 = v8 == 0;
v9 = this[1224];
LOBYTE(v50) = 2;
v29 = v9;
v10 = sub_6B4EF0(&theProductR, (int)(this + 1176));
LOBYTE(v50) = 3;
sub_59E960(v30, v10, 0, dword_72BF1C);
LOBYTE(v50) = 2;
sub_59FDF0(&theProductR, 1);
v11 = (_DWORD *)sub_6B5020((int)&theProductR, aCataclysm);
LOBYTE(v50) = 4;
sub_59E960(v31, v11, 0, dword_72BF1C);
LOBYTE(v50) = 2;
sub_59FDF0(&theProductR, 1);
v12 = sub_6B4EF0(&theProductR, (int)(this + 1192));
LOBYTE(v50) = 5;
sub_59E960(v32, v12, 0, dword_72BF1C);
LOBYTE(v50) = 2;
sub_59FDF0(&theProductR, 1);
v13 = sub_6B4EF0(&theProductR, (int)(this + 1208));
LOBYTE(v50) = 6;
sub_59E960(v33, v13, 0, dword_72BF1C);
LOBYTE(v50) = 2;
sub_59FDF0(&theProductR, 1);
LOBYTE(theProductR) = v44;
stdStringReset(&theProductR, 0);
v14 = strlen(aCataclysm) + 1;
v48 = (void *)(v14 - 1);
if ( stdStringResizeOrReserve(&theProductR, v14 - 1, 1) )
{
v15 = v14 - 1;
qmemcpy(v46, aCataclysm, v15);
v47 = v15 - 1;
v46[v15 - 1] = 0;
}
LOBYTE(v50) = 7;
WONCDKey_ClientCDKey_constructor(v40, &theProductR, 0);
LOBYTE(v50) = 9;
stdStringReset(&theProductR, 1);
if ( (unsigned __int8)WONCDKey_ClientCDKey_Load(v40) )
{
if ( (unsigned __int8)WONCDKey_ClientCDKey_IsValid(v40) )
{
if ( (unsigned __int8)WONCDKey_ClientCDKey_IsBeta(v40) )
{
titanDebug(aFailAuthhandle_2);
mgDisplayMessage((const char *)dword_BB3E58);
}
else
{
v16 = WONCDKey_ClientCDKey_BufferFromFields(v40);
if ( stdStringResizeOrReserveAlt(&v34, 8u, 1) )
{
*(_QWORD *)v35 = v16;
v36 = 8;
*(_BYTE *)(v35 + 8) = 0;
}
sub_58ECD0(this, v41);
if ( stdStringResizeOrReserveAlt(&v37, 8u, 1) )
{
v17 = v38;
v18 = v41;
v19 = 8;
do
{
*v17++ = *v18++;
--v19;
}
while ( v19 );
v39 = 8;
v38[8] = 0;
}
v20 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)this + 252);
if ( v20 )
(**v20)(v20, 1);
*((_DWORD *)this + 252) = 0;
LOBYTE(v50) = 10;
v21 = operator_new(0x14u);
v48 = v21;
LOBYTE(v50) = 11;
if ( v21 )
v22 = sub_6B85B0(v21, 8, 0);
else
v22 = 0;
v24 = *((_DWORD *)this + 248);
v50 = 9;
*((_DWORD *)this + 252) = v22;
LOBYTE(v50) = 13;
sub_6AAD00(theMsgR, v24, (int)v22);
v50 = 15;
sub_6AAF80(theMsgR);
v23 = &this[16 * *((_DWORD *)this + 270) + 1612];
v50 = 9;
*((_DWORD *)this + 218) = ConnectAndSend(this, v23, theMsgR, 1, 5);
}
}
else
{
titanDebug(aFailAuthhandle_1);
mgDisplayMessage((const char *)dword_BB3E48);
}
}
else
{
titanDebug(aFailAuthhandle_0);
mgDisplayMessage(dword_BB3E5C);
}
LOBYTE(v50) = 2;
WONCDKey_ClientCDKey_destructor(v40);
LOBYTE(v50) = 0;
sub_6AAB70(theMsgR);
}
}
else
{
titanDebug(aFailNoAuthServ);
}
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== AuthHandleChallenge (0x58f6a0) ===*/
void __thiscall AuthHandleChallenge(_DWORD *this, DWORD NumberOfBytesRead)
{
_DWORD *v2; // edi
int v3; // ebx
int v4; // eax
int v5; // edx
int v6; // eax
_DWORD *v7; // esi
bool v8; // zf
int v9; // eax
char v10; // al
_DWORD *v11; // esi
HMODULE ModuleHandleA; // eax
int v13; // eax
_BYTE *v14; // esi
int v15; // edi
int i; // esi
int v17; // [esp+0h] [ebp-2B0h] BYREF
CHAR Filename[260]; // [esp+Ch] [ebp-2A4h] BYREF
unsigned int v19[22]; // [esp+110h] [ebp-1A0h] BYREF
unsigned int v20[22]; // [esp+168h] [ebp-148h] BYREF
unsigned int context[22]; // [esp+1C0h] [ebp-F0h] BYREF
_DWORD theMsgR[9]; // [esp+218h] [ebp-98h] BYREF
_DWORD v23[4]; // [esp+23Ch] [ebp-74h] BYREF
int v24; // [esp+24Ch] [ebp-64h]
unsigned __int8 Buffer[16]; // [esp+264h] [ebp-4Ch] BYREF
unsigned __int8 digest[16]; // [esp+274h] [ebp-3Ch] BYREF
char v27[4]; // [esp+284h] [ebp-2Ch] BYREF
_BYTE *v28; // [esp+288h] [ebp-28h]
_DWORD *v29; // [esp+28Ch] [ebp-24h]
char v30; // [esp+290h] [ebp-20h]
void *v31; // [esp+294h] [ebp-1Ch]
LPCRITICAL_SECTION lpCriticalSection; // [esp+298h] [ebp-18h]
int v33; // [esp+29Ch] [ebp-14h]
int *v34; // [esp+2A0h] [ebp-10h]
int v35; // [esp+2ACh] [ebp-4h]
v2 = this;
v34 = &v17;
v29 = this;
titanDebug(aTitaninterface_18);
lpCriticalSection = (LPCRITICAL_SECTION)(v2 + 682);
EnterCriticalSection((LPCRITICAL_SECTION)(v2 + 682));
v3 = 1;
v33 = 1;
v4 = v2[252];
v35 = 0;
if ( v4 )
{
LOBYTE(v35) = 1;
sub_6AB090(v23, NumberOfBytesRead);
v5 = *(_DWORD *)v2[252];
LOBYTE(v35) = 2;
v6 = (*(int (__cdecl **)(char *, _DWORD, _DWORD))(v5 + 24))(v27, v23[3], (unsigned __int16)v24);
v7 = *(_DWORD **)v6;
v8 = *(_DWORD *)v6 == 0;
v9 = *(_DWORD *)(v6 + 4);
if ( v8 )
{
titanDebug(aFailUnableToDe);
AuthFailOver((char *)v2);
LOBYTE(v35) = 1;
sub_6AB100(v23);
while ( v3 )
{
LeaveCriticalSection(lpCriticalSection);
--v3;
}
}
else
{
v30 = 1;
v31 = v7;
LOBYTE(v35) = 3;
if ( v9 == 16 )
{
sub_58EDB0(v2, v7, 0);
if ( dword_A7517C )
{
sub_59DE30(1, Buffer);
MD5Init2(context);
MD5Update2(context, (unsigned __int8 *)v7, 0x10u);
while ( sub_59DE30(0, digest) )
MD5Update2(context, digest, 0x10u);
MD5Final2(digest, context);
}
else
{
MD5Init2(v20);
MD5Init2(context);
MD5Update2(context, (unsigned __int8 *)v7, 0x10u);
v10 = 1;
v11 = 0;
while ( 1 )
{
if ( v10 )
{
ModuleHandleA = GetModuleHandleA(0);
if ( !GetModuleFileNameA(ModuleHandleA, Filename, 0x104u) )
break;
hFile = CreateFileA(Filename, 0x80000000, 1u, 0, 3u, 0x8000000u, 0);
if ( !v11 )
v11 = operator_new(0x4000u);
}
if ( !ReadFile(hFile, v11, 0x4000u, &NumberOfBytesRead, 0) )
break;
if ( !NumberOfBytesRead )
break;
MD5Init2(v19);
MD5Update2(v19, (unsigned __int8 *)v11, NumberOfBytesRead);
MD5Final2(digest, v19);
if ( !NumberOfBytesRead )
break;
MD5Update2(v20, (unsigned __int8 *)v11, NumberOfBytesRead);
MD5Update2(context, digest, 0x10u);
v10 = 0;
}
MD5Final2(Buffer, v20);
MD5Final2(digest, context);
operator_delete(v11);
v2 = v29;
v3 = v33;
}
v13 = (*(int (__thiscall **)(_DWORD, char *, unsigned __int8 *, int))(*(_DWORD *)v2[252] + 20))(
v2[252],
v27,
Buffer,
32);
v14 = *(_BYTE **)v13;
v15 = *(_DWORD *)(v13 + 4);
if ( *(_DWORD *)v13 )
{
v27[0] = 1;
v28 = v14;
LOBYTE(v35) = 4;
sub_6AB180(theMsgR);
LOBYTE(v35) = 5;
sub_6AA230((int)theMsgR, v14, v15, 1);
sub_6AB290(theMsgR);
if ( (unsigned __int8)SendMsg((void *)v29[218], theMsgR, 0) )
{
LOBYTE(v35) = 4;
sub_6AB280(theMsgR);
operator_delete(v14);
if ( v30 )
operator_delete(v31);
LOBYTE(v35) = 1;
sub_6AB100(v23);
for ( i = v33; i; --i )
LeaveCriticalSection(lpCriticalSection);
}
else
{
titanDebug(aFailUnableToSe);
LOBYTE(v35) = 4;
sub_6AB280(theMsgR);
operator_delete(v14);
if ( v30 )
operator_delete(v31);
LOBYTE(v35) = 1;
sub_6AB100(v23);
while ( v33 )
{
LeaveCriticalSection(lpCriticalSection);
--v33;
}
}
}
else
{
titanDebug(aFailUnableToEn);
if ( v30 )
operator_delete(v31);
LOBYTE(v35) = 1;
sub_6AB100(v23);
while ( v3 )
{
LeaveCriticalSection(lpCriticalSection);
--v3;
}
}
}
else
{
titanDebug(aFailChallengeS);
AuthFailOver((char *)v2);
operator_delete(v7);
LOBYTE(v35) = 1;
sub_6AB100(v23);
while ( v3 )
{
LeaveCriticalSection(lpCriticalSection);
--v3;
}
}
}
}
else
{
titanDebug(aFailDonTHaveAu);
do
{
LeaveCriticalSection(lpCriticalSection);
--v3;
}
while ( v3 );
}
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== AuthHandleLoginReply (0x58fb70) ===*/
void __thiscall AuthHandleLoginReply(char *this, void *a2)
{
char *v2; // ebx
int v3; // eax
void *v4; // esi
int v5; // edi
void (__thiscall ***v6)(_DWORD, int); // ecx
void *v7; // ecx
int *v8; // ecx
int v9; // eax
int v10; // edx
void (__thiscall ***v11)(_DWORD, int); // ecx
void (__thiscall ***v12)(_DWORD, int); // ecx
int v13; // eax
_WORD *v14; // esi
int v15; // edi
_BYTE *v16; // ecx
_DWORD *v17; // eax
_DWORD *v18; // ecx
void (__thiscall ***v19)(_DWORD, int); // ecx
int v20; // esi
int v21; // eax
const void *v22; // esi
int v23; // edi
void (__thiscall ***v24)(_DWORD, int); // ecx
_DWORD *v25; // ecx
_DWORD *v26; // eax
int v27; // ecx
int v28; // esi
_DWORD *v29; // eax
_DWORD *v30; // eax
int v31; // ecx
void *v32; // eax
unsigned int v33; // ecx
int v34; // edx
char *v35; // edi
char *v36; // ebx
unsigned int v37; // esi
size_t v38; // edi
unsigned int v39; // esi
CHAR *v40; // esi
int v41; // ecx
unsigned int v42; // kr08_4
unsigned int v43; // ebx
_DWORD *v44; // esi
int v45; // eax
unsigned int v46; // eax
char *v47; // eax
int v48; // [esp-8h] [ebp-80h]
int v49; // [esp+0h] [ebp-78h] BYREF
_DWORD v50[3]; // [esp+Ch] [ebp-6Ch] BYREF
__int16 v51[36]; // [esp+18h] [ebp-60h]
char *v52; // [esp+64h] [ebp-14h]
int *v53; // [esp+68h] [ebp-10h]
int v54; // [esp+74h] [ebp-4h]
void *v55; // [esp+80h] [ebp+8h]
_BYTE *v56; // [esp+80h] [ebp+8h]
v2 = this;
v53 = &v49;
v52 = this;
titanDebug(aTitaninterface_19);
EnterCriticalSection((LPCRITICAL_SECTION)(v2 + 2728));
*((_DWORD *)v2 + 218) = 0;
LeaveCriticalSection((LPCRITICAL_SECTION)(v2 + 2728));
if ( !*((_DWORD *)v2 + 248) )
{
titanDebug(aFailAuthhandle);
return;
}
if ( !*((_DWORD *)v2 + 252) )
{
titanDebug(aFailDonTHaveAu);
return;
}
v48 = *((_DWORD *)v2 + 252);
v54 = 0;
sub_6A81B0(v50, (int)a2, v48, 0);
LOBYTE(v54) = 1;
if ( *(int *)v51 >= 0 )
{
v3 = sub_6A85E0(v50, 3);
v4 = *(void **)v3;
v5 = *(_DWORD *)(v3 + 4);
if ( *(_DWORD *)v3 )
{
v6 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)v2 + 248);
if ( v6 )
(**v6)(v6, 1);
*((_DWORD *)v2 + 248) = 0;
v7 = operator_new(0x30u);
LOBYTE(v54) = 2;
v8 = v7 ? (int *)Auth1PublicKeyBlock_constructor(v7, v4, v5) : 0;
v9 = *((_DWORD *)v2 + 246);
*((_DWORD *)v2 + 248) = v8;
v10 = *v8;
LOBYTE(v54) = 1;
if ( !(*(unsigned __int8 (__stdcall **)(int))(v10 + 8))(v9) )
{
titanDebug(aFailFailedToVe);
v11 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)v2 + 248);
if ( v11 )
(**v11)(v11, 1);
*((_DWORD *)v2 + 248) = 0;
AuthFailOver(v2);
goto LABEL_78;
}
}
v12 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)v2 + 249);
if ( v12 )
(**v12)(v12, 1);
*((_DWORD *)v2 + 249) = 0;
v13 = sub_6A85E0(v50, 1);
v14 = *(_WORD **)v13;
v15 = *(_DWORD *)(v13 + 4);
if ( !*(_DWORD *)v13 )
{
titanDebug(aFailDidnTGetCe);
AuthFailOver(v2);
goto LABEL_78;
}
v16 = operator_new(0x44u);
LOBYTE(v54) = 3;
if ( v16 )
v17 = sub_6D5EE0(v16, v14, v15);
else
v17 = 0;
v18 = (_DWORD *)*((_DWORD *)v2 + 248);
LOBYTE(v54) = 1;
*((_DWORD *)v2 + 249) = v17;
if ( !sub_6D5AA0(v18, v17) )
{
titanDebug(aFailCouldnTVer);
v19 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)v2 + 249);
if ( v19 )
goto LABEL_41;
goto LABEL_42;
}
v20 = *(_DWORD *)(*((_DWORD *)v2 + 249) + 20);
*((_DWORD *)v2 + 250) = v20 - time(0);
v21 = sub_6A85E0(v50, 2);
v22 = *(const void **)v21;
v23 = *(_DWORD *)(v21 + 4);
if ( *(_DWORD *)v21 )
{
v24 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)v2 + 247);
if ( v24 )
(**v24)(v24, 1);
*((_DWORD *)v2 + 247) = 0;
v25 = operator_new(0x18u);
LOBYTE(v54) = 4;
if ( v25 )
v26 = WONCryptPrivateKey_ctor_with_key(v25, v23, v22);
else
v26 = 0;
LOBYTE(v54) = 1;
*((_DWORD *)v2 + 247) = v26;
}
else if ( !*((_DWORD *)v2 + 247) )
{
titanDebug(aFailDidnTGetPr);
v19 = (void (__thiscall ***)(_DWORD, int))*((_DWORD *)v2 + 249);
if ( v19 )
LABEL_41:
(**v19)(v19, 1);
LABEL_42:
*((_DWORD *)v2 + 249) = 0;
LABEL_43:
AuthFailOver(v2);
goto LABEL_78;
}
titanDebug(aTitaninterface_13);
v27 = *((_DWORD *)v2 + 249);
*((_DWORD *)v2 + 269) = 0;
v28 = 1000 * (*(_DWORD *)(v27 + 24) - *(_DWORD *)(v27 + 20) - 120);
v29 = operator_new(0x1Cu);
LOBYTE(v54) = 5;
if ( v29 )
v30 = sub_6D1D40(v29, v28, 0);
else
v30 = 0;
v31 = *((_DWORD *)v2 + 232);
LOBYTE(v54) = 1;
sub_6D0850(v31, (int)v30);
v32 = (void *)*((_DWORD *)v2 + 304);
v33 = strlen((const char *)&data);
v55 = (void *)v33;
if ( (unsigned int)v32 < v33 )
v33 = *((_DWORD *)v2 + 304);
v34 = memcmp(*((const char **)v2 + 303), (const char *)&data, v33);
if ( !v34 && v32 >= v55 )
{
LOBYTE(v34) = v32 != v55;
if ( !v34 )
{
LABEL_77:
v2[1224] = 0;
sub_58EDB0(v2, 0, 0);
authReceiveReply(v51[0]);
goto LABEL_78;
}
}
v35 = v52 + 1208;
v36 = v52 + 1192;
v37 = *((_DWORD *)v52 + 304);
v56 = (_BYTE *)v37;
if ( dword_72BDB0 < v37 )
{
v37 = dword_72BDB0;
v56 = (_BYTE *)dword_72BDB0;
}
if ( v36 == v35 )
{
v38 = dword_72BDB0;
if ( *((_DWORD *)v52 + 300) < v37 )
std_Xran();
if ( *((_DWORD *)v52 + 300) - v37 < dword_72BDB0 )
v38 = *((_DWORD *)v52 + 300) - v37;
if ( v38 )
{
stdStringUnshare((_DWORD *)v52 + 298);
memcpy(
(void *)(v37 + *((_DWORD *)v36 + 1)),
(const void *)(v37 + *((_DWORD *)v36 + 1) + v38),
*((_DWORD *)v36 + 2) - v38 - v37);
v39 = *((_DWORD *)v36 + 2) - v38;
if ( stdStringResizeOrReserve(v36, v39, 0) )
std_basic_string_char_std_char_traits_char_std_allocator_char_Eos(v36, v39);
}
}
else if ( stdStringResizeOrReserve((_DWORD *)v52 + 298, v37, 1) )
{
v40 = (CHAR *)*((_DWORD *)v35 + 1);
if ( !v40 )
v40 = FileName;
qmemcpy(*((void **)v36 + 1), v40, (unsigned int)v56);
v41 = *((_DWORD *)v36 + 1);
*((_DWORD *)v36 + 2) = v56;
v56[v41] = 0;
}
v42 = strlen((const char *)&data) + 1;
v43 = v42 - 1;
if ( v42 - 1 > 0xFFFFFFFD )
std_Xlen();
v44 = v52 + 1208;
v45 = *((_DWORD *)v52 + 303);
*((_DWORD *)v52 + 304) = 0;
if ( v45 && *(_BYTE *)(v45 - 1) )
{
if ( v42 == 1 )
{
LABEL_69:
stdStringReset(v44, 1);
LABEL_76:
v2 = v52;
goto LABEL_77;
}
}
else
{
if ( v42 == 1 )
goto LABEL_69;
v46 = v44[3];
if ( v46 <= 0x1F && v46 >= v43 )
{
LABEL_75:
v47 = v52 + 1208;
qmemcpy(*((void **)v52 + 303), &data, v43);
*((_DWORD *)v47 + 2) = v43;
*(_BYTE *)(v43 + *((_DWORD *)v47 + 1)) = 0;
goto LABEL_76;
}
stdStringReset(v44, 1);
}
stdStringReallocate(v44, v43);
goto LABEL_75;
}
titanDebug("TitanInterface::AuthHandleLoginReply Status = %d", *(_DWORD *)v51);
if ( *(_DWORD *)v51 == -1503 )
sub_58EDB0(v2, 0, (int *)1);
if ( !authReceiveReply(v51[0]) )
goto LABEL_43;
titanDebug(aTitaninterface_13);
*((_DWORD *)v2 + 269) = 0;
LABEL_78:
LOBYTE(v54) = 0;
sub_6A8440(v50);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== AuthHandleRefresh (0x590130) ===*/
void __thiscall AuthHandleRefresh(_DWORD ***this)
{
__int16 v2; // di
int v3; // eax
_DWORD *v4; // eax
_DWORD *v5; // edi
int v6; // ebx
bool v7; // zf
__int16 v8; // ax
_DWORD ***v9; // [esp-10h] [ebp-90h]
int v10; // [esp-4h] [ebp-84h]
int v11; // [esp+0h] [ebp-80h] BYREF
_DWORD theMsgR[3]; // [esp+Ch] [ebp-74h] BYREF
__int16 v13; // [esp+18h] [ebp-68h]
void *v14; // [esp+4Ch] [ebp-34h]
_DWORD v15[2]; // [esp+54h] [ebp-2Ch] BYREF
_DWORD v16[2]; // [esp+5Ch] [ebp-24h] BYREF
int v17; // [esp+64h] [ebp-1Ch] BYREF
_DWORD *v18; // [esp+68h] [ebp-18h]
int *v19; // [esp+70h] [ebp-10h]
int v20; // [esp+7Ch] [ebp-4h]
v19 = &v11;
titanDebug(aTitaninterface_21);
if ( !this[248] )
{
titanDebug(aFailDonTHavePu);
return;
}
if ( !sub_58ECD0(this, v15) )
{
titanDebug(aAuthhandlerefr);
return;
}
sub_694AE0(&v17);
v20 = 0;
v2 = time(0);
sub_694C40(&v17);
sub_694500(v18, v2);
sub_694C40(&v17);
sub_694560(v18, 8u, v15, 1, 1);
v10 = sub_6944B0(v18);
v3 = sub_694C30(&v17);
v4 = sub_6D5AF0(this[248], v16, v3, v10);
v5 = (_DWORD *)*v4;
v6 = v4[1];
v7 = *v4 == 0;
v14 = v5;
LOBYTE(v16[0]) = !v7;
v16[1] = v5;
LOBYTE(v20) = 1;
if ( v5 )
{
sub_6AB330(theMsgR);
v8 = *((_WORD *)this[248] + 16);
LOBYTE(v20) = 2;
v13 = v8;
sub_6AA660((int)theMsgR, v5, v6, 1);
sub_6AA750((int)theMsgR, 0, 0, 0);
LOBYTE(v20) = 3;
sub_6AB440(theMsgR);
v9 = &this[4 * (_DWORD)this[270] + 403];
v20 = 2;
this[218] = (_DWORD **)ConnectAndSend(this, v9, theMsgR, 1, 5);
LOBYTE(v20) = 1;
sub_6AB430(theMsgR);
if ( LOBYTE(v16[0]) )
LABEL_9:
operator_delete(v5);
}
else
{
titanDebug(aFailCouldnTEnc);
if ( LOBYTE(v16[0]) )
goto LABEL_9;
}
v20 = -1;
sub_694BE0(&v17);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== Authenticate (0x590340) ===*/
void __thiscall Authenticate(char *this, _DWORD *a2, _DWORD *a3, _DWORD *a4, char a5)
{
_DWORD *v6; // eax
_DWORD *v7; // eax
sdword v8; // eax
int v9; // ebp
titanDebug(aTitaninterface_22);
EnterCriticalSection((LPCRITICAL_SECTION)(this + 2728));
if ( *((_DWORD *)this + 218) )
{
titanDebug(aNoteTitaninter_2);
v6 = operator_new(0x18u);
if ( v6 )
v7 = sub_6D1790(v6, 0, 0, 0, 0);
else
v7 = 0;
sub_6D08A0(*((_DWORD *)this + 218), (int)v7);
*((_DWORD *)this + 218) = 0;
}
LeaveCriticalSection((LPCRITICAL_SECTION)(this + 2728));
if ( *((_DWORD *)this + 246) )
{
PlayVoiceMessage((_DWORD *)this + 294, a2, 0, dword_72BDB0);
PlayVoiceMessage((_DWORD *)this + 298, a3, 0, dword_72BDB0);
PlayVoiceMessage((_DWORD *)this + 302, a4, 0, dword_72BDB0);
this[1224] = a5;
if ( !this[58] )
{
v8 = dword_886014;
if ( dword_886014 )
{
this[59] = 1;
this[56] = 1;
if ( v8 != 1 )
FirewallDetect(this);
}
else
{
this[59] = 0;
this[56] = 1;
}
this[58] = 1;
}
if ( *((_DWORD *)this + 397) )
{
if ( *((_DWORD *)this + 248) )
sub_58F1C0(this);
else
sub_58E9C0(this);
}
else
{
this[1004] = 1;
titanDebug(aTitaninterface_23);
EnterCriticalSection((LPCRITICAL_SECTION)(this + 2728));
v9 = 1;
if ( *((_DWORD *)this + 219) )
{
titanDebug(aNoteTitaninter_3);
do
{
LeaveCriticalSection((LPCRITICAL_SECTION)(this + 2728));
--v9;
}
while ( v9 );
}
else
{
LeaveCriticalSection((LPCRITICAL_SECTION)(this + 2728));
if ( *((_DWORD *)this + 397) )
{
if ( *((_DWORD *)this + 255) )
sub_5936B0(this);
else
sub_593C40(this);
}
else
{
sub_5933F0(this);
}
}
}
}
else
{
titanDebug(aFailCanTAuthen);
}
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== DestroyLoginKeyServerIndexMap (0x58f100) ===*/
void __thiscall DestroyLoginKeyServerIndexMap(void *this)
{
  int v2; // ecx
  int *v3; // edi
  int v4; // eax
  _DWORD *i; // ebx
  int v6; // edi
  _DWORD *v7; // eax
  int *v8; // ebx
  _DWORD *v9; // [esp-8h] [ebp-24h]
  int *v10; // [esp+10h] [ebp-Ch] BYREF
  int v11; // [esp+14h] [ebp-8h] BYREF
  int *v12; // [esp+18h] [ebp-4h] BYREF

  v2 = *((_DWORD *)this + 4);
  v3 = (int *)*((_DWORD *)this + 1);
  v4 = *v3;
  v10 = (int *)*v3;
  if ( v2 )
  {
    v6 = v3[1];
    for ( i = (_DWORD *)v6; v6 != *((_DWORD *)this + 2); i = (_DWORD *)v6 )
    {
      sub_5A37A0(this, *(void ***)(v6 + 8));
      v6 = *(_DWORD *)v6;
      operator_delete(i);
    }
    *(_DWORD *)(*((_DWORD *)this + 1) + 4) = *((_DWORD *)this + 2);
    v7 = (_DWORD *)*((_DWORD *)this + 1);
    *((_DWORD *)this + 4) = 0;
    *v7 = v7;
    *(_DWORD *)(*((_DWORD *)this + 1) + 8) = *((_DWORD *)this + 1);
    sub_6DC320((_DWORD **)this, &v11);
  }
  else if ( (int *)v4 != v3 )
  {
    do
    {
      v8 = (int *)v4;
      sub_5A3380(&v10);
      sub_5A33D0(this, &v12, v8);
      v4 = (int)v10;
    }
    while ( v10 != v3 );
  }
  operator_delete((_DWORD *)*((_DWORD *)this + 1));
  v9 = (_DWORD *)*((_DWORD *)this + 2);
  *((_DWORD *)this + 1) = 0;
  *((_DWORD *)this + 4) = 0;
  operator_delete(v9);
  *((_DWORD *)this + 2) = 0;
}
#endif


#if 0  // Decompiled pseudocode reference
/*=== scriptSetPortNumbers (0x52C8E0) ===*/
const char *__cdecl scriptSetPortNumbers(int a1, char *a2, int a3)
{
  int i; // esi
  const char *result; // eax

  for ( i = 0; i < 10; ++i )
  {
    result = strtok(a2, asc_87BA40);
    a2 = 0;
    if ( !result )
      break;
    result = (const char *)sscanf(result, "%d", a3);
    a3 += 4;
  }
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== scriptSetIPStrings (0x52C920) ===*/
const char *__cdecl scriptSetIPStrings(int a1, char *a2, int a3)
{
  int i; // esi
  const char *result; // eax

  for ( i = 0; i < 10; ++i )
  {
    result = strtok(a2, asc_87BA40);
    a2 = 0;
    if ( !result )
      break;
    result = (const char *)sscanf(result, "%s", a3);
    a3 += 50;
  }
  return result;
}
#endif
