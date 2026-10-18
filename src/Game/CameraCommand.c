/*=============================================================================
    Name    : CameraCommand.c
    Purpose : Handles Camera commands such as focus and cancel focus, by
              keeping a "camera stack"

    Created 7/3/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include <math.h>
#include "glinc.h"
#include "types.h"
#include "fastmath.h"
#include "debug.h"
#include "memory.h"
#include "linkedlist.h"
#include "camera.h"
#include "utility.h"
#include "prim2d.h"
#include "statscript.h"
#include "universe.h"
#include "Ship.h"
#include "shipselect.h"
#include "select.h"
#include "cameracommand.h"
#include "proximitysensor.h"
#include "sensors.h"
#include "main.h"
#include "alliance.h"
#include "tutor.h"

#include "singlePlayer.h"
#include "task.h"
#include "Randy.h"
#include "render.h"
#include "UnivUpdate.h"

#ifdef gshaw
#define DEBUG_CAMERACOMMAND     0
#else
#define DEBUG_CAMERACOMMAND     0
#endif

extern udword utyNFrameTicks;
void telldir(void);
extern sdword nisWideScreenActive;
extern ubyte byte_A2DCD4;
sdword univIsObjFriendly(SpaceObj *obj, Player *player);
sdword fogObjectVisibleToPlayer(Player *player, SpaceObj *obj, sdword treatMultiplayerAsVisible);

/*=============================================================================
    Defines:
=============================================================================*/

#define TOLERANCE   30

#define FOCUS_OFF_SCREEN    1
#define FOCUS_NEAR_BORDER   2
#define FOCUS_NEAR_BIGBORDER    4
#define FOCUS_NO_SHIPS      8

/*=============================================================================
    Tweakables:
=============================================================================*/

udword MAX_CAMERA_STACKS = 20;

real32 CAMERA_DEFAULT_DISTANCE =   4000.0f;
real32 CAMERA_FAR_DISTANCE     =   6000.0f;
real32 CAMERA_FOCUSFAR_DISTANCE =  6000.0f;

real32 CAMERA_MOTHERSHIPFOCUS_DISTANCE = 6000.0f;
real32 CAMERA_MOTHERSHIPFOCUS_ANGLE = PI;
real32 CAMERA_MOTHERSHIPFOCUS_DECLINATION = -0.5f;

real32 GO_CAMERA_DELAY_TIME = 5.0f;

sdword CAMERA_MAX_ZOOM_OUT_RATE =   40;
sdword CAMERA_SCREEN_BORDER     =   25;
sdword CAMERA_NEAR_BORDER       =   50;
sdword CAMERA_NEAR_BIGBORDER    = 150;

real32 CLOSE_ON_ANGLE_SPEED = 10.0f;

real32 CAMERA_ELASTIC_DISTANCE = 10000.0f;

real32 CAMERA_ELASTIC_PULL_FACTOR = 1000.0f;

real32 CAMERA_OUTBY_SCALE_ZOOM = 25.0f;

real32 CAMERA_MIN_CLOSEFAST_RATE = 1.0f;

real32 CAM_CUBIC_EVAL_TWEAK = 1.6f;
real32 CAM_CUBIC_EVAL_ANGLE_TWEAK = 2.0f;

//Falko: tweak camTrackDistSpeed[0] and camTrackPosSpeed - larger denominator
real32 camTrackAngSpeed[2] = {1.0f/20.0f, 1.0f/10.0f};
real32 camTrackDistSpeed[2] = {1.0f/66.6666f, 1.0f/14.0f};
real32 camTrackDistBase[2] = {10.0f, 10.0f};
real32 camTrackDistThreshold = 1.0f/50.0f;
real32 camTrackPosSpeed = 1.0f/50.0f;
real32 camTrackPosThreshold = 1.0f/50.0f;
real32 camTrackPosBase = 3.0f;

#define MaxCameraDistanceFromPlayer 15000.0f
static float MaxCamDistSquared = MaxCameraDistanceFromPlayer * MaxCameraDistanceFromPlayer;
static long CollapseInterval = 8;      // Happens every 8 frames

static real32 CollapseTimeDelay = 7.0f;   // Seconds before an invalid focus dies
static real32 CollapseTimeStart = 0.0f;   // Start Time when focus became invalid

ubyte byte_8B8844 = 1;
PilotEffect pilotEffect = {0};

static real32 ccPilotEffectM0X = 0.0f;
static real32 ccPilotEffectM0Y = 0.0f;
static real32 ccPilotEffectM0Z = 0.0f;

scriptEntry CameraCommandTweaks[] =
{
    makeEntry(CAMERA_DEFAULT_DISTANCE,scriptSetReal32CB),
    makeEntry(CAMERA_FAR_DISTANCE,scriptSetReal32CB),
    makeEntry(CAMERA_FOCUSFAR_DISTANCE,scriptSetReal32CB),
    makeEntry(CAMERA_MOTHERSHIPFOCUS_DISTANCE,scriptSetReal32CB),
    makeEntry(CAMERA_MOTHERSHIPFOCUS_ANGLE,scriptSetAngCB),
    makeEntry(CAMERA_MOTHERSHIPFOCUS_DECLINATION,scriptSetAngCB),
    makeEntry(GO_CAMERA_DELAY_TIME,scriptSetReal32CB),
    makeEntry(CAMERA_MAX_ZOOM_OUT_RATE,scriptSetSdwordCB),
    makeEntry(CAMERA_SCREEN_BORDER,scriptSetSdwordCB),
    makeEntry(CAMERA_NEAR_BORDER,scriptSetSdwordCB),
    makeEntry(CAMERA_NEAR_BIGBORDER,scriptSetSdwordCB),
    makeEntry(CLOSE_ON_ANGLE_SPEED,scriptSetReal32CB),
    makeEntry(CAMERA_ELASTIC_DISTANCE,scriptSetReal32CB),
    makeEntry(CAMERA_ELASTIC_PULL_FACTOR,scriptSetReal32CB),
    makeEntry(CAMERA_OUTBY_SCALE_ZOOM,scriptSetReal32CB),
    makeEntry(CAMERA_MIN_CLOSEFAST_RATE,scriptSetReal32CB),
    makeEntry(MAX_CAMERA_STACKS,scriptSetUdwordCB),
    {"CAM_TRACK_ANG_AUTO", scriptSetReal32CB, &camTrackAngSpeed[0]},
    {"CAM_TRACK_ANG_USER", scriptSetReal32CB, &camTrackAngSpeed[1]},
    {"CAM_TRACK_DIST_AUTO", scriptSetReal32CB, &camTrackDistSpeed[0]},
    {"CAM_TRACK_DIST_USER", scriptSetReal32CB, &camTrackDistSpeed[1]},
    {"CAM_TRACK_DIST_THRESHOLD", scriptSetReal32CB, &camTrackDistThreshold},
    {"CAM_EXP_DIST_AUTO", scriptSetReal32CB, &camTrackDistBase[0]},
    {"CAM_EXP_DIST_USER", scriptSetReal32CB, &camTrackDistBase[1]},
    {"CAM_TRACK_POS", scriptSetReal32CB, &camTrackPosSpeed},
    {"CAM_TRACK_THRESHOLD", scriptSetReal32CB, &camTrackPosThreshold},
    {"CAM_EXP_POS", scriptSetReal32CB, &camTrackPosBase},
    makeEntry(CAM_CUBIC_EVAL_TWEAK,scriptSetReal32CB),
    makeEntry(CAM_CUBIC_EVAL_ANGLE_TWEAK,scriptSetReal32CB),
    makeEntry(CollapseTimeDelay,scriptSetReal32CB),
    endEntry
};

/*=============================================================================
    Private Functions:
=============================================================================*/

// This is the stuff for the camera collapse functions

bool32 ccCameraTimeoutOverride  = FALSE;
extern bool32 kasfTutCameraFocusActive;
bool32 ccFocusIsLegal(SpaceObj *obj);

bool32 ccFocusIsLegalToFocusOn(CameraStackEntry *entry)
{
    struct Player *pPlayer;
    FocusCommand *pFocus;
    Node *pShipNode;
    Ship *pShip;
    long i;
    vector Vect, LookAt;

    pPlayer = universe.curPlayerPtr;
    LookAt = entry->remembercam.lookatpoint;
    pFocus = &entry->focus;

    for (i = 0; i < pFocus->numShips; i++)
    {
        if (ccFocusIsLegal((SpaceObj *)pFocus->ShipPtr[i]))
        {
            vecSub(Vect, LookAt, pFocus->ShipPtr[i]->posinfo.position);
            if (vecMagnitudeSquared(Vect) < MaxCamDistSquared)
            {
                return TRUE;
            }
        }
    }

    pShipNode = universe.ShipList.head;
    while (pShipNode != NULL)
    {
        pShip = (Ship *)listGetStructOfNode(pShipNode);
        if (univIsObjFriendly((SpaceObj *)pShip, pPlayer))
        {
            vecSub(Vect, LookAt, pShip->posinfo.position);
            if (vecMagnitudeSquared(Vect) < MaxCamDistSquared)
            {
                return TRUE;
            }
        }
        pShipNode = pShipNode->next;
    }
    return(FALSE);
}

static bool32 ccFocusStackEntryIsLegal(CameraStackEntry *entry)
{
    if (ccFocusIsLegalToFocusOn(entry))
    {
        CollapseTimeStart = taskTimeElapsed;
        return(TRUE);
    }

   if(!ccCameraTimeoutOverride &&
      (taskTimeElapsed - CollapseTimeStart) > CollapseTimeDelay)
      return FALSE;
   else
      return TRUE;
}

bool32 ccFocusIsLegal(SpaceObj *obj)
{
    udword slot4;

    if (universe.curPlayerPtr->playerState == PLAYER_DEAD)
    {
       return TRUE;
    }

    slot4 = obj->objtype;
    if (slot4 != 0 &&
       (slot4 != 8 || ((Missile *)obj)->missileType != 1))
    {
       return FALSE;
    }

    if (univIsObjFriendly(obj, universe.curPlayerPtr))
    {
       return TRUE;
    }

    if (fogObjectVisibleToPlayer(universe.curPlayerPtr, obj, FALSE))
    {
       return TRUE;
    }

    return nisWideScreenActive || byte_A2DCD4;
}

