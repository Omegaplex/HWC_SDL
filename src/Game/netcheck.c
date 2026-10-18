
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "switches.h"
#include "types.h"
#include "file.h"
#include "universe.h"
#include "univupdate.h"
#include "netcheck.h"
#include "commandnetwork.h"
#include "blobs.h"
#include "globals.h"
#include "titannet.h"
#include "SaveGame.h"
#include "randy.h"
#include "titaninterfacec.h"
#include "commandwrap.h"
#include "stringsonly.h"
#include "stats.h"

char OrigRecordPacketFileName[MAX_RECORDPACKETFILENAME_STRLEN] = "";


#ifdef GOD_LIKE_SYNC_CHECKING


#define GOD_NUMBER_CHECKSUMS_REMEMBERED 16384
#define GOD_NUMBER_CHECKSUMS_REMEMBERED_MASK 16383


//checksum stack for local machine
GodSyncCheckSums godnetsyncchecksums[GOD_NUMBER_CHECKSUMS_REMEMBERED];

//first frame that a sync error occurred between systems
//<not the frame we broke at>
udword godRandErrFrame;
udword godUnivErrFrame;
udword godBlobErrFrame;
udword godShipErrFrame;
sdword breaked;

//typedef struct in spaceobj.h


GodWritePacketHeader *godUniverseSnapShotHeader=NULL;

#endif

filehandle netlogfileFH;
FILE *netlogfile = NULL;

/*=============================================================================
    Private data:
=============================================================================*/

#define NETCHECK_REMEMBERED     128
#define NETCHECK_MASK           127

#define CHECKSUMVALID_NOT       0
#define CHECKSUMVALID_VALID     1
#define CHECKSUMVALID_CHECKED   2

static NetSyncChecksums netcheckChecksums[NETCHECK_REMEMBERED];
static udword netcheckValid[NETCHECK_REMEMBERED];
static sdword netcheckLastSlot;

udword netcheckRandSyncErr;
udword netcheckRandSyncErrFrame;
udword netcheckUnivSyncErr;
udword netcheckUnivSyncErrFrame;
udword netcheckBlobSyncErr;
udword netcheckBlobSyncErrFrame;
udword netcheckPktSyncErr;
udword netcheckCheatDetectCount;
sdword netcheckCheatDetectFrame;

udword netcheckExtraData = 0;


filehandle playPacketFileFH;
FILE *playPacketFile = NULL;

/*=============================================================================
    Functions:
=============================================================================*/

#ifdef GOD_LIKE_SYNC_CHECKING

void godSyncInit()
{
    sdword sizeofHeader,i;
    //init some global variables
    godRandErrFrame=0;
    godUnivErrFrame=0;
    godBlobErrFrame=0;
    godShipErrFrame=0;
    breaked=FALSE;

    //calculate sizeof the universesnapshotheader.
    sizeofHeader =  sizeofGodHeader(syncDumpWindowSize);

    godUniverseSnapShotHeader=memAlloc(sizeofHeader,"SyncSnapHeader",ExtendedPool);
    if(godUniverseSnapShotHeader == NULL)
    {
        dbgFatalf(DBG_Loc,"\nCouldn't Allocate %d bytes for syncChecking Header.",sizeofHeader);
    }

    strcpy(godUniverseSnapShotHeader->playerName,playerNames[universe.curPlayerIndex]);
    godUniverseSnapShotHeader->windowSize=syncDumpWindowSize;
    //init all ptr's to NULL just to be sure.
    for(i=0;i<syncDumpWindowSize;i++)
    {
        godUniverseSnapShotHeader->universeSnapShot[i]=NULL;
    }
}
void godSyncShutDown()
{
    sdword i;
    if(godUniverseSnapShotHeader != NULL)
    {
        for(i=0;i<godUniverseSnapShotHeader->windowSize;i++)
        {
            if(godUniverseSnapShotHeader->universeSnapShot[i]!=NULL)
            {
                memFree(godUniverseSnapShotHeader->universeSnapShot[i]);
            }
        }
        memFree(godUniverseSnapShotHeader);
    }
}

