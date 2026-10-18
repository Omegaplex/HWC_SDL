/*=============================================================================
        Name    : glcaps.h
        Purpose : determine the capabilities of the GL currently in use

Created 19/06/1998 by khent
Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _GLCAPS_H
#define _GLCAPS_H

#ifndef SW_Render
#include <windows.h>
#endif
#include "types.h"
#include "glinc.h"
#include "glext.h"

#define GL_SWAPFRIENDLY 0xffe01
#define GL_COMPILED_ARRAYS_EXT 0xffe02

extern char GENERIC_OPENGL_RENDERER[];

extern GLenum glCapDepthFunc;
extern bool32 glNT;
extern bool32 gl95;

extern char const* GLC_VENDOR;
extern char const* GLC_RENDERER;
extern char const* GLC_EXTENSIONS;

bool32 glCapFastFeature(GLenum feature);
bool32 glCapFeatureExists(GLenum feature);
bool32 glCapTexSupport(GLenum format);
void glCapStartup(void);
bool32 glCapLoadOpenGL(char* dllName);
char const *glCapGetValidationSummary(void);

sdword glCapNumBuffers(void);
bool32 glCapNT(void);
bool32 glCap95(void);

#endif
