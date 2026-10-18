/*=============================================================================
    SPACEOBJ.H: Object-oriented "Space Object" formats

    Created June 1997 by Gary Shaw
=============================================================================*/

#ifndef ___SPACEOBJ_H
#define ___SPACEOBJ_H

#include "Color.h"
#include "clouds.h"
#include "ETG.h"
#include "Globals.h"
#include "LinkedList.h"
#include "LOD.h"
#include "Matrix.h"
#include "MaxMultiplayer.h"
#include "MeshAnim.h"
#include "Nebulae.h"
#include "ObjTypes.h"
#include "SoundStructs.h"
#include "Trails.h"
#include "Vector.h"
#include "Timer.h"

struct Ship;
typedef struct SalvageInfo SalvageInfo;

/*=============================================================================
    Switches:
=============================================================================*/
#ifdef HW_BUILD_FOR_DEBUGGING

#define ISMOVING_MOVING     1
#define ISMOVING_ROTATING   2

#define SO_ERROR_CHECKING       1               //general error checking
#define SO_VERBOSE_LEVEL        2               //print extra info
#define SO_CLOOGE_SCALE         1               //clooge a largeness factor for the ships

#else

#define ISMOVING_MOVING     1
#define ISMOVING_ROTATING   2

#define SO_ERROR_CHECKING       0               //general error checking
#define SO_VERBOSE_LEVEL        0               //print extra info
#define SO_CLOOGE_SCALE         0               //clooge a largeness factor for the ships

#endif


/*-----------------------------------------------------------------------------
    Gun Object
    Note: the gun object is not an independent object like an asteroid or ship.
          It must be part of a ship object.
-----------------------------------------------------------------------------*/
//flags for the gun object
#define GF_MultiLevelMatrix     0x00000001
typedef struct
{
    GunType guntype;
    GunSoundType gunsoundtype;
    BulletType bulletType;      // type of bullet gun fires
    WeaponType startUpgradeLevel;
    WeaponType endUpgradeLevel;
    GunSize gunSize;
    real32 damageMult;
    udword flags;               //flags for the gun
    udword gunindex;            //index of gun

    vector position;            // position relative to ship

    // for GUN_Gimble
    vector gunnormal;           // normal vector describing gun orientation (rel to ship)
    real32 cosminAngleFromNorm; // cos of min angle required between gunnormal and gunheading
    real32 cosmaxAngleFromNorm; // cos of max angle possible between gunnormal and gunheading

    // for GUN_NewGimble
    matrix defaultgunorientation;
    matrix defaultGunOrientationNonConcat;

    real32 minturnangle,maxturnangle;
    real32 mindeclination,maxdeclination;
    real32 maxanglespeed;
    real32 maxdeclinationspeed;
    real32 angletracking;
    real32 declinationtracking;

    // for GUN_MissileLauncher
    sdword maxMissiles;

    real32 triggerHappy;        // cosine of maximum allowed angle allowed before a fixed gun will shoot
    real32 gunDamageLo[NUM_TACTICS_TYPES];          // low damage potential
    real32 gunDamageHi[NUM_TACTICS_TYPES];          // high damage potential
    real32 baseGunDamageLo;     //based values loaded in from script
    real32 baseGunDamageHi;   //dido, only used before game loads to precalculate different damages
    real32 bulletlength;        // length of bullets fired by gun
    real32 bulletlifetime;      // life time of bullet
    real32 bulletspeed;         // speed of bullet (0 indicates infinite speed)
    real32 bulletrange;         // total distance bullet can travel
    real32 bulletmass;          // mass of bullet gun shoots
    real32 firetime;            // minimum time required before gun can fire again
    real32 burstFireTime;       // time in seconds a gun can burst (if burstFireTime == 0, this gun doesn't fire in bursts)
    real32 burstWaitTime;       // time in seconds gun must cool down after a burst
    real32 bulletlifetime2;
    real32 bulletspeed2;
    real32 bulletrange2;
    real32 firetime2;

    real32 barrelLength;        // length of gun barrel for offsetting muzzle flashes
    real32 recoilLength;        // amount of recoil on the gun when it fires

    //additional stuff for use only in the R1 Heavy Cruiser main ion guns
    vector offset;              //offset to rotate about gun's pivot point
    sdword slaveDriver;         //for slaving guns together

} GunStatic;

static_assert(offsetof(GunStatic, startUpgradeLevel) == 0x0C, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, endUpgradeLevel) == 0x10, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, gunSize) == 0x14, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, damageMult) == 0x18, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, cosminAngleFromNorm) == 0x3C, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, cosmaxAngleFromNorm) == 0x40, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, minturnangle) == 0x8C, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, maxMissiles) == 0xAC, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, triggerHappy) == 0xB0, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletlength) == 0xD4, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletlifetime) == 0xD8, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletspeed) == 0xDC, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletrange) == 0xE0, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletmass) == 0xE4, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, firetime) == 0xE8, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, burstFireTime) == 0xEC, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, burstWaitTime) == 0xF0, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletlifetime2) == 0xF4, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletspeed2) == 0xF8, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, bulletrange2) == 0xFC, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, firetime2) == 0x100, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, barrelLength) == 0x104, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, recoilLength) == 0x108, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, offset) == 0x10C, "GunStatic layout mismatch");
static_assert(offsetof(GunStatic, slaveDriver) == 0x118, "GunStatic layout mismatch");
static_assert(sizeof(GunStatic) == 0x11C, "GunStatic size mismatch");

typedef struct
{
    NAVLightType lightType;      // type of nav light
    vector position;             // position relative to ship
    rgbquad color;               // color of nav light
    real32 flashrateon;          // delay in seconds each flash is on.
    real32 flashrateoff;         // delay in seconds each flash is off.
    real32 startdelay;             // time to wait before starting.
    real32 size;                 // size of light in meters.
    udword minLOD;               // min LOD to draw light at.
    char name[20];               // name of nav light
    memsize texturehandle;      // handle to a texture (if any).		// actually trhandle, but not defined here
    ubyte lodBindingIndices[8];  // retail per-LOD mesh binding indices from MEX trailer names
} NAVLightStatic;

static_assert(offsetof(NAVLightStatic, lodBindingIndices) == 0x40, "NAVLightStatic layout mismatch");
static_assert(sizeof(NAVLightStatic) == 0x48, "NAVLightStatic size mismatch");

typedef struct
{
    vector position;            // position relative to ship
    vector normal;              // normal vector (rel to ship)
} DamageLightStatic;

typedef struct
{
    vector position;            // position relative to ship
    vector nozzlenormal;        // normal vector describing nozzle orientation (rel to ship)
} ResNozzleStatic;

typedef struct
{
    vector position;            // position relative to ship
} RepairNozzleStatic;

typedef struct
{
    vector position;            // position relative to ship
    vector nozzlenormal;        // normal vector describing nozzle orientation (rel to ship)
} TractorBeamStatic;

// types for mine formations

#define PULSE_START 0
#define PULSE_SPAWN_EFFECT 1
#define PULSE_MONITOR_EFFECT 2
#define PULSE_REDIRECT 3

typedef struct
{
    Node FormLink;
    LinkedList MineList;
    vector pulse_dir;               //stores pulses current location
    vector effect_going_to;
    real32 waittime;
    struct Player *playerowner;
    bool32 FULL;
    sdword wallstate;
    struct Effect *effect;
    udword numat;
} MineFormationInfo;

typedef struct
{
    WeaponType weaponMode;

    // for GUN_Gimble
    vector gunheading;          // gun heading (relative to ship)

    // for GUN_NewGimble
    matrix curgunorientation;
    matrix curGunOrientationNonConcat;
    real32 angle;
    real32 declination;
    real32 anglespeed;
    real32 declinationspeed;

    // for GUN_MissileLauncher
    sdword numMissiles;

    sdword gimblehandle;

    real32 lasttimefired;       // last time gun was fired
    real32 burstStartTime;      // last time gun started burst
    real32 useBurstFireTime;    // actual burst fire/wait times to use after modified (optimization)
    real32 useBurstWaitTime;
    udword burstState;          // gun burst state variable
    GunStatic *gunstatic;       // pointer to static information
} Gun;

static_assert(offsetof(Gun, weaponMode) == 0x00, "Gun layout mismatch");
static_assert(offsetof(Gun, gunheading) == 0x04, "Gun layout mismatch");
static_assert(offsetof(Gun, numMissiles) == 0x68, "Gun missile count offset mismatch");
static_assert(offsetof(Gun, lasttimefired) == 0x70, "Gun layout mismatch");
static_assert(offsetof(Gun, burstState) == 0x80, "Gun layout mismatch");
static_assert(offsetof(Gun, gunstatic) == 0x84, "Gun layout mismatch");
static_assert(sizeof(Gun) == 0x88, "Gun size mismatch");

typedef struct
{
    udword lightstate;
    real32 lastTimeFlashed;
    NAVLightStatic *navlightstatic;
} NAVLight;

struct madanim;
struct shipbindings;

typedef struct
{
    udword madOwnedBindings;    // unk_4B8 - current active bindings pointer (raw udword for legacy code)
    udword madAnimationData;    // unk_4BC - madanim pointer (raw udword for legacy code)
    udword runtimeController;  // unk_4C0
} ShipRuntimeAnimationOverlay;

static_assert(offsetof(ShipRuntimeAnimationOverlay, madOwnedBindings) == 0x00);
static_assert(offsetof(ShipRuntimeAnimationOverlay, madAnimationData) == 0x04);
static_assert(offsetof(ShipRuntimeAnimationOverlay, runtimeController) == 0x08);

typedef struct
{
    udword state;
    real32 phaseStartTime;
    real32 phaseEndTime;
    real32 phaseScale;
    union
    {
        udword chainHitCount;
        void *repairDroidController; // stale legacy alias; retail repair-droid controllers live at Ship+0x4C0
    };
    real32 activeUntil;
} ShipStatusWindowOverlay;

typedef struct
{
    struct Ship *sourceShip;
    union
    {
        struct Player *sourcePlayer;
        udword raw_4EC; // overloaded with non-Beast timer scratch in the EMP/disable path
    };
    udword raw_4F0;           // overloaded tail word: SalCap realtime scratch, hyperspace speech scratch, BigGun sim-time scratch
    udword raw_4F4;           // overloaded tail word: BigGun cascade budget, SalCap reassignment time, hyperspace speech timing
    real32 nextChainTime;
    struct Effect *effect;
    udword effectColorScheme;
} ShipBeastAttackOverlay;

typedef struct
{
    udword active;
    udword visualsEnabled;
    vector target;
    Timer timer;
    struct Effect *effect;
} ShipCrystalProcessingOverlay;

static_assert(sizeof(ShipCrystalProcessingOverlay) == 0x28, "ShipCrystalProcessingOverlay size mismatch");

typedef struct
{
    union
    {
        udword words[13]; // 0x1DC..0x20C retail shipApplyEMPShock / delayed-death scan window
        struct
        {
            ubyte unk_00_0F[0x10];
            void *busyStatePtr1EC;   // retail only proves this as a busy-state window word
            union
            {
                struct Ship *dockingship;
                udword sentinelFormationState;
            };
            ubyte unk_18_2F[0x18];
        };
    };
} ShipCataBusyStateWindow;

static_assert(sizeof(ShipCataBusyStateWindow) == 0x34, "ShipCataBusyStateWindow size mismatch");
static_assert(offsetof(ShipCataBusyStateWindow, busyStatePtr1EC) == 0x10, "ShipCataBusyStateWindow busyStatePtr1EC offset mismatch");
static_assert(offsetof(ShipCataBusyStateWindow, dockingship) == 0x14, "ShipCataBusyStateWindow dockingship offset mismatch");

typedef struct
{
    ubyte unk_00;
    ubyte unk_01;
    ubyte unk_02_03[2];
    real32 effectStartTime;   // 0x04
    ubyte unk_08;
    ubyte unk_09_33[0x2B];
    udword unk_34;
    udword unk_38;
    udword unk_3C;
    udword unk_40;
    udword unk_44;
} WorkerShipSpecifics;

typedef struct
{
    struct Ship *receiverShip;
    sdword state;
    sdword unk_08;
    sdword unk_0C;
    vector primaryDirection;
    vector secondaryDirection;
} ShipLaunchInitSpecificsOverlay;

typedef struct
{
    udword words[12];
} ShipDamageLevelCounters;

static_assert(sizeof(ShipDamageLevelCounters) == 0x30, "ShipDamageLevelCounters size mismatch");

typedef struct
{
    real32 lastFireTime;   // 0x00 - shared attack helper cadence timestamp
    ubyte unk_04_15[0x12];
    uword currentGunIndex; // 0x16 - shared attack helper rotating gun index
} GunStateSpecifics;

static_assert(offsetof(GunStateSpecifics, lastFireTime) == 0x00, "GunStateSpecifics lastFireTime offset mismatch");
static_assert(offsetof(GunStateSpecifics, currentGunIndex) == 0x16, "GunStateSpecifics currentGunIndex offset mismatch");
static_assert(sizeof(GunStateSpecifics) == 0x18, "GunStateSpecifics size mismatch");

typedef GunStateSpecifics LinkableSpecifics;   // ShipSpecifics+0x00 head shared with gun-state (see gunStateSpecifics());
                                                // the linked-fission relink timer lives at Ship::linkTransformTimer
                                                // (Ship+0x624), confirmed via retail disassembly (lea ecx/edx,[reg+624h]
                                                // in processLinkedLaunchToDo), not in this ShipSpecifics overlay

static_assert(sizeof(LinkableSpecifics) == 0x18, "LinkableSpecifics size mismatch");

typedef struct
{
    LinkableSpecifics linkable;             // 0x00 - gun-state head (currentGunIndex/lastFireTime); see gunStateSpecifics()
    struct Ship *linkPartner;                // 0x18 - retail link partner / pending link target pointer
    sdword linkState;                        // 0x1C - retail link command state; AcolyteRetargetAttack/CanHalt clear it
    sdword linkStateUnk20;                   // 0x20 - preserved retail slot in the link command block
    sdword linkAnimationIndex;               // 0x24 - active link animation selector
    ubyte unk_28_3B[0x14];                   // 0x28
    struct Effect *linkEffect;               // 0x3C - transient effect dropped in PreFix
    ubyte linkAnimationWanted;               // 0x40
    ubyte linkAnimationDelayArmed;           // 0x41
    ubyte unk_42_43[2];                      // 0x42
    real32 linkAnimationDelayUntil;          // 0x44
    udword linkAnimationState;               // 0x48
    struct SpaceObjRotImpTarg *linkTarget;   // 0x4C - selected special target
    sdword linkShotPending;                  // 0x50
    sdword linkedGroupAttackPending;         // 0x54
    udword unk_58;                           // 0x58
} AcolyteSpecifics;

static_assert(offsetof(AcolyteSpecifics, linkable) == 0x00, "AcolyteSpecifics linkable offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkPartner) == 0x18, "AcolyteSpecifics linkPartner offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkState) == 0x1C, "AcolyteSpecifics linkState offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkStateUnk20) == 0x20, "AcolyteSpecifics linkStateUnk20 offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkAnimationIndex) == 0x24, "AcolyteSpecifics linkAnimationIndex offset mismatch");
static_assert(offsetof(AcolyteSpecifics, unk_28_3B) == 0x28, "AcolyteSpecifics unk_28_3B offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkEffect) == 0x3C, "AcolyteSpecifics linkEffect offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkAnimationWanted) == 0x40, "AcolyteSpecifics linkAnimationWanted offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkAnimationDelayArmed) == 0x41, "AcolyteSpecifics linkAnimationDelayArmed offset mismatch");
static_assert(offsetof(AcolyteSpecifics, unk_42_43) == 0x42, "AcolyteSpecifics unk_42_43 offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkAnimationDelayUntil) == 0x44, "AcolyteSpecifics linkAnimationDelayUntil offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkAnimationState) == 0x48, "AcolyteSpecifics linkAnimationState offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkTarget) == 0x4C, "AcolyteSpecifics linkTarget offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkShotPending) == 0x50, "AcolyteSpecifics linkShotPending offset mismatch");
static_assert(offsetof(AcolyteSpecifics, linkedGroupAttackPending) == 0x54, "AcolyteSpecifics linkedGroupAttackPending offset mismatch");
static_assert(offsetof(AcolyteSpecifics, unk_58) == 0x58, "AcolyteSpecifics unk_58 offset mismatch");
static_assert(sizeof(AcolyteSpecifics) == 0x5C, "AcolyteSpecifics size mismatch");

typedef struct
{
    ubyte state;                      // 0x00 - retail leech special state machine state
    ubyte activeMode;                 // 0x01 - currently selected leech mode
    ubyte requestedMode;              // 0x02 - queued leech mode
    ubyte animationState;             // 0x03 - clamp/open animation latch
    struct Ship *currentTarget;       // 0x04 - active leech target
    void *targetList;                 // 0x08 - filtered special target list (MaxSelection *)
    sdword claimedHardpointIndex;     // 0x0C - claimed target hardpoint, -1 when none
    real32 clampDistance;             // 0x10 - current standoff distance while approaching
    real32 nextModeEffectTime;        // 0x14 - next delayed mode-effect retry time
    real32 nextDrainTime;             // 0x18 - next damage/resource drain tick
    ubyte unk_1C_3B[0x20];
    ubyte dockRequested;              // 0x3C - issue a return-to-dock command in housekeep
    ubyte dockingIssued;              // 0x3D - retail dock command already issued
    ubyte preserveDockingState;       // 0x3E - retain the docked/returning latch
    ubyte reclampRequested;           // 0x3F - reattach clamp to the current target
    ubyte removeDoingStuffRequested;  // 0x40 - deferred RemoveShipsFromDoingStuff request
    ubyte unk_41_43[0x03];
    real32 feedbackTimer;             // 0x44 - retail delayed feedback / saved-target timer
    struct Ship *savedTarget;         // 0x48 - registry-saved target pointer
} LeechSpecifics;

static_assert(sizeof(LeechSpecifics) == 0x4C);

typedef struct
{
    GunStateSpecifics gunState;
    ubyte unk_18_13B[0x124];
} MCVSpecifics;

typedef struct
{
    GunStateSpecifics gunState;
    ubyte unk_18_D67[0xD50];
} DreadnoughtSpecifics;

typedef struct
{
    ubyte ramState;                  // 0x00
    ubyte unk_01;                    // 0x01 - no proven retail readers/writers yet
    ubyte unk_02;                    // 0x02 - no proven retail readers/writers yet
    ubyte pad_03;                    // 0x03
    ubyte unk_04_0F[0x0C];           // 0x04 - no proven retail readers/writers yet
    udword unk_10;                   // 0x10 - retail state-2 -> state-3 transition clears this
    struct SpaceObj *target;         // 0x14 - save/fix uses SpaceObjRegistryGetID/GetShip
    udword unk_18;                   // 0x18 - no proven retail readers/writers yet
    udword unk_1C;                   // 0x1C - retail init writes 0x12C; no proven reader yet
    Timer clampTimer;                // 0x20
    vector clampTrackVector;         // 0x30
    struct Effect *effect;           // 0x3C - retail trail/impact effect pointer at Ship+0x674
    udword unk_40;                   // 0x40 - retail init clears it; no proven reader yet
    Timer counterTimer;              // 0x44
    Timer headingResetTimer;         // 0x54 - retail state-3 reset timer
    Timer impactEffectTimer;         // 0x64 - retail state-5 impact-effect timer
    sdword clampDamageStarted;       // 0x74 - retail state machine uses this as the clamp-damage latch
    bool32 targetIsHostile;          // 0x78
    Timer lineOfSightTimer;          // 0x7C - retail line-of-sight retry timer
    ubyte unk_8C_A3[0x18];           // 0x8C - no proven retail readers/writers yet
} RammingFrigateSpecifics;

