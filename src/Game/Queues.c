/*=============================================================================
    Name    : Queues.c
    Purpose : Decompiled functions from cataclysm.exe (Queues module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x573100
_DWORD *__cdecl ResetQueue(_DWORD *queue)
{
  _DWORD *result; // eax

  result = a1;
  a1[3] = 0;
  a1[4] = 0;
  a1[5] = 0;
  a1[6] = 0;
  a1[7] = 0;
  a1[8] = 0;
  return result;
}
