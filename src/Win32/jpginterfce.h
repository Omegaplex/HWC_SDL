#ifndef ___JPGINTERFCE_H
#define ___JPGINTERFCE_H

#include <stdio.h>
#include "types.h"
#include "file.h"

typedef struct _jpegdata
{
    unsigned char *ptr;
    int width;
    int height;
    FILE *output_file;
    filehandle input_file;
    int aritcoding;
    int CCIR601sampling;
    int smoothingfactor;
    int quality;
    udword hWnd;
    int ProgressMsg;
    int status;
    int components;
} JPEGDATA;

void JpegWrite(JPEGDATA *data);
void JpegInfo(JPEGDATA *data);
void JpegRead(JPEGDATA *data);

#endif
