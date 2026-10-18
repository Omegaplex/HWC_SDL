/*============================================================================
 * TitanInterfaceC.stub.c
 * Dummy functions to simulate a WONnet connection that never works.
 *
 * Author:  Ted Cipicchio <ted@thereisnospork.com>
 * Created: Sat Oct 4 2003
 *==========================================================================*/

/* Define constants before including the header to avoid parse failures */
#ifndef MAX_PORTS
#define MAX_PORTS      10
#define MAX_IPS        (MAX_PORTS)
#define MAXIP_STRLEN   50
#endif
#ifndef MAX_PASSWORD_LEN
#define MAX_PASSWORD_LEN 20
#define MAX_TITAN_GAME_NAME_LEN 32
#endif
#define PATCHFAIL_UNABLE_TO_CONNECT 1
typedef char ipString[MAXIP_STRLEN];

#include "TitanInterfaceC.h"

/*----------------------------------------------------------------------------
 * Global Variables
 *--------------------------------------------------------------------------*/

wchar_t ChannelPassword[MAX_PASSWORD_LEN];
TPChannelList tpChannelList;
TPServerList tpServerList;
CaptainGameInfo tpGameCreated;
Address myAddress;
unsigned long DIRSERVER_PORTS[MAX_PORTS];
unsigned long PATCHSERVER_PORTS[MAX_PORTS];
ipString DIRSERVER_IPSTRINGS[MAX_IPS];
ipString PATCHSERVER_IPSTRINGS[MAX_IPS];
unsigned long HomeworldCRC[4];
wchar_t GameWereInterestedIn[MAX_TITAN_GAME_NAME_LEN];
void *GameWereInterestedInMutex = 0;


/*----------------------------------------------------------------------------
 * Functions
 *--------------------------------------------------------------------------*/

void titanGotNumUsersInRoomCB(const wchar_t *theRoomName, int theNumUsers)
{ }


unsigned long titanStart(unsigned long isLan, unsigned long isIP)
{ return 0; }


unsigned long titanCheckCanNetwork(unsigned long isLan, unsigned long isIP)
{ return 0; }


// --MikeN
// Call this method to begin shutdown of titan.  Parameters specify packet to send
// to connected client(s) (a shutdown message).  The callback titanNoClientsCB() will
// be invoked when complete.
void titanStartShutdown(unsigned long titanMsgType, const void* thePacket,
                        unsigned short theLen)
{
	/* Probably won't ever be called, but we'll be consistent anyway... */
	titanNoClientsCB();
}


void titanLeaveGameNotify(void)
{ }


void titanShutdown(void)
{ }


void titanRefreshRequest(char* theDir)
{ }


unsigned long titanReadyToStartGame(unsigned char *routingaddress)
{ return 0; }


unsigned long titanBehindFirewall(void)
{ return 0; }


void titanCreateGame(wchar_t *str, DirectoryCustomInfo* myInfo)
{ }


void titanRemoveGame(wchar_t *str)
{ }


void titanCreateDirectory(char *str, char* desc)
{ }


void titanSendLanBroadcast(const void* thePacket, unsigned short theLen)
{ }


void titanSendPacketTo(Address *address, unsigned char titanMsgType,
                       const void* thePacket, unsigned short theLen)
{ }


void titanBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen)
{ }


void titanAnyoneSendPacketTo(Address *address, unsigned char titanMsgType,
                       const void* thePacket, unsigned short theLen)
{ }


void titanAnyoneBroadcastPacket(unsigned char titanMsgType, const void* thePacket, unsigned short theLen)
{ }


void titanConnectToClient(Address *address)
{ }


int titanStartChatServer(wchar_t *password)
{ return 0; }


void titanSendPing(Address *address,unsigned int pingsizebytes)
{ }


void titanPumpEngine()
{ }


void titanSetGameKey(unsigned char *key)
{ }


const unsigned char *titanGetGameKey(void)
{ return 0; }


Address titanGetMyPingAddress(void)
{ return myAddress; }


