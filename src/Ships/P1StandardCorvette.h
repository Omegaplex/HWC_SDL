/*=============================================================================
    Name    : P1StandardCorvette.h
    Purpose : Definitions for P1StandardCorvette

    Created 5/06/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___P1_STANDARD_CORVETTE_H
#define ___P1_STANDARD_CORVETTE_H

#include "types.h"
#include "spaceobj.h"
#include "attack.h"

/*=============================================================================
    Defines:
=============================================================================*/

/*=============================================================================
    Types:
=============================================================================*/

typedef struct
{
    AttackSideStep attacksidestep;
    ubyte unk_08_167[0x160];
} P1StandardCorvetteSpec;

static_assert(offsetof(P1StandardCorvetteSpec, attacksidestep) == 0x0, "P1StandardCorvetteSpec layout mismatch");
static_assert(sizeof(P1StandardCorvetteSpec) == 0x168, "P1StandardCorvetteSpec size mismatch");

/*=============================================================================
    Public data:
=============================================================================*/

extern CustShipHeader P1StandardCorvetteHeader;

#endif //___P1_STANDARD_CORVETTE_H
