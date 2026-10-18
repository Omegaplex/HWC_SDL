/*=============================================================================
    Name    : formation.c
    Purpose : Implementation of formations

    Created 10/20/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "types.h"
#include "memory.h"
#include "debug.h"
#include "fastmath.h"
#include "spaceobj.h"
#include "aiship.h"
#include "aitrack.h"
#include "Physics.h"
#include "shipselect.h"
#include "formation.h"
#include "commandlayer.h"
#include "universe.h"
#include "univupdate.h"
#include "statscript.h"
#include "proximitysensor.h"
#include "DDDFrigate.h"
#include "Ship.h"
#include "tactics.h"
#include "strings.h"
#include "salcapcorvette.h"
#include "ShipDefs.h"
#include "singleplayer.h"
//real32 DELTA_FORMATION_PADDING = 50.0f;
//real32 BROAD_FORMATION_PADDING = 50.0f;
real32 DELTA_FORMATION_PADDING_SCALE = 1.5f;
real32 BROAD_FORMATION_PADDING_SCALE = 1.5f;
real32 CLAW_FORMATION_PADDING_SCALE = 1.5f;
real32 DELTA3D_FORMATION_PADDING_SCALE = 1.5f;
real32 WALL_FORMATION_PADDING_SCALE = 2.0f;

real32 WALL_FORMATION_PADDING = 20.0f;
real32 SPHERE_FORMATION_PADDING_SCALE = 4.0f;
real32 CLAW_FORMATION_CURVATURE = 1.1f;
real32 SPHERE_MINIMUM_RADIUS = 500.0f;
real32 SPHERE_MAXIMUM_RADIUS = 3500.0f;
real32 SPHERESIZE_IGNORE_NUMDECLINATIONS = 600.0f;
real32 SENTINEL_FORMATION_B_SPACING = 300.0f;
real32 SENTINEL_FORMATION_RADIUS_SCALE = 1.1f;
real32 SENTINEL_SPECIAL_RADIUS_SCALE = 0.9f;
real32 SENTINEL_HEX_RING_SCALE = 3.0f;

real32 STRIKECRAFT_PADDING_MODIFIER = 1.2f;

real32 FORMATION_TRAVELVEL_MAXSCALE = 1.0f;
real32 FORMATION_TRAVELVEL_MINSCALE = 0.7f;
real32 FORMATION_ERROR_BIG = 10000.0f;
real32 FORMATION_ERROR_NEGLIGIBLE = 100.0f;

real32 PARADE_CONSIDERED_STEADY_VELSQR = 2500.0f;

bool8 FORMATION_TRAVEL_SLOWEST_SHIP = TRUE;

real32 TARGETDRONE_FORMATION_SIZE = 1000.0f;

static NumStrXlate formationtypeinfo[] =
{
    { (uword)DELTA_FORMATION,  str$(DELTA_FORMATION) },
    { (uword)BROAD_FORMATION, str$(BROAD_FORMATION) },
    { (uword)DELTA3D_FORMATION, str$(DELTA3D_FORMATION) },
    { (uword)CLAW_FORMATION, str$(CLAW_FORMATION) },
    { (uword)WALL_FORMATION, str$(WALL_FORMATION) },
    { (uword)SPHERE_FORMATION, str$(SPHERE_FORMATION) },
    { (uword)CUSTOM_FORMATION, str$(CUSTOM_FORMATION) },
    { (uword)FORCEFIELD_WALL_FORMATION, str$(FORCEFIELD_WALL_FORMATION) },
    { (uword)FORCEFIELD_SPHERE_FORMATION, str$(FORCEFIELD_SPHERE_FORMATION) },
    { (uword)NO_FORMATION, str$(NO_FORMATION) },
    { (uword)SAME_FORMATION, str$(SAME_FORMATION) },
    { 0,NULL }
};

static NumStrXlate formationtypeniceinfo[] =
{
    { (uword)DELTA_FORMATION,  "Delta Wing" },
    { (uword)BROAD_FORMATION, "Broad Attack" },
    { (uword)DELTA3D_FORMATION, "Formation X" },
    { (uword)CLAW_FORMATION, "The Claw" },
    { (uword)WALL_FORMATION, "The Wall" },
    { (uword)SPHERE_FORMATION, "Sphere" },
    { (uword)CUSTOM_FORMATION, "Custom" },
    { (uword)FORCEFIELD_WALL_FORMATION, "Wall Force Field" },
    { (uword)FORCEFIELD_SPHERE_FORMATION, "Sphere Force Field" },
    { 0,NULL }
};

scriptEntry FormationTweaks[] =
{
    makeEntry(FORMATION_TRAVEL_SLOWEST_SHIP,scriptSetBool8),
    makeEntry(DELTA_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(BROAD_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(DELTA3D_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(CLAW_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(CLAW_FORMATION_CURVATURE,scriptSetReal32CB),
    makeEntry(WALL_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(WALL_FORMATION_PADDING,scriptSetReal32CB),
    makeEntry(SPHERE_FORMATION_PADDING_SCALE,scriptSetReal32CB),
    makeEntry(SPHERE_MINIMUM_RADIUS,scriptSetReal32CB),
    makeEntry(SPHERE_MAXIMUM_RADIUS,scriptSetReal32CB),
    makeEntry(STRIKECRAFT_PADDING_MODIFIER,scriptSetReal32CB),
    makeEntry(FORMATION_ERROR_CALCULATE_RATE,scriptSetSdwordCB),
    makeEntry(FORMATION_TRAVELVEL_MAXSCALE,scriptSetReal32CB),
    makeEntry(FORMATION_TRAVELVEL_MINSCALE,scriptSetReal32CB),
    makeEntry(FORMATION_ERROR_BIG,scriptSetReal32CB),
    makeEntry(FORMATION_ERROR_NEGLIGIBLE,scriptSetReal32CB),
    makeEntry(TARGETDRONE_FORMATION_SIZE,scriptSetReal32CB),
    makeEntry(SPHERESIZE_IGNORE_NUMDECLINATIONS,scriptSetReal32CB),
    endEntry
};

static const matrix cataclysmSentinelIdentityMatrix =
{
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

static SphereTableEntry *cataclysmSentinelSphereTableEntry = NULL;

udword formationSortType[FORMATION_TYPE_COUNT] =
{
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //DELTA_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //BROAD_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //DELTA3D_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //CLAW_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //WALL_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //SPHERE_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //CUSTOM_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //NO_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //SAME_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST,      //FORCEFIELD_WALL_FORMATION
    FORMATION_SORT_BIGGEST_THEN_CLOSEST       //FORCEFIELD_SPHERE_FORMATION
};

//prototypes!
void formationArrageCrazyOptimum(CommandToDo *formationcommand);
void clSetAttribute(Ship *ship, sdword value, sdword attribute);
void formationInit(CommandToDo *command, TypeOfFormation formationType, ShipPtr leader, bool32 resetLeader);

static void formationBuildSentinelLinkInfo(SentinelSpecifics *specifics, SelectCommand *selection, sdword startIndex, sdword endIndex);
static void formationSetSentinelLevel(SelectCommand *selection);
static void formationComputeSentinelOffsetsA(CommandToDo *formationtodo, sdword setSentinelLevels);
static void formationComputeSentinelOffsetsB(CommandToDo *formationtodo);
static bool32 formationAllShipsAreSentinels(SelectCommand *selection);
static real32 formationGetSentinelExtent(Ship *ship);
static real32 formationGetSentinelAttackExtent(Ship *ship);
static real32 formationGetSentinelTacticExtent(Ship *ship);
static Ship *formationGetSentinelAttackAnchor(Ship *ship);

bool32 isCapitalShipStaticOrBig(ShipStaticInfo *shipstatic)
{
    if (isCapitalShipStatic(shipstatic))
    {
        return TRUE;
    }

    return ShipTypeIsCloakGeneratorType(shipstatic->shiptype) ||
           ShipTypeIsGravWellGeneratorType(shipstatic->shiptype);
}

/*-----------------------------------------------------------------------------
    Name        : TypeOfFormationToStr
    Description : converts TypeOfFormation to its string equivalent
    Inputs      : formation
    Outputs     :
    Return      : string representing formation
----------------------------------------------------------------------------*/
char *TypeOfFormationToStr(TypeOfFormation formation)
{
    return NumToStr(formationtypeinfo,(uword)formation);
}

/*-----------------------------------------------------------------------------
    Name        : StrToTypeOfFormation
    Description : converts from string to TypeOfFormation
    Inputs      : str
    Outputs     :
    Return      : returns TypeOfFormation enumeration based on string
----------------------------------------------------------------------------*/
TypeOfFormation NiceStrToTypeOfFormation(char *str)
{
    return StrToNum(formationtypeniceinfo,str);
}

/*-----------------------------------------------------------------------------
    Name        : TypeOfFormationToStr
    Description : converts TypeOfFormation to its string equivalent
    Inputs      : formation
    Outputs     :
    Return      : string representing formation
----------------------------------------------------------------------------*/
char *TypeOfFormationToNiceStr(TypeOfFormation formation)
{
    if (formation == -1)
    {
        return strGetString(strPARADE_FORMATION);
    }
    return (strGetString(formation+strFormationOffset));
}

/*-----------------------------------------------------------------------------
    Name        : StrToTypeOfFormation
    Description : converts from string to TypeOfFormation
    Inputs      : str
    Outputs     :
    Return      : returns TypeOfFormation enumeration based on string
----------------------------------------------------------------------------*/
TypeOfFormation StrToTypeOfFormation(char *str)
{
    return StrToNum(formationtypeinfo,str);
}

typedef struct
{
    real32 distance;
    ShipPtr ShipPtr;
} ShipDistInfo;

