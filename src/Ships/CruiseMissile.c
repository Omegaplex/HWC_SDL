/*=============================================================================
    Name    : CruiseMissile.c
    Purpose : Specifics for the Cruise Missile

    Created by reconstruction from Cataclysm retail callbacks
=============================================================================*/

#include "types.h"
#include "spaceobj.h"
#include "AIShip.h"
#include "Collision.h"
#include "attack.h"
#include "camera.h"
#include "DefaultShip.h"

void seekdir(void);

static ubyte cruiseMissileStaticBlob = 0xFF;

static void CruiseMissileStaticInit(char *directory, char *filename, ShipStaticInfo *statinfo)
{
    (void)directory;
    (void)filename;

    statinfo->custstatinfo = &cruiseMissileStaticBlob;
    statinfo->canSpecialBandBoxFriendlies = FALSE;
    statinfo->unk4 = TRUE;
    statinfo->unk5 = FALSE;
    (void)beastWeaponInfoInit();
}

static bool32 CruiseMissileAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 gunRange)
{
    vector trajectory;

    aishipGetTrajectory(ship, target, &trajectory);
    if (RangeToTarget(ship, target, &trajectory) <= 5000.0f)
    {
        ship->specialFlags1C8 |= 0x6u;
        attackStraightForward(ship, target, gunRange, 0.0f);
    }
    else
    {
        aishipFlyToPointAvoidingObjsFunc(ship, &target->posinfo.position, 7u, -400.0f, &target->posinfo.velocity);
    }

    return TRUE;
}

CustShipHeader CruiseMissileHeader =
{
    sizeof(CruiseMissileSpecifics),
    CruiseMissileStaticInit,
    NULL,
    seekdir,
    NULL,
    NULL,
    seekdir,
    NULL,
    NULL,
    NULL,
    seekdir,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    CruiseMissileAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