//pass in FALSE to generate another frame and keep it in memory
//pass in TRUE to write the whole memory dump to disk without
//generating a new snapshot
void syncDebugDump(char *filename1,sdword counter,bool32 save)
{
    filehandle syncFH;
    FILE *syncFP = NULL;
    Node *node = universe.ShipList.head;
    sdword numShips = universe.ShipList.num;
    Ship *ship;
    sdword i,sizeofHeader,sizeofsnapshot;
    char filename[200];

    sizeofsnapshot = sizeofSnapShot(numShips);
    if(save)
    {
        //delete old file if its there.
        sprintf(filename,"SyncDump_CRC_%X_%X_%X_%X_Player_%s.txt",HomeworldCRC[0],
                HomeworldCRC[1],HomeworldCRC[2],HomeworldCRC[3],
                playerNames[universe.curPlayerIndex]);

        fileDelete(filename);

        syncFH = fileOpen(filename,FF_WriteMode);
        dbgAssert(!fileUsingBigfile(syncFH));
        syncFP = fileStream(syncFH);

        //don't want Ptr array in file!
        sizeofHeader =  sizeof(GodWritePacketHeader);
        fwrite(godUniverseSnapShotHeader,sizeofHeader,1,syncFP);
    }
    else
    {
        //not SAVEING!
        //operating on block syncDumpGranTrack
        if(godUniverseSnapShotHeader->universeSnapShot[syncDumpWindowPos] != NULL)
        {
            memFree(godUniverseSnapShotHeader->universeSnapShot[syncDumpWindowPos]);
        }

        //allocate memory for this frame
        godUniverseSnapShotHeader->universeSnapShot[syncDumpWindowPos] = (UniverseSnapShot *) memAlloc(sizeofsnapshot,"snapshot",ExtendedPool);
        if(godUniverseSnapShotHeader->universeSnapShot[syncDumpWindowPos] == NULL)
        {
            //couldn't get the memory
            dbgFatalf(DBG_Loc,"Memory Allocation Failure on Universe Sync SnapShot %d",counter);
        }

    }

    if(!save)
    {
        UniverseSnapShot *us=godUniverseSnapShotHeader->universeSnapShot[syncDumpWindowPos];
        sdword j=0;
        us->univUpdateCounterValue = counter;
        us->numShips = numShips;

        while(node != NULL)
        {
            ship = (Ship *) listGetStructOfNode(node);
            us->ship[j] = *ship;

    //NULLify all ptrs...
            us->ship[j].objlink.next=0;
            us->ship[j].objlink.prev=0;
            us->ship[j].objlink.belongto=0;
            us->ship[j].objlink.structptr=0;


            us->ship[j].staticinfo=0;
            us->ship[j].renderlink.next=0;
            us->ship[j].renderlink.prev=0;
            us->ship[j].renderlink.belongto=0;
            us->ship[j].renderlink.structptr=0;

            us->ship[j].collMyBlob = 0;
            us->ship[j].newDockWithTransfer = 0;

            us->ship[j].currentLOD=0;
            us->ship[j].renderedLODs=0;
            us->ship[j].cameraDistanceSquared=0;
            us->ship[j].cameraDistanceVector.x=0;
            us->ship[j].cameraDistanceVector.y=0;
            us->ship[j].cameraDistanceVector.z=0;

            us->ship[j].collInfo.selCircleX=0;
            us->ship[j].collInfo.selCircleY=0;
            us->ship[j].collInfo.selCircleRadius=0;
            us->ship[j].collInfo.precise=0;

            us->ship[j].flashtimer=0;
            us->ship[j].chatterBits=0;
            us->ship[j].hyperspacePing=0;
            us->ship[j].hotKeyGroup=0;
            us->ship[j].retaliateSpeechVar=0;
            us->ship[j].retaliateSpeechTime=0;

            if(us->ship[j].flags & SOF_Dead)
            {
                us->ship[j].flags|=SOF_Dead;
            }
            else
            {
                us->ship[j].flags=0;
            }

            us->ship[j].impactablelink.next=0;
            us->ship[j].impactablelink.prev=0;
            us->ship[j].impactablelink.belongto=0;
            us->ship[j].impactablelink.structptr=0;

            us->ship[j].clampInfo=0;
            us->ship[j].cataBusyStateWindow.busyStatePtr1EC=0;
            if(us->ship[j].cataBusyStateWindow.dockingship != NULL)
            {
                us->ship[j].cataBusyStateWindow.dockingship=(Ship *)us->ship[j].cataBusyStateWindow.dockingship->shipID.shipNumber;
            }
            else
            {
                us->ship[j].cataBusyStateWindow.dockingship=-1;
            }
            us->ship[j].navLightInfo=0;

            us->ship[j].shiplink.next=0;
            us->ship[j].shiplink.prev=0;
            us->ship[j].shiplink.belongto=0;
            us->ship[j].shiplink.structptr=0;

            if(us->ship[j].rowGetOutOfWay != NULL)
            {
                us->ship[j].rowGetOutOfWay=(Ship *)us->ship[j].rowGetOutOfWay->shipID.shipNumber;
            }
            else
            {
                us->ship[j].rowGetOutOfWay=-1;
            }

            if(us->ship[j].playerowner != NULL)
            {
                us->ship[j].playerowner = (Player *)us->ship[j].playerowner->playerIndex;
            }
            else
            {
                us->ship[j].playerowner = -1;
            }

            if(us->ship[j].gettingrocked != NULL)
            {
                us->ship[j].gettingrocked=(Ship *)us->ship[j].gettingrocked->shipID.shipNumber;     // ship which is rocking you (shooting you)
            }
            else
            {
                us->ship[j].gettingrocked=-1;
            }
            if(us->ship[j].recentAttacker != NULL)
            {
                us->ship[j].recentAttacker=(Ship *)us->ship[j].recentAttacker->shipID.shipNumber;    // ship which has attacked you recently (ie, not just this AI tick) -- see recentlyAttacked
            }
            else
            {
                us->ship[j].recentAttacker=-1;
            }
            if(us->ship[j].firingAtUs != NULL)
            {
                us->ship[j].firingAtUs=(Ship *)us->ship[j].firingAtUs->shipID.shipNumber;        // ship which has recently fired upon us (but needn't have hit) see recentlyFiredUpon
            }
            else
            {
                us->ship[j].firingAtUs=-1;
            }

            us->ship[j].flightmanInfo=0;
            us->ship[j].formationcommand = 0;
            if(us->ship[j].command != NULL)
            {
                us->ship[j].command = (CommandToDo *)us->ship[j].command->ordertype.order;
            }
            else
            {
                us->ship[j].command = -1;
            }

            //turn off this COMPUTER PLAYER specific variale!
            us->ship[j].specialFlags &= (~(SPECIAL_Resourcing|SPECIAL_Finished_Resource));

            us->ship[j].attackvars.multipleAttackTargets=0;   // only used by ships which can target multiple targets.

            us->ship[j].attackvars.attacktarget=0;

            us->ship[j].attackvars.myLeaderIs=0;
            us->ship[j].attackvars.myWingmanIs=0;


            if(us->ship[j].dockvars.dockship!=NULL)
            {
                us->ship[j].dockvars.dockship = us->ship[j].dockvars.dockship->shipID.shipNumber;
            }
            else
            {
                us->ship[j].dockvars.dockship=0;
            }
            us->ship[j].dockvars.busyingThisShip=0;
            us->ship[j].dockvars.dockstaticpoint=0;
            us->ship[j].dockvars.customdockinfo=0;
            us->ship[j].dockvars.launchpoints=0;

            us->ship[j].rceffect=0;
            us->ship[j].ShipXHarvestsResourceY = 0;

            us->ship[j].damageLights=0;

            us->ship[j].showingDamage[0][0]=0;
            us->ship[j].showingDamage[0][1]=0;
            us->ship[j].showingDamage[0][2]=0;
            us->ship[j].showingDamage[1][0]=0;
            us->ship[j].showingDamage[1][1]=0;
            us->ship[j].showingDamage[1][2]=0;
            us->ship[j].showingDamage[2][0]=0;
            us->ship[j].showingDamage[2][1]=0;
            us->ship[j].showingDamage[2][2]=0;

            us->ship[j].upgradeState.level=0;
            for(i=0;i<MAX_NUM_TRAILS;i++)
            {
                us->ship[j].trail[i]=0;
            }

            memset(&us->ship[j].soundevent,0,sizeof(SOUNDEVENT));
            us->ship[j].cataclysmGunInfo = 0;

            us->ship[j].slavelink.next=0;
            us->ship[j].slavelink.prev=0;
            us->ship[j].slavelink.belongto=0;
            us->ship[j].slavelink.structptr=0;

            us->ship[j].slaveinfo=0;
            us->ship[j].legacy_unk_4E0_ptr=0;
            us->ship[j].dockInfo=0;
            us->ship[j].unk_4EC_ptr=0;
            us->ship[j].bindings=0;
            us->ship[j].madBindings=0;
            us->ship[j].shipSinglePlayerGameInfo=0;
            us->ship[j].lightning[0]=0;
            us->ship[j].lightning[1] = 0;
            us->ship[j].magnitudeSquared=0;
            us->ship[j].ShipSpecifics[0]=0;
            j++;
            node=node->next;
        }
    }
    else
    {
        //first find oldest...
        {
            udword oldest;
            sdword oldestSnapShot=0;
            sdword windowSize=godUniverseSnapShotHeader->windowSize;

            if(godUniverseSnapShotHeader->universeSnapShot[0] != NULL)
            {
                oldest = godUniverseSnapShotHeader->universeSnapShot[0]->univUpdateCounterValue;
                for(i=0;i<godUniverseSnapShotHeader->windowSize;i++)
                {
                    if(godUniverseSnapShotHeader->universeSnapShot[i] == NULL)
                    {
                        windowSize=i;
                        break;
                    }
                    if(godUniverseSnapShotHeader->universeSnapShot[i]->univUpdateCounterValue < oldest)
                    {
                        oldest = godUniverseSnapShotHeader->universeSnapShot[i]->univUpdateCounterValue;
                        oldestSnapShot=i;
                    }
                }
                for(i=oldestSnapShot;i<windowSize;i++)
                {
                    sizeofsnapshot = sizeofSnapShot(godUniverseSnapShotHeader->universeSnapShot[i]->numShips);
                    fwrite(&sizeofsnapshot,sizeof(sdword),1,syncFP);
                    fwrite(godUniverseSnapShotHeader->universeSnapShot[i],sizeofsnapshot,1,syncFP);
                }
                for(i=0;i<oldestSnapShot;i++)
                {
                    sizeofsnapshot = sizeofSnapShot(godUniverseSnapShotHeader->universeSnapShot[i]->numShips);
                    fwrite(&sizeofsnapshot,sizeof(sdword),1,syncFP);
                    fwrite(godUniverseSnapShotHeader->universeSnapShot[i],sizeofsnapshot,1,syncFP);
                }

            }
        }
        fileClose(syncFH);
    }


}


