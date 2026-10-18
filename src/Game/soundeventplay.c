/*=============================================================================
    Name    : SoundEventPlay.c
    Purpose : Sound Event Play functions

    Created 5/8/98 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "SoundEventPrivate.h"
#include "soundevent.h"
#include "gun.h"
#include "matrix.h"
#include "NIS.h"

#define OBJ_None    -1
#define OBJ_Gun     -2

#define EFFECT_SIZE	500.0f

extern real32 volSFX;
typedef struct
{
	sdword max;
	sdword handle[2];
} effectHandles;

effectHandles effectHandle[NUM_HIT_EVENTS];

enum
{
    UI_SpecialShipRedirect = 0x1C8,
    Ship_SpecialUiRedirect = 0x821,
    Exp_ExtraLarge = 0x1007,
    Exp_ExtraFalloff = 0x100D,
};

#define NOMAD_MOON_NIS_EQ_DISTANCE_SCALE 0.125f

extern ShipRace whichRaceSelected;


static sdword SEhitEventShipClass(sdword event)
{
	if (event == Hit_DefenseFieldDeflect)
	{
		return WeaponHit_DefenseFieldDeflect;
	}

	return (event - Hit_Flag) + HIT_OFFSET;
}

void SEinitHandles(void)
{
	sdword i;

	for (i = 0; i < NUM_HIT_EVENTS; i++)
	{
		if (i == (Hit_Nick & SFX_Event_Mask) || i == (Hit_Ricochet & SFX_Event_Mask))
		{
			effectHandle[i].max = 1;
		}
		else
		{
			effectHandle[i].max = 2;
		}
		effectHandle[i].handle[0] = SOUND_NOTINITED;
		effectHandle[i].handle[1] = SOUND_NOTINITED;
	}
}

/*-----------------------------------------------------------------------------
    Name        :   soundEventPlay
    Description :
    Inputs      :   object - pointer to a spaceobj, ie a Ship or an Effect
                    event - the sound event to play
                    gun - pointer to a gun if this is a gun event
    Outputs     :
    Return      :   handle to the sound, this can be used to turn off the sound later
----------------------------------------------------------------------------*/
sdword soundEventPlay(void *object, sdword event, Gun *gun)
{
	sdword handle = SOUND_NOTINITED;
#if SOUND
	Ship *ship;
	GunSoundType activeSoundType;
	sword vol;
	sword pan;
	sdword i;
	real32 dist;
	sdword priority = SOUND_PRIORITY_MIN;
	SpaceObj *spaceobject;
	Effect *effect;
	ObjType objtype;
	real32 tempEQ[SOUND_EQ_SIZE];
	sdword eventflag;
	ShipClass shipclass;
	sdword patch;
	if (!enableSFX)
	{
	    return (SOUND_OK);
    }

    if (object != NULL)
    {
        spaceobject = (SpaceObj *)object;
        if (!univSpaceObjInRenderList(spaceobject))
        {
            return (handle);        // only do sound events for viewable objects
        }

        if (gun != NULL)
        {
            objtype = OBJ_Gun;
        }
        else
        {
            objtype = spaceobject->objtype;
        }

        dist = fsqrt(spaceobject->cameraDistanceSquared);
    }
    else
    {
        objtype = OBJ_None;
    }

    if (event & SFX_Flag_Mask)
    {
        eventflag = event & SFX_Flag_Mask;

        switch (eventflag)
        {
            case UI_Flag:
                if (event <= UI_ClickCancel)
                {
                    soundstop(ambienthandle, SOUND_FADE_STOPTIME);
                    patch = GetPatch(UIEventsLUT, whichRaceSelected, event);
                    ambienthandle = splayFPRVL(UIBank, UIEventsLUT->lookup[patch],
                                               NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_PRIORITY_STREAM,
                                               (sword)(SOUND_VOL_MAX * volSFX), FALSE, TRUE, FALSE);
                    return handle;
                }

                vol = (sword)(SOUND_VOL_MAX * volSFX);
                if (event == UI_SpecialShipRedirect)
                {
                    patch = GetPatch(ShipEventsLUT, 0, Ship_SpecialUiRedirect);
                    return splayFPRVL(ShipBank, ShipEventsLUT->lookup[patch],
                                      NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_PRIORITY_MAX,
                                      vol, FALSE, FALSE, FALSE);
                }

                patch = GetPatch(UIEventsLUT, whichRaceSelected, event);
                return splayFPRVL(UIBank, UIEventsLUT->lookup[patch],
                                  NULL, (real32)SOUND_DEFAULT, SOUND_PAN_CENTER, SOUND_PRIORITY_MAX,
                                  vol, FALSE, FALSE, FALSE);

            case Gun_Flag:
                if (smSensorsActive || !mrRenderMainScreen)
                {
                    break;
                }

                ship = (Ship *)object;
                dbgAssert(gun != NULL);
                dbgAssert(gun->gunstatic != NULL);
                activeSoundType = (GunSoundType)SEgetShipClass((Ship *)gun);
                if (activeSoundType >= GunEventsLUT->numobjects)
                {
                    break;
                }

                if (event == Gun_WeaponFireLooped)
                {
                    if (activeSoundType != GS_LargeEnergyCannon &&
                        activeSoundType != GS_MediumEnergyCannon &&
                        activeSoundType != GS_SmallEnergyCannon)
                    {
                        sdword patchEvent = Gun_WeaponFireLooped;
                        sdword patchObject = activeSoundType;

                        if (!SEinrange(activeSoundType + GUNSHOT_OFFSET, dist) ||
                            ship->soundevent.burstHandle != SOUND_NOTINITED ||
                            !soundover(SOUND_NOTINITED))
                        {
                            break;
                        }

                        ship->soundevent.burstfiring = TRUE;
                        vol = (sword)(SEequalize(activeSoundType + GUNSHOT_OFFSET, dist, tempEQ)
                                      * (1.0f - (ship->soundevent.coverage * 0.5f)));
                        pan = getPanAngle(ship->posinfo.position,
                                          ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize,
                                          dist);

                        if (ship->shiptype == sMultiBeamFrigate || ship->shiptype == bMultiBeamFrigate)
                        {
                            patchEvent = Gun_WeaponFireMultiBeamLooped;
                            patchObject = 0;
                        }
                        else if (ship->shiptype == dAcolyte)
                        {
                            patchEvent = Gun_WeaponFireAcolyteLooped;
                            patchObject = 0;
                        }
                        if (patchObject == 0 && patchEvent != Gun_WeaponFireLooped)
                        {
                            patch = GetPatch(GunSpecEventsLUT, patchObject, patchEvent);
                        }
                        else
                        {
                            patch = GetPatch(GunEventsLUT, patchObject, patchEvent);
                        }

                        if (patchObject == 0 && patchEvent != Gun_WeaponFireLooped)
                        {
                            handle = splayFPRVL(GunBank, GunSpecEventsLUT->lookup[patch],
                                                tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MAX,
                                                vol, FALSE, TRUE, FALSE);
                        }
                        else
                        {
                            handle = splayFPRVL(GunBank, GunEventsLUT->lookup[patch],
                                                tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MAX,
                                                vol, FALSE, TRUE, FALSE);
                        }
                        ship->soundevent.burstHandle = handle;
                        ship->burstGunIndex = gun->gunstatic->gunindex;
                        return handle;
                    }
                }

                if (event == Gun_WeaponFireLooped || event == Gun_WeaponShot)
                {
                    if (activeSoundType == GS_BeastWeapon)
                    {
                        if (!SEinrange(GS_MissleLauncher, dist))
                        {
                            break;
                        }

                        vol = SEequalize(GS_MissleLauncher, dist, tempEQ);
                    }
                    else
                    {
                        if (!SEinrange(activeSoundType + GUNSHOT_OFFSET, dist) ||
                            !soundover(ship->soundevent.gunHandle))
                        {
                            break;
                        }

                        vol = (sword)(SEequalize(activeSoundType + GUNSHOT_OFFSET, dist, tempEQ)
                                      * (1.0f - ship->soundevent.coverage));
                    }

                    pan = getPanAngle(ship->posinfo.position,
                                      ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize,
                                      dist);
                    {
                        sdword patchEvent = Gun_WeaponShot;
                        sdword patchObject = activeSoundType;

                    if (ship->shiptype == sDreadnought ||
                        ship->shiptype == sDestroyer ||
                        ship->shiptype == tMissileDestroyer)
                    {
                        patchEvent = Gun_WeaponShotCapital;
                        patchObject = 0;
                    }
                    if (patchObject == 0 && patchEvent == Gun_WeaponShotCapital)
                    {
                        patch = GetPatch(GunSpecEventsLUT, patchObject, patchEvent);
                    }
                    else
                    {
                        patch = GetPatch(GunEventsLUT, patchObject, patchEvent);
                    }

                    if (patchObject == 0 && patchEvent == Gun_WeaponShotCapital)
                    {
                        handle = splayFPRVL(GunBank, GunSpecEventsLUT->lookup[patch],
                                            tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_HIGH,
                                            vol, FALSE, TRUE, FALSE);
                    }
                    else
                    {
                        handle = splayFPRVL(GunBank, GunEventsLUT->lookup[patch],
                                            tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_HIGH,
                                            vol, FALSE, TRUE, FALSE);
                    }
                    }
                    if (activeSoundType != GS_BeastWeapon)
                    {
                        ship->soundevent.gunHandle = handle;
                    }
                    return handle;
                }

                if (!SEinrange(activeSoundType + GUNMOVE_OFFSET, dist) ||
                    !soundover(ship->soundevent.gunHandle))
                {
                    break;
                }

                vol = (sword)(SEequalize(activeSoundType + GUNMOVE_OFFSET, dist, tempEQ)
                              * (1.0f - ship->soundevent.coverage));
                pan = getPanAngle(ship->posinfo.position,
                                  ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize,
                                  dist);
                patch = GetPatch(GunEventsLUT, activeSoundType, event);
                handle = splayFPRVL(GunBank, GunEventsLUT->lookup[patch],
                                    tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MIN,
                                    vol, FALSE, TRUE, FALSE);
                return handle;

            case ShipCmn_Flag:
                if (smSensorsActive || !mrRenderMainScreen)
                {
                    break;
                }
                if (objtype != OBJ_ShipType)
                {
                    break;
                }

                ship = (Ship *)object;
                shipclass = ship->staticinfo->shipclass;
                if (!SEinrange(shipclass, dist))
                {
                    break;
                }

                if (event == ShipCmn_Engine)
                {
                    if (ship->soundevent.engineHandle != SOUND_NOTINITED)
                    {
                        break;
                    }

                    ship->soundevent.engineState = SOUND_STARTING;
                    if (shipclass == CLASS_Corvette)
                    {
                        priority = SOUND_PRIORITY_MAX - 1;
                    }
                    else if (shipclass == CLASS_Fighter)
                    {
                        priority = SOUND_PRIORITY_MAX - 2;
                    }
                    else
                    {
                        priority = SOUND_PRIORITY_MAX;
                    }

                    vol = SEequalize(shipclass, dist, tempEQ);
                    if (shipclass == CLASS_Fighter || shipclass == CLASS_Corvette)
                    {
                        vol = (sword)(vol * (1.0f - ship->soundevent.coverage));
                    }
                }
                else
                {
                    vol = (sword)(SEequalize(shipclass + AMBIENT_OFFSET, dist, tempEQ)
                                  * (1.0f - ship->soundevent.coverage));
                }

                pan = getPanAngle(ship->posinfo.position,
                                  ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize,
                                  dist);
                if (ship->shiptype < (udword)ShipCmnEventsLUT->numobjects)
                {
                    patch = GetPatch(ShipCmnEventsLUT, ship->shiptype, event);
                    handle = splayFPRVL(ShipBank, ShipCmnEventsLUT->lookup[patch],
                                        tempEQ, (real32)SOUND_DEFAULT, pan, priority,
                                        vol, FALSE, TRUE, FALSE);
                }

                if (event == ShipCmn_Engine)
                {
                    ship->soundevent.engineHandle = handle;
                    if (handle == SOUND_NOTINITED)
                    {
                        ship->soundevent.engineState = SOUND_NOTINITED;
                    }
                }
                return handle;

            case Ship_Flag:
                if (smSensorsActive || !mrRenderMainScreen || objtype != OBJ_ShipType)
                {
                    break;
                }

                ship = (Ship *)object;
                vol = SEequalize(ship->staticinfo->shipclass, dist, tempEQ);
                if (nisIsRunning_ && ship->shiptype == dNomadMoon)
                {
                    vol = SEequalize(ship->staticinfo->shipclass,
                                     dist * NOMAD_MOON_NIS_EQ_DISTANCE_SCALE,
                                     tempEQ);
                }

                pan = getPanAngle(ship->posinfo.position,
                                  ship->staticinfo->staticheader.staticCollInfo.approxcollspheresize,
                                  dist);
                patch = GetPatch(ShipEventsLUT, 0, event);
                return splayFPRVL(ShipBank, ShipEventsLUT->lookup[patch],
                                  tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_NORMAL,
                                  vol, FALSE, TRUE, FALSE);

            case Exp_Flag:
                if (smSensorsActive || !mrRenderMainScreen)
                {
                    break;
                }

                effect = (Effect *)object;
                dist = (real32)fsqrt(effect->cameraDistanceSquared);
                shipclass = 82;
                if (event <= 0x100F)
                {
                    shipclass = event - 4028;
                }

                if (!SEinrange(shipclass, dist))
                {
                    break;
                }

                vol = SEequalize(shipclass, dist, tempEQ);
                pan = getPanAngle(effect->posinfo.position, EFFECT_SIZE, dist);
                patch = GetPatch(SpecExpEventsLUT, 0, event);
                handle = splayFPRVL(SpecialEffectBank, SpecExpEventsLUT->lookup[patch],
                                    tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MAX + 1,
                                    vol, FALSE, TRUE, FALSE);

                if (event == Exp_DestDamageLarge ||
                    event == Exp_DestProjectileLarge ||
                    event == Exp_ExtraLarge)
                {
                    if (!SEinrange(81, dist))
                    {
                        return handle;
                    }

                    vol = SEequalize(81, dist, tempEQ);
                    if (vol > 100)
                    {
                        patch = GetPatch(SpecExpEventsLUT, 0, Exp_ExtraFalloff);
                        return splayFPRVL(SpecialEffectBank, SpecExpEventsLUT->lookup[patch],
                                          tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_MAX + 1,
                                          vol, FALSE, TRUE, FALSE);
                    }
                }
                return handle;

            case Hit_Flag:
                if (smSensorsActive || !mrRenderMainScreen)
                {
                    break;
                }

                effect = (Effect *)object;
                shipclass = SEhitEventShipClass(event);
                if (!SEinrangeSqr(shipclass, effect->cameraDistanceSquared))
                {
                    break;
                }

                if (effectHandle[event & SFX_Event_Mask].max <= 0)
                {
                    break;
                }

                for (i = 0; i < effectHandle[event & SFX_Event_Mask].max; i++)
                {
                    if (soundover(effectHandle[event & SFX_Event_Mask].handle[i]))
                    {
                        dist = (real32)fsqrt(effect->cameraDistanceSquared);
                        vol = SEequalize(shipclass, dist, tempEQ);
                        pan = getPanAngle(effect->posinfo.position, EFFECT_SIZE, dist);
                        patch = GetPatch(SpecHitEventsLUT, 0, event);
                        handle = splayFPRVL(SpecialEffectBank, SpecHitEventsLUT->lookup[patch],
                                            tempEQ, (real32)SOUND_DEFAULT, pan, SOUND_PRIORITY_HIGH,
                                            vol, FALSE, TRUE, FALSE);
                        effectHandle[event & SFX_Event_Mask].handle[i] = handle;
                        return handle;
                    }
                }
                return handle;

            default:
                dbgMessagef("\nsoundEventPlaySound: %d event", event);
                break;
        }
    }
#endif

    return (handle);
}