int compareSizeThenDist(const void *arg1,const void *arg2)
{
    ShipDistInfo *a1 = (ShipDistInfo *)arg1;
    ShipDistInfo *a2 = (ShipDistInfo *)arg2;
    real32 size1 = a1->ShipPtr->staticinfo->staticheader.staticCollInfo.collspheresize;
    real32 size2 = a2->ShipPtr->staticinfo->staticheader.staticCollInfo.collspheresize;

    if (size1 == size2)
    {
        if (a1->distance < a2->distance)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        if (size1 > size2)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}

void formationBiggestThenClosestOptimum(SelectCommand *selection,TypeOfFormation formationtype)
{
    sdword numShips = selection->numShips;
    vector avgpos = { 0.0f,0.0f,0.0f };

    real32 contender;
    real32 biggestship = 0.0f;
    real32 smallestship = 1000000.0f;
    sdword biggestshipindex = -1;
    real32 temp;

    ShipDistInfo *shipdist;
    Ship *tempship;
    sdword i,cloakinsel,cloakindex;
    vector diff;

    for (i=0;i<numShips;i++)
    {
        contender = (selection->ShipPtr[i])->staticinfo->staticheader.staticCollInfo.collspheresize;
        if (contender > biggestship)
        {
            biggestship = contender;
            biggestshipindex = i;
        }
        if (contender < smallestship)
        {
            smallestship = contender;
        }
    }

    dbgAssert(biggestshipindex >= 0);

    if (smallestship == biggestship)        // all ships are the same size
    {
        for (i=0;i<numShips;i++)            // so find average position of all ships
        {
            vecAddTo(avgpos,(selection->ShipPtr[i])->posinfo.position);
        }
        vecDivideByScalar(avgpos,(real32)numShips,temp);
    }
    else                                    // ships are not same size
    {
        // so let average position be position of biggest ship
        avgpos = (selection->ShipPtr[biggestshipindex])->posinfo.position;
    }

    shipdist = memAlloc(sizeof(ShipDistInfo)*selection->numShips,"TmpShipDistArray",0);

    for (i=0;i<selection->numShips;i++)
    {
        vecSub(diff,(selection->ShipPtr[i])->posinfo.position,avgpos);
        shipdist[i].distance = vecMagnitudeSquared(diff);
        shipdist[i].ShipPtr = selection->ShipPtr[i];
    }

    qsort(shipdist,selection->numShips,sizeof(ShipDistInfo),compareSizeThenDist);

    for (i=0;i<selection->numShips;i++)
    {
        selection->ShipPtr[i] = shipdist[i].ShipPtr;
    }

    cloakinsel = FALSE;
    for (i=0;i<selection->numShips;i++)
    {
        if (ShipTypeIsCloakGeneratorType(selection->ShipPtr[i]->shiptype))
        {
            cloakinsel = TRUE;
            cloakindex = i;
        }
    }

    if(cloakinsel)
    {
        //go through selection, and move any cloak generator to the
        //leader position, that way we cover other ships with cloaking
        //field more effectivly
        tempship = selection->ShipPtr[0];
        selection->ShipPtr[0] = selection->ShipPtr[cloakindex];
        selection->ShipPtr[cloakindex] = tempship;
    }

    memFree(shipdist);

}

void formationArrangeOptimum(struct CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;

    dbgAssert(numShips >= ABSOLUTE_MIN_SHIPS_IN_FORMATION);

    if (selection->numShips == 1)
    {
        return;     // only 1 item, so don't sort
    }

    switch (formationSortType[formationtodo->formation.formationtype])
    {
        case FORMATION_SORT_BIGGEST_THEN_CLOSEST:
            formationBiggestThenClosestOptimum(selection,formationtodo->formation.formationtype);
            formationtodo->formation.sortorder = FORMATION_SORT_BIGGEST_THEN_CLOSEST;
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

#define NUM_SPHERE_TABLE_ENTRYS     50

struct SphereStaticInfo *createSphereStaticInfo(void)
{
    sdword sizeofspherestaticinfo;
    SphereStaticInfo *sphereStaticInfo;
    sdword tableindex;
    SphereTableEntry *tableentry;
    sdword sizeofSphereTableEntry;
    sdword numDeclinations;
    sdword numCircleDivisions;
    real32 angleDivision;
    real32 angle;
    real32 rotang;
    sdword totalships;
    sdword numships;
    sdword i;
    SphereDeclination *sphereDeclination;

    sizeofspherestaticinfo = sizeofSphereStaticInfo(NUM_SPHERE_TABLE_ENTRYS);

    sphereStaticInfo = memAlloc(sizeofspherestaticinfo,"spherestatinfo",NonVolatile);
    memset(sphereStaticInfo,0,sizeofspherestaticinfo);

    sphereStaticInfo->numTableEntries = NUM_SPHERE_TABLE_ENTRYS;

    for (tableindex=0;tableindex<NUM_SPHERE_TABLE_ENTRYS;tableindex++)
    {
        numDeclinations = tableindex+1;
        numCircleDivisions = numDeclinations + numDeclinations + 2;
        angleDivision = TWOPI / ((real32)numCircleDivisions);

        sizeofSphereTableEntry = sizeofSphereTableEntry(numDeclinations);
        tableentry = memAlloc(sizeofSphereTableEntry,"spheretabentry",NonVolatile);
        memset(tableentry,0,sizeofSphereTableEntry);

        sphereStaticInfo->sphereTableEntryPtrs[tableindex] = tableentry;

        totalships = 0;
        tableentry->numDeclinations = numDeclinations;
        angle = PI / 2.0f;     // 90 degrees

        for (i=0,sphereDeclination=&tableentry->sphereDeclinations[0];i<numDeclinations;i++,sphereDeclination++)
        {
            angle -= angleDivision;
            numships = (sdword) (cos(angle) * (real32)numCircleDivisions);
            totalships += numships;

            sphereDeclination->cosdeclination = (real32)cos(angle);
            sphereDeclination->sindeclination = (real32)sin(angle);
            sphereDeclination->numAnglePoints = numships;
            rotang = TWOPI / (real32)numships;
            sphereDeclination->cosrotang = (real32)cos(rotang);
            sphereDeclination->sinrotang = (real32)sin(rotang);
        }

        tableentry->numShipsCanHandle = totalships + 3;   // leader goes in center, two ships at poles (+3)

        if (tableentry->numShipsCanHandle == 13)
        {
            cataclysmSentinelSphereTableEntry = tableentry;
        }
    }

    if (cataclysmSentinelSphereTableEntry == NULL)
    {
        dbgFatal(DBG_Loc, "Cataclysm Sentinel sphere layout needs a 13-ship sphere table entry");
    }

    return sphereStaticInfo;
}

/*-----------------------------------------------------------------------------
    Name        : freeSphereStaticInfo
    Description : frees sphereStaticInfo
    Inputs      : sphereStaticInfo
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void freeSphereStaticInfo(struct SphereStaticInfo *sphereStaticInfo)
{
    sdword numTableEntrys = sphereStaticInfo->numTableEntries;
    sdword i;

    for (i=0;i<numTableEntrys;i++)
    {
        memFree(sphereStaticInfo->sphereTableEntryPtrs[i]);
    }

    memFree(sphereStaticInfo);
}

SphereTableEntry *findSphereTableEntry(sdword numShips)
{
    sdword tableindex;
    sdword numTables = sphereStaticInfo->numTableEntries;
    SphereTableEntry *tableentry;

    for (tableindex=0;tableindex<numTables;tableindex++)
    {
        tableentry = sphereStaticInfo->sphereTableEntryPtrs[tableindex];

        if (numShips <= tableentry->numShipsCanHandle)
        {
            return tableentry;
        }
    }
    dbgFatal(DBG_Loc,"\nSphere formation too big");
}

static bool32 formationAllShipsAreSentinels(SelectCommand *selection)
{
    sdword i;

    for (i = 0; i < selection->numShips; i++)
    {
        ShipType shiptype = selection->ShipPtr[i]->shiptype;
        if (shiptype != sSentinel && shiptype != bSentinel)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static real32 formationGetSentinelExtent(Ship *ship)
{
    return ((real32 *)ship->staticinfo->custstatinfo)[2];
}

static real32 formationGetSentinelAttackExtent(Ship *ship)
{
    return ship->staticinfo->forceFieldProtectionDistanceScale *
        ship->staticinfo->staticheader.staticCollInfo.avoidcollspheresize;
}

static real32 formationGetSentinelTacticExtent(Ship *ship)
{
    return ship->staticinfo->bulletRange[ship->tacticstype];
}

static Ship *formationGetSentinelAttackAnchor(Ship *ship)
{
    return (Ship *)ship->attackvars.attacktarget;
}

static void formationBuildSentinelLinkInfo(SentinelSpecifics *specifics, SelectCommand *selection, sdword startIndex, sdword endIndex)
{
    specifics->formationLinkCount = 0;

    switch (endIndex - startIndex)
    {
        case 1:
            return;
        case 2:
            specifics->formationLinkCount = 1;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 1];
            break;
        case 3:
            specifics->formationLinkCount = 1;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 2];
            break;
        case 4:
            specifics->formationLinkCount = 1;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 3];
            break;
        case 5:
            specifics->formationLinkCount = 2;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 4];
            specifics->formationLinks[2] = selection->ShipPtr[startIndex + 1];
            specifics->formationLinks[3] = selection->ShipPtr[startIndex];
            break;
        case 6:
            specifics->formationLinkCount = 1;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex + 1];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 5];
            break;
        case 7:
            specifics->formationLinkCount = 2;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex + 1];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 6];
            specifics->formationLinks[2] = selection->ShipPtr[startIndex + 2];
            specifics->formationLinks[3] = selection->ShipPtr[startIndex + 1];
            break;
        case 8:
            specifics->formationLinkCount = 2;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex + 2];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 7];
            specifics->formationLinks[2] = selection->ShipPtr[startIndex + 3];
            specifics->formationLinks[3] = selection->ShipPtr[startIndex + 2];
            break;
        case 9:
            specifics->formationLinkCount = 2;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex + 4];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 3];
            specifics->formationLinks[2] = selection->ShipPtr[startIndex + 3];
            specifics->formationLinks[3] = selection->ShipPtr[startIndex + 8];
            break;
        case 10:
            specifics->formationLinkCount = 3;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex + 5];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 4];
            specifics->formationLinks[2] = selection->ShipPtr[startIndex + 4];
            specifics->formationLinks[3] = selection->ShipPtr[startIndex + 9];
            specifics->formationLinks[4] = selection->ShipPtr[startIndex + 6];
            specifics->formationLinks[5] = selection->ShipPtr[startIndex + 5];
            break;
        case 11:
            specifics->formationLinkCount = 5;
            specifics->formationLinks[0] = selection->ShipPtr[startIndex + 6];
            specifics->formationLinks[1] = selection->ShipPtr[startIndex + 7];
            specifics->formationLinks[2] = selection->ShipPtr[startIndex + 7];
            specifics->formationLinks[3] = selection->ShipPtr[startIndex + 8];
            specifics->formationLinks[4] = selection->ShipPtr[startIndex + 8];
            specifics->formationLinks[5] = selection->ShipPtr[startIndex + 9];
            specifics->formationLinks[6] = selection->ShipPtr[startIndex + 9];
            specifics->formationLinks[7] = selection->ShipPtr[startIndex + 10];
            specifics->formationLinks[8] = selection->ShipPtr[startIndex + 10];
            specifics->formationLinks[9] = selection->ShipPtr[startIndex + 6];
            break;
        default:
            dbgAssert(FALSE);
            break;
    }
}

static void formationSetSentinelLevel(SelectCommand *selection)
{
    sdword i;
    sdword level = 2;

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];
        ShipType shiptype = ship->shiptype;

        if (level == 2)
        {
            if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_3))
            {
                level = 1;
                if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_2))
                {
                    level = 0;
                }
            }
        }
        else if (level == 1)
        {
            if (!ShipHasUpgrade(ship, RESEARCH_UPGRADE_MASK_FORCE_FIELD_2))
            {
                level = 0;
            }
        }

        if (shiptype != sSentinel && shiptype != bSentinel)
        {
            dbgFatalf(DBG_Loc, "Can only have Sentinels in retail Sentinel formations");
        }
    }

    for (i = 0; i < selection->numShips; i++)
    {
        sentinelShipSpecifics(selection->ShipPtr[i])->formationLevel = level;
    }
}

static void formationComputeSentinelOffsetsA(CommandToDo *formationtodo, sdword setSentinelLevels)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    sdword formationShipCount = numShips;
    Ship *leader;
    sdword firstIndex;
    bool32 allSentinels;
    real32 leaderExtent;
    real32 radius;
    SphereTableEntry *tableEntry;
    sdword currentIndex;
    sdword declinationIndex;

    if (setSentinelLevels)
    {
        formationSetSentinelLevel(selection);
    }

    if (formationtodo->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        leader = formationtodo->protect->ShipPtr[0];
        firstIndex = 0;
        formationShipCount++;
    }
    else if (formationtodo->ordertype.order == COMMAND_ATTACK &&
             formationtodo->attack != NULL &&
             formationtodo->attack->numTargets > 0 &&
             formationtodo->formation.enders == TRUE)
    {
        Ship *attackAnchor = formationGetSentinelAttackAnchor(selection->ShipPtr[0]);
        leader = (attackAnchor != NULL) ? attackAnchor : (Ship *)formationtodo->attack->TargetPtr[0];
        firstIndex = 0;
        formationShipCount++;
    }
    else
    {
        leader = selection->ShipPtr[0];
        firstIndex = 1;
    }

    allSentinels = formationAllShipsAreSentinels(selection);
    if (leader != selection->ShipPtr[0] &&
        leader->shiptype != sSentinel &&
        leader->shiptype != bSentinel)
    {
        allSentinels = FALSE;
    }

    if (formationShipCount == 1)
    {
        return;
    }

    if (allSentinels &&
        (leader->shiptype == sSentinel || leader->shiptype == bSentinel))
    {
        leaderExtent = formationGetSentinelExtent(leader);
    }
    else
    {
        leaderExtent = leader->staticinfo->staticheader.staticCollInfo.collspheresize;
    }

    if (setSentinelLevels)
    {
        tableEntry = cataclysmSentinelSphereTableEntry;
    }
    else
    {
        tableEntry = findSphereTableEntry(formationShipCount);
    }

    if (allSentinels)
    {
        Ship *referenceShip = (selection->numShips > 1) ? selection->ShipPtr[1] : selection->ShipPtr[0];
        radius = leaderExtent + formationGetSentinelExtent(referenceShip);
    }
    else if (formationtodo->formation.enders == TRUE)
    {
        if (!setSentinelLevels)
        {
            sdword i;
            real32 maxExtent = 0.0f;

            for (i = 1; i < numShips; i++)
            {
                real32 tacticExtent = formationGetSentinelTacticExtent(selection->ShipPtr[i]);
                if (maxExtent < tacticExtent)
                {
                    maxExtent = tacticExtent;
                }
            }
            radius = maxExtent * SENTINEL_SPECIAL_RADIUS_SCALE;
        }
        else
        {
            radius = formationGetSentinelAttackExtent(leader);
        }
    }
    else if (setSentinelLevels)
    {
        radius = formationGetSentinelAttackExtent(leader);
    }
    else if (leaderExtent <= SPHERESIZE_IGNORE_NUMDECLINATIONS)
    {
        sdword minDeclinations = tableEntry->numDeclinations;
        if (minDeclinations < 2)
        {
            minDeclinations = 2;
        }
        radius = ((real32)minDeclinations) * SENTINEL_FORMATION_RADIUS_SCALE * leaderExtent;
    }
    else
    {
        radius = SENTINEL_FORMATION_RADIUS_SCALE * leaderExtent * 2.0f;
    }

    if (!setSentinelLevels && radius < SPHERE_MINIMUM_RADIUS)
    {
        radius = SPHERE_MINIMUM_RADIUS;
    }
    if (radius > SPHERE_MAXIMUM_RADIUS)
    {
        radius = SPHERE_MAXIMUM_RADIUS;
    }

    selection->ShipPtr[firstIndex]->formationOffset.x = 0.0f;
    selection->ShipPtr[firstIndex]->formationOffset.y = 0.0f;
    selection->ShipPtr[firstIndex]->formationOffset.z = radius;
    selection->ShipPtr[firstIndex]->formationHeading.x = 0.0f;
    selection->ShipPtr[firstIndex]->formationHeading.y = 0.0f;
    selection->ShipPtr[firstIndex]->formationHeading.z = 1.0f;

    if (setSentinelLevels)
    {
        Ship *ship = selection->ShipPtr[firstIndex];
        vector offset = ship->formationOffset;
        vector heading = ship->formationHeading;

        sentinelShipSpecifics(ship)->formationLinkCount = 0;
        matMultiplyVecByMat(&ship->formationOffset, &offset, (matrix *)&cataclysmSentinelIdentityMatrix);
        matMultiplyVecByMat(&ship->formationHeading, &heading, (matrix *)&cataclysmSentinelIdentityMatrix);
    }

    currentIndex = firstIndex + 1;
    if (currentIndex == numShips)
    {
        return;
    }

    if (!setSentinelLevels)
    {
        Ship *ship = selection->ShipPtr[currentIndex];
        ship->formationOffset.x = 0.0f;
        ship->formationOffset.y = 0.0f;
        ship->formationOffset.z = -radius;
        ship->formationHeading.x = 0.0f;
        ship->formationHeading.y = 0.0f;
        ship->formationHeading.z = -1.0f;
        currentIndex++;
        if (currentIndex == numShips)
        {
            return;
        }
    }

    for (declinationIndex = 0; declinationIndex < tableEntry->numDeclinations && currentIndex < numShips; declinationIndex++)
    {
        SphereDeclination *declination = &tableEntry->sphereDeclinations[declinationIndex];
        matrix rotation;
        vector direction;
        sdword pointIndex;

        direction.x = declination->cosdeclination;
        direction.y = 0.0f;
        direction.z = declination->sindeclination;
        matMakeRotAboutZ(&rotation, declination->cosrotang, declination->sinrotang);

        for (pointIndex = 0; pointIndex < declination->numAnglePoints && currentIndex < numShips; pointIndex++)
        {
            Ship *ship = selection->ShipPtr[currentIndex];

            ship->formationHeading = direction;
            ship->formationOffset.x = direction.x * radius;
            ship->formationOffset.y = direction.y * radius;
            ship->formationOffset.z = direction.z * radius;

            if (setSentinelLevels)
            {
                vector offset = ship->formationOffset;
                vector heading = ship->formationHeading;

                matMultiplyVecByMat(&ship->formationOffset, &offset, (matrix *)&cataclysmSentinelIdentityMatrix);
                matMultiplyVecByMat(&ship->formationHeading, &heading, (matrix *)&cataclysmSentinelIdentityMatrix);
                formationBuildSentinelLinkInfo(sentinelShipSpecifics(ship), selection, firstIndex, currentIndex);
            }

            currentIndex++;
            if (currentIndex < numShips && pointIndex + 1 < declination->numAnglePoints)
            {
                vector temp = direction;
                matMultiplyMatByVec(&direction, &rotation, &temp);
            }
        }
    }

    if (setSentinelLevels && currentIndex < numShips)
    {
        Ship *ship = selection->ShipPtr[currentIndex];
        vector offset;
        vector heading;

        ship->formationOffset.x = 0.0f;
        ship->formationOffset.y = 0.0f;
        ship->formationOffset.z = -radius;
        ship->formationHeading.x = 0.0f;
        ship->formationHeading.y = 0.0f;
        ship->formationHeading.z = -1.0f;

        formationBuildSentinelLinkInfo(sentinelShipSpecifics(ship), selection, firstIndex, currentIndex);
        offset = ship->formationOffset;
        heading = ship->formationHeading;
        matMultiplyVecByMat(&ship->formationOffset, &offset, (matrix *)&cataclysmSentinelIdentityMatrix);
        matMultiplyVecByMat(&ship->formationHeading, &heading, (matrix *)&cataclysmSentinelIdentityMatrix);
    }
}

static void formationComputeSentinelOffsetsB(CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    sdword currentIndex;
    sdword ringIndex;
    Ship *ship;
    SentinelSpecifics *specifics;
    real32 spacing = SENTINEL_FORMATION_B_SPACING;

    formationSetSentinelLevel(selection);

    ship = selection->ShipPtr[0];
    ship->formationOffset.x = 0.0f;
    ship->formationOffset.y = 0.0f;
    ship->formationOffset.z = 0.0f;
    sentinelShipSpecifics(ship)->formationLinkCount = 0;

    currentIndex = 1;
    ringIndex = 0;
    while (currentIndex < numShips && ringIndex < 4)
    {
        ship = selection->ShipPtr[currentIndex];
        specifics = sentinelShipSpecifics(ship);
        specifics->formationLinkCount = 0;

        switch (ringIndex)
        {
            case 0:
                ship->formationOffset.x = spacing;
                ship->formationOffset.y = 0.0f;
                ship->formationOffset.z = 0.0f;
                break;
            case 1:
                ship->formationOffset.x = 0.0f;
                ship->formationOffset.y = spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[1];
                specifics->formationLinks[1] = selection->ShipPtr[0];
                break;
            case 2:
                ship->formationOffset.x = -spacing;
                ship->formationOffset.y = 0.0f;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[2];
                specifics->formationLinks[1] = selection->ShipPtr[0];
                break;
            case 3:
                ship->formationOffset.x = 0.0f;
                ship->formationOffset.y = -spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[3];
                specifics->formationLinks[1] = selection->ShipPtr[0];
                specifics->formationLinks[2] = selection->ShipPtr[0];
                specifics->formationLinks[3] = selection->ShipPtr[1];
                break;
        }

        currentIndex++;
        ringIndex++;
    }

    ringIndex = 0;
    while (currentIndex < numShips && ringIndex < 8)
    {
        ship = selection->ShipPtr[currentIndex];
        specifics = sentinelShipSpecifics(ship);
        specifics->formationLinkCount = 0;

        switch (ringIndex)
        {
            case 0:
                ship->formationOffset.x = spacing;
                ship->formationOffset.y = spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[1];
                specifics->formationLinks[1] = selection->ShipPtr[2];
                break;
            case 1:
                ship->formationOffset.x = 0.0f;
                ship->formationOffset.y = spacing + spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[5];
                specifics->formationLinks[1] = selection->ShipPtr[2];
                break;
            case 2:
                ship->formationOffset.x = -spacing;
                ship->formationOffset.y = spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[2];
                specifics->formationLinks[1] = selection->ShipPtr[3];
                specifics->formationLinks[2] = selection->ShipPtr[6];
                specifics->formationLinks[3] = selection->ShipPtr[2];
                break;
            case 3:
                ship->formationOffset.x = -(spacing + spacing);
                ship->formationOffset.y = 0.0f;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[7];
                specifics->formationLinks[1] = selection->ShipPtr[3];
                break;
            case 4:
                ship->formationOffset.x = -spacing;
                ship->formationOffset.y = -spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[8];
                specifics->formationLinks[1] = selection->ShipPtr[3];
                specifics->formationLinks[2] = selection->ShipPtr[3];
                specifics->formationLinks[3] = selection->ShipPtr[4];
                break;
            case 5:
                ship->formationOffset.x = 0.0f;
                ship->formationOffset.y = -(spacing + spacing);
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[9];
                specifics->formationLinks[1] = selection->ShipPtr[4];
                break;
            case 6:
                ship->formationOffset.x = spacing;
                ship->formationOffset.y = -spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[10];
                specifics->formationLinks[1] = selection->ShipPtr[4];
                specifics->formationLinks[2] = selection->ShipPtr[4];
                specifics->formationLinks[3] = selection->ShipPtr[1];
                break;
            case 7:
                ship->formationOffset.x = spacing + spacing;
                ship->formationOffset.y = 0.0f;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[11];
                specifics->formationLinks[1] = selection->ShipPtr[1];
                specifics->formationLinks[2] = selection->ShipPtr[1];
                specifics->formationLinks[3] = selection->ShipPtr[5];
                break;
        }

        currentIndex++;
        ringIndex++;
    }

    ringIndex = 0;
    while (currentIndex < numShips && ringIndex < 12)
    {
        ship = selection->ShipPtr[currentIndex];
        specifics = sentinelShipSpecifics(ship);
        specifics->formationLinkCount = 0;

        switch (ringIndex)
        {
            case 0:
                ship->formationOffset.x = spacing + spacing;
                ship->formationOffset.y = spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[12];
                specifics->formationLinks[1] = selection->ShipPtr[5];
                break;
            case 1:
                ship->formationOffset.x = spacing;
                ship->formationOffset.y = spacing + spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[13];
                specifics->formationLinks[1] = selection->ShipPtr[5];
                specifics->formationLinks[2] = selection->ShipPtr[5];
                specifics->formationLinks[3] = selection->ShipPtr[6];
                break;
            case 2:
                ship->formationOffset.x = 0.0f;
                ship->formationOffset.y = spacing * SENTINEL_HEX_RING_SCALE;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[14];
                specifics->formationLinks[1] = selection->ShipPtr[6];
                break;
            case 3:
                ship->formationOffset.x = -spacing;
                ship->formationOffset.y = spacing + spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[6];
                specifics->formationLinks[1] = selection->ShipPtr[7];
                specifics->formationLinks[2] = selection->ShipPtr[15];
                specifics->formationLinks[3] = selection->ShipPtr[6];
                break;
            case 4:
                ship->formationOffset.x = -(spacing + spacing);
                ship->formationOffset.y = spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[16];
                specifics->formationLinks[1] = selection->ShipPtr[7];
                specifics->formationLinks[2] = selection->ShipPtr[7];
                specifics->formationLinks[3] = selection->ShipPtr[8];
                break;
            case 5:
                ship->formationOffset.x = -(spacing * SENTINEL_HEX_RING_SCALE);
                ship->formationOffset.y = 0.0f;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[17];
                specifics->formationLinks[1] = selection->ShipPtr[8];
                break;
            case 6:
                ship->formationOffset.x = -(spacing + spacing);
                ship->formationOffset.y = -spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[18];
                specifics->formationLinks[1] = selection->ShipPtr[8];
                specifics->formationLinks[2] = selection->ShipPtr[8];
                specifics->formationLinks[3] = selection->ShipPtr[9];
                break;
            case 7:
                ship->formationOffset.x = -spacing;
                ship->formationOffset.y = -(spacing + spacing);
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[19];
                specifics->formationLinks[1] = selection->ShipPtr[9];
                specifics->formationLinks[2] = selection->ShipPtr[9];
                specifics->formationLinks[3] = selection->ShipPtr[10];
                break;
            case 8:
                ship->formationOffset.x = 0.0f;
                ship->formationOffset.y = -(spacing * SENTINEL_HEX_RING_SCALE);
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 1;
                specifics->formationLinks[0] = selection->ShipPtr[20];
                specifics->formationLinks[1] = selection->ShipPtr[10];
                break;
            case 9:
                ship->formationOffset.x = spacing;
                ship->formationOffset.y = -(spacing + spacing);
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[21];
                specifics->formationLinks[1] = selection->ShipPtr[10];
                specifics->formationLinks[2] = selection->ShipPtr[10];
                specifics->formationLinks[3] = selection->ShipPtr[11];
                break;
            case 10:
                ship->formationOffset.x = spacing + spacing;
                ship->formationOffset.y = -spacing;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[22];
                specifics->formationLinks[1] = selection->ShipPtr[11];
                specifics->formationLinks[2] = selection->ShipPtr[11];
                specifics->formationLinks[3] = selection->ShipPtr[12];
                break;
            case 11:
                ship->formationOffset.x = spacing * SENTINEL_HEX_RING_SCALE;
                ship->formationOffset.y = 0.0f;
                ship->formationOffset.z = 0.0f;
                specifics->formationLinkCount = 2;
                specifics->formationLinks[0] = selection->ShipPtr[23];
                specifics->formationLinks[1] = selection->ShipPtr[12];
                specifics->formationLinks[2] = selection->ShipPtr[12];
                specifics->formationLinks[3] = selection->ShipPtr[13];
                break;
        }

        currentIndex++;
        ringIndex++;
    }
}

/*-----------------------------------------------------------------------------
    Name        : sphere_special_proximity_solution(selection);
    Description : retruns true if we can creativly change the sphere formation for proximity sensors
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/

bool32 sphere_special_proximity_solution(Ship *centreship, SelectCommand *selection)
{
   sdword i;
   for(i=1;i<selection->numShips;i++)
   {
      if (!ShipTypeIsProximitySensorType(selection->ShipPtr[i]->shiptype))
      {
         //non-proximity sensor detected, return false
         return(FALSE);
      }
   }
   //Entire selection is proximity sensors, except maybe centreship

   if(centreship == selection->ShipPtr[0])
      return(TRUE);                          //not protecting anything
   else if (ShipTypeIsProximitySensorType(selection->ShipPtr[0]->shiptype))
      return(TRUE);                          //all proximity sensors

   return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : formationSpecificsSphere
    Description : calculates and returns custom information for the specifics of a sphere formation command
    Inputs      : command
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void formationSpecificsSphere(CommandToDo *command)
{
    SelectCommand *selection = command->selection;
    sdword numShips = selection->numShips;
    sdword shipWorkingOn;
    real32 sphereRadius;
    real32 shipRadius,minrange;
    sdword i,j,k;
    sdword numDeclinations;
//    sdword totalDeclinations;
    SphereDeclination *sphereDeclination;
    vector declination;
    vector tempvec;
    matrix rotaboutz;
    Ship *centreship;
    sdword numShipsIncludingCentre;
    Ship *ship;

    SphereTableEntry *tableentry;
    sdword prox_sol;
    dbgAssert(numShips > 0);

    if (command->ordertype.attributes & COMMAND_IS_PROTECTING)
    {
        // We should form around protected ship
//        command->formation.formAroundProtectedShip = TRUE;
        dbgAssert(command->protect->numShips > 0);
        centreship = command->protect->ShipPtr[0];
        shipWorkingOn = 0;
        numShipsIncludingCentre = numShips + 1;
    }
    else if(command->ordertype.order == COMMAND_ATTACK && command->attack->numTargets)
    {
        if(command->formation.enders == TRUE)
        {
            //centreship = command->formation.endership;

            //if this case never happens, we can remove the check..but the check is
            if(command->selection->ShipPtr[0]->attackvars.attacktarget == NULL)
            {
                //don't know if this will happen ever...we'll find out
                //
                //
                //  Tell Bryce about this!
                //
                //
#ifdef DEBUG_TACTICS
    #ifdef bpasechn
                _asm int 3
    #endif

#endif
                centreship = (Ship*)command->attack->TargetPtr[0];      //this can't possibly be invalid!
            }
            else
            {
                centreship = (Ship *)command->selection->ShipPtr[0]->attackvars.attacktarget;
            }
            shipWorkingOn = 0;
            numShipsIncludingCentre = numShips + 1;
        }
        else
        {
            centreship = selection->ShipPtr[0];
            shipWorkingOn = 1;
            numShipsIncludingCentre = numShips;
        }
    }
    else
    {
        centreship = selection->ShipPtr[0];
        shipWorkingOn = 1;
        numShipsIncludingCentre = numShips;
    }

    prox_sol = sphere_special_proximity_solution(centreship, selection);

    if (numShipsIncludingCentre == 1)
    {
        return;             // if only leader is present, don't need to calculate anything
    }

    // calculate sphereRadius

    if (prox_sol && ShipTypeIsProximitySensorType(centreship->shiptype))
    {
        shipRadius = ((ProximitySensorStatics *) ((ShipStaticInfo *)(centreship->staticinfo))->custstatinfo)->SearchRadius;
    }
    else
    {
        shipRadius = centreship->staticinfo->staticheader.staticCollInfo.collspheresize;

    }
//    totalDeclinations = numDeclinations + numDeclinations + 2;

    tableentry = findSphereTableEntry(numShipsIncludingCentre);
    numDeclinations = tableentry->numDeclinations;

    if (ShipTypeIsDefenseFieldFrigateType(centreship->shiptype) &&
        ShipTypeIsTargetDroneType(selection->ShipPtr[0]->shiptype))
    {   //if centre ship is DDDF and the 1st ship is a drone, the assumption is all are drones
        sphereRadius = ((DDDFrigateStatics *) ((ShipStaticInfo *)(centreship->staticinfo))->custstatinfo)->droneDeploymentRange;
    }
    else if(prox_sol)
    {
        sphereRadius = shipRadius + ((ProximitySensorStatics *) ((ShipStaticInfo *)(selection->ShipPtr[1]->staticinfo))->custstatinfo)->SearchRadius;
    }
    else if (singlePlayerGame &&
             singlePlayerGameInfo.currentMission == 14 &&
             (selection->ShipPtr[0]->playerowner->playerIndex) &&
             ShipTypeIsHomeworldIonCannonFrigateType(selection->ShipPtr[0]->shiptype))
    {                                                                         //^^^^^^^^^^^^^  special case changed to be more general
        sphereRadius = SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE;
    }
    else if (command->formation.enders == TRUE)
    {
        minrange = REALlyBig;
        for(k=1;k<numShips;k++)
        {
            if(command->selection->ShipPtr[k]->staticinfo->bulletRange[command->selection->ShipPtr[k]->tacticstype] < minrange)
                minrange = command->selection->ShipPtr[k]->staticinfo->bulletRange[command->selection->ShipPtr[k]->tacticstype];
        }
        sphereRadius = minrange*0.9f;
    }
    else
    {
        if (shipRadius > SPHERESIZE_IGNORE_NUMDECLINATIONS)
            sphereRadius = shipRadius * SPHERE_FORMATION_PADDING_SCALE * 2;
        else
            sphereRadius = shipRadius * SPHERE_FORMATION_PADDING_SCALE * (real32)((numDeclinations < 2) ? 2 : numDeclinations);
    }

    if (sphereRadius < SPHERE_MINIMUM_RADIUS)
    {
        sphereRadius = SPHERE_MINIMUM_RADIUS;
    }
    else if ((sphereRadius > SPHERE_MAXIMUM_RADIUS) && (sphereRadius != SINGLEPLAYER_MISSION14_SPHERE_OVERRIDE))
    {
        sphereRadius = SPHERE_MAXIMUM_RADIUS;
    }

    // position ship at top pole
    ship = selection->ShipPtr[shipWorkingOn];

    vecSet(ship->formationOffset,0.0f,0.0f,sphereRadius);
    vecSet(ship->formationHeading,0.0f,0.0f,1.0f);

    shipWorkingOn++;
    if (numShips == shipWorkingOn)
    {
        return;
    }
    ship = selection->ShipPtr[shipWorkingOn];

    // position ship at bottom pole
    vecSet(ship->formationOffset,0.0f,0.0f,-sphereRadius);
    vecSet(ship->formationHeading,0.0f,0.0f,-1.0f);

    shipWorkingOn++;
    if (numShips == shipWorkingOn)
    {
        return;
    }
    ship = selection->ShipPtr[shipWorkingOn];

    // now position rest of ships.  Look up the table we need for the number of ships we have

    dbgAssert(shipWorkingOn < numShips);
    dbgAssert(numShips <= tableentry->numShipsCanHandle);

    for (i=0,sphereDeclination=&tableentry->sphereDeclinations[0];i<numDeclinations;i++,sphereDeclination++)
    {
        // get the declination vector

        vecSet(declination,sphereDeclination->cosdeclination,0.0f,sphereDeclination->sindeclination);

        matMakeRotAboutZ(&rotaboutz,sphereDeclination->cosrotang,sphereDeclination->sinrotang);

        for (j=0;j<sphereDeclination->numAnglePoints;j++)
        {
            // place the position

            ship->formationHeading = declination;
            vecScalarMultiply(ship->formationOffset,declination,sphereRadius);

            shipWorkingOn++;
            if (numShips == shipWorkingOn)
            {
                return;
            }
            ship = selection->ShipPtr[shipWorkingOn];

            tempvec = declination;
            matMultiplyMatByVec(&declination,&rotaboutz,&tempvec);      // later optimize by just having a table of unit
                                                                        // vectors describing the sphere?
        }
    }
    dbgAssert(FALSE);       // should return before getting here
}

// define for a ship moving into formation, without changing heading, and trying to steady formation
// Retail uses the frame-transient 0x00400000 ship flag here; univupdate clears it every frame.
#define FormationShipJustMoveSteady(desiredpos) \
    if (MoveReachedDestinationVariable(ship,&desiredpos,100.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        ship->flags |= SOF_TargetableHW1;                                                                                                                                                                                       \
    }                                                                                                                                                                                                                           \
    if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints|AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)             \
        {                                                                                                                                                                                                                       \
            aitrackSteadyShipDriftOnly(ship);                                                                                                           \
        }                                                                                                                                                                                                                       \
    }                                                                                                                                                   \
    else                                                                                                                                                \
    {                                                                                                                                                   \
        aitrackSteadyShipDriftOnly(ship);                                                                                                               \
    }

// define for a ship moving into formation, without changing heading
#define FormationShipJustMove(desiredpos) \
    if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity);             \
    }

// define for a ship moving into formation, without changing heading, and not trying to fly to obscured points
#define FormationShipJustMoveNotToObscuredPoints(desiredpos) \
    if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))                                                                                                                                                                \
    {                                                                                                                                                                                                                           \
        if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints|AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)             \
        {                                                                                                                                                                                                                                                            \
            aitrackSteadyShipDriftOnly(ship);                                                                                                           \
        }                                                                                                                                                                                                                                                            \
    }

// define for a ship moving into formation, including heading, and trying to steady formation
#define FormationShipMoveToSteady(desiredpos,head) \
    {                                                                                                                                                       \
        real32 tmpdistaway = MoveLeftToGoDistance(ship,&desiredpos);                                                                                        \
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;        \
                                                                                                                                                            \
        if (isCapitalShipStaticOrBig(shipstatic))                                                                                                                \
        {                                                                                                                                                   \
            if (tmpdistaway <= 500.0f)                                                                                                                      \
            {                                                                                                                                               \
                aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                           \
                tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;  \
                if (tmpdistaway <= 100.0f) ship->flags |= SOF_TargetableHW1;                                                                                \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else if (tmpdistaway <= 100.0f)                                                                                                                     \
        {                                                                                                                                                   \
            aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                               \
            tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;      \
            ship->flags |= SOF_TargetableHW1;                                                                                                               \
        }                                                                                                                                                   \
                                                                                                                                                            \
        if (tmpdistaway >= 10.0f)                                                                                                                           \
        {                                                                                                                                                   \
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)                \
            {                                                                                                                                               \
                if (tmpuseflags & AISHIP_PointInDirectionFlying)                                                                                            \
                {                                                                                                                                           \
                    aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                       \
                }                                                                                                                                           \
                aitrackSteadyShipDriftOnly(ship);                                                                                                           \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else                                                                                                                                                \
        {                                                                                                                                                   \
            aitrackSteadyShipDriftOnly(ship);                                                                                                               \
        }                                                                                                                                                   \
    }

// define for a ship moving into formation, including heading, and not trying to fly to obscured points
#define FormationShipMoveToNotToObscuredPoints(desiredpos,head) \
    {                                                                                                                                                       \
        real32 tmpdistaway = MoveLeftToGoDistance(ship,&desiredpos);                                                                                        \
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;        \
                                                                                                                                                            \
        if (isCapitalShipStaticOrBig(shipstatic))                                                                                                                \
        {                                                                                                                                                   \
            if (tmpdistaway <= 500.0f)                                                                                                                      \
            {                                                                                                                                               \
                aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                           \
                tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;  \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else if (tmpdistaway <= 100.0f)                                                                                                                     \
        {                                                                                                                                                   \
            aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                               \
            tmpuseflags = AISHIP_FastAsPossible|AISHIP_DontFlyToObscuredPoints|AISHIP_ReturnImmedIfPointObscured|AISHIP_IgnoreFormationObscuredPoints;      \
        }                                                                                                                                                   \
                                                                                                                                                            \
        if (tmpdistaway >= 10.0f)                                                                                                                           \
        {                                                                                                                                                   \
            if (aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity) & AISHIP_FLY_OBJECT_IN_WAY)                \
            {                                                                                                                                               \
                if (tmpuseflags & AISHIP_PointInDirectionFlying)                                                                                            \
                {                                                                                                                                           \
                    aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                       \
                }                                                                                                                                           \
                aitrackSteadyShipDriftOnly(ship);                                                                                                           \
            }                                                                                                                                               \
        }                                                                                                                                                   \
    }

// define for a ship moving into formation, including heading
#define FormationShipMoveTo(desiredpos,head) \
    {                                                                                                                                                       \
        real32 tmpdistaway = MoveLeftToGoDistance(ship,&desiredpos);                                                                                        \
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;                                                                         \
                                                                                                                                                            \
        if (isCapitalShipStaticOrBig(shipstatic))                                                                                                                \
        {                                                                                                                                                   \
            if (tmpdistaway <= 500.0f)                                                                                                                      \
            {                                                                                                                                               \
                aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                           \
                tmpuseflags = AISHIP_FastAsPossible;                                                                                                        \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        else if (tmpdistaway <= 100.0f)                                                                                                                     \
        {                                                                                                                                                   \
            aitrackHeadingWithBank(ship,&(head),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);                                                               \
            tmpuseflags = AISHIP_FastAsPossible;                                                                                                            \
        }                                                                                                                                                   \
                                                                                                                                                            \
        if (tmpdistaway >= 10.0f)                                                                                                                           \
        {                                                                                                                                                   \
            aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity);                                               \
        }                                                                                                                                                   \
    }

    static real32 formationGetMaxDistToDestination(Ship *ship, vector *destination)
{
        real32 dx = destination->x - ship->posinfo.position.x;
        real32 dy = destination->y - ship->posinfo.position.y;
        real32 dz = destination->z - ship->posinfo.position.z;
        real32 maxXY;

        if (dx < 0.0f)
        {
            dx = -dx;
        }
        if (dy < 0.0f)
        {
            dy = -dy;
        }
        if (dz < 0.0f)
        {
            dz = -dz;
        }

        maxXY = (dx > dy) ? dx : dy;
        return (maxXY > dz) ? maxXY : dz;
    }

    static void formationShipMoveToSteadyRetail(Ship *ship, Ship *leader, vector *desiredpos, vector *heading, bool32 usePointInDirection)
    {
        ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
        real32 maxDistToDestination = formationGetMaxDistToDestination(ship, desiredpos);
    udword useFlags = AISHIP_FastAsPossible |
                      AISHIP_DontFlyToObscuredPoints |
                      AISHIP_ReturnImmedIfPointObscured |
                      AISHIP_IgnoreFormationObscuredPoints |
                      (usePointInDirection ? AISHIP_PointInDirectionFlying : 0);

    if (isCapitalShipStatic(shipstatic) ||
        ship->shiptype == kCloakGenerator ||
        ship->shiptype == kGravWellGenerator ||
        ship->shiptype == tCloakGenerator ||
        ship->shiptype == tGravWellGenerator)
    {
        if (maxDistToDestination <= 500.0f)
        {
            aitrackHeadingWithBank(ship, heading, FLYSHIP_HEADINGACCURACY, shipstatic->sinbank);
            useFlags = AISHIP_FastAsPossible |
                       AISHIP_DontFlyToObscuredPoints |
                       AISHIP_ReturnImmedIfPointObscured |
                       AISHIP_IgnoreFormationObscuredPoints;
        }
    }
    else if (maxDistToDestination <= 100.0f)
    {
        aitrackHeadingWithBank(ship, heading, FLYSHIP_HEADINGACCURACY, shipstatic->sinbank);
        useFlags = AISHIP_FastAsPossible |
                   AISHIP_DontFlyToObscuredPoints |
                   AISHIP_ReturnImmedIfPointObscured |
                   AISHIP_IgnoreFormationObscuredPoints;
    }

    if (maxDistToDestination >= 10.0f)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship, desiredpos, useFlags, 0.0f, &leader->posinfo.velocity);
    }
}

void formationWingmanTrackLeader(struct Ship *ship,struct Ship *leader,bool32 rotate)
{
    ShipStaticInfo *shipstatic = (ShipStaticInfo *)ship->staticinfo;
    vector heading;
    vector right;
    vector rightback;
    vector desiredpos;
    real32 sepdist;

    matGetVectFromMatrixCol2(right,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);

    sepdist = leader->staticinfo->staticheader.staticCollInfo.collspheresize;
    sepdist += ship->staticinfo->staticheader.staticCollInfo.collspheresize;
    sepdist *= DELTA_FORMATION_PADDING_SCALE;

    vecSub(rightback,right,heading);
    vecMultiplyByScalar(rightback,sepdist);

    vecAdd(desiredpos,leader->posinfo.position,rightback);

//    FormationShipMoveTo(desiredpos,heading);
    if (rotate)
    {
        real32 tmpdistaway = MoveLeftToGoDistance(ship,&desiredpos);
        udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;

        if (tmpdistaway <= 100.0f)
        {
            aitrackHeadingWithBank(ship,&(heading),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);
            tmpuseflags = AISHIP_FastAsPossible;
        }

        if (tmpdistaway >= 10.0f)
        {
            aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,tmpuseflags,0.0f,&leader->posinfo.velocity);
        }
    }
    else
    {
        if (!MoveReachedDestinationVariable(ship,&desiredpos,10.0f))
        {
            aishipFlyToPointAvoidingObjsWithVel(ship,&desiredpos,AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity);
        }
    }
}

//function to determine if selection contains nothing but proximity sensors
bool32 need_Proximity_Sensor_Solution(SelectCommand *selection)
{
   sdword i;
   for(i=0;i<selection->numShips;i++)
   {
      if (!ShipTypeIsProximitySensorType(selection->ShipPtr[i]->shiptype))
      {
         //non-proximity sensor detected, return false
         return(FALSE);
      }
   }

   //Entire selection is proximity sensors
   return(TRUE);
}

//function to determine if selection contains nothing but Target Drones
bool32 need_TargetDrone_Solution(SelectCommand *selection)
{
   sdword i;
   for(i=0;i<selection->numShips;i++)
   {
      if(selection->ShipPtr[i]->shiptype != TargetDrone)
      {
         return(FALSE);
      }
   }
   return(TRUE);
}


#define oneOverRootTwo 0.7071068f

void setFormationToDo(struct CommandToDo *formationtodo)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader = selection->ShipPtr[0];
    matrix *coordsys;
    vector position;
    vector desiredposition;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    vector heading;

    formationtodo->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE;

    dbgAssert(numShips > 0);
    if (numShips == 1)
    {
        return;
    }

    position = leader->collInfo.collPosition;

    /*  Setup formations coordinate system */
    if(formationtodo->formation.formationLocked)
    {
        coordsys = &formationtodo->formation.coordsys;
        matGetVectFromMatrixCol3(heading,formationtodo->formation.coordsys);
    }
    else
    {
        coordsys = &leader->rotinfo.coordsys;
        matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case CUSTOM_FORMATION:
        case FORCEFIELD_WALL_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                matMultiplyMatByVec(&desiredposition,coordsys,&ship->formationOffset);
                vecAddTo(desiredposition,position);

                ship->posinfo.position = desiredposition;
                ship->rotinfo.coordsys = *coordsys;
                univUpdateObjRotInfo((SpaceObjRot *)ship);
            }
            break;

        case SPHERE_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                vecAdd(ship->posinfo.position,position,ship->formationOffset);
                ship->rotinfo.coordsys = *coordsys;
                univUpdateObjRotInfo((SpaceObjRot *)ship);
            }
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

void processFormationToDo(struct CommandToDo *formationtodo,bool32 steadyFormation,bool32 passiveAttacked)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader = selection->ShipPtr[0];
    matrix *coordsys;
    vector position;
    vector desiredposition;
    bool32 dontrotate;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    vector heading;
    real32 error = 0;
    vector errorvect;
    bool32 calcError = ((universe.univUpdateCounter & FORMATION_ERROR_CALCULATE_RATE) == FORMATION_ERROR_CALCULATE_FRAME);
    // Update the leader's live command because delegateCommand can pass a stack copy here.
    CommandToDo *formationtomodify = leader->command;

    dbgAssert(numShips > 0);
    if (numShips == 1)
    {
        formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE;
        setFormationTravelVelocity(formationtomodify);
        return;
    }

    position = leader->collInfo.collPosition;

    /*  Setup formations coordinate system */
    if(formationtodo->formation.formationLocked)
    {
        coordsys = &formationtodo->formation.coordsys;
        matGetVectFromMatrixCol3(heading,formationtodo->formation.coordsys);
    }
    else
    {
        coordsys = &leader->rotinfo.coordsys;
        matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case CUSTOM_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;
                dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
                ship->aistateattack = 0;

                matMultiplyMatByVec(&desiredposition,coordsys,&ship->formationOffset);
                vecAddTo(desiredposition,position);

                if (calcError)
                {
                    vecSub(errorvect,desiredposition,ship->posinfo.position);
                    error += vecMagnitudeSquared(errorvect);
                }
                if(!bitTest(ship->specialFlags1C8, CATA_RUNTIME_BROKEN_FORMATION))
                if (dontrotate)
                {
                    if (steadyFormation)
                    {
                        FormationShipJustMoveSteady(desiredposition)
                    }
                    else
                    {
                        FormationShipJustMove(desiredposition)
                    }
                }
                else
                {
                    if (steadyFormation)
                    {
                        FormationShipMoveToSteady(desiredposition,heading)
                    }
                    else
                    {
                        FormationShipMoveTo(desiredposition,heading)
                    }
                }
            }
            break;

        case SPHERE_FORMATION:
        case FORCEFIELD_SPHERE_FORMATION:
        {
            bool32 protectsurround = FALSE;
            bool32 useTargetSpace = FALSE;
            Ship *targetship = NULL;

            if (ShipTypeIsTargetDroneType(leader->shiptype))
            {
                aishipTempDisableAvoiding = TRUE;
            }

            if (formationtodo->ordertype.attributes & COMMAND_IS_PROTECTING)
            {
                dbgAssert(formationtodo->protect->numShips > 0);
                targetship = formationtodo->protect->ShipPtr[0];
                leader = targetship;
                position = leader->collInfo.collPosition;       // retail uses collision-position anchor
                matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);
                protectsurround = TRUE;
                useTargetSpace = (formationtodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION);
                i = 0;
            }
            else if (formationtodo->ordertype.order == COMMAND_ATTACK)
            {
                if(formationtodo->formation.enders == TRUE)
                {
                    Ship *protectship;
                    //protectship = formationtodo->formation.endership;
                    if(formationtodo->selection->ShipPtr[0]->attackvars.attacktarget == NULL)
                    {
                        //don't know if this will happen ever...we'll find out
                        //
                        //
                        //  Tell Bryce about this!
                        //
                        //
#ifdef DEBUG_TACTICS
                        _asm int 3
#endif

                        protectship = (Ship*)formationtodo->attack->TargetPtr[0];       //this can't possibly be invalid!
                    }
                    else
                    {
                        protectship = (Ship *)formationtodo->selection->ShipPtr[0]->attackvars.attacktarget;
                    }
                    //protectship = formationtodo->selection->ShipPtr[0]->attackvars.attacktarget;
                    targetship = protectship;
                    leader = protectship;
                    position = leader->collInfo.collPosition;       // retail uses collision-position anchor
                    protectsurround = TRUE;
                    useTargetSpace = (formationtodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION);
                    i = 0;
                }
                else
                    i=1;
            }
            else
            {
                i = 1;
            }

            for (;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;
                dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
                if ((i & 7) != 0)
                {
                    ship->aistateattack = 0;
                }

                if (useTargetSpace)
                {
                    matMultiplyMatByVec(&desiredposition, &leader->rotinfo.coordsys, &ship->formationOffset);
                    vecAddTo(desiredposition, position);
                }
                else
                {
                    vecAdd(desiredposition,position,ship->formationOffset);
                }

                if (calcError)
                {
                    vecSub(errorvect,desiredposition,ship->posinfo.position);
                    error += vecMagnitudeSquared(errorvect);
                }

                if(!bitTest(ship->specialFlags1C8, CATA_RUNTIME_BROKEN_FORMATION))
                if (dontrotate)
                {
                    if (steadyFormation)
                    {
                        FormationShipJustMoveSteady(desiredposition)
                    }
                    else
                    {
                        if (protectsurround)
                        {
                            FormationShipJustMoveNotToObscuredPoints(desiredposition)
                        }
                        else
                        {
                            FormationShipJustMove(desiredposition)
                        }
                    }
                }
                else
                {
                    if (steadyFormation)
                    {
                        if (useTargetSpace)
                        {
                            FormationShipMoveTo(desiredposition,ship->formationHeading)
                        }
                        else
                        {
                            FormationShipMoveToSteady(desiredposition,ship->formationHeading)
                        }
                    }
                    else
                    {
                        if (protectsurround)
                        {
                            if (useTargetSpace)
                            {
                                vector targetHeading;

                                matMultiplyMatByVec(&targetHeading, &leader->rotinfo.coordsys, &ship->formationHeading);
                                formationShipMoveToSteadyRetail(ship, leader, &desiredposition, &targetHeading, TRUE);
                            }
                            else if (leader->posinfo.isMoving & ISMOVING_MOVING)
                            {
                                FormationShipMoveToNotToObscuredPoints(desiredposition,heading);
                            }
                            else
                            {
                                FormationShipMoveToNotToObscuredPoints(desiredposition,ship->formationHeading);
                            }
                        }
                        else
                        {
                            FormationShipMoveTo(desiredposition,heading)
                        }
                    }
                }
            }

            aishipTempDisableAvoiding = FALSE;
        }
        break;

        default:
            break;
    }

    if (calcError)
    {
        error /= numShips;
#if 0
        dbgMessagef("\nFormation Error %f  ",error);
#endif
        if (error > FORMATION_ERROR_BIG)
        {
            formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MINSCALE;
        }
        else
        {
            if (error < FORMATION_ERROR_NEGLIGIBLE)
            {
                formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE;
            }
            else
            {
                formationtomodify->formation.percentmaxspeed = FORMATION_TRAVELVEL_MAXSCALE - (error / FORMATION_ERROR_BIG) * (FORMATION_TRAVELVEL_MAXSCALE - FORMATION_TRAVELVEL_MINSCALE);
            }
        }
#if 0
        dbgMessagef("Perc Max speed %f",formationtomodify->formation.percentmaxspeed);
#endif

        setFormationTravelVelocity(formationtomodify);
    }
}