//Saves the sync info every so often depending on
//syncDumpWindowSize -
//and
//syncDumpGranularity - every Xth frame it saves a window slice
void industrialStrengthSyncDebugging(sdword FrameNumber)
{
    syncDumpGranTrack++;
    if(syncDumpGranTrack >= syncDumpGranularity)
    {
        //exceeded granularity so lets save!
        syncDumpGranTrack=0;

        syncDebugDump("syncSnapShot.txt", FrameNumber,FALSE);

        syncDumpWindowPos++;
        if(syncDumpWindowPos >= syncDumpWindowSize)
        {
            syncDumpWindowPos = 0;  //reset window position
        }
    }
}

void netcheckIndustrialChecksum()
{
    sdword numShipsInChecksum;
    sdword num;

    //And with a bit mask so our 'num' cycles around a fixed number
    //GOD_NUMBER_CHECKSUMS_REMEMBERED 1024
    num = universe.univUpdateCounter & GOD_NUMBER_CHECKSUMS_REMEMBERED_MASK;

    godnetsyncchecksums[num].randcheck=gamerand();
    godnetsyncchecksums[num].univcheck=univGetChecksum(&numShipsInChecksum);
    godnetsyncchecksums[num].blobcheck=statsGetStatChecksum();
    godnetsyncchecksums[num].shipcheck=univCalcShipChecksum();
    godnetsyncchecksums[num].univframe=universe.univUpdateCounter;

    if(!IAmCaptain)
    {
        if(universe.univUpdateCounter & 7)
        {
            clSendGodSync(&godnetsyncchecksums[num],universe.curPlayerIndex,GOD_COMMAND_GOD_HALTONSYNC);
        }
    }
}