/*-----------------------------------------------------------------------------
    Name        :   soundEffect
    Description :   This is just a wrapper for soundEventPlay since the ETG
                    doesn't like it when I use a #define for soundEffect.
    Inputs      :   effect - pointer to an Effect
                    event - the sound event to play
    Outputs     :
    Return      :   handle to the sound, this can be used to turn off the sound later
----------------------------------------------------------------------------*/
sdword soundEffect(void *effect, sdword event)
{
    return (soundEventPlay(effect, event, NULL));
}


/*-----------------------------------------------------------------------------
    Name        :   soundEffectType
    Description :
    Inputs      :   effect - pointer to an Effect
                    event - the sound event to play
                    objecttype - the type of object playing the event (gun sound type)
    Outputs     :
    Return      :   handle to the sound, this can be used to turn off the sound later
----------------------------------------------------------------------------*/
sdword soundEffectType(Effect *effect, sdword event, sdword objecttype)
{
    sdword handle = SOUND_NOTINITED;
    sword vol;
    sword pan;
    real32 dist;
    sdword priority = SOUND_PRIORITY_MIN;
    real32 tempEQ[SOUND_EQ_SIZE];
    sdword eventflag;

    if (!enableSFX)
    {
        return (SOUND_OK);
    }

    if (event & SFX_Flag_Mask)
    {
        eventflag = event & SFX_Flag_Mask;

        switch (eventflag)
        {
            case Hit_Flag:
				if (smSensorsActive || !mrRenderMainScreen)
				{
					// don't want to play this sound from the manager screens
					break;
				}
                if (SEinrangeSqr(SEhitEventShipClass(event), effect->cameraDistanceSquared))
                {
                    dist = (real32)fsqrt(effect->cameraDistanceSquared);
                    vol = SEequalize(SEhitEventShipClass(event), dist, tempEQ);
                    pan = getPanAngle(effect->posinfo.position, EFFECT_SIZE, dist);

                    handle = splayEPRV(SpecialEffectBank, SpecHitEventsLUT->lookup[GetPatch(SpecHitEventsLUT, objecttype, event)], tempEQ, pan, SOUND_PRIORITY_NORMAL, vol);
                }
                break;

            default:
                dbgMessagef("\nShould use soundEffect for %d event instead.", event);
                break;
        }
    }

    return (handle);
}

// Cat3: soundEventPlayAsteroidBreak (0x5215D0)
sdword soundEventPlayAsteroidBreak(SpaceObj *asteroid)
{
#if SOUND
    Derelict *derelict = (Derelict *)asteroid;
    real32 dist;
    sword vol;
    sword pan;
    real32 tempEQ[SOUND_EQ_SIZE];
    sdword Patch;

    if (smSensorsActive || !mrRenderMainScreen)
        return 0;

    if (!SEinrangeSqr(68, derelict->cameraDistanceSquared))
        return 0;

    dist = (real32)fmathSqrtDouble(derelict->cameraDistanceSquared);
    vol = SEequalize(68, dist, tempEQ);
    pan = getPanAngle(derelict->posinfo.position, EFFECT_SIZE, dist);
    Patch = GetPatch(ShipEventsLUT, 0, 0xC);
    return splayFPRVL(ShipBank, ShipEventsLUT->lookup[Patch], tempEQ, -1.0f, pan, 50, vol, 0, 1, 0);
#else
    return 0;
#endif
}
