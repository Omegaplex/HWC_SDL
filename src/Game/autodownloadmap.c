/*=============================================================================
    Name    : Autodownloadmap.c
    Purpose : Code for autodownloading maps

    Created 99/03/29 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "memory.h"
#include "debug.h"
#include "file.h"
#include "commandnetwork.h"
#include "titan.h"
#include "horserace.h"
#include "StringsOnly.h"

#define NUM_FILES_OF_MAP_GROW       20

typedef struct FileOfMapInfo
{
    char filename[100];
    char filedirname[100];
    sdword fileSize;
    sdword fileCRC;
} FileOfMapInfo;

typedef struct AutoDownloadMapInfo
{
    char mapname[100];      // note, includes %d of players, e.g. Curtain%d
    char dirname[100];      // e.g., Multiplayer\\Curtain%d\\            directory
    char exactmapname[100]; // same as mapname but with actual number of players
    char exactdirname[100]; // same as dirname but with actual number of players
    sdword numPlayers;
    sdword minPlayers;      // min players map can support
    sdword maxPlayers;      // max players map can support
    bool32 needAutoDownload;
    sdword numFilesSent;
    sdword numFilesAutodownloaded;      // only has meaning if you are the one downloading
    sdword totalFilesToAutodownload;    // only has meaning if you are the one downloading
    sdword mapCRC[MAX_MULTIPLAYER_PLAYERS];
    bool8 playersNeedMap[MAX_MULTIPLAYER_PLAYERS];
    sdword numFilesOfMap;
    sdword numFilesOfMapAllocated;
    FileOfMapInfo *fileOfMapInfo;
} AutoDownloadMapInfo;

AutoDownloadMapInfo autodownloadmapInfo;


FilePacket *CreateFilePacket(char *dirname,char *filename,sdword filecontentssize);

/*=============================================================================
    Functions:
=============================================================================*/

void autodownloadmapStartup()
{
    memset(&autodownloadmapInfo,0,sizeof(autodownloadmapInfo));
}

void autodownloadmapShutdown()
{
    if (autodownloadmapInfo.fileOfMapInfo)
    {
        memFree(autodownloadmapInfo.fileOfMapInfo);
        autodownloadmapInfo.fileOfMapInfo = NULL;
    }

    memset(&autodownloadmapInfo,0,sizeof(autodownloadmapInfo));
}

void autodownloadmapReset()
{
    autodownloadmapShutdown();
}

void GetExactMapDirNames(sdword num)
{
    sprintf(autodownloadmapInfo.exactmapname,autodownloadmapInfo.mapname,num);
    sprintf(autodownloadmapInfo.exactdirname,autodownloadmapInfo.dirname,num);
}

