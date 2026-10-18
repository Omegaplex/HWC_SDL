/*=============================================================================
    Name    : compat.h
    Purpose : Compatibility defines and extern declarations needed across
              most source files. Force-included via compiler options.
=============================================================================*/

#ifndef ___COMPAT_H
#define ___COMPAT_H

#include "debug.h"

/*=============================================================================
    Temporary diagnostic tracing
    CATACLYSM_DIAG defaults to off outside Release builds. Because compat.h is
    force-included, source files that need temporary tracing must explicitly
    #undef/#define it locally, then guard the block with
    #if !defined(HW_Release) && CATACLYSM_DIAG so Release builds always compile
    the tracing out even when a file opts in locally.
=============================================================================*/
#if !defined(HW_Release) && !defined(CATACLYSM_DIAG)
    #define CATACLYSM_DIAG 0
#endif

#define CATACLYSM_DIAG_LABEL "[CATACLYSM_DIAG] "

// Use standard malloc/free instead of custom pool allocator for ASAN compatibility
#define MEM_USE_STDLIB

#include "memory.h"
#include "tweak.h"

/*=============================================================================
    Titan network globals (defined in Globals.c)
=============================================================================*/
extern int sigsNumPlayers;
extern int sigsPlayerIndex;
extern int sigsPressedStartGame;
extern signed int captainIndex;
extern int LANGame;
extern int IPGame;
extern int GameCreator;
extern int WaitingForGame;
extern unsigned long TitanActive;

#define IAmCaptain  (sigsPressedStartGame ? (sigsPlayerIndex == captainIndex) : (GameCreator))
#define CaptainNotDefined (captainIndex == -1)

/*=============================================================================
    Renderer globals
=============================================================================*/


/*=============================================================================
    Misc globals
=============================================================================*/
#define RANDOM_AI_PLAYER    6

extern void *jp;


/*=============================================================================
    Ship aliases - uppercase P1 variants
=============================================================================*/
#ifndef AdvanceSupportFrigate
#define AdvanceSupportFrigate 0
#endif
#ifndef AttackBomber
#define AttackBomber 1
#endif
#ifndef Carrier
#define Carrier 2
#endif
#ifndef CloakedFighter
#define CloakedFighter 3
#endif
#ifndef CloakGenerator
#define CloakGenerator 4
#endif
#ifndef DDDFrigate
#define DDDFrigate 5
#endif
#ifndef DefenseFighter
#define DefenseFighter 6
#endif
#ifndef DFGFrigate
#define DFGFrigate 7
#endif
#ifndef GravWellGenerator
#define GravWellGenerator 8
#endif
#ifndef HeavyCorvette
#define HeavyCorvette 9
#endif
#ifndef HeavyCruiser
#define HeavyCruiser 10
#endif
#ifndef HeavyDefender
#define HeavyDefender 11
#endif
#ifndef HeavyInterceptor
#define HeavyInterceptor 12
#endif
#ifndef IonCannonFrigate
#define IonCannonFrigate 13
#endif
#ifndef LightCorvette
#define LightCorvette 14
#endif
#ifndef LightDefender
#define LightDefender 15
#endif
#ifndef LightInterceptor
#define LightInterceptor 16
#endif
#ifndef MinelayerCorvette
#define MinelayerCorvette 17
#endif
#ifndef MissileDestroyer
#define MissileDestroyer 18
#endif
#ifndef Mothership
#define Mothership 19
#endif
#ifndef MultiGunCorvette
#define MultiGunCorvette 20
#endif
#ifndef Probe
#define Probe 21
#endif
#ifndef ProximitySensor
#define ProximitySensor 22
#endif
#ifndef RepairCorvette
#define RepairCorvette 23
#endif
#ifndef ResearchShip
#define ResearchShip 24
#endif
#ifndef ResourceCollector
#define ResourceCollector 25
#endif
#ifndef ResourceController
#define ResourceController 26
#endif
#ifndef SalCapCorvette
#define SalCapCorvette 27
#endif
#ifndef SensorArray
#define SensorArray 28
#endif
#ifndef StandardDestroyer
#define StandardDestroyer 29
#endif
#ifndef StandardFrigate
#define StandardFrigate 30
#endif
#ifndef Drone
#define Drone 31
#endif
#ifndef TargetDrone
#define TargetDrone 32
#endif
#ifndef HeadShotAsteroid
#define HeadShotAsteroid 33
#endif
#ifndef CryoTray
#define CryoTray 34
#endif

#ifndef P1Fighter
#define P1Fighter          p1Fighter
#endif
#ifndef P1Mothership
#define P1Mothership       p1Mothership
#endif
#ifndef P1MissileCorvette
#define P1MissileCorvette  p1MissileCorvette
#endif
#ifndef P1IonArrayFrigate
#define P1IonArrayFrigate  p1IonArrayFrigate
#endif
#ifndef P1StandardCorvette
#define P1StandardCorvette p1StandardCorvette
#endif
#ifndef P1Outpost
#define P1Outpost          p1Outpost
#endif

#ifndef FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND
#define FE_DONT_DELETE_REGION_IF_SCREEN_NOT_FOUND   1
#endif

/* Multiplayer game info */
struct CaptainGameInfo;
extern struct CaptainGameInfo tpGameCreated;

/* Additional game globals */
struct tagRegion;
void feStaticRectangleDraw(struct tagRegion *region);
void feUserRegionDraw(struct tagRegion *region);
extern char const* GLC_VENDOR;
extern char const* GLC_RENDERER;
extern void *costData;

extern real32 RENDER_VIEWABLE_DISTANCE_SQR;

extern sdword dbgInt3Enabled;
extern char dbgFatalErrorString[];
extern char PATCHNAME[];

extern unsigned int RGL;
extern unsigned int RGLtype;


extern sdword IgnoreBigfiles;

#endif // ___COMPAT_H
