#include <string.h>
#include "types.h"
#include "TitanInterfaceC.h"
#include "ChannelFSM.h"
#include "msg\ServerStatus.h"
#include "debug.h"
#include "multiplayergame.h"
#include "strings.h"
#include "titannet.h"

// defines for the state of the connection to the chat server
#define CS_Idle                 -1
#define CS_IsRoomThere          3
#define CS_RoomConnecting       4
#define CS_RoomConnected        5
#define CS_RoomCreating         6
#define CS_RoomCreate           7
#define CS_IsADefaultRoomThere  8
#define CS_CreatingGame         20
#define CS_CreatedGame          21
#define CS_StartingTitan        30
#define CS_TitanStarted         31


// function prototypes

void cChannelStateMachine(void);

// data

sdword cChannelFSMState=-1;
char   channeltry[MAX_CHANNEL_NAME_LEN];
char   channeldesctry[MAX_CHANNEL_DESCRIPTION_LEN];

// functions to start the state machine

bool32 cCreateChannel(wchar_t *name, wchar_t *desc)
{
    //titanCreateDirectory(name, desc);

    SetChannel(name,desc);

    cChannelFSMState = CS_RoomCreate;
    cChannelStateMachine();

    return(FALSE);
}

bool32 cJoinChannelRequest(wchar_t *name, wchar_t *desc)
{
    SetChannel(name,desc);
    if (cChannelFSMState != CS_Idle)
    {
        return (TRUE);
    }

    cChannelFSMState = CS_IsRoomThere;

    cChannelStateMachine();

    titanRefreshRequest(NULL);

    return(FALSE);
}

bool32 cJoinADefaultRoom(void)
{
    ChannelPassword[0] = 0;
    if (cChannelFSMState != CS_Idle)
    {
        return (TRUE);
    }

    cChannelFSMState = CS_IsADefaultRoomThere;

    cChannelStateMachine();

    titanRefreshRequest(NULL);

    return(FALSE);
}

bool32 cCreateGame(wchar_t *name)
{

    return(FALSE);
}

bool32 cStartTitan(void)
{
    if (cChannelFSMState != CS_Idle)
    {
        return(TRUE);
    }

    cChannelFSMState = CS_StartingTitan;

    cChannelStateMachine();

    return(FALSE);
}

void cResetFSM(void)
{
    cChannelFSMState = CS_Idle;
}

void cChannelStateMachine(void)
{
    switch (cChannelFSMState)
    {
        case CS_RoomCreating :
            mgDisplayMessage(strGetString(strCreatingRoom));
        break;

        case CS_IsADefaultRoomThere:
        case CS_IsRoomThere :
            mgDisplayMessage(strGetString(strQueryingChat));
        break;

        case CS_RoomConnecting :
            mgDisplayMessage(strGetString(strConnectingToChat));
            chatConnect(ChannelPassword);
        break;

        case CS_RoomConnected :
            cChannelFSMState = CS_Idle;
            mgDisplayMessage(strGetString(strConnectedToChat));
        break;

        case CS_RoomCreate :
        {
            if (!titanStartChatServer(ChannelProtectedFlag ? ChannelPassword : NULL))
            {
                cChannelFSMState = CS_Idle;
            }
            mgDisplayMessage(strGetString(strStartingChat));
        }
        break;

        case CS_Idle :

        break;
    }
}

#if 0
void cNotifyCurrentRoomPresent(int present)
{
}
#endif

#define FIND_DEFAULT_ROOM_TRIES     3

sdword FindADefaultRoom(void)
{
    sdword i,tryagain;

    // first iteration match both language and someone criteria
    // second iteration just language criteria
    // third iteration no criteria
    bool32 languagecriteria[FIND_DEFAULT_ROOM_TRIES] = { TRUE, TRUE,  FALSE };
    bool32 someonecriteria[FIND_DEFAULT_ROOM_TRIES] =  { TRUE, FALSE, FALSE };

    tpchannel *tc;

    for (tryagain=0;tryagain<FIND_DEFAULT_ROOM_TRIES;tryagain++)
    {
        for (i=0;i<tpChannelList.numChannels;i++)
        {
            tc = &tpChannelList.tpChannels[i];
            if ((tc->roomflags & (ROOMFLAGS_ADEFAULTROOM|ROOMFLAGS_PASSWORDPROTECTED)) == (ROOMFLAGS_ADEFAULTROOM))
            {
                if ((!languagecriteria[tryagain]) ||
                    ((tc->roomflags & ROOMFLAGS_LANGUAGE_MASK) == strCurLanguage))
                {
                    if ((!someonecriteria[tryagain]) || ((tc->roomfullness >= ROOM_MIN_THRESHOLD) && (tc->roomfullness <= ROOM_MAX_THRESHOLD)))
                    {
                        return i;
                    }
                }
            }
        }
    }

    return -1;
}

void cNotifyRoomListPresent(void)
{
    if (cChannelFSMState==CS_IsADefaultRoomThere)
    {
        sdword foundroom;

        tpLockChannelList();

        foundroom = FindADefaultRoom();
        if (foundroom < 0)
        {
            ResetChannel();
        }
        else
        {
            SetChannel(tpChannelList.tpChannels[foundroom].ChannelName,tpChannelList.tpChannels[foundroom].ChannelDescription);
        }
        tpUnLockChannelList();

    }
}