void formationTypeHasChanged(struct CommandToDo *command)
{
    TypeOfFormation formationtype = command->formation.formationtype;

    if (formationSortType[formationtype] != command->formation.sortorder)
    {
        formationArrangeOptimum(command);
    }

    FormationCalculateOffsets(command);
}

void formationContentHasChanged(struct CommandToDo *command)
{
    FormationCalculateOffsets(command);
}

/*-----------------------------------------------------------------------------
    Name        : clSelectionAlreadyInFormation
    Description : Call this function to see if this selection is already in formation
    Inputs      :
    Outputs     :
    Return      : returns NO_FORMATION if this selection is not already in formation, otherwise
                  returns the type of formation the selection is in.
----------------------------------------------------------------------------*/
TypeOfFormation clSelectionAlreadyInFormation(CommandLayer *comlayer,SelectCommand *selectcom)
{
    CommandToDo *alreadyformation;

    if ((alreadyformation = IsSelectionAlreadyDoingSomething(comlayer,selectcom)) != NULL)
    {
        if (alreadyformation->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            return alreadyformation->formation.formationtype;
        }
    }
    return NO_FORMATION;
}

/*-----------------------------------------------------------------------------
    Name        : FillInShipFormationStuff
    Description : Fills in formation stuff in ship
    Inputs      : ship, formationcommand
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FillInShipFormationStuff(Ship *ship,struct CommandToDo *formationcommand)
{
    ship->formationcommand = formationcommand;
}

/*-----------------------------------------------------------------------------
    Name        : GetShipsTravelVel
    Description : returns the selection's velocity at which it can all travel, scaled by scalevel
    Inputs      : selection, scalevel
    Outputs     :
    Return      : returns the selection's velocity at which it can all travel, scaled by scalevel
----------------------------------------------------------------------------*/
real32 GetShipsTravelVel(SelectCommand *selection,real32 scalevel)
{
    real32 minvel = REALlyBig;
    real32 testvel;
    real32 speedThrottle;
    sdword i;
    Ship *ship;
    bool32 useProtectSpeed;
    CommandToDo *command;
    SelectCommand *protect;

    for (i=0;i<selection->numShips;i++)
    {
        ship = selection->ShipPtr[i];
        useProtectSpeed = FALSE;
        if ((ship->shiptype == sSentinel || ship->shiptype == bSentinel) &&
            univObjIsDying((SpaceObj *)ship))
        {
            command = ship->command;
            useProtectSpeed = TRUE;
            if (command != NULL)
            {
                protect = command->protect;
                if (protect != NULL && protect->numShips > 0)
                {
                    testvel = (real32)(shipGetMaxSpeed(protect->ShipPtr[0]) * tacticsGetShipsMaxVelocity(protect->ShipPtr[0]));
                }
            }
        }
        else
        {
            testvel = (real32)(shipGetMaxSpeed(ship) * tacticsGetShipsMaxVelocity(ship));
        }

        if (testvel < minvel)
        {
            minvel = testvel;
        }

        if (!useProtectSpeed &&
            (speedThrottle = ship->speedThrottle) > 0.0f &&
            speedThrottle < minvel)
        {
            minvel = speedThrottle;
        }
    }

    return minvel * scalevel;
}