static_assert(offsetof(RammingFrigateSpecifics, ramState) == 0x00);
static_assert(offsetof(RammingFrigateSpecifics, unk_01) == 0x01);
static_assert(offsetof(RammingFrigateSpecifics, unk_02) == 0x02);
static_assert(offsetof(RammingFrigateSpecifics, pad_03) == 0x03);
static_assert(offsetof(RammingFrigateSpecifics, unk_04_0F) == 0x04);
static_assert(offsetof(RammingFrigateSpecifics, unk_10) == 0x10);
static_assert(offsetof(RammingFrigateSpecifics, target) == 0x14);
static_assert(offsetof(RammingFrigateSpecifics, unk_18) == 0x18);
static_assert(offsetof(RammingFrigateSpecifics, unk_1C) == 0x1C);
static_assert(offsetof(RammingFrigateSpecifics, clampTimer) == 0x20);
static_assert(offsetof(RammingFrigateSpecifics, clampTrackVector) == 0x30);
static_assert(offsetof(RammingFrigateSpecifics, effect) == 0x3C);
static_assert(offsetof(RammingFrigateSpecifics, unk_40) == 0x40);
static_assert(offsetof(RammingFrigateSpecifics, counterTimer) == 0x44);
static_assert(offsetof(RammingFrigateSpecifics, headingResetTimer) == 0x54);
static_assert(offsetof(RammingFrigateSpecifics, impactEffectTimer) == 0x64);
static_assert(offsetof(RammingFrigateSpecifics, clampDamageStarted) == 0x74);
static_assert(offsetof(RammingFrigateSpecifics, targetIsHostile) == 0x78);
static_assert(offsetof(RammingFrigateSpecifics, lineOfSightTimer) == 0x7C);
static_assert(offsetof(RammingFrigateSpecifics, unk_8C_A3) == 0x8C);
static_assert(sizeof(RammingFrigateSpecifics) == 0xA4);

typedef struct
{
    ubyte unk[0x8];
} RepairDroidSpecifics;

typedef struct DockStaticPoint DockStaticPoint;

typedef struct
{
    struct Ship *sourceShip;            // 0x00
    DockStaticPoint *dockStaticPoint;   // 0x04
    struct Ship *targetShip;            // 0x08
    ubyte retargetTimer[0x10];          // 0x0C
    struct Effect *effect;              // 0x1C
} ProcessorTendrilEntry;

typedef struct
{
    ubyte unk_000_143[0x144];
    ProcessorTendrilEntry tendrils[4];  // 0x144
    sdword tendrilCount;                // 0x1C4
} ProcessorSpecifics;

typedef struct
{
    udword unk0;
    real32 forcefieldChargeTimer;
    sdword formationLinkCount;
    struct Ship *formationLinks[10];
    ubyte unk_34_32B[0x2F8];
    sdword formationLevel;
    real32 forcefieldPulseStartTime;
    real32 forcefieldPulseScale;
    udword unk338;
    real32 forcefieldArmor;
    sdword unk340;
    sdword unk344;
    sdword unk348;
    bool32 forcefieldShipInPosition;
} SentinelSpecifics;

typedef struct
{
    udword launchState;                          // HiveLaunch_* state machine (see HiveFrigate.h)
    struct Ship *swarmers[6];                    // HIVE_MAX_SWARMERS swarmer drone slots (NULL = empty)
    sdword swarmerCount;                         // number of currently live swarmer drones
    struct SpaceObjRotImpTarg *targetList[1000]; // ordered target list from last SpecialTarget call
    sdword targetCount;                          // number of valid entries in targetList
    struct SpaceObjRotImpTarg *currentTarget;    // primary target for swarmer dispatch
    sdword specialState;                         // approach/dock sub-state (0=none, 1=approaching, 2=dock-pending)
    struct Ship *dockReceiver;                   // dock destination when specialState==2
    ubyte madState;                              // unused padding (was madAnimationStart tracker; now ship->attackvars.attackevents)
    ubyte flyState;                              // unused padding
    ubyte busyFlags;                             // unused padding (was busy flag; now ship->specialFlags1C8)
    ubyte unkFD7;                                // unused padding
} HiveFrigateSpecifics;

typedef struct
{
    real32 attackSideStepTime;       // 0x00 - retail SharedSwarmerInit/SharedSideStepAttackCommand
    udword attackSideStepDirection;  // 0x04 - retail SharedSwarmerInit/SharedSideStepAttackCommand
    udword unk08;                    // 0x08 - initialized/used by unreconstructed retail swarmer paths
    struct Ship *trackedTarget;      // 0x0C - retail SharedSwarmer_PreFix/Fix
    udword unk10;                    // 0x10 - retail SharedSwarmerInit clears this
    ubyte unk14_1F[0x0C];            // 0x14..0x1F - serialized in retail saves, semantics unresolved
} SwarmerSpecifics;

typedef struct
{
    real32 attackSideStepTime;
    udword attackSideStepDirection;
    sdword miningState;
    udword dropcount;
    vector attackVector;
    real32 attackTimer;
    udword mineFormationActive;
    vector formationPosition;
    vector formationUp;
    vector formationRight;
    vector formationLeft;
    vector formationDown;
    vector formationHeading;
    sdword mineDropNumber;
    sdword miningSideMax;
    sdword miningSideCount;
    MineFormationInfo *mineFormationInfo;
    sdword formationNumberX;
    sdword formationNumberY;
    real32 lastMineRegenerateTime;
} BeastSwarmerSpecifics;

typedef struct
{
    ubyte unk[0x4];
} CruiseMissileSpecifics;

typedef struct
{
    ubyte unk[0x4];
} CargoBargeSpecifics;

typedef struct
{
    sdword postConsumeActive;
    sdword postConsumeShortScatter;
    udword cachedNaggarokEmpDuration;
    Timer postConsumeTimer;
    vector postConsumeTarget;
    ubyte state;
    ubyte pad_29[3];
    struct Ship *targetCrystal;
    vector crystalPosition;
    vector approachPosition;
    vector approachHeading;
    vector targetDelta;
    real32 targetDistance;
    real32 phaseStartTime;
    real32 phaseEndTime;
    udword processedCrystalCount;
    real32 reacquireTime;
    sdword unk_74;
    sdword unk_78;
    sdword unk_7C;
    real32 processingEffectDistanceLimit;
    real32 processingTargetBiasLimit;
    sdword unk_88;
    sdword arrivalDamageActive;
    vector arrivalDamageTarget;
    real32 arrivalDamageRadiusSq;
    ubyte unk_A0[0x0C];
    Timer arrivalDamageTimer;
    sdword headingTimerArmed;
    Timer headingTimer;
} RKOSpecifics;

static_assert(sizeof(RKOSpecifics) == 0xD0, "RKOSpecifics size mismatch");
static_assert(offsetof(RKOSpecifics, postConsumeTimer) == 0x0C, "RKOSpecifics postConsumeTimer offset mismatch");
static_assert(offsetof(RKOSpecifics, postConsumeTarget) == 0x1C, "RKOSpecifics postConsumeTarget offset mismatch");
static_assert(offsetof(RKOSpecifics, state) == 0x28, "RKOSpecifics state offset mismatch");
static_assert(offsetof(RKOSpecifics, targetCrystal) == 0x2C, "RKOSpecifics targetCrystal offset mismatch");
static_assert(offsetof(RKOSpecifics, crystalPosition) == 0x30, "RKOSpecifics crystalPosition offset mismatch");
static_assert(offsetof(RKOSpecifics, approachPosition) == 0x3C, "RKOSpecifics approachPosition offset mismatch");
static_assert(offsetof(RKOSpecifics, approachHeading) == 0x48, "RKOSpecifics approachHeading offset mismatch");
static_assert(offsetof(RKOSpecifics, targetDelta) == 0x54, "RKOSpecifics targetDelta offset mismatch");
static_assert(offsetof(RKOSpecifics, targetDistance) == 0x60, "RKOSpecifics targetDistance offset mismatch");
static_assert(offsetof(RKOSpecifics, processedCrystalCount) == 0x6C, "RKOSpecifics processedCrystalCount offset mismatch");
static_assert(offsetof(RKOSpecifics, reacquireTime) == 0x70, "RKOSpecifics reacquireTime offset mismatch");
static_assert(offsetof(RKOSpecifics, arrivalDamageActive) == 0x8C, "RKOSpecifics arrivalDamageActive offset mismatch");
static_assert(offsetof(RKOSpecifics, arrivalDamageTarget) == 0x90, "RKOSpecifics arrivalDamageTarget offset mismatch");
static_assert(offsetof(RKOSpecifics, arrivalDamageRadiusSq) == 0x9C, "RKOSpecifics arrivalDamageRadiusSq offset mismatch");
static_assert(offsetof(RKOSpecifics, arrivalDamageTimer) == 0xAC, "RKOSpecifics arrivalDamageTimer offset mismatch");
static_assert(offsetof(RKOSpecifics, headingTimerArmed) == 0xBC, "RKOSpecifics headingTimerArmed offset mismatch");
static_assert(offsetof(RKOSpecifics, headingTimer) == 0xC0, "RKOSpecifics headingTimer offset mismatch");

typedef struct
{
    ubyte unk[0x1C];
} NomadMoonSpecifics;

typedef struct
{
    ubyte unk[0x4];
} NomadFieldEmitterSpecifics;

typedef struct
{
    struct Ship *currentTarget;
    LinkedList queuedTargets;
    udword unk_644;
    udword unk_648;
} ShipBeastCascadeWeaponState;

static_assert(sizeof(AcolyteSpecifics) == 0x5C, "AcolyteSpecifics size mismatch");
static_assert(sizeof(LeechSpecifics) == 0x4C, "LeechSpecifics size mismatch");
static_assert(sizeof(MCVSpecifics) == 0x13C, "MCVSpecifics size mismatch");
static_assert(sizeof(DreadnoughtSpecifics) == 0xD68, "DreadnoughtSpecifics size mismatch");
static_assert(sizeof(RammingFrigateSpecifics) == 0xA4, "RammingFrigateSpecifics size mismatch");
static_assert(sizeof(RepairDroidSpecifics) == 0x8, "RepairDroidSpecifics size mismatch");
static_assert(sizeof(ProcessorTendrilEntry) == 0x20, "ProcessorTendrilEntry size mismatch");
static_assert(offsetof(ProcessorSpecifics, tendrils) == 0x144, "ProcessorSpecifics tendrils offset mismatch");
static_assert(offsetof(ProcessorSpecifics, tendrilCount) == 0x1C4, "ProcessorSpecifics tendrilCount offset mismatch");
static_assert(sizeof(ProcessorSpecifics) == 0x1C8, "ProcessorSpecifics size mismatch");
static_assert(sizeof(SentinelSpecifics) == 0x350, "SentinelSpecifics size mismatch");
static_assert(sizeof(HiveFrigateSpecifics) == 0xFD4, "HiveFrigateSpecifics size mismatch");
static_assert(offsetof(SwarmerSpecifics, attackSideStepTime) == 0x00, "SwarmerSpecifics attackSideStepTime offset mismatch");
static_assert(offsetof(SwarmerSpecifics, attackSideStepDirection) == 0x04, "SwarmerSpecifics attackSideStepDirection offset mismatch");
static_assert(offsetof(SwarmerSpecifics, trackedTarget) == 0x0C, "SwarmerSpecifics trackedTarget offset mismatch");
static_assert(offsetof(SwarmerSpecifics, unk10) == 0x10, "SwarmerSpecifics unk10 offset mismatch");
static_assert(sizeof(SwarmerSpecifics) == 0x20, "SwarmerSpecifics size mismatch");
static_assert(sizeof(BeastSwarmerSpecifics) == 0x88, "BeastSwarmerSpecifics size mismatch");
static_assert(sizeof(CruiseMissileSpecifics) == 0x4, "CruiseMissileSpecifics size mismatch");
static_assert(sizeof(CargoBargeSpecifics) == 0x4, "CargoBargeSpecifics size mismatch");
static_assert(sizeof(RKOSpecifics) == 0xD0, "RKOSpecifics size mismatch");
static_assert(sizeof(NomadMoonSpecifics) == 0x1C, "NomadMoonSpecifics size mismatch");
static_assert(sizeof(NomadFieldEmitterSpecifics) == 0x4, "NomadFieldEmitterSpecifics size mismatch");

typedef struct
{
    sdword numGuns;
    Gun guns[1];
} GunInfo;

typedef struct
{
    sdword numLights;
    NAVLight navLights[1];
} NAVLightInfo;

typedef struct
{
    sdword numGuns;
    GunStatic gunstatics[1];
} GunStaticInfo;

typedef struct
{
    sdword numNAVLights;
    NAVLightStatic navlightstatics[1];
} NAVLightStaticInfo;

/*  -------------------------------------------------------
    Static Salvaging Info
    - used to define latch on points for salvage
      capturing vessels
    ------------------------------------------------------- */

//maximum number of ships that can be captured and waiting inside
//a mothership or carrier at any one time to pop out after being
//retrofitted
#define SALVAGE_MAX_CAPTURABLE  50

typedef struct
{
    udword number;
    SalvagePointType type;
    vector position;
    vector conenormal;
    real32 coneangle;
    char name[20];
}SalvageStaticPoint;

typedef struct
{
    sdword numSalvagePoints;
    sdword numNeededForSalvage;
    sdword needBigR1;
    sdword needBigR2;
    sdword willFitCarrier;
    SalvageStaticPoint headingPoint;
    SalvageStaticPoint salvageStaticPoints[1];   //ragged array
} SalvageStaticInfo;

//Non static salvage Information
typedef struct
{
    sdword busy;
    SalvageStaticPoint *salvageStaticPoint;
}SalvagePoint;

typedef struct SalvageInfo
{
    sdword numSalvagePoints;
    sdword numSalvagePointsFree;
    sdword numNeededForSalvage;
    SalvagePoint salvagePoints[1];   //ragged array
} SalvageInfo;

#define sizeofSalvageStaticInfo(x)  sizeof(SalvageStaticPoint)*(x-1)+sizeof(SalvageStaticInfo)
#define sizeofSalvageInfo(x)  sizeof(SalvagePoint)*(x) +sizeof(SalvageInfo)

/*-----------------------------------------------------------------------------
    Static Docking Information
    - not an independent object, but part of ShipStaticInfo
-----------------------------------------------------------------------------*/
#define LIGHTNAME_LENGTH    20
typedef struct
{
  ShipType shiptype;
  ShipRace shiprace;
  sword useNewOrientation;
  uword heading;
  uword up;
  sword useNewOffset;
  vector offset;    //offset modification
  bool32 lightNameUsed;
  char lightName[LIGHTNAME_LENGTH];
} DockStaticOveride;

typedef struct
{
  sdword numDockOverides;
  DockStaticOveride dockOverides[1];
} DockOverideInfo;


typedef struct DockStaticPoint
{
    DockPointType type;
    sdword dockindex;
    vector position;
    vector conenormal;
    real32 coneangle;
    real32 flyawaydist;
    real32 mindist;
    real32 maxdist;
    udword headingdirection;
    udword updirection;
    char name[LIGHTNAME_LENGTH];
} DockStaticPoint;

typedef struct
{
    real32 jackRetractTime;               // 0x00
    DockStaticPoint *grinderDockPoint;    // 0x04
    DockStaticPoint *jackhammerDockPoint; // 0x08 ("gun0")
    udword hostQueueScriptData;           // 0x0C
    DockStaticPoint *hostQueueDockPoint;  // 0x10 ("Center")
    real32 repairDistance;                // 0x14
    real32 repairCone;                    // 0x18
    real32 repairBeamWidth;               // 0x1C
    real32 repairConeCosHalfAngle;        // 0x20
    vector jackhammerPosition;            // 0x24
} WorkerHarvestStaticInfo;

#define CATA_DOCK_TYPE_TABLE_SIZE NUM_DOCK_POINT_TYPES

typedef struct
{
    ubyte firstPointOfType[CATA_DOCK_TYPE_TABLE_SIZE];
    ubyte numPointsOfType[CATA_DOCK_TYPE_TABLE_SIZE];
    ubyte pad_46[2];
    sdword numDockPoints;
    udword pad_4C;
    DockStaticPoint dockstaticpoints[1];
} DockStaticInfo;
// Binary: header is 80 bytes (0x50), first DockStaticPoint at 0x50, sizeof(DockStaticPoint) == 76, sizeof(DockStaticInfo) == 156
static_assert(sizeof(DockStaticPoint) == 76);
static_assert(offsetof(DockStaticInfo, numDockPoints) == 0x48);
static_assert(offsetof(DockStaticInfo, dockstaticpoints) == 0x50);

/*-----------------------------------------------------------------------------
    Docking Information
    - not an independent object, but part of Ship
-----------------------------------------------------------------------------*/

typedef struct
{
    DockStaticPoint *dockstaticpoint;
    udword thisDockBusy;
} DockPoint;

typedef struct
{
    sdword busyness;
    sdword numDockPoints;
    DockPoint dockpoints[1];
} DockInfo;

typedef struct
{
    Node node;
    struct Ship *ship;
} InsideShip;

typedef struct
{
    LinkedList insideList;
    sword      FightersInsideme;
    sword      CorvettesInsideme;
} ShipsInsideMe;

//CLAMPING INFO

typedef struct
{
    struct SpaceObjRotImpTarg *host;
    matrix clampCoordsys;
    vector clampOffset;
    void *selection;
}ClampInfo;

/*=============================================================================
    Object Static Types
=============================================================================*/

/*=============================================================================
    Basic SpaceObjStaticInfo type, from which all StaticInfo objects are derived
=============================================================================*/

typedef struct
{
    real32 approxcollspheresize;    // rough collision sphere size more repesentative of object's actual size, rather
                                    // than worst case size used for trivial rejection
    real32 avoidcollspheresize;     // collision sphere size used for collision avoidance
    real32 avoidcollspherepad;      // collision sphere size used for collision avoidance, with padding

    real32 approxcollmodifier;    // approxcollspheresize = approxcollmodifier * collspheresize
    real32 avoidcollmodifier;     // avoidcollspheresize = avoidcollmodifier * collspheresize

    real32 originalcollspheresize;  // original collspheresize DO NOT CHANGE THIS IF OBJECT SCALES!
    real32 collspheresize;  // bounding sphere for rough collision detection
    real32 collspheresizeSqr; // square of collspheresize precomputed
    vector collsphereoffset;// bounding sphere offset in object co-ordinates

    vector collrectoffset;  // collision rectangle offset in object co-ordinates (leftdownback corner of cube)
    real32 collRectFarHalf[3]; // precomputed (far_corner[i]) / 2 for each axis; set by mexSetCollisionInfo
    real32 uplength;        // length of rectangle in up direction
    real32 rightlength;     // length of rectangle in right direction
    real32 forwardlength;   // length of rectangle in forward direction
    real32 diagonallength;  // length of rectangle from point 0 to point 6 or rectangle
    udword preciseSelection; //object uses precise selection mechanism if LOD <= this level

    //dimensional scale factors
    real32 collBoxScaleForward;
    real32 collBoxScaleUp;
    real32 collBoxScaleRight;

    real32 collBoxOffsetX;
    real32 collBoxOffsetY;
    real32 collBoxOffsetZ;
} StaticCollInfo;

