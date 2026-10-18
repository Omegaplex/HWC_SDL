/*=============================================================================
    Name    : SoundStructs.h
    Purpose : Structures used by the sound event layer.

    Created 02/11/98 by salfreds
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___SOUNDSTRUCTS_H
#define ___SOUNDSTRUCTS_H

#include "Types.h"


typedef struct
{
	sdword	engineHandle;
	sdword	engineState;
    sdword	ambientHandle;
	sdword	damageHandle;
	sdword	specialHandle;
	sdword	hyperspaceHandle;
    sdword	actorNum;

	real32	coverage;
	real32	distance;
	sword	pan;
	sword	engineVol;

	sdword	randomHandle;
	real32	nextRandom;

	sdword  lastAmbient;

	real32	timeLastStatus;

	sdword	burstHandle;
	bool32	burstfiring;

	sdword	gunHandle;
} SOUNDEVENT;


typedef struct
{
	sdword	ID;
	udword	checksum;
	sword	numvariations;
	sword	numevents;
	sword	numobjects;
	sword	lookup[];
} SFXLUT;


typedef struct
{
	sdword	ID;				// version identification
	udword	checksum;		// checksum of this generate
	uword	numcolumns;		// how wide is the lookup table
	uword	numevents;		// how many events are there
	uword	numactors;		// how many actors are there
	uword	compbitrate;	// Cataclysm stores one shared stream bitrate here
	sword	lookup[];		// lookup is numcolumns wide by (numactors * numevents) long
/* columns are:
	priority - what priority this event is (0 - 3)
	number of variations - how many different ways there are to say this event
	max variable - what is the range for the variable in this sentence
	index - into the phrase table
*/
} SENTENCELUT;

typedef struct
{
   sdword ID;
   udword checksum;
   uword numcolumns;
   uword numevents;
   uword unk0C;
   uword streamBitrate;   // Cataclysm stores one shared stream bitrate here
} CataSentenceLUTHeader;


typedef struct
{
	sdword	ID;				// version identification
	udword	checksum;		// checksum of this generate
	uword	numcolumns;		// how wide is the lookup table
	uword	numsentences;	// how long is the lookup table
    /*
    */
	sdword	lookupsy[];		// lookup is numcolumns wide by numsentences long
/* rows are:
	actor
		variation
			variable
*/
/* columns are:
	duration - approximatley how much time will this sentence take to play
	probability - what is the weighting for how often this variation should be played
	numoffsets - how many phrases are stitched together for this sentence
	offsets - (numcolumns - 3) offsets, only numoffsets of them have values, others will be -1
*/
} PHRASELUT;

/*-----------------------------------------------------------------------------
    For choosing a random variation of a speech event, we use the high words
    of the probability and numOffsets above.  As such, it is useful to represent
    the above phrase LUT's entries in the following structure.
-----------------------------------------------------------------------------*/
typedef struct
{
    sdword duration;                            //duration of sentence, in milliseconds
    ubyte probability;                          //sentence variation, 0..100
    ubyte pad0;
    ubyte low[2];                               //low word of random sequence
    ubyte numoffsets;                           //number of speech fragments in this sentence
    ubyte pad1;
    ubyte high[2];                              //high word of random sequence
}
lookupinterp;

typedef struct
{
	sdword	ID;
	sword	columns;
	sword	rows;
	sdword	lookup[];
} TABLELUT;

typedef struct
{
	sdword	ID;
	sword	columns;
	sword	rows;
	real32	lookup[];
} TABLELUTFLOAT;


typedef struct
{
	sdword	ID;
	sword	columns;
	sword	rows;
	real32	lookup[];
} FREQUENCYLUT;

#endif