void ccCollapseFocusStack(CameraCommand *cameracommand)
{
    CameraStackEntry *entry;
    static long CollapseCount = 0;

    CollapseCount++;
    if(CollapseCount < CollapseInterval)
    {
        return;
    }

    CollapseCount = 0;

    entry = currentCameraStackEntry(cameracommand);
    if(!ccFocusStackEntryIsLegal(entry))
    {
        if (kasfTutCameraFocusActive)
        {
            return;
        }
        if(ccFocusOnFleet(cameracommand))
        {
            // the FleetFocus worked, so delete the empty focus
            cameracommand->dontUseVelocityPredInChase = TRUE;
        }
        else
        {
            // if not, pop to sensors, then focus on the mothership
            smFocusOnMothershipOnClose = TRUE;
            smSensorsBegin(NULL, NULL);
        }
    }
}

typedef struct
{
   double   x,y,z;
} DVect;

void ccGetShipCollCenter(Ship *ship, vector *pos, real32 *rad)
{
   *pos = ship->collInfo.collPosition;
   *rad = ship->staticinfo->staticheader.staticCollInfo.collspheresize;
}

static const real32 TAN_FOVConstant = 1.303225f;
static const real32 INV_TAN_FOVConstant = 0.767327207f;

udword NewSetFocusPoint(CameraStackEntry *curentry, real32 *targetDistance)
{
udword numShipsFocusOn = curentry->focus.numShips;
udword i;
Ship *ship;
real32 minx,maxx,miny,maxy,minz,maxz, radius, closezoom, minimumZoom;
vector   position;
real32   MaxDist, Rad, LookCamLen, LookObjLen, invLookCam, invLookObj;
real32   ObjZ, ObjRad, NewDist;
DVect LookToObj, LookToCam, nLookToObj, nLookToCam;


   closezoom = curentry->remembercam.closestZoom;
    curentry->remembercam.oldlookatpoint = curentry->remembercam.lookatpoint;

    if (numShipsFocusOn == 0)
    {
      if(targetDistance)
           *targetDistance = closezoom;
        return FOCUS_NO_SHIPS;
    }

    ship = curentry->focus.ShipPtr[0];

   ccGetShipCollCenter(ship, &position, &radius);

    minx = maxx = position.x;
    miny = maxy = position.y;
    minz = maxz = position.z;

    for (i=1;i<numShipsFocusOn;i++)
    {
        ship = curentry->focus.ShipPtr[i];
      ccGetShipCollCenter(ship, &position, &radius);

        if (position.x < minx) minx = position.x;
        else if (position.x > maxx) maxx = position.x;

        if (position.y < miny) miny = position.y;
        else if (position.y > maxy) maxy = position.y;

        if (position.z < minz) minz = position.z;
      else if (position.z > maxz) maxz = position.z;
    }

    curentry->remembercam.lookatpoint.x = (minx + maxx) * 0.5f;
    curentry->remembercam.lookatpoint.y = (miny + maxy) * 0.5f;
    curentry->remembercam.lookatpoint.z = (minz + maxz) * 0.5f;

   vecSub(LookToCam, curentry->remembercam.eyeposition, curentry->remembercam.lookatpoint);
   LookCamLen = fsqrt(vecMagnitudeSquared(LookToCam));
   invLookCam = 1.0f / LookCamLen;
   vecScalarMultiply(nLookToCam, LookToCam, invLookCam);

   vecNegate(nLookToCam);

    MaxDist = 0.0;
    for(i=0; i<numShipsFocusOn; i++)
    {
        ship = curentry->focus.ShipPtr[i];
        ccGetShipCollCenter(ship, &position, &radius);

        vecSub(LookToObj, curentry->remembercam.lookatpoint, position);
        LookObjLen = fsqrt(vecMagnitudeSquared(LookToObj));
        if (LookObjLen > 0.0f)
        {
            invLookObj = 1.0f / LookObjLen;
            vecScalarMultiply(nLookToObj, LookToObj, invLookObj);

            Rad = (real32)vecDotProduct(nLookToObj, nLookToCam);

            ObjZ = Rad * LookObjLen;
            ObjRad = sqrtf(1.0f - (Rad * Rad)) * LookObjLen;
        }
        else
        {
            Rad = ObjZ = ObjRad = 0.0;
        }

        if (ship->objtype == OBJ_ShipType)
        {
            if ((ship->flags2 & BIT3) != 0)
            {
                minimumZoom = shipGetMinZoomDistance((SpaceObj *)ship);
            }
            else
            {
                minimumZoom = ship->staticinfo->minimumZoomDistance;
            }
        }
        else if (ship->objtype == OBJ_DerelictType)
        {
            DerelictStaticInfo* derelictstaticinfo = (DerelictStaticInfo*)ship->staticinfo;
            minimumZoom = derelictstaticinfo->minimumZoomDistance;
        }
        else
        {
            minimumZoom = 0.0f;
        }
        if (minimumZoom < 0.10f)
        {
            ObjRad += radius;
        }
        else
        {
            ObjRad += minimumZoom * INV_TAN_FOVConstant;
        }

        NewDist = ObjRad * TAN_FOVConstant;

        if(NewDist < closezoom)
            NewDist = closezoom;

        NewDist += ObjZ;

        if (NewDist > MaxDist)
        {
            MaxDist = NewDist;
        }

    }

   if(targetDistance)
      *targetDistance = MaxDist;

   useSlowWheelZoomIn = FALSE;
   return 0;
}