static_assert(offsetof(StaticCollInfo, collRectFarHalf) == 56, "StaticCollInfo::collRectFarHalf offset mismatch with Cataclysm binary");
static_assert(offsetof(StaticCollInfo, uplength) == 68, "StaticCollInfo::uplength offset mismatch with Cataclysm binary");
static_assert(sizeof(StaticCollInfo) == 112, "StaticCollInfo size mismatch with Cataclysm binary");

#define IF_InfoNeeded           0x01
#define IF_InfoLoaded           0x02
#define IF_InfoWillBeNeeded     0x04
#define IF_NotPostLoaded        0x08

typedef struct
{
    lodinfo *LOD;           // Level-of-detail information
    void *pMexData;         // pointer to MEX data (mesh extensions)
    real32 mass;            // object mass (can be negative for neat effects)
    real32 momentOfInertiaX;// moment of inertia of object about X axis
    real32 momentOfInertiaY;// moment of inertia of object about Y axis
    real32 momentOfInertiaZ;// moment of inertia of object about Z axis
    real32 oneOverMass;            // object mass (can be negative for neat effects)
    real32 oneOverMomentOfInertiaX;// moment of inertia of object about X axis
    real32 oneOverMomentOfInertiaY;// moment of inertia of object about Y axis
    real32 oneOverMomentOfInertiaZ;// moment of inertia of object about Z axis
    real32 maxvelocity;     // maximum velocity of object
    real32 maxrot;          // maximum rotate speed (rad/s) of object
//    bool8  infoNeeded;      //static info is needed for this level and should be loaded
//    bool8  infoLoaded;      //static info has been loaded properly
    ubyte  infoFlags;       //IF_InfoNeeded, IF_InfoLoaded, IF_InfoWillBeNeeded
    bool8  immobile;        // flag indicating object is immobile
    sbyte  rightOfWay;
    StaticCollInfo staticCollInfo;
} StaticHeader;

typedef struct
{
    StaticHeader staticheader;
} StaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
} StaticInfoHealth;

#define TRANS_UP                0
#define TRANS_DOWN              1
#define TRANS_RIGHT             2
#define TRANS_LEFT              3
#define TRANS_FORWARD           4
#define TRANS_BACKWARD          5
#define NUM_TRANS_DEGOFFREEDOM  6
#define TRANS_BACK              TRANS_BACKWARD

#define ROT_ABOUTXCCW           0
#define ROT_ABOUTXCW            1
#define ROT_ABOUTYCCW           2
#define ROT_ABOUTYCW            3
#define ROT_ABOUTZCCW           4
#define ROT_ABOUTZCW            5
#define NUM_ROT_DEGOFFREEDOM    6

#define ROT_YAWLEFT             ROT_ABOUTXCCW
#define ROT_YAWRIGHT            ROT_ABOUTXCW
#define ROT_PITCHUP             ROT_ABOUTYCCW
#define ROT_PITCHDOWN           ROT_ABOUTYCW
#define ROT_ROLLRIGHT           ROT_ABOUTZCCW
#define ROT_ROLLLEFT            ROT_ABOUTZCW

#define TURN_ABOUTX             0
#define TURN_ABOUTY             1
#define TURN_ABOUTZ             2
#define NUM_TURN_TYPES          3

#define TURN_YAW                TURN_ABOUTX
#define TURN_PITCH              TURN_ABOUTY
#define TURN_ROLL               TURN_ABOUTZ

#define MAX_NUM_TRAILS          4

typedef struct StaticInfoHealthGuidance
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];
} StaticInfoHealthGuidance;

typedef struct StaticInfoHealthGuidanceShipDerelict
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];

    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be
    SalvageStaticInfo   *salvageStaticInfo;           //static info for salvageable objects

    //these 3 variables must be next
    real32 sinbank;             // variables used by the AIShip routines
    real32 pitchturn;
    real32 pitchdescend;
} StaticInfoHealthGuidanceShipDerelict;

//info for SpaceObjRotImpTarg objects that was once
//static, but now needs to be non-static
typedef struct
{
    real32 thruststrength[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrength[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeed[NUM_TURN_TYPES];
} NonStats;

/*=============================================================================
    Ship StaticInfo
=============================================================================*/

#define THISSHIPIS_FIGHTER              0
#define THISSHIPIS_CORVETTE             1
#define THISSHIPIS_RESOURCER            2
#define THISSHIPIS_OTHERNONCAPITALSHIP  3

struct ShipStaticInfo;
struct CommandToDo;

typedef struct
{
    udword sizeofShipSpecifics;
    void (*CustShipStaticInit) (char *directory,char *filename,struct ShipStaticInfo *statinfo);     // +0x04 retail slot 1
    void (*CustShipStaticInitPost) (struct ShipStaticInfo *statinfo);                                 // +0x08 retail slot 2
    void (*CustShipInit) (struct Ship *ship);                                                         // +0x0C retail slot 3
    void (*CustShipFreeShipContents) (struct Ship *ship);                                             // +0x10 retail slot 4
    void (*CustShipAttack) (struct Ship *ship,struct SpaceObjRotImpTarg *target,real32 maxdist);     // +0x14 retail slot 5; retail headers use this as the immediate weapon-fire / shoot callback
    void (*CustShipRotateAndShoot) (struct Ship *ship, struct SpaceObjRotImpTarg *target, sdword rotateFlags); // +0x18 retail slot 6; retail headers use this as the higher-level attack behavior callback
    union
    {
        bool32 (*CustShipSpecialActivateNoTarget) (struct Ship *ship);                                 // retail no-target special callback
        void (*CustShipAttackPassive) (struct Ship *ship,struct Ship *target,bool32 rotate);          // legacy/source view of slot 7
    };                                                                                                 // +0x1C retail slot 7
    union
    {
        bool32 (*CustShipSpecialTargetRaw) (struct Ship *ship,void *custom);                           // retail targeted special callback
        bool32 (*CustShipSpecialActivate) (struct Ship *ship);                                         // legacy/source view of slot 8
    };                                                                                                 // +0x20 retail slot 8
    union
    {
        bool32 (*CustShipSpecialTarget) (struct Ship *ship,void *custom);                             // legacy/source view of slot 9
        void (*CustShipSpecialOrderCleanup) (void *context);                                          // retail slot 9 is a unary special-target clear/reset hook (carrier/mothership/salcap), not a second targeted-special callback
    };                                                                                                 // +0x24 retail slot 9
    void (*CustShipHousekeep) (struct Ship *ship);                                                     // +0x28 retail slot 10
    void (*CustShipRemoveShipReferences)(struct Ship *ship, struct Ship *shiptoremove);               // +0x2C retail slot 11
    void (*CustShipNotifyShipDied)(struct Ship *ship, struct Ship *deadShip);                         // +0x30 retail slot 12
    void (*CustShipNotifyObjectDeleted)(struct Ship *ship, sdword objectID);                          // +0x34 retail slot 13; clears currently tracked object refs (for example mothership big-gun current target)
    void (*CustShipNotifySpecialTarget)(struct Ship *ship, void *custom);                             // +0x38 retail slot 14; secondary object-delete callback used for queued/deferred refs (for example carrier/mothership big-gun queues)
    void (*CustShipClose) (struct Ship *ship);                                                         // +0x3C retail slot 15
    void (*CustShip_PreFix)(struct Ship *ship);                                                        // +0x40 retail slot 16, pre-save hook run before the main ship chunk is serialized
    void (*CustShip_Save)(struct Ship *ship);                                                          // +0x44 retail slot 17, post-main-chunk save hook
    void (*CustShip_Load)(struct Ship *ship);                                                          // +0x48 retail slot 18
    void (*CustShip_Fix)(struct Ship *ship);                                                           // +0x4C retail slot 19
    union
    {
        bool32 (*CustShipMoveOrderCleanup)(struct Ship *ship, void *arg1, void *arg2);               // +0x50 retail slot 20; clMove passes packed move-path args here, and special ships reuse it as a pre-move cleanup/reset hook
        bool32 (*CustShipMoveConstraint)(struct Ship *ship, vector *fromPoint, vector *toPoint);     // legacy/source alias of slot 20
    };                                                                                                 // +0x50 retail slot 20
    union
    {
        bool32 (*CustShipActiveAttackCommand)(struct Ship *ship, struct SpaceObjRotImpTarg *target, real32 maxdist); // +0x54 retail slot 21; attack-capable headers store their active attack-command callback here, while a few special ships reuse it for equivalent special-order/state hooks
        bool32 (*CustShip_SpecialOrder)(struct Ship *ship, void *custom);                             // retail/source special-order users of slot 21 (ACV, HiveFrigate, Sentinel, etc.)
        bool32 (*CustShipAttackState)(struct Ship *ship, struct SpaceObjRotImpTarg *target);         // retail/source attack-state users of slot 21 (for example MultiBeamFrigate)
        bool32 (*CustShip_SpecialActivate2)(struct Ship *ship, void *custom);                         // legacy/source alias of slot 21
    };                                                                                                 // +0x54 retail slot 21
    bool32 (*CustShip_DockStart)(struct Ship *ship, sdword dockType, struct Ship *dockwith);         // +0x58 retail slot 22, dock-start / pre-dock cleanup hook
    bool32 (*CustShipLaunchFilter)(struct Ship *ship, struct Ship *receiverShip);                     // +0x5C retail slot 23, launch-order filter
    void *CustShip_UnknownSlot24;                                                                      // +0x60 retail slot 24, type unverified
    bool32 (*CustShipPathMoveConstraint)(struct Ship *ship, vector initialPoint, sdword pointCount, vector *points); // +0x64 retail slot 25, type/signature still unverified
    void *CustShip_UnknownSlot26;                                                                      // +0x68 retail slot 26; usually NULL in retail header templates, except Worker which points it at pingEvalAlwaysTrue
    bool32 (*CustShipHaltFilter)(struct Ship *ship);                                                  // +0x6C retail slot 27, halt-order filter / special-state cleanup hook
    void *CustShip_UnknownSlot28;                                                                      // +0x70 retail slot 28, type unverified
    void *CustShip_UnknownSlot29;                                                                      // +0x74 retail slot 29, type unverified
    void *CustShip_UnknownSlot30;                                                                      // +0x78 retail slot 30, type unverified
    void *CustShip_UnknownSlot31;                                                                      // +0x7C retail slot 31, type unverified
    void *CustShip_UnknownSlot32;                                                                      // +0x80 retail slot 32, type unverified
    void *CustShip_UnknownSlot33;                                                                      // +0x84 retail slot 33, type unverified
    bool32 (*CustShipFormationFilter)(struct Ship *ship, sdword formation, struct Ship *leader);      // +0x88 retail slot 34
} CustShipHeader;

typedef struct ShipStaticInfo
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    //these 3 things are static and don't affect a ship after the
    //ship has been built.  They are copied into the NonStats data
    //structure due to a need for them to be nonstatic
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];

    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be

    //these 3 variables must be next
    real32 sinbank;             // variables used by the AIShip routines
    real32 pitchturn;
    real32 pitchdescend;

    NAVLightStaticInfo *navlightStaticInfo;
/*ship specific stuff below */
    CustShipHeader custshipheader;
    void *custstatinfo;
    union
    {
        real32 bulletRangeSquared[NUM_TACTICS_TYPES];
        WorkerHarvestStaticInfo *workerHarvestStatic;
    };
    real32 bulletRange[NUM_TACTICS_TYPES];
    real32 minBulletRange[NUM_TACTICS_TYPES];
    real32 blastRadiusShockWave;
    real32 blastRadiusDamage;
    real32 passiveRetaliateZone;
    real32 collSideModifiers[NUM_TRANS_DEGOFFREEDOM];

    ShipType shiptype;
    ShipRace shiprace;
    ShipClass shipclass;
    udword shipFlags;
    bool32 externalBuild;
    real32 forceFieldProtectionDistanceScale;
    real32 upgradeTimeModifier;

    bool8 canBuildShips;
    bool8 canReceiveShips;
    bool8 canReceiveResources;
    bool8 canReceiveShipsPermanently;
    bool8 canReceiveShipsForRetire;
    bool8 canReceiveSomething;
    bool8 canBuildBigShips;
    bool8 canTargetMultipleTargets;
    bool8 rotateToRetaliate;
    bool8 canReceiveTheseShips[4];

    bool32 passiveAttackPenaltyExempt;

    bool8 specialActivateIsContinuous;
    bool8 canSpecialBandBoxFriendlies;
    bool8 unk4;
    bool8 unk5;
    bool8 canSingleClickSpecialActivate;

    sword maxDockableFighters;  // maximum fighters that can dock at this ship
    sword maxDockableCorvettes; // maximum corvettes that can dock at this ship

    sdword canHandleNumShipsDocking;
    sdword canHandleNumShipsDepositingRU;

    bool32 repairBeamCapable;
    real32 healthPerSecond;
    real32 CapitalDistanceRepairStart2;
    real32 CapitalDistanceRepairStart;
    real32 AngleDotProdThreshold;

    sdword buildCost;
    sdword buildTime;
    real32 fowRadius;
    sdword svManeuverability;
    sdword svCoverage;
    sdword svFirePower;
    real32 RKOEatMeTime;

    sdword resourcesAtOneTime;
    sdword maxresources;
    sdword harvestRate;
    sdword harvestAmount;

    sdword groupSize;

    real32 maxfuel;
    real32 lowfuelpoint;
    real32 fuelburnrate;
    real32 attackfuelburnrate;

    real32 repairTime;          // self-repair for capital ships
    real32 repairDamage;
    real32 repairCombatTime;    // self-repair for capital ships in combat
    real32 repairCombatDamage;

    real32 pumpFuelRate;        // rate at which ship can refuel other ships
    real32 repairOtherShipRate; // rate at which ship can repair other ships
    real32 repairResourceCollectorRate; //specifically for when a ship is repairing a resource collector.  0.0f deafults to previous value
    real32 clearanceDistance;   // distance which other docking ship should fly away for clearance
    udword clearanceDirection;  // direction which other ship should fly away for clearance (0..7)

    real32 dockShipRange;       // distance to ship to get for docking, in metres

    real32 formationPaddingModifier;
    bool32 cannotForceAttackIfOwnShip;    //variable..if set to TRUE player cannot force attack this ship IF it is their own ship
    bool32 shipIsCapital;                 //Set to TRUE is ship is a TRUE capital ship (class defs are unreliable)
    bool32 cantMoveAndAttack;             //if set, SHIP explicitly cannot move and attack...will be given a special move order.
    vector engineNozzleOffset[MAX_NUM_TRAILS];      //offset of engine in object co-ordinates
    trailstatic* trailStatic[MAX_NUM_TRAILS];
    udword multipleEngines;
    ubyte unk6[24];

    real32 scaleCap;            //minimum sizing cap, which prevents a ship from getting too small

    sdword hierarchySize;       //size of the ship hierarchy
    madstatic *madStatic;       //mesh animation binding and animation data
    real32 madGunOpenDamagedHealthThreshold;
    real32 madWingOpenDamagedHealthThreshold;

    udword nRepairDroids;

    // Keith's engine trail stuff...
    udword trailStyle[MAX_NUM_TRAILS];
    real32 trailWidth[MAX_NUM_TRAILS];
    real32 trailHeight[MAX_NUM_TRAILS];
    real32 trailAngle[MAX_NUM_TRAILS];
    real32 trailRibbonAdjust[MAX_NUM_TRAILS];
    real32 trailLength[MAX_NUM_TRAILS];
    real32 trailScaleCap[MAX_NUM_TRAILS];
    real32 trailSpriteRadius[MAX_NUM_TRAILS];
    real32 trailSpriteOffset[MAX_NUM_TRAILS];

    real32 minimumZoomDistance;
    real32 renderlistFade, renderlistLimitSqr;

    real32 dockLightNear, dockLightFar;
    color  dockLightColor;

    color  hyperspaceColor;

    real32 hyperspaceBoxScale;
    real32 leechRUsPerSecond;
    real32 leechHPsPerSecond;
    real32 beastAttackCascadeCost;
    udword beastAttacksRequired;
    ShipType beastAttackConvertTo;
    real32 tacticalSelectionScale;

    GunStaticInfo *gunStaticInfo;
    DockStaticInfo *dockStaticInfo;
    DockOverideInfo *dockOverideInfo;
    ResNozzleStatic *resNozzleStatic;
    RepairNozzleStatic *repairNozzleStatic;
    TractorBeamStatic *tractorEmitterStatic;
    // Legacy HW1/CET tail names below are not retail-accurate past 0x404.
    // Use ShipStaticInfoCataclysmTail accessors for active Cataclysm fields in this region.
    bool32 (*ShipXDocksAtShipY) (struct CommandToDo *docktodo,struct Ship *ship,struct Ship *dockwith);
    bool32 (*LaunchShipXFromShipY) (struct Ship *ship,struct Ship *dockwith);
    real32 linkDistance;
    udword RKOGetsMissiles;
    ubyte unk8[28];
    bool32 bUseRayTriangleIntersectionForHitLocation;
    real32 unk12;
    udword supportUnits;
    bool32 UpgradeByDocking;
    real32 pilotViewOffsetX;
    real32 pilotViewOffsetY;
    real32 pilotViewOffsetZ;
    udword unk10;
    union
    {
        udword providesSU;
        udword builtInUpgradeMask;
    };
    real32 RamDamage;
    real32 cuttingBeamDamagePerSecond;
    real32 cuttingBeamUpgradedDamagePerSecond;
    real32 RamPushTime;
    real32 RamWaitAfterPushTime;
    udword unk9;
    real32 TimeShockedByEMP;
    real32 EMP_Range;
    real32 EMP_Max_Combine_Distance;

#if SO_CLOOGE_SCALE
    real32 scaleFactor;
#endif
} ShipStaticInfo;

typedef bool32 (*ShipDockProcessCallback)(struct CommandToDo *docktodo, struct Ship *ship, struct Ship *dockwith);