void  setFormationTravelVelocity(CommandToDo *formationcommand)
{
    //won't hurt if we were to set the travel velocity and ships aren't
    //in formation, but lets not let it happen anyways.
    dbgAssert((formationcommand->ordertype.attributes & COMMAND_IS_FORMATION) ||
              formationcommand->ordertype.order == COMMAND_ATTACK);

    formationcommand->formation.travelvel = GetShipsTravelVel(formationcommand->selection,formationcommand->formation.percentmaxspeed);
}


/*-----------------------------------------------------------------------------
    Name        : formationRemoveShipFromSelection
    Description : removes ship from selection of formation
    Inputs      :
    Outputs     :
    Return      : -1 if not found, shipindex if found
----------------------------------------------------------------------------*/
sdword formationRemoveShipFromSelection(struct CommandToDo *formationtodo,Ship *removeship)
{
    SelectCommand *selection = formationtodo->selection;
    sdword i,j;
    sdword returnval;

    dbgAssert(formationtodo->ordertype.attributes & COMMAND_IS_FORMATION);

    for (i=0;i<selection->numShips; )
    {
        if (removeship == selection->ShipPtr[i])
        {
            goto foundshiptoremove;
        }
        i++;
    }

    return -1;

foundshiptoremove:
    returnval = i;

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
            if (selection->numShips <= 2)
            {
                goto defaultremove;
            }

            // move one wing forward
            for (j=i;j<selection->numShips;j+=2)
            {
                if ((j+2) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+2];
                }
                else if ((j+1) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+1];
                }
            }
            selection->numShips--;
            break;

        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
            // move one wing forward
            for (j=i;j<selection->numShips;j+=4)
            {
                if ((j+4) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+4];
                }
                else if ((j+3) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+3];
                }
                else if ((j+2) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+2];
                }
                else if ((j+1) < selection->numShips)
                {
                    selection->ShipPtr[j] = selection->ShipPtr[j+1];
                }
            }
            selection->numShips--;
            break;

defaultremove:
        default:
            selection->numShips--;
            selection->ShipPtr[i] = selection->ShipPtr[selection->numShips];
            break;
    }

    return returnval;
}

void FormationCalculateOffsets(struct CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader;
    ShipStaticInfo *leaderstatic;
    real32 tacticsFormationPadding;
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    sdword prox_sol;
    real32 searchRadius;

    dbgAssert(numShips > 0);

    if (formationtodo->formation.formationtype == FORCEFIELD_SPHERE_FORMATION)
    {
        formationComputeSentinelOffsetsA(formationtodo, TRUE);
        for (i = 0; i < numShips; i++)
        {
            clSetAttribute(selection->ShipPtr[i], 1, 5);
        }
        return;
    }
    else
    {
        for (i = 0; i < numShips; i++)
        {
            if (selection->ShipPtr[i]->cataBusyStateWindow.sentinelFormationState == 1)
            {
                clSetAttribute(selection->ShipPtr[i], 0, 5);
            }
        }
    }

    if (formationtodo->formation.formationtype == WALL_FORMATION &&
        formationAllShipsAreSentinels(selection))
    {
        formationComputeSentinelOffsetsB(formationtodo);
        return;
    }

    if (formationtodo->formation.formationtype == SPHERE_FORMATION)
    {
        formationComputeSentinelOffsetsA(formationtodo, FALSE);
        return;
    }

    if (formationtodo->formation.formationtype == FORCEFIELD_WALL_FORMATION)
    {
        formationComputeSentinelOffsetsB(formationtodo);
        return;
    }

    // Non-parity assert: tacticalState is only ever written as TightFormation(0) or
    // LooseFormation(1) (formation.c defaults / tacticsGetFormationOptimalState), and
    // indexes the 2-entry tacticsInfo.formationPadding[] array (retail FormationCalculateOffsets
    // 0x48372c/0x483735 does the identical unguarded [ecx+25Ch] read then array index). A value
    // outside this range means formationtodo has been corrupted by something else before this
    // point; catch it here with full context instead of silently reading adjacent static data
    // out of bounds (observed as an ASAN AV several frames removed from the real corruption).
    if (formationtodo->formation.tacticalState < 0 || formationtodo->formation.tacticalState >= NUM_FORMATION_PADDINGS)
    {
        dbgFatalf(DBG_Loc, "\nFormationCalculateOffsets saw corrupt tacticalState: todo=%p tacticalState=%d formationtype=%d formationLocked=%d numShips=%d order=%d attributes=0x%x leader=%p",
                  formationtodo, formationtodo->formation.tacticalState, formationtodo->formation.formationtype,
                  formationtodo->formation.formationLocked, numShips, formationtodo->ordertype.order,
                  formationtodo->ordertype.attributes, selection->ShipPtr[0]);
    }

    leader = selection->ShipPtr[0];
    leaderstatic = leader->staticinfo;
    tacticsFormationPadding = tacticsInfo.formationPadding[formationtodo->formation.tacticalState];

    prox_sol = need_Proximity_Sensor_Solution(selection);
    if (prox_sol)
    {
        searchRadius = ((ProximitySensorStatics *)leaderstatic->custstatinfo)->SearchRadius;
    }
    else
    {
        if ((formationtodo->formation.formationtype == WALL_FORMATION) && (need_TargetDrone_Solution(selection)))
        {
            prox_sol = TRUE;
            searchRadius = TARGETDRONE_FORMATION_SIZE;
        }
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // rightback is (0,1,-1)
            // leftback is (0,-1,-1)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    desiredposright.z -= rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;

                    desiredposleft.z -= leftsepdist;

                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case BROAD_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // right is (0,1,0)
            // left is (0,-1,0)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;


                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case DELTA3D_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // uprightback is (1,1,-1)
                        desiredposupright.x += uprightsepdist;

                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
//                        desiredposupright.z -= uprightsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // upleftback is (1,-1,-1)
                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
//                        desiredposupleft.z -= upleftsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnleftback is (-1,-1,-1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
  //                      desiredposdnleft.z -= dnleftsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnrightback is (-1,1,-1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
//                        desiredposdnright.z -= dnrightsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case CLAW_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            real32 forwardmodifier = 1.0f;
            real32 forsepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = uprightsepdist * forwardmodifier;

                        // uprightforward is (1,1,1)
                        desiredposupright.x += uprightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
                        desiredposupright.z += forsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = upleftsepdist * forwardmodifier;

                        // upleftforward is (1,-1,1)

                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
                        desiredposupleft.z += forsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnleftsepdist * forwardmodifier;

                        // dnleftforward is (-1,-1,1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
                        desiredposdnleft.z += forsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;

                        forwardmodifier *= CLAW_FORMATION_CURVATURE;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnrightsepdist * forwardmodifier;

                        // dnrightforward is (-1,1,1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
                        desiredposdnright.z += forsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case WALL_FORMATION:
        {
            sdword squaresize;
            sdword curpos,currow;
            real32 sepdist;
            vector desiredpos = { 0.0f, 0.0f, 0.0f };
            vector desiredv = { 0.0f, 0.0f, 0.0f };
            Ship *ship;
            ShipStaticInfo *shipstatic;

            squaresize = (udword)fsqrt((real32)numShips);
            if ((squaresize*squaresize) < numShips)
            {
                squaresize++;
            }

            if (prox_sol)
            {
                sepdist = searchRadius*2.0f;
            }
            else
            {
                sepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*tacticsFormationPadding*
                            leaderstatic->formationPaddingModifier*WALL_FORMATION_PADDING_SCALE + WALL_FORMATION_PADDING;
            }

            // rightspacing is (0,1,0)
            // downspacing is (-1,0,0)
            if(formationtodo->formation.flipState)
                desiredpos.y -= sepdist;
            else
                desiredpos.y += sepdist;

            for (curpos=1,currow=0,i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                ship->formationOffset = desiredpos;

                curpos++;
                if (curpos >= squaresize)
                {
                    curpos = 0;
                    currow++;
                    desiredv.x -= sepdist;
                    desiredpos = desiredv;
                }
                else
                {
                    if(formationtodo->formation.flipState)
                        desiredpos.y -= sepdist;
                    else
                        desiredpos.y += sepdist;
                }
            }
        }
        break;

        case CUSTOM_FORMATION:
            {
                vector desiredpos = { 0.0f, 0.0f, 0.0f };
                vector averagepos = { 0.0f, 0.0f, 0.0f };
                vector error = { 0.0f, 0.0f, 0.0f };
                vector desiredv = { 0.0f, 0.0f, 0.0f };
                Ship *ship;
                real32 minError,totalError;
                sdword minErrorIndex;


                //get center point
                for(i=0;i<numShips;i++)
                {
                    vecAddTo(averagepos,selection->ShipPtr[i]->posinfo.position);
                }
                vecScalarMultiply(averagepos,averagepos,1.0f/numShips);

                //find ship closest to centerpoint to use for leader
                minError = REALlyBig;
                minErrorIndex = 0;
                for(i=0;i<numShips;i++)
                {
                    vecSub(error,selection->ShipPtr[i]->posinfo.position,averagepos);
                    error.x = abs(error.x);
                    error.y = abs(error.y);
                    error.z = abs(error.z);
                    totalError = error.x+error.y+error.z;
                    if(totalError < minError)
                    {
                        //new closest ship!
                        minError = totalError;
                        minErrorIndex = i;
                    }
                }

                //swap center ship with leader position
                ship = selection->ShipPtr[0];
                selection->ShipPtr[0] = selection->ShipPtr[minErrorIndex];
                selection->ShipPtr[minErrorIndex] = ship;

                leader = selection->ShipPtr[0];

                //convert all ships current positions to relatavistic offsets
                for(i=1;i<numShips;i++)
                {
                    ship = (Ship *) selection->ShipPtr[i];

                    vecSub(desiredpos,ship->posinfo.position,leader->posinfo.position);
                    matMultiplyVecByMat(&ship->formationOffset,&desiredpos,&leader->rotinfo.coordsys);
                }
            }
        break;
        default:
            dbgAssert(FALSE);
            break;
    }
}

