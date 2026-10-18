/*=============================================================================
    Name    : autodownloadmap.h
    Purpose : Definitions for autodownloadmap.c

    Created date by user
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___AUTODOWNLOADMAP_H
#define ___AUTODOWNLOADMAP_H

#include "types.h"

void autodownloadmapStartup();

void autodownloadmapShutdown();

void autodownloadmapReset();

void autodownloadmapGetFilesOfMap(void);

void autodownloadmapGotMapName(char *mapname,sdword numPlayers,sdword minPlayers,sdword maxPlayers);

bool32 autodownloadmapRequired();

void receivedFilePacketCB(ubyte *packet,udword sizeofPacket);

bool32 autodownloadmapReceivedAllFiles();

void autodownloadmapSendAllFiles(void);

void autodownloadmapPrintStatusStart();

void autodownloadmapPrintStatus();

real32 autodownloadmapPercentReceivedFiles();

real32 autodownloadmapPercentSentFiles();

bool32 autodownloadmapSendAFile(void);

#endif