typedef struct
{
    TractorBeamStatic *tractorEmitterStatic;      // 0x00 / ShipStaticInfo+0x400
    ShipDockProcessCallback dockProcessCallback;  // 0x04 / retail copied in FixShipStaticInfoAfterLoad
    ubyte unk408[0x10];                           // 0x08-0x17 / unresolved retail tail bytes
    udword unk418;                                // 0x18 / copied by rammingFrigateSelectParams
    bool32 bUseRayTriangleIntersectionForHitLocation; // 0x1C
    real32 linkDistance;                          // 0x20
    udword supportUnits;                          // 0x24
    bool32 UpgradeByDocking;                      // 0x28
    ubyte unk2C[0x28];                            // 0x2C-0x53 / unresolved retail tail bytes (ShipStaticInfo 0x42C-0x453)
    ubyte crystalProcessedChunkLimit;             // 0x54 / ShipStaticInfo+0x454: RKO crystal chunk processing limit (ubyte)
} ShipStaticInfoCataclysmTail;
static_assert(sizeof(ShipStaticInfoCataclysmTail) == 0x58);
static_assert(offsetof(ShipStaticInfoCataclysmTail, tractorEmitterStatic) == 0x00);
static_assert(offsetof(ShipStaticInfoCataclysmTail, dockProcessCallback) == 0x04);
static_assert(offsetof(ShipStaticInfoCataclysmTail, unk418) == 0x18);
static_assert(offsetof(ShipStaticInfoCataclysmTail, bUseRayTriangleIntersectionForHitLocation) == 0x1C);
static_assert(offsetof(ShipStaticInfoCataclysmTail, linkDistance) == 0x20);
static_assert(offsetof(ShipStaticInfoCataclysmTail, supportUnits) == 0x24);
static_assert(offsetof(ShipStaticInfoCataclysmTail, UpgradeByDocking) == 0x28);
static_assert(offsetof(ShipStaticInfoCataclysmTail, crystalProcessedChunkLimit) == 0x54);
static_assert(offsetof(ShipStaticInfo, tractorEmitterStatic) + offsetof(ShipStaticInfoCataclysmTail, crystalProcessedChunkLimit) == 0x454);

#define SHIP_STATIC_INFO_CATA_TAIL_OFFSET(field) \
    (offsetof(ShipStaticInfo, tractorEmitterStatic) + offsetof(ShipStaticInfoCataclysmTail, field))

static inline ShipStaticInfoCataclysmTail *shipStaticInfoCataclysmTail(ShipStaticInfo *statinfo)
{
    return (ShipStaticInfoCataclysmTail *)((ubyte *)statinfo + offsetof(ShipStaticInfo, tractorEmitterStatic));
}

static inline const ShipStaticInfoCataclysmTail *shipStaticInfoCataclysmTailConst(const ShipStaticInfo *statinfo)
{
    return (const ShipStaticInfoCataclysmTail *)((const ubyte *)statinfo + offsetof(ShipStaticInfo, tractorEmitterStatic));
}

static inline ShipDockProcessCallback ShipStaticInfoGetDockProcessCallback(const ShipStaticInfo *statinfo)
{
    return shipStaticInfoCataclysmTailConst(statinfo)->dockProcessCallback;
}

static inline void ShipStaticInfoSetDockProcessCallback(ShipStaticInfo *statinfo, ShipDockProcessCallback callback)
{
    shipStaticInfoCataclysmTail(statinfo)->dockProcessCallback = callback;
}

static inline udword ShipStaticInfoGetUnk418(const ShipStaticInfo *statinfo)
{
    return shipStaticInfoCataclysmTailConst(statinfo)->unk418;
}

static inline bool32 ShipStaticInfoGetRayTriangleHitLocationFlag(const ShipStaticInfo *statinfo)
{
    return shipStaticInfoCataclysmTailConst(statinfo)->bUseRayTriangleIntersectionForHitLocation;
}

static inline real32 ShipStaticInfoGetLinkDistanceValue(const ShipStaticInfo *statinfo)
{
    return shipStaticInfoCataclysmTailConst(statinfo)->linkDistance;
}

static inline udword ShipStaticInfoGetSupportUnitsField(const ShipStaticInfo *statinfo)
{
    return shipStaticInfoCataclysmTailConst(statinfo)->supportUnits;
}

static inline bool32 ShipStaticInfoUpgradeByDockingEnabled(const ShipStaticInfo *statinfo)
{
    return shipStaticInfoCataclysmTailConst(statinfo)->UpgradeByDocking;
}

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;
} ResourceStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    AsteroidType asteroidtype;      // Asteroid specific stuff from here on
    bool8 asteroidCanShrink;
    bool8 asteroidCanBreak;
    bool8 pad2;
    bool8 pad3;
    real32 asteroidMinShrink;
    udword breakinto[NUM_ASTEROIDTYPES];
} AsteroidStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    NebulaType nebulatype;
    bool8 nebulaCanShrink;
    bool8 pad[3];
    real32 nebulaMinShrink;
} NebulaStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    GasCloudType gascloudtype;
    bool8 gascloudCanShrink;
    bool8 pad[3];
    real32 gascloudMinShrink;
} GasCloudStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;

    sdword resourcevalue;
    udword harvestableByMultipleShips;

    DustCloudType dustcloudtype;
    bool8 dustcloudCanShrink;
    bool8 pad[3];
    real32 dustcloudMinShrink;
} DustCloudStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];

    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be
    SalvageStaticInfo   *salvageStaticInfo;           //static info for salvageable objects

    //these 3 variables must be next to be compatible with ship static info for aiship stuff
    real32 sinbank;             // variables used by the AIShip routines
    real32 pitchturn;
    real32 pitchdescend;

    NAVLightStaticInfo *navlightStaticInfo;
/* Derelict specific below */
    DerelictType derelicttype;
    bool32    salvageable;                        //TRUE if derelict is to be targetable by salcap corvettes

    real32 scaleFactor;                         //only used for 'world rendering'
    bool32   worldRender;                         //TRUE for Angel moon, homeworld, and POO

    real32 minimumZoomDistance;
    real32 renderlistFade, renderlistLimitSqr;
} DerelictStaticInfo;

typedef struct
{
    StaticHeader staticheader;
    real32 maxhealth;
    real32 oneOverMaxHealth;
    real32 minCollDamage;
    real32 maxCollDamage;
    real32 thruststrengthstat[NUM_TRANS_DEGOFFREEDOM];
    real32 rotstrengthstat[NUM_ROT_DEGOFFREEDOM];
    real32 turnspeedstat[NUM_TURN_TYPES];                   // these 8 first lines are same as ShipStaticInfo, allowing
    bool8 teamColor[MAX_MULTIPLAYER_PLAYERS];           //what color schemes this can be


/* missile specific below*/
    real32 mineRange;
    real32 mineRangeForced;
    real32 MINE_STOP_FRICTION;
    real32 MINE_RANGESQR;
    real32 MINE_RANGESQR_FORCED;
    real32 maxvelocity_FORCED;

    //missile trail stuff
    vector engineNozzleOffset;
    trailstatic* trailStatic;

    real32 trailWidth;
    real32 trailHeight;
} MissileStaticInfo;

/*=============================================================================
    Object Instance Information
=============================================================================*/
/*
typedef struct
{
    sdword nSides;          //3 or 4 sides
    vector p[4];            //corners;
}
precisepoly;
*/
typedef struct
{
    vector worldRectPos[8]; //collision rect in world space
    ubyte  corner[24];      //3 polys, 4 corners each, plus padding for run-over
//    precisepoly poly[5];        //there could be up to 5 polys
    sdword  nPolys;         //number of camera-facing sides of collision box (0-5)
}
PreciseSelection;

#define MOVING_LINEARLY_BIT         BIT0
#define MOVING_ROTATIONALLY_BIT     BIT1

#define SET_MOVING_LINEARLY(x)      bitSet((x), MOVING_LINEARLY_BIT)
#define SET_MOVING_ROTATIONALLY(x)  bitSet((x), MOVING_ROTATIONALLY_BIT)
#define SET_MOVING_IMMOBILE(x)      bitClear((x), MOVING_LINEARLY_BIT | MOVING_ROTATIONALLY_BIT)

#define IS_MOVING_LINEARLY(x)       bitTest((x), MOVING_LINEARLY_BIT)
#define IS_MOVING_ROTATIONALLY(x)   bitTest((x), MOVING_ROTATIONALLY_BIT)
#define IS_MOVING_IMMOBILE(x)       (!(IS_MOVING_LINEARLY(x) || IS_MOVING_ROTATIONALLY(x)))

typedef struct
{
    vector position;        // object position in universe
    vector velocity;        // velocity of object
    vector force;           // force acting on object
    bitflag16 isMoving;           // bit flag indicating how object is moving
    bool16 haventCalculatedDist;  // flag indicating haven't yet calculated distance
    union
    {
        ubyte posPad[24];         // Cataclysm-added (binary PosInfo = 64 bytes, not 40)
        struct
        {
            lodmeshbindings *lodMeshBindings;   // retail mesh auxiliary binding table at +0x28
            void *unk_2C;                       // Cataclysm-added pointer slot at +0x2C
            ubyte unk_30_3F[16];
        };
    };
} PosInfo;

typedef struct
{
    vector collOffset;      // collision sphere offset from position in object coordinates.
    vector collPosition;    // object collision position in universe = position + collOffset
    vector rectpos[8];      // rectangle corner positions (in object coordinates)
    PreciseSelection *precise;//precise collision info; not all ships have this
    real32 selCircleX, selCircleY;// on-screen location of selection circle on-screen
    real32 selCircleRadius; // on-screen size of selection circle
    real32 selCircleDepth;  // depth of object in camera space
} CollInfo;

typedef struct
{
    matrix coordsys;        // matrix describing co-ordinate system of object
    vector rotspeed;        // rotational speed (rad/s)
    vector torque;          // torque acting on object
} RotInfo;

#define     SF_SLAVE    0x01
#define     SF_MASTER   0x02
#define     SF_DEAD     0x04
typedef struct
{
    udword flags;           //indicates status of Slavery
    struct Ship *Master;           //
    vector offset;          //ships position in MASTER space
    matrix coordsys;        //ships orientation in MASTER space
    LinkedList slaves;      //Defined only if a master (master not in list)
} SlaveInfo;

/*-----------------------------------------------------------------------------
Layout of co-ordinate system:

The ship heading, upvector, and rightvectors are stored in the coordsys
matrix as follows:

|   upx     rightx  headingx  |
|   upy     righty  headingy  |
|   upz     rightz  headingz  |
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
    SpaceObj - General, base object from which all objects are derived
-----------------------------------------------------------------------------*/

#define SOF_Rotatable           0x00000001
#define SOF_Impactable          0x00000002      // objects such as asteroids, ships which
                                                // can impart a change in momentum
#define SOF_DontApplyPhysics    0x00000004      // don't apply physics to such things as
                                                // explosions
#define SOF_AttachPosition      0x00000008      // prevent 'torpedo effects'
#define SOF_AttachCoordsys      0x00000010      // ditto, but more so
#define SOF_AttachVelocity      0x00000020
#define SOF_AttachNLips         0x00000040      // for ETG N-Lips inherited from ships

#define SOF_NISShip             0x00000080      //subject to 'NIS physics'

#define SOF_CloakGenField       0x00000100      //if the object is cloaked with the local players own field...i.e. if he/she can see the object
#define SOF_Cloaked             0x00000200      //object is cloaked for whatever reason
#define SOF_Cloaking            0x00000400      //Set if Object is In Process of Cloaking
#define SOF_Dead                0x00000800      //Retail: univDeleteDeadShip/MothershipHouseKeep set bit 0x800 on death
#define SOF_DontCreateBlob      0x00000800      //transient blob-management alias for SOF_Dead; set by bobAllObjectsFlagDontUpdate, cleared by bobListUpdate

#define SOF_DeCloaking          0x00001000      //Set if Object is in process of DeCloaking

#define SOF_Selectable          0x00002000      //Indicates object is selectable
#define SOF_BitForSaleCheap     0x00004000

#define SOF_Hide                0x00008000      // Retail render-list builders and kasfHideShips use 0x8000 as the real hide bit
#define SOF_DockMarker          SOF_Hide        // Legacy stale alias pending per-site audit of old dock-marker assumptions

#define SOF_ForceVisible        0x00010000      // Retail univUpdateRenderList adds 0x10000 objects unconditionally to RenderList
#define SOF_Resource            SOF_ForceVisible // Legacy stale alias pending per-site audit of former resource-bit assumptions

#define SOF_BigObject           0x00020000      // Retail univUpdateRenderList uses 0x20000 for the long-range big-object visibility branch

#define SOF_StaticInfoIsDynamicCata 0x00040000  // Cataclysm runtime bit for per-object dynamic staticinfo; source refs must not assume ForceVisible lives here

#define SOF_NotBumpable         0x00200000      //Indicates colliding with object should not result in momentum transferal

#define SOF_Targetable          0x00100000      // Indicates object is targetable
#define SOF_TargetableHW1       0x00400000      // Legacy HW1 targetable bit value

#define SOF_Slaveable           0x00800000      // Indicates if object is ABLE to be slaved..and
                                                //hence has relevant info in its slaveinfo struct
                                                // IDA (univSetupShipForControl @ 0x54a650): also the bit gating
                                                // tacticsUpdate/out-of-world check, idle-ship auto-retaliation,
                                                // and not-idle passive-attack-while-moving in univCheckShipState
                                                // -- NOT SOF_Disabled, which our code previously (incorrectly) used there

#define SOF_Clamped             0x01000000      // Retail clampObjectSetup/clDetachShipFromCommand set and clear this on clamped children

#define SOF_Crazy               0x00200000      // Retail delayed-death/careen ship paths reuse the non-bumpable bit on ships

#define SOF_DontDrawTrails      0x02000000      // Indicates we do not want to draw engine trails. (Used for ships only)

#define SOF_Disabled            0x04000000      // Legacy stale alias pending per-site audit; retail render-list code uses 0x04000000 for always-sort-front

#define SOF_Hyperspace          0x10000000      //indicates ship is in hyperspace ether

#define SOF_AlwaysSortFront     0x04000000      // Retail render-list builders sort-front on 0x04000000

#define SOF_ForwardDamageToClampHost 0x40000000 // ApplyDamageToTarget forwards flagged clamped-component damage to the host ship
#define SOF_PreviouslyHiddenByKAS    0x08000000 // Retail kasfHideShips/kasfUnhideShips preserve pre-hidden state with 0x08000000
#define SOF_AvoidanceHidden          0x80000000 // exclude from avoidance/collision while still renderable; reused by hide-all as a restore marker
#define SOF_PreviouslyHiddenByHideAll 0x80000000
#define SOF_HideForAvoidance         (SOF_Hide | SOF_AvoidanceHidden)

// Cataclysm-added flags word at offset 0x18 in all SpaceObj-family structs
#define SOF2_BayDocked          0x00000001      // ship is bay-docked inside a carrier
#define SOF2_Unknown4           0x00000004      // retail CarrierClose, BeastMothershipHouseKeep, and slipgate init set Ship+0x18 BIT2; semantics still unresolved
#define SOF2_Unknown8           0x00000008      // retail univSetupShipForControl gates self-destruct/auto-attack/passive-retaliate scans on this bit alongside shipHasModuleSlotState(ship); semantics still unresolved
#define SOF2_NotifyShipDiedPending 0x00000100   // retail univNotifyShipDied/univShipHouseKeep latch at Ship+0x18

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;           // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
} SpaceObj;

typedef SpaceObj *SpaceObjPtr;

/*=============================================================================
    SpaceObj - rotatable
=============================================================================*/

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;           // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
} SpaceObjRot;

/*=============================================================================
    SpaceObj - rotatable, impactable
=============================================================================*/

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
} SpaceObjRotImp;

/*=============================================================================
    SpaceObj - rotatable, impactable, targetable
=============================================================================*/

typedef struct SpaceObjRotImpTarg
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    StaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints (binary: before nonstatvars)
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;

} SpaceObjRotImpTarg;

typedef SpaceObjRotImpTarg *TargetPtr;

/*=============================================================================
    SpaceObj - rotatable, impactable, targetable, guidance
=============================================================================*/

typedef struct SpaceObjRotImpTargGuidance
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    StaticInfoHealthGuidance *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;

    // Cataclysm ships/derelicts keep clampInfo at 0x1BC; preserve that layout
    // here so shared clamp helpers can safely operate on guidance objects too.
    ubyte aistate;
    ubyte pad1[2];
    sbyte putOnDoor;
    ClampInfo *clampInfo;

} SpaceObjRotImpTargGuidance;   // used by missiles, ships, derelicts

typedef struct SpaceObjRotImpTargGuidanceShipDerelict
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;            // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    StaticInfoHealthGuidanceShipDerelict *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;

    //must come next to be compatible with Derelict in aiship stuff
    ubyte aistate;                  // AI state variable
    ubyte pad1[2];
    sbyte putOnDoor;
    ClampInfo *clampInfo;           // at 0x1BC, confirmed via binary
    udword specialFlags;            //general purpose ship flags, see above for defines
    udword specialFlags2;           //more general purpose ship flags
    ubyte cataRuntimeState[0x4C];   // Cataclysm-only runtime block; keeps navLightInfo at 0x234
    udword specialFlags3;           //Cataclysm general purpose ship flags
    udword specialFlags4;           //Cataclysm general purpose ship flags
    sdword specialFlagsUnknown2;
    sdword specialFlagsUnknown3;
    sdword specialFlagsUnknown4;
    sdword specialFlagsUnknown5;
    sdword specialFlagsUnknown6;
    sdword specialFlagsUnknown7;
    NAVLightInfo *navLightInfo;
    // HERE UP SAME for Derelicts and Ships
} SpaceObjRotImpTargGuidanceShipDerelict;   // ships, derelicts

typedef struct TargetID
{
    uword objNumber;    // must be first member of structure to be compatible with other ID's
    uword objtype;
} TargetID;

/*=============================================================================
    Repair Droid Stuff:
=============================================================================*/
/*
typedef struct RepairDroid
{
    udword flags;       // RD Flags...
    PosInfo posinfo;    // Position
    RotInfo rotinfo;    // Rotation
} RepairDroid;
*/
/*-----------------------------------------------------------------------------
    Ship Object
-----------------------------------------------------------------------------*/

#define ATTACKEVENT_ENEMYTARGET_DIED    1
#define ATTACKEVENT_LEADER_DIED         2
#define ATTACKEVENT_WINGMAN_DIED        4

#define ATTACKSITUATION_EVADINGREPOS    1
#define ATTACKSITUATION_JUSTEVADING     2
#define ATTACKSITUATION_SPLITTING       3
#define ATTACKSITUATION_SANDWICH        4

//  universe ticks before a ship "forgets" that it was under attack
//  (used by the recentlyAttacked member -- keep this within a ubyte for now)
#define RECENT_ATTACK_DURATION        ((ubyte)(5 * UNIVERSE_UPDATE_RATE))

typedef struct
{
    sdword numAttackTargets;
    TargetPtr TargetPtr[1];
} AttackTargets;

#define sizeofAttackTargets(n) (sizeof(AttackTargets) + (n-1)*sizeof(TargetPtr))

typedef struct
{
    AttackTargets *multipleAttackTargets;   // only used by ships which can target multiple targets.
    SpaceObjRotImpTarg *attacktarget;
    struct Ship *myLeaderIs;
    struct Ship *myWingmanIs;
    uword flightmansLeft;       // number of flight maneuvers left
    uword attacksituation;      // what I am currently doing
    uword attackevents;         // event notification using ATTACKEVENT flags
    uword pad;
} SpecificAttackVars;

// Variable-length activity array allocated per docking ship.
// Total size: offsetof(DockPointActivity, reserved) + sizeof(sdword) * numDockPoints
typedef struct
{
    ubyte numDockedOfType[NUM_DOCK_POINT_TYPES]; // occupancy count per dock point type
    ubyte _pad;                                   // alignment padding before reserved[]
    sdword reserved[1];                           // [numDockPoints]: 1 = reserved, 0 = free
} DockPointActivity;
static_assert(offsetof(DockPointActivity, reserved) == 36);