void FormationCalculateSpecialOffsets(struct CommandToDo *formationtodo)
{
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader;
    ShipStaticInfo *leaderstatic;
    real32 tacticsFormationPadding;
    sdword i;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    sdword prox_sol;
    real32 searchRadius;

    dbgAssert(numShips > 0);

    if (formationtodo->formation.formationtype == SPHERE_FORMATION)
    {
        formationSpecificsSphere(formationtodo);
        return;
    }

    // Non-parity assert: see matching check in FormationCalculateOffsets. tacticalState
    // should only ever be TightFormation(0) or LooseFormation(1); anything else means
    // formationtodo was corrupted before reaching here.
    if (formationtodo->formation.tacticalState < 0 || formationtodo->formation.tacticalState >= NUM_FORMATION_PADDINGS)
    {
        dbgFatalf(DBG_Loc, "\nFormationCalculateSpecialOffsets saw corrupt tacticalState: todo=%p tacticalState=%d formationtype=%d formationLocked=%d numShips=%d order=%d attributes=0x%x leader=%p",
                  formationtodo, formationtodo->formation.tacticalState, formationtodo->formation.formationtype,
                  formationtodo->formation.formationLocked, numShips, formationtodo->ordertype.order,
                  formationtodo->ordertype.attributes, selection->ShipPtr[0]);
    }

    leader = selection->ShipPtr[0];
    leaderstatic = leader->staticinfo;
    tacticsFormationPadding = tacticsInfo.formationPadding[formationtodo->formation.tacticalState];

    prox_sol = need_Proximity_Sensor_Solution(selection);
    if (prox_sol)
    {
        searchRadius = ((ProximitySensorStatics *)leaderstatic->custstatinfo)->SearchRadius;
    }
    else
    {
        if ((formationtodo->formation.formationtype == WALL_FORMATION) && (need_TargetDrone_Solution(selection)))
        {
            prox_sol = TRUE;
            searchRadius = TARGETDRONE_FORMATION_SIZE;
        }
    }

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // rightback is (0,1,-1)
            // leftback is (0,-1,-1)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    desiredposright.z -= rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= DELTA_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;

                    desiredposleft.z -= leftsepdist;

                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case BROAD_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            // right is (0,1,0)
            // left is (0,-1,0)
            real32 rightsepdist;
            real32 leftsepdist;
            real32 thissepdist;
            vector desiredposright = { 0.0f, 0.0f, 0.0f };
            vector desiredposleft = { 0.0f, 0.0f, 0.0f };

            if (prox_sol)
            {
                leftsepdist = searchRadius;
            }
            else
            {
                leftsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }
            rightsepdist = leftsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                if (i & 1)
                {
                    rightsepdist += thissepdist;
                    if (!prox_sol) rightsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposright.y -= rightsepdist;
                    else
                        desiredposright.y += rightsepdist;

                    ship->formationOffset = desiredposright;

                    rightsepdist = thissepdist;
                }
                else
                {
                    leftsepdist += thissepdist;
                    if (!prox_sol) leftsepdist *= BROAD_FORMATION_PADDING_SCALE;

                    if(formationtodo->formation.flipState)
                        desiredposleft.y += leftsepdist;
                    else
                        desiredposleft.y -= leftsepdist;


                    ship->formationOffset = desiredposleft;

                    leftsepdist = thissepdist;
                }
            }
        }
        break;

        case DELTA3D_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // uprightback is (1,1,-1)
                        desiredposupright.x += uprightsepdist;

                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
//                        desiredposupright.z -= uprightsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // upleftback is (1,-1,-1)
                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
//                        desiredposupleft.z -= upleftsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnleftback is (-1,-1,-1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
  //                      desiredposdnleft.z -= dnleftsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= DELTA3D_FORMATION_PADDING_SCALE;

                        // dnrightback is (-1,1,-1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
//                        desiredposdnright.z -= dnrightsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case CLAW_FORMATION:
        {
            // offset in format up,right,heading for x,y,z
            vector desiredposupright = { 0.0f, 0.0f, 0.0f };
            vector desiredposupleft = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnright = { 0.0f, 0.0f, 0.0f };
            vector desiredposdnleft = { 0.0f, 0.0f, 0.0f };
            real32 uprightsepdist;
            real32 upleftsepdist;
            real32 dnrightsepdist;
            real32 dnleftsepdist;
            real32 thissepdist;

            real32 forwardmodifier = 1.0f;
            real32 forsepdist;

            if (prox_sol)
            {
                uprightsepdist = searchRadius;
            }
            else
            {
                uprightsepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*leaderstatic->formationPaddingModifier*tacticsFormationPadding;
            }

            upleftsepdist = uprightsepdist;
            dnrightsepdist = uprightsepdist;
            dnleftsepdist = uprightsepdist;

            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                if (prox_sol)
                {
                    thissepdist = searchRadius;
                }
                else
                {
                    thissepdist = shipstatic->staticheader.staticCollInfo.collspheresize*shipstatic->formationPaddingModifier*tacticsFormationPadding;
                }

                switch (i & 3)
                {
                    case 1:
                        uprightsepdist += thissepdist;
                        if (!prox_sol) uprightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = uprightsepdist * forwardmodifier;

                        // uprightforward is (1,1,1)
                        desiredposupright.x += uprightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupright.y -= uprightsepdist;
                        else
                            desiredposupright.y += uprightsepdist;
                        desiredposupright.z += forsepdist;

                        ship->formationOffset = desiredposupright;

                        uprightsepdist = thissepdist;
                        break;

                    case 2:
                        upleftsepdist += thissepdist;
                        if (!prox_sol) upleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = upleftsepdist * forwardmodifier;

                        // upleftforward is (1,-1,1)

                        desiredposupleft.x += upleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposupleft.y += upleftsepdist;
                        else
                            desiredposupleft.y -= upleftsepdist;
                        desiredposupleft.z += forsepdist;

                        ship->formationOffset = desiredposupleft;

                        upleftsepdist = thissepdist;
                        break;

                    case 0:
                        dnleftsepdist += thissepdist;
                        if (!prox_sol) dnleftsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnleftsepdist * forwardmodifier;

                        // dnleftforward is (-1,-1,1)
                        desiredposdnleft.x -= dnleftsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnleft.y += dnleftsepdist;
                        else
                            desiredposdnleft.y -= dnleftsepdist;
                        desiredposdnleft.z += forsepdist;

                        ship->formationOffset = desiredposdnleft;

                        dnleftsepdist = thissepdist;

                        forwardmodifier *= CLAW_FORMATION_CURVATURE;
                        break;

                    case 3:
                        dnrightsepdist += thissepdist;
                        if (!prox_sol) dnrightsepdist *= CLAW_FORMATION_PADDING_SCALE;
                        forsepdist = dnrightsepdist * forwardmodifier;

                        // dnrightforward is (-1,1,1)
                        desiredposdnright.x -= dnrightsepdist;
                        if(formationtodo->formation.flipState)
                            desiredposdnright.y -= dnrightsepdist;
                        else
                            desiredposdnright.y += dnrightsepdist;
                        desiredposdnright.z += forsepdist;

                        ship->formationOffset = desiredposdnright;

                        dnrightsepdist = thissepdist;
                        break;
                }
            }
        }
        break;

        case WALL_FORMATION:
        {
            sdword squaresize;
            sdword curpos,currow;
            real32 sepdist;
            vector desiredpos = { 0.0f, 0.0f, 0.0f };
            vector desiredv = { 0.0f, 0.0f, 0.0f };
            Ship *ship;
            ShipStaticInfo *shipstatic;

            squaresize = (udword)fsqrt((real32)numShips);
            if ((squaresize*squaresize) < numShips)
            {
                squaresize++;
            }

            if (prox_sol)
            {
                sepdist = searchRadius*2.0f;
            }
            else
            {
                sepdist = leaderstatic->staticheader.staticCollInfo.collspheresize*tacticsFormationPadding*
                            leaderstatic->formationPaddingModifier*WALL_FORMATION_PADDING_SCALE + WALL_FORMATION_PADDING;
            }

            // rightspacing is (0,1,0)
            // downspacing is (-1,0,0)
            if(formationtodo->formation.flipState)
                desiredpos.y -= sepdist;
            else
                desiredpos.y += sepdist;

            for (curpos=1,currow=0,i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];
                shipstatic = (ShipStaticInfo *)ship->staticinfo;

                ship->formationOffset = desiredpos;

                curpos++;
                if (curpos >= squaresize)
                {
                    curpos = 0;
                    currow++;
                    desiredv.x -= sepdist;
                    desiredpos = desiredv;
                }
                else
                {
                    if(formationtodo->formation.flipState)
                        desiredpos.y -= sepdist;
                    else
                        desiredpos.y += sepdist;
                }
            }
        }
        break;

        case CUSTOM_FORMATION:
            {
                vector desiredpos = { 0.0f, 0.0f, 0.0f };
                vector averagepos = { 0.0f, 0.0f, 0.0f };
                vector error = { 0.0f, 0.0f, 0.0f };
                vector desiredv = { 0.0f, 0.0f, 0.0f };
                Ship *ship;
                real32 minError,totalError;
                sdword minErrorIndex;


                //get center point
                for(i=0;i<numShips;i++)
                {
                    vecAddTo(averagepos,selection->ShipPtr[i]->posinfo.position);
                }
                vecScalarMultiply(averagepos,averagepos,1.0f/numShips);

                //find ship closest to centerpoint to use for leader
                minError = REALlyBig;
                minErrorIndex = 0;
                for(i=0;i<numShips;i++)
                {
                    vecSub(error,selection->ShipPtr[i]->posinfo.position,averagepos);
                    error.x = abs(error.x);
                    error.y = abs(error.y);
                    error.z = abs(error.z);
                    totalError = error.x+error.y+error.z;
                    if(totalError < minError)
                    {
                        //new closest ship!
                        minError = totalError;
                        minErrorIndex = i;
                    }
                }

                //swap center ship with leader position
                ship = selection->ShipPtr[0];
                selection->ShipPtr[0] = selection->ShipPtr[minErrorIndex];
                selection->ShipPtr[minErrorIndex] = ship;

                leader = selection->ShipPtr[0];

                //convert all ships current positions to relatavistic offsets
                for(i=1;i<numShips;i++)
                {
                    ship = (Ship *) selection->ShipPtr[i];

                    vecSub(desiredpos,ship->posinfo.position,leader->posinfo.position);
                    matMultiplyVecByMat(&ship->formationOffset,&desiredpos,&leader->rotinfo.coordsys);
                }
            }
        break;
        default:
            dbgAssert(FALSE);
            break;
    }
}

real32 calculateTravelDistance(CommandToDo *formationtodo)
{
    sdword i;
    SelectCommand *selection = formationtodo->selection;
    sdword numShips = selection->numShips;
    Ship *leader = selection->ShipPtr[0];
    matrix *coordsys;
    vector position;
    vector desiredposition;
    Ship *ship;
    real32  distanceSqr=0.0f;
    vector distvec;

    position = leader->posinfo.position;
    coordsys = &leader->rotinfo.coordsys;

    switch (formationtodo->formation.formationtype)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case CUSTOM_FORMATION:
            for (i=1;i<numShips;i++)
            {
                ship = selection->ShipPtr[i];

                //calculate realspace position
                matMultiplyMatByVec(&desiredposition,coordsys,&ship->formationOffset);
                vecAddTo(desiredposition,position);

                //calculate distance ship must go
                vecSub(distvec,ship->posinfo.position,desiredposition);
                distanceSqr += vecMagnitudeSquared(distvec);
            }
        case SPHERE_FORMATION:
            break;
    }
    return distanceSqr;
}

//if smallest ship radius/biggest ship radius is bigger than this, then
//we ignore leader size in choosing a leader and find the center leader!
#define BIG_SMALL_RATIO_TO_BE_EQUAL_FOR_LEADER      0.7f

void formationArrageCrazyOptimum(CommandToDo *formationcommand)
{
    //algorithm...go through each ship...selecting each to be the leader
    //then go through each iteration of that to calculate the minimum distance
    //then select the minimum distance for actual use!
    sdword i,j,a,p;
    sdword numShips = formationcommand->selection->numShips;
    SelectCommand *selection = formationcommand->selection;
    SelectCommand *optimumSel;
    SelectCommand *originalSel,*testSel;
    real32 tempdist,biggestToSmallestRatio;
    sdword numWhoWant,poswanted;
    real32 minDist = REALlyBig;
    real32 mindistleader,tempdist3;

    vector positionsMax[COMMAND_MAX_SHIPS];
    sdword shipWantsPositionMax[COMMAND_MAX_SHIPS];
    sdword shipGetsPositionMax[COMMAND_MAX_SHIPS];
    sdword positionTakenMax[COMMAND_MAX_SHIPS];

    vector *positions;
    sdword *shipWantsPosition;
    sdword *shipGetsPosition;
    sdword *positionTaken;
    bool32 redo = FALSE;

    matrix *coordsys;
    vector center = {0.0f,0.0f,0.0f};
    sdword shipwhogetsit;
    vector tempvec;
    Ship *ship;
    //could possible select 'middle' ship as leader...would be faster...
    //will test effect on efficiency

    //allocation enough memory for
    optimumSel = memAlloc(sizeofSelectCommand(selection->numShips),"FormationOpt2",0);
    originalSel = memAlloc(sizeofSelectCommand(selection->numShips),"FormationOpt1",0);
    testSel = memAlloc(sizeofSelectCommand(selection->numShips),"FormationOpt1",0);


    if (numShips > COMMAND_MAX_SHIPS-1)
    {
        positions=memAlloc(sizeof(vector)*numShips,"forma1",0);
        shipWantsPosition=memAlloc(sizeof(sdword)*numShips,"forma2",0);
        shipGetsPosition=memAlloc(sizeof(sdword)*numShips,"forma3",0);
        positionTaken=memAlloc(sizeof(sdword)*numShips,"forma4",0);
    }
    else
    {
        positions=positionsMax;
        shipWantsPosition=shipWantsPositionMax;
        shipGetsPosition=shipGetsPositionMax;
        positionTaken=positionTakenMax;
    }

    optimumSel->numShips = numShips;

    formationArrangeOptimum(formationcommand);
    biggestToSmallestRatio = selection->ShipPtr[numShips-1]->staticinfo->staticheader.staticCollInfo.collspheresize/selection->ShipPtr[0]->staticinfo->staticheader.staticCollInfo.collspheresize;
    if(biggestToSmallestRatio > BIG_SMALL_RATIO_TO_BE_EQUAL_FOR_LEADER)
    {
        redo=TRUE;
/*
        for(i=0;i<numShips;i++)
        {
            vecAddTo(center,selection->ShipPtr[i]->posinfo.position);
        }
        tempdist = 1.0f/numShips;
        vecScalarMultiply(center,center,tempdist);
        minDist = REALlyBig;
        for(i=0;i<numShips;i++)
        {
            vecSub(tempvec,selection->ShipPtr[i]->posinfo.position,center);
            tempdist = vecMagnitudeSquared(tempvec);
            if(tempdist < minDist)
            {
                minDist = tempdist;
                shipwhogetsit = i;
            }
        }

        ship = selection->ShipPtr[0];
        selection->ShipPtr[0] = selection->ShipPtr[shipwhogetsit];
        selection->ShipPtr[shipwhogetsit] = ship;
*/
    }

    //find center ship...i.e. find leader!

    //assume ships are in the correct order of sizeness
    //and the leader has been picked!  I.E. position 0 is secure!

    //go through all leaders and use optimal version IF redo is true

    originalSel->numShips = numShips;
    for(a=0;a<numShips;a++)
    {
        originalSel->ShipPtr[a] = selection->ShipPtr[a];
    }

    mindistleader = REALlyBig;

    for(a=0;a<numShips;a++)
    {
        //reassemble original list
        for(p=0;p<numShips;p++)
        {
            selection->ShipPtr[p] = originalSel->ShipPtr[p];
        }
        //use a as leader
        selection->ShipPtr[0] = selection->ShipPtr[a];
        selection->ShipPtr[a] = originalSel->ShipPtr[0];

        FormationCalculateOffsets(formationcommand);
        coordsys = &selection->ShipPtr[0]->rotinfo.coordsys;
        for(i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            matMultiplyMatByVec(&positions[i],coordsys,&ship->formationOffset);
            vecAddTo(positions[i],selection->ShipPtr[0]->posinfo.position);

            selection->ShipPtr[i]->formationOffset;
            shipWantsPosition[i]=0;
            shipGetsPosition[i]=0;
            positionTaken[i] = FALSE;
        }

        //positions,posiitiontaken,shipwantsposiiton initialized
        //find position that ship WANTS
        for(i=1;i<numShips;i++)
        {
            ship = selection->ShipPtr[i];
            minDist = REALlyBig;
            for(j=1;j<numShips;j++)
            {
                vecSub(tempvec,ship->posinfo.position,positions[j]);
                tempdist = vecMagnitudeSquared(tempvec);
                if(tempdist < minDist)
                {
                    //new best point
                    minDist = tempdist;
                    shipWantsPosition[i] = j;
                }
            }
        }

        //shipWantsPosition full of points that each ship WANTS to go to
        //now go through list and assign positions in
        for(i=1;i<numShips;i++)
        {
            poswanted = shipWantsPosition[i];
            if(poswanted == 0)
                continue;
            numWhoWant = 1;
            //see if someone else wants this position too...
            for(j=1;j<numShips;j++)
            {
                if(j==i)
                    continue;
                if(shipWantsPosition[j] == poswanted)
                {
                    //contention exists for point poswanted
                    numWhoWant++;
                }
            }
            if(numWhoWant == 1)
            {
                //no contention
                shipGetsPosition[i] = poswanted;
                positionTaken[poswanted] = TRUE;
            }
            else
            {
                //numWhoWant > 1 hence contention
                //must resolve conflict
                minDist = REALlyBig;
                for(j=1;j<numShips;j++)
                {
                    ship = selection->ShipPtr[j];
                    if(shipWantsPosition[j] == poswanted)
                    {
                        vecSub(tempvec,ship->posinfo.position,positions[poswanted]);
                        tempdist = vecMagnitudeSquared(tempvec);
                        if(tempdist < minDist)
                        {
                            minDist = tempdist;
                            shipwhogetsit = j;
                        }
                    }
                }
                //shipwhogetsit is the shipindex of the ship that won the point
                for(j=1;j<numShips;j++)
                {
                    if(shipWantsPosition[j] == poswanted &&
                       j != shipwhogetsit)
                    {
                        shipWantsPosition[j] = 0;   //set so we know to fix it up later
                        shipGetsPosition[j] = 0;
                    }
                }
                shipGetsPosition[shipwhogetsit] = poswanted;
                positionTaken[poswanted] = TRUE;
            }
        }
        //now match up free ships with closest free slots
        for(i=1;i<numShips;i++)
        {
            if(shipWantsPosition[i] == 0)
            {
                //ship needs a position
                minDist = REALlyBig;
                ship = selection->ShipPtr[i];
                for(j=1;j<numShips;j++)
                {
                    if(positionTaken[j] == FALSE)
                    {
                        vecSub(tempvec,ship->posinfo.position,positions[j]);
                        tempdist = vecMagnitudeSquared(tempvec);
                        if(tempdist < minDist)
                        {
                            minDist = tempdist;
                            shipwhogetsit = j;      //use as tempvar
                        }
                    }
                }
                shipWantsPosition[i] = shipwhogetsit;   //set so we know to fix it up later
                shipGetsPosition[i] = shipwhogetsit;
                positionTaken[shipwhogetsit] = TRUE;
            }
        }

        testSel->ShipPtr[0] = selection->ShipPtr[0];
        for(i=1;i<numShips;i++)
        {
        #ifdef HW_DEBUG
            dbgAssert(shipGetsPosition[i] != 0);
        #endif
            testSel->ShipPtr[shipGetsPosition[i]] = selection->ShipPtr[i];
        }
        for(i=0;i<numShips;i++)
        {
            selection->ShipPtr[i]=testSel->ShipPtr[i];
        }
        //selection has been re-organized correctly...so now lets
        //recalculate the proper offsets and start drinking
        FormationCalculateOffsets(formationcommand);

        tempdist3 = calculateTravelDistance(formationcommand);
        if(tempdist3 < mindistleader)
        {
            mindistleader = tempdist3;
            for(i=0;i<numShips;i++)
            {
                optimumSel->ShipPtr[i] = selection->ShipPtr[i];
            }
        }

        if (!redo)
        {
            break;
        }
    }

/*    optimumSel->ShipPtr[0] = selection->ShipPtr[0];
    for(i=1;i<numShips;i++)
    {
#ifdef HW_DEBUG
        dbgAssert(shipGetsPosition[i] != 0);
#endif
        optimumSel->ShipPtr[shipGetsPosition[i]] = selection->ShipPtr[i];
    }

  */
    for(i=0;i<numShips;i++)
    {
        selection->ShipPtr[i]=optimumSel->ShipPtr[i];
    }
    //selection has been re-organized correctly...so now lets
    //recalculate the proper offsets and start drinking
    FormationCalculateOffsets(formationcommand);

    //MAYBE make the big/small arangement here...then do this?

    memFree(optimumSel);
    memFree(testSel);
    memFree(originalSel);

    if (numShips > COMMAND_MAX_SHIPS-1)
    {
        memFree(positions);
        memFree(shipWantsPosition);
        memFree(shipGetsPosition);
        memFree(positionTaken);
    }

}

