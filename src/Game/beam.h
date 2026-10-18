#ifndef ___BEAM_H
#define ___BEAM_H

#include "types.h"
#include "spaceobj.h"
#include "select.h"

void beamUpdateEffects(const vector *cameraPosition, SpaceObj *spaceobj);
void beamFlushEffects(void);
void beamRenderBeams(const vector *cameraPosition, SelectCommand *selection);

#endif