typedef struct
{
    struct Ship *dockship;
    union
    {
        struct Ship *busyingThisShip;
        real32 coneDistance;
    };
    sdword dockLaunchState;      // 0x08 / Ship+0x49C - retail launch/dock state word
    sdword dockPointIndex;       // 0x0C / Ship+0x4A0 - retail reserved dock-point index (-1 when none)
    real32 dockStateDeadline;    // 0x10 / Ship+0x4A4 - retail launch/dock timer/deadline
    DockPointActivity *dockPointActivity; // 0x14 / Ship+0x4A8 - host dock activity block
    DockStaticPoint *dockstaticpoint;     // 0x18 / Ship+0x4AC - transient dock-point pointer (local-scope usage only)
    void *customdockinfo;        // 0x1C / Ship+0x4B0 - polymorphic: heap state, gun tables, creator/target ptr
    LinkedList *launchpoints;
} ShipDockRuntime;
static_assert(sizeof(ShipDockRuntime) == 0x24);
static_assert(offsetof(ShipDockRuntime, dockship) == 0x00);
static_assert(offsetof(ShipDockRuntime, coneDistance) == 0x04);
static_assert(offsetof(ShipDockRuntime, dockLaunchState) == 0x08);
static_assert(offsetof(ShipDockRuntime, dockPointIndex) == 0x0C);
static_assert(offsetof(ShipDockRuntime, dockStateDeadline) == 0x10);
static_assert(offsetof(ShipDockRuntime, dockPointActivity) == 0x14);
static_assert(offsetof(ShipDockRuntime, dockstaticpoint) == 0x18);
static_assert(offsetof(ShipDockRuntime, customdockinfo) == 0x1C);
static_assert(offsetof(ShipDockRuntime, launchpoints) == 0x20);

/*
 * Cataclysm repurposes the pointer-bearing tail that begins at Ship+0x490.
 * The legacy field names remain for compile compatibility, but Cataclysm code and
 * retail save helpers show:
 *   0x494 -> dockship *
 *   0x498 -> dock cone distance / busyingThisShip overlay
 *   0x49C -> launch/dock state word
 *   0x4A0 -> reserved dock-point index (-1 when none)
 *   0x4A4 -> launch/dock timer/deadline
 *   0x4A8 -> dockPointActivity * on hosts
 *   0x4AC -> dockstaticpoint * (transient, local-scope only; never persists across frames)
 *   0x4B0 -> customdockinfo * (polymorphic: heap state, gun tables, or creator/target ptr)
 *   0x4B4 -> launchpoints LinkedList * of InsideShip nodes, not the HW1 array type
 *   0x4B8 -> bindings *
 *   0x4BC -> madBindings *
 *   0x4C4 -> ShipSinglePlayerGameInfo *
 *   0x4C8..0x4CC -> ship lightning effect handles
 *   0x4D0..0x500 -> status-effect / beast overlay, not the old HW1 slave tail
 * Keep using offset-safe accessors when touching save/load code in this area.
 */

typedef struct ShipID
{
    uword shipNumber;
} ShipID;

typedef struct
{
    sdword level;
    udword mask;
} ShipUpgradeState;

typedef struct
{
    ShipType buildTargetType;
    ShipRace buildTargetRace;
    lodinfo *buildTargetLodInfo;
    shipbindings *buildTargetBindings;
    sdword colorScheme;
    real32 displaySphereRadius;
    sdword slotIndex;
    sdword transformModuleType;
    real32 transformCollSphereSize;
    real32 transformMinimumZoomDistance;
    sdword displayCount;
} ShipModuleSlotBuildTarget;

typedef struct
{
    ShipType moduleType;
    sdword iconId;
} ShipModuleSlotIconState;

typedef enum
{
    ShipModuleSlotState_Disabled = 0,
    ShipModuleSlotState_Active = 1,
    ShipModuleSlotState_Opening = 2,
    ShipModuleSlotState_Closing = 3,
    ShipModuleSlotState_Resetting = 4,
    ShipModuleSlotState_Destroying = 5,
    ShipModuleSlotState_Destroyed = 8,
    ShipModuleSlotState_WaitingToOpen = 9,
    ShipModuleSlotState_WaitingToClose = 10,
    ShipModuleSlotState_WaitingToReset = 11,
} ShipModuleSlotState;

typedef enum
{
    BS_None = 0,
    BS_Thrust = 1,
    BS_Infection = 2,
    BS_DefensiveWeapons = 3,
    BS_Research = 4,
    BS_Build = 5,
} BeastSystemType;

typedef struct
{
    ubyte unk0[0x60];
    ShipModuleSlotIconState iconState[7];
    ubyte unk98_A7[0x10];
    udword slotAvailability[8];
    ShipModuleSlotBuildTarget activeBuildTarget;
    ShipModuleSlotBuildTarget pendingBuildTarget;
    real32 morphProgress;
    ShipModuleSlotState state;
    real32 transitionTime;
    bool32 transitionArmed;
    struct Resource *activeEffect;
} ShipModuleSlotRuntime;

typedef struct
{
    ubyte unk0[0x9C4];
    udword swarmerReadyState;
} ShipSwarmerRuntimeOverlay;

/* Collision polygon overlay for ships that support ray-triangle hit testing
   (e.g., dying Mothership/Carrier-class ships).  Starts at sizeof(Ship). */
typedef struct
{
    udword unk_63C;        /* Ship+0x63C: overlay preamble */
    sdword polyCount;      /* Ship+0x640: number of collision polygon entries */
    udword polyEntries[1]; /* Ship+0x644: ragged array of SpaceObj pointer pairs
                              {vtx1_base, vtx2_base}; vertex = obj->posinfo.position */
} ShipCollisionPolyOverlay;

/* KAS "wait for ships" scratch state used by shipStartWaitingForShips/
   shipGetWaitingForShipsPoints/shipWaitingForShipsDone/shipWaitingForShipsActive.
   Located in ship-type tail storage at a fixed offset from Ship base, confirmed
   via raw disasm of all four functions (IDA 0x5c5500-0x5c5758): the waiting flag,
   fromPoint/toPoint vectors, and safe-distance-squared all live at Ship+0x6C4
   onward, NOT at sizeof(Ship) (Ship+0x63C) as a naive `ship[1]` pointer-arithmetic
   reading of the decompile would suggest. */
typedef struct
{
    ubyte unk0[0x88];               /* Ship+0x63C..0x6C4: unrelated tail storage */
    udword waitingFlag;             /* Ship+0x6C4: 1 while a wait is pending */
    vector fromPoint;               /* Ship+0x6C8: ship's position when wait started */
    real32 safeDistanceSquared;     /* Ship+0x6D4: squared safe-distance radius */
    vector toPoint;                 /* Ship+0x6D8: randomly chosen destination point */
    Timer timer;                    /* Ship+0x6E4: wait timer */
} ShipWaitingForShipsOverlay;

/* Beam damage state overlay for ships that support per-polygon beam damage.
   Located at a fixed offset from Ship base within ship-type tail storage. */
typedef struct
{
    real32 unk_968;         /* Ship+0x968: timestamp of last beam damage hit */
    ubyte  pad_96C[8];      /* Ship+0x96C..0x973 */
    real32 unk_974;         /* Ship+0x974: beam damage health accumulator */
    sdword unk_978;         /* Ship+0x978: beam segment destroyed flag */
} ShipBeamDamageState;

#define MAX_ID_NUMBER   65535

#define MAD_MAX_CUEABLE 2           //maximum number of animations that can be cued in a row

//Ship death flags for flag 'howDidIDie'
//howDidIDie == 0 ==> not dead, or didn't die by these means!
#define DEATH_Killed_By_Player          666 //another player killed this person (could be self..check other variable)
#define DEATH_Killed_By_Player_Explosion 667
#define DEATH_Killed_By_Kamikaze        668
#define DEATH_I_DIDNT_DIE_I_WAS_SALAVAGED 1004 //ship was salvaged!  So deal with gamestats accordingly
//non player dependent
#define DEATH_Killed_By_Collision       777 //ship raninto another ship/asteroid/object and died
#define DEATH_Killed_By_Cloud           888 //Dust Cloud damage kills a ship
#define DEATH_Killed_By_Dead_Player     999 //player that did killing is dead and unreferenceable
#define DEATH_BY_GAME_END               1000 //game ended, ship was freed
#define DEATH_BY_RETIREMENT             1001 //ship was retired by player
#define DEATH_BY_AFTER_DEAD             1002 //ship self destructed after player was already dead
#define DEATH_Left_Behind_At_Hyperspace 1003 //ship didn't finish docking in time for hyperspace

//If howDidIDie isn't set with the above, it is assumed to have been POOFED out of existence.
//#define DEATH_By_Poofing                888 //somewheres in the code this ship was poofed out of existence.

//ship struct "specialflags" defines
#define SPECIAL_StopForResearchDocking              BIT0
#define SPECIAL_Kamikaze                            BIT1
#define SPECIAL_BrokenFormation                     BIT2
#define SPECIAL_KamikazeCrazyFast                   BIT3
#define SPECIAL_SpeedBurst                          BIT4
#define SPECIAL_SpeedBurstCooling                   BIT5
#define SPECIAL_BurstFiring                         BIT6
#define SPECIAL_SinglePlayerWithinHyperSpaceRange   BIT7
#define SPECIAL_SinglePlayerLimitSpeed              BIT8
#define SPECIAL_BUSY_BECAUSE_SALVAGE_DOCKING        BIT9
#define SPECIAL_Harvested                           BIT10
#define SPECIAL_EXITING_DOOR                        BIT11
#define SPECIAL_STAY_TILL_EXPLICITLAUNCH            BIT12
#define SPECIAL_KasSpecialDamageModifier            BIT13
#define SPECIAL_Finished_Resource                   BIT14  //flag indicates when a resource collector has just finished collecting resources from one particular resource (computer player specific)
#define SPECIAL_Resourcing                          BIT15  //flag indicates when a resource collector has started resourcing - computer player specific
#define SPECIAL_SinglePlayerInParade                BIT16
#define SPECIAL_rowGettingOutOfWay                  BIT17
#define SPECIAL_ATTACKMOVECANCEL                    BIT18
#define SPECIAL_FriendlyStatus                      BIT19  //set by KAS - player's ships don't autoretaliate against this ship.  Also allows you to dock with them.
#define SPECIAL_IsASalvager                         BIT20  //used by junkyard dawg and salcap corvette...elminates double checks of the ship types!
#define SPECIAL_SalvagerHasSomethingAttachedAndIsDoingSomethingElse BIT21
#define SPECIAL_AttackFromAbove                     BIT22
#define SPECIAL_SalvagedTargetGoingIntoDockWithShip BIT23
#define SPECIAL_SalvageTakingHomeTechnology         BIT24
#define SPECIAL_Hyperspacing                        BIT25
#define SPECIAL_ForcedAttackStatus                  BIT26  //ship is in combat status, despite not actually attacking ships (must be turned off - doesn't expire)
#define SPECIAL_GravWellCaptured                    BIT28

#define SPECIAL_Launching                           BIT30  //indicates ship is launching
#define SPECIAL_ParadeNeedTomoveCloser              BIT31

// Unaudited Ship+0x1CC source bits that still need a retail consumer audit.
#define SPECIAL_2_BIT6_UNAUDITED                   BIT6
#define SPECIAL_2_BIT7_UNAUDITED                   BIT7

// Retail Cataclysm ship specialFlags2 bits at Ship+0x1CC.
#define SPECIAL_2_DontPlowThroughEnemyShips        BIT0
#define SPECIAL_2_DockFlag                         BIT3
#define SPECIAL_2_CrystalDockSlot                  BIT13  // retail InitCrystalDockSlots (0x4fc806): or [ship+1CCh],2000h
#define SPECIAL_2_SinglePlayerInParade            BIT16
#define SPECIAL_2_RowGettingOutOfWay              BIT17
#define SPECIAL_2_DisabledState                   BIT20
#define SPECIAL_2_NotBumpable                     BIT21
#define SPECIAL_2_SalvageTargetAlreadyDriven      BIT25
#define SPECIAL_2_SupportCapacityAccounted        BIT28
#define SPECIAL_2_FriendlyDockingOverride         BIT29

// Cataclysm-only runtime flags stored in the dword at Ship+0x1C8.
#define CATA_RUNTIME_KAMIKAZE                    BIT1
#define CATA_RUNTIME_BROKEN_FORMATION           BIT2
#define CATA_RUNTIME_KAMIKAZE_CRAZY_FAST        BIT3
#define CATA_RUNTIME_LAUNCHED_FROM_KAS         BIT23
#define CATA_RUNTIME_KAS_CHECK_DONE_LAUNCHING  BIT24
#define CATA_RUNTIME_ROW_GETTING_OUT_OF_WAY    BIT16
#define CATA_RUNTIME_ATTACKMOVECANCEL          BIT17
#define CATA_RUNTIME_GRAVWELL_CAPTURED         BIT28
#define CATA_RUNTIME_IN_UNIT_CAPS                 BIT29
#define CATA_RUNTIME_CIRCULAR_GUARD               BIT30
#define CATA_RUNTIME_ATTACK_FROM_ABOVE          BIT20  // SetupExtraAttackInfo's specialFlags1C8 equivalent of HW1's SPECIAL_AttackFromAbove (see InitExtraAttackInfo)
#define CATA_RUNTIME_SALVAGE_CLAMP_APPROACH    BIT19  // Set while a salvage/crystal-harvest worker is closing on its tracked target's clamp point; collCheckShipShipColl (0x43ade0) checks this + SalvagerFunc(ship)'s trackedTarget/unk_10 to suppress ship-vs-target collision during the overlap-by-design clamp approach

#define SPECIAL3_Unknown0                           BIT0
#define SPECIAL3_Unknown1                           BIT1
#define SPECIAL3_Unknown7                           BIT7
#define SPECIAL3_Unknown12                          BIT12
#define SPECIAL3_Unknown16                          BIT16
#define SPECIAL3_Unknown19                          BIT19
#define SPECIAL3_Unknown21                          BIT21
#define SPECIAL3_Unknown27                          BIT27
#define SPECIAL3_LaunchedFromKas                    BIT23  // 0x800000
#define SPECIAL3_KasCheckDoneLaunching              BIT24  // 0x1000000
#define SPECIAL3_Unknown30                          BIT30

#define SPECIAL4_ExternalBuildMember                BIT3
#define SPECIAL4_DockingClamp                       BIT6
#define SPECIAL4_NaggarokEMP                        BIT14
#define SPECIAL4_Unknown15                          BIT15
#define SPECIAL4_Unknown17                          BIT17
#define SPECIAL4_Unknown18                          BIT18
#define SPECIAL4_Unknown20                          BIT20
#define SPECIAL4_Unknown25                          BIT25
#define SPECIAL4_SupportCapacityAccounted           BIT28
#define SPECIAL4_Unknown29                          BIT29