void netReceivedSyncFromNonCaptain(void *checksums,udword frame,udword playerIndex)
{
    bool32 BREAK=FALSE;
    sdword num = frame & GOD_NUMBER_CHECKSUMS_REMEMBERED_MASK;

    if(godnetsyncchecksums[num].randcheck != ((GodSyncCheckSums *)checksums)->randcheck)
    {
        if(!godRandErrFrame)
        {
            godRandErrFrame=frame;
        }
        BREAK=TRUE;
    }
    if(godnetsyncchecksums[num].univcheck != ((GodSyncCheckSums *)checksums)->univcheck)
    {
        if(!godUnivErrFrame)
        {
            godUnivErrFrame=frame;
        }
        BREAK=TRUE;
    }
    if(godnetsyncchecksums[num].blobcheck != ((GodSyncCheckSums *)checksums)->blobcheck)
    {
        if(!godBlobErrFrame)
        {
            godBlobErrFrame=frame;
        }
        BREAK=TRUE;
    }
    if(godnetsyncchecksums[num].shipcheck != ((GodSyncCheckSums *)checksums)->shipcheck)
    {
        if(!godShipErrFrame)
        {
            godShipErrFrame=frame;
        }
        BREAK=TRUE;
    }
    if(BREAK && !breaked)
    {
        breaked=TRUE;
        clSendGodSync(&godnetsyncchecksums[num],universe.curPlayerIndex,GOD_COMMAND_HALTONSYNC);
    }
}