/*-----------------------------------------------------------------------------
    Name        : FillInFormationSpecifics
    Description : fills in the formation details
    Inputs      : formationcommand
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void FillInFormationSpecifics(CommandToDo *formationcommand,TypeOfFormation formation,sdword newformation)
{
    sdword i;
    SelectCommand *selection = formationcommand->selection;
    sdword numShips = selection->numShips;

    // fill in formation specifics:
    formationcommand->formation.formationtype = formation;
    formationcommand->formation.tacticalState = TightFormation;       //default behavior
    formationcommand->formation.formationLocked = FALSE;                //initialize a formation as unlocked
    formationcommand->formation.tacticsUpdate = 0.0f;                 //this formation needs updating
    formationcommand->formation.flipselectionfixed = FALSE;
    formationcommand->formation.needFix = FALSE;
    formationcommand->formation.enders = FALSE;
    formationcommand->formation.percentmaxspeed = FORMATION_TRAVELVEL_MINSCALE;
    setFormationTravelVelocity(formationcommand);
    formationcommand->formation.flagTravelSlowestShip = FORMATION_TRAVEL_SLOWEST_SHIP;  // obsolete flag, but set anyway
    formationcommand->formation.doneInitialAttack = FALSE;
    formationcommand->formation.sortorder = 0;
    formationcommand->formation.pulse = 0;

    if(newformation)
    {
        //formation group is new so set this properly
        formationcommand->formation.flipState = 0;
    }

    for (i=0;i<numShips;i++)
    {
        FillInShipFormationStuff(selection->ShipPtr[i],formationcommand);
    }

//    formationArrangeOptimum(formationcommand);
//    FormationCalculateOffsets(formationcommand);

    formationArrageCrazyOptimum(formationcommand);

    switch (formation)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case SPHERE_FORMATION:
        case CUSTOM_FORMATION:
        case FORCEFIELD_WALL_FORMATION:
        case FORCEFIELD_SPHERE_FORMATION:
            formationcommand->formation.formationSpecificInfo = NULL;
            break;

        default:
            dbgAssert(FALSE);
            break;
    }
}

typedef struct UniqueObjAndFreq
{
    void *obj;
    sdword freq;
} UniqueObjAndFreq;

typedef struct CountFreqObjs
{
    sdword numUniqueObjs;
    UniqueObjAndFreq *uniqueObjFreqs;
} CountFreqObjs;

void uofInit(CountFreqObjs *countFreqObjs,sdword max)
{
    countFreqObjs->numUniqueObjs = 0;
    countFreqObjs->uniqueObjFreqs = memAlloc(sizeof(UniqueObjAndFreq)*max,"uniqueobjfreq",Pyrophoric);
}

void uofClose(CountFreqObjs *countFreqObjs)
{
    memFree(countFreqObjs->uniqueObjFreqs);
}

void uofAddObj(CountFreqObjs *countFreqObjs,void *obj)
{
    sdword i;

    for (i=0;i<countFreqObjs->numUniqueObjs;i++)
    {
        if (obj == countFreqObjs->uniqueObjFreqs[i].obj)
        {
             countFreqObjs->uniqueObjFreqs[i].freq++;
             return;
        }
    }

    countFreqObjs->uniqueObjFreqs[countFreqObjs->numUniqueObjs].obj = obj;
    countFreqObjs->uniqueObjFreqs[countFreqObjs->numUniqueObjs].freq = 1;
    countFreqObjs->numUniqueObjs++;
}

void *uofFindObjWithMaxFreq(CountFreqObjs *countFreqObjs)
{
    sdword i;
    sdword max = -1;
    sdword maxindex = -1;

    for (i=0;i<countFreqObjs->numUniqueObjs;i++)
    {
        if (countFreqObjs->uniqueObjFreqs[i].freq > max)
        {
            maxindex = i;
            max = countFreqObjs->uniqueObjFreqs[i].freq;
        }
    }

    if (maxindex != -1)
    {
        return countFreqObjs->uniqueObjFreqs[maxindex].obj;
    }

    return NULL;
}


/*-----------------------------------------------------------------------------
    Name        : MajorityShipsAreDoing
    Description : returns what majority of ships are doing (attacking or moving)
    Inputs      : selectcom
    Outputs     : if COMMAND_MOVE returned, outputs movingto
                  if COMMAND_ATTACK returned, outputs attacking.  In this case you must
                    do a growSelectClose on attacking.
    Return      : returns what majority of ships are doing (attacking or moving)
----------------------------------------------------------------------------*/
sdword MajorityShipsAreDoing(SelectCommand *selectcom,vector *movingto,GrowSelection *attacking)
{
    sdword numShips = selectcom->numShips;
    sdword i;
    Ship *ship;
    sdword nummoving = 0;
    sdword numattacking = 0;

    // first pass: are any ships moving or attacking?
    for (i=0;i<numShips;i++)
    {
        ship = selectcom->ShipPtr[i];
        if (ship->command)
        {
            if (ship->command->ordertype.order == COMMAND_ATTACK)
            {
                numattacking++;
            }
            else if (ship->command->ordertype.order == COMMAND_MOVE)
            {
                nummoving++;
            }
        }
    }

    if (numattacking && nummoving)
    {
        return 0;       // ambiguous - some attacking, some moving
    }

    if (numattacking > (numShips - numattacking))
    {
        if (AreAllShipsAttackCapable(selectcom))
        {
            // amalgamate a list of targets everyone is going to attack in formation
            growSelectInit(attacking);

            for (i=0;i<numShips;i++)
            {
                ship = selectcom->ShipPtr[i];
                if ((ship->command) && (ship->command->ordertype.order == COMMAND_ATTACK))
                {
                    SelectCommand *attacksel = (SelectCommand *)ship->command->attack;
                    sdword numTargets = attacksel->numShips;
                    sdword j;
                    for (j=0;j<numTargets;j++)
                    {
                        if (!ShipInSelection(selectcom,attacksel->ShipPtr[j]))      // in case force attacking
                        {
                            growSelectAddShipNoDuplication(attacking,attacksel->ShipPtr[j]);
                        }
                    }
                }
            }

            if (attacking->selection->numShips == 0)
            {
                growSelectClose(attacking);
                return 0;
            }

            return COMMAND_ATTACK;
        }
        else
        {
            return 0;       // not all ships attack capable - therefore do not attack
        }
    }
    else if (nummoving > (numShips - nummoving))
    {
        if (AreShipsMobile(selectcom))
        {
            // find a tovector the majority of ships are moving to
            CountFreqObjs countFreqObjs;
            CommandToDo *mostfreqmoveto;

            uofInit(&countFreqObjs,nummoving);

            for (i=0;i<numShips;i++)
            {
                ship = selectcom->ShipPtr[i];
                if ((ship->command) && (ship->command->ordertype.order == COMMAND_MOVE))
                {
                    uofAddObj(&countFreqObjs,ship->command);
                }
            }

            mostfreqmoveto = uofFindObjWithMaxFreq(&countFreqObjs);
            uofClose(&countFreqObjs);

            if (!mostfreqmoveto)
            {
                return 0;
            }

            *movingto = mostfreqmoveto->move.destination;

            return COMMAND_MOVE;
        }
        else
        {
            return 0;       // not all ships are mobile - therefore do not move
        }
    }
    else
    {
        return 0;           // no one attacking or moving
    }
}

/*-----------------------------------------------------------------------------
    Name        : clFormation
    Description : Command to form a formation
    Inputs      : selectcom, formation
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void clFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation)
{
    CommandToDo *newcommand;
    CommandToDo *alreadyformation;
    SelectCommand *formationSelection;
    SelectCommand *selection;
    udword sizeofselection;
    sdword majorityshipsare;
    sdword pathState;
    sdword numWaypoints;
    sdword curWaypoint;
    vector movePoints[20];
    GrowSelection majorityshipsattacking;
    MaxSelection filteredSelection;
    sdword i;

    if (!FORMATION_IS_VALID(formation))
    {
        dbgFatalf(DBG_Loc,"Invalid formation %d",formation);
    }

    filteredSelection.numShips = 0;
    filteredSelection.timeLastStatus = selectcom->timeLastStatus;
    for (i = 0; i < selectcom->numShips; i++)
    {
        Ship *ship = selectcom->ShipPtr[i];
        ShipStaticInfo *shipstaticinfo = (ShipStaticInfo *)ship->staticinfo;

        if (shipstaticinfo->custshipheader.CustShipFormationFilter == NULL ||
            shipstaticinfo->custshipheader.CustShipFormationFilter(ship, formation, NULL))
        {
            filteredSelection.ShipPtr[filteredSelection.numShips++] = ship;
        }
    }

    if (formation == FORCEFIELD_SPHERE_FORMATION &&
        filteredSelection.numShips > 12)
    {
        for (i = 12; i < filteredSelection.numShips; i++)
        {
            clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)filteredSelection.ShipPtr[i]);
            filteredSelection.ShipPtr[i] = NULL;
        }
        filteredSelection.numShips = 12;
    }
    else if (formation == FORCEFIELD_WALL_FORMATION &&
             filteredSelection.numShips > 24)
    {
        for (i = 24; i < filteredSelection.numShips; i++)
        {
            clRemoveTargetFromSelection((SelectAnyCommand *)&selSelected, (TargetPtr)filteredSelection.ShipPtr[i]);
            filteredSelection.ShipPtr[i] = NULL;
        }
        filteredSelection.numShips = 24;
    }

    if (filteredSelection.numShips < ABSOLUTE_MIN_SHIPS_IN_FORMATION)
    {
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nNot enough ships to do a formation");
#endif
        return;
    }


    formationSelection = (SelectCommand *)&filteredSelection;

    //treat cleanup like a halt!
    salCapExtraSpecialOrderCleanUp(formationSelection,COMMAND_HALT,NULL,NULL);


    if ((alreadyformation = IsSelectionAlreadyDoingSomething(comlayer,formationSelection)) != NULL)
    {
        if (alreadyformation->ordertype.attributes & COMMAND_IS_FORMATION)
        {
            if (alreadyformation->formation.formationtype == formation)
            {
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nAlready gave that formation order");
#endif
                return;
            }
            else
            {
                if (alreadyformation->formation.formationSpecificInfo != NULL)
                {
                    memFree(alreadyformation->formation.formationSpecificInfo);
                    alreadyformation->formation.formationSpecificInfo = NULL;
                }

                alreadyformation->formation.formationtype = formation;
                formationTypeHasChanged(alreadyformation);

                PrepareShipsForCommand(alreadyformation,TRUE);
#ifdef DEBUG_FORMATIONS
        dbgMessage("\nChanging formation");
#endif
                return;
            }
        }
        else if (alreadyformation->ordertype.order == COMMAND_MOVE)
        {
            // Change from normal move command to move in formation
            alreadyformation->ordertype.attributes |= COMMAND_IS_FORMATION;
            formationInit(alreadyformation, formation, NULL, TRUE);
            PrepareShipsForCommand(alreadyformation,TRUE);
            return;
        }
        else if (alreadyformation->ordertype.attributes & COMMAND_IS_PROTECTING)
        {
            if ((formation == FORCEFIELD_WALL_FORMATION ||
                 formation == FORCEFIELD_SPHERE_FORMATION) &&
                (alreadyformation->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING))
            {
                ClearPassiveAttacking(alreadyformation);
            }
            alreadyformation->ordertype.attributes |= COMMAND_IS_FORMATION;
            formationInit(alreadyformation, formation, NULL, TRUE);
            PrepareShipsForCommand(alreadyformation,TRUE);
            return;
        }
        else if (alreadyformation->ordertype.order == COMMAND_ATTACK)
        {
            alreadyformation->ordertype.attributes |= COMMAND_IS_FORMATION;
            formationInit(alreadyformation, formation, NULL, TRUE);
            PrepareShipsForCommand(alreadyformation,TRUE);
            return;
        }
    }

    majorityshipsare = formMajorityShipsAreDoing(
        formationSelection,
        &pathState,
        &numWaypoints,
        (sdword *)movePoints,
        &curWaypoint,
        &majorityshipsattacking);

#ifdef DEBUG_FORMATIONS
    dbgMessage("\nReceived Order to do formation");
#endif

    RemoveShipsFromDoingStuff(comlayer,formationSelection);

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    sizeofselection = sizeofSelectCommand(formationSelection->numShips);
    selection = memAlloc(sizeofselection,"ToDoSelection",NonVolatile);
    memcpy(selection,formationSelection,sizeofselection);

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_NULL;
    newcommand->ordertype.attributes = COMMAND_IS_FORMATION;

    formationInit(newcommand, formation, NULL, TRUE);

    InitShipsForAI(selection,TRUE);

    PrepareShipsForCommand(newcommand,TRUE);
    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    if (majorityshipsare == COMMAND_MOVE)
    {
        sdword initialWaypoint = curWaypoint - 1;

        dbgAssert(numWaypoints > 0);
        if (initialWaypoint < 0)
        {
            initialWaypoint = numWaypoints - 1;
        }

        {
            MoveCommandPacked packedMove;

            packedMove = clBuildMovePathCommand(
                pathState,
                movePoints[initialWaypoint],
                numWaypoints,
                movePoints);
            ChangeOrderToMove(newcommand, packedMove);
        }
    }
    else if (majorityshipsare == COMMAND_ATTACK)
    {
        ChangeOrderToAttack(newcommand,(SelectAnyCommand *)majorityshipsattacking.selection);
        growSelectClose(&majorityshipsattacking);
    }
}

/*=============================================================================
    The following routines are for the military parades ships form when they are launched from a ship
=============================================================================*/

typedef struct
{
    real32 upoffset;
    real32 rightoffset;
    real32 headingoffset;
    real32 upspacing;
    real32 rightspacing;
    real32 headingspacing;
    udword facedirection;
} SlotInfo;

typedef struct
{
    SlotInfo slotinfos[MAX_MILITARY_SLOTS];
} ParadeTypeInfo;

#define PARADE_SMOTHERSHIP       0
#define PARADE_BMOTHERSHIP       1
#define PARADE_SCARRIER          2
#define PARADE_BCARRIER          3
#define PARADE_SLIPGATE          4
#define PARADE_SPROCESSOR        5
#define PARADE_BPROCESSOR        6
#define PARADE_DPRISONSHIP       7
#define PARADE_R1CARRIER         8
#define PARADE_R2CARRIER         9
#define PARADE_P1MOTHERSHIP      10
#define PARADE_BHEAVYCRUISER     11
#define NUMBER_PARADE_TYPES      12

// HW1 aliases
#define PARADE_R1MOTHERSHIP      PARADE_SMOTHERSHIP
#define PARADE_R2MOTHERSHIP      PARADE_BMOTHERSHIP
#define PARADE_P2MOTHERSHIP      PARADE_P1MOTHERSHIP

#define SUBSLOT_BATCH      10      // subslots are allocated in batches of this many

static ParadeTypeInfo paradeTypeInfos[NUMBER_PARADE_TYPES];

void scriptSetSlotInfoCB(char *directory,char *field,void *dataToFillIn);