struct Resource;
typedef struct SpaceObjSelection SpaceObjSelection;
typedef struct Ship         // Ship object
{
    Node objlink;
    ObjType objtype;                // object type (ship or bullet, etc)
    udword flags;                   // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added
    udword unk_1C;              // Cataclysm-added
    ShipStaticInfo *staticinfo;         // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;               // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];             // Cataclysm-added
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;

    //must come next to be compatible with Derelict in aiship stuff
    ubyte aistate;                  // AI state variable
    ubyte pad1[2];
    sbyte putOnDoor;
    ClampInfo *clampInfo;           // at 0x1BC, confirmed via binary (updateClampedObject)
    bitflag32 specialFlags;         // retail Ship+0x1C0 flag word; low byte is also used by AI fly-state helpers
    struct Ship *dockingCollisionTarget; // retail Ship+0x1C4 saved as a SpaceObj ref and used by docking collision / order gating
    udword specialFlags1C8;         // retail Cataclysm Ship+0x1C8 flag word
    udword specialFlags2;           // retail Cataclysm second general-purpose ship flag word at 0x1CC
    udword unk_1D0;
    udword cataResourcingNeedsReset;// at 0x1D4
    void *cataResourcingTarget;     // at 0x1D8
    ShipCataBusyStateWindow cataBusyStateWindow; // 0x1DC..0x213 retail busy-state window scanned by EMP / delayed-death paths
    udword specialFlags3;           //Cataclysm general purpose ship flags
    udword specialFlags4;           //Cataclysm general purpose ship flags
    sdword specialFlagsUnknown2;
    sdword specialFlagsUnknown3;
    sdword specialFlagsUnknown4;
    sdword specialFlagsUnknown5;
    SpaceObjSelection *selection22C; // runtime/save selection list at 0x22C; retail alliance/formation/ping paths iterate numSpaceObjs and SpaceObjPtr[] here
    sdword specialFlagsUnknown7;
    NAVLightInfo *navLightInfo;     // at 0x234, confirmed via RenderNAVLights / univFreeShipContents
    udword salcapTargetClaimOwner; // salcap target-claim owner index (0xFFFFFFFF = unclaimed)
    // HERE UP SAME for Derelicts and Ships
    Node shiplink;
    vector moveTo;                  // move destination of the ship
    vector moveFrom;                // where the ship is moving from
    vector formationOffset;         // offset in formation
    vector formationHeading;        // heading in formation (optional)
    vector engineOffset;    // engine nozzle offset from position in object coordinates.
    vector enginePosition;  // engine position in universe
    udword unk_290;                 // retail-only runtime dword before right-of-way state; semantics unresolved
    struct Ship *rowGetOutOfWay;    // Right of way - ship to get out of way of
    vector rowOriginalPoint;        // Right of way - my original point
    vector rowOutOfWayPoint;        // Right of way - point to go to be out of way
    ShipType shiptype;
    ShipRace shiprace;
    //*******Tactics stuff start
    //FIX LATER to support fewer variables using bitmasks
    TacticsType tacticstype;        // ships current tactics setting
    bool32 isDodging;               // retail missile/attack code reads dodge flag from +0x2BC
    real32 DodgeTime;               //variable used to control when a ship will stop dodging
    uword  DodgeDir;
    uword  pad2;
    bool32 tacticsFormationVar1;      //variable for formation attacking
    udword DodgeFlag;               //info flag for dodging
    sdword tactics_ordertype;       //ships order type convulved into the Tactics Domain
    udword tacticsTalk;             //var used for intership comunication
    sdword tacticsNeedToFlightMan;  //same as above
    //*******Tactics stuff end
    sdword dockwaitforNorm;
    real32 timeCreated;
    sdword kamikazeState;
    vector kamikazeVector;
    real32 speedBurstTime;
    real32 singlePlayerSpeedLimiter;  // limiter of ships maxvelocity, single player game only!
    real32 deathtime;       // time at which this ship should blow up, if SOF_Crazy is set
    //mad animation variabes
    udword madAnimationFlags;       //general purpose flags for mad animations
    udword madGunStatus;            //gun status
    udword madWingStatus;           //wing animation status
    udword madDoorStatus;
    udword madSpecialStatus;
    udword cuedAnimationIndex;            //Animation index waiting in the wings to be played
    udword cuedAnimationType;      //cue of animation types (guns, wings, etc...)
    udword nextAnim;
    udword soundEventAnimationTypeFlag;    //opening,closing,damaged_opening,damaged_closing
    sdword forceCancelDock;
    real32 gravwellTimeEffect;      //time the gravwell effect sould be played again
    struct Player *playerowner;
    sdword resources;
    real32 fuel;
    real32 lastTimeRepaired;
    real32 magnitudeSquared;        // most recent scaling factor
    struct Ship *gettingrocked;     // ship which is rocking you (shooting you)
    struct Ship *recentAttacker;    // ship which has attacked you recently (ie, not just this AI tick) -- see recentlyAttacked
    struct Ship *firingAtUs;        // ship which has recently fired upon us (but needn't have hit) see recentlyFiredUpon
    bool8 shipidle;                 // flag to indicate if ship has no command to execute
    bool8 autostabilizeship;        // if TRUE, we should stabilize the ship
    ubyte autostabilizestate;       // state of autostabilization
    ubyte autostabilizewait;        // how many frames to wait before stabilizing in autostabilize
    ubyte aistateattack;            // AI attack state variable
    ubyte flightmanState1;          // generic state variables for flight maneuvers
    ubyte flightmanState2;          // generic state variables for flight maneuvers
    ubyte rcstate1;                 // state variable for resource collecting
    ubyte rcstate2;                 // state variable for resource collecting
    ubyte aistatecommand;
    sbyte dontrotateever;
    sbyte dontapplyforceever;
    bool8 shipisattacking;
    sbyte colorScheme;              // what color scheme does this ship use?
    ubyte recentlyAttacked;         // counts down from RECENT_ATTACK_DURATION -- see recentAttacker
    ubyte recentlyFiredUpon;        // same, see firingAtUs
    ubyte chatterBits;              // flags for battle chatter
    ubyte rowState;                 // state variable for right of way
    real32 aidescend;               // AI variable for capital ships descending
    udword flightman;               // current flight maneuver handle
    void *flightmanInfo;            // pointer to any additional flight maneuver info
    struct CommandToDo *formationcommand;   // pointer to formationcommand, if ship in formation
    struct CommandToDo *command;    // pointer to ship's current command, if any
    SpecificAttackVars attackvars;  // attack information
    struct Effect *rceffect;        // effect variable for resource collecting
    void (*ShipXHarvestsResourceY)(struct Ship *ship,struct Resource *resource);    // function pointer for resource collecting
    sdword visibleToWho;              //bitmask flag indicating to which players this ship is visible to...relevant only if ship is cloaked
    sdword visibleToWhoPreviousFrame; //same as above, but relevant for the previous univupdate frame
    sdword howDidIDie;              //defined above: Set when a ship is set to die, that way, at somepoint (hopefully) we can tell how the bitch died.
    sdword whoKilledMe;             //playerIndex of the player that killed this ship
    udword nDamageLights;
    DamageLightStatic* damageLights;
    struct Effect* showingDamage[3][3];
    vector holdingPatternPos;
    real32 damageModifier;
    real32 passiveAttackCancelTimer;

    //later will make most of these into a single flag
    udword tractorbeaminfo;
    real32 buildTimerElapsed;
    struct Ship *specialTargetShip;
    ShipUpgradeState upgradeState;
    sdword tractorstate;            //info variable for tractorbeaming state
    sdword hyperspacePing;          //true or false flag indicating if the ship has had a hyperspace ping created for it...NON-deterministic!
    sdword tractorPosinfo[4];        //more tractor info!
    udword unk_410;
    udword unk_414;
    udword unk_418;

    sdword needNewLeader;
    struct Ship *newDockWithTransfer;
    sdword newDockIndexTransfer;

    real32 shipDeCloakTime;         //time is set when a ship decloaks...this time is used to give decloaking ships an advantage
    shiptrail* trail[MAX_NUM_TRAILS];   //ship's engine trail(s)
    ShipID shipID;
    uword hotKeyGroup;              // what hot-key group this ship belongs to
    sdword retaliateSpeechVar;
    real32 retaliateSpeechTime;

    SOUNDEVENT soundevent;
    udword burstGunIndex;          // retail Ship+0x48C stores the gunindex that owns the active burst loop
    union
    {
        udword unk_490;
        void *runtimeData490;
        GunInfo *cataclysmGunInfo;      // retail Ship+0x490 runtime block: gun/module-slot state starts here
    };
    union
    {
        ShipDockRuntime dockvars;       // retail dock runtime begins at Ship+0x494 and its launchpoints slot overlaps Ship+0x4B4
        struct
        {
            ubyte unk_494_4B3[0x20];
            ShipsInsideMe *shipsInsideMe; // retail Ship+0x4B4 pointer consumed by dock/launch paths
        };
    };
    union
    {
        struct
        {
            udword unk_4B8;
            udword unk_4BC;
            udword unk_4C0;
        };
        ShipRuntimeAnimationOverlay runtimeAnimationOverlay;
        struct
        {
            struct shipbindings *bindings;   // typed access to unk_4B8 (Cataclysm-correct offset)
            struct madanim *madBindings;     // typed access to unk_4BC (Cataclysm-correct offset)
            udword unk_4C0_b;
        };
    };
    struct ShipSinglePlayerGameInfo *shipSinglePlayerGameInfo;     // stuff for the single player game
    lightning *lightning[2];        // ship lightning effect handles
    union
    {
        struct
        {
            Node slavelink;                 // legacy HW1 view of this overlay region
            SlaveInfo *slaveinfo;
            void *legacy_unk_4E0_ptr;
            DockInfo *dockInfo;
            void *unk_4EC_ptr;              // mislocated HW1 shipsInsideMe name lived here; Cataclysm meaning unresolved
            udword unk_4E8;
            udword unk_4EC;
            udword overlayRaw_4F8;          // overlaps beastAttack.nextChainTime in Cataclysm
            udword overlayRaw_4FC;          // overlaps beastAttack.effect in Cataclysm
            udword unk_500;
        };
        struct
        {
            ShipStatusWindowOverlay statusWindow; // generic Cataclysm phase/timer window
            ShipBeastAttackOverlay beastAttack;   // Beast infect/conversion/cascade tail
        };
    };
    union
    {
        struct
        {
            udword unk_504;
            udword unk_508;
            udword unk_50C;
            udword unk_510;
            udword unk_514;
        };
        struct
        {
            ubyte replacementStateActive;
            ubyte replacementStatePad_505[3];
            real32 replacementStateStartTime;
            real32 replacementStateEndTime;
            void *replacementEffect;
            ubyte replacementColorScheme;
            ubyte replacementStatePad_515[3];
        };
    };
    udword unk_518;
    union
    {
        struct
        {
            real32 linkCombinedHealth;      // retail processLinkToDo caches merged health at 0x51C
            real32 linkSourceShipHealth0;   // retail processLinkToDo stores source ship 0 health at 0x520
            real32 linkSourceShipHealth1;   // retail processLinkToDo stores source ship 1 health at 0x524
        };
        struct
        {
            real32 unk_51C;
            real32 unk_520;
            real32 unk_524;
        };
    };
    bool32 disableStateStarted;
    real32 disableStateElapsed;
    bool32 disableStateActive;
    Timer disableStateTimer;
    bool32 disableStateSavedFlag23;
    bool32 disableStateSavedFlag21;
    bool32 disableStateSavedAutostabilize;
    bool32 velocityFreezeActive;
    vector velocityFreezeInitialVelocity;
    Timer velocityFreezeTimer;
    union
    {
        ShipCrystalProcessingOverlay crystalProcessingState; // crystal-processing overlay across 0x570..0x594
        struct
        {
            ubyte crystalProcessingPad_570_58F[0x20];
            udword unk_590; // retail shipIsCloaking flag at 0x590; Dreadnought/gun code also gates on value 1
        };
    };
    bool32 upgradeActive;          // retail clUpgrade / univShipHouseKeep pair at 0x598
    real32 upgradeStartTime;       // retail clUpgrade writes the start timestamp at 0x59C
    real32 upgradeEndTime;         // retail clUpgrade writes the completion timestamp at 0x5A0
    vector lastDockApproachHeading;
    udword unk_5B0;
    Timer buildTimer;
    real32 speedThrottle;
    ShipDamageLevelCounters damageLevelCounters;
    udword experienceLevel;
    udword unk_5FC;
    udword unk_600;
    udword unk_604;
    struct Ship *salCapDockApproachHost;
    udword unk_60C;
    bool32 cataBurstFiring;        // retail gunCanShoot / univShipHouseKeep ship-level burst-fire flag at 0x610
    union
    {
        udword unk_614;
        struct
        {
            ubyte unk_614_0;
            sbyte crystalDockSlotIndex;   // crystal dock slot assignment (-1 = none)
            ubyte unk_614_2;
            ubyte unk_614_3;
        };
    };
    udword unk_618;
    udword unk_61C;
    real32 dockStateStartTime;
    union
    {
        Timer linkTransformTimer;
        struct
        {
            udword unk_624;
            udword unk_628;
            udword unk_62C;
            udword unk_630;
        };
    };
    udword unk_634;
    udword ShipSpecifics[1];          // per-ship custom struct payload; must stay last
} Ship;

#define SHIP_CATA_BUSY_STATE_WORD_COUNT 13
#define SHIP_CATA_BUSY_STATE_DISABLE_LATCH_INDEX 9

static __inline Ship *shipDockingCollisionTarget(const Ship *ship)
{
    return ship->dockingCollisionTarget;
}

static __inline bool32 shipSharesDockingCollisionTarget(const Ship *ship, const Ship *other)
{
    Ship *target = shipDockingCollisionTarget(ship);

    return target != NULL && target == shipDockingCollisionTarget(other);
}

static __inline bool32 shipHasCataBusyState(const Ship *ship)
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

static __inline Ship *shipDockingTarget(const Ship *ship)
{
    return ship->cataBusyStateWindow.dockingship;
}

static __inline bool32 shipSharesDockingTarget(const Ship *ship, const Ship *other)
{
    Ship *target = shipDockingTarget(ship);

    return target != NULL && target == shipDockingTarget(other);
}

static inline WorkerShipSpecifics *workerShipSpecifics(Ship *ship)
{
    return (WorkerShipSpecifics *)ship->ShipSpecifics;
}

static inline const WorkerShipSpecifics *workerShipSpecificsConst(const Ship *ship)
{
    return (const WorkerShipSpecifics *)ship->ShipSpecifics;
}

static inline AcolyteSpecifics *acolyteShipSpecifics(Ship *ship)
{
    return (AcolyteSpecifics *)ship->ShipSpecifics;
}

static inline const AcolyteSpecifics *acolyteShipSpecificsConst(const Ship *ship)
{
    return (const AcolyteSpecifics *)ship->ShipSpecifics;
}

/* Fixed byte offsets of the runtime overlays accessed below.  Kept as named
   constants so the canonical accessors carry no bare offset literals; the
   values are pinned by the static_asserts further down. */
enum
{
    kShipLaunchInitSpecificsOverlayOffset = 0x18,  /* within ShipSpecifics (Ship+0x650) */
    kShipBeamDamageStateOverlayOffset     = 0x968, /* within Ship */
};

static inline ShipLaunchInitSpecificsOverlay *shipLaunchInitSpecifics(Ship *ship)
{
    return (ShipLaunchInitSpecificsOverlay *)((ubyte *)ship->ShipSpecifics + kShipLaunchInitSpecificsOverlayOffset);
}

static inline const ShipLaunchInitSpecificsOverlay *shipLaunchInitSpecificsConst(const Ship *ship)
{
    return (const ShipLaunchInitSpecificsOverlay *)((const ubyte *)ship->ShipSpecifics + kShipLaunchInitSpecificsOverlayOffset);
}

static inline GunStateSpecifics *gunStateSpecifics(Ship *ship)
{
    return (GunStateSpecifics *)ship->ShipSpecifics;
}

static inline const GunStateSpecifics *gunStateSpecificsConst(const Ship *ship)
{
    return (const GunStateSpecifics *)ship->ShipSpecifics;
}

static inline LinkableSpecifics *linkableSpecifics(Ship *ship)
{
    return (LinkableSpecifics *)ship->ShipSpecifics;
}

static inline const LinkableSpecifics *linkableSpecificsConst(const Ship *ship)
{
    return (const LinkableSpecifics *)ship->ShipSpecifics;
}

static inline SentinelSpecifics *sentinelShipSpecifics(Ship *ship)
{
    return (SentinelSpecifics *)ship->ShipSpecifics;
}

static inline const SentinelSpecifics *sentinelShipSpecificsConst(const Ship *ship)
{
    return (const SentinelSpecifics *)ship->ShipSpecifics;
}

static inline HiveFrigateSpecifics *hiveFrigateSpecifics(Ship *ship)
{
    return (HiveFrigateSpecifics *)ship->ShipSpecifics;
}

static inline const HiveFrigateSpecifics *hiveFrigateSpecificsConst(const Ship *ship)
{
    return (const HiveFrigateSpecifics *)ship->ShipSpecifics;
}

static inline ShipModuleSlotRuntime *shipModuleSlotRuntime(Ship *ship)
{
    return (ShipModuleSlotRuntime *)((ubyte *)ship + sizeof(Ship));
}

static inline const ShipModuleSlotRuntime *shipModuleSlotRuntimeConst(const Ship *ship)
{
    return (const ShipModuleSlotRuntime *)((const ubyte *)ship + sizeof(Ship));
}

static inline ShipModuleSlotIconState *shipModuleSlotIconState(Ship *ship)
{
    return shipModuleSlotRuntime(ship)->iconState;
}

static inline const ShipModuleSlotIconState *shipModuleSlotIconStateConst(const Ship *ship)
{
    return shipModuleSlotRuntimeConst(ship)->iconState;
}

static inline ShipSwarmerRuntimeOverlay *shipSwarmerRuntime(Ship *ship)
{
    return (ShipSwarmerRuntimeOverlay *)((ubyte *)ship + sizeof(Ship));
}

static inline const ShipSwarmerRuntimeOverlay *shipSwarmerRuntimeConst(const Ship *ship)
{
    return (const ShipSwarmerRuntimeOverlay *)((const ubyte *)ship + sizeof(Ship));
}

static inline ShipCollisionPolyOverlay *shipCollisionPolyOverlay(Ship *ship)
{
    return (ShipCollisionPolyOverlay *)((ubyte *)ship + sizeof(Ship));
}

static inline ShipWaitingForShipsOverlay *shipWaitingForShipsOverlay(Ship *ship)
{
    return (ShipWaitingForShipsOverlay *)((ubyte *)ship + sizeof(Ship));
}

static inline ShipBeamDamageState *shipBeamDamageState(Ship *ship)
{
    return (ShipBeamDamageState *)((ubyte *)ship + kShipBeamDamageStateOverlayOffset);
}

typedef Ship *ShipPtr;

