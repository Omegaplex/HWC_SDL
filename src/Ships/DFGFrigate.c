/*=============================================================================
    Name    : DFGFrigate.c
    Purpose : Specifics for the DFGFrigate

    Created 6/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/
#include <stdlib.h>
#include <math.h>
#include "glinc.h"
#include "types.h"
#include "debug.h"
#include "spaceobj.h"
#include "DFGFrigate.h"
#include "CommandLayer.h"
#include "statscript.h"
#include "gun.h"
#include "attack.h"
#include "DefaultShip.h"
#include "vector.h"
#include "fastmath.h"
#include "universe.h"
#include "univupdate.h"
#include "attack.h"
#include "randy.h"

DFGFrigateStatics DFGFrigateStatic;

DFGFrigateStatics DFGFrigateStaticRace1;
DFGFrigateStatics DFGFrigateStaticRace2;

#define DFG_FIELD_RUNTIME_FLAG_OFFSET        0x97C
#define DFG_FIELD_RUNTIME_EXPIRE_OFFSET      0x980

// Retail marks ships in force-field formations through two shared tail-overlay words.
// Their wider ownership is still unresolved, so keep the raw offsets local here.
static udword *dfgFieldRuntimeFlag(Ship *ship)
{
    return (udword *)((ubyte *)ship + DFG_FIELD_RUNTIME_FLAG_OFFSET);
}

static real32 *dfgFieldRuntimeExpireTime(Ship *ship)
{
    return (real32 *)((ubyte *)ship + DFG_FIELD_RUNTIME_EXPIRE_OFFSET);
}

scriptStructEntry DFGStaticScriptTable[] =
{
    { "fieldRadius",    scriptSetReal32CB, (udword) &(DFGFrigateStatic.DFGFrigateFieldRadius), (udword) &(DFGFrigateStatic) },
    { "BulletDamageMultiplier",    scriptSetReal32CB, (udword) &(DFGFrigateStatic.BulletDamageMultiplier), (udword) &(DFGFrigateStatic) },
    { "BulletLifeExtension",    scriptSetReal32CB, (udword) &(DFGFrigateStatic.BulletLifeExtension), (udword) &(DFGFrigateStatic) },
    { "percentageThatGetThrough",    scriptSetUdwordCB, (udword) &(DFGFrigateStatic.percentageThatGetThrough), (udword) &(DFGFrigateStatic) },
    { "angleVariance",    scriptSetUdwordCB, (udword) &(DFGFrigateStatic.angleVariance), (udword) &(DFGFrigateStatic) },

    { NULL,NULL,0,0 }
};


void DFGFrigateStaticInit(char *directory,char *filename,struct ShipStaticInfo *statinfo)
{
    DFGFrigateStatics *dfgstat = (statinfo->shiprace != RACE_Sect) ? &DFGFrigateStaticRace1 : &DFGFrigateStaticRace2;

    statinfo->custstatinfo = dfgstat;

    scriptSetStruct(directory,filename,DFGStaticScriptTable,(ubyte *)dfgstat);
    dfgstat->DFGFrigateFieldRadiusSqr = dfgstat->DFGFrigateFieldRadius*dfgstat->DFGFrigateFieldRadius;
}

void DFGFInit(Ship *ship)
{
    DFGFrigateSpec *spec = (DFGFrigateSpec *) ship->ShipSpecifics;
}
void univDFGFieldEffect(Ship *ship, Bullet *bullet,real32 totaltimeelapsed)
{
    vector  dir1,shiptobullet;
    vector  newbulletdir;
    sdword  randomnum;
    real32 bulletspeed;
    real32 theta;
    matrix rotmatrix,newmatrix,tmpmatrix;
    DFGFrigateSpec *spec = (DFGFrigateSpec *) ship->ShipSpecifics;
    sdword LOD;
    etglod *etgLOD;
    etgeffectstatic *stat;
    udword intDamage;
    real32 floatDamage;

    DFGFrigateStatics *DFGFstatics;
    DFGFstatics = (DFGFrigateStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    if(bullet->playerowner == ship->playerowner)
    {
        //don't affect players bullets
        return;
    }
    if(bullet->SpecialEffectFlag & 0x0001)
    {
        //bullet has been effected by field so don't touch it ever again!
        return;
    }

    bullet->SpecialEffectFlag |= 0x0001;

    randomnum = randombetween(0,100);
    if(randomnum < DFGFstatics->percentageThatGetThrough)
    {
        //bullet is fortunate enough to not be deflected
        return;
    }

    bulletspeed = fsqrt(vecMagnitudeSquared(bullet->posinfo.velocity));

    //coorsys for deflection effect
    vecSub(shiptobullet,bullet->posinfo.position,ship->posinfo.position);

    vecNormalize(&shiptobullet);
    matCreateCoordSysFromHeading(&tmpmatrix,&shiptobullet);

    //create an effect where the collision occurred
//    etgLOD = etgGunEventTable[ship->staticinfo->shiprace][bullet->soundType][EGT_BulletDeflected];//get pointer to bullet effect
    etgLOD = etgSpecialPurposeEffectTable[EGT_BULLET_DEFLECT];
    if (etgLOD != NULL)
    {
        LOD = ship->currentLOD;
        if (LOD >= etgLOD->nLevels)
        {
            stat = NULL;
        }
        else
        {
            stat = etgLOD->level[LOD];
        }
    }
    else
    {
        stat = NULL;
    }
    if (univSpaceObjInRenderList((SpaceObj *)ship))
    {
#if ETG_DISABLEABLE
        if (stat != NULL && etgHitEffectsEnabled && etgEffectsEnabled && !etgFrequencyExceeded(stat))
#else
        if (stat != NULL && etgHitEffectsEnabled && !etgFrequencyExceeded(stat))
#endif
        {                                                   //if there is a deflection effect
//            matCreateCoordSysFromHeading(&newCoordsys,&shiptobullet);
            floatDamage = (real32)bullet->damage;
            if (RGLtype == SWtype)
            {                                               //smaller deflection effects in software
                floatDamage *= etgSoftwareScalarHit;
            }
            floatDamage *= ship->magnitudeSquared;
            intDamage = TreatAsUdword(floatDamage);
            etgEffectCreate(stat, ship, &bullet->posinfo.position, NULL, &tmpmatrix, ship->magnitudeSquared, EAF_Velocity | EAF_NLips, 1, intDamage);
        }
    }

    theta = DEG_TO_RAD(randombetween(0,DFGFstatics->angleVariance));
    if(randombetween(1,2) & 1)
    {
        theta *= -1.0f;             //could optimize by using a bit toggle
    }

    matMakeRotAboutZ(&rotmatrix,(real32) cos(theta),(real32) sin(theta));
    matMultiplyMatByMat(&newmatrix,&rotmatrix,&tmpmatrix);

    matGetVectFromMatrixCol1(dir1,newmatrix);

    if(randombetween(1,2) & 1)
    {
        vecScalarMultiply(dir1,dir1,-1.0f);
    }

    newbulletdir = dir1;

    vecNormalize(&newbulletdir);     //maybe don't need

    matCreateCoordSysFromHeading(&bullet->rotinfo.coordsys,&newbulletdir);
    vecScalarMultiply(bullet->posinfo.velocity,newbulletdir,bulletspeed);
    vecScalarMultiply(bullet->lengthvec,newbulletdir,bullet->lengthmag);
    bullet->bulletheading = newbulletdir;

    bullet->damage = (sword) (bullet->damage*DFGFstatics->BulletDamageMultiplier);
    bullet->totallifetime += DFGFstatics->BulletLifeExtension;
}

void DFGFrigateAttack(Ship *ship,SpaceObjRotImpTarg *target,real32 maxdist)
{
    DFGFrigateStatics *DFGFstatics = (DFGFrigateStatics *) ((ShipStaticInfo *)(ship->staticinfo))->custstatinfo;

    attackStraightForward(ship,target,DFGFstatics->DFGFrigateFieldRadius,1000.0f);
}

bool32 DFGFrigateAttackCommand(Ship *ship, SpaceObjRotImpTarg *target, real32 maxdist)
{
    DFGFrigateStatics *DFGFstatics = (DFGFrigateStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    attackStraightForward(ship, target, DFGFstatics->DFGFrigateFieldRadius, 1000.0f);
    return TRUE;
}

void DFGFrigateDisruptForcefieldFormations(Ship *ship)
{
    Node *node;
    DFGFrigateStatics *DFGFstatics = (DFGFrigateStatics *)((ShipStaticInfo *)ship->staticinfo)->custstatinfo;

    if ((universe.univUpdateCounter & 0xC) == 0)
    {
        return;
    }

    for (node = universe.mainCommandLayer.todolist.head; node != NULL; node = node->next)
    {
        CommandToDo *command = (CommandToDo *)listGetStructOfNode(node);
        SelectCommand *selection;
        Ship *leadShip;
        vector separation;
        sdword i;

        if ((command->ordertype.attributes & COMMAND_IS_FORMATION) == 0)
        {
            continue;
        }
        if (command->formation.formationtype != FORCEFIELD_WALL_FORMATION &&
            command->formation.formationtype != FORCEFIELD_SPHERE_FORMATION)
        {
            continue;
        }

        selection = command->selection;
        if (selection->numShips <= 0)
        {
            continue;
        }

        leadShip = selection->ShipPtr[0];
        if (leadShip->playerowner == ship->playerowner)
        {
            continue;
        }

        vecSub(separation, leadShip->posinfo.position, ship->posinfo.position);
        if (vecMagnitudeSquared(separation) >= DFGFstatics->DFGFrigateFieldRadiusSqr)
        {
            continue;
        }

        for (i = 0; i < selection->numShips; i++)
        {
            Ship *formationShip = selection->ShipPtr[i];

            *dfgFieldRuntimeFlag(formationShip) = TRUE;
            *dfgFieldRuntimeExpireTime(formationShip) = universe.totaltimeelapsed + 2.0f;
        }
    }
}


CustShipHeader DFGFrigateHeader =
{
    sizeof(DFGFrigateSpec),
    DFGFrigateStaticInit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    DFGFrigateDisruptForcefieldFormations,
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
    DFGFrigateAttackCommand,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
