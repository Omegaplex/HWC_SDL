/*=============================================================================
    Name    : Light.c
    Purpose : Decompiled functions from cataclysm.exe (Light module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x55F2A0
void __cdecl lightSetAmbient(sdword index, real32 *ambient)
{
  _DWORD *result; // eax

  result = (_DWORD *)currentLight;
  if ( a1 )
    result = &unk_A74CB0;
  if ( result )
  {
    result[13] = *a2;
    result[14] = a2[1];
    result[15] = a2[2];
    result[16] = a2[3];
  }
  return result;
}