static_assert(sizeof(ShipRuntimeAnimationOverlay) == 0x0C);
static_assert(sizeof(ShipStatusWindowOverlay) + sizeof(ShipBeastAttackOverlay) == 0x34);
static_assert(sizeof(ShipCrystalProcessingOverlay) == 0x28);
static_assert(sizeof(ShipBeastCascadeWeaponState) == 0x18);
static_assert(sizeof(ShipUpgradeState) == 0x08);
static_assert(sizeof(ShipModuleSlotBuildTarget) == 0x2C);
static_assert(offsetof(Ship, unk_290) == 0x290);
static_assert(offsetof(Ship, dontrotateever) == 0x356);
static_assert(offsetof(Ship, dontapplyforceever) == 0x357);
static_assert(offsetof(Ship, shipisattacking) == 0x358);
static_assert(offsetof(Ship, colorScheme) == 0x359);
static_assert(offsetof(Ship, recentlyAttacked) == 0x35A);
static_assert(offsetof(Ship, recentlyFiredUpon) == 0x35B);
static_assert(offsetof(Ship, chatterBits) == 0x35C);
static_assert(offsetof(Ship, rowState) == 0x35D);
static_assert(offsetof(Ship, aidescend) == 0x360);
static_assert(offsetof(ShipModuleSlotRuntime, iconState) == 0x60);
static_assert(offsetof(ShipModuleSlotRuntime, slotAvailability) == 0xA8);
static_assert(offsetof(ShipModuleSlotRuntime, activeBuildTarget) == 0xC8);
static_assert(offsetof(ShipModuleSlotRuntime, pendingBuildTarget) == 0xF4);
static_assert(offsetof(ShipModuleSlotRuntime, morphProgress) == 0x120);
static_assert(offsetof(ShipModuleSlotRuntime, state) == 0x124);
static_assert(offsetof(ShipModuleSlotRuntime, transitionTime) == 0x128);
static_assert(offsetof(ShipModuleSlotRuntime, transitionArmed) == 0x12C);
static_assert(offsetof(ShipModuleSlotRuntime, activeEffect) == 0x130);
static_assert(sizeof(ShipModuleSlotRuntime) == 0x134);
static_assert(offsetof(ShipSwarmerRuntimeOverlay, swarmerReadyState) == 0x9C4);
static_assert(sizeof(Ship) + offsetof(ShipCollisionPolyOverlay, polyCount) == 0x640);
static_assert(sizeof(Ship) + offsetof(ShipCollisionPolyOverlay, polyEntries) == 0x644);
static_assert(sizeof(Ship) + offsetof(ShipWaitingForShipsOverlay, waitingFlag) == 0x6C4);
static_assert(sizeof(Ship) + offsetof(ShipWaitingForShipsOverlay, fromPoint) == 0x6C8);
static_assert(sizeof(Ship) + offsetof(ShipWaitingForShipsOverlay, safeDistanceSquared) == 0x6D4);
static_assert(sizeof(Ship) + offsetof(ShipWaitingForShipsOverlay, toPoint) == 0x6D8);
static_assert(sizeof(Ship) + offsetof(ShipWaitingForShipsOverlay, timer) == 0x6E4);
static_assert(kShipBeamDamageStateOverlayOffset == 0x968);
static_assert(kShipLaunchInitSpecificsOverlayOffset == 0x18);
static_assert(0x968 + offsetof(ShipBeamDamageState, unk_968) == 0x968);
static_assert(0x968 + offsetof(ShipBeamDamageState, unk_974) == 0x974);
static_assert(0x968 + offsetof(ShipBeamDamageState, unk_978) == 0x978);
static_assert(sizeof(Ship) == 0x63C);
static_assert(offsetof(Ship, flags2) == 0x18);
static_assert(offsetof(Ship, staticinfo) == 0x20);
// Confirmed via retail univHandleShipCollision (0x545540): "test ah,2" on the DWORD
// read at [esi+38h] tests the high byte of the sword attributesParam (bit 9 = 0x0200
// = ATTRIBUTES_HeadShotVelToMothership), not the uword attributes field at 0x36.
// posinfo/posinfo.velocity confirmed via the same function's [esi+58h]/[esi+64h] reads.
static_assert(offsetof(Ship, currentLOD) == 0x34);
static_assert(offsetof(Ship, renderedLODs) == 0x35);
static_assert(offsetof(Ship, attributes) == 0x36);
static_assert(offsetof(Ship, attributesParam) == 0x38);
static_assert(offsetof(Ship, posinfo) == 0x58);
static_assert(offsetof(Ship, posinfo.velocity) == 0x64);
static_assert(offsetof(Ship, health) == 0x170);
static_assert(offsetof(Ship, nonstatvars) == 0x17C);
static_assert(offsetof(Ship, flashtimer) == 0x174);
static_assert(offsetof(Ship, lasttimecollided) == 0x178);
static_assert(offsetof(SpaceObjRotImpTarg, flashtimer) == offsetof(Ship, flashtimer));
static_assert(offsetof(SpaceObjRotImpTarg, lasttimecollided) == offsetof(Ship, lasttimecollided));
static_assert(offsetof(SpaceObjRotImpTarg, nonstatvars) == offsetof(Ship, nonstatvars));
static_assert(offsetof(Ship, aistate) == 0x1B8);
static_assert(offsetof(Ship, clampInfo) == 0x1BC);  // binary confirmed via updateClampedObject
static_assert(offsetof(Ship, specialFlags) == 0x1C0);
static_assert(offsetof(Ship, dockingCollisionTarget) == 0x1C4);
static_assert(offsetof(Ship, specialFlags1C8) == 0x1C8);
static_assert(offsetof(Ship, specialFlags2) == 0x1CC);
static_assert(offsetof(Ship, cataBusyStateWindow) + offsetof(ShipCataBusyStateWindow, busyStatePtr1EC) == 0x1EC);
static_assert(offsetof(Ship, cataBusyStateWindow) + offsetof(ShipCataBusyStateWindow, dockingship) == 0x1F0);
static_assert(offsetof(Ship, specialFlags3) == 0x210);
static_assert(offsetof(Ship, navLightInfo) == 0x230);
static_assert(offsetof(Ship, salcapTargetClaimOwner) == 0x234);
static_assert(offsetof(Ship, shiplink) == 0x238);
static_assert(offsetof(Ship, shiplink) + offsetof(Node, structptr) == 0x244);
static_assert(offsetof(ShipStaticInfo, shipclass) == 0x1E8);
static_assert(offsetof(ShipStaticInfo, shipFlags) == 0x1EC);
static_assert(offsetof(ShipStaticInfo, externalBuild) == 0x1F0);
static_assert(offsetof(Ship, nonstatvars) == offsetof(SpaceObjRotImpTargGuidanceShipDerelict, nonstatvars));
static_assert(offsetof(SpaceObjRotImpTargGuidance, clampInfo) == 0x1BC);
static_assert(offsetof(Ship, clampInfo) == offsetof(SpaceObjRotImpTargGuidanceShipDerelict, clampInfo));
static_assert(offsetof(Ship, specialFlags) == offsetof(SpaceObjRotImpTargGuidanceShipDerelict, specialFlags));
static_assert(offsetof(Ship, formationcommand) == 0x36C);
static_assert(offsetof(Ship, command) == 0x370);
static_assert(offsetof(Ship, attackvars) == 0x374);
static_assert(offsetof(Ship, attackvars) + offsetof(SpecificAttackVars, attacktarget) == 0x378);
static_assert(offsetof(Ship, attackvars) + offsetof(SpecificAttackVars, myLeaderIs) == 0x37C);      // IDA: SetupExtraAttackInfo
static_assert(offsetof(Ship, attackvars) + offsetof(SpecificAttackVars, myWingmanIs) == 0x380);     // IDA: SetupExtraAttackInfo
static_assert(offsetof(Ship, attackvars) + offsetof(SpecificAttackVars, flightmansLeft) == 0x384);  // IDA: SetupExtraAttackInfo
static_assert(offsetof(Ship, attackvars) + offsetof(SpecificAttackVars, attacksituation) == 0x386); // IDA: SetupExtraAttackInfo
static_assert(offsetof(Ship, attackvars) + offsetof(SpecificAttackVars, attackevents) == 0x388);    // IDA: SetupExtraAttackInfo
static_assert(offsetof(Ship, rceffect) == 0x38C);
static_assert(offsetof(Ship, buildTimerElapsed) == 0x3E8);
static_assert(offsetof(Ship, upgradeState) == 0x3F0);
static_assert(offsetof(Ship, upgradeState.mask) == 0x3F4);
static_assert(offsetof(Ship, tractorPosinfo) == 0x400);
static_assert(offsetof(Ship, soundevent) == 0x448);
static_assert(offsetof(Ship, soundevent) + offsetof(SOUNDEVENT, specialHandle) == 0x458);
static_assert(offsetof(Ship, selection22C) == 0x228);
static_assert(offsetof(Ship, burstGunIndex) == 0x48C);
static_assert(offsetof(Ship, runtimeData490) == 0x490);
static_assert(offsetof(Ship, dockvars) == 0x494);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, dockship) == 0x494);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, coneDistance) == 0x498);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, dockLaunchState) == 0x49C);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, dockPointIndex) == 0x4A0);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, dockStateDeadline) == 0x4A4);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, dockPointActivity) == 0x4A8);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, dockstaticpoint) == 0x4AC);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, customdockinfo) == 0x4B0);
static_assert(offsetof(Ship, dockvars) + offsetof(ShipDockRuntime, launchpoints) == 0x4B4);
static_assert(offsetof(Ship, shipsInsideMe) == 0x4B4);

static_assert(offsetof(Ship, bindings) == 0x4B8);
static_assert(offsetof(Ship, madBindings) == 0x4BC);
static_assert(offsetof(Ship, shipSinglePlayerGameInfo) == 0x4C4);
static_assert(offsetof(Ship, lightning) == 0x4C8);
static_assert(offsetof(Ship, runtimeAnimationOverlay) == 0x4B8);
static_assert(sizeof(ShipStatusWindowOverlay) == 0x18);
static_assert(sizeof(ShipBeastAttackOverlay) == 0x1C);
static_assert(offsetof(Ship, statusWindow) == 0x4D0);
static_assert(offsetof(Ship, statusWindow.chainHitCount) == 0x4E0);
static_assert(offsetof(Ship, statusWindow.activeUntil) == 0x4E4);
static_assert(offsetof(Ship, overlayRaw_4F8) == 0x4F8);
static_assert(offsetof(Ship, overlayRaw_4FC) == 0x4FC);
static_assert(offsetof(Ship, beastAttack) == 0x4E8);
static_assert(offsetof(Ship, beastAttack.sourceShip) == 0x4E8);
static_assert(offsetof(Ship, beastAttack.sourcePlayer) == 0x4EC);
static_assert(offsetof(Ship, beastAttack.raw_4EC) == 0x4EC);
static_assert(offsetof(Ship, beastAttack.raw_4F0) == 0x4F0);
static_assert(offsetof(Ship, beastAttack.raw_4F4) == 0x4F4);
static_assert(offsetof(Ship, beastAttack.nextChainTime) == 0x4F8);
static_assert(offsetof(Ship, beastAttack.effect) == 0x4FC);
static_assert(offsetof(Ship, beastAttack.effectColorScheme) == 0x500);
static_assert(offsetof(Ship, unk_518) == 0x518);
static_assert(offsetof(Ship, velocityFreezeActive) == 0x550);
static_assert(offsetof(Ship, velocityFreezeInitialVelocity) == 0x554);
static_assert(offsetof(Ship, velocityFreezeTimer) == 0x560);
static_assert(offsetof(Ship, crystalProcessingState) == 0x570);
static_assert(offsetof(Ship, crystalProcessingState) + offsetof(ShipCrystalProcessingOverlay, effect) == 0x594);
static_assert(offsetof(Ship, unk_590) == 0x590);
static_assert(offsetof(Ship, lastDockApproachHeading) == 0x5A4);
static_assert(offsetof(Ship, buildTimer) == 0x5B4);
static_assert(offsetof(Ship, speedThrottle) == 0x5C4);
static_assert(offsetof(Ship, salCapDockApproachHost) == 0x608);
static_assert(offsetof(Ship, cataBurstFiring) == 0x610);
static_assert(offsetof(Ship, dockStateStartTime) == 0x620);
static_assert(offsetof(Ship, linkTransformTimer) == 0x624); // confirmed via retail disasm: lea ecx/edx,[reg+624h] in processLinkedLaunchToDo (0x468650)
static_assert(offsetof(Ship, unk_634) == 0x634);
static_assert(offsetof(Ship, ShipSpecifics) == 0x638);
static_assert(offsetof(Ship, rowGetOutOfWay) == 0x294);
static_assert(offsetof(Ship, rowOriginalPoint) == 0x298);
static_assert(offsetof(Ship, rowOutOfWayPoint) == 0x2A4);
static_assert(offsetof(Ship, shiptype) == 0x2B0);    // IDA: formGetShipSlot reads *(DWORD*)(ship + 0x2B0)
static_assert(offsetof(Ship, shiprace) == 0x2B4);    // IDA: CreateRepairDroid reads *(DWORD*)(ship + 0x2B4)
static_assert(offsetof(Ship, tacticstype) == 0x2B8); // IDA: shouldMissileLoseTarget reads *(DWORD*)(ship + 0x2B8)
static_assert(offsetof(Ship, isDodging) == 0x2BC);   // IDA: shouldMissileLoseTarget reads *(DWORD*)(ship + 0x2BC)
// IDA pseudocode mis-labeled these two writes (false-friend field names, same pattern as the
// isDodging/tacticstype mixup above): disasm of SetupExtraAttackInfo (0x447fb8/0x447fbe) clears
// only ship+0x2D4 and ship+0x2D8, matching HW1's InitExtraAttackInfo which clears exactly
// tacticsTalk and tacticsNeedToFlightMan (not dockwaitforNorm) for a fresh attack order.
static_assert(offsetof(Ship, tacticsTalk) == 0x2D4);            // IDA: SetupExtraAttackInfo clears *(DWORD*)(ship + 0x2D4)
static_assert(offsetof(Ship, tacticsNeedToFlightMan) == 0x2D8); // IDA: SetupExtraAttackInfo clears *(DWORD*)(ship + 0x2D8)
static_assert(offsetof(Ship, staticinfo) == 0x20); // IDA: shipAddSupportUsage reads *(DWORD*)(ship + 0x20) + {0x410,0x414}
static_assert(offsetof(Ship, playerowner) == 0x32C);
// Binary: dockStaticInfo is at offset 0x3F0 in ShipStaticInfo (from scriptSetDockStatics: *(_DWORD *)(a3 + 1008) = v3)
static_assert(sizeof(CustShipHeader) == 0x8C);
static_assert(offsetof(ShipStaticInfo, custshipheader) == 0x108);
static_assert(offsetof(ShipStaticInfo, custstatinfo) == 0x194);
static_assert(offsetof(ShipStaticInfo, navlightStaticInfo) == 0x104);
// IDA: analyze_struct_detailed(ShipStaticInfo) + universe.c script-table registration of
// "collSideModifiers[TRANS_FORWARD]" confirm these are real fields, not a mislabeled passiveRetaliateZone
static_assert(offsetof(ShipStaticInfo, passiveRetaliateZone) == 0x1C4);
static_assert(offsetof(ShipStaticInfo, collSideModifiers) == 0x1C8);
static_assert(offsetof(ShipStaticInfo, shiptype) == 0x1E0);
static_assert(offsetof(ShipStaticInfo, shiprace) == 0x1E4);
static_assert(offsetof(ShipStaticInfo, shipclass) == 0x1E8);
static_assert(offsetof(ShipStaticInfo, shipFlags) == 0x1EC);
static_assert(offsetof(ShipStaticInfo, externalBuild) == 0x1F0);
static_assert(offsetof(ShipStaticInfo, forceFieldProtectionDistanceScale) == 0x1F4);
static_assert(offsetof(ShipStaticInfo, upgradeTimeModifier) == 0x1F8);
static_assert(offsetof(ShipStaticInfo, canBuildShips) == 0x1FC);
static_assert(offsetof(ShipStaticInfo, rotateToRetaliate) == 0x204);
static_assert(offsetof(ShipStaticInfo, passiveAttackPenaltyExempt) == 0x20C);
static_assert(offsetof(ShipStaticInfo, specialActivateIsContinuous) == 0x210);
static_assert(offsetof(ShipStaticInfo, canSpecialBandBoxFriendlies) == 0x211);
static_assert(offsetof(ShipStaticInfo, unk4) == 0x212);
static_assert(offsetof(ShipStaticInfo, unk5) == 0x213);
static_assert(offsetof(ShipStaticInfo, canSingleClickSpecialActivate) == 0x214);
static_assert(offsetof(ShipStaticInfo, repairBeamCapable) == 0x224);
static_assert(offsetof(ShipStaticInfo, buildCost) == 0x238);
static_assert(offsetof(ShipStaticInfo, buildTime) == 0x23C);
static_assert(offsetof(ShipStaticInfo, fowRadius) == 0x240);
static_assert(offsetof(ShipStaticInfo, svManeuverability) == 0x244);
static_assert(offsetof(ShipStaticInfo, svCoverage) == 0x248);
static_assert(offsetof(ShipStaticInfo, svFirePower) == 0x24C);
static_assert(offsetof(ShipStaticInfo, RKOEatMeTime) == 0x250);
static_assert(offsetof(ShipStaticInfo, resourcesAtOneTime) == 0x254);
static_assert(offsetof(ShipStaticInfo, maxresources) == 0x258);
static_assert(offsetof(ShipStaticInfo, harvestRate) == 0x25C);
static_assert(offsetof(ShipStaticInfo, harvestAmount) == 0x260);
static_assert(offsetof(ShipStaticInfo, groupSize) == 0x264);
static_assert(offsetof(ShipStaticInfo, repairTime) == 0x278);
static_assert(offsetof(ShipStaticInfo, repairCombatTime) == 0x280);
static_assert(offsetof(ShipStaticInfo, dockShipRange) == 0x29C);
static_assert(offsetof(ShipStaticInfo, shipIsCapital) == 0x2A8);
// IDA (univSetupShipForControl @ 0x54a650): non-idle passive-attack-while-moving gate tests
// LODWORD(staticinfo->engineNozzleOffset[1].x) != 0, not shipIsCapital -- same "big ship" heuristic
// used elsewhere in singleplayer.c alongside shipclass == CLASS_Component checks
static_assert(offsetof(ShipStaticInfo, engineNozzleOffset) == 0x2B0);
static_assert(offsetof(ShipStaticInfo, beastAttackCascadeCost) == 0x3DC);
static_assert(offsetof(ShipStaticInfo, beastAttacksRequired) == 0x3E0);
static_assert(offsetof(ShipStaticInfo, beastAttackConvertTo) == 0x3E4);
static_assert(offsetof(ShipStaticInfo, tacticalSelectionScale) == 0x3E8);
static_assert(offsetof(ShipStaticInfo, gunStaticInfo) == 0x3EC);
static_assert(offsetof(ShipStaticInfo, dockStaticInfo) == 0x3F0);
static_assert(offsetof(ShipStaticInfo, dockOverideInfo) == 0x3F4);
static_assert(offsetof(ShipStaticInfo, resNozzleStatic) == 0x3F8);
static_assert(offsetof(ShipStaticInfo, repairNozzleStatic) == 0x3FC);
static_assert(offsetof(ShipStaticInfo, tractorEmitterStatic) == 0x400);
static_assert(offsetof(ShipStaticInfo, ShipXDocksAtShipY) == 0x404);
static_assert(offsetof(ShipStaticInfo, LaunchShipXFromShipY) == 0x408);
static_assert(offsetof(ShipStaticInfo, linkDistance) == 0x40C);
static_assert(offsetof(ShipStaticInfo, UpgradeByDocking) == 0x43C);
static_assert(offsetof(ShipStaticInfo, RamDamage) == 0x454);

#define SSTATF_BeastShip BIT14
#define SSTATF_LinkedLaunchCapable BIT6        // 0x40 - Beast sACV/bACV/sMCV: eligible for clLaunchShipSelection's COMMAND_LINKED_LAUNCH fission-launch order (retail clLaunchShipSelection @0x44d7a0 filters selection by staticinfo->shipFlags & 0x40; set in universe.c InitStatShipTypeFlags)

typedef struct
{
    udword state;
    udword unk04[32];
    GunStatic *gunStatic;
} ShipUpgradeSlotStateEntry;

typedef struct
{
    sdword count;
    ShipUpgradeSlotStateEntry entries[1];
} ShipUpgradeSlotStateBlock;

static_assert(sizeof(ShipUpgradeSlotStateEntry) == 0x88, "ShipUpgradeSlotStateEntry size mismatch");
static_assert(sizeof(GunInfo) == sizeof(ShipUpgradeSlotStateBlock), "0x490 runtime block header mismatch");
static_assert(offsetof(GunInfo, numGuns) == offsetof(ShipUpgradeSlotStateBlock, count), "0x490 runtime block count mismatch");
static_assert(sizeof(Gun) == sizeof(ShipUpgradeSlotStateEntry), "0x490 runtime block entry size mismatch");
static_assert(offsetof(Gun, gunstatic) == offsetof(ShipUpgradeSlotStateEntry, gunStatic), "0x490 runtime block static pointer mismatch");

bool32 ShipHasUpgrade(Ship *ship, udword upgradeMask);
void shipInitUpgradeSlots(Ship *ship);
void shipRestoreUpgradeSlots(Ship *ship);
void shipApplyPlayerUpgrades(Ship *ship);

/*-----------------------------------------------------------------------------
    Missile Object
-----------------------------------------------------------------------------*/

typedef struct MissileID
{
    uword missileNumber;
} MissileID;

typedef struct
{
    Node objlink;
    ObjType objtype;                            // object type (ship or bullet, etc)
    udword flags;                               // flags whether the object is rotatable, etc.
    udword flags2;                              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;                              // Cataclysm-added; matches SpaceObj layout
    MissileStaticInfo *staticinfo;              // pointer to static info
    Node renderlink;                            // link for rendering list
    ubyte currentLOD;                           // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];                // Cataclysm-added; matches SpaceObj layout
    real32 cameraDistanceSquared;               // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;                // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;                    // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;

    ubyte aistate;                  // AI state variable; matches SpaceObjRotImpTarg layout
    ubyte pad1[2];
    sbyte putOnDoor;
    ClampInfo *clampInfo;

    //missile trail stuff
    missiletrail* trail;
    vector enginePosition;

    Node missilelink;
    MissileType missileType;
    GunSoundType soundType;
    ShipRace race;
    real32 maxtraveldist;
    real32 timelived;
    real32 totallifetime;
    struct Player *playerowner;
    Ship *owner;
    SpaceObjRotImpTarg *target;
    etglod *hitEffect;                          //effect to play on impact
    real32 damage;
    sdword haveCheckedForLoss;
    MissileID missileID;
    uword pad2;
    udword mineAIState;                         //ai state of missile MINES
    vector formation_position;                  //vector that records the position the mine WANTS to be in while in formation
    sdword FORCE_DROPPED;                       //flag..true if force_dropped...used for variable selection
    Node formationLink;
    udword cataUnknown238[3];                   // Cataclysm-only missile save/runtime fields
    MineFormationInfo *formationinfo;
    sdword formation_number_X;
    sdword formation_number_Y;
    ubyte colorScheme;
    ubyte pad[3];
    udword cataUnknown254;                      // Cataclysm-only missile save/runtime field
} Missile;

typedef Missile *MissilePtr;