/*-----------------------------------------------------------------------------
    Name        : CloseOnAngle
    Description : gradually changes tracking to desired, at rate rate.  Takes into
                  account that angles can range from 0..2PI, and that 0 == 2PI.
    Inputs      : tracking, desired, rate
    Outputs     : tracking
    Return      : returns 1 if tracking has locked on.
----------------------------------------------------------------------------*/
udword CloseOnAngle(real32 *tracking,real32 desired,real32 rate)
{
    real32 diff = desired - *tracking;
    real32 absdiff = fabsf(diff);

    if (absdiff > PI)       // at most we will be PI radians off, so if we are "more" than
    {                       // PI radians off we are going the wrong way.
        if (desired > PI)
        {
            desired -= TWOPI;
        }
        else
        {
            desired += TWOPI;
        }
        diff = desired - *tracking;
        absdiff = fabsf(diff);
    }

    if (fabsf(diff) < rate)
    {
        *tracking = desired;
        return 1;
    }

    if (diff > 0.0f)
    {
        *tracking += rate;
    }
    else
    {
        *tracking -= rate;
    }

    if (*tracking < 0.0f)
    {
        *tracking += TWOPI;
    }
    else if (*tracking > TWOPI)
    {
        *tracking -= TWOPI;
    }

    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : CloseOn
    Description : gradually changes tracking to desired, at rate rate.
    Inputs      : tracking, desired, rate
    Outputs     : tracking
    Return      : returns 1 if tracking has locked on.
----------------------------------------------------------------------------*/
udword CloseOn(real32 *tracking,real32 desired,real32 rate)
{
    real32 diff = desired - *tracking;

    if (fabsf(diff) < rate)
    {
        *tracking = desired;
        return 1;
    }

    if (diff > 0.0f)
    {
        *tracking += rate;
    }
    else
    {
        *tracking -= rate;
    }
    return 0;
}

/*-----------------------------------------------------------------------------
    Name        : CloseOnFast
    Description : Changes tracking to desired, at a rate proportional to the
                  difference between tracking and desired.  This routine
                  guarantees to close on at least with a rate of minrate.
    Inputs      : tracking, desired, minrate
    Outputs     : tracking
    Return      : returns TRUE if tracking has locked on.
----------------------------------------------------------------------------*/
udword CloseOnFast(real32 *tracking,real32 desired,real32 minrate)
{
    real32 diff = desired - *tracking;
    real32 dist = fabsf(diff);
    real32 rate = dist / 8.0f;

    if (rate <= minrate)
    {
        rate = minrate;
    }

    if (dist < rate)
    {
        return 1;
    }

    if (diff > 0.0f)
    {
        *tracking += rate;
    }
    else
    {
        *tracking -= rate;
    }
    return 0;
}

#if 0
void EyePositionFollowsLookatpoint(Camera *camera)
{
    vector eyeToLookat;
    real32 distmag;
    real32 scale;

    vecSub(eyeToLookat,camera->lookatpoint,camera->eyeposition);

    distmag = fsqrt(vecMagnitudeSquared(eyeToLookat));

    if (distmag >= CAMERA_ELASTIC_DISTANCE)
    {
        // have camera "pulled" in this directio

        scale = CAMERA_ELASTIC_PULL_FACTOR / distmag;
        vecMultiplyByScalar(eyeToLookat,scale);

        vecAddTo(camera->eyeposition,eyeToLookat);
    }
}
#endif


/*----------------------------------------------------------------------------
   EvalCubic:

   This is the Magic Function� - It's basically a cubic spline smoothing
   function, which uses a current position and speed (*y0 and *m0) as well
   as a target value and a step size (y1 and t) to smoothly interpolate from
   the current value of *y0 to the desired target at y1.

   Inputs:  y0 - pointer to the current position value
         m0 - pointer to the current speed value
             (initialize this to 0, then let EvalCubic handle it)
         y1 - Target or destination value
         t  - Step interval (0 < t < 1.0)

   Outputs: y0 - *y0 is overwritten by the new position value
          m0 - *m0 is overwritten by the new speed value
-----------------------------------------------------------------------------*/
void EvalCubic( float *y0, float *m0, float y1, float t )
{
float a,b,c,d,e;

   c = *m0;
   d = *y0;
   e = y1-d;

   a = c-e-e;
// b = e+e+e-c-c;
   b = e+e - (c * CAM_CUBIC_EVAL_TWEAK);      // Different curve prediction


   *y0 = ((((a*t+b)*t)+c)*t)+d;
   *m0 = ((((a+a+a)*t)+(b+b))*t)+c;
}

/*----------------------------------------------------------------------------
   EvalCubicAngle - Same as EvalCube, but handles *y0 & y1 in radians
   between 0 and 2PI.  Performs the interpolation along the shortest
   angular path between *y0 and y1.

   Inputs: 0.0 < (*y0, y1) < 2PI
         0.0 < t < 1.0

   Outputs: *y0 is modified to reflect the new position, and is
          corrected to be between 0.0 to 2PI
          *m0 is the updated speed value
-----------------------------------------------------------------------------*/

void EvalCubicAngle( float *y0, float *m0, float y1, float t )
{
float a,b,c,d,e;

   c = *m0;
   d = *y0;

   e = y1-d;

   if(e < -PI)
      e += TWOPI;
   else if(e > PI)
      e += -TWOPI;

   a = c-e-e;
//   b = e+e+e-c-c;
   b = e+e+e- (c * CAM_CUBIC_EVAL_ANGLE_TWEAK);

   y1 = ((((a*t+b)*t)+c)*t)+d;

   if(y1 < 0.0f)
      y1 += TWOPI;
   else if(y1 > TWOPI)
      y1 -= TWOPI;

   *y0 = y1;
   *m0 = ((((a+a+a)*t)+(b+b))*t)+c;
}

/*-----------------------------------------------------------------------------
    Name        : CubicLogUnsigned
    Description : Performs value tracking in log-base-n space rather than linearly.
    Inputs      : targ - value to track
                  threshold - how close to target we are to track
                  step - how many tracks to do in total
                  logBase - base of the logarithm space in which tracking is done
    Outputs     : pos - updated position
                  speed - updated speed
    Return      : void
----------------------------------------------------------------------------*/
void CubicLogUnsigned(real32 *pos, real32 *speed, real32 targ, real32 threshold, real32 step, real32 logBase)
{
    real32 cubicA;
    real32 cubicB;
    real32 currentSpeed;
    real32 deltaToTarget;
    real32 deltaMagnitude;
    real32 deltaSign;
    real32 thresholdSign;
    real32 logBaseLn;
    real32 logDelta;
    real32 logThreshold;

    deltaToTarget = *pos - targ;
    if (*pos <= targ)
    {
        deltaMagnitude = deltaToTarget + threshold;
        thresholdSign = -1.0f;
    }
    else
    {
        deltaMagnitude = deltaToTarget - threshold;
        thresholdSign = 1.0f;
    }

    if (deltaMagnitude < 0.0f)
    {
        deltaSign = -1.0f;
        deltaMagnitude = -deltaMagnitude;
    }
    else
    {
        deltaSign = 1.0f;
        if (deltaMagnitude == 0.0f)
        {
            deltaMagnitude = 1.0e-19f;
        }
    }

    logBaseLn = logf(logBase);
    logDelta = logf(deltaMagnitude) / logBaseLn;
    logThreshold = logf(threshold) / logBaseLn - logDelta;
    currentSpeed = *speed;
    cubicA = currentSpeed - logThreshold - logThreshold;
    cubicB = logThreshold + logThreshold - CAM_CUBIC_EVAL_TWEAK * currentSpeed;
    *speed = (cubicA * 3.0f * step + cubicB + cubicB) * step + currentSpeed;
    *pos = powf(logBase, ((cubicA * step + cubicB) * step + currentSpeed) * step + logDelta) * deltaSign
         + thresholdSign * threshold
         + targ;
}

/*-----------------------------------------------------------------------------
    Name        : CameraChase
    Description : causes the actualcamera to track the currently set camera.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void CameraChase(CameraCommand *cameracommand,real32 zoomfactor)
{
CameraStackEntry *current = currentCameraStackEntry(cameracommand);
Camera *desired = &current->remembercam;
vector targetVelocity;
vector desiredEyePosition;
ubyte *chaseState = cameracommand->actualcamera.cataUnknown55;
real32 *pendingDistance = (real32 *)(chaseState + 3);
bool32 dontUseVelocityPredInChase = FALSE;
bool32 snapChaseState = FALSE;
sdword frames;
sdword iterationCount;
sdword iterationIndex;
sdword indAng;
real32 savedDistSpeed;
real32 distance;
static real32 oldTime = 0.0f;
static real32 angSpeed = 0.0f;
static real32 decSpeed = 0.0f;
static real32 xSpeed = 0.0f;
static real32 ySpeed = 0.0f;
static real32 zSpeed = 0.0f;
static real32 distSpeed = 0.0f;
static real32 eyeXSpeed = 0.0f;
static real32 eyeYSpeed = 0.0f;
static real32 eyeZSpeed = 0.0f;

savedDistSpeed = distSpeed;
if (cameracommand->dontUseVelocityPredInChase == TRUE)
{
    dontUseVelocityPredInChase = TRUE;
    cameracommand->dontUseVelocityPredInChase = FALSE;
}

if (cameracommand->ccMode & CCMODE_PILOT_SHIP)
{
    if (current->focus.numShips == 1)
    {
        cameraSetEyePositionBasedOnShip(&cameracommand->actualcamera, current->focus.ShipPtr[0]);
        return;
    }

    bitClear(cameracommand->ccMode, CCMODE_PILOT_SHIP);
}

vecSet(cameracommand->actualcamera.upvector, 0.0f, 0.0f, 1.0f);
if (cameracommand->transition != CAMERA_FOCUSORIENT &&
    cameracommand->transition != CAMERA_FOCUSMOVE &&
    cameracommand->transition != CAMERA_FOCUSPOP)
{
    if (cameracommand->transition == CAMERA_SET)
    {
        cameracommand->actualcamera.oldlookatpoint = cameracommand->actualcamera.lookatpoint;
        cameracommand->actualcamera.lookatpoint = desired->lookatpoint;
        cameracommand->actualcamera.distance = desired->distance;
        cameracommand->actualcamera.angle = desired->angle;
        cameracommand->actualcamera.declination = desired->declination;
        cameracommand->actualcamera.eyeposition = desired->eyeposition;
        ccSetDirtyBit(&pilotEffect);
    }
    return;
}

frames = utyNFrameTicks;
if (frames > 64)
{
    frames = 64;
}

indAng = cameracommand->UserControlled & CAM_USER_MOVED;
if (!dontUseVelocityPredInChase)
{
    vecSub(targetVelocity, desired->lookatpoint, desired->oldlookatpoint);
    vecAddTo(cameracommand->actualcamera.lookatpoint, targetVelocity);
    vecAddTo(cameracommand->actualcamera.eyeposition, targetVelocity);
}

if (chaseState[0] == 1)
{
    snapChaseState = TRUE;
    chaseState[0] = 0;
}

iterationCount = snapChaseState ? 1 : frames;
for (iterationIndex = 0; iterationIndex < iterationCount; iterationIndex++)
{
    if (snapChaseState)
    {
        cameracommand->actualcamera.lookatpoint = desired->lookatpoint;
        xSpeed = 0.0f;
        ySpeed = 0.0f;
        zSpeed = 0.0f;
        distSpeed = 0.0f;
        cameracommand->actualcamera.angle = desired->angle;
        angSpeed = 0.0f;
        cameracommand->actualcamera.declination = desired->declination;
        decSpeed = 0.0f;
        if (*pendingDistance > 0.0f)
        {
            desired->distance = *pendingDistance;
            *pendingDistance = 0.0f;
            cameracommand->zoomInCloseAsPossible = FALSE;
        }
    }

    EvalCubicAngle(&cameracommand->actualcamera.angle, &angSpeed, desired->angle, camTrackAngSpeed[indAng]);
    EvalCubic(&cameracommand->actualcamera.declination, &decSpeed, desired->declination, camTrackAngSpeed[indAng]);

    CubicLogUnsigned(
        &cameracommand->actualcamera.lookatpoint.x,
        &xSpeed,
        desired->lookatpoint.x,
        camTrackPosThreshold,
        camTrackPosSpeed,
        camTrackPosBase);
    CubicLogUnsigned(
        &cameracommand->actualcamera.lookatpoint.y,
        &ySpeed,
        desired->lookatpoint.y,
        camTrackPosThreshold,
        camTrackPosSpeed,
        camTrackPosBase);
    CubicLogUnsigned(
        &cameracommand->actualcamera.lookatpoint.z,
        &zSpeed,
        desired->lookatpoint.z,
        camTrackPosThreshold,
        camTrackPosSpeed,
        camTrackPosBase);

    vecSub(desiredEyePosition, cameracommand->actualcamera.eyeposition, cameracommand->actualcamera.lookatpoint);
    vecNormalize(&desiredEyePosition);
    vecMultiplyByScalar(desiredEyePosition, desired->distance);
    vecAddTo(desiredEyePosition, desired->lookatpoint);

    if (snapChaseState)
    {
        cameracommand->actualcamera.eyeposition = desiredEyePosition;
        eyeXSpeed = 0.0f;
        eyeYSpeed = 0.0f;
        eyeZSpeed = 0.0f;
        distSpeed = 0.0f;
    }

    CubicLogUnsigned(
        &cameracommand->actualcamera.eyeposition.x,
        &eyeXSpeed,
        desiredEyePosition.x,
        camTrackPosThreshold,
        camTrackPosSpeed,
        camTrackPosBase);
    CubicLogUnsigned(
        &cameracommand->actualcamera.eyeposition.y,
        &eyeYSpeed,
        desiredEyePosition.y,
        camTrackPosThreshold,
        camTrackPosSpeed,
        camTrackPosBase);
    CubicLogUnsigned(
        &cameracommand->actualcamera.eyeposition.z,
        &eyeZSpeed,
        desiredEyePosition.z,
        camTrackPosThreshold,
        camTrackPosSpeed,
        camTrackPosBase);
    vecSub(desiredEyePosition, cameracommand->actualcamera.eyeposition, cameracommand->actualcamera.lookatpoint);
    distance = (real32)fmathSqrtDouble(vecMagnitudeSquared(desiredEyePosition));
    cameracommand->actualcamera.distance = distance;
}

if ((cameracommand->ccMode & (CCMODE_FOCUS_CHANGES_LOOKATPOINT_ONLY | CCMODE_FOCUS_CHANGES_NOTHING)) == 0)
{
    cameraSetEyePosition(&cameracommand->actualcamera);
}

if (snapChaseState)
{
    univUpdateRenderList();
    univUpdateMinorRenderList();
    distSpeed = savedDistSpeed;
}
}

/*-----------------------------------------------------------------------------
    Name        : GetDistanceAngleDeclination
    Description : Sets the distance, angle and declination given the distvec
                  vector which is a vector pointing from the lookatpoint to
                  the eyepoint.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void GetDistanceAngleDeclination(Camera *camera,vector *distvec)
{
    real32 value;
    // calculate distance
    camera->distance = sqrtf(vecMagnitudeSquared(*distvec));

    // now calculate angle and declination to face target
    camera->angle = (real32)atan2(distvec->y,distvec->x);
    value = -distvec->z/camera->distance;

    dbgAssert(fabsf(value) <= 1.01f);       // USE 1.01 SO ROUND OFF ERRORS DON'T CRASH US

    if(value < -1.0f)
        value = -1.0f;
    else if(value > 1.0f)
        value = 1.0f;

    camera->declination = (PI/2.0f) - (real32)acos(value);
}

void FocusOnNewEntry(CameraCommand *cameracommand,CameraStackEntry *entry)
{
    Node *deletenode;
    Node *nextnode;

    CameraStackEntry *curentry = currentCameraStackEntry(cameracommand);

    dbgAssert(curentry);

    listAddNodeAfter(&curentry->stacklink,&entry->stacklink,entry);

    // Delete all nodes after new entry
    deletenode = entry->stacklink.next;
    while (deletenode != NULL)
    {
        nextnode = deletenode->next;
        listDeleteNode(deletenode);
        deletenode = nextnode;
    }

    cameracommand->currentCameraStack = entry;

    if (cameracommand->camerastack.num >= MAX_CAMERA_STACKS)
    {
        // remove head of stack
        listDeleteNode(cameracommand->camerastack.head);
    }

#ifdef gshaw
    dbgMessagef("\nStack Depth: %d",cameracommand->camerastack.num);
#endif
}

/*=============================================================================
        Public Functions:
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : ccInit
    Description : Initializes the camera command object
    Inputs      : sphereCameraCommandIsIn
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccInit(CameraCommand *cameracommand)
{
    CameraStackEntry *firstentry;

    listInit(&cameracommand->camerastack);
    cameracommand->ViewEntireMissphere = FALSE;
    cameracommand->dontUseVelocityPredInChase = FALSE;

    cameraInit(&cameracommand->actualcamera,CAMERA_FAR_DISTANCE);

    firstentry = memAlloc(sizeof(CameraStackEntry),"CamStackEntry",NonVolatile);
    firstentry->focus.numShips = 0;
    firstentry->remembercam = cameracommand->actualcamera;

    listAddNode(&cameracommand->camerastack,&firstentry->stacklink,firstentry);
    cameracommand->currentCameraStack = firstentry;

// cameracommand->transition = CAMERA_SET;
   cameracommand->transition = CAMERA_FOCUSORIENT;
   cameracommand->action = CAMACTION_FOCUS;
   cameracommand->transTimeStamp = 0.0f;
// cameracommand->zoomInCloseAsPossible = FALSE;
   cameracommand->zoomInCloseAsPossible = TRUE;
   cameracommand->UserControlled = 0;
   cameracommand->lockCameraTimeStamp = 0.0f;

    cameracommand->ccMode = 0;
    cameracommand->dontFocusOnMe = firstentry;
}

/*-----------------------------------------------------------------------------
    Name        : ccSetModeFlag
    Description : sets a cameracommand mode flag
    Inputs      : ccModeFlag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccSetModeFlag(CameraCommand *cameracommand,udword ccModeFlag)
{
    cameracommand->ccMode |= ccModeFlag;
}

/*-----------------------------------------------------------------------------
    Name        : ccClearModeFlag
    Description : clears a cameracommand mode flag
    Inputs      : ccModeFlag
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccClearModeFlag(CameraCommand *cameracommand,udword ccModeFlag)
{
    bitClear(cameracommand->ccMode,ccModeFlag);
}

bool32 ccIsInPilotMode(void)
{
    FocusCommand *focus = &universe.mainCameraCommand.currentCameraStack->focus;

    return focus->numShips == 1 &&
           bitTest(universe.mainCameraCommand.ccMode, CCMODE_PILOT_SHIP) &&
           focus->ShipPtr[0]->objtype == OBJ_ShipType;
}

bool32 ccIsPilotingShip(Ship *ship)
{
    if (!ccIsInPilotMode())
    {
        return FALSE;
    }

    return universe.mainCameraCommand.currentCameraStack->focus.ShipPtr[0] == ship;
}


/*-----------------------------------------------------------------------------
    Name        : ccFocusOnMyMothership
    Description : changes focus point to point to current players MotherShip
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccFocusOnMyMothership(CameraCommand *cameracommand)
{
FocusCommand   focus;
CameraStackEntry *entry;

   if (universe.curPlayerPtr->PlayerMothership != NULL)
   {
      focus.ShipPtr[0] = universe.curPlayerPtr->PlayerMothership;
      focus.numShips = 1;
      ccFocus(cameracommand, &focus);
      cameracommand->zoomInCloseAsPossible = FALSE;

      entry = currentCameraStackEntry(cameracommand);
     entry->remembercam.distance = CAMERA_FAR_DISTANCE;

     cameraSetEyePosition(&entry->remembercam);
   }
}

real32 GetObjAngle(SpaceObjRot *robj)
{
    vector heading;
    real32 angle;

    matGetVectFromMatrixCol3(heading,robj->rotinfo.coordsys);

    angle = (real32)atan2(heading.y,heading.x);

    return angle;
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusOnPlayersMissionSphere
    Description : changes focus point to point to current players mission sphere
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccFocusOnPlayersMothership(CameraCommand *cameracommand,uword playerindex)
{
    FocusCommand   focus;
    CameraStackEntry *entry;
    // find players mission sphere
    Ship *mothership = universe.players[playerindex].PlayerMothership;
    real32 angle, declination, distance;
    vector distVector;

    if (mothership == NULL)
    {
        return;
    }

    // New Version ----------------------------
    focus.ShipPtr[0] = mothership;
    focus.numShips = 1;
    ccFocus(cameracommand, &focus);
    cameracommand->zoomInCloseAsPossible = FALSE;

    entry = currentCameraStackEntry(cameracommand);

    if (!singlePlayerGame && spFindCameraAttitude(&entry->remembercam.eyeposition))
    {
        vecSub(distVector, entry->remembercam.eyeposition, entry->remembercam.lookatpoint);
        GetDistanceAngleDeclination(&entry->remembercam, &distVector);
        distance = entry->remembercam.distance;
        angle = entry->remembercam.angle;
        declination = entry->remembercam.declination;
        angle += GetObjAngle((SpaceObjRot *)mothership);
    }
    else
    {
        if (singlePlayerGame && singlePlayerGameInfo.currentMission == 2)
        {
            angle = GetObjAngle((SpaceObjRot *)mothership) - CAMERA_MOTHERSHIPFOCUS_ANGLE;
        }
        else
        {
            angle = GetObjAngle((SpaceObjRot *)mothership) + CAMERA_MOTHERSHIPFOCUS_ANGLE;
        }
        distance = CAMERA_MOTHERSHIPFOCUS_DISTANCE;
        declination = CAMERA_MOTHERSHIPFOCUS_DECLINATION;
    }
    if (angle < 0)
    {
        angle += TWOPI;
    }
    else if (angle > TWOPI)
    {
        angle -= TWOPI;
    }
    entry->remembercam.distance = distance;
    entry->remembercam.angle = angle;
    entry->remembercam.declination = declination;

    cameraSetEyePosition(&entry->remembercam);

    cameracommand->actualcamera = entry->remembercam;

   // New Version ----------------------------

// Old Version -------------------------------
// entry = currentCameraStackEntry(cameracommand);
// dbgAssert(entry);
// cameraChangeLookatpoint(&cameracommand->actualcamera,&mothership->posinfo.position);
// entry->remembercam = cameracommand->actualcamera;
// cameracommand->UserControlled = 0;
// Old Version -------------------------------
}

/*-----------------------------------------------------------------------------
    Name        : ccClose
    Description : Closes the camera command object
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccClose(CameraCommand *cameracommand)
{
    listDeleteAll(&cameracommand->camerastack);
}

/*-----------------------------------------------------------------------------
    Name        : ccReset
    Description : Resets the camera command object
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccReset(CameraCommand *cameracommand)
{
    ccClose(cameracommand);
    ccInit(cameracommand);
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusGeneral
    Description : gives the camera command object the command to focus on
                  a group of ships
    Inputs      : focuscom, command which tells what ships to focus on
              bCloseUp - Zoom in Close = TRUE / FALSE
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccFocusGeneral(CameraCommand *cameracommand,FocusCommand *focuscom, bool32 bCloseUp)
{
    CameraStackEntry *oldentry;
    CameraStackEntry *entry;
    udword excesssize;
    vector distvec;
    vector lookatdiffvec;
    real32 lookatdiffdist, newDist, MinDist;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bFocus)
    {
        return;
    }

    if (focuscom->numShips < 1)
    {
        return;
    }

    //dbgAssert(focuscom->numShips >= 1);

    ccFocusCullRadiusMean(focuscom, RENDER_VIEWABLE_DISTANCE_SQR, NULL);

    if (focuscom->numShips == 0)
    {                                                       //if all ships focussed out (should never happen)
        return;
    }

    oldentry = currentCameraStackEntry(cameracommand);

    if (SelectionsAreEquivalent((SelectCommand *)&oldentry->focus,(SelectCommand *)focuscom))
    {
#if DEBUG_CAMERACOMMAND
        dbgMessage("\nAutozooming in...");
#endif
        cameracommand->zoomInCloseAsPossible = TRUE;
        cameracommand->action = CAMACTION_FOCUS;
        cameracommand->UserControlled = 0;

        if (cameracommand->lockCameraTimeStamp != 0.0f)     // cancel any freeze camera action
        {
            cameracommand->action = CAMACTION_FOCUS;
            cameracommand->lockCameraTimeStamp = 0.0f;
            cameracommand->transition = CAMERA_FOCUSORIENT;
            cameracommand->dontUseVelocityPredInChase = TRUE;
        }

        return;
    }

    excesssize = (focuscom->numShips-1)*sizeof(ShipPtr);

    entry = memAlloc(sizeof(CameraStackEntry)+excesssize,"CamStackEntry",NonVolatile);

    entry->remembercam = oldentry->remembercam;         // start with previous camera and change it

    memcpy(&entry->focus,focuscom,sizeof(FocusCommand)+excesssize);

    // add code here to focus on new object
    NewSetFocusPoint(entry,&MinDist);

   vecSub(distvec, cameracommand->actualcamera.eyeposition, entry->remembercam.lookatpoint);

    // calculate distance
   newDist = fsqrt(vecMagnitudeSquared(distvec));
   entry->remembercam.distance = oldentry->remembercam.distance;
   if(entry->remembercam.distance < MinDist)
      entry->remembercam.distance = MinDist;

    vecSub(lookatdiffvec,entry->remembercam.lookatpoint,oldentry->remembercam.lookatpoint);
    lookatdiffdist = vecMagnitudeSquared(lookatdiffvec);

    if (lookatdiffdist >= RENDER_VIEWABLE_DISTANCE_SQR)
    {
        // now calculate angle and declination to face target
        entry->remembercam.angle = (real32)atan2(distvec.y,distvec.x);
        if (entry->remembercam.angle < 0)
        {
            entry->remembercam.angle += TWOPI;
        }

        entry->remembercam.declination = (real32)acos(distvec.z/newDist) - (PI/2.0f);
    }

    cameraSetEyePosition(&entry->remembercam);

    cameracommand->ViewEntireMissphere = FALSE;

    FocusOnNewEntry(cameracommand,entry);

    cameracommand->action = CAMACTION_FOCUS;
    cameracommand->transition = CAMERA_FOCUSORIENT;
    cameracommand->transTimeStamp = taskTimeElapsed;
    cameracommand->zoomInCloseAsPossible = bCloseUp;
    cameracommand->UserControlled = 0;
    cameracommand->lockCameraTimeStamp = 0.0f;

   if(oldentry->focus.numShips == 0)
      listDeleteNode(&oldentry->stacklink);
}

// General case of ccFocus : Don't zoom in close to focus target
void ccFocus(CameraCommand *cameracommand, FocusCommand *focuscom)
{
   ccFocusGeneral(cameracommand, focuscom, FALSE);
}

// Special case of ccFocus : Zoom in close to target when using ALT-Focus
void ccFocusClose(CameraCommand *cameracommand, FocusCommand *focuscom)
{
   ccFocusGeneral(cameracommand, focuscom, TRUE);
}

#if 0
/*-----------------------------------------------------------------------------
    Name        : ccFocusSideways
    Description : gives the camera command object the command to focus on
                  a group of ships, with a given angle and declination of camera
    Inputs      : focuscom, command which tells what ships to focus on
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccFocusSideways(CameraCommand *cameracommand,FocusCommand *focuscom)
{
    CameraStackEntry *entry;

    ccFocus(cameracommand,focuscom);

    entry = currentCameraStackEntry(cameracommand);

    if ((entry->remembercam.angle > DEG_TO_RAD(90.0f)) || (entry->remembercam.angle < DEG_TO_RAD(-90.0f)))
    {
        entry->remembercam.angle = DEG_TO_RAD(180.0f);
    }
    else
    {
        entry->remembercam.angle = 0.0f;
    }

    entry->remembercam.declination = 0.0f;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : ccChangeAngleDeclination
    Description : changes the angle and declination of camera
    Inputs      : angle, declination
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccChangeAngleDeclination(CameraCommand *cameracommand,real32 angle,real32 declination)
{
    CameraStackEntry *entry;

    entry = currentCameraStackEntry(cameracommand);

    entry->remembercam.angle = angle;
    entry->remembercam.declination = declination;

    cameracommand->actualcamera.angle = angle;
    cameracommand->actualcamera.declination = declination;
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusFar
    Description : gives the camera command object the command to focus on
                  a group of ships, from a far distance
    Inputs      : focuscom, command which tells what ships to focus on
                  currentCamera - camera to start with
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccFocusFar(CameraCommand *cameracommand,FocusCommand *focuscom, Camera *currentCamera)
{
    CameraStackEntry *oldentry;
    CameraStackEntry *entry;
    udword excesssize;

    if (focuscom->numShips < 1)
    {
        return;
    }

    //dbgAssert(focuscom->numShips >= 1);

    oldentry = currentCameraStackEntry(cameracommand);

    excesssize = (focuscom->numShips-1)*sizeof(ShipPtr);

    entry = memAlloc(sizeof(CameraStackEntry)+excesssize,"CamStackEntry",NonVolatile);

//    cameraCopyPositionInfo(&entry->remembercam, currentCamera);
    entry->remembercam = *currentCamera;         // start with current camera and change it


    memcpy(&entry->focus,focuscom,sizeof(FocusCommand)+excesssize);

    // add code here to focus on new object
    NewSetFocusPoint(entry,NULL);

    // calculate distance
    entry->remembercam.distance = CAMERA_FAR_DISTANCE;
    cameraSetEyePosition(&entry->remembercam);

    cameracommand->ViewEntireMissphere = FALSE;

    FocusOnNewEntry(cameracommand,entry);

    cameracommand->action = CAMACTION_FOCUSFAR;
    cameracommand->transition = CAMERA_FOCUSORIENT;
    cameracommand->transTimeStamp = taskTimeElapsed;
    cameracommand->zoomInCloseAsPossible = FALSE;
    cameracommand->UserControlled = 0;
    cameracommand->lockCameraTimeStamp = 0.0f;
}


/*-----------------------------------------------------------------------------
    Name        : ccFocusExact
    Description : copies focus of focustocopy to cameracommand
    Inputs      : cameracommand, focustocopy
    Outputs     :
    Return      : TRUE if successful, FALSE if not able to do the operation because
                  focustocopy wasn't focused on any ships.
----------------------------------------------------------------------------*/
bool32 ccFocusExact(CameraCommand *cameracommand,CameraCommand *focustocopy)
{
    FocusCommand *currentfocus = currentFocus(focustocopy);

    if (currentfocus->numShips < 1)
    {
        return FALSE;
    }

    ccFocus(cameracommand,currentfocus);
    cameraCopyPositionInfo(&currentCameraStackEntry(cameracommand)->remembercam,&currentCameraStackEntry(focustocopy)->remembercam);
    cameraCopyPositionInfo(&cameracommand->actualcamera,&focustocopy->actualcamera);

    cameracommand->transition = focustocopy->transition;
    cameracommand->action = focustocopy->action;
    cameracommand->transTimeStamp = focustocopy->transTimeStamp;
    cameracommand->UserControlled = focustocopy->UserControlled;
    cameracommand->zoomInCloseAsPossible = focustocopy->zoomInCloseAsPossible;
    cameracommand->lockCameraTimeStamp = focustocopy->lockCameraTimeStamp;
    cameracommand->dontUseVelocityPredInChase = focustocopy->dontUseVelocityPredInChase;

    return TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : ccCopyCamera
    Description : Copies cameraToCopy into the cameracommand current camera
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccCopyCamera(CameraCommand *cameracommand,Camera *cameraToCopy)
{
    vector distvec;

    CameraStackEntry *curentry = currentCameraStackEntry(cameracommand);

    vecSub(distvec, cameraToCopy->eyeposition, cameraToCopy->lookatpoint);

    GetDistanceAngleDeclination(cameraToCopy,&distvec);
//    if (cameraToCopy->distance < 1000.0f)
//    {
//        cameraToCopy->distance = 1000.0f;
//        cameraSetEyePosition(cameraToCopy);
//    }

    cameraCopyPositionInfo(&curentry->remembercam,cameraToCopy);
    cameracommand->actualcamera = curentry->remembercam;

}

/*-----------------------------------------------------------------------------
    Name        : ccCancelFocus
    Description : Cancel's focus (pops off current focused items of the camera
                  stack, and reverts to the old one)
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccCancelFocus(CameraCommand *cameracommand)
{
    Node *node, *prevNode;
    CameraStackEntry *entry, *oldentry;

    if((tutorial==TUTORIAL_ONLY) && !tutEnable.bFocusCancel)
        return;

    oldentry = currentCameraStackEntry(cameracommand);
    node = oldentry->stacklink.prev;

    if (node == NULL)
    {
        return;
    }

    entry = (CameraStackEntry *)listGetStructOfNode(node);

    while (!ccFocusIsLegalToFocusOn(entry) && node != NULL && entry != cameracommand->dontFocusOnMe)
    {
        prevNode = node->prev;
        listDeleteNode(node);
        node = prevNode;
        if (node == NULL)
        {
            return;
        }
        entry = (CameraStackEntry *)listGetStructOfNode(node);
    }

    if (entry == cameracommand->dontFocusOnMe)
    {
        return;
    }

    cameracommand->ViewEntireMissphere = FALSE;

    ccFocusCullRadiusMean(&entry->focus, RENDER_VIEWABLE_DISTANCE_SQR, NULL);

    entry->remembercam.angle = cameracommand->actualcamera.angle;
    entry->remembercam.declination = cameracommand->actualcamera.declination;
    NewSetFocusPoint(entry,NULL);

    cameraSetEyePosition(&entry->remembercam);

    cameracommand->currentCameraStack = entry;

    cameracommand->action = CAMACTION_CANCELFOCUS;
    cameracommand->transition = CAMERA_FOCUSORIENT;
    cameracommand->transTimeStamp = taskTimeElapsed;
    cameracommand->zoomInCloseAsPossible = FALSE;
    cameracommand->UserControlled = 0;
    cameracommand->lockCameraTimeStamp = 0.0f;

    if(oldentry->focus.numShips == 0)
    {
        listDeleteNode(&oldentry->stacklink);
    }
}

/*-----------------------------------------------------------------------------
    Name        : ccForwardFocus
    Description : Move's forward on the focus stack, similar to hitting the
                  forward button on internet explorer.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccForwardFocus(CameraCommand *cameracommand)
{
    Node *next, *beyondNext;
    CameraStackEntry *entry, *oldentry;

    cameracommand->ViewEntireMissphere = FALSE;

   oldentry = currentCameraStackEntry(cameracommand);
    next = oldentry->stacklink.next;

    if (next == NULL)
    {
        return;
    }

    entry = (CameraStackEntry *)listGetStructOfNode(next);

    while (next != NULL && !ccFocusIsLegalToFocusOn(entry))
    {
        beyondNext = next->next;
        listDeleteNode(next);
        next = beyondNext;
        if (next == NULL)
        {
            return;
        }
        entry = (CameraStackEntry *)listGetStructOfNode(next);
    }

    ccFocusCullRadiusMean(&entry->focus, RENDER_VIEWABLE_DISTANCE_SQR, NULL);

    entry->remembercam.angle = cameracommand->actualcamera.angle;
    entry->remembercam.declination = cameracommand->actualcamera.declination;
    NewSetFocusPoint(entry,NULL);

    cameraSetEyePosition(&entry->remembercam);

    cameracommand->currentCameraStack = entry;

    cameracommand->action = CAMACTION_FORWARDFOCUS;
    cameracommand->transition = CAMERA_FOCUSORIENT;
    cameracommand->transTimeStamp = taskTimeElapsed;
    cameracommand->zoomInCloseAsPossible = TRUE;
    cameracommand->UserControlled = 0;
    cameracommand->lockCameraTimeStamp = 0.0f;

   if(oldentry->focus.numShips == 0)
      listDeleteNode(&oldentry->stacklink);
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusOnFleet
    Description : focuses on fleet
    Inputs      :
    Outputs     :
    Return      : TRUE if there were ships to focus on, FALSE otherwise
----------------------------------------------------------------------------*/
bool32 ccFocusOnFleet(CameraCommand *cameracommand)
{
    uword numShips = 0;
    uword i;
    CameraStackEntry *entry, *oldentry;
    Node *objnode = universe.RenderList.head;
    Ship *ship;

   oldentry = currentCameraStackEntry(cameracommand);

    // count number of ships to focus on (current player's fleet)
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if (ship->objtype == OBJ_ShipType && !(ship->flags & SOF_Dead))
        {
            if (ship->playerowner == universe.curPlayerPtr)
            {
                numShips++;
            }
        }
        objnode = objnode->next;
    }

    if (numShips == 0)
    {
        return FALSE;
    }

    entry = memAlloc(sizeofCameraStackEntry(numShips),"CamStackEntry",NonVolatile);

    // put ships of current player's fleet into focus
    entry->focus.numShips = numShips;
    i = 0;
    objnode = universe.RenderList.head;
    while (objnode != NULL)
    {
        ship = (Ship *)listGetStructOfNode(objnode);
        if (ship->objtype == OBJ_ShipType && !(ship->flags & SOF_Dead))
        {
            if (ship->playerowner == universe.curPlayerPtr)
            {
                entry->focus.ShipPtr[i++] = ship;
            }
        }

        objnode = objnode->next;
    }

    ccFocusCullRadiusMean(&entry->focus, RENDER_VIEWABLE_DISTANCE_SQR, NULL);

    entry->remembercam = cameracommand->actualcamera;
    entry->remembercam.distance = CAMERA_FAR_DISTANCE;
    NewSetFocusPoint(entry,NULL);

    cameraSetEyePosition(&entry->remembercam);

    FocusOnNewEntry(cameracommand,entry);

   if(oldentry->focus.numShips == 0)
      listDeleteNode(&oldentry->stacklink);

    return TRUE;
}

void ccFocusOnFleetSmooth(CameraCommand *cameracommand)
{
    if (ccFocusOnFleet(cameracommand))
    {
        cameracommand->action = CAMACTION_VIEWMISSPHERE;
        cameracommand->transition = CAMERA_FOCUSORIENT;
        cameracommand->transTimeStamp = taskTimeElapsed;
        cameracommand->zoomInCloseAsPossible = FALSE;
        cameracommand->UserControlled = 0;
        cameracommand->lockCameraTimeStamp = 0.0f;
    }
}

/*-----------------------------------------------------------------------------
    Name        : ccViewToggleMissionSphere
    Description : toggles between far-out mission sphere view and regular view
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccViewToggleMissionSphere(CameraCommand *cameracommand)
{
    if (cameracommand->ViewEntireMissphere)
    {
        ccCancelFocus(cameracommand);
    }
    else
    {
        if (ccFocusOnFleet(cameracommand))
        {
            cameracommand->ViewEntireMissphere = TRUE;

            cameracommand->action = CAMACTION_VIEWMISSPHERE;
            cameracommand->transition = CAMERA_FOCUSORIENT;
            cameracommand->transTimeStamp = taskTimeElapsed;
            cameracommand->zoomInCloseAsPossible = FALSE;
            cameracommand->UserControlled = 0;
            cameracommand->lockCameraTimeStamp = 0.0f;
        }
        else
        {
            return;
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : ccFreezeLookAtPoint
    Description :
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccFreezeLookAtPoint(CameraCommand *cameracommand,SelectCommand *selectcom)
{
#if 0
    CameraStackEntry *entry = currentCameraStackEntry(cameracommand);

    if (SelectionsAreEquivalent(selectcom,&entry->focus))
    {
        cameracommand->lockCameraTimeStamp = taskTimeElapsed;
    }
#endif
}

/*-----------------------------------------------------------------------------
    Name        : ccRemoveTheseShips
    Description : call this routine to remove references to theseships from cameracommand
    Inputs      : theseships
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccRemoveTheseShips(CameraCommand *cameracommand,SelectCommand *theseships)
{
    Node *curnode = cameracommand->camerastack.head;
   Node *nextNode;
    CameraStackEntry *stackentry;
   sdword oldFocusShips;
    CameraStackEntry *curentry = currentCameraStackEntry(cameracommand);

   oldFocusShips = curentry->focus.numShips;

    while (curnode != NULL)
    {
      nextNode = curnode->next;
        stackentry = (CameraStackEntry *)listGetStructOfNode(curnode);
        clRemoveTargetFromSelection((SelectAnyCommand *)&stackentry->focus, (TargetPtr)theseships);

      if(!smGhostMode && (stackentry->focus.numShips == 0) && (stackentry != curentry))
           listDeleteNode(curnode);
      else
          cameracommand->zoomInCloseAsPossible = FALSE;

        curnode = nextNode;
    }

   if(curentry->focus.numShips != oldFocusShips)
      cameracommand->dontUseVelocityPredInChase = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : ccRemoveShip
    Description : call this routine to remove references to ship from cameracommand
    Inputs      : ship
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccRemoveShip(CameraCommand *cameracommand,Ship *ship)
{
    Node *curnode = cameracommand->camerastack.head;
   Node *nextNode;
    CameraStackEntry *stackentry;
   sdword oldFocusShips;
    CameraStackEntry *curentry = currentCameraStackEntry(cameracommand);

   oldFocusShips = curentry->focus.numShips;

    while (curnode != NULL)
    {
      nextNode = curnode->next;
        stackentry = (CameraStackEntry *)listGetStructOfNode(curnode);
        clRemoveShipFromSelection(&stackentry->focus,ship);

      if(!smGhostMode && (stackentry->focus.numShips == 0) && (stackentry != curentry))
      {
           listDeleteNode(curnode);
      }
      else
      {
          cameracommand->zoomInCloseAsPossible = FALSE;
      }

        curnode = nextNode;
    }

   if(curentry->focus.numShips != oldFocusShips)
      cameracommand->dontUseVelocityPredInChase = TRUE;
}

/*-----------------------------------------------------------------------------
    Name        : ccControl
    Description : does all control for the Camera Command.  This routine should
                  be called everytime you want the user to be able to control
                  the camera.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void ccControl(CameraCommand *cameracommand)
{
    udword focusOnScreen;
    real32 zoomfactor = 0.0f;
    CameraStackEntry *curentry = currentCameraStackEntry(cameracommand);
    float targetDistance;
    sdword UserAction;
    static long collapseCheckCount = 0;
    static real32 collapseLastLegalFocusTime = -1.0f;

    if (cameracommand->actualcamera.ignoreZoom)
        return;

    if (cameracommand->lockCameraTimeStamp == 0.0f)
        focusOnScreen = NewSetFocusPoint(curentry, &targetDistance);
    else
    {
        if ((taskTimeElapsed - cameracommand->lockCameraTimeStamp) > GO_CAMERA_DELAY_TIME)
        {
            cameracommand->lockCameraTimeStamp = 0.0f;
            cameracommand->transition = CAMERA_FOCUSORIENT;
            cameracommand->dontUseVelocityPredInChase = TRUE;

            focusOnScreen = NewSetFocusPoint(curentry, &targetDistance);
        }
        else
            focusOnScreen = FOCUS_NO_SHIPS;
    }

//  if(curentry->remembercam.distance > targetDistance)
//      zoomInAllowed = TRUE;
//  else
//      zoomInAllowed = FALSE;

   if ((cameracommand->ccMode & CCMODE_LOCK_OUT_USER_INPUT) == 0)
   {
      UserAction = cameraControl(&curentry->remembercam, TRUE);

      if(UserAction)
      {
         cameracommand->UserControlled = UserAction;
         if(UserAction & CAM_USER_ZOOMED)
            cameracommand->zoomInCloseAsPossible = FALSE;

         if(curentry->remembercam.distance < targetDistance)
            curentry->remembercam.distance = targetDistance;
      }
   }


    if (cameracommand->ccMode & CCMODE_FOCUS_CHANGES_LOOKATPOINT_ONLY)
      ;
    else if (!(cameracommand->ccMode & CCMODE_FOCUS_CHANGES_NOTHING))
        cameraSetEyePosition(&curentry->remembercam);


   if(cameracommand->zoomInCloseAsPossible)
   {
        if (cameracommand->action == CAMACTION_FOCUSFAR)
      {
         curentry->remembercam.distance = CAMERA_FOCUSFAR_DISTANCE;
         cameracommand->zoomInCloseAsPossible = FALSE;
      }
      else
         curentry->remembercam.distance = targetDistance;
    }

   if (!smGhostMode && (!(tutorial == TUTORIAL_ONLY) || tutEnable.bGameRunning))
   {
       collapseCheckCount++;
       if (collapseCheckCount >= CollapseInterval)
       {
           collapseCheckCount = 0;
           if (ccFocusIsLegalToFocusOn(cameracommand->currentCameraStack))
           {
               collapseLastLegalFocusTime = taskTimeElapsed;
               CameraChase(cameracommand, zoomfactor);
               return;
           }
           if (!kasfTutCameraFocusActive &&
               collapseLastLegalFocusTime >= 0.0f &&
               (taskTimeElapsed - collapseLastLegalFocusTime) > CollapseTimeDelay)
           {
               if (ccFocusOnFleet(cameracommand))
               {
                   cameracommand->dontUseVelocityPredInChase = TRUE;
                   CameraChase(cameracommand, zoomfactor);
                   return;
               }
               smFocusOnMothershipOnClose = TRUE;
               smSensorsBegin(NULL, NULL);
           }
       }
   }

   CameraChase(cameracommand,zoomfactor);
}

void ccLockOnTargetNow(CameraCommand *cameracommand)
{
   real32   targetDistance;
    CameraStackEntry *curentry = currentCameraStackEntry(cameracommand);
   Camera *desired = &curentry->remembercam;

   NewSetFocusPoint(curentry, &targetDistance);

    if (cameracommand->ccMode & CCMODE_FOCUS_CHANGES_LOOKATPOINT_ONLY)
      ;
    else if (!(cameracommand->ccMode & CCMODE_FOCUS_CHANGES_NOTHING))
        cameraSetEyePosition(&curentry->remembercam);

   if(cameracommand->zoomInCloseAsPossible)
   {
        if (cameracommand->action == CAMACTION_FOCUSFAR)
      {
         curentry->remembercam.distance = CAMERA_FOCUSFAR_DISTANCE;
         cameracommand->zoomInCloseAsPossible = FALSE;
      }
      else
         curentry->remembercam.distance = targetDistance;
    }

    cameracommand->actualcamera.lookatpoint = desired->lookatpoint;
    cameracommand->actualcamera.oldlookatpoint = cameracommand->actualcamera.lookatpoint;
    cameracommand->actualcamera.distance = desired->distance;
    cameracommand->actualcamera.angle = desired->angle;
    cameracommand->actualcamera.declination = desired->declination;
    cameracommand->actualcamera.eyeposition = desired->eyeposition;
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusCullRadius
    Description : Cull a selection for focussing based on an arbitrary centre point
    Inputs      : in - selection to cull
                  raduisSqr - Square of the radius from mean that is acceptable.
                  centre - centre point to cull about
    Outputs     : out - where to cull the selection to
    Return      :
----------------------------------------------------------------------------*/
sdword ccFocusCullRadiusGeneral(FocusCommand *out, FocusCommand *in, real32 radiusSqr, vector *centre)
{
    sdword index;
    real32 dx, dy, dz, distanceSqr;

    for (out->numShips = index = 0; index < in->numShips; index++)
    {
        dx = fabsf(in->ShipPtr[index]->posinfo.position.x - centre->x);
        dy = fabsf(in->ShipPtr[index]->posinfo.position.y - centre->y);
        dz = fabsf(in->ShipPtr[index]->posinfo.position.z - centre->z);
        distanceSqr = dx * dx + dy * dy + dz * dz;          //compute distance from centre
        if (distanceSqr <= radiusSqr)
        {                                                   //if this ship close enough to the mean
            out->ShipPtr[out->numShips] = in->ShipPtr[index];
            out->numShips++;                                //add this ship to selection
        }
    }
    return(out->numShips);
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusCullRadiusMean
    Description : Cull out any objects in the specified selection that are more
                    than radius from the mean position.
    Inputs      : selection - selection to cull
                  radius - Square of the radius from mean that is acceptable.
    Outputs     : centre - where to store the calculated centre point.  May be NULL.
    Return      : Number of objects remaining in selection
----------------------------------------------------------------------------*/
sdword ccFocusCullRadiusMean(FocusCommand *selection, real32 radiusSqr, vector *centreDest)
{
    sdword index, iOuter;
    vector centre = {0.0f, 0.0f, 0.0f};
    MaxSelection localSelection;
    sdword maxFocussed, iMaxFocussed;

    if (selection->numShips == 0)
    {                                                       //nothing to focus on
        return 0;
    }

    //get average position
    for (index = 0; index < selection->numShips; index++)
    {
        vecAddTo(centre, selection->ShipPtr[index]->posinfo.position);
    }
    vecMultiplyByScalar(centre, (1.0f / (real32)index));

    //now go through the list and cull out any objects farther from the origin
    //than a specified amount
    ccFocusCullRadiusGeneral((FocusCommand *)&localSelection, selection, radiusSqr, &centre);

    //using the mean didn't work; try to centre around any given ship
    if (localSelection.numShips == 0)
    {                                                       //if everything was culled out
        maxFocussed = 0;
        for (iOuter = 0; iOuter < selection->numShips; iOuter++)
        {
            centre = selection->ShipPtr[iOuter]->posinfo.position;
            ccFocusCullRadiusGeneral((FocusCommand *)&localSelection, selection, radiusSqr, &centre);
            if (localSelection.numShips > maxFocussed)
            {                                               //keep track of the best selection
                maxFocussed = localSelection.numShips;
                iMaxFocussed = iOuter;
            }
        }
        dbgAssert(maxFocussed > 0);                         //one selection combo should have worked
        centre = selection->ShipPtr[iMaxFocussed]->posinfo.position;//use the best selection we found
        ccFocusCullRadiusGeneral((FocusCommand *)&localSelection, selection, radiusSqr, &centre);
    }
    selSelectionCopy((MaxAnySelection *)selection, (MaxAnySelection *)&localSelection);
    if (centreDest != NULL)
    {                                                       //store dest centre point
        *centreDest = centre;
    }
    return(selection->numShips);
}

static real32 ccPilotViewRandomCoordinate(real32 minBound, real32 maxBound)
{
    sdword span = (sdword)(maxBound - minBound);

    if (span <= 0)
    {
        return 0.0f;
    }

    return (real32)(ranRandom(0) % span) - ((real32)span * 0.5f);
}

static void ccPilotViewUpdateMatrix(PilotEffect *effect)
{
    hmatMultiplyHMatByHMat(&effect->viewProjection, &rndProjectionMatrix, &rndCameraMatrix);
}

static void ccPilotViewTransformPoint(vector *dst, hmatrix *matrix, vector *src)
{
    hvector source;
    hvector result;

    vecMakeHVecFromVec(source, *src);
    hmatMultiplyHMatByHVec(&result, matrix, &source);
    vecGrabVecFromHVec(*dst, result);
}

static void ccPilotViewResetEffect(Camera *camera, PilotEffect *effect)
{
    sdword index;

    vecZeroVector(effect->drift);
    ccPilotViewUpdateMatrix(effect);
    effect->driftMagnitude = 0.0f;
    effect->updatedThisFrame = 0;

    if (camera != NULL)
    {
        effect->focusCentre = camera->lookatpoint;
    }
    else
    {
        vecZeroVector(effect->focusCentre);
    }

    if (effect->mode != 0)
    {
        vecSet(effect->minBounds, -4500.0f, -4500.0f, -4500.0f);
        vecSet(effect->maxBounds, 4500.0f, 4500.0f, 4500.0f);
        effect->count = 100;
    }
    else
    {
        vecSet(effect->minBounds, -15000.0f, -15000.0f, -15000.0f);
        vecSet(effect->maxBounds, 15000.0f, 15000.0f, 15000.0f);
        effect->count = 1500;
    }

    if (effect->count > 1500)
    {
        effect->count = 1500;
    }

    for (index = 0; index < effect->count; index++)
    {
        PilotEffectParticle *particle = &effect->particles[index];

        vecSet(particle->position,
               ccPilotViewRandomCoordinate(effect->minBounds.x, effect->maxBounds.x),
               ccPilotViewRandomCoordinate(effect->minBounds.y, effect->maxBounds.y),
               ccPilotViewRandomCoordinate(effect->minBounds.z, effect->maxBounds.z));

        if (camera != NULL)
        {
            vecAddTo(particle->position, camera->eyeposition);
        }

        vecZeroVector(particle->screenPosition);
        if (effect->mode == 0)
        {
            ccPilotViewTransformPoint(&particle->screenPosition, &effect->viewProjection, &particle->position);
        }
    }
}

bool32 ccPilotViewGetFocusCentre(vector *centre)
{
    CameraStackEntry *currentCameraStack = universe.mainCameraCommand.currentCameraStack;
    sdword numShips;
    sdword index;

    vecZeroVector(*centre);
    numShips = currentCameraStack->focus.numShips;
    if (numShips == 0)
    {
        return FALSE;
    }

    for (index = 0; index < numShips; index++)
    {
        Ship *ship = currentCameraStack->focus.ShipPtr[index];

        if (ship->objtype != OBJ_ShipType)
        {
            return FALSE;
        }

        if (((universe.mainCameraCommand.ccMode & CCMODE_PILOT_SHIP) == 0) || numShips != 1)
        {
            CommandToDo *command = ship->command;

            if (command != NULL &&
                command->ordertype.order != COMMAND_MOVE)
            {
                return FALSE;
            }
        }

        if (fabs(ship->posinfo.velocity.x) +
            fabs(ship->posinfo.velocity.y) +
            fabs(ship->posinfo.velocity.z) < 130.0f)
        {
            return FALSE;
        }
        vecAddTo(*centre, ship->posinfo.velocity);
    }

    centre->x /= (real32)numShips;
    centre->y /= (real32)numShips;
    centre->z /= (real32)numShips;
    return TRUE;
}

static void ccPilotViewUpdateEffect(Camera *camera, PilotEffect *effect)
{
    real32 normalScalar;
    real32 t;
    vector centre;
    vector target;
    bool32 haveFocusCentre;

    vecZeroVector(centre);

    if (camera == NULL || effect->updatedThisFrame != 0)
    {
        return;
    }

    effect->updatedThisFrame = 1;
    if (effect->mode == 0)
    {
        return;
    }

    if (universe.unknown1 == 0.0f)
    {
        telldir();
        normalScalar = 1.0f;
    }
    else
    {
        normalScalar = 1.0f / universe.unknown1;
    }
    normalScalar *= effect->speedScale * 0.2f;

    haveFocusCentre = FALSE;
    if ((taskTimeElapsed - universe.mainCameraCommand.transTimeStamp) >= 0.35f &&
        ccPilotViewGetFocusCentre(&centre))
    {
        haveFocusCentre = TRUE;
        vecSet(target, centre.x * -0.1f, centre.y * -0.1f, centre.z * -0.1f);
    }
    else
    {
        vecZeroVector(target);
    }
    vecMultiplyByScalar(target, normalScalar);

    t = universe.unknown1 * 0.8f;
    if (t > 0.9f)
    {
        t = 0.9f;
    }

    EvalCubic(&effect->drift.x, &ccPilotEffectM0X, target.x, t);
    EvalCubic(&effect->drift.y, &ccPilotEffectM0Y, target.y, t);
    EvalCubic(&effect->drift.z, &ccPilotEffectM0Z, target.z, t);
    effect->drift = target;
    effect->focusCentre = camera->lookatpoint;
    effect->driftMagnitude = (real32)fmathSqrtDouble(vecMagnitudeSquared(effect->drift));
}

static void ccPilotViewRenderScreenSpaceEffect(PilotEffect *effect)
{
    sdword index;

    if (effect->speedScale == 0.0f)
    {
        return;
    }

    ccPilotViewUpdateMatrix(effect);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_LINES);

    for (index = 0; index < effect->count; index++)
    {
        PilotEffectParticle *particle = &effect->particles[index];
        vector transformed;
        vector delta;
        real32 alpha;

        ccPilotViewTransformPoint(&transformed, &effect->viewProjection, &particle->position);
        vecMultiplyByScalar(transformed, 0.0001f);
        vecSub(delta, particle->screenPosition, transformed);
        vecMultiplyByScalar(delta, effect->speedScale);
        particle->screenPosition = transformed;

        alpha = (real32)((fabs(delta.x) + fabs(delta.y)) * 60.0);
        glColor4f(0.7f, 0.7f, 0.7f, 0.0f);
        glVertex3f(transformed.x + delta.x + delta.x,
                   transformed.y + delta.y + delta.y,
                   transformed.z + delta.z + delta.z);
        glColor4f(0.7f, 0.7f, 0.7f, alpha);
        glVertex3f(transformed.x + delta.x,
                   transformed.y + delta.y,
                   transformed.z + delta.z);
        glColor4f(0.7f, 0.7f, 0.7f, alpha);
        glVertex3f(transformed.x + delta.x,
                   transformed.y + delta.y,
                   transformed.z + delta.z);
        glColor4f(0.7f, 0.7f, 0.7f, 0.0f);
        glVertex3f(transformed.x, transformed.y, transformed.z);
    }

    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void ccSetMode(PilotEffect *effect, sdword mode)
{
    effect->mode = mode;
    effect->speedScale = 1.0f;
}

void ccSetDirtyBit(PilotEffect *effect)
{
    effect->dirty = 1;
}

void ccPilotViewRenderEffect(Camera *camera, PilotEffect *effect)
{
    sdword index;
    sdword textureEnabled;
    sdword lightingEnabled;

    if (effect->dirty != 0)
    {
        ccPilotViewResetEffect(camera, effect);
        effect->dirty = 0;
    }

    ccPilotViewUpdateEffect(camera, effect);
    if (camera == NULL)
    {
        return;
    }

    effect->updatedThisFrame = 0;
    for (index = 0; index < effect->count; index++)
    {
        PilotEffectParticle *particle = &effect->particles[index];
        real32 minX = effect->minBounds.x + camera->eyeposition.x;
        real32 maxX = effect->maxBounds.x + camera->eyeposition.x;
        real32 minY = effect->minBounds.y + camera->eyeposition.y;
        real32 maxY = effect->maxBounds.y + camera->eyeposition.y;
        real32 minZ = effect->minBounds.z + camera->eyeposition.z;
        real32 maxZ = effect->maxBounds.z + camera->eyeposition.z;
        real32 range;

        range = maxX - minX;
        if (maxX >= particle->position.x)
        {
            if (particle->position.x < minX)
            {
                particle->position.x = floorf((maxX - particle->position.x) / range) * range + particle->position.x;
            }
        }
        else
        {
            particle->position.x -= floorf((particle->position.x - minX) / range) * range;
        }

        range = maxY - minY;
        if (maxY >= particle->position.y)
        {
            if (particle->position.y < minY)
            {
                particle->position.y = floorf((maxY - particle->position.y) / range) * range + particle->position.y;
            }
        }
        else
        {
            particle->position.y -= floorf((particle->position.y - minY) / range) * range;
        }

        range = maxZ - minZ;
        if (maxZ >= particle->position.z)
        {
            if (particle->position.z < minZ)
            {
                particle->position.z = floorf((maxZ - particle->position.z) / range) * range + particle->position.z;
            }
        }
        else
        {
            particle->position.z -= floorf((particle->position.z - minZ) / range) * range;
        }
    }

    textureEnabled = rndTextureEnable(FALSE);
    lightingEnabled = rndLightingEnable(FALSE);
    glPushAttrib(GL_BLEND);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (effect->mode != 0)
    {
        if (effect->driftMagnitude >= 0.5f)
        {
            real32 alpha = effect->driftMagnitude * 0.03f;

            if (alpha > 0.9f)
            {
                alpha = 0.9f;
            }

            glBegin(GL_LINES);
            for (index = 0; index < effect->count; index++)
            {
                PilotEffectParticle *particle = &effect->particles[index];
                vector start;
                vector mid;

                start.x = effect->drift.x + effect->drift.x + particle->position.x;
                start.y = effect->drift.y + effect->drift.y + particle->position.y;
                start.z = effect->drift.z + effect->drift.z + particle->position.z;
                mid.x = effect->drift.x + particle->position.x;
                mid.y = effect->drift.y + particle->position.y;
                mid.z = effect->drift.z + particle->position.z;

                glColor4f(0.7f, 0.7f, 0.7f, 0.0f);
                glVertex3f(start.x, start.y, start.z);
                glColor4f(0.7f, 0.7f, 0.7f, alpha);
                glVertex3f(mid.x, mid.y, mid.z);
                glColor4f(0.7f, 0.7f, 0.7f, alpha);
                glVertex3f(mid.x, mid.y, mid.z);
                glColor4f(0.7f, 0.7f, 0.7f, 0.0f);
                glVertex3f(particle->position.x, particle->position.y, particle->position.z);
            }
            glEnd();
        }
    }
    else
    {
        ccPilotViewRenderScreenSpaceEffect(effect);
    }
    glPopAttrib();
    rndTextureEnable(textureEnabled);
    rndLightingEnable(lightingEnabled);
}




void ccFocusCloseIgnoreZoom(CameraCommand *cameracommand, SelectCommand *focuscom)
{
    ccFocusGeneral(cameracommand, (FocusCommand *)focuscom, TRUE);
    cameracommand->zoomInCloseAsPossible = TRUE;
}

void ccFocusIgnoreZoom(CameraCommand *cameracommand, SelectCommand *focuscom)
{
    ccFocusGeneral(cameracommand, (FocusCommand *)focuscom, FALSE);
    cameracommand->zoomInCloseAsPossible = TRUE;
}

void ccSetDistanceIgnoreZoom(CameraCommand *cameracommand, real32 distance)
{
    cameracommand->currentCameraStack->remembercam.distance = distance;
    cameracommand->actualcamera.distance = distance;
    cameracommand->zoomInCloseAsPossible = TRUE;
    cameracommand->transition = distance;
}

/*-----------------------------------------------------------------------------
    Name        : ccSetDistance
    Description : Set the camera distance without the zoom-ignore side effects.
    Inputs      : cameracommand - camera command block
                  distance      - new distance value
    Return      : void
    Binary      : 0x42E210
----------------------------------------------------------------------------*/
void ccSetDistance(CameraCommand *cameracommand, real32 distance)
{
    cameracommand->currentCameraStack->remembercam.distance = distance;
    cameracommand->actualcamera.distance = distance;
}

/*-----------------------------------------------------------------------------
    Name        : ccFocusCullByViewAngle
    Description : Cull a selection to ships within a squared radius of focusPoint.
                  Despite the name, viewAngle is treated as a maximum squared
                  distance from focusPoint (dx^2+dy^2+dz^2 <= viewAngle).
    Inputs      : out        - output selection (caller-allocated)
                  in         - input selection to filter
                  viewAngle  - maximum squared distance threshold
                  focusPoint - center point
    Return      : number of ships kept in out
    Binary      : 0x42EA50
----------------------------------------------------------------------------*/
sdword ccFocusCullByViewAngle(SelectCommand *out, SelectCommand *in, real32 viewAngle, vector *focusPoint)
{
    sdword i;
    real32 dx, dy, dz;
    Ship *ship;

    out->numShips = 0;
    if (in->numShips <= 0)
        return out->numShips;

    for (i = 0; i < in->numShips; i++)
    {
        ship = in->ShipPtr[i];
        dx = (real32)fabs(ship->posinfo.position.x - focusPoint->x);
        dy = (real32)fabs(ship->posinfo.position.y - focusPoint->y);
        dz = (real32)fabs(ship->posinfo.position.z - focusPoint->z);
        if (dx*dx + dy*dy + dz*dz <= viewAngle)
            out->ShipPtr[out->numShips++] = ship;
    }
    return out->numShips;
}














void ccAdjustDistanceTowardMinimumZoom(CameraCommand *cameracommand, real32 scale)
{
    real32 minZoom = cameraMinimumZoom(&cameracommand->actualcamera);
    real32 zoomDelta = cameracommand->actualcamera.farthestZoom - minZoom;

    if (zoomDelta > 0.0f)
    {
        real32 distance = minZoom + zoomDelta * scale;
        cameracommand->currentCameraStack->remembercam.distance = distance;
        cameracommand->zoomInCloseAsPossible = TRUE;
        cameracommand->actualcamera.distance = distance;
        cameracommand->transition = distance;
    }
}
