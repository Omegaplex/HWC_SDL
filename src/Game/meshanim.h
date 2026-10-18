/*=============================================================================
    Name    : MeshAnim.h
    Purpose : Definitions for scripted mesh animations.

    Created 7/21/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___MESHANIM_H
#define ___MESHANIM_H

#include "B-Spline.h"
#include "CRC32.h"
#include "Mesh.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define MAD_TEST_ANIMATION      0               //test animations
#ifdef HW_BUILD_FOR_DEBUGGING

#define MAD_ERROR_CHECKING      1               //basic error checking
#define MAD_VERBOSE_LEVEL       1               //control verbose printing

#else

#define MAD_ERROR_CHECKING      0               //no error ckecking in retail
#define MAD_VERBOSE_LEVEL       0               //don't print any verbose strings in retail

#endif


/*=============================================================================
    Definitions:
=============================================================================*/
#define MAD_FileVersion         0.91f
#define MAD_FileIdentifier      "Mellonn"
#define MAD_Extension           ".MAD"
#define MAD_MaxAnimations       32

//animation flags
#define MAF_Loop                0x00000001      //this animation will loop
#define MAD_NoAnimation         -1              //nCurrentAnim will be this if no animation playing

/*=============================================================================
    Type definitions
=============================================================================*/
//structure for a single object's keyframe list in the file.
typedef struct
{
    char *name;                                 //name of the object for this keyframe list
    crc16 nameCRC;                              //16-bit crc of the name.  (should upgrade to 32 later).
    ubyte pad[2];
    udword animationBits;                       //bit-flags: one for each animation
    sdword nKeyframes;                          //number of keyframes in all of file for this object
    real32 *times;                              //times array
    tcb *parameters;                            //tension, continuity, bias
    real32 *path[6];                            //arrays of x,y,z,h,p,b
}
madobjpath;

//structure for a single animation within the file
typedef struct
{
    char *name;                                 //name of the animation
    real32 startTime;                           //limits of the animation
    real32 endTime;
    udword flags;                               //flags for this animation
}
madanimation;

//structure for the header of a mesh animation file
typedef struct
{
    char identifier[8];                         //compared to MAD_FileIdentifier
    real32 version;                             //compared to MAD_FileVersion
    sdword stringBlockLength;                   //length of string block
    char *stringBlock;                          //string block: where all strings for this file are located and offsets are relative to
    real32 length;                              //length of all animations
    real32 framesPerSecond;                     //rate the animation was scripted at
    sdword nObjects;                            //number of objects in the animation (should be same as number of objects in the mesh to animate)
    madobjpath *objPath;                        //list of keyframe lists for each object
    sdword nAnimations;                         //number of separate animations in the file
    madanimation anim[1];                       //ragged array of animation headers
}
madheader;

//structure for animation data in shipstaticinfo
typedef struct
{
    madheader *header;                          //animation data loaded from disk
    sdword needStartPlacedAnimation;                   //True or false flag indicating the object possesing these animations needs a starting animation/pause hack (thats right luke, a hack!)
    sdword startPlacedAnimationIndex;                 //if above flag is true, this index is valid and points to that starting animation
    sdword needStartBuiltAnimation;                   //True or false flag indicating the object possesing these animations needs a starting animation/pause hack (thats right luke, a hack!)
    sdword startBuiltAnimationIndex;                 //if above flag is true, this index is valid and points to that starting animation
    //extra context sensitive animation data
    sdword numGunOpenIndexes;                   //number of below indices
    sdword numGunOpenDamagedIndexes;                   //number of below indices
    sdword numGunCloseIndexes;                   //number of below indices
    sdword numGunCloseDamagedIndexes;                   //number of below indices
    sdword *gunOpenIndexes;                      //arrays for gunopening/closing animation indexes both damaged and undamaged.  So we can support a number of different animationsfor the same ship
    sdword *gunOpenDamagedIndexes;
    sdword *gunCloseIndexes;
    sdword *gunCloseDamagedIndexes;
    //same, but for docking animations
    sdword numDockIndexes;                   //number of below indices
    sdword numDockDamagedIndexes;                   //number of below indices
    sdword numPostDockIndexes;                   //number of below indices
    sdword numPostDockDamagedIndexes;                   //number of below indices
    sdword *DockIndexes;                      //arrays for gunopening/closing animation indexes both damaged and undamaged.  So we can support a number of different animationsfor the same ship
    sdword *DockDamagedIndexes;
    sdword *PostDockIndexes;
    sdword *PostDockDamagedIndexes;
    sdword numDoorOpenIndexes;
    sdword numDoorCloseIndexes;
    sdword *DoorOpenIndexes;
    sdword *DoorCloseIndexes;
    sdword numSpecialOpenIndexes;
    sdword numSpecialCloseIndexes;
    sdword numSpecialOpenDamagedIndexes;
    sdword numSpecialCloseDamagedIndexes;
    sdword *specialOpenIndexes;
    sdword *specialCloseIndexes;
    sdword *specialOpenDamagedIndexes;
    sdword *specialCloseDamagedIndexes;
    sdword unk_0x84;                             //Cataclysm: unknown field (+0x84)
    sdword *animIndexRemap;                      //Cataclysm: pointer to animation index remap table (+0x88)
    sdword animIndexRemapCount;                  //Cataclysm: number of entries; if 0, use nCurrentAnim directly (+0x8C)
}
madstatic;