bool32 autodownloadmapBuiltIn()
{
    char fullfile[200];

    GetExactMapDirNames(autodownloadmapInfo.numPlayers);

    strcpy(fullfile,autodownloadmapInfo.exactdirname);
    strcat(fullfile,autodownloadmapInfo.exactmapname);
    strcat(fullfile,".level");

    if (fileExistsInBigFile(fullfile))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void autodownloadmapGotMapName(char *mapname,sdword numPlayers,sdword minPlayers,sdword maxPlayers)
{
    strcpy(autodownloadmapInfo.mapname,mapname);

    strcpy(autodownloadmapInfo.dirname,"Multiplayer\\");
    strcat(autodownloadmapInfo.dirname,autodownloadmapInfo.mapname);
    strcat(autodownloadmapInfo.dirname,"\\");

    autodownloadmapInfo.numPlayers = numPlayers;
    autodownloadmapInfo.minPlayers = minPlayers;
    autodownloadmapInfo.maxPlayers = maxPlayers;

    if ((!autodownloadmapBuiltIn()) && (sigsNumPlayers > 1))       // sigsNumPlayers is # of network players, numPlayers includes computer players
    {
        autodownloadmapInfo.needAutoDownload = TRUE;
    }
}

bool32 autodownloadmapRequired()
{
    return autodownloadmapInfo.needAutoDownload;
}

void AddFileOfMap(char *filedirname,char *filename,sdword fileSize)
{
    sdword index = autodownloadmapInfo.numFilesOfMap++;

    if (autodownloadmapInfo.numFilesOfMap > autodownloadmapInfo.numFilesOfMapAllocated)
    {
        if (autodownloadmapInfo.fileOfMapInfo == NULL)
        {
            autodownloadmapInfo.fileOfMapInfo = memAlloc(sizeof(FileOfMapInfo)*NUM_FILES_OF_MAP_GROW,"fileofmap",0);
        }
        else
        {
            autodownloadmapInfo.fileOfMapInfo = memRealloc(autodownloadmapInfo.fileOfMapInfo,sizeof(FileOfMapInfo)*(autodownloadmapInfo.numFilesOfMapAllocated + NUM_FILES_OF_MAP_GROW),"rfileofmap",0);
        }

        autodownloadmapInfo.numFilesOfMapAllocated += NUM_FILES_OF_MAP_GROW;
    }

    strcpy(autodownloadmapInfo.fileOfMapInfo[index].filename,filename);
    strcpy(autodownloadmapInfo.fileOfMapInfo[index].filedirname,filedirname);
    autodownloadmapInfo.fileOfMapInfo[index].fileSize = fileSize;
    autodownloadmapInfo.fileOfMapInfo[index].fileCRC = 0;
}

void autodownloadmapGetFilesOfMap(void)
{
    struct _finddata_t find;
    sdword handle, startHandle;
    char dir[200];
    char file[200];
    sdword fileSize;
    sdword i;

    autodownloadmapInfo.numFilesOfMap = 0;

    for (i=autodownloadmapInfo.minPlayers;i<=autodownloadmapInfo.maxPlayers;i++)
    {
        GetExactMapDirNames(i);

        strcpy(dir,filePathPrepend(autodownloadmapInfo.exactdirname,0));
        strcat(dir,"\\*.*");

        startHandle = handle = _findfirst(dir, &find);

        while (handle != -1)
        {
            if (find.name[0] == '.')
            {
                goto next;
            }

            if (strstr(find.name,".mdb"))
            {
                goto next;      // don't send .mdb files
            }

            strcpy(file,autodownloadmapInfo.exactdirname);
            strcat(file,find.name);

            fileSize = fileSizeGet(file,FF_IgnoreBIG);

            if ((fileSize <= 0) || (fileSize > 64000))
            {
                goto next;      // file too big
            }

            AddFileOfMap(autodownloadmapInfo.exactdirname,find.name,fileSize);

    next:
            handle = _findnext(startHandle, &find);
        }
    }
}

void SendFile(char *dirname,char *filename,sdword fileSize)
{
    FilePacket *pkt;

    pkt = CreateFilePacket(dirname,filename,fileSize);
    // for now, just broadcast pkt
    titanDebug("autodownloadmap: Sending file packet of size %d\n",sizeofFilePacketGivenPacket(pkt));
    titanSendBroadcastMessage((ubyte *)pkt,sizeofFilePacketGivenPacket(pkt));
}

bool32 autodownloadmapSendAFile(void)
{
    sdword i = autodownloadmapInfo.numFilesSent;

    if (i >= autodownloadmapInfo.numFilesOfMap)
    {
        return TRUE;
    }

    SendFile(autodownloadmapInfo.fileOfMapInfo[i].filedirname,autodownloadmapInfo.fileOfMapInfo[i].filename,autodownloadmapInfo.fileOfMapInfo[i].fileSize);
    autodownloadmapInfo.numFilesSent++;

    if (autodownloadmapInfo.numFilesSent >= autodownloadmapInfo.numFilesOfMap)
    {
        return TRUE;
    }

    return FALSE;
}

void autodownloadmapSendAllFiles(void)
{
    sdword i;

    for (i=0;i<autodownloadmapInfo.numFilesOfMap;i++)
    {
        SendFile(autodownloadmapInfo.fileOfMapInfo[i].filedirname,autodownloadmapInfo.fileOfMapInfo[i].filename,autodownloadmapInfo.fileOfMapInfo[i].fileSize);
    }
}

bool32 autodownloadmapReceivedAllFiles()
{
    if (autodownloadmapInfo.numFilesAutodownloaded == 0)
    {
        return FALSE;
    }

    if (autodownloadmapInfo.numFilesAutodownloaded >= autodownloadmapInfo.totalFilesToAutodownload)
    {
        return TRUE;
    }

    return FALSE;
}

real32 autodownloadmapPercentReceivedFiles()
{
    if (autodownloadmapInfo.numFilesAutodownloaded == 0)
    {
        return 0.0f;
    }

    return (real32)autodownloadmapInfo.numFilesAutodownloaded / (real32)autodownloadmapInfo.totalFilesToAutodownload;
}

real32 autodownloadmapPercentSentFiles()
{
    if (autodownloadmapInfo.numFilesSent == 0)
    {
        return 0.0f;
    }

    return (real32)autodownloadmapInfo.numFilesSent / (real32)autodownloadmapInfo.numFilesOfMap;
}

FilePacket *CreateFilePacket(char *dirname,char *filename,sdword filecontentssize)
{
    char file[200];
    sdword packetsize;
    sdword filenamesize;
    FilePacket *packet;

    strcpy(file,dirname);
    strcat(file,filename);

    filenamesize = strlen(file) + 1;

    packetsize = sizeofFilePacket(filenamesize,filecontentssize);

    packet = memAlloc(packetsize,"filepkt",0);

    packet->packetheader.type = PACKETTYPE_FILE;
    packet->packetheader.from = sigsPlayerIndex;
    packet->packetheader.frame = filecontentssize;
    packet->packetheader.numberOfCommands = filenamesize;

    packet->totalNumFiles = autodownloadmapInfo.numFilesOfMap;
    strcpy(packet->filename,file);
    fileLoad(file, filecontentsAddress(packet,filenamesize), FF_IgnoreBIG);

    return packet;
}

void SaveFilePacketToFile(FilePacket *fpacket)
{
    titanDebug("autodownloadmap: Received file packet of size %d\n", sizeofFilePacketGivenPacket(fpacket));

    if (sigsPlayerIndex == 0)
    {
        titanDebug("autodownloadmap: WARNING: player 0 received file");
        return;
    }

    // lets write file to filesystem, but don't use file.h since this is in another thread
    // and file.c, file.h is not reentrant (and we're not writing to a big file)

    FILE *outFile;
    char file[250];
    char *findlastslashptr;
    sdword lengthWrote;
    sdword filecontentssize = fpacket->packetheader.frame;
    sdword filenamesize = fpacket->packetheader.numberOfCommands;

    strcpy(file,filePrependPath);
    strcat(file,fpacket->filename);

    findlastslashptr = &file[strlen(file)];
    while (--findlastslashptr >= file)
    {
        if (*findlastslashptr == '\\')
        {
            *findlastslashptr = 0;
            break;
        }
    }

    _mkdir(file);      // try to make directory.  If it already exists, this will fail, that's ok

    strcpy(file,filePrependPath);
    strcat(file,fpacket->filename);

    if ((outFile = fopen(file, "wb")) == NULL)           //open the file
    {
        titanDebug("autodownloadmap: WARNING could not open file %s for writing\n",file);
    }
    else
    {
        lengthWrote = fwrite(filecontentsAddress(fpacket,filenamesize),1,filecontentssize,outFile);
        if (lengthWrote != filecontentssize)
        {
            titanDebug("autodownloadmap: ERROR tried to write %d bytes only wrote %d bytes to file %s\n",filecontentssize,lengthWrote,file);
        }

        fclose(outFile);
    }

}

void receivedFilePacketCB(ubyte *packet,udword sizeofPacket)
{
#define fpacket ((FilePacket *)packet)

    dbgAssert(sizeofPacket == sizeofFilePacketGivenPacket(fpacket));
    SaveFilePacketToFile(fpacket);
    autodownloadmapInfo.totalFilesToAutodownload = fpacket->totalNumFiles;
    autodownloadmapInfo.numFilesAutodownloaded++;
#undef fpacket
}

void autodownloadmapPrintStatusStart()
{
    if (sigsPlayerIndex == 0)
    {
        strcpy(horseracestatus.hrstatusstr[sigsPlayerIndex],strGetString(strCustomMapAutoupload));
    }
    else
    {
        strcpy(horseracestatus.hrstatusstr[sigsPlayerIndex],strGetString(strCustomMapAutodownload));
    }
}

void autodownloadmapPrintStatus()
{
#if 0
    if (sigsPlayerIndex != 0)
    {
        if (autodownloadmapInfo.numFilesAutodownloaded != 0)
        {
            sprintf(horseracestatus.hrstatusstr[sigsPlayerIndex],"Detected User Map - Autodownloaded %d/%d files",autodownloadmapInfo.numFilesAutodownloaded,autodownloadmapInfo.totalFilesToAutodownload);
        }
    }
#endif
}



#if 0  // Cat2: Binary autodownloadmapSendAFile; live implementation at autodownloadmap.c L215 (0x422940)
// ============================================================================
// Decompiled from cataclysm.exe @ 0x422940
bool autodownloadmapSendAFile(void)
{
  int v1;
  int v2;
  unsigned int v3;
  unsigned int v4;
  char *v5;
  int v6;
  char *v7;
  char v9[200];

  if ( dword_C52320 >= dword_C52354 )
    return 1;
  v1 = dword_C5235C + 208 * dword_C52320;
  strcpy(v9, (const char *)(v1 + 100));
  v2 = *(_DWORD *)(v1 + 200);
  v3 = strlen((const char *)v1) + 1;
  qmemcpy(&v9[strlen(v9)], (const void *)v1, v3);
  v4 = strlen(v9) + 1;
  v5 = (char *)memAllocFunction(v2 + v4 + 31, MBF_Volatile);
  v6 = v2;
  v7 = v5;
  *(_WORD *)v5 = -3810;
  *((_WORD *)v5 + 1) = *(_WORD *)sigsPlayerIndex;
  *((_DWORD *)v5 + 1) = v6;
  *((_WORD *)v5 + 12) = v4;
  *((_WORD *)v5 + 14) = dword_C52354;
  strcpy(v5 + 30, v9);
  fileLoad(v9, &v5[v4 + 30], 256);
  titanDebug(
    "autodownloadmap: Sending file packet of size %d\n",
    *((unsigned __int16 *)v7 + 12) + *((_DWORD *)v7 + 1) + 31);
  titanSendBroadcastMessage((unsigned __int16 *)v7, *((unsigned __int16 *)v7 + 12) + *((_DWORD *)v7 + 1) + 31);
  return ++dword_C52320 >= dword_C52354;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4226B0
void autodownloadmapGetFilesOfMap(void)
{
  int result;
  int v1;
  int v2;
  int v3;
  int v4;
  void *v5;
  int v6;
  int i;
  int v8;
  struct _finddata_t v9;
  char v10[200];
  char v11[200];

  result = dword_C52318;
  v1 = dword_C52314;
  dword_C52354 = 0;
  for ( i = dword_C52314; v1 <= dword_C52318; i = v1 )
  {
    sprintf(byte_C52248, byte_C52180, v1);
    sprintf(path, dword_C521E4, v1);
    strcpy(v11, filePathPrepend(path, 0));
    strcat(v11, "\\*.*");
    v8 = _findfirst(v11, &v9);
    if ( v8 != -1 )
    {
      do
      {
        if ( v9.name[0] != 46 && !strstr(v9.name, ".mdb") )
        {
          strcpy(v10, path);
          strcat(v10, v9.name);
          v2 = fileSizeGet(v10, 256);
          v3 = v2;
          if ( v2 > 0 && v2 <= 64000 )
          {
            v4 = dword_C52354++;
            if ( dword_C52354 > dword_C52358 )
            {
              if ( dword_C5235C )
                v5 = memReallocFunction(
                       (ubyte *)dword_C5235C,
                       16 * (dword_C52358 + 4 * (3 * dword_C52358 + 60) + 20),
                       0);
              else
                v5 = memAllocFunction(4160, MBF_Volatile);
              dword_C5235C = (int)v5;
              dword_C52358 += 20;
            }
            v6 = 208 * v4;
            strcpy((char *)(208 * v4 + dword_C5235C), v9.name);
            strcpy((char *)(208 * v4 + dword_C5235C + 100), path);
            *(_DWORD *)(v6 + dword_C5235C + 200) = v3;
            *(_DWORD *)(v6 + dword_C5235C + 204) = 0;
          }
        }
      }
      while ( _findnext(v8, &v9) != -1 );
      v1 = i;
    }
    result = dword_C52318;
    ++v1;
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x422AA0
bool autodownloadmapReceivedAllFiles(void)
{
  int result;

  result = dword_C52324;
  if ( dword_C52324 )
    return dword_C52324 >= dword_C52328;
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x422AC0
real32 autodownloadmapPercentReceivedFiles(void)
{
  if ( dword_C52324 )
    return (double)dword_C52324 / (double)dword_C52328;
  else
    return ZERO;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x422AE0
real32 autodownloadmapPercentSentFiles(void)
{
  if ( dword_C52320 )
    return (double)dword_C52320 / (double)dword_C52354;
  else
    return ZERO;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4224F0
void autodownloadmapGotMapName(char *mapname, sdword numPlayers, sdword minPlayers, sdword maxPlayers)
{
  int v4;
  BOOL result;
  char v6[200];

  v4 = dword_74E620;
  strcpy(byte_C52180, a1);
  *(_DWORD *)dword_C521E4 = dword_74E61C;
  dword_C521E8 = v4;
  strcpy((char *)&dword_C521EC, "yer\\");
  strcat(dword_C521E4, byte_C52180);
  strcat(dword_C521E4, asc_74E618);
  dword_C52310 = a2;
  dword_C52314 = a3;
  dword_C52318 = a4;
  sprintf(byte_C52248, byte_C52180, a2);
  sprintf(path, dword_C521E4, a2);
  strcpy(v6, path);
  strcat(v6, byte_C52248);
  strcat(v6, ".level");
  result = fileExistsInBigFile(v6);
  if ( !result )
  {
    result = 1;
    if ( dword_A43DF0 > 1 )
      dword_C5231C = 1;
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x4226A0
bool autodownloadmapRequired(void)
{
  return dword_C5231C;
}
#endif


#if 0  // Cat2: receivedFilePacketCB live code differs from binary
/*=== receivedFilePacketCB (0x422CC0) ===*/
void receivedFilePacketCB(ubyte *packet, udword sizeofPacket)
{
    titanDebug("autodownloadmap: Received file packet of size %d\n", sizeofFilePacketGivenPacket((FilePacket *)packet));

    if (sigsPlayerIndex == 0)
    {
        titanDebug("autodownloadmap: WARNING: player 0 received file");
        return;
    }
#define fpacket ((FilePacket *)packet)
    {
        char dirtomake[250];
        char *findlastslashptr;

        strcpy(dirtomake, filePrependPath);
        strcat(dirtomake, fpacket->filename);
        findlastslashptr = &dirtomake[strlen(dirtomake)];

        while (--findlastslashptr >= dirtomake)
        {
            if (*findlastslashptr == '\\')
            {
                *findlastslashptr = 0;
                break;
            }
        }

        dbgAssert(*findlastslashptr == 0);
        _mkdir(dirtomake);
    }

    dbgAssert(sizeofPacket == sizeofFilePacketGivenPacket(fpacket));
    SaveFilePacketToFile(fpacket);
    autodownloadmapInfo.totalFilesToAutodownload = fpacket->totalNumFiles;
    autodownloadmapInfo.numFilesAutodownloaded++;
#undef fpacket
}
#endif