int titanGetPatch(char *filename,char *saveFileName)
{
	titanGetPatchFailedCB(PATCHFAIL_UNABLE_TO_CONNECT);
	return PATCHFAIL_UNABLE_TO_CONNECT;
}


void titanReplaceGameInfo(wchar_t *str, DirectoryCustomInfo* myInfo, unsigned long replaceTimeout)
{ }


void chatConnect(wchar_t *password)
{ }


void chatClose(void)
{ }


void BroadcastChatMessage(unsigned short size, const void* chatData)
{ }


void SendPrivateChatMessage(unsigned long* userIDList, unsigned short numUsersInList,
                            unsigned short size, const void* chatData)
{ }


void authAuthenticate(char *loginName, char *password)
{ }


void authCreateUser(char *loginName, char *password)
{ }


void authChangePassword(char *loginName, char *oldpassword, char *newpassword)
{ }


int titanSaveWonstuff()
{ return 0; }


void titanWaitShutdown(void)
{ }


void titanConnectingCancelHit(void)
{ }

void titanSendPointMessage(int playerIndex, unsigned char *packet, unsigned long sizeofPacket)
{ }

void titanSendBroadcastMessage(unsigned char *packet, unsigned long sizeofPacket)
{ }

void titanAnyoneSendPointMessage(int playerIndex, unsigned char *packet, unsigned long sizeofPacket)
{ }

void titanAnyoneSendBroadcastMessage(unsigned char *packet, unsigned long sizeofPacket)
{ }