static_assert(offsetof(madstatic, animIndexRemap) == 0x88);
static_assert(offsetof(madstatic, animIndexRemapCount) == 0x8C);

//structure of animation data stored within a ship
typedef struct madanim
{
    madheader *header;                          //static animation data          (+0x00)
    sdword totalSize;                           //size of this structure, bindings, motion curves and the whole bit (+0x04)
    udword flags;                               //flags for this animation        (+0x08)
    sdword nCurrentAnim;                        //animation we're currently on, or MAD_NoAnimation for none (+0x0C)
    sdword nPrevAnim;                           //Cataclysm: previous animation index (+0x10)
    bool32 bPaused;                             //TRUE when the animation is paused (+0x14)
    sdword nPendingAnim;                        //Cataclysm: deferred binding activation flag; set to 1 on start, cleared to -1 on first update (+0x18)
    real32 startTime;                           //time the animation starts at    (+0x1C)
    real32 time;                                //time within the animation, from starttime to endtime inclusive (+0x20)
    real32 timeElapsed;                         //time elapsed since last rendered (+0x24)
    splinecurve *curves;                        //spline curve array, enough for 6 * number of objects to animate (+0x28)
    shipbindings *saveBindings;                 //gun-mesh bindings are saved here when scripted animation is going on. (+0x2C)
    shipbindings bindings;                      //animation bindings for this ship, must be last in list; variable size (+0x30)
}
madanim;

static_assert(offsetof(madanim, nPrevAnim) == 0x10);
static_assert(offsetof(madanim, bPaused) == 0x14);
static_assert(offsetof(madanim, nPendingAnim) == 0x18);
static_assert(offsetof(madanim, saveBindings) == 0x2C);
static_assert(offsetof(madanim, bindings) == 0x30);


typedef struct
{
    udword name; //*                                //name of the object for this keyframe list
    crc16 nameCRC;                              //16-bit crc of the name.  (should upgrade to 32 later).
    ubyte pad[2];
    udword animationBits;                       //bit-flags: one for each animation
    sdword nKeyframes;                          //number of keyframes in all of file for this object
    udword times; //real32*                     //times array
    udword parameters;   //tcp*                   //tension, continuity, bias
    udword path[6];  //real32*                 //arrays of x,y,z,h,p,b
}
madobjpath_disk;

typedef struct
{
    udword name;   // *                                //name of the animation
    real32 startTime;                           //limits of the animation
    real32 endTime;
    udword flags;                               //flags for this animation
}
madanimation_disk;

typedef struct
{
    char identifier[8];                         //compared to MAD_FileIdentifier
    real32 version;                             //compared to MAD_FileVersion
    sdword stringBlockLength;                   //length of string block
    udword stringBlock;  //*                        //string block: where all strings for this file are located and offsets are relative to
    real32 length;                              //length of all animations
    real32 framesPerSecond;                     //rate the animation was scripted at
    sdword nObjects;                            //number of objects in the animation (should be same as number of objects in the mesh to animate)
    udword objPath; //*                       //list of keyframe lists for each object
    sdword nAnimations;                         //number of separate animations in the file
    madanimation_disk anim[1];                       //ragged array of animation headers
}
madheader_disk;


/*=============================================================================
    Macros:
=============================================================================*/
#define madHeaderSize(n)    ((sdword)sizeof(madheader) + (sdword)sizeof(madanimation) * ((n) - 1))
#define madAnimSize(n)      (sizeof(madanim) - sizeof(shipbindings) + shipBindingsLength(n))

/*=============================================================================
    Functions:
=============================================================================*/
struct Ship;
struct ShipStaticInfo;

madheader *madFileLoad(char *fileName);
void madHeaderDelete(madheader *header);

//make ship local bindings for mesh animation
void madAnimBindingsDupe(struct Ship *ship, struct ShipStaticInfo *staticInfo,bool32 LoadingGame);

//play an animation
void madAnimationStart(struct Ship *ship, sdword animNumber);
void madAnimationStartWithFlags(struct Ship *ship, sdword animNumber, udword flags);
void madAnimationInit(madanim *state);
void madAnimationPause(struct Ship *ship, bool32 bFreeze);
bool32 madAnimationUpdate(struct Ship *ship, real32 timeElapsed);
bool32 madAnimationCompleted(struct Ship *ship);
void madAnimationStop(struct Ship *ship);

//find things in animations
sdword madAnimIndexFindByName(madheader *header, char *name);
sdword madGunBindingIndexFindByName(struct ShipStaticInfo *info, char *name);
sdword madBindingIndexFindByName(madheader *header, char *name);
void madAnimBindingMatrix(matrix *destMatrix, vector *destPos, struct Ship *ship, sdword gunIndex, sdword madIndex);

void PreFix_madBindings(struct Ship *ship,struct Ship *fixcontents);
void Save_madBindings(struct Ship *ship);
void Load_madBindings(struct Ship *ship);
void Fix_madBindings(struct Ship *ship);

#endif