#endif

void recPackInGameStartCB(char *filename)
{
    if (recordPackets)
    {
        // already recording
        if (debugPacketRecord)
        {
            strcpy(OrigRecordPacketFileName,recordPacketFileName);      // allow /packetRecord option and recording game too
        }
        else
        {
            return;
        }
    }

    strcpy(recordPacketSaveFileName,filename);
    strcpy(recordPacketFileName,filename);
    strcat(recordPacketFileName,PKTS_EXTENSION);

    startRecordingGameWhenSafe = TRUE;
}

void recPackInGameStartCBSafeToStart()
{
    filehandle fh;

    fileDelete(recordPacketSaveFileName);
    fileDelete(recordPacketFileName);

    recordPackets = TRUE;
    recordplayPacketsInGame = TRUE;
    recordingActive = TRUE;
    recordSaveGameInProgress = TRUE;

    if (SaveGame(recordPacketSaveFileName))
    {
        clCommandMessage(strGetString(strRecordingGame), -1);
        recordSaveGameInProgress = FALSE;

        if (multiPlayerGame)
        {
            multiPlayerRecording = TRUE;
        }
        else
        {
            memset(netcheckValid, 0, sizeof(netcheckValid));
            recordFakeSendPackets = TRUE;
            multiPlayerRecording = FALSE;
            captainIndex = 0;
            sigsPlayerIndex = 0;
            sigsNumPlayers = 1;
            sigsPressedStartGame = TRUE;
            netcheckLastSlot = -1;
            netcheckRandSyncErr = 0;
            netcheckUnivSyncErr = 0;
            netcheckBlobSyncErr = 0;
            netcheckPktSyncErr = 0;
            netcheckCheatDetectCount = 0;
            netcheckRandSyncErrFrame = 0;
            netcheckUnivSyncErrFrame = 0;
            netcheckBlobSyncErrFrame = 0;
            netcheckCheatDetectFrame = 0;
            netlogfile = NULL;
            if (logEnable)
            {
                if (logFilePath[0])
                    fh = fileOpen(logFilePath, FF_WriteMode | FF_TextMode | FF_ReturnNULLOnFail | FF_IgnorePrepend);
                else
                    fh = fileOpen("netlog.txt", FF_WriteMode | FF_TextMode | FF_ReturnNULLOnFail);
                netlogfileFH = fh;
                if (fh)
                    netlogfile = (FILE *)fileStream(fh);
            }
        }
    }
    else
    {
        clCommandMessage(strGetString(strPatchUnableWriteFile), -16776961);
        recordSaveGameInProgress = FALSE;
    }
}