#if 0  // Decompiled pseudocode reference
/*=== titanConnectingCancelHit (0x58AB20) ===*/
void __cdecl titanConnectingCancelHit()
{
  if ( *(_DWORD *)(*(_DWORD *)titanInterface + 828) == 1 )
    *(_DWORD *)(*(_DWORD *)titanInterface + 828) = 0;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanSendPing (0x58AE20) ===*/
void __cdecl titanSendPing(Address *address, unsigned int pingsizebytes)
{
  sub_594D10(*(_DWORD **)titanInterface, (int)address, pingsizebytes);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanBehindFirewall (0x58AD60) ===*/
unsigned int __cdecl titanBehindFirewall()
{
  return *(_BYTE *)(*(_DWORD *)titanInterface + 59) != 0;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanRefreshRequest (0x58AD80) ===*/
void __cdecl titanRefreshRequest(char *theDir)
{
  if ( !theDir )
    sub_593E00(*(char **)titanInterface);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanRemoveGame (0x58ADC0) ===*/
void __cdecl titanRemoveGame(wchar_t *str)
{
  sub_594210(*(char **)titanInterface, str);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanStartChatServer (0x58AE40) ===*/
int __cdecl titanStartChatServer(int password)
{
  wchar_t *CurrentChannelDescription;
  wchar_t *CurrentChannel;

  CurrentChannelDescription = GetCurrentChannelDescription();
  CurrentChannel = GetCurrentChannel();
  return sub_594E00(*(_DWORD *)titanInterface, CurrentChannel, CurrentChannelDescription, password, 0, 0);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanSendPacketTo (0x58AE90) ===*/
void __cdecl titanSendPacketTo(int *address, char titanMsgType, int thePacket, int theLen)
{
  sub_595F10(*(_BYTE **)titanInterface, address, titanMsgType, thePacket, theLen, 0, 0);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanCheckCanNetwork (0x58AFC0) ===*/
signed int __cdecl titanCheckCanNetwork(int isLan, int isIP)
{
  _DWORD *iface;

  if ( *(_DWORD *)titanInterface )
    return 0;
  iface = operator_new(0xB64u);
  if ( iface )
    iface = sub_58B8A0(iface, isLan, isIP);
  *(_DWORD *)titanInterface = iface;
  if ( !CanNetwork(iface) )
  {
    if ( *(_DWORD *)titanInterface )
      (*(void (__stdcall **)(int))(**(_DWORD **)titanInterface + 4))(1);
    *(_DWORD *)titanInterface = 0;
    return 0;
  }
  if ( *(_DWORD *)titanInterface )
    (*(void (__stdcall **)(int))(**(_DWORD **)titanInterface + 4))(1);
  *(_DWORD *)titanInterface = 0;
  return 1;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanStartShutdown (0x58B1B0) ===*/
void __cdecl titanStartShutdown(char titanMsgType, int thePacket, int theLen)
{
  if ( *(_DWORD *)titanInterface )
    sub_59AB00(*(_BYTE **)titanInterface, titanMsgType, thePacket, theLen);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanShutdown (0x58B1D0) ===*/
int __cdecl titanShutdown()
{
  int result;

  result = *(_DWORD *)titanInterface;
  if ( *(_DWORD *)titanInterface )
  {
    seekdir();
    result = (*(int (__thiscall **)(_DWORD))(**(_DWORD **)titanInterface + 12))(*(_DWORD *)titanInterface);
    if ( *(_DWORD *)titanInterface )
      result = (*(int (__thiscall **)(_DWORD, int))(**(_DWORD **)titanInterface + 4))(*(_DWORD *)titanInterface, 1);
    *(_DWORD *)titanInterface = 0;
  }
  multiPlayerGame = 0;
  TitanActive = 0;
  dword_A6B2A0 = 0;
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanGetPatch (0x58B3C0) ===*/
int __cdecl titanGetPatch(char *filename, char *saveFileName)
{
  return sub_58C6E0(
           byte_BB2DA0,
           word_BAB7A0,
           filename,
           saveFileName,
           aPatchModifiedT,
           (int)titanPatchProgressCB,
           (int)titanGetPatchCompleteCB,
           titanGetPatchFailedCB,
           (int)&dword_A4C7D0);
}
#endif
#if 0  // Decompiled pseudocode reference
/*=== titanIOResolutionChange (0x5c9cd0) ===*/
_DWORD *__cdecl titanIOResolutionChange(_DWORD *a1)
{
  _DWORD *result; // eax
  int v2; // ecx
  int v3; // edx
  int v4; // [esp-4h] [ebp-294h]
  void (__cdecl __noreturn *v5[33])(int); // [esp+4h] [ebp-28Ch] BYREF
  char v6[64]; // [esp+88h] [ebp-208h] BYREF
  int v7[114]; // [esp+C8h] [ebp-1C8h] BYREF

  v7[0] = (int)jpeg_std_error(v5);
  v5[0] = titanExceptionThrow;
  if ( _setjmp3(v6, 0, v4) )
  {
    jpeg_destroy_decompress((int)v7);
    result = a1;
    a1[11] = 1;
  }
  else
  {
    ioResolutionChange();
    jpeg_mem_src((int)v7, a1[4]);
    jpeg_read_header(v7, 1);
    jpeg_start_decompress((int)v7);
    v2 = v7[28];
    v3 = v7[8];
    a1[1] = v7[27];
    a1[2] = v2;
    a1[12] = v3;
    result = (_DWORD *)jpeg_destroy_decompress((int)v7);
    a1[11] = 0;
  }
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanJpegCompress (0x5c9dc0) ===*/
int __cdecl titanJpegCompress(_DWORD *a1)
{
  int result; // eax
  int v2; // ecx
  int v3; // edx
  int v4; // edx
  int v5; // eax
  signed int v6; // ecx
  int v7; // edi
  int i; // eax
  HWND v9; // eax
  int v10; // [esp+0h] [ebp-23Ch]
  char v11[64]; // [esp+Ch] [ebp-230h] BYREF
  int v12[33]; // [esp+4Ch] [ebp-1F0h] BYREF
  int v13[90]; // [esp+D0h] [ebp-16Ch] BYREF
  int v14; // [esp+238h] [ebp-4h] BYREF

  if ( _setjmp3(v11, 0, v10) )
  {
    a1[11] = 1;
    return jpeg_destroy_decompress((int)v13);
  }
  else
  {
    memset(v13, 0, sizeof(v13));
    memset(v12, 0, sizeof(v12));
    v13[0] = (int)jpeg_std_error(v12);
    v12[0] = (int)titanExceptionThrow;
    jpeg_CreateCompress(v13);
    jpeg_mem_dest((int)v13, a1[3]);
    v2 = a1[1];
    v3 = a1[2];
    v13[9] = 2;
    v13[6] = v2;
    v13[7] = v3;
    v13[8] = 3;
    jpeg_set_defaults((int)v13);
    v4 = a1[6];
    v5 = a1[7];
    v13[43] = a1[5];
    v6 = a1[8];
    v13[45] = v4;
    v13[12] = 8;
    v13[44] = 1;
    v13[46] = v5;
    jpeg_set_quality(v13, v6, 1);
    jpeg_default_colorspace(v13);
    jpeg_start_compress(v13, 1);
    v7 = 3 * a1[1];
    for ( i = v13[54]; v13[54] < (unsigned int)v13[7]; i = v13[54] )
    {
      v14 = *a1 + v7 * i;
      jpeg_write_scanlines(v13, (int)&v14, 1u);
      v9 = (HWND)a1[9];
      if ( v9 )
        SendMessageA(v9, a1[10], v13[54], v13[7]);
    }
    jpeg_finish_compress(v13);
    result = jpeg_destroy_decompress((int)v13);
    a1[11] = 0;
  }
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanJpegDecompress (0x5c9f80) ===*/
int __cdecl titanJpegDecompress(char **a1)
{
  int result; // eax
  char **v2; // edi
  unsigned int v3; // ebx
  int v4; // edx
  const void **v5; // esi
  HWND v6; // eax
  int v7; // [esp-20h] [ebp-2C4h]
  char *v8; // [esp-Ch] [ebp-2B0h]
  int v9; // [esp-4h] [ebp-2A8h]
  void (__cdecl __noreturn *v10[33])(int); // [esp+Ch] [ebp-298h] BYREF
  char v11[64]; // [esp+90h] [ebp-214h] BYREF
  int v12[27]; // [esp+D0h] [ebp-1D4h] BYREF
  int v13; // [esp+13Ch] [ebp-168h]
  unsigned int v14; // [esp+140h] [ebp-164h]
  int v15; // [esp+148h] [ebp-15Ch]
  unsigned int v16; // [esp+158h] [ebp-14Ch]
  const void **v17; // [esp+298h] [ebp-Ch]
  WPARAM v18; // [esp+29Ch] [ebp-8h]
  char *v19; // [esp+2A0h] [ebp-4h]

  v12[0] = (int)jpeg_std_error(v10);
  v10[0] = titanExceptionThrow;
  if ( _setjmp3(v11, 0, v9) )
  {
    a1[11] = (char *)1;
    return jpeg_destroy_decompress((int)v12);
  }
  else
  {
    ioResolutionChange();
    v2 = a1;
    jpeg_mem_src((int)v12, (int)a1[4]);
    jpeg_read_header(v12, 1);
    jpeg_start_decompress((int)v12);
    v3 = v13 * v15;
    v4 = v12[1];
    v7 = v13 * v15;
    a1[12] = (char *)v12[8];
    v5 = (const void **)(*(int (__cdecl **)(int *, int, int, int))(v4 + 8))(v12, 1, v7, 1);
    v19 = *a1;
    v17 = v5;
    v18 = 0;
    if ( v16 < v14 )
    {
      while ( 1 )
      {
        jpeg_read_scanlines(v12, (int)v5, 1);
        qmemcpy(v19, *v5, v3);
        v6 = (HWND)a1[9];
        v19 += v3;
        if ( v6 )
        {
          v8 = a1[10];
          SendMessageA(v6, (UINT)v8, ++v18, v12[7]);
        }
        if ( v16 >= v14 )
          break;
        v5 = v17;
      }
      v2 = a1;
    }
    jpeg_finish_decompress(v12);
    result = jpeg_destroy_decompress((int)v12);
    v2[11] = 0;
  }
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== titanExceptionThrow (0x5c9da0) ===*/
void __cdecl __noreturn titanExceptionThrow(int a1)
{
  longjmp((int *)(*(_DWORD *)a1 + 132), 1);
}
#endif