static_assert(offsetof(Missile, flags2) == offsetof(SpaceObj, flags2), "Missile layout mismatch");
static_assert(offsetof(Missile, renderlink) == offsetof(SpaceObj, renderlink), "Missile layout mismatch");
static_assert(offsetof(Missile, posinfo) == offsetof(SpaceObj, posinfo), "Missile layout mismatch");
static_assert(offsetof(Missile, clampInfo) == 0x1BC, "Missile layout mismatch");
static_assert(offsetof(Missile, missileType) == 0x1E0, "Missile layout mismatch");
static_assert(offsetof(Missile, race) == 0x1E8, "Missile layout mismatch");
static_assert(offsetof(Missile, playerowner) == 0x1F8, "Missile layout mismatch");
static_assert(offsetof(Missile, owner) == 0x1FC, "Missile layout mismatch");
static_assert(offsetof(Missile, target) == 0x200, "Missile layout mismatch");
static_assert(offsetof(Missile, hitEffect) == 0x204, "Missile layout mismatch");
static_assert(offsetof(Missile, formationinfo) == 0x244, "Missile layout mismatch");
static_assert(offsetof(Missile, formationLink) == 0x228, "Missile formationLink offset mismatch");
static_assert(offsetof(Missile, formationLink) + offsetof(Node, belongto) == 0x230, "Missile formationLink.belongto offset mismatch");
static_assert(sizeof(Missile) == 0x258, "Missile size mismatch");

/*-----------------------------------------------------------------------------
    Effect Object
-----------------------------------------------------------------------------*/

typedef struct Effect
{
    Node objlink;
    ObjType objtype;                            // object type (ship or bullet, etc)
    udword flags;                               // flags whether the object is rotatable, etc.
    udword flags2;                              // Cataclysm-added; matches SpaceObjRot layout
    udword unk_1C;                              // Cataclysm-added; matches SpaceObjRot layout
    StaticInfo *staticinfo;                     // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;                           // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];                            // Cataclysm-added; matches SpaceObjRot layout
    real32 cameraDistanceSquared;               // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;                    // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;

    //... effect-specific stuff
    Node effectLink;
    udword effectFlags;                         //effects-specific flags
    Ship *owner;                                //owner of this effect (e.g. ship that blew up)
    real32 magnitudeSquared;                    //N-lips scale factor, if any
    sdword nSpawnedAttachedToOwner;             //number of spawned effects attached to this effect's owner
    real32 timeElapsed;                         //age of effect (can be negative for tim-offset effects)
    vector LoF;                                 //line-of-fire vector
    vector velocity;                            //velocity vector
    real32 theta;                               //R-plane rotation vector

    real32 drag;                                //drag for moving entire effect

    real32 spin;                                //control the spin of the effect
    real32 deltaSpin;

    ubyte *variable;                            //variable block
    ubyte *variableRate;                        //variable rate block (effectors)
    ubyte *effectID;                            //effector ID block for variables
    sdword nParticleBlocks;                     //max number of particle systems
    sdword iParticleBlock;                      //current number of particle systems
    ubyte *particleBlock[1];                    //ragged array of particle systems in effect
} Effect;

static_assert(offsetof(Effect, flags2) == offsetof(SpaceObj, flags2), "Effect layout mismatch");
static_assert(offsetof(Effect, renderlink) == offsetof(SpaceObj, renderlink), "Effect layout mismatch");
static_assert(offsetof(Effect, posinfo) == offsetof(SpaceObj, posinfo), "Effect layout mismatch");
static_assert(offsetof(Effect, rotinfo) == offsetof(SpaceObjRot, rotinfo), "Effect layout mismatch");
static_assert(offsetof(Effect, effectLink) == 0xD4, "Effect layout mismatch");

/*-----------------------------------------------------------------------------
    Bullet Object
-----------------------------------------------------------------------------*/

typedef struct                                  // Bullet object
{
    Node objlink;
    ObjType objtype;                            // object type (ship or bullet, etc)
    udword flags;                               // flags whether the object is rotatable, etc.
    udword flags2;                              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;                              // Cataclysm-added; matches SpaceObj layout
    StaticInfo *staticinfo;                     // pointer to static info
    Node renderlink;                            // link for rendering list
    ubyte currentLOD;                           // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];                // Cataclysm-added; matches SpaceObj layout
    real32 cameraDistanceSquared;               // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;                // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;                    // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;

    Node bulletlink;
    vector bulletheading;                       // heading in universe
    vector lengthvec;
    real32 lengthmag;
    real32 traveldist;
    real32 beamtraveldist;                      // only used for BULLET_Beam
    BulletType bulletType;
    udword cataUnknown10C;
    GunSoundType soundType;
    real32 timelived;
    real32 totallifetime;
    Ship const* owner;                          // pointer to bullet's owner
    Gun *gunowner;                              // pointer to bullet's gun from which it was fired
    SpaceObjRotImpTarg *target;                 // target of bullet
    color bulletColor;                          // race of player who shot bullet
    real32 bulletmass;
    etglod *hitEffect;                          //effect to play when it hits
    Effect *effect;                             //effect to play as it flies
    real32 damage;                              //current damage of bullet
    real32 damageFull;                          //unadjusted (initial) damage
    udword SpecialEffectFlag;                   //Used by special ships to see if they have affected this bullet
    real32 DFGFieldEntryTime;                   //used by DFGF.
    real32 BulletSpeed;
    struct Player *playerowner;
    udword cataUnknown150[5];
} Bullet;

static_assert(offsetof(Bullet, flags2) == offsetof(SpaceObj, flags2), "Bullet layout mismatch");
static_assert(offsetof(Bullet, renderlink) == offsetof(SpaceObj, renderlink), "Bullet layout mismatch");
static_assert(offsetof(Bullet, posinfo) == offsetof(SpaceObj, posinfo), "Bullet layout mismatch");
static_assert(offsetof(Bullet, bulletType) == 0x108, "Bullet layout mismatch");
static_assert(offsetof(Bullet, soundType) == 0x110, "Bullet layout mismatch");
static_assert(offsetof(Bullet, timelived) == 0x114, "Bullet layout mismatch");
static_assert(offsetof(Bullet, totallifetime) == 0x118, "Bullet layout mismatch");
static_assert(offsetof(Bullet, owner) == 0x11C, "Bullet layout mismatch");
static_assert(offsetof(Bullet, effect) == 0x134, "Bullet layout mismatch");
static_assert(offsetof(Bullet, damage) == 0x138, "Bullet damage offset mismatch");
static_assert(offsetof(Bullet, playerowner) == 0x14C, "Bullet layout mismatch");
static_assert(sizeof(Bullet) == 0x164, "Bullet size mismatch");

typedef Bullet *BulletPtr;

/*=============================================================================
    Derelict
=============================================================================*/

typedef struct DerelictID
{
    uword derelictNumber;
    uword unused;
} DerelictID;

typedef struct
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;          // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;          // Cataclysm-added; matches SpaceObj layout
    DerelictStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    ubyte unk_3C[4];                // Cataclysm-added; matches SpaceObj layout
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;
    NonStats nonstatvars;
    SalvageInfo *salvageInfo;       //salvage info...
    ClampInfo *clampInfo;

    //must come next to be compatible with Derelict in aiship stuff
    ubyte aistate;                  // AI state variable
    ubyte pad1[2];
    sbyte putOnDoor;
    struct Ship *dockingship;       //Used to trick collision detection when being sent into docking bays
    udword specialFlags;            //general purpose ship flags, see above for defines
    udword specialFlags2;           //more general purpose ship flags, see above for defines
    ubyte cataRuntimeState[68];     // Cataclysm-only derelict runtime state
    udword specialFlags3;
    udword specialFlags4;
    sdword specialFlagsUnknown2;
    sdword specialFlagsUnknown3;
    sdword specialFlagsUnknown4;
    sdword specialFlagsUnknown5;
    sdword specialFlagsUnknown6;
    sdword specialFlagsUnknown7;
    NAVLightInfo *navLightInfo;
    // HERE UP SAME for Derelicts and Ships

    Node derelictlink;
    DerelictID derelictID;
    sbyte colorScheme;              //what color scheme does this ship use?
    ubyte pad[3];

    sdword ambientSoundHandle;      //used by sound engine to keep track of the ambient sound the derelict makes
    sdword randomSoundHandle;       //used by sound engine to keep track of the random ambient sound the derelict makes
    real32 nextRandomTime;          //time to play the next random ambient sound

    sdword TractorTrue;             //used to stop friction on derelicts when being pushed
    udword tractorbeaminfo;         //counter for # of tractor beams on derelict
    udword tractorvector;
    struct Player *tractorbeam_playerowner;
    sdword tractorstate;            //info variable for tractorbeaming state
    sdword tractorPosinfo[4];        //more tractor info!
    sdword needNewLeader;
    struct Ship *newDockWithTransfer;
    sdword newDockIndexTransfer;
    real32 creationTime;
    DerelictType derelicttype;
} Derelict;

typedef Derelict *DerelictPtr;

typedef struct
{
    Node objlink;
    ObjType objtype;
    udword flags;
    udword flags2;
    udword unk_1C;
    DerelictStaticInfo *staticinfo;
    Node renderlink;
    ubyte currentLOD;
    ubyte renderedLODs;
    uword attributes;
    sword attributesParam;
    ubyte attributesPad[2];
    ubyte unk_3C[4];
    real32 cameraDistanceSquared;
    vector cameraDistanceVector;
    real32 collOptimizeDist;
    struct blob *collMyBlob;
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;
    real32 flashtimer;
    real32 lasttimecollided;
    NonStats nonstatvars;
    SalvageInfo *salvageInfo;
    ClampInfo *clampInfo;
    ubyte aistate;
    ubyte pad1[2];
    sbyte putOnDoor;
    struct Ship *dockingship;
    udword specialFlags;
    udword specialFlags2;
    ubyte cataRuntimeState[68];
    udword specialFlags3;
    udword specialFlags4;
    sdword specialFlagsUnknown2;
    sdword specialFlagsUnknown3;
    sdword specialFlagsUnknown4;
    sdword specialFlagsUnknown5;
    sdword specialFlagsUnknown6;
    sdword specialFlagsUnknown7;
    NAVLightInfo *navLightInfo;
    Node derelictlink;
    DerelictID derelictID;
    sbyte colorScheme;
    ubyte pad[3];
    sdword ambientSoundHandle;
    sdword randomSoundHandle;
    real32 nextRandomTime;
    sdword TractorTrue;
    udword tractorbeaminfo;
    udword tractorvector;
    struct Player *tractorbeam_playerowner;
    sdword tractorstate;
    sdword tractorPosinfo[4];
    sdword needNewLeader;
    struct Ship *newDockWithTransfer;
    sdword newDockIndexTransfer;
    real32 creationTime;
    DerelictType derelicttype;
} CataDerelictSaveBody;

static_assert(sizeof(DerelictID) == 4);
static_assert(sizeof(Derelict) == 0x294);
static_assert(sizeof(CataDerelictSaveBody) == 0x294);
static_assert(offsetof(Derelict, staticinfo) == 0x20);
static_assert(offsetof(Derelict, cameraDistanceSquared) == 0x40);
static_assert(offsetof(Derelict, collMyBlob) == 0x54);
static_assert(offsetof(Derelict, health) == 0x170);
static_assert(offsetof(Derelict, nonstatvars) == 0x17C);
static_assert(offsetof(Derelict, salvageInfo) == 0x1B8);
static_assert(offsetof(Derelict, clampInfo) == 0x1BC);
static_assert(offsetof(Derelict, aistate) == 0x1C0);
static_assert(offsetof(Derelict, dockingship) == 0x1C4);
static_assert(offsetof(Derelict, specialFlags) == 0x1C8);
static_assert(offsetof(Derelict, specialFlags2) == 0x1CC);
static_assert(offsetof(Derelict, specialFlags3) == 0x214);
static_assert(offsetof(Derelict, navLightInfo) == 0x234);
static_assert(offsetof(Derelict, derelictlink) == 0x238);
static_assert(offsetof(Derelict, derelictID) == 0x248);
static_assert(offsetof(Derelict, colorScheme) == 0x24C);
static_assert(offsetof(Derelict, ambientSoundHandle) == 0x250);
static_assert(offsetof(Derelict, nextRandomTime) == 0x258);
static_assert(offsetof(Derelict, TractorTrue) == 0x25C);
static_assert(offsetof(Derelict, newDockWithTransfer) == 0x284);
static_assert(offsetof(Derelict, creationTime) == 0x28C);
static_assert(offsetof(Derelict, derelicttype) == 0x290);

/*=============================================================================
    Resource Objects
=============================================================================*/

typedef struct ResourceID
{
    uword resourceNumber;
} ResourceID;

typedef struct CataResourcePreBlobState
{
    ubyte unknown[4];
} CataResourcePreBlobState;

typedef struct CataAsteroidRuntimeState
{
    sdword unknown0;
    struct Derelict *linkedDerelict;
    udword unknown1;
    udword unknown2;
    sdword unknown3;
} CataAsteroidRuntimeState;

/*-----------------------------------------------------------------------------
    Generic Resource, from which all resources (asteroids, nebula, etc. are derived)
-----------------------------------------------------------------------------*/

typedef struct Resource
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;              // Cataclysm-added; matches SpaceObj layout
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    CataResourcePreBlobState cataPreBlobState;   // Cataclysm-added; 4 bytes pushes collMyBlob to offset 0x54
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to
} Resource;

typedef Resource *ResourcePtr;    // all resources are assumed to be rotatable, and impactable

/*-----------------------------------------------------------------------------
    Asteroid
-----------------------------------------------------------------------------*/

typedef struct Asteroid
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;              // Cataclysm-added; matches SpaceObj layout
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    CataResourcePreBlobState cataPreBlobState;
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    AsteroidType asteroidtype;  // asteroid specific information
    real32 scaling;
    CataAsteroidRuntimeState cataRuntimeState2;
} Asteroid;

/*-----------------------------------------------------------------------------
    Nebula
-----------------------------------------------------------------------------*/

typedef struct Nebula
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;              // Cataclysm-added; matches SpaceObj layout
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    CataResourcePreBlobState cataPreBlobState;
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    // put nebula specific stuff here only
    NebulaType nebulatype;
    real32 scaling;
    nebChunk* stub;
    sdword nebNebulaeIndex;
} Nebula;

/*-----------------------------------------------------------------------------
    Gas Cloud
-----------------------------------------------------------------------------*/

typedef struct GasCloud
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;              // Cataclysm-added; matches SpaceObj layout
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    CataResourcePreBlobState cataPreBlobState;
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    // put Gas Cloud pecific stuff here only
    GasCloudType gascloudtype;  // gascloud specific information
    real32 scaling;
} GasCloud;

/*-----------------------------------------------------------------------------
    Dust Cloud
-----------------------------------------------------------------------------*/

typedef struct DustCloud
{
    Node objlink;
    ObjType objtype;        // object type (ship or bullet, etc)
    udword flags;           // flags whether the object is rotatable, etc.
    udword flags2;              // Cataclysm-added; matches SpaceObj layout
    udword unk_1C;              // Cataclysm-added; matches SpaceObj layout
    ResourceStaticInfo *staticinfo; // pointer to static info
    Node renderlink;        // link for rendering list
    ubyte currentLOD;       // current level of detail
    ubyte renderedLODs;     //what LODs have been rendered already
    uword attributes;               // settable attributes from mission editor
    sword attributesParam;          // parameter for attributes, set from mission editor
    ubyte attributesPad[2];          // leftover, free space
    real32 cameraDistanceSquared;    // distance to camera (for sound, level of detail)
    vector cameraDistanceVector;     // vector to camera (camera eyeposition - obj->posinfo.position)
    real32 collOptimizeDist;         // distance used for optimizing collision checking
    CataResourcePreBlobState cataPreBlobState;
    struct blob *collMyBlob;         // collision blob I belong to
    PosInfo posinfo;
    RotInfo rotinfo;
    CollInfo collInfo;
    Node impactablelink;
    real32 health;                  // hitpoints
    real32 flashtimer;              // determines if object flashes on screen
    real32 lasttimecollided;

    Node resourcelink;
    ResourceID resourceID;
    uword resourceNotAccessible;
    sdword resourcevalue;
    struct ResourceVolume *resourceVolume;      // resourceVolume I belong to

    // Dust Cloud specific stuff
    DustCloudType dustcloudtype;
    real32 scaling;
    cloudSystem* stub;
} DustCloud;

// Binary: sizeof(Asteroid) = 436 (SaveAllSpaceObjs case 2: qmemcpy 0x1B4 bytes)
// Binary: sizeof(Nebula) = 424 (LoadSpaceObj case 3: memAllocFunction(424))
// Binary: sizeof(DustCloud) = 420 (verified from save body layout)
static_assert(sizeof(Asteroid) == 436);
static_assert(sizeof(Nebula) == 424);
static_assert(sizeof(DustCloud) == 420);
static_assert(offsetof(Asteroid, collMyBlob) == 0x54);
static_assert(offsetof(Resource, collMyBlob) == 0x54);
static_assert(offsetof(Asteroid, resourceVolume) == 404);
static_assert(offsetof(Asteroid, asteroidtype) == 408);

#define MAX_NUM_DROP    5        //maximum number of targets that can be dropped at a motherships/carrier


#ifdef GOD_LIKE_SYNC_CHECKING

typedef struct
{
    udword univUpdateCounterValue;
    udword numShips;    //number of ships in this snap shot
    Ship ship[1];       //dynamic array of ships
}UniverseSnapShot;

typedef struct
{
    char playerName[20];    //name of curplayer
    sdword windowSize;      //number of universeSnapShots in this file
    UniverseSnapShot *universeSnapShot[1]; //array of snapshots
}GodWritePacketHeader;

extern GodWritePacketHeader *godUniverseSnapShotHeader;

#define sizeofGodHeader(n)      sizeof(GodWritePacketHeader) + sizeof(UniverseSnapShot *)*(n-1)
#define sizeofSnapShot(n)       sizeof(UniverseSnapShot)+ (offsetof(Ship, ShipSpecifics) + sizeof(udword))*(n)



#endif

/*=============================================================================
    Macros:
=============================================================================*/

#define sizeofGunInfo(n) (sizeof(GunInfo) + (n-1)*sizeof(Gun))
#define sizeofGunStaticInfo(n) (sizeof(GunStaticInfo) + (n-1)*sizeof(GunStatic))
#define sizeofDockStaticInfo(n) (sizeof(DockStaticInfo) + (n-1)*sizeof(DockStaticPoint))
#define sizeofNavLightStaticInfo(n) (sizeof(NAVLightStaticInfo) + (n-1)*sizeof(NAVLightStatic))
#define sizeofDockInfo(n) (sizeof(DockInfo) + (n-1)*sizeof(DockPoint))
#define sizeofNavLightInfo(n) (sizeof(NAVLightInfo) + (n-1)*sizeof(NAVLight))

//#define isCapitalShip(ship) (((ShipStaticInfo *)((ship)##->staticinfo))->shipclass <= CLASS_Frigate)
//#define isCapitalShipStatic(shipstatic) (shipstatic##->shipclass <= CLASS_Frigate)
#define isCapitalShip(ship) (((ShipStaticInfo *)((ship)->staticinfo))->shipIsCapital)
#define isCapitalShipStatic(shipstatic) ((shipstatic)->shipIsCapital)

#define isShipOfClass(ship,checkclass) (((ShipStaticInfo *)((ship)->staticinfo))->shipclass == (checkclass))

/*=============================================================================
    Support Functions:
=============================================================================*/

#endif //___SPACEOBJ