void recPackInGameStopCB(void)
{
    if (!recordPackets)
    {
        return;
    }

    recordPackets = FALSE;
    recordplayPacketsInGame = FALSE;
}

void recPackRecordSaveHeader();

void recPackRecordInit(void)
{
    char tempstr[] = "_x.pkts";
    sprintf(recordPacketFileName,tpGameCreated.MapName,numPlayers);
    tempstr[1] = '0' + (ubyte)curPlayer;
    strcat(recordPacketFileName,tempstr);

    // clear file
    fileDelete(recordPacketFileName);

    dbgAssert(!recordplayPacketsInGame);
    recPackRecordSaveHeader();
}

#define HEADERCHECK 0x44414548
void recPackRecordSaveHeader()
{
    udword size = sizeof(CaptainGameInfo);
    udword validcheck = HEADERCHECK;
    filehandle fh = fileOpen(recordPacketFileName,FF_AppendMode);
    FILE *fp;
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    dbgAssert(!recordplayPacketsInGame);

    fwrite(&validcheck, sizeof(udword), 1 ,fp);
    fwrite(&size, sizeof(udword), 1, fp);
    fwrite(&tpGameCreated, size, 1, fp);
    fileClose(fh);
}

#define VALIDCHECK 0x4b434150
void recPackRecordPacket(ubyte *packet,udword sizeofPacket)
{
    udword size = sizeofPacket;
    udword validcheck = VALIDCHECK;

    filehandle fh = fileOpen(recordPacketFileName,FF_AppendMode);
    FILE *fp;
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    fwrite(&validcheck, sizeof(udword), 1 ,fp);
    fwrite(&size, sizeof(udword), 1, fp);
    fwrite(packet, sizeofPacket, 1, fp);
    fileClose(fh);
}

void recPackRecordPacketFilename(ubyte *packet,udword sizeofPacket,char *filename)
{
    udword size = sizeofPacket;
    udword validcheck = VALIDCHECK;

    filehandle fh = fileOpen(filename,FF_AppendMode);
    FILE *fp;
    dbgAssert(!fileUsingBigfile(fh));
    fp = fileStream(fh);

    fwrite(&validcheck, sizeof(udword), 1 ,fp);
    fwrite(&size, sizeof(udword), 1, fp);
    fwrite(packet, sizeofPacket, 1, fp);
    fileClose(fh);
}

