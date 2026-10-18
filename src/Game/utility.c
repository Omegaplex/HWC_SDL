/*=============================================================================
    Name    : utility.c
    Purpose : Decompiled functions from cataclysm.exe (utility module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include <string.h>

void *qmemcpy(void *dest, const void *src, size_t count)
{
    return memcpy(dest, src, count);
}


// ============================================================================
// Decompiled from cataclysm.exe @ 0x5A83A0
void utyRegistryOptionsRead(void)
{
  char *result; // eax
  char **i; // ebx
  const CHAR *v2; // [esp-1Ch] [ebp-B8h]
  HKEY hKey; // [esp+8h] [ebp-94h] BYREF
  DWORD cbData; // [esp+Ch] [ebp-90h] BYREF
  HKEY phkResult; // [esp+10h] [ebp-8Ch] BYREF
  DWORD Type; // [esp+14h] [ebp-88h] BYREF
  DWORD dwDisposition; // [esp+18h] [ebp-84h] BYREF
  BYTE Data[4]; // [esp+1Ch] [ebp-80h] BYREF

  scriptSetFileSystem((const char *)&Default, aCataclysmCfg, (const char **)&off_8C1478);
  sub_42BEB0(dword_88BE88);
  sub_424D80(dword_88BE8C);
  if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey, 0, 0xF003Fu, &phkResult) )
  {
    result = (char *)RegCreateKeyExA(
                       HKEY_LOCAL_MACHINE,
                       SubKey,
                       0,
                       (LPSTR)&Default,
                       0,
                       0xF003Fu,
                       0,
                       &phkResult,
                       &dwDisposition);
    if ( result )
      return result;
    RegCloseKey(phkResult);
  }
  result = off_8C0E58;
  for ( i = &off_8C0E58; result; i += 3 )
  {
    if ( !RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey, 0, 0xF003Fu, &hKey) )
    {
      v2 = *i;
      cbData = 128;
      if ( !RegQueryValueExA(hKey, v2, 0, &Type, Data, &cbData) )
      {
        if ( Type == 1 )
        {
          qmemcpy(i[2], Data, cbData);
        }
        else if ( Type == 4 )
        {
          *(_DWORD *)i[2] = *(_DWORD *)Data;
        }
      }
      RegCloseKey(hKey);
    }
    result = i[3];
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5A84F0
void utyRegistryOptionsWrite(void)
{
  FILE *v0; // edi
  char **v1; // esi
  char *v2; // eax
  char *result; // eax
  char **i; // esi
  char *v5; // eax
  HKEY hKey; // [esp+10h] [ebp-Ch] BYREF
  HKEY phkResult; // [esp+14h] [ebp-8h] BYREF
  DWORD dwDisposition; // [esp+18h] [ebp-4h] BYREF

  v0 = fopen(aCataclysmCfg, aWt);
  if ( v0 )
  {
    if ( off_8C1478 )
    {
      v1 = &off_8C1478;
      do
      {
        if ( v1[1] == (char *)scriptSetUdwordCB )
          fprintf(v0, "%s    %d\n", *v1, *(_DWORD *)v1[2]);
        else
          fprintf(v0, "%s    %s\n", *v1, v1[2]);
        v2 = v1[3];
        v1 += 3;
      }
      while ( v2 );
    }
    fclose(v0);
  }
  if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey, 0, 0xF003Fu, &phkResult) )
  {
    result = (char *)RegCreateKeyExA(
                       HKEY_LOCAL_MACHINE,
                       SubKey,
                       0,
                       (LPSTR)&Default,
                       0,
                       0xF003Fu,
                       0,
                       &phkResult,
                       &dwDisposition);
    if ( result )
      return result;
    RegCloseKey(phkResult);
  }
  loadedDevcaps = gDevcaps;
  result = off_8C0E58;
  loadedDevcaps2 = gDevcaps2;
  for ( i = &off_8C0E58; result; i += 3 )
  {
    if ( !RegOpenKeyExA(HKEY_LOCAL_MACHINE, SubKey, 0, 2u, &hKey) )
    {
      v5 = i[1];
      if ( v5 )
      {
        if ( v5 == (char *)1 )
          RegSetValueExA(hKey, *i, 0, 1u, (const BYTE *)i[2], strlen(i[2]) + 1);
      }
      else
      {
        RegSetValueExA(hKey, *i, 0, 4u, (const BYTE *)i[2], 4u);
      }
      RegCloseKey(hKey);
    }
    result = i[3];
  }
  return result;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x5AA0F0
void gameEndMultiplayerCleanup(void)
{
  if ( multiPlayerGame )
  {
    sub_451AF0(*(int *)sigsPlayerIndex);
    sub_42ED50();
    if ( TitanActive )
      titanPumpEngine();
    Sleep(0x64u);
    if ( TitanActive )
      sub_52CA90();
  }
}