void cNotifyCurrentRoomPresent(int present)
{
    if (cChannelFSMState==CS_IsRoomThere)
    {
        if (present)
        {
            cChannelFSMState = CS_RoomConnecting;
            mgDisplayMessage(strGetString(strConnectingToChat));
            chatConnect(ChannelPassword);
            return;
        }
        else
        {
            cChannelFSMState = CS_RoomCreate;
            if (!titanStartChatServer(ChannelProtectedFlag ? ChannelPassword : NULL))
            {
                cChannelFSMState = CS_Idle;
            }
            mgDisplayMessage(strGetString(strStartingChat));
        }
    }
    else if (cChannelFSMState == CS_IsADefaultRoomThere)
    {
        if (present)
        {
            cChannelFSMState = CS_RoomConnecting;
            mgDisplayMessage(strGetString(strConnectingToChat));
            chatConnect(ChannelPassword);
        }
        else
        {
            SetChannel(L"Default",L"Default");
            cChannelFSMState = CS_IsRoomThere;
            titanRefreshRequest(NULL);
        }
    }
}

void titanStartChatReplyReceivedCB(short theStatus)
{
    if ((theStatus == StatusCommon_Success) || (theStatus == StatusDir_ServiceExists))
    {
        if (cChannelFSMState == CS_RoomCreate)
        {
            cChannelFSMState = CS_RoomConnecting;
            mgDisplayMessage(strGetString(strConnectingToChat));
            chatConnect(ChannelPassword);
        }
    }
    else
    {
        if (cChannelFSMState == CS_RoomCreate)
        {
            ResetChannel();
            mgDisplayMessage(strGetString(strFailedToCreateChat));
        }
    }
}

void cNotifyChatConnected(void)
{
    if (cChannelFSMState == CS_RoomConnecting)
    {
        cChannelFSMState = CS_RoomConnected;
        cChannelStateMachine();
        if (wcslen(RemoveGameRequest) >= 1)
        {
            titanRemoveGame(RemoveGameRequest);
            RemoveGameRequest[0] = 0;
        }
        return;
    }
}

void cNotifyDirStatus(short theStatus)
{
    if (theStatus==StatusCommon_Success)
    {
        // obsolete, use to be callback when directory is created
    }
}

void cNotifyGameCreationStatus(short theStatus)
{
    if (LANGame)
    {
        return;
    }
    // Called when the Routing Server replies to our game creation requests
    // Two possible status values:
    //  o StatusCommon_Success: everything's great
    //  o StatusRouting_ObjectAlreadyExists: the game name is already in use
    switch (theStatus)
    {
        case StatusCommon_Success:
            mgCreateGameConfirmed();
            break;

        case StatusRouting_ObjectAlreadyExists:
            mgGameAlreadyExists();
            break;

        default:
            break;
    }
}

void cNotifyChatConnectFailed(void)
{
    ResetChannel();

    if (cChannelFSMState == CS_RoomConnecting)
    {
        mgFailedToConnectToChannel(TRUE);
    }
    else
    {
        mgChatConnectionFailed();
    }
//    else if (cChannelFSMState == CS_ChatCreateConnecting)
//    {
//        mgFailedToConnectToChannel(FALSE);
//    }

    cChannelFSMState = CS_Idle;
}

void cNotifyChatBadResponse(void)
{
    ResetChannel();
    cChannelFSMState = CS_Idle;
}

#if 0  // Cat2: cNotifyRoomListPresent live code differs from binary
/*=== cNotifyRoomListPresent (0x4342E0) ===*/
void cNotifyRoomListPresent(void)
{
signed int DefaultRoom; // eax

if ( dword_7525E8 == 8 )
{
tpLockChannelList();
DefaultRoom = cFindDefaultRoom();
if ( DefaultRoom < 0 )
ResetChannel();
else
SetChannel(&word_BA4704 + 72 * DefaultRoom, (wchar_t *)&word_BA4744[72 * DefaultRoom]);
CxxRestoreUnhandledExceptionFilter();
}
}
#endif

#if 0  // Cat2: cNotifyCurrentRoomPresent live code differs from binary
/*=== cNotifyCurrentRoomPresent (0x434320) ===*/
void __cdecl cNotifyCurrentRoomPresent(int present)
{
if ( dword_7525E8 == 3 )
{
if ( present )
{
LABEL_3:
dword_7525E8 = 4;
mgDisplayMessage(dword_BB3DDC);
chatConnect(&word_A6B328);
return;
}
dword_7525E8 = 7;
if ( !sub_58AE40(dword_A6B324 != 0 ? (unsigned int)&word_A6B328 : 0) )
dword_7525E8 = -1;
mgDisplayMessage(dword_BB3DE4);
}
else if ( dword_7525E8 == 8 )
{
if ( present )
goto LABEL_3;
SetChannel(aDe, aDe);
dword_7525E8 = 3;
sub_58AD80(0);
}
}
#endif

#if 0  // Cat2: titanStartChatReplyReceivedCB live code differs from binary
/*=== titanStartChatReplyReceivedCB (0x4343D0) ===*/
void __cdecl titanStartChatReplyReceivedCB(__int16 theStatus)
{
if ( !theStatus || theStatus == -1203 )
{
if ( dword_7525E8 == 7 )
{
dword_7525E8 = 4;
mgDisplayMessage(dword_BB3DDC);
chatConnect(&word_A6B328);
}
}
else if ( dword_7525E8 == 7 )
{
ResetChannel();
mgDisplayMessage(dword_BB3E60);
}
}
#endif

#if 0  // Cat2: cNotifyChatConnectFailed live code differs from binary
/*=== cNotifyChatConnectFailed (0x4344B0) ===*/
void cNotifyChatConnectFailed(void)
{
ResetChannel();
if ( dword_7525E8 == 4 )
mgFailedToConnectToChannel();
else
mgChatConnectionFailed();
dword_7525E8 = -1;
}
#endif