scriptStructEntry ParadeInfoScriptTable[] =
{
    // Somtaaw ship slots
    { "SlotPositionInfo[SLOT_SRECON]",           scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SRECON],           (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SMIMIC]",           scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SMIMIC],           (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SMCV]",             scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SMCV],             (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SWORKER]",          scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SWORKER],          (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SPROCESSOR]",       scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SPROCESSOR],       (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SCARRIER0]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SCARRIER0],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SCARRIER1]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SCARRIER1],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SCARRIER2]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SCARRIER2],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SSENTINEL]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SSENTINEL],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SACOLYTE]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SACOLYTE],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SACV]",             scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SACV],             (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SRAMMINGFRIGATE]",  scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SRAMMINGFRIGATE],  (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SMULTIBEAMFRIGATE]",scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SMULTIBEAMFRIGATE],(udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SHIVEFRIGATE]",     scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SHIVEFRIGATE],     (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SDREADNOUGHT]",     scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SDREADNOUGHT],     (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SLEECH]",           scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SLEECH],           (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SDESTROYER]",       scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SDESTROYER],       (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SMOTHERSHIP]",      scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SMOTHERSHIP],      (udword)&paradeTypeInfos[0] },

    // Beast ship slots
    { "SlotPositionInfo[SLOT_BRECON]",           scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BRECON],           (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BWORKER]",          scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BWORKER],          (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BPROCESSOR]",       scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BPROCESSOR],       (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BHIVEFRIGATE]",     scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BHIVEFRIGATE],     (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BMULTIBEAMFRIGATE]",scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BMULTIBEAMFRIGATE],(udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BCARRIER0]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BCARRIER0],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BCARRIER1]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BCARRIER1],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BCARRIER2]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BCARRIER2],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BSENTINEL]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BSENTINEL],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BACOLYTE]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BACOLYTE],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BACV]",             scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BACV],             (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BINTERCEPTOR]",     scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BINTERCEPTOR],     (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BATTACKBOMBER]",    scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BATTACKBOMBER],    (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BCLOAKEDFIGHTER]",  scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BCLOAKEDFIGHTER],  (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BMULTIGUNCORVETTE]",scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BMULTIGUNCORVETTE],(udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BDFGFRIGATE]",      scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BDFGFRIGATE],      (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BHEAVYCORVETTE]",   scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BHEAVYCORVETTE],   (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BIONARRAYFRIGATE]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BIONARRAYFRIGATE], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BHEAVYCRUISER]",    scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BHEAVYCRUISER],    (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BMISSILECORVETTE]", scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BMISSILECORVETTE], (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BCRUISEMISSILE]",   scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BCRUISEMISSILE],   (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BRAMMINGFRIGATE]",  scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BRAMMINGFRIGATE],  (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_BMOTHERSHIP]",      scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_BMOTHERSHIP],      (udword)&paradeTypeInfos[0] },

    // HW1 k/t-race generic slots (R1/R2 carrier parade scripts)
    { "SlotPositionInfo[SLOT_Fighter0]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter0],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter1]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter1],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter2]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter2],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter3]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter3],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter4]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter4],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Fighter5]",         scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Fighter5],         (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette0]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette0],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette1]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette1],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette2]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette2],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Corvette3]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Corvette3],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Frigate]",          scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Frigate],          (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_SupportCorvette]",  scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_SupportCorvette],  (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_ResCollector]",     scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_ResCollector],     (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_NonCombat]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_NonCombat],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_Carrier]",          scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Carrier],          (udword)&paradeTypeInfos[0] },

    // P1 ship slots
    { "SlotPositionInfo[SLOT_P1Fighter]",        scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1Fighter],        (udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1IonArrayFrigate]",scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1IonArrayFrigate],(udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1MissileCorvette]",scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_P1MissileCorvette],(udword)&paradeTypeInfos[0] },
    { "SlotPositionInfo[SLOT_P1StandardCorvette]",scriptSetSlotInfoCB,(udword)&paradeTypeInfos[0].slotinfos[SLOT_P1StandardCorvette],(udword)&paradeTypeInfos[0] },

    // Fallback
    { "SlotPositionInfo[SLOT_Misc]",             scriptSetSlotInfoCB, (udword)&paradeTypeInfos[0].slotinfos[SLOT_Misc],             (udword)&paradeTypeInfos[0] },
    { NULL, NULL, 0, 0 }
};

void scriptSetSlotInfoCB(char *directory,char *field,void *dataToFillIn)
{
    SlotInfo *slotInfo = (SlotInfo *)dataToFillIn;

    RemoveCommasFromString(field);

    slotInfo->facedirection = 0;
    sscanf(field,"%f %f %f %f %f %f %d",&slotInfo->upoffset,&slotInfo->rightoffset,&slotInfo->headingoffset,&slotInfo->upspacing,&slotInfo->rightspacing,&slotInfo->headingspacing,&slotInfo->facedirection);
}

