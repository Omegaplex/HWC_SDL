/*=============================================================================
    Name    : Ship.c
    Purpose : Decompiled functions from cataclysm.exe (Ship module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "ACV.h"
#include "BeastMothership.h"
#include "FontReg.h"
#include "HiveFrigate.h"
#include "SalCapCorvette.h"
#include "SaveGame.h"
#include "singleplayer.h"
#include "soundevent.h"
#include "utility.h"
#include "mainrgn.h"
#include "etg.h"
#include "objtypes.h"
#include "universe.h"

// Time range (seconds) for spawning the caught-in-gravwell effect on disabled ships (binary: 0x8C47C0/C4)
real32 shipDisabledEffectLifeLow  = 3.0f;
real32 shipDisabledEffectLifeHigh = 5.0f;

static const sdword shipUpgradeMaxLevels[TOTAL_NUM_SHIPS] =
{
    1, 1, 1, 1, 1, 1, 2, 2, 2, 0, 2, 2, 2, 2, 2, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 2, 2, 2, 2,
    2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0
};

static const udword shipUpgradeMasks[TOTAL_NUM_SHIPS] =
{
    0x00800001, 0x04000009, 0x000000A9, 0x00000040, 0x00000211, 0x000000B1, 0x02600100, 0x00200002,
    0x00200802, 0x00000000, 0x00200404, 0x00000000, 0x00300004, 0x00000000, 0x006C0000, 0x00000000,
    0x00340000, 0x00807000, 0x00300000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00200004, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00800001, 0x04000009, 0x00000001, 0x00020001, 0x00000001, 0x00000211, 0x00000001,
    0x00000001, 0x00000002, 0x00000802, 0x00000000, 0x00020002, 0x00008002, 0x00010004, 0x00110004,
    0x02000100, 0x000C0000, 0x00000000, 0x00140000, 0x00807000, 0x00110000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00010001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00080000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x01000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00800000,
    0x00000000, 0x00080000, 0x00000000, 0x00000000, 0x00800000, 0x00000000, 0x00020000, 0x00000000,
    0x00800000, 0x00080020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00010000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000
};

Resource *shipModuleSlotEffectCreate(sdword moduleType, vector *position);
void shipModuleSlotEffectDelete(Resource *effect);
void shipFreeModuleSlotResources(Ship *ship);
Ship *shipFreeModuleData(Ship *data, Ship *ship);
real32 mothershipGetSpeedBonus(Ship *ship);
real32 rammingFrigateGetSpeedBonus(Ship *ship);
void hrShutdownBackground(void);
extern sdword mouseCursorType;
extern void *gameStatsSnapshotBuffer;
extern fonthandle gameStatsOverlayLabelFont;
extern fonthandle gameStatsOverlayValueFont;
extern fonthandle gameStatsOverlayHeaderFont;
void telldir(void);
bool32 univIsPositionInBounds(vector *position);

bool32 shipIsCloaking(Ship *ship)
{
    return ship->unk_590 == 1;
}

// Cataclysm-specific: beast mothership gains a speed bonus only while the live
// runtime system state is active and set to the energy system.
real32 mothershipGetSpeedBonus(Ship *ship)
{
    static const real32 bMothershipThrustSpeedBonus = 0.2f;
    const BeastMothershipRuntimeState *runtimeState = bMothershipRuntimeStateConst(ship);

    if (runtimeState->infectionLevel != 0 &&
        runtimeState->systemType == BS_Thrust)
    {
        return bMothershipThrustSpeedBonus;
    }

    return 0.0f;
}

// Cataclysm-specific: retail ramming frigates get an extra speed tier while
// their ram mode is active and the engine upgrade is present.
real32 rammingFrigateGetSpeedBonus(Ship *ship)
{
    static const real32 rammingFrigateEngineSpeedBonus = 0.6f;
    const RammingFrigateSpecifics *specifics = (const RammingFrigateSpecifics *)ship->ShipSpecifics;

    if ((ship->shiptype == bRammingFrigate || ship->shiptype == sRammingFrigate) &&
        specifics->ramState == 3 &&
        ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_AFTERBURNER))
    {
        return rammingFrigateEngineSpeedBonus;
    }

    return 0.0f;
}

// Cataclysm-specific: returns the retail ship speed scalar used by movement and physics.
real64 shipGetMaxSpeed(Ship *ship)
{
    static const real32 speedClassTable[] =
    {
        1.0f,
        1.1f,
        1.2f,
        1.3f,
        1.4f,
    };
    udword speedStatusFlags = ship->upgradeState.mask;
    real32 baseSpeed;
    real32 healthRatio;

    dbgAssert(ship->experienceLevel < (sizeof(speedClassTable) / sizeof(speedClassTable[0])));
    baseSpeed = speedClassTable[ship->experienceLevel];

    if ((speedStatusFlags & (1 | 2 | 4)) != 0)
    {
        baseSpeed += 0.1f;
    }

    if (ship->shiptype == bMothership)
    {
        baseSpeed += mothershipGetSpeedBonus(ship);
    }

    baseSpeed += rammingFrigateGetSpeedBonus(ship);

    healthRatio = ship->health / ship->staticinfo->maxhealth;
    if (healthRatio < 0.15f)
    {
        return baseSpeed - 0.6f;
    }
    if (healthRatio < 0.25f)
    {
        return baseSpeed - 0.4f;
    }
    if (healthRatio < 0.5f)
    {
        return baseSpeed - 0.2f;
    }
    if (healthRatio < 0.75f)
    {
        return baseSpeed - 0.15f;
    }

    return baseSpeed;
}

void *GetShipInfectionData(Ship *ship)
{
    return ship->ShipSpecifics;
}


sdword shipGetModuleFlashLightIndex(Ship *ship)
{
    ShipType shipType;

    if (ship->objtype != OBJ_ShipType || (ship->flags2 & BIT3) != BIT3)
    {
        shipType = ship->shiptype;
    }
    else
    {
        shipType = shipModuleSlotRuntime(ship)->activeBuildTarget.buildTargetType;
    }

    return GetShipStaticInfo(shipType, GetValidRaceForShipType(shipType))->shipclass;
}

static bool32 shipCataBusyStateWindowHasEntries(const Ship *ship)
{
    sdword i;

    for (i = 0; i < SHIP_CATA_BUSY_STATE_WORD_COUNT; i++)
    {
        if (ship->cataBusyStateWindow.words[i] != 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}


static void shipEndDisableState(Ship *ship)
{
    telldir();
    ship->cataBusyStateWindow.words[SHIP_CATA_BUSY_STATE_DISABLE_LATCH_INDEX] = 0;
    ship->flags = (ship->flags & 0xFF5FFFFF) |
        (((ship->disableStateSavedFlag21 & 1) | (4 * (ship->disableStateSavedFlag23 & 1))) << 21);
    ship->autostabilizeship = (bool8)ship->disableStateSavedAutostabilize;
    if (ship->posinfo.velocity.x != 0.0f ||
        ship->posinfo.velocity.y != 0.0f ||
        ship->posinfo.velocity.z != 0.0f)
    {
        ship->autostabilizeship = TRUE;
    }

    ship->disableStateActive = FALSE;
    if (ship->disableStateStarted == 1)
    {
        ship->disableStateStarted = 0;
    }
}

void shipUpdateFreezeState(Ship *ship)
{
    const real32 cataDisableVelocityScale = 0.9f;

    if (ship->velocityFreezeActive == 1 &&
        (!univIsPositionInBounds((vector *)&ship->posinfo.position) ||
         TimerExpired((int)&ship->velocityFreezeTimer)))
    {
        if (ship->posinfo.velocity.x != 0.0f ||
            ship->posinfo.velocity.y != 0.0f ||
            ship->posinfo.velocity.z != 0.0f)
        {
            ship->autostabilizeship = TRUE;
        }
        ship->velocityFreezeActive = FALSE;
    }

    if (ship->disableStateActive == 1)
    {
        if (TimerExpired((int)&ship->disableStateTimer))
        {
            if (ship->disableStateActive)
            {
                shipEndDisableState(ship);
            }
        }
        else
        {
            udword disableContext;

            if (ship->disableStateStarted == 1)
            {
                ship->posinfo.velocity.x = cataDisableVelocityScale * ship->posinfo.velocity.x;
                ship->posinfo.velocity.y = cataDisableVelocityScale * ship->posinfo.velocity.y;
                ship->posinfo.velocity.z = cataDisableVelocityScale * ship->posinfo.velocity.z;
            }

            if (!singlePlayerGame)
            {
                disableContext = (udword)ship->command;
                if (disableContext != 0 &&
                    *(uword *)(disableContext + 20) == 10 &&
                    ship->disableStateActive)
                {
                    shipEndDisableState(ship);
                }
            }
        }
    }
}

bool32 shipIsAliveCarrierType(Ship *ship)
{
    return ship->objtype == OBJ_ShipType &&
           (ship->shiptype == sWorker || ship->shiptype == bWorker);
}
