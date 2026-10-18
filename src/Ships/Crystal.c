/*=============================================================================
    Name    : Crystal.c
    Purpose : Crystal-related decompiled pseudocode reference blocks
=============================================================================*/


#if 0  // Cat2: Binary crystalDeleteEffect; live code in SalCapCorvette.c::crystalDeleteEffect and dock.c static (0x5BBBB0)
void __cdecl crystalDeleteEffect(struct Effect *effect)
{
  if ( effect )
  {
    etgEffectDelete(effect);
    univRemoveObjFromRenderList(effect);
    listDeleteNode((Node *)effect);
  }
}
#endif

#if 0  // Cat3: crystalBeginDeath (0x5bd5f0) — needs CrystalSpec struct (a1[1] raw accesses); clSetAttribute and ClearCrystalDockSlots needed (0x5bd5f0)
unsigned __int8 __cdecl crystalBeginDeath(Ship *a1, real32 a2)
{
  unsigned __int8 result; // al
  double v3; // st7

  result = LOBYTE(a1[1].?.force.y);
  if ( !result )
  {
    LOBYTE(a1[1].?.force.y) = 1;
    v3 = (double)ranRandom(10) * ((flt_8C53B4 - flt_8C53B0) * flt_72A4E0)
       + flt_8C53B0
       + universe.totaltimeelapsed;
    a1[1].?.coordsys.m11 = a2;
    a1[1].?.force.z = v3;
    clSetAttribute((CommandToDo *)a1, 1, 10);
    return (unsigned __int8)ClearCrystalDockSlots(a1);
  }
  return result;
}
#endif