void paradeSetTweakables()
{
    sdword i;
    for (i = 0; i < NUMBER_PARADE_TYPES; i++)
        memset(&paradeTypeInfos[i], 0, sizeof(ParadeTypeInfo));

    scriptSetStruct(NULL,"Parade_sMothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_SMOTHERSHIP]);
    scriptSetStruct(NULL,"Parade_bMothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_BMOTHERSHIP]);
    scriptSetStruct(NULL,"Parade_sCarrier.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_SCARRIER]);
    scriptSetStruct(NULL,"Parade_bCarrier.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_BCARRIER]);
    scriptSetStruct(NULL,"Parade_Slipgate.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_SLIPGATE]);
    scriptSetStruct(NULL,"Parade_sProcessor.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_SPROCESSOR]);
    scriptSetStruct(NULL,"Parade_bProcessor.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_BPROCESSOR]);
    scriptSetStruct(NULL,"Parade_dPrisonShip.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_DPRISONSHIP]);
    scriptSetStruct(NULL,"Parade_R1Carrier.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_R1CARRIER]);
    scriptSetStruct(NULL,"Parade_R2Carrier.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_R2CARRIER]);
    scriptSetStruct(NULL,"Parade_P1Mothership.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_P1MOTHERSHIP]);
    scriptSetStruct(NULL,"Parade_bHeavyCruiser.script",ParadeInfoScriptTable,(ubyte *)&paradeTypeInfos[PARADE_BHEAVYCRUISER]);

}

/*-----------------------------------------------------------------------------
    Name        : processMilitaryParadeToDo
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void processMilitaryParadeToDo(struct CommandToDo *command,bool32 passiveAttacked)
{
    MilitaryParadeCommand *militaryParade = command->militaryParade;
    Ship *leader = militaryParade->aroundShip;
    sdword paradeType = militaryParade->paradeType;
    ParadeTypeInfo *paradeTypeInfo = &paradeTypeInfos[paradeType];
    vector up,right,heading;
    vector faceheading;
    sdword slot;
    sdword subslot;
    MilitarySlot *thisslot;
    SlotInfo *thisslotinfo;
    vector positionat;
    vector positionchange;
    Ship *ship;
    ShipStaticInfo *shipstatic;
    bool32 dontrotate;
    bool32 firstShipOfSlot;
    bool32 steadyFormation = TRUE;

    dbgAssert(paradeType >= 0);
    dbgAssert(paradeType < NUMBER_PARADE_TYPES);

    matGetVectFromMatrixCol1(up,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);

    if (vecMagnitudeSquared(leader->posinfo.velocity) >= PARADE_CONSIDERED_STEADY_VELSQR)
    {
        steadyFormation = FALSE;
    }

    // for each slot, position the ship correctly

    for (slot=0;slot<MAX_MILITARY_SLOTS;slot++)
    {
        thisslot = militaryParade->militarySlots[slot];

        if (thisslot->highestSlotUsed < 0)
        {
            continue;
        }

        thisslotinfo = &paradeTypeInfo->slotinfos[slot];

        // calculate starting positonat of this slot
        positionat = leader->posinfo.position;
        vecAddToScalarMultiply(positionat,up,thisslotinfo->upoffset);
        vecAddToScalarMultiply(positionat,right,thisslotinfo->rightoffset);
        vecAddToScalarMultiply(positionat,heading,thisslotinfo->headingoffset);

        vecScalarMultiply(positionchange,right,thisslotinfo->rightspacing);
        vecAddToScalarMultiply(positionchange,up,thisslotinfo->upspacing);              // added by MG July 23
        vecAddToScalarMultiply(positionchange,heading,thisslotinfo->headingspacing);    //

        switch (thisslotinfo->facedirection)
        {
            case 0:
                faceheading = heading;
                break;
            case 1:
                vecCopyAndNegate(faceheading,heading);
                break;
            case 2:
                faceheading = right;
                break;
            case 3:
                vecCopyAndNegate(faceheading,right);
                break;
            default:
                dbgAssert(FALSE);
                break;
        }

        dbgAssert(thisslot->highestSlotUsed < thisslot->numSubslots);

        firstShipOfSlot = TRUE;

        for (subslot=0;subslot<=thisslot->highestSlotUsed;subslot++)
        {
            ship = thisslot->subslots[subslot].ship;
            if (ship != NULL)
            {
                shipstatic = (ShipStaticInfo *)ship->staticinfo;
                dontrotate = (passiveAttacked & (bool32)shipstatic->rotateToRetaliate);
                if (firstShipOfSlot)
                {
                    firstShipOfSlot = FALSE;
                }
                else
                {
                    ship->aistateattack = 0;
                }

                if (dontrotate)
                {
                    if (steadyFormation && MoveReachedDestinationVariable(ship,&positionat,100.0f))
                    {
                        ship->flags |= SOF_TargetableHW1;
                    }
                    if (!MoveReachedDestinationVariable(ship,&positionat,10.0f))
                    {
                        aishipFlyToPointAvoidingObjsWithVel(ship,&positionat,AISHIP_FastAsPossible,0.0f,&leader->posinfo.velocity);
                    }
                    else if (steadyFormation)
                    {
                        aitrackSteadyShipDriftOnly(ship);
                    }
                }
                else
                {
                    real32 tmpdistaway = MoveLeftToGoDistance(ship,&positionat);
                    udword tmpuseflags = AISHIP_PointInDirectionFlying | AISHIP_FastAsPossible;
                    bool32 trackheading = FALSE;

                    if (thisslotinfo->facedirection)
                    {
                        trackheading = TRUE;        // if facing nonstandard direction, always track heading
                    }

                    if (isCapitalShipStaticOrBig(shipstatic))
                    {
                        if (tmpdistaway <= 500.0f)
                        {
                            if ((tmpdistaway <= 100.0f) && (steadyFormation)) ship->flags |= SOF_TargetableHW1;
                            trackheading = TRUE;
                        }
                    }
                    else if (tmpdistaway <= 100.0f)
                    {
                        if (steadyFormation)
                        {
                            ship->flags |= SOF_TargetableHW1;
                        }

                        trackheading = TRUE;
dontadjustheading:;
                    }

                    if (trackheading)
                    {
                        aitrackHeadingWithBank(ship,&(faceheading),FLYSHIP_HEADINGACCURACY,shipstatic->sinbank);
                        tmpuseflags = AISHIP_FastAsPossible;
                    }

                    if (tmpdistaway >= 10.0f)
                    {
                        aishipFlyToPointAvoidingObjsWithVel(ship,&positionat,tmpuseflags,0.0f,&leader->posinfo.velocity);
dontmovecloser:;
                    }
                    else if (steadyFormation)
                    {
                        aitrackSteadyShipDriftOnly(ship);
                    }
                }
            }

            vecAddTo(positionat,positionchange);
        }
    }
}

bool32 shipInMilitaryParade(ShipPtr ship)
{
    CommandToDo *command = getShipAndItsCommand(&universe.mainCommandLayer,ship);
    if (command == NULL)
    {
        return FALSE;
    }
    if (command->ordertype.order == COMMAND_MILITARYPARADE)
    {
        return TRUE;
    }
    return FALSE;
}

void setMilitaryParade(struct CommandToDo *command)
{
    MilitaryParadeCommand *militaryParade = command->militaryParade;
    Ship *leader = militaryParade->aroundShip;
    sdword paradeType = militaryParade->paradeType;
    ParadeTypeInfo *paradeTypeInfo = &paradeTypeInfos[paradeType];
    vector up,right,heading;
    sdword slot;
    sdword subslot;
    MilitarySlot *thisslot;
    SlotInfo *thisslotinfo;
    vector positionat;
    vector positionchange;
    Ship *ship;

    dbgAssert(paradeType >= 0);
    dbgAssert(paradeType < NUMBER_PARADE_TYPES);

    matGetVectFromMatrixCol1(up,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol2(right,leader->rotinfo.coordsys);
    matGetVectFromMatrixCol3(heading,leader->rotinfo.coordsys);

    // for each slot, position the ship correctly

    for (slot=0;slot<MAX_MILITARY_SLOTS;slot++)
    {
        thisslot = militaryParade->militarySlots[slot];

        if (thisslot->highestSlotUsed < 0)
        {
            continue;
        }

        thisslotinfo = &paradeTypeInfo->slotinfos[slot];

        // calculate starting positonat of this slot
        positionat = leader->posinfo.position;
        vecAddToScalarMultiply(positionat,up,thisslotinfo->upoffset);
        vecAddToScalarMultiply(positionat,right,thisslotinfo->rightoffset);
        vecAddToScalarMultiply(positionat,heading,thisslotinfo->headingoffset);

        vecScalarMultiply(positionchange,right,thisslotinfo->rightspacing);
        vecAddToScalarMultiply(positionchange,up,thisslotinfo->upspacing);              // added by MG July 23
        vecAddToScalarMultiply(positionchange,heading,thisslotinfo->headingspacing);    //

        dbgAssert(thisslot->highestSlotUsed < thisslot->numSubslots);

        for (subslot=0;subslot<=thisslot->highestSlotUsed;subslot++)
        {
            ship = thisslot->subslots[subslot].ship;
            if (ship != NULL)
            {
                posSetShipPosition(ship, &positionat);
                switch (thisslotinfo->facedirection)
                {
                    vector temp1,temp2;

                    case 0:     // forward
                        ship->rotinfo.coordsys = leader->rotinfo.coordsys;
                        break;

                    case 1:     // back
                        vecCopyAndNegate(temp1,heading);
                        vecCopyAndNegate(temp2,right);
                        matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&temp2,&temp1);
                        break;

                    case 2:     // right
                        vecCopyAndNegate(temp1,heading);
                        matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&temp1,&right);
                        break;

                    case 3:     // left
                        vecCopyAndNegate(temp2,right);
                        matCreateMatFromVecs(&ship->rotinfo.coordsys,&up,&heading,&temp2);
                        break;

                    default:
                        dbgAssert(FALSE);
                        break;
                }
                univUpdateObjRotInfo((SpaceObjRot *)ship);
            }

            vecAddTo(positionat,positionchange);
        }
    }
}

void FreeMilitaryParadeContents(MilitaryParadeCommand *militaryParade)
{
    sdword i;

    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        dbgAssert(militaryParade->militarySlots[i]);
        memFree(militaryParade->militarySlots[i]);
        militaryParade->militarySlots[i] = NULL;
    }
}

/*-----------------------------------------------------------------------------
    Name        : GetShipSlot
    Description : returns the correct slot a ship belongs to
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
sdword GetShipSlot(Ship *ship, MilitaryParadeCommand *militaryParade)
{
    switch (ship->shiptype)
    {
        // Somtaaw ships
        case sRecon:                return SLOT_SRECON;
        case sAcolyte:
        case 0x85:                  return SLOT_SACOLYTE;
        case sMimic:                return SLOT_SMIMIC;
        case sLeech:                return SLOT_SLEECH;
        case sACV:                  return SLOT_SACV;
        case sMCV:                  return SLOT_SMCV;
        case sRammingFrigate:       return SLOT_SRAMMINGFRIGATE;
        case sMultiBeamFrigate:     return SLOT_SMULTIBEAMFRIGATE;
        case sHiveFrigate:          return SLOT_SHIVEFRIGATE;
        case sSwarmer:
        case bSwarmer:              return SLOT_SLEECH;
        case sDreadnought:          return SLOT_SDREADNOUGHT;
        case sCarrier:
            if (militaryParade->militarySlots[SLOT_SCARRIER0]->highestSlotUsed >= 0)
                return (militaryParade->militarySlots[SLOT_SCARRIER1]->highestSlotUsed >= 0) + SLOT_SCARRIER1;
            return SLOT_SCARRIER0;
        case sWorker:               return SLOT_SWORKER;
        case sProcessor:            return SLOT_SPROCESSOR;
        case sSentinel:             return SLOT_SSENTINEL;
        case sMothership:           return SLOT_SMOTHERSHIP;
        case sDestroyer:            return SLOT_SDESTROYER;

        // Beast ships
        case bRecon:                return SLOT_BRECON;
        case bAcolyte:              return SLOT_BACOLYTE;
        case bInterceptor:          return SLOT_BINTERCEPTOR;
        case bCloakedFighter:       return SLOT_BCLOAKEDFIGHTER;
        case bAttackBomber:         return SLOT_BATTACKBOMBER;
        case bACV:                  return SLOT_BACV;
        case bMultiGunCorvette:     return SLOT_BMULTIGUNCORVETTE;
        case bHeavyCorvette:        return SLOT_BHEAVYCORVETTE;
        case bMultiBeamFrigate:     return SLOT_BMULTIBEAMFRIGATE;
        case bHiveFrigate:          return SLOT_BHIVEFRIGATE;
        case bIonArrayFrigate:      return SLOT_BIONARRAYFRIGATE;
        case bDFGFrigate:           return SLOT_BDFGFRIGATE;
        case bHeavyCruiser:
            return (militaryParade->aroundShip->shiptype != bMothership) ? SLOT_Misc : SLOT_BHEAVYCRUISER;
        case bCarrier:
            if (militaryParade->militarySlots[SLOT_BCARRIER0]->highestSlotUsed >= 0)
                return (militaryParade->militarySlots[SLOT_BCARRIER1]->highestSlotUsed >= 0) + SLOT_BCARRIER1;
            return SLOT_BCARRIER0;
        case bRammingFrigate:       return SLOT_BRAMMINGFRIGATE;
        case bWorker:               return SLOT_BWORKER;
        case bProcessor:            return SLOT_BPROCESSOR;
        case bSentinel:             return SLOT_BSENTINEL;
        case bMothership:           return SLOT_BMOTHERSHIP;
        case bCruiseMissile:        return SLOT_BCRUISEMISSILE;
        case bMissileCorvette:      return SLOT_BMISSILECORVETTE;

        // HW1 k/t-race ships
        case kAttackBomber:
        case tAttackBomber:         return SLOT_Fighter1;
        case kCarrier:              return SLOT_Carrier;
        case kCloakGenerator:
        case kGravWellGenerator:
        case tCloakGenerator:
        case tGravWellGenerator:    return SLOT_NonCombat;
        case kHeavyCorvette:        return SLOT_Corvette0;
        case kHeavyCruiser:
        case tHeavyCruiser:         return SLOT_HeavyCruiser;
        case kHeavyDefender:
        case tHeavyDefender:        return SLOT_Fighter2;
        case kHeavyInterceptor:
        case tHeavyInterceptor:     return SLOT_Fighter0;
        case kMultiGunCorvette:
        case tMultiGunCorvette:     return SLOT_Corvette1;
        case kSalCapCorvette:
        case tSalCapCorvette:       return SLOT_Corvette2;
        case kStandardDestroyer:
        case tStandardDestroyer:    return SLOT_Destroyer;
        case tMinelayerCorvette:    return SLOT_Corvette3;

        // P1 ships
        case p1Fighter:             return SLOT_P1Fighter;
        case p1IonArrayFrigate:     return SLOT_P1IonArrayFrigate;
        case p1MissileCorvette:     return SLOT_P1MissileCorvette;
        case p1StandardCorvette:    return SLOT_P1StandardCorvette;

        default:
            return SLOT_Misc;
    }
}

void CalculateNewHighestUsedSlot(MilitarySlot *militarySlot)
{
    sdword i;

    for (i=militarySlot->numSubslots-1;i>=0;i--)
    {
        if (militarySlot->subslots[i].ship != NULL)
        {
            militarySlot->highestSlotUsed = i;
            return;
        }
    }
    militarySlot->highestSlotUsed = -1;
}

void RemoveShipFromMilitaryParade(Ship *shiptoremove,MilitaryParadeCommand *militaryParade)
{
    sdword slot;
    sdword subslot;
    MilitarySlot *militarySlot;

    for (slot=0;slot<MAX_MILITARY_SLOTS;slot++)
    {
        militarySlot = militaryParade->militarySlots[slot];

        dbgAssert(militarySlot->highestSlotUsed < militarySlot->numSubslots);

        for (subslot=0;subslot<=militarySlot->highestSlotUsed;subslot++)
        {
            if (militarySlot->subslots[subslot].ship == shiptoremove)
            {
                // found shiptoremove
                militarySlot->subslots[subslot].ship = NULL;
                CalculateNewHighestUsedSlot(militarySlot);
                return;
            }
        }
    }

    dbgAssert(FALSE);       // should never reach here
}

/*-----------------------------------------------------------------------------
    Name        : PutShipInMilitaryParade
    Description : puts a ship into the correct position in a militaryParade
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void PutShipInMilitaryParade(Ship *ship,MilitaryParadeCommand *militaryParade)
{
    sdword slot;
    MilitarySlot *militarySlot;
    sdword i;
    sdword paradeType = militaryParade->paradeType;
    ParadeTypeInfo *paradeTypeInfo = &paradeTypeInfos[paradeType];
    SlotInfo *slotinfo;

    sdword numSubslotsToAllocate;
    sdword sizeofNewSlot;
    MilitarySlot *newMilitarySlot;

    slot = GetShipSlot(ship, militaryParade);
    dbgAssert(slot >= 0);
    dbgAssert(slot < MAX_MILITARY_SLOTS);
    slotinfo = &paradeTypeInfo->slotinfos[slot];
    if ((slotinfo->upoffset == 0.0f) && (slotinfo->rightoffset == 0.0f) && (slotinfo->headingoffset == 0.0f))
    {
        // this slot isn't defined well, perhaps a captured ship in another race's parade?  Put in MiscSlot
        slot = SLOT_Misc;
    }
    militarySlot = militaryParade->militarySlots[slot];

    // find first empty subslot
    for (i=0;i<militarySlot->numSubslots;i++)
    {
        if (militarySlot->subslots[i].ship == NULL)
        {
            // found a empty slot
            militarySlot->subslots[i].ship = ship;
            CalculateNewHighestUsedSlot(militarySlot);
            return;
        }
    }

    // did not find a empty slot.  All full.  We should allocate more slots.

    numSubslotsToAllocate = militarySlot->numSubslots + SUBSLOT_BATCH;
    sizeofNewSlot = sizeofMilitarySlot(numSubslotsToAllocate);
    newMilitarySlot = memAlloc(sizeofNewSlot,"newmilslot",0);

    newMilitarySlot->numSubslots = numSubslotsToAllocate;
    for (i=0;i<militarySlot->numSubslots;i++)
    {
        newMilitarySlot->subslots[i] = militarySlot->subslots[i];
    }
    dbgAssert(i == militarySlot->numSubslots);
    for ( ;i<numSubslotsToAllocate;i++)
    {
        newMilitarySlot->subslots[i].ship = NULL;
    }

    // put ship in:
    newMilitarySlot->subslots[militarySlot->numSubslots].ship = ship;
    newMilitarySlot->highestSlotUsed = militarySlot->numSubslots;

    memFree(militarySlot);
    militaryParade->militarySlots[slot] = newMilitarySlot;
}

/*-----------------------------------------------------------------------------
    Name        : AddShipToMilitaryGroup
    Description : adds a ship to a military group command
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void AddShipToMilitaryGroup(ShipPtr ship,struct CommandToDo *militaryGroup)
{
    AddShipToGroup(ship,militaryGroup);

    dbgAssert(militaryGroup->ordertype.order == COMMAND_MILITARYPARADE);

    PutShipInMilitaryParade(ship,militaryGroup->militaryParade);
}

/*-----------------------------------------------------------------------------
    Name        : CreateMilitaryGroupAroundShip
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
struct CommandToDo *CreateMilitaryGroupAroundShip(struct CommandLayer *comlayer,ShipPtr ship,ShipPtr aroundShip)
{
    CommandToDo *newcommand;
    sdword i,j;
    MilitarySlot *militarySlot;
    SelectCommand *selection;
    MilitaryParadeCommand *militaryParade;

    newcommand = memAlloc(sizeof(CommandToDo),"ToDo",NonVolatile);
    InitCommandToDo(newcommand);

    selection = memAlloc(sizeofSelectCommand(1),"ToDoSelection",NonVolatile);
    selection->numShips = 1;
    selection->ShipPtr[0] = ship;

    newcommand->selection = selection;
    newcommand->ordertype.order = COMMAND_MILITARYPARADE;
    newcommand->ordertype.attributes = 0;

    newcommand->militaryParade = militaryParade = memAlloc(sizeof(MilitaryParadeCommand),"milparadecom",0);
    militaryParade->aroundShip = aroundShip;
    for (i=0;i<MAX_MILITARY_SLOTS;i++)
    {
        militarySlot = memAlloc(sizeofMilitarySlot(SUBSLOT_BATCH),"milslot",0);
        militarySlot->numSubslots = SUBSLOT_BATCH;
        militarySlot->highestSlotUsed = -1;
        for (j=0;j<SUBSLOT_BATCH;j++)
        {
            militarySlot->subslots[j].ship = NULL;
        }
        militaryParade->militarySlots[i] = militarySlot;
    }

    militaryParade->paradeType = -1;
    switch (aroundShip->shiptype)
    {
        case sMothership:
        case trFloatingCity:
        case trGhostship:
            militaryParade->paradeType = PARADE_SMOTHERSHIP;
            break;

        case bMothership:
        case dBeastMothership:
        case dKushanCarrier:
            militaryParade->paradeType = PARADE_BMOTHERSHIP;
            break;

        case sCarrier:
            militaryParade->paradeType = PARADE_SCARRIER;
            break;

        case bCarrier:
            militaryParade->paradeType = PARADE_BCARRIER;
            break;

        case dSlipgateSmall:
        case dSlipgateMedium:
        case dSlipgateLarge:
            militaryParade->paradeType = PARADE_SLIPGATE;
            break;

        case sProcessor:
        case dEcclesiamInfected:
            militaryParade->paradeType = PARADE_SPROCESSOR;
            break;

        case bProcessor:
            militaryParade->paradeType = PARADE_BPROCESSOR;
            break;

        case dRKO:
        case dPrisonShip:
        case dPrisonShipInfected:
            militaryParade->paradeType = PARADE_DPRISONSHIP;
            break;

        case kCarrier:
            militaryParade->paradeType = PARADE_R1CARRIER;
            break;

        case tCarrier:
        case trResearchStation:
            militaryParade->paradeType = PARADE_R2CARRIER;
            break;

        case p1Outpost:
        case p1Mothership:
            militaryParade->paradeType = PARADE_P1MOTHERSHIP;
            break;

        case bHeavyCruiser:
            militaryParade->paradeType = PARADE_BHEAVYCRUISER;
            break;

        default:
            break;
    }

    PutShipInMilitaryParade(ship,militaryParade);

    PrepareShipsForCommand(newcommand,TRUE);

    listAddNode(&comlayer->todolist,&newcommand->todonode,newcommand);

    return newcommand;
}

void lockFormation(CommandToDo *formationcommand,udword specialEffect)
{
    //copy leader coordinate system...
    //maybe...but probably don't want to relock formations over and over
    vector heading;
    //dbgAssert(formationcommand->formation.formationLocked == FALSE);
    formationcommand->formation.coordsys = formationcommand->selection->ShipPtr[0]->rotinfo.coordsys;
    if(specialEffect == 1)
    {
        //reverse Heading
        matGetVectFromMatrixCol3(heading,formationcommand->formation.coordsys);
        vecScalarMultiply(heading,heading,-1.0f);
        matPutVectIntoMatrixCol3(heading,formationcommand->formation.coordsys);
    }
    formationcommand->formation.formationLocked = TRUE;
}

void unlockFormation(CommandToDo *formationcommand)
{
    //copy leader coordinate system...
    //maybe...but probably don't want to unlock formations over and over
    //it shows we don't know what we're doing :)
    //dbgAssert(formationcommand->formation.formationLocked == TRUE);
    formationcommand->formation.formationLocked = FALSE;
}


real32 formationImmobileCloseDistance = 500.0f;
real32 formationMoveMinimumDistance = 10.0f;

real32 getMaxDistToDestination(Ship *ship, vector *destination)
{
    return formationGetMaxDistToDestination(ship, destination);
}

static sdword formSelectionAlreadyInFormation(CommandLayer *comlayer, SelectCommand *selectcom)
{
    CommandToDo *command = IsSelectionAlreadyDoingSomething(comlayer, selectcom);

    if (command != NULL && (command->ordertype.attributes & COMMAND_IS_FORMATION))
    {
        return (sdword)command->formation.formationtype;
    }

    return NO_FORMATION;
}

static void formSetTravelVelocity(CommandToDo *command)
{
    command->formation.travelvel = GetShipsTravelVel(command->selection, command->formation.percentmaxspeed);
}

static sdword formRemoveShipFromSelection(CommandToDo *formationtodo, Ship *removeship)
{
    return formationRemoveShipFromSelection(formationtodo, removeship);
}

static void formFillInSpecifics(CommandToDo *command)
{
    formationArrageCrazyOptimum(command);
}

static sdword formMajorityShipsAreDoing(SelectCommand *selection, sdword *a2, sdword *a3, sdword *a4, sdword *a5, GrowSelection *a6)
{
    sdword numShips = selection->numShips;
    sdword numMoving = 0;
    sdword numAttacking = 0;
    sdword i;

    for (i = 0; i < numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];
        CommandToDo *cmd = ship->command;
        if (cmd)
        {
            uword order = cmd->ordertype.order;
            if (order == COMMAND_ATTACK)
                numAttacking++;
            else if (order == COMMAND_MOVE)
                numMoving++;
        }
    }

    if (numAttacking && numMoving)
        return 0;

    if (numAttacking > numShips - numAttacking)
    {
        if (!AreAllShipsAttackCapable(selection))
            return 0;

        growSelectInit(a6);
        for (i = 0; i < numShips; i++)
        {
            Ship *ship = selection->ShipPtr[i];
            CommandToDo *cmd = ship->command;
            if (cmd && cmd->ordertype.order == COMMAND_ATTACK)
            {
                SelectCommand *attackSel = (SelectCommand *)cmd->attack;
                sdword j;
                for (j = 0; j < attackSel->numShips; j++)
                {
                    if (!ShipInSelection(selection, (Ship *)attackSel->ShipPtr[j]))
                        growSelectAddShipNoDuplication(a6, (Ship *)attackSel->ShipPtr[j]);
                }
            }
        }

        if (a6->selection->numShips == 0)
        {
            growSelectClose(a6);
            return 0;
        }
        return 2;
    }
    else if (numMoving > numShips - numMoving)
    {
        typedef struct { CommandToDo *cmd; sdword count; } CmdFreq;
        sdword numUnique = 0;
        sdword bestIdx = -1;
        sdword bestCount = -1;
        sdword j;
        CommandToDo *bestCmd;
        sdword numWaypoints;
        sdword curWaypoint;

        if (!AreShipsMobile(selection))
            return 0;

        CmdFreq *freqTable = memAlloc(8 * numMoving, "formMajFreq", MBF_SmallBlockHeap);

        for (i = 0; i < numShips; i++)
        {
            Ship *ship = selection->ShipPtr[i];
            CommandToDo *cmd = ship->command;
            if (cmd && cmd->ordertype.order == COMMAND_MOVE)
            {
                bool32 found = FALSE;
                for (j = 0; j < numUnique; j++)
                {
                    if (freqTable[j].cmd == cmd)
                    {
                        freqTable[j].count++;
                        found = TRUE;
                        break;
                    }
                }
                if (!found)
                {
                    freqTable[numUnique].cmd = cmd;
                    freqTable[numUnique].count = 1;
                    numUnique++;
                }
            }
        }

        for (i = 0; i < numUnique; i++)
        {
            if (freqTable[i].count > bestCount)
            {
                bestIdx = i;
                bestCount = freqTable[i].count;
            }
        }

        bestCmd = (bestIdx >= 0) ? freqTable[bestIdx].cmd : NULL;
        memFree(freqTable);

        if (!bestCmd)
            return 0;

        {
            sdword *cmdPtr = (sdword *)bestCmd;
            *a2 = cmdPtr[6];
            *a3 = cmdPtr[7];
            *a5 = cmdPtr[71];
            numWaypoints = cmdPtr[7];
            curWaypoint = cmdPtr[71];
            for (i = 0; i < numWaypoints; a4 += 3, i++)
            {
                sdword prevIdx = curWaypoint - 1;
                sdword nextIdx = curWaypoint + i;
                if (prevIdx < 0) prevIdx = numWaypoints;
                if (nextIdx >= numWaypoints) nextIdx -= numWaypoints;
                if (nextIdx != prevIdx)
                {
                    sdword *waypoint = &cmdPtr[3 * nextIdx + 8];
                    a4[0] = waypoint[0];
                    a4[1] = waypoint[1];
                    a4[2] = waypoint[2];
                }
                numWaypoints = cmdPtr[7];
            }
        }
        return 1;
    }

    return 0;
}

static sdword formGetShipSlot(Ship *ship, MilitaryParadeCommand *militaryParade)
{
    return GetShipSlot(ship, militaryParade);
}

static sdword formRemoveShipFromMilitaryParade(Ship *shiptoremove, MilitaryParadeCommand *militaryParade)
{
    sdword slot;
    sdword subslot;

    for (slot = 0; slot < MAX_MILITARY_SLOTS; slot++)
    {
        MilitarySlot *militarySlot = militaryParade->militarySlots[slot];

        for (subslot = 0; subslot <= militarySlot->highestSlotUsed; subslot++)
        {
            if (militarySlot->subslots[subslot].ship == shiptoremove)
            {
                militarySlot->subslots[subslot].ship = NULL;
                CalculateNewHighestUsedSlot(militarySlot);
                return militarySlot->highestSlotUsed;
            }
        }
    }

    return 0;
}

static void formSortSelectionByDistance(SelectCommand *selection)
{
    formationBiggestThenClosestOptimum(selection, DELTA_FORMATION);
}

void formationInit(CommandToDo *command, TypeOfFormation formationType, ShipPtr leader, bool32 resetLeader)
{
    command->formation.formationSpecificInfo = leader;
    command->formation.formationtype = formationType;
    command->formation.tacticalState = TightFormation;
    command->formation.formationLocked = FALSE;
    command->formation.tacticsUpdate = 0.0f;
    command->formation.flipselectionfixed = FALSE;
    command->formation.needFix = FALSE;
    command->formation.enders = FALSE;
    command->formation.percentmaxspeed = FORMATION_TRAVELVEL_MINSCALE;
    command->formation.travelvel = GetShipsTravelVel(command->selection, FORMATION_TRAVELVEL_MINSCALE);
    command->formation.flagTravelSlowestShip = FORMATION_TRAVEL_SLOWEST_SHIP;
    command->formation.doneInitialAttack = FALSE;
    command->formation.sortorder = 0;
    command->formation.pulse = FALSE;
    if (resetLeader)
    {
        command->formation.flipState = 0;
    }

    formFillInSpecifics(command);

    switch (formationType)
    {
        case DELTA_FORMATION:
        case BROAD_FORMATION:
        case DELTA3D_FORMATION:
        case CLAW_FORMATION:
        case WALL_FORMATION:
        case SPHERE_FORMATION:
        case CUSTOM_FORMATION:
        case FORCEFIELD_WALL_FORMATION:
        case FORCEFIELD_SPHERE_FORMATION:
            dockCommandSetMode(command, 0);
            break;

        default:
            break;
    }
}

void formationShipMoveTo(Ship *ship, Ship *leader, vector *desiredpos, vector *heading)
{
    ShipStaticInfo *shipstatic = ship->staticinfo;
    real32 maxDistToDestination = getMaxDistToDestination(ship, desiredpos);
    udword useFlags = AISHIP_PointInDirectionFlying |
                      AISHIP_FastAsPossible |
                      AISHIP_DontFlyToObscuredPoints |
                      AISHIP_ReturnImmedIfPointObscured |
                      AISHIP_IgnoreFormationObscuredPoints;

    if (shipstatic->staticheader.immobile ||
        shipstatic->shiptype == kCloakGenerator ||
        shipstatic->shiptype == kGravWellGenerator ||
        shipstatic->shiptype == tCloakGenerator ||
        shipstatic->shiptype == tGravWellGenerator)
    {
        if (maxDistToDestination <= 500.0f)
        {
            aitrackHeadingWithBank(ship, heading, FLYSHIP_HEADINGACCURACY, shipstatic->sinbank);
            useFlags = AISHIP_FastAsPossible |
                       AISHIP_DontFlyToObscuredPoints |
                       AISHIP_ReturnImmedIfPointObscured |
                       AISHIP_IgnoreFormationObscuredPoints;
        }
    }
    else if (maxDistToDestination <= 100.0f)
    {
        aitrackHeadingWithBank(ship, heading, FLYSHIP_HEADINGACCURACY, shipstatic->sinbank);
        useFlags = AISHIP_FastAsPossible |
                   AISHIP_DontFlyToObscuredPoints |
                   AISHIP_ReturnImmedIfPointObscured |
                   AISHIP_IgnoreFormationObscuredPoints;
    }

    if (maxDistToDestination >= 10.0f)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship, desiredpos, useFlags, 0.0f, &leader->posinfo.velocity);
    }
}

void formationShipMoveToSteady(Ship *ship, Ship *leader, vector *desiredpos, vector *heading, bool32 useAvoidance)
{
    ShipStaticInfo *shipstatic = ship->staticinfo;
    real32 maxDistToDestination = getMaxDistToDestination(ship, desiredpos);
    udword useFlags = AISHIP_FastAsPossible |
                      AISHIP_DontFlyToObscuredPoints |
                      AISHIP_ReturnImmedIfPointObscured |
                      AISHIP_IgnoreFormationObscuredPoints |
                      (useAvoidance ? AISHIP_PointInDirectionFlying : 0);

    if (shipstatic->staticheader.immobile ||
        shipstatic->shiptype == kCloakGenerator ||
        shipstatic->shiptype == kGravWellGenerator ||
        shipstatic->shiptype == tCloakGenerator ||
        shipstatic->shiptype == tGravWellGenerator)
    {
        if (maxDistToDestination <= formationImmobileCloseDistance)
        {
            aitrackHeadingWithBank(ship, heading, FLYSHIP_HEADINGACCURACY, shipstatic->sinbank);
            useFlags = AISHIP_FastAsPossible |
                       AISHIP_DontFlyToObscuredPoints |
                       AISHIP_ReturnImmedIfPointObscured |
                       AISHIP_IgnoreFormationObscuredPoints;
        }
    }
    else if (maxDistToDestination <= 100.0f)
    {
        aitrackHeadingWithBank(ship, heading, FLYSHIP_HEADINGACCURACY, shipstatic->sinbank);
        useFlags = AISHIP_FastAsPossible |
                   AISHIP_DontFlyToObscuredPoints |
                   AISHIP_ReturnImmedIfPointObscured |
                   AISHIP_IgnoreFormationObscuredPoints;
    }

    if (maxDistToDestination >= formationMoveMinimumDistance)
    {
        aishipFlyToPointAvoidingObjsWithVel(ship, desiredpos, useFlags, 0.0f, &leader->posinfo.velocity);
    }
}