ubyte *recPackPlayGetNextPacket(udword *sizeofPacket)
{
    udword size;
    udword validcheck;
    ubyte *packet;

tryagain:

    if (fread(&validcheck,sizeof(udword),1,playPacketFile) == 0) return NULL;
    if (validcheck == HEADERCHECK)      // skip headers
    {
        if (fread(&size,sizeof(udword),1,playPacketFile) == 0) return NULL;
        packet = memAlloc(size,"ld(loadpacket)",Pyrophoric);
        fread(packet,size,1,playPacketFile);
        memFree(packet);
        goto tryagain;
    }
    if (validcheck != VALIDCHECK)
    {
        dbgFatalf(DBG_Loc,"Bad packet recording in file %s",recordPacketFileName);
    }
    if (fread(&size,sizeof(udword),1,playPacketFile) == 0) return NULL;
    packet = memAlloc(size,"ld(loadpacket)",Pyrophoric);

    fread(packet,size,1,playPacketFile);

    *sizeofPacket = size;
    return packet;
}

bool32 recPackPlayLoadHeader()
{
    udword size;
    udword validcheck;

    if (fread(&validcheck,sizeof(udword),1,playPacketFile) == 0) return FALSE;
    if (validcheck == HEADERCHECK)
    {
        if (fread(&size,sizeof(udword),1,playPacketFile) == 0) return FALSE;
        if (size != sizeof(tpGameCreated)) return FALSE;
        fread(&tpGameCreated,size,1,playPacketFile);
        return TRUE;
    }
    else
    {
        // woops, we read something that isn't header, seek back and ignore
        fseek(playPacketFile,-((sdword)sizeof(udword)),SEEK_CUR);
        return FALSE;
    }

    return FALSE;
}

void recPackPlayInGameInit(void)
{
    playPackets = TRUE;
    recordplayPacketsInGame = TRUE;
    playPacketFileFH = fileOpen(recordPacketFileName, 0);
    dbgAssert(!fileUsingBigfile(playPacketFileFH));
    playPacketFile = fileStream(playPacketFileFH);
    netcheckInit();
}

void recPackPlayInit(void)
{
    char *str;
    playPacketFileFH = fileOpen(recordPacketFileName, 0);
    dbgAssert(!fileUsingBigfile(playPacketFileFH));
    playPacketFile = fileStream(playPacketFileFH);

    dbgAssert(!recordplayPacketsInGame);

    str = strstr(recordPacketFileName,".pkts");
    dbgAssert(str);
    str -= 3; // backup three bytes
    dbgAssert(str[1] == '_');
    numPlayers = str[0] - '0';
    curPlayer = str[2] - '0';
    dbgAssert(numPlayers > 0);
    dbgAssert(numPlayers <= MAX_MULTIPLAYER_PLAYERS);
    dbgAssert(curPlayer < MAX_MULTIPLAYER_PLAYERS);

    recPackPlayLoadHeader();
}

void recPackPlayClose(void)
{
    if (playPacketFileFH)
    {
        fileClose(playPacketFileFH);
        playPacketFile = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : netcheckReset
    Description : resets netCheck module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void netcheckReset(void)
{
    memset(netcheckValid, 0, sizeof(netcheckValid));
    netcheckLastSlot = -1;
    netcheckRandSyncErr = 0;
    netcheckUnivSyncErr = 0;
    netcheckBlobSyncErr = 0;
    netcheckPktSyncErr = 0;
    netcheckCheatDetectCount = 0;
    netcheckRandSyncErrFrame = 0;
    netcheckUnivSyncErrFrame = 0;
    netcheckBlobSyncErrFrame = 0;
    netcheckCheatDetectFrame = 0;
}

/*-----------------------------------------------------------------------------
    Name        : netCheckInit
    Description : initialize netCheck module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void netcheckInit(void)
{
    memset(netcheckValid, 0, sizeof(netcheckValid));
    netcheckLastSlot = -1;
    netcheckRandSyncErr = 0;
    netcheckUnivSyncErr = 0;
    netcheckBlobSyncErr = 0;
    netcheckPktSyncErr = 0;
    netcheckCheatDetectCount = 0;
    netcheckRandSyncErrFrame = 0;
    netcheckUnivSyncErrFrame = 0;
    netcheckBlobSyncErrFrame = 0;
    netcheckCheatDetectFrame = 0;

    netlogfile = NULL;
    if (logEnable)
    {
        if (logFilePath[0])
            netlogfileFH = fileOpen(logFilePath, FF_WriteMode | FF_TextMode | FF_ReturnNULLOnFail | FF_IgnorePrepend);
        else
            netlogfileFH = fileOpen("netlog.txt", FF_WriteMode | FF_TextMode | FF_ReturnNULLOnFail);

        if (netlogfileFH)
        {
            netlogfile = fileStream(netlogfileFH);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : netCheckClose
    Description : closes netCheck module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void netcheckClose(void)
{
    if (netlogfile != NULL)
    {
        fileClose(netlogfileFH);
        netlogfile = NULL;
    }
}

void netcheckFillInChecksum(HWPacketHeader *packet)
{
    if (netcheckLastSlot >= 0)
    {
        packet->checksums = netcheckChecksums[netcheckLastSlot];
    }
    else
    {
        packet->checksums.randcheck = 0;
        packet->checksums.extra = netcheckExtraData;
    }
}

void netcheckCheckChecksum(NetSyncChecksums *checksum)
{
    udword slot;

    if (checksum->randcheck == 0)
    {
        return;
    }

    slot = checksum->packetnum & NETCHECK_MASK;

    if ((netcheckValid[slot] & (CHECKSUMVALID_VALID | CHECKSUMVALID_CHECKED)) == CHECKSUMVALID_VALID &&
        checksum->packetnum == netcheckChecksums[slot].packetnum)
    {
        if (netcheckChecksums[slot].randcheck != checksum->randcheck)
        {
            if (!netcheckRandSyncErr)
                netcheckRandSyncErrFrame = universe.univUpdateCounter;
            netcheckRandSyncErr++;
        }

        if (netcheckChecksums[slot].univcheck != checksum->univcheck)
        {
            if (!netcheckUnivSyncErr)
                netcheckUnivSyncErrFrame = universe.univUpdateCounter;
            netcheckUnivSyncErr++;
        }

        if (netcheckChecksums[slot].blobcheck != checksum->blobcheck)
        {
            if (!netcheckBlobSyncErr)
            {
                netcheckBlobSyncErrFrame = universe.univUpdateCounter;
                if (multiPlayerGame)
                    SendCheatDetect();
            }
            netcheckBlobSyncErr++;
        }

        netcheckValid[slot] |= CHECKSUMVALID_CHECKED;
    }
}

void rinOutputCRC(HWPacketHeader *packet)
{
    sdword numShipsInChecksum;
    real32 univcheck = univGetChecksum(&numShipsInChecksum);
    udword statcheck = statsGetStatChecksum();
    uword randcheck = (uword)gamerand();
    uword packetnum = (uword)packet->frame;
    udword slot = ((udword)packetnum) & NETCHECK_MASK;

    netcheckChecksums[slot].packetnum = packetnum;
    netcheckChecksums[slot].randcheck = randcheck;
    netcheckChecksums[slot].univcheck = univcheck;
    netcheckChecksums[slot].blobcheck = statcheck;
    netcheckChecksums[slot].extra = netcheckExtraData;

    netcheckValid[slot] = CHECKSUMVALID_VALID;
    netcheckLastSlot = (sdword)slot;

    netcheckCheckChecksum(&packet->checksums);

    if (netlogfile)
    {
        fprintf(netlogfile, "Packet:%d Rand:%d Checksum:%f %d CDET:%d\n",
                packetnum, randcheck, univcheck, numShipsInChecksum, statcheck);
    }
}
