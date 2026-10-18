/*=============================================================================
    Name    : BMP.c
    Purpose : Functions to read .BMP files.

    Created 1/11/1999 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include "file.h"
#include "debug.h"
#include "bmp.h"

typedef struct bmpBlobWindow
{
    sdword unknown0;
    ubyte *data;
    sdword offset;
    sdword length;
    ubyte pad[8];
} bmpBlobWindow;

typedef struct bmpImageRuntime
{
    ubyte pad0[1676];
    FILE *file;
    ubyte pad1[3740];
    bmpBlobWindow blobWindow;
    struct bmpImageRuntime *previous;
    struct bmpImageRuntime *next;
} bmpImageRuntime;

typedef struct bmpStdString
{
    udword unknown0;
    char *data;
    udword length;
    udword capacity;
} bmpStdString;

size_t ReadBlob(void *image, size_t length, void *data)
{
    bmpImageRuntime *bmpImage = (bmpImageRuntime *)image;
    ubyte *blobData = bmpImage->blobWindow.data;

    if (blobData == NULL)
    {
        return fread(data, 1, length, bmpImage->file);
    }

    if (length > (size_t)(bmpImage->blobWindow.length - bmpImage->blobWindow.offset))
    {
        length = (size_t)(bmpImage->blobWindow.length - bmpImage->blobWindow.offset);
    }

    if ((sdword)length > 0)
    {
        memcpy(data, blobData + bmpImage->blobWindow.offset, length);
    }
    bmpImage->blobWindow.offset += (sdword)length;
    return length;
}

sdword ReadByte(void *image)
{
    ubyte byte;

    if (ReadBlob(image, 1, &byte))
    {
        return byte;
    }

    return -1;
}

void seekdir(void)
{
}

real64 won_6F5C10(char *text, char **endPointer)
{
    return strtod(text, endPointer);
}

sdword won_6F5A60(char *geometry, sdword *xOffset, sdword *yOffset, udword *width, udword *height)
{
    char *cursor;
    char *endPointer;
    sdword flags;
    sdword value;
    udword parsedWidth;
    udword parsedHeight;

    cursor = geometry;
    flags = 0;
    parsedWidth = *width;
    parsedHeight = *height;
    if (geometry == NULL || *geometry == '\0')
    {
        return 0;
    }
    if (*cursor == '=')
    {
        cursor++;
    }

    if (*cursor != '+' && *cursor != '-' && *cursor != 'x' && *cursor != 'X')
    {
        value = (sdword)won_6F5C10(cursor, &endPointer);
        if (cursor == endPointer)
        {
            return 0;
        }
        parsedWidth = (udword)value;
        cursor = endPointer;
        flags |= 4;
    }

    if (*cursor == 'x' || *cursor == 'X')
    {
        value = (sdword)won_6F5C10(cursor + 1, &endPointer);
        if (cursor + 1 == endPointer)
        {
            return 0;
        }
        parsedHeight = (udword)value;
        cursor = endPointer;
        flags |= 8;
    }

    if (*cursor == '+' || *cursor == '-')
    {
        value = (sdword)won_6F5C10(cursor + 1, &endPointer);
        if (cursor + 1 == endPointer)
        {
            return 0;
        }
        if (*cursor == '-')
        {
            value = -value;
            flags |= 0x10;
        }
        *xOffset = value;
        cursor = endPointer;
        flags |= 1;

        if (*cursor == '+' || *cursor == '-')
        {
            value = (sdword)won_6F5C10(cursor + 1, &endPointer);
            if (cursor + 1 == endPointer)
            {
                return 0;
            }
            if (*cursor == '-')
            {
                value = -value;
                flags |= 0x20;
            }
            *yOffset = value;
            cursor = endPointer;
            flags |= 2;
        }
    }

    if (*cursor != '\0')
    {
        return 0;
    }

    if (flags & 4)
    {
        *width = parsedWidth;
    }
    if (flags & 8)
    {
        *height = parsedHeight;
    }
    return flags;
}

sdword won_6F56F0(char *geometry, sdword *xOffset, sdword *yOffset, udword *width, udword *height)
{
    char *src;
    char *dst;
    char filtered[1664];
    sdword flags;

    if (geometry == NULL || *geometry == '\0')
    {
        return 0;
    }

    flags = 0;
    dst = filtered;
    for (src = geometry; *src != '\0' && dst < filtered + sizeof(filtered) - 1; src++)
    {
        if (isspace((unsigned char)*src))
        {
            continue;
        }

        switch (*src)
        {
            case '!':
                flags |= 0x2000;
                break;
            case '%':
                flags |= 0x1000;
                break;
            case '<':
                flags |= 0x4000;
                break;
            case '>':
                flags |= 0x8000;
                break;
            default:
                *dst++ = *src;
                break;
        }
    }
    *dst = '\0';

    return flags | won_6F5A60(filtered, xOffset, yOffset, width, height);
}

/*=============================================================================
    Functions
=============================================================================*/

/*-----------------------------------------------------------------------------
    Name        : bmpFileOpen
    Description : Opens a .bmp file and reads in it's header.  Error checking
                    will be performed on the header to make sure it's a format
                    we can readily read.
    Inputs      : fileName - name of file to open
    Outputs     : header - where to store the bitmap information.
    Return      : a filehandle if the file was valid or 0 if invalid.
----------------------------------------------------------------------------*/
filehandle bmpFileOpen(bmpheader *header, char *fileName)
{
    filehandle handle;

    handle = fileOpen(fileName, FF_ReturnNULLOnFail);
    if (handle)
    {
        fileBlockRead(handle, header, sizeof(bmpheader));
        if (header->ident != BMP_Ident)
        {
#if BMP_VERBOSE_LEVEL >= 1
            dbgMessagef("\nBMP file '%s' has invalid ident of 0x%d", fileName, header->ident);
#endif

            fileClose(handle);
            return(0);
        }
        if (header->biBitCount != 24 || header->biCompression != 0)
        {
#if BMP_VERBOSE_LEVEL >= 1
            dbgMessagef("\nBMP file '%s' has invalid format", fileName);
#endif

            fileClose(handle);
            return(0);
        }
//        fileSeek(handle, 54, FS_Current);
        return(handle);
    }
    return(0);
}

/*-----------------------------------------------------------------------------
    Name        : bmpBodyRead
    Description : Read in and close the body of a previously opened .BMP file.
    Inputs      : handle - previously opened file handle.
                  header - header previously opened.
    Outputs     : dest - where to store the read file.
                    Must be header->biWidth * header->biWidth dwords.
    Return      : void
----------------------------------------------------------------------------*/
void bmpBodyRead(color *dest, filehandle handle, bmpheader *header)
{
    sdword x, y;
    ubyte colors[3];
    color *writeDest;

    for (y = header->biHeight - 1; y >= 0; y--)
    {
        writeDest = dest + y * header->biWidth;
        for (x = 0; x < header->biWidth; x++, writeDest++)
        {
            fileBlockRead(handle, colors, 3);
            *writeDest = colRGB(colors[2], colors[1], colors[0]);
        }
    }
    fileClose(handle);
}

bool32 bmpHeaderHasKnownSignature(char *signature, udword length)
{
    if (length < 2)
    {
        return FALSE;
    }

    if (!strncmp(signature, "BM", 2) ||
        !strncmp(signature, "IC", 2) ||
        !strncmp(signature, "PI", 2) ||
        !strncmp(signature, "CI", 2) ||
        !strncmp(signature, "CP", 2))
    {
        return TRUE;
    }

    return FALSE;
}

sdword *ResetRectangleInfo(sdword *rectangleInfo)
{
    rectangleInfo[0] = 0;
    rectangleInfo[1] = 0;
    rectangleInfo[2] = 0;
    rectangleInfo[3] = 0;
    return rectangleInfo;
}

typedef struct bmpGeometryInfo
{
    sdword width;
    sdword height;
    sdword xOffset;
    sdword yOffset;
    ubyte xNegative;
    ubyte yNegative;
    ubyte valid;
    ubyte percent;
    ubyte aspect;
    ubyte less;
    ubyte greater;
} bmpGeometryInfo;

bmpGeometryInfo *GeometryInfoConstruct(bmpGeometryInfo *geometryInfo, sdword width, sdword height, sdword xOffset, sdword yOffset, ubyte xNegative, ubyte yNegative)
{
    geometryInfo->width = width;
    geometryInfo->height = height;
    geometryInfo->xOffset = xOffset;
    geometryInfo->yOffset = yOffset;
    geometryInfo->xNegative = xNegative;
    geometryInfo->yNegative = yNegative;
    geometryInfo->valid = 1;
    geometryInfo->percent = 0;
    geometryInfo->aspect = 0;
    geometryInfo->less = 0;
    geometryInfo->greater = 0;
    return geometryInfo;
}

bmpImageRuntime *SyncNextImageInList(bmpImageRuntime *image)
{
    memcpy(&image->next->blobWindow, &image->blobWindow, sizeof(image->blobWindow));
    image->next->file = image->file;
    return image->next;
}

sdword GetImageListLength(bmpImageRuntime *image)
{
    sdword count = 0;

    while (image->previous != NULL)
    {
        image = image->previous;
    }

    while (image != NULL)
    {
        count++;
        image = image->next;
    }
    return count;
}
sdword bmpDecodeRLEData(void *input, sdword format, sdword width, sdword height, sbyte *output)
{
   sdword y = 0;
   sbyte *dest = output;
   sdword lastNibblePair = 0;
   sdword x = 0;
   sdword lastRow = 0;

   if (height * width > 0)
   {
       memset(output, 0, height * width);
   }

   if (height > 0)
   {
       while (TRUE)
       {
           sdword count = ReadByte(input);
           if (count == -1)
           {
               break;
           }

           if (count != 0)
           {
               sdword value = ReadByte(input);
               sdword i;

               lastNibblePair = value;
               if (count > 0)
               {
                   x += count;
                   for (i = 0; i < count; i++)
                   {
                       if (format == 1)
                       {
                           *dest = (sbyte)value;
                       }
                       else
                       {
                           sdword nibbleValue = value;
                           if ((i & 1) == 0)
                           {
                               nibbleValue >>= 4;
                           }
                           *dest = (sbyte)(nibbleValue & 0xF);
                       }
                       dest++;
                   }
               }
           }
           else
           {
               sdword command = ReadByte(input);

               switch (command)
               {
                   case 1:
                       return 1;

                   case 0:
                       y++;
                       x = 0;
                       lastRow = y;
                       dest = &output[width * y];
                       break;

                   case 2:
                       x += ReadByte(input);
                       y += ReadByte(input);
                       lastRow = y;
                       dest = &output[x + width * y];
                       break;

                   default:
                   {
                       sdword i;

                       x += command;
                       for (i = 0; i < command; i++)
                       {
                           sbyte value;

                           if (format == 1)
                           {
                               value = (sbyte)ReadByte(input);
                           }
                           else
                           {
                               sdword nibbleValue;

                               if ((i & 1) != 0)
                               {
                                   nibbleValue = lastNibblePair;
                               }
                               else
                               {
                                   nibbleValue = ReadByte(input);
                                   lastNibblePair = nibbleValue;
                               }

                               if ((i & 1) == 0)
                               {
                                   nibbleValue >>= 4;
                               }
                               value = (sbyte)(nibbleValue & 0xF);
                           }

                           *dest++ = value;
                       }

                       if (format == 1)
                       {
                           if ((command & 1) != 0)
                           {
                               ReadByte(input);
                           }
                       }
                       else
                       {
                           sdword remainder = command & 3;
                           if (remainder == 1 || remainder == 2)
                           {
                               ReadByte(input);
                           }
                       }

                       y = lastRow;
                       break;
                   }
               }
           }

           if (((~(height - y - 1) & (height - y - 2)) + 1) == (height - y - 1))
           {
               seekdir();
           }
           if (y >= height)
           {
               break;
           }
       }
   }

   ReadByte(input);
   ReadByte(input);
   return 1;
}

sdword bmpEncodeRLE8(ubyte *input, sdword width, sdword height, ubyte *output)
{
   ubyte *src = input;
   ubyte *dest = output;
   sdword row = 0;
   ubyte *rowStart = input;
   sdword paddedWidth = width + (width & 1);

   for (row = 0; row < height; row++)
   {
       sdword x = 0;

       if (paddedWidth > 0)
       {
           do
           {
               sdword runPos = x + 1;
               sdword runLength = 1;

               if (x + 1 < paddedWidth)
               {
                   sbyte value = *src;
                   ubyte *scan = &src[-x];

                   do
                   {
                       if (scan[runPos] != (ubyte)value || runLength == 255)
                       {
                           break;
                       }
                       runLength++;
                       runPos++;
                   }
                   while (runPos < paddedWidth);

                   src = rowStart;
               }

               *dest++ = (ubyte)runLength;
               *dest++ = *src;
               x += runLength;
               src += runLength;
               rowStart = src;
           }
           while (x < paddedWidth);
       }

       *dest++ = 0;
       *dest++ = 0;

       if (((~(height - row - 1) & (height - row - 2)) + 1) == (height - row - 1))
       {
           seekdir();
       }
   }

   *dest++ = 0;
   *dest++ = 1;
   return (sdword)(dest - output);
}

char *GetImageInfo(char *imageInfo)
{
   memset(imageInfo, 0, 24);
   *(sdword *)(imageInfo + 24) = 0;
   *(imageInfo + 28) = 0;
   *(imageInfo + 1692) = 0;

   *(imageInfo + 3356) = 0;
   strcat(imageInfo + 3356, "U");
   *(imageInfo + 5020) = 0;

   *(sdword *)(imageInfo + 6684) = 0;
   *(sdword *)(imageInfo + 6688) = 1;
   *(sdword *)(imageInfo + 6692) = 0;
   *(sdword *)(imageInfo + 6696) = 0;
   *(sdword *)(imageInfo + 6700) = 8;
   *(sdword *)(imageInfo + 6704) = 0;
   *(sdword *)(imageInfo + 6708) = 0;
   *(sdword *)(imageInfo + 6712) = 0;
   *(sdword *)(imageInfo + 6716) = 0;
   *(sdword *)(imageInfo + 6720) = 1;
   *(sdword *)(imageInfo + 6724) = 0;
   *(sdword *)(imageInfo + 6728) = 0;
   *(sdword *)(imageInfo + 6732) = 75;
   *(sdword *)(imageInfo + 6736) = 0;
   *(sdword *)(imageInfo + 6740) = 0;
   *(sdword *)(imageInfo + 6744) = 0;
   *(sdword *)(imageInfo + 6748) = 0;
   *(sdword *)(imageInfo + 6752) = 0;
   *(sdword *)(imageInfo + 6756) = 0;
   *(double *)(imageInfo + 6760) = atof("12");
   *(sdword *)(imageInfo + 6768) = 1;
   *(sdword *)(imageInfo + 6772) = 1;
   *(sdword *)(imageInfo + 6776) = 0;
   strcpy(imageInfo + 6780, "#bdbdbd");
   strcpy(imageInfo + 6784, "#bdbdbd");
   strcpy(imageInfo + 6788, "#bdbdbd");
   *(sdword *)(imageInfo + 6792) = 1;
   *(sdword *)(imageInfo + 6796) = 0;
   *(sdword *)(imageInfo + 6800) = 0;
   *(sdword *)(imageInfo + 6804) = 0;
   *(sdword *)(imageInfo + 6808) = 0;
   *(sdword *)(imageInfo + 6812) = 0;
   *(sdword *)(imageInfo + 6816) = 0;
   *(sdword *)(imageInfo + 6820) = 0;
   *(sdword *)(imageInfo + 6824) = 0;
   *(sdword *)(imageInfo + 6828) = 0;
   *(sdword *)(imageInfo + 6832) = 29;
   *(sdword *)(imageInfo + 6836) = 0;
   *(sdword *)(imageInfo + 6840) = 0;
   return imageInfo;
}

static char *bmpCloneString(const char *source)
{
   size_t length;
   char *copy;

   if (source == NULL)
   {
       return NULL;
   }

   length = strlen(source) + 1;
   copy = malloc(length);
   if (copy == NULL)
   {
       dbgFatalf(DBG_Loc, "Unable to clone image info string");
       return NULL;
   }

   memcpy(copy, source, length);
   return copy;
}

char **CloneImageInfo(char **source)
{
   static const sdword cloneStringIndices[] =
   {
       1677, 1678, 1679, 1684, 1685, 1686, 1687, 1688, 1689, 1701, 1702, 1703, 1709
   };
   char **clone;
   sdword i;

   clone = malloc(0x1AC0);
   if (clone == NULL)
   {
       dbgFatalf(DBG_Loc, "Unable to clone image info");
       return NULL;
   }

   if (source != NULL)
   {
       memcpy(clone, source, 0x1AC0);
       for (i = 0; i < sizeof(cloneStringIndices) / sizeof(cloneStringIndices[0]); i++)
       {
           if (source[cloneStringIndices[i]] != NULL)
           {
               clone[cloneStringIndices[i]] = bmpCloneString(source[cloneStringIndices[i]]);
           }
       }
   }
   else
   {
       GetImageInfo((char *)clone);
   }

   return clone;
}

void DestroyImageInfo(void **imageInfo)
{
   static const sdword destroyOrder[] =
   {
       1684, 1677, 1678, 1679, 1689, 1701, 1702, 1703, 1688, 1685, 1686, 1687, 1709
   };
   sdword i;

   for (i = 0; i < sizeof(destroyOrder) / sizeof(destroyOrder[0]); i++)
   {
       if (imageInfo[destroyOrder[i]] != NULL)
       {
           free(imageInfo[destroyOrder[i]]);
           imageInfo[destroyOrder[i]] = NULL;
       }
   }

   free(imageInfo);
}
#if 0  // Decompiled pseudocode reference
/*=== AllocateImage (0X6E4BA0) ===*/
_DWORD *__cdecl sub_6E4BA0(int a1)
{
_DWORD *v1; // eax
_DWORD *v2; // ebx
void *v4; // edi
int v5; // edx
const char *v6; // eax
_DWORD *v7; // edi
_DWORD *v8; // esi
char *v9; // eax
const char *v10; // eax
const char *v11; // ecx
char *v12; // eax
int v13; // eax
int v14; // eax
int v15; // eax
int v16; // [esp+Ch] [ebp-4h] BYREF
v1 = sub_6FAB50(0x1568u);
v2 = v1;
if ( v1 )
{
v4 = v1 + 1355;
sub_6F99B0(v1 + 1355);
v2[419] = 0;
*v2 = 0;
v2[1] = 0;
v2[2] = 0;
*((_BYTE *)v2 + 12) = 0;
v2[420] = 0;
v2[421] = 0;
strcpy((char *)v2 + 1688, "MIFF");
v2[838] = 0;
v2[839] = 0;
v2[840] = 1;
v2[841] = 0;
v2[842] = 0;
v2[843] = 0;
v2[844] = 0;
v2[845] = 8;
v2[847] = 0;
v2[848] = 0;
v2[849] = 0;
v2[850] = 0;
v2[846] = 0;
v2[851] = 1;
v2[852] = 0;
v2[890] = 0;
v2[892] = 0;
v2[893] = 0;
v2[894] = 0;
v2[895] = 0;
v2[853] = 0;
v2[854] = 0;
v2[855] = 0;
v2[856] = 0;
v2[857] = 1;
v2[858] = 0;
v2[860] = 0;
v2[861] = 0;
v2[862] = 0;
v2[863] = 0;
v2[864] = 0;
v2[865] = 0;
v2[868] = 0;
v2[869] = 0;
v2[870] = 0;
v2[871] = 0;
v2[874] = 0;
v2[875] = 0;
v2[876] = 0;
v2[877] = 0;
v2[880] = 0;
v2[881] = 0;
v2[882] = 0;
v2[883] = 0;
v2[886] = 0;
v2[887] = 0;
v2[888] = 0;
v2[889] = 0;
sub_6FA580(v2 + 1361);
v2[1364] = 0;
v2[1365] = 0;
v2[1362] = 0;
v2[1363] = 0;
v2[897] = 0;
v2[898] = 0;
sub_6FC260(off_8DA830[0], (_BYTE *)v2 + 3596);
sub_6FC260(off_8DA834[0], (_BYTE *)v2 + 3600);
sub_6FC260(off_8DA83C, (_BYTE *)v2 + 3604);
v2[902] = 0;
ResetRectangleInfo(v2 + 903);
v2[912] = 0;
v2[916] = 0;
v2[918] = 0;
v2[907] = 0;
v2[908] = 0;
v2[909] = 1;
v2[910] = 0;
v2[911] = 13;
v2[913] = 1072693248;
v2[914] = 0;
v2[917] = 0;
v2[919] = 0;
v2[915] = 0;
v2[896] = 0;
*((_BYTE *)v2 + 3680) = 0;
v2[1336] = 0;
v2[1337] = 0;
sub_6FD730(v2 + 1340);
v5 = a1;
v2[1339] = 0;
v2[1354] = 0;
v2[1366] = 0;
v2[1367] = 0;
v2[1368] = 0;
v2[1338] = 0;
if ( a1 )
{
qmemcpy(v4, (const void *)a1, 0x18u);
*v2 = *(_DWORD *)(a1 + 24) != 0;
strcpy((char *)v2 + 12, (const char *)(a1 + 28));
strcpy((char *)v2 + 3680, (const char *)(a1 + 28));
strcpy((char *)v2 + 1688, (const char *)(a1 + 1692));
v6 = *(const char **)(a1 + 6708);
if ( v6 )
{
v7 = v2 + 844;
v8 = v2 + 843;
sscanf(v6, "%ux%u", v2 + 843, v2 + 844);
if ( (sub_6F5A60(*(char **)(a1 + 6708), v2 + 846, &v16, v2 + 843, v2 + 844) & 8) == 0 )
*v7 = *v8;
v2[847] = *v8;
v5 = a1;
v2[848] = *v7;
}
v9 = *(char **)(v5 + 6712);
if ( v9 )
{
if ( !sub_6E75D0(v9, 0) )
{
sscanf(*(const char **)(a1 + 6712), "%ux%u", v2 + 843, v2 + 844);
if ( (sub_6F5A60(*(char **)(a1 + 6712), v2 + 849, v2 + 850, v2 + 843, v2 + 844) & 8) == 0 )
v2[844] = v2[843];
}
v5 = a1;
}
v2[842] = *(_DWORD *)(v5 + 6728);
v2[851] = *(_DWORD *)(v5 + 6720);
v2[890] = *(_DWORD *)(v5 + 6724);
v10 = *(const char **)(v5 + 6756);
if ( v10 )
{
if ( sscanf(v10, "%lfx%lf", v2 + 892, v2 + 894) != 2 )
{
v2[894] = v2[892];
v2[895] = v2[893];
}
v5 = a1;
}
v11 = *(const char **)(v5 + 6716);
if ( v11 )
{
v12 = sub_6F5CB0(v11);
ParseImageGeometry(v12, (int)(v2 + 905), (int)(v2 + 906), (int)(v2 + 903), (int)(v2 + 904));
v5 = a1;
}
if ( *(_DWORD *)(v5 + 6804) )
{
v13 = atoi(*(const char **)(v5 + 6804));
v5 = a1;
v2[907] = v13;
}
if ( *(_DWORD *)(v5 + 6808) )
{
v14 = atoi(*(const char **)(v5 + 6808));
v5 = a1;
v2[908] = v14;
}
if ( *(_DWORD *)(v5 + 6812) )
{
v15 = atoi(*(const char **)(v5 + 6812));
v5 = a1;
v2[909] = v15;
}
v2[845] = *(_DWORD *)(v5 + 6700);
v2[899] = *(_DWORD *)(v5 + 6780);
v2[900] = *(_DWORD *)(v5 + 6784);
v2[901] = *(_DWORD *)(v5 + 6788);
}
return v2;
}
else
{
sub_6FD910(300, (int)aUnableToAlloca, (int)aMemoryAllocati);
return 0;
}
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== AllocateNextImage (0X6E50F0) ===*/
char *__cdecl sub_6E50F0(int a1, int a2)
{
char *result; // eax
rinCloseCRCLog(*(_DWORD *)(a2 + 5444));
result = (char *)AllocateImage(a1);
*(_DWORD *)(a2 + 5472) = result;
if ( result )
{
strcpy(result + 12, (const char *)(a2 + 12));
if ( a1 )
strcpy((char *)(*(_DWORD *)(a2 + 5472) + 12), (const char *)(a1 + 28));
qmemcpy((void *)(*(_DWORD *)(a2 + 5472) + 5420), (const void *)(a2 + 5420), 0x18u);
*(_DWORD *)(*(_DWORD *)(a2 + 5472) + 1680) = *(_DWORD *)(a2 + 1680);
*(_DWORD *)(*(_DWORD *)(a2 + 5472) + 1676) = *(_DWORD *)(a2 + 1676);
*(_DWORD *)(*(_DWORD *)(a2 + 5472) + 1680) = *(_DWORD *)(a2 + 1680);
*(_DWORD *)(*(_DWORD *)(a2 + 5472) + 3408) = *(_DWORD *)(a2 + 3408) + 1;
result = *(char **)(a2 + 5472);
*((_DWORD *)result + 1366) = a2;
}
return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== CloneImage (0X6E51F0) ===*/
void **__cdecl sub_6E51F0(int a1, void *a2, void *a3, int a4)
{
void **v4; // ebx
void **result; // eax
size_t v7; // esi
unsigned int v8; // esi
unsigned int v9; // esi
void *v10; // eax
int v11; // edi
const void *v12; // esi
void *v13; // eax
const char *v14; // ebp
_DWORD *v15; // eax
_DWORD *v16; // eax
int v17; // [esp+14h] [ebp+4h]
v4 = (void **)sub_6FAB50(0x1568u);
if ( !v4 )
return 0;
qmemcpy(v4, (const void *)a1, 0x1568u);
v4[853] = 0;
v4[854] = 0;
v4[838] = 0;
v4[839] = 0;
v4[896] = 0;
if ( *(_DWORD *)(a1 + 3352) )
sub_6F5630(v4 + 838, *(const char **)(a1 + 3352));
if ( *(_DWORD *)(a1 + 3356) )
sub_6F5630(v4 + 839, *(const char **)(a1 + 3356));
if ( *(_DWORD *)(a1 + 3420) )
{
v7 = 4 * *(_DWORD *)(a1 + 3424);
result = (void **)sub_6FAB50(v7);
v4[855] = result;
if ( !result )
return result;
qmemcpy(result, *(const void **)(a1 + 3420), v7);
}
v8 = *(_DWORD *)(a1 + 3544);
if ( v8 )
{
result = (void **)sub_6FAB50(*(_DWORD *)(a1 + 3544));
v4[887] = result;
if ( !result )
return result;
qmemcpy(result, *(const void **)(a1 + 3548), v8);
}
v9 = *(_DWORD *)(a1 + 3552);
if ( v9 )
{
result = (void **)sub_6FAB50(*(_DWORD *)(a1 + 3552));
v4[889] = result;
if ( !result )
return result;
qmemcpy(result, *(const void **)(a1 + 3556), v9);
}
sub_6F99B0(v4 + 1355);
sub_6FA580(v4 + 1361);
v4[1364] = 0;
v4[1365] = 0;
v4[1362] = 0;
v4[1363] = 0;
v4[897] = 0;
v4[898] = 0;
if ( *(void **)(a1 + 3372) == a2 && (v10 = *(void **)(a1 + 3376), v10 == a3) )
{
v11 = 0;
v17 = 0;
if ( (int)v10 > 0 )
{
while ( 1 )
{
v12 = (const void *)sub_6F6DD0(a1, 0, v11, *(_DWORD *)(a1 + 3372), 1);
v13 = (void *)imagePreparePixelRegion((int)v4, 0, v11, (unsigned int)v4[843], 1);
if ( !v12 || !v13 )
break;
if ( *(_DWORD *)(a1 + 3360) == 2 )
qmemcpy(v4[898], *(const void **)(a1 + 3592), 2 * *(_DWORD *)(a1 + 3372));
qmemcpy(v13, v12, 4 * *(_DWORD *)(a1 + 3372));
if ( !imageSyncPixelRegion((int)v4) )
break;
if ( ++v17 >= *(_DWORD *)(a1 + 3376) )
break;
v11 = v17;
}
}
if ( *(_DWORD *)(a1 + 3412) )
sub_6F5630(v4 + 853, *(const char **)(a1 + 3412));
if ( *(_DWORD *)(a1 + 3416) )
sub_6F5630(v4 + 854, *(const char **)(a1 + 3416));
v14 = *(const char **)(a1 + 3584);
if ( v14 )
sub_6F5630(v4 + 896, v14);
}
else
{
v4[843] = a2;
v4[844] = a3;
ResetRectangleInfo(v4 + 903);
}
if ( v4[1354] || a4 )
{
v4[1354] = 0;
*v4 = (void *)1;
v4[1366] = 0;
v4[1368] = 0;
}
else
{
v15 = v4[1366];
if ( v15 )
v15[1368] = v4;
v16 = v4[1368];
if ( v16 )
{
v16[1366] = v4;
return v4;
}
}
return v4;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== DestroyImage (0X6E68B0) ===*/
void __cdecl sub_6E68B0(void *a1)
{
void *v1; // eax
int v2; // eax
int v3; // ecx
int v4; // eax
int v5; // ecx
sub_6F9980((int)a1 + 5420);
if ( *((_DWORD *)a1 + 419) )
{
sub_6F98D0((int)a1);
if ( *((_DWORD *)a1 + 2) )
remove((const char *)a1 + 12);
}
pixelCacheBackingStoreDestroy(*((void **)a1 + 1361));
v1 = (void *)*((_DWORD *)a1 + 838);
*((_DWORD *)a1 + 1361) = 0;
if ( v1 )
sub_6FAB60(v1);
if ( *((_DWORD *)a1 + 839) )
sub_6FAB60(*((void **)a1 + 839));
if ( *((_DWORD *)a1 + 853) )
sub_6FAB60(*((void **)a1 + 853));
if ( *((_DWORD *)a1 + 854) )
sub_6FAB60(*((void **)a1 + 854));
if ( *((_DWORD *)a1 + 855) )
sub_6FAB60(*((void **)a1 + 855));
if ( *((_DWORD *)a1 + 886) )
sub_6FAB60(*((void **)a1 + 887));
if ( *((_DWORD *)a1 + 888) )
sub_6FAB60(*((void **)a1 + 889));
if ( *((_DWORD *)a1 + 896) )
sub_6FAB60(*((void **)a1 + 896));
if ( !*((_DWORD *)a1 + 1354) )
{
v2 = *((_DWORD *)a1 + 1366);
if ( v2 )
{
v3 = *((_DWORD *)a1 + 1368);
if ( v3 )
*(_DWORD *)(v2 + 5472) = v3;
else
*(_DWORD *)(v2 + 5472) = 0;
}
v4 = *((_DWORD *)a1 + 1368);
if ( v4 )
{
v5 = *((_DWORD *)a1 + 1366);
if ( v5 )
{
*(_DWORD *)(v4 + 5464) = v5;
sub_6FAB60(a1);
return;
}
*(_DWORD *)(v4 + 5464) = 0;
}
}
sub_6FAB60(a1);
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== DestroyImageList (0X6E6B80) ===*/
_DWORD *__cdecl sub_6E6B80(_DWORD *a1)
{
_DWORD *result; // eax
_DWORD *i; // ecx
_DWORD *v3; // esi
result = a1;
if ( a1 )
{
for ( i = (_DWORD *)a1[1366]; i; i = (_DWORD *)i[1366] )
result = i;
do
{
v3 = (_DWORD *)result[1368];
if ( v3 )
v3[1366] = 0;
DestroyImage(result);
result = v3;
}
while ( v3 );
}
return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== ExportImagePixelArea (0x6E6E00) ===*/
int (__cdecl *__cdecl ExportImagePixelArea(
        _DWORD *a1,
        int a2,
        int a3,
        unsigned int a4,
        int a5,
        const char *a6,
        int a7,
        _BYTE *a8))(int, int, int)
{
  unsigned int i; // ebx
  int (__cdecl *result)(int, int, int); // eax
  _BYTE *v12; // ebp
  _BYTE *v13; // edx
  const char *v14; // ebx
  int v15; // esi
  char v16; // cl
  unsigned __int8 *v19; // esi
  _WORD *v20; // edx
  int v21; // ebp
  __int16 v22; // cx
  unsigned __int8 *v25; // esi
  _DWORD *v26; // edx
  int v27; // ebp
  int v28; // ecx
  unsigned __int8 *v31; // ebx
  float *v32; // esi
  unsigned int v33; // edx
  unsigned int v34; // ebp
  const char *v35; // edi
  double v36; // st7
  unsigned __int8 *v39; // ebx
  double *v40; // esi
  unsigned int v41; // edx
  int v42; // ebp
  double v43; // st7
  unsigned int j; // [esp+20h] [ebp+10h]
  unsigned int k; // [esp+20h] [ebp+10h]
  unsigned int m; // [esp+20h] [ebp+10h]
  unsigned int n; // [esp+20h] [ebp+10h]
  unsigned int ii; // [esp+20h] [ebp+10h]
  unsigned int v49; // [esp+24h] [ebp+14h]
  unsigned int v50; // [esp+24h] [ebp+14h]
  unsigned int v51; // [esp+24h] [ebp+14h]
  unsigned int v52; // [esp+24h] [ebp+14h]
  unsigned int v53; // [esp+24h] [ebp+14h]

  for ( i = 0; i < strlen(a6); ++i )
  {
    switch ( a6[i] )
    {
      case 'C':
      case 'K':
      case 'M':
      case 'Y':
      case 'c':
      case 'k':
      case 'm':
      case 'y':
        if ( a1[857] != 11 )
          sub_6E81C0(a1, 11);
        break;
      default:
        continue;
    }
  }
  switch ( a7 )
  {
    case 0:
      result = (int (__cdecl *)(int, int, int))sub_6F6DD0((int)a1, a2, a3, a4, a5);
      v12 = result;
      if ( result )
      {
        v13 = a8;
        result = (int (__cdecl *)(int, int, int))(a5 * a4);
        v49 = 0;
        for ( j = a5 * a4; v49 < j; ++v49 )
        {
          v14 = a6;
          v15 = 0;
          if ( strlen(a6) )
          {
            while ( 2 )
            {
              switch ( a6[v15] )
              {
                case 'A':
                case 'K':
                case 'a':
                case 'k':
                  v16 = v12[3];
                  goto LABEL_15;
                case 'B':
                case 'M':
                case 'b':
                case 'm':
                  *v13 = *v12;
                  goto LABEL_16;
                case 'C':
                case 'R':
                case 'c':
                case 'r':
                  *v13 = v12[2];
                  goto LABEL_16;
                case 'G':
                case 'Y':
                case 'g':
                case 'y':
                  v16 = v12[1];
LABEL_15:
                  *v13 = v16;
LABEL_16:
                  ++v13;
                  if ( ++v15 >= strlen(a6) )
                    break;
                  continue;
                default:
                  return sub_6FD910(310, (int)aInvalidPixelMa, (int)v14);
              }
              break;
            }
          }
          v12 += 4;
          result = (int (__cdecl *)(int, int, int))(v49 + 1);
        }
      }
      break;
    case 1:
      result = (int (__cdecl *)(int, int, int))sub_6F6DD0((int)a1, a2, a3, a4, a5);
      v19 = (unsigned __int8 *)result;
      if ( result )
      {
        v20 = a8;
        v50 = 0;
        for ( k = a5 * a4; v50 < k; ++v50 )
        {
          v14 = a6;
          v21 = 0;
          if ( strlen(a6) )
          {
            while ( 2 )
            {
              switch ( a6[v21] )
              {
                case 'A':
                case 'K':
                case 'a':
                case 'k':
                  v22 = v19[3];
                  goto LABEL_28;
                case 'B':
                case 'M':
                case 'b':
                case 'm':
                  *v20 = *v19;
                  goto LABEL_29;
                case 'C':
                case 'R':
                case 'c':
                case 'r':
                  *v20 = v19[2];
                  goto LABEL_29;
                case 'G':
                case 'Y':
                case 'g':
                case 'y':
                  v22 = v19[1];
LABEL_28:
                  *v20 = v22;
LABEL_29:
                  ++v20;
                  if ( ++v21 >= strlen(a6) )
                    break;
                  continue;
                default:
                  return sub_6FD910(310, (int)aInvalidPixelMa, (int)v14);
              }
              break;
            }
          }
          v19 += 4;
          result = (int (__cdecl *)(int, int, int))(v50 + 1);
        }
      }
      break;
    case 2:
      result = (int (__cdecl *)(int, int, int))sub_6F6DD0((int)a1, a2, a3, a4, a5);
      v25 = (unsigned __int8 *)result;
      if ( result )
      {
        v26 = a8;
        result = (int (__cdecl *)(int, int, int))(a5 * a4);
        v51 = 0;
        for ( m = a5 * a4; v51 < m; ++v51 )
        {
          v14 = a6;
          v27 = 0;
          if ( strlen(a6) )
          {
            while ( 2 )
            {
              switch ( a6[v27] )
              {
                case 'A':
                case 'K':
                case 'a':
                case 'k':
                  v28 = v25[3];
                  goto LABEL_40;
                case 'B':
                case 'M':
                case 'b':
                case 'm':
                  *v26 = *v25;
                  goto LABEL_41;
                case 'C':
                case 'R':
                case 'c':
                case 'r':
                  *v26 = v25[2];
                  goto LABEL_41;
                case 'G':
                case 'Y':
                case 'g':
                case 'y':
                  v28 = v25[1];
LABEL_40:
                  *v26 = v28;
LABEL_41:
                  ++v26;
                  if ( ++v27 >= strlen(a6) )
                    break;
                  continue;
                default:
                  return sub_6FD910(310, (int)aInvalidPixelMa, (int)v14);
              }
              break;
            }
          }
          v25 += 4;
          result = (int (__cdecl *)(int, int, int))(v51 + 1);
        }
      }
      break;
    case 3:
      result = (int (__cdecl *)(int, int, int))sub_6F6DD0((int)a1, a2, a3, a4, a5);
      v31 = (unsigned __int8 *)result;
      if ( result )
      {
        v32 = (float *)a8;
        v33 = a5 * a4;
        v52 = 0;
        for ( n = a5 * a4; v52 < v33; ++v52 )
        {
          v34 = 0;
          if ( strlen(a6) )
          {
            do
            {
              v35 = a6;
              switch ( a6[v34] )
              {
                case 'A':
                case 'K':
                case 'a':
                case 'k':
                  v36 = (double)(v31[3] / 0xFFu);
                  break;
                case 'B':
                case 'M':
                case 'b':
                case 'm':
                  v36 = (double)(*v31 / 0xFFu);
                  break;
                case 'C':
                case 'R':
                case 'c':
                case 'r':
                  v36 = (double)(v31[2] / 0xFFu);
                  break;
                case 'G':
                case 'Y':
                case 'g':
                case 'y':
                  v36 = (double)(v31[1] / 0xFFu);
                  break;
                default:
                  return sub_6FD910(310, (int)aInvalidPixelMa, (int)v35);
              }
              *v32++ = v36;
              ++v34;
            }
            while ( v34 < strlen(a6) );
            v33 = n;
          }
          v31 += 4;
          result = (int (__cdecl *)(int, int, int))(v52 + 1);
        }
      }
      break;
    case 4:
      result = (int (__cdecl *)(int, int, int))sub_6F6DD0((int)a1, a2, a3, a4, a5);
      v39 = (unsigned __int8 *)result;
      if ( result )
      {
        v40 = (double *)a8;
        v41 = a5 * a4;
        v53 = 0;
        for ( ii = a5 * a4; v53 < v41; ++v53 )
        {
          v42 = 0;
          if ( strlen(a6) )
          {
            while ( 2 )
            {
              v35 = a6;
              switch ( a6[v42] )
              {
                case 'A':
                case 'K':
                case 'a':
                case 'k':
                  v43 = (double)(v39[3] / 0xFFu);
                  goto LABEL_64;
                case 'B':
                case 'M':
                case 'b':
                case 'm':
                  v43 = (double)(*v39 / 0xFFu);
                  goto LABEL_64;
                case 'C':
                case 'R':
                case 'c':
                case 'r':
                  v43 = (double)(v39[2] / 0xFFu);
                  goto LABEL_64;
                case 'G':
                case 'Y':
                case 'g':
                case 'y':
                  v43 = (double)(v39[1] / 0xFFu);
LABEL_64:
                  *v40++ = v43;
                  if ( ++v42 < strlen(a6) )
                    continue;
                  v41 = ii;
                  break;
                default:
                  return sub_6FD910(310, (int)aInvalidPixelMa, (int)v35);
              }
              break;
            }
          }
          v39 += 4;
          result = (int (__cdecl *)(int, int, int))(v53 + 1);
        }
      }
      break;
    default:
      result = sub_6FD910(310, (int)aInvalidPixelTy, 0);
      break;
  }
  return result;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== SetImageOpacity (0x6E7700) ===*/
int __cdecl SetImageOpacity(_DWORD *a1, char a2)
{
  int v2; // edi
  int result; // eax
  int v4; // ecx
  _BYTE *v5; // eax

  v2 = 0;
  result = a1[844];
  a1[840] = 1;
  a1[841] = 1;
  if ( result > 0 )
  {
    do
    {
      result = sub_6F6DD0((int)a1, 0, v2, a1[843], 1);
      if ( !result )
        break;
      v4 = 0;
      if ( (int)a1[843] > 0 )
      {
        v5 = (_BYTE *)(result + 3);
        do
        {
          *v5 = a2;
          v5 += 4;
          ++v4;
        }
        while ( v4 < a1[843] );
      }
      result = imageSyncPixelRegion((int)a1);
      if ( !result )
        break;
      result = a1[844];
      ++v2;
    }
    while ( v2 < result );
  }
  return result;
}
#endif

sdword ParseImageGeometry(char *geometry, sdword *xOffset, sdword *yOffset, udword *width, udword *height)
{
    real64 widthScale;
    real64 heightScale;
    real64 scale;
    udword parsedWidth;
    udword parsedHeight;
    udword originalWidth;
    udword originalHeight;
    sdword parsedX;
    sdword parsedY;
    sdword flags;
    sdword widthIndependent;
    sdword heightIndependent;
    sdword delta;
    sdword scanCount;

    if (geometry == NULL || *geometry == '\0')
    {
        return 0;
    }

    originalWidth = *width;
    originalHeight = *height;
    flags = won_6F56F0(geometry, xOffset, yOffset, width, height);
    if (flags & 0x1000)
    {
        widthScale = *width;
        heightScale = *height;
        scanCount = sscanf(geometry, "%lf%%x%lf", &widthScale, &heightScale);
        if (scanCount != 2)
        {
            scanCount = sscanf(geometry, "%lfx%lf", &widthScale, &heightScale);
        }
        if (scanCount == 1)
        {
            heightScale = widthScale;
        }

        *width = (udword)((real64)originalWidth * widthScale * 0.01);
        if (*width <= 1)
        {
            *width = 1;
        }
        *height = (udword)((real64)originalHeight * heightScale * 0.01);
        if (*height <= 1)
        {
            *height = 1;
        }
        originalWidth = *width;
        originalHeight = *height;
    }

    if ((flags & 0x2000) == 0)
    {
        scale = 1.0;
        if (originalWidth * originalHeight != 0)
        {
            if ((flags & 4) && (flags & 8))
            {
                scale = (real64)*width / (real64)originalWidth;
                if (scale > (real64)*height / (real64)originalHeight)
                {
                    scale = (real64)*height / (real64)originalHeight;
                }
            }
            else if (flags & 4)
            {
                scale = (real64)*width / (real64)originalWidth;
            }
            else
            {
                scale = (real64)*height / (real64)originalHeight;
            }
        }

        *width = (udword)((real64)originalWidth * scale);
        if (*width <= 1)
        {
            *width = 1;
        }
        *height = (udword)((real64)originalHeight * scale);
        if (*height <= 1)
        {
            *height = 1;
        }
    }

    widthIndependent = flags & 1;
    if ((flags & 1) == 0)
    {
        *width -= 2 * *xOffset;
    }
    heightIndependent = flags & 2;
    if ((flags & 2) == 0)
    {
        *height -= 2 * *yOffset;
    }

    if (flags & 0x8000)
    {
        if (originalWidth < *width)
        {
            *width = originalWidth;
        }
        if (originalHeight < *height)
        {
            *height = originalHeight;
        }
    }
    if (flags & 0x4000)
    {
        if (originalWidth > *width)
        {
            *width = originalWidth;
        }
        if (originalHeight > *height)
        {
            *height = originalHeight;
        }
    }

    parsedWidth = *width;
    parsedHeight = *height;
    parsedX = *xOffset;
    parsedY = *yOffset;
    won_6F56F0(geometry, &parsedX, &parsedY, &parsedWidth, &parsedHeight);

    if (widthIndependent)
    {
        if (flags & 0x10)
        {
            *xOffset += parsedWidth - *width;
        }
    }
    else
    {
        delta = (sdword)parsedWidth - (sdword)*width;
        if (delta >= 0)
        {
            *xOffset = delta >> 1;
        }
    }

    if (heightIndependent)
    {
        if (flags & 0x20)
        {
            *yOffset += parsedHeight - *height;
        }
    }
    else
    {
        delta = (sdword)parsedHeight - (sdword)*height;
        if (delta >= 0)
        {
            *yOffset = delta >> 1;
        }
    }

    if (flags & 0x8000)
    {
        if (*width + 2 * *xOffset > parsedWidth)
        {
            delta = 2 * *xOffset;
            if ((sdword)*width > delta)
            {
                *width -= delta;
            }
            delta = 2 * *yOffset;
            if ((sdword)*height > delta)
            {
                *height -= delta;
            }
        }
        if (*height + 2 * *yOffset > parsedHeight)
        {
            delta = 2 * *xOffset;
            if ((sdword)*width > delta)
            {
                *width -= delta;
            }
            delta = 2 * *yOffset;
            if ((sdword)*height > delta)
            {
                *height -= delta;
            }
        }
    }

    return flags;
}

#if 0  // Decompiled pseudocode reference
/*=== ReadImage (0x6E7B20) ===*/
char *__cdecl ReadImage(int a1)
{
  char *result; // eax
  int v2; // ebx
  int v3; // eax
  int (__cdecl *v4)(int); // eax
  char *v5; // ebx
  signed int v6; // esi
  int v7; // eax
  void **v8; // edi
  int v9; // eax
  const char *v10; // esi
  int v11; // eax
  bool v12; // zf
  unsigned int v13; // eax
  int v14; // ebp
  int v15; // esi
  void **v16; // eax
  int v17; // eax
  void **i; // eax
  unsigned int v19; // eax
  unsigned int v20; // ecx
  BOOL v21; // ecx
  int v22; // eax
  int j; // eax
  int v24; // ebp
  char *lpTempFileName; // [esp+10h] [ebp-13A4h]
  unsigned int v26; // [esp+14h] [ebp-13A0h] BYREF
  char *v27; // [esp+18h] [ebp-139Ch]
  int v28; // [esp+1Ch] [ebp-1398h] BYREF
  unsigned int v29; // [esp+20h] [ebp-1394h] BYREF
  char v30[1664]; // [esp+24h] [ebp-1390h] BYREF
  char v31[3344]; // [esp+6A4h] [ebp-D10h] BYREF

  lpTempFileName = (char *)(a1 + 28);
  if ( *(_BYTE *)(a1 + 28) == 64 )
    return (char *)ReadImages(a1);
  SetImageInfo(a1, 0);
  v2 = 0;
  strcpy(v30, (const char *)(a1 + 28));
  v3 = sub_6FABF0((char *)(a1 + 1692));
  if ( v3 )
  {
    v4 = *(int (__cdecl **)(int))(v3 + 4);
    if ( v4 )
      goto LABEL_5;
  }
  if ( !sub_6FC740((char *)(a1 + 1692), 0, v31) )
    goto LABEL_12;
  result = (char *)AllocateImage(a1);
  v5 = result;
  if ( !result )
    return result;
  v27 = result + 12;
  strcpy(result + 12, lpTempFileName);
  sub_6F6590(lpTempFileName);
  v6 = sub_6FCDC0(a1, (int)v5, (char *)(a1 + 1692), 0);
  DestroyImageList(v5);
  v2 = 0;
  if ( !v6 )
    *(_DWORD *)(a1 + 6684) = 1;
  SetImageInfo(a1, 0);
  v7 = sub_6FABF0((char *)(a1 + 1692));
  if ( v7 && (v4 = *(int (__cdecl **)(int))(v7 + 4)) != 0 )
LABEL_5:
    v2 = v4(a1);
  else
LABEL_12:
    sub_6FD910(320, (int)aNoDelegateForT, (int)lpTempFileName);
  if ( *(_DWORD *)(a1 + 6684) )
  {
    remove(lpTempFileName);
    *(_DWORD *)(a1 + 6684) = 0;
    if ( !v2 )
      return 0;
    strcpy((char *)(v2 + 12), v30);
  }
  if ( !v2 )
    return 0;
  if ( *(_DWORD *)(v2 + 8) )
    remove(lpTempFileName);
  if ( sub_6E75D0(*(char **)(a1 + 6712), 0) )
  {
    v8 = 0;
    v9 = atoi(*(const char **)(a1 + 6712));
    v10 = *(const char **)(a1 + 6712);
    v26 = v9;
    v27 = (char *)v10;
    if ( *v10 )
    {
      while ( 1 )
      {
        v28 = 0;
        v11 = sscanf(v10, "%u%n-%u%n", &v26, &v28, &v29, &v28);
        if ( v11 )
        {
          v12 = v11 == 1;
          v13 = v26;
          if ( v12 )
            v29 = v26;
          v14 = v29 < v26 ? -1 : 1;
          if ( v26 != v29 + v14 )
            break;
        }
LABEL_38:
        v17 = v28;
        if ( v28 <= 1 )
          v17 = 1;
        v10 += v17;
        v27 = (char *)v10;
        if ( !*v10 )
          goto LABEL_41;
      }
LABEL_26:
      v15 = v2;
      while ( 1 )
      {
        if ( *(_DWORD *)(v15 + 3408) == v13 )
        {
          v16 = CloneImage(v15, *(void **)(v15 + 3372), *(void **)(v15 + 3376), 1);
          if ( !v16 )
          {
            sub_6FD910(300, (int)aMemoryAllocati, (int)lpTempFileName);
LABEL_36:
            v13 = v14 + v26;
            v26 = v13;
            if ( v13 == v29 + v14 )
            {
              v10 = v27;
              goto LABEL_38;
            }
            goto LABEL_26;
          }
          if ( v8 )
          {
            v8[1368] = v16;
            v16[1366] = v8;
            v8 = (void **)v8[1368];
          }
          else
          {
            v8 = v16;
          }
        }
        v15 = *(_DWORD *)(v15 + 5472);
        if ( !v15 )
          goto LABEL_36;
        v13 = v26;
      }
    }
LABEL_41:
    DestroyImageList((void *)v2);
    if ( !v8 )
    {
      sub_6FD910(310, (int)aSubimageSpecif, (int)lpTempFileName);
      return 0;
    }
    for ( i = (void **)v8[1366]; i; i = (void **)i[1366] )
      v8 = i;
    v2 = (int)v8;
    goto LABEL_64;
  }
  if ( *(_DWORD *)(a1 + 6696) && *(_DWORD *)(v2 + 5472) )
  {
    while ( 1 )
    {
      v19 = *(_DWORD *)(v2 + 3408);
      v20 = *(_DWORD *)(a1 + 6692);
      v21 = v19 >= v20 && v19 <= *(_DWORD *)(a1 + 6696) + v20 - 1;
      v22 = *(_DWORD *)(v2 + 5472);
      if ( !v22 )
        break;
      v2 = *(_DWORD *)(v2 + 5472);
      if ( !v21 )
        DestroyImage(*(void **)(v22 + 5464));
    }
    if ( *(_DWORD *)(v2 + 5464) )
    {
      v2 = *(_DWORD *)(v2 + 5464);
      if ( !v21 )
        DestroyImage(*(void **)(v2 + 5472));
    }
    else if ( !v21 )
    {
      DestroyImage((void *)v2);
      goto LABEL_59;
    }
    if ( !v2 )
    {
LABEL_59:
      sub_6FD910(310, (int)aSubimageSpecif, (int)lpTempFileName);
      return 0;
    }
    for ( j = *(_DWORD *)(v2 + 5464); j; j = *(_DWORD *)(j + 5464) )
      v2 = j;
  }
LABEL_64:
  if ( *(_DWORD *)(v2 + 4) )
    sub_6FD910(325, (int)aAnErrorHasOccu, v2 + 12);
  sub_6F9980(v2 + 5420);
  v24 = v2;
  do
  {
    sub_6F99B0((_DWORD *)(v24 + 5420));
    *(_DWORD *)(v24 + 5356) = 0;
    strcpy((char *)(v24 + 3680), lpTempFileName);
    if ( *(_DWORD *)(v2 + 8) )
      strcpy((char *)(v24 + 12), lpTempFileName);
    if ( !*(_DWORD *)(v24 + 5344) )
      *(_DWORD *)(v24 + 5344) = *(_DWORD *)(v24 + 3372);
    if ( !*(_DWORD *)(v24 + 5348) )
      *(_DWORD *)(v24 + 5348) = *(_DWORD *)(v24 + 3376);
    v24 = *(_DWORD *)(v24 + 5472);
  }
  while ( v24 );
  return (char *)v2;
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== ReadImages (0x6E8030) ===*/
char *__cdecl ReadImages(int a1)
{
  FILE *v1; // ebx
  signed int v3; // ebp
  char *v4; // esi
  char *v5; // edi
  int v6; // eax
  char v7; // bl
  const char *v8; // eax
  _DWORD *v9; // edi
  char *v10; // ebp
  int v11; // edx
  const char **v12; // ebx
  char *v13; // eax
  int v14; // edx
  char *i; // ecx
  FILE *v16; // [esp+10h] [ebp-8h]
  int v17; // [esp+10h] [ebp-8h]
  int v18; // [esp+14h] [ebp-4h] BYREF

  v1 = fopen((const char *)(a1 + 29), aR);
  v16 = v1;
  if ( v1 )
  {
    v3 = 1664;
    v4 = (char *)sub_6FAB50(0x680u);
    v5 = v4;
    if ( v4 )
    {
      while ( 1 )
      {
        v6 = fgetc(v1);
        v7 = v6;
        if ( v6 == -1 )
          break;
        if ( v5 - v4 + 1 >= v3 )
        {
          v3 *= 2;
          *v5 = 0;
          v8 = (const char *)sub_6FAB70(v4, v3);
          v4 = (char *)v8;
          if ( !v8 )
            break;
          v5 = (char *)&v8[strlen(v8)];
        }
        *v5 = v7;
        v1 = v16;
        ++v5;
      }
      v1 = v16;
    }
    fclose(v1);
    if ( v4 )
    {
      *v5 = 0;
      sub_6F6490(v4);
      v9 = sub_6F5EB0(v4, &v18);
      sub_6FAB60(v4);
      v10 = 0;
      v17 = 1;
      if ( v18 > 1 )
      {
        v11 = a1;
        v12 = (const char **)(v9 + 1);
        while ( 1 )
        {
          strcpy((char *)(v11 + 28), *v12);
          v13 = ReadImage(v11);
          if ( v13 )
          {
            if ( v10 )
            {
              v14 = *((_DWORD *)v10 + 1368);
              for ( i = v10; v14; v14 = *(_DWORD *)(v14 + 5472) )
                i = (char *)v14;
              *((_DWORD *)v13 + 1366) = i;
              *((_DWORD *)i + 1368) = v13;
            }
            else
            {
              v10 = v13;
            }
          }
          ++v12;
          if ( ++v17 >= v18 )
            break;
          v11 = a1;
        }
      }
      return v10;
    }
    else
    {
      sub_6FD910(300, (int)aUnableToReadIm, (int)aMemoryAllocati);
      return 0;
    }
  }
  else
  {
    sub_6FD910(330, (int)aUnableToReadIm, a1 + 28);
    return 0;
  }
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== SetImageInfo (0x6E91C0) ===*/
void __cdecl SetImageInfo(char *image_info, int a2)
{
  int v2; // ebx
  char *v3; // edx
  unsigned int v4; // ecx
  unsigned int v5; // esi
  unsigned int v6; // edi
  char *v7; // ebp
  const char **v8; // edi
  int *v9; // ebx
  int v10; // eax
  const char *v11; // ecx
  int *v12; // ebp
  int v13; // eax
  int v14; // edx
  int v15; // ecx
  int v16; // edx
  char v17; // cl
  char v18; // al
  char v19; // al
  char *v20; // ecx
  char *i; // esi
  char *v23; // edi
  int v24; // eax
  Image *Image; // ebp
  FILE *v26; // edi
  int v27; // esi
  int j; // eax
  int k; // ebp
  int (__cdecl *v30)(char *, int); // eax
  const char *v31; // [esp-10h] [ebp-D2Ch]
  char v32[4]; // [esp+0h] [ebp-D1Ch]
  char *v33; // [esp+4h] [ebp-D18h]
  unsigned int v34; // [esp+18h] [ebp-D04h]
  char data[8]; // [esp+1Ch] [ebp-D00h] BYREF
  char v36[1656]; // [esp+24h] [ebp-CF8h] BYREF
  char v37[1664]; // [esp+69Ch] [ebp-680h] BYREF

  v2 = (int)image_info;
  v3 = image_info + 28;
  data[0] = 0;
  v4 = strlen(image_info + 28);
  v5 = (unsigned int)&image_info[v4 + 27];
  if ( *(_BYTE *)v5 == 93 )
  {
    v6 = (unsigned int)&image_info[v4 + 26];
    if ( v6 > (unsigned int)v3 )
    {
      while ( *(_BYTE *)v6 != 91 )
      {
        if ( --v6 <= (unsigned int)v3 )
          goto LABEL_13;
      }
      if ( sub_6E7570((char *)(v6 + 1)) )
      {
        v34 = v5 - v6;
        v7 = (char *)sub_6FAB50(v5 - v6);
        if ( v7 )
        {
          strncpy(v7, (const char *)(v6 + 1), v34 - 1);
          v7[v5 - v6 - 1] = 0;
          *(_BYTE *)v6 = 0;
          v5 = v6;
          v8 = (const char **)(image_info + 6712);
          sub_6F5630((void **)image_info + 1678, v7);
          sub_6FAB60(v7);
          if ( sub_6E75D0(*((char **)image_info + 1678), 1) )
          {
            v9 = (int *)(image_info + 6692);
            v10 = atoi(*v8);
            v11 = *v8;
            *((_DWORD *)image_info + 1673) = v10;
            v12 = (int *)(image_info + 6696);
            v13 = atoi(v11);
            v31 = *v8;
            *((_DWORD *)image_info + 1674) = v13;
            sscanf(v31, "%u-%u", image_info + 6692, image_info + 6696);
            if ( *((_DWORD *)image_info + 1674) >= *((_DWORD *)image_info + 1673) )
            {
              sub_6FAB60((void *)*v8);
              *v8 = 0;
            }
            else
            {
              v14 = *v9;
              *v9 ^= *((_DWORD *)image_info + 1674);
              v15 = v14 ^ *v9;
              *v12 = v14;
              *v9 = v15;
            }
            v16 = *v9;
            v2 = (int)image_info;
            *v12 += 1 - v16;
          }
        }
      }
    }
  }
LABEL_13:
  if ( *(_BYTE *)v5 != 46 )
  {
    do
    {
      if ( v5 <= v2 + 29 )
        break;
      v17 = *(_BYTE *)--v5;
    }
    while ( v17 != 46 );
  }
  if ( sub_6F5950((char *)v5, (char *)&off_8DABDC) && sub_6F5950((char *)v5, aZ) && sub_6F5950((char *)v5, aBz2) )
  {
LABEL_21:
    if ( *(_BYTE *)v5 != 46 )
      goto LABEL_32;
  }
  else
  {
    while ( 1 )
    {
      v18 = *(_BYTE *)--v5;
      if ( v18 == 46 )
        break;
      if ( v5 <= v2 + 29 )
        goto LABEL_21;
    }
  }
  if ( (int)strlen((const char *)v5) < 1664 )
  {
    strcpy(data, (const char *)(v5 + 1));
    v19 = data[0];
    v20 = data;
    if ( data[0] )
    {
      while ( v19 != 46 )
      {
        v19 = *++v20;
        if ( !v19 )
          goto LABEL_28;
      }
      *v20 = 0;
    }
LABEL_28:
    sub_6F5A10(data);
    if ( (strncmp((const char *)(v2 + 1692), off_8DABCC, 3u) || sub_6F5950(data, (char *)&off_87B9AC))
      && sub_6F5950(data, (char *)&off_8DABC8) )
    {
      strcpy((char *)(v2 + 1692), data);
    }
  }
LABEL_32:
  v34 = 0;
  for ( i = (char *)(v2 + 28); (int)cbMultiByte <= 1 ? *((_WORD *)off_8CE5D0 + *i) & 0x107 : _isctype(*i, 263); ++i )
    ;
  if ( *i == 58 )
  {
    v23 = &i[-v2];
    if ( (int)&i[-v2 - 28] < 1664 )
    {
      strncpy(data, (const char *)(v2 + 28), (size_t)(v23 - 28));
      v32[(_DWORD)v23] = 0;
      sub_6F5A10(data);
      if ( !ImageFormatConflict(data) )
      {
        v33 = aImplicit;
        strcpy((char *)(v2 + 28), i + 1);
        if ( sub_6F5950(data, v33) )
        {
          v33 = (char *)&off_8DABC8;
          strcpy((char *)(v2 + 1692), data);
          if ( sub_6F5950(data, v33) )
            v34 = 1;
          else
            *(_DWORD *)(v2 + 6684) = 1;
        }
      }
    }
  }
  if ( a2 )
  {
    sub_6F56D0(v37, (char *)(v2 + 28), 0);
    if ( sub_6F5950(v37, (char *)(v2 + 28)) && !strchr(v37, 37) )
      *(_DWORD *)(v2 + 6688) = 0;
    v24 = sub_6FABF0(data);
    if ( v24 )
      *(_DWORD *)(v2 + 6688) &= *(_DWORD *)(v24 + 16);
  }
  else if ( !v34 )
  {
    Image = AllocateImage((void *)v2);
    if ( Image )
    {
      strcpy((char *)Image + 12, (const char *)(v2 + 28));
      if ( !sub_6F9B90(v2, (int)Image, off_8DA81C) )
      {
        DestroyImage(Image);
        return;
      }
      if ( *((_DWORD *)Image + 1356) || !*(_DWORD *)Image )
      {
        ReadBlob(Image, 0x67Fu, data);
      }
      else
      {
        *(_DWORD *)(v2 + 24) = 0;
        sub_6F6590((LPSTR)Image + 12);
        *(_DWORD *)v32 = Image + 12;
        *(_DWORD *)(v2 + 6684) = 1;
        sub_6F56D0((char *)(v2 + 28), "%.1024s", *(const char **)v32);
        v26 = fopen((const char *)Image + 12, off_8DA82C);
        if ( !v26 )
        {
          sub_6FD910(330, (int)aUnableToWriteF, (int)(Image + 12));
          return;
        }
        v27 = 0;
        for ( j = fgetc(*((FILE **)Image + 419)); j != -1; j = fgetc(*((FILE **)Image + 419)) )
        {
          if ( v27 < 1664 )
            data[v27++] = j;
          fputc(j, v26);
        }
        fclose(v26);
      }
      DestroyImage(Image);
      v36[1655] = 0;
      if ( !strncmp(data, aBegmf, 3u) )
        *(_DWORD *)(v2 + 1692) = off_8DAB90;
      if ( !strncmp(data, aDigraph, 7u) )
        *(_DWORD *)(v2 + 1692) = dword_8DAB84;
      if ( !strncmp(data, aFig, 4u) )
        *(_DWORD *)(v2 + 1692) = dword_8DAB78;
      if ( !strncmp(data, aUsrLocalBinGnu, 0x18u) )
        strcpy((char *)(v2 + 1692), "GPLT");
      if ( !strncmp(data, aIn, 3u) )
        strcpy((char *)(v2 + 1692), "HPGL");
      if ( !strncmp(data, aE, 4u) )
        strcpy((char *)(v2 + 1692), "HPGL");
      if ( !strncmp(v36, aIlbm, 2u) )
        strcpy((char *)(v2 + 1692), "ILBM");
      if ( !data[0] && !data[1] && data[2] == 1 && data[3] == -77 )
        *(_DWORD *)(v2 + 1692) = dword_8DAB34;
      if ( !strncmp(data, aRadiance, 0xAu) )
        *(_DWORD *)(v2 + 1692) = dword_8DAB24;
      if ( !strncmp(data, aGimpXcfFile, 0xDu) )
        *(_DWORD *)(v2 + 1692) = dword_8DAB10;
      for ( k = sub_6FABF0(0); k; k = *(_DWORD *)(k + 40) )
      {
        v30 = *(int (__cdecl **)(char *, int))(k + 12);
        if ( v30 && v30(data, 1664) )
          strcpy((char *)(v2 + 1692), *(const char **)k);
      }
    }
  }
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== TransformRGBImage (0x6E99D0) ===*/
void __cdecl TransformRGBImage(_DWORD *a1, int a2)
{
  int v2; // edi
  unsigned __int8 *v3; // eax
  int v4; // edx
  int v5; // ecx
  int v6; // ebx
  double *v7; // esi
  char *v8; // edi
  _DWORD *v9; // eax
  char *v10; // ebx
  double v11; // st7
  double *v12; // eax
  double v13; // st6
  double v14; // st5
  bool v15; // cc
  char *v16; // eax
  double v17; // st7
  double v18; // st6
  double *v19; // eax
  double v20; // st6
  char *v21; // eax
  double v22; // st6
  char *v23; // eax
  double v24; // st7
  double v25; // st6
  double *v26; // eax
  double v27; // st6
  char *v28; // eax
  double v29; // st6
  char *v30; // eax
  double v31; // st6
  _DWORD *v32; // eax
  int v33; // ecx
  int v34; // edx
  double v35; // st6
  double v36; // st5
  char v38; // c0
  double v39; // st5
  char v41; // c0
  double v42; // st5
  int v43; // eax
  int v44; // edx
  unsigned __int8 *v45; // eax
  int v46; // edx
  int v47; // ecx
  double v48; // st6
  double v49; // st5
  char v51; // c0
  double v52; // st5
  char v54; // c0
  double v55; // st5
  int v56; // [esp+Ch] [ebp-3Ch]
  int v57; // [esp+Ch] [ebp-3Ch]
  int v58; // [esp+Ch] [ebp-3Ch]
  int v59; // [esp+Ch] [ebp-3Ch]
  int v60; // [esp+Ch] [ebp-3Ch]
  int v61; // [esp+Ch] [ebp-3Ch]
  int v62; // [esp+Ch] [ebp-3Ch]
  int v63; // [esp+Ch] [ebp-3Ch]
  int v64; // [esp+Ch] [ebp-3Ch]
  int v65; // [esp+10h] [ebp-38h]
  int v66; // [esp+10h] [ebp-38h]
  int v67; // [esp+14h] [ebp-34h]
  unsigned __int8 *v68; // [esp+14h] [ebp-34h]
  int j; // [esp+18h] [ebp-30h]
  int i; // [esp+18h] [ebp-30h]
  double *v71; // [esp+1Ch] [ebp-2Ch]
  double *v72; // [esp+1Ch] [ebp-2Ch]
  double *v73; // [esp+1Ch] [ebp-2Ch]
  double *v74; // [esp+1Ch] [ebp-2Ch]
  double *v75; // [esp+1Ch] [ebp-2Ch]
  double *v76; // [esp+1Ch] [ebp-2Ch]
  double *v77; // [esp+1Ch] [ebp-2Ch]
  double *v78; // [esp+1Ch] [ebp-2Ch]
  int v79; // [esp+20h] [ebp-28h]
  double *v80; // [esp+20h] [ebp-28h]
  double *v81; // [esp+20h] [ebp-28h]
  double *v82; // [esp+20h] [ebp-28h]
  double *v83; // [esp+20h] [ebp-28h]
  double *v84; // [esp+20h] [ebp-28h]
  double *v85; // [esp+20h] [ebp-28h]
  double *v86; // [esp+20h] [ebp-28h]
  double *v87; // [esp+20h] [ebp-28h]
  int v88; // [esp+20h] [ebp-28h]
  double v89; // [esp+20h] [ebp-28h]
  int v90; // [esp+20h] [ebp-28h]
  double v91; // [esp+20h] [ebp-28h]
  char *v92; // [esp+2Ch] [ebp-1Ch]
  double *v93; // [esp+2Ch] [ebp-1Ch]
  double *v94; // [esp+2Ch] [ebp-1Ch]
  double v95; // [esp+30h] [ebp-18h]
  double v96; // [esp+30h] [ebp-18h]
  double v97; // [esp+30h] [ebp-18h]
  double v98; // [esp+30h] [ebp-18h]
  double v99; // [esp+30h] [ebp-18h]
  double v100; // [esp+30h] [ebp-18h]
  double v101; // [esp+30h] [ebp-18h]
  double v102; // [esp+30h] [ebp-18h]
  double v103; // [esp+30h] [ebp-18h]
  double v104; // [esp+30h] [ebp-18h]
  double v105; // [esp+30h] [ebp-18h]
  double v106; // [esp+38h] [ebp-10h]
  double v107; // [esp+40h] [ebp-8h]

  if ( a1[857] != 11 )
  {
    if ( a2 == 1 )
      return;
    goto LABEL_21;
  }
  if ( a2 != 1 )
  {
LABEL_21:
    if ( a2 == 2 || a2 == 3 )
      return;
    v7 = (double *)sub_6FAB50(0x1800u);
    v8 = (char *)sub_6FAB50(0x1800u);
    v9 = sub_6FAB50(0x1800u);
    v10 = (char *)v9;
    if ( !v7 || !v8 || !v9 )
    {
      sub_6FD910(300, (int)aUnableToTransf, (int)aMemoryAllocati);
      return;
    }
    v11 = dbl_732220;
    v106 = dbl_732220;
    switch ( a2 )
    {
      case 4:
        v12 = (double *)(v8 + 2048);
        v80 = (double *)(v10 + 4096);
        v56 = 0;
        v71 = v7;
        v92 = v8 + 2048;
        do
        {
          v13 = (double)v56;
          *v71 = v13;
          v95 = v13 + v13 - dbl_732220;
          *(v12 - 256) = v95 * dbl_72A480;
          v107 = v95 * dbl_7324E0;
          *(v80 - 512) = v107;
          *(double *)((char *)v12 + (char *)v7 - v8) = v13;
          *(_DWORD *)v12 = 0;
          v14 = v95 * dbl_7324D8;
          *((_DWORD *)v12 + 1) = 0;
          *(double *)((char *)v12 + v10 - v8) = v14;
          *(double *)((char *)v80++ + (char *)v7 - v10) = v13;
          v12[256] = v95 * dbl_732398;
          *(v80 - 1) = v107;
          ++v71;
          v12 = (double *)(v92 + 8);
          v15 = ++v56 <= 255;
          v92 += 8;
        }
        while ( v15 );
        break;
      case 5:
        v19 = (double *)(v8 + 2048);
        v73 = (double *)(v10 + 4096);
        v58 = 0;
        v82 = v7;
        v93 = (double *)(v8 + 2048);
        do
        {
          v20 = (double)v58;
          *v82 = v20 * dbl_732490;
          *(v19 - 256) = v20 * dbl_732488;
          *(v73 - 512) = v20 * dbl_732480;
          *(double *)((char *)v19 + (char *)v7 - v8) = v20 * dbl_732478;
          *v19 = v20 * dbl_732470;
          *(double *)((char *)v19 + v10 - v8) = v20 * dbl_732468;
          *(double *)((char *)v73 + (char *)v7 - v10) = v20 * dbl_732460;
          v19 = v93 + 1;
          v93 = v19;
          v19[255] = v20 * dbl_732458;
          *v73 = v20 * dbl_732450;
          ++v58;
          ++v82;
          ++v73;
        }
        while ( v58 <= 255 );
        break;
      case 6:
        v59 = 0;
        v74 = (double *)(v9 + 1024);
        v83 = v7;
        v21 = v8 + 2048;
        do
        {
          v22 = (double)v59;
          v21 += 8;
          *v83 = v22;
          *((_DWORD *)v21 - 514) = 0;
          *((_DWORD *)v21 - 513) = 0;
          v98 = v22 + v22 - dbl_732220;
          *(v74 - 512) = v98 * dbl_732448;
          *(double *)&v21[(char *)v7 - v8 - 8] = v22;
          *((double *)v21 - 1) = v98 * dbl_732440;
          *(double *)&v21[v10 - v8 - 8] = v98 * dbl_732438;
          *(double *)((char *)v74 + (char *)v7 - v10) = v22;
          *((double *)v21 + 255) = v98 * dbl_732430;
          *(_DWORD *)v74 = 0;
          *((_DWORD *)v74 + 1) = 0;
          ++v59;
          ++v83;
          ++v74;
        }
        while ( v59 <= 255 );
        break;
      case 7:
        v60 = 0;
        v75 = (double *)(v9 + 1024);
        v84 = v7;
        v23 = v8 + 2048;
        do
        {
          v23 += 8;
          v99 = (double)v60;
          v24 = v99 * dbl_732428;
          *v84 = v99 * dbl_732428;
          v25 = v99 - dbl_7324C8;
          *((_DWORD *)v23 - 514) = 0;
          *((_DWORD *)v23 - 513) = 0;
          *(v75 - 512) = v25 * dbl_732420;
          *(double *)&v23[(char *)v7 - v8 - 8] = v24;
          v100 = v99 - dbl_7324B8;
          *((double *)v23 - 1) = v100 * dbl_732418;
          *(double *)&v23[v10 - v8 - 8] = v25 * dbl_732410;
          *(double *)((char *)v75 + (char *)v7 - v10) = v24;
          *((double *)v23 + 255) = v100 * dbl_732408;
          *(_DWORD *)v75 = 0;
          *((_DWORD *)v75 + 1) = 0;
          ++v60;
          ++v84;
          ++v75;
        }
        while ( v60 <= 255 );
        v11 = dbl_732498;
        v106 = dbl_732498;
        break;
      case 8:
        v26 = (double *)(v8 + 2048);
        v76 = (double *)(v10 + 4096);
        v61 = 0;
        v85 = v7;
        v94 = (double *)(v8 + 2048);
        do
        {
          v27 = (double)v61;
          *v85 = v27;
          v101 = v27 + v27 - dbl_732220;
          *(v26 - 256) = v101 * dbl_732400;
          *(v76 - 512) = v101 * dbl_7323F8;
          *(double *)((char *)v26 + (char *)v7 - v8) = v27;
          *v26 = v101 * dbl_7323F0;
          *(double *)((char *)v26 + v10 - v8) = v101 * dbl_7323E8;
          *(double *)((char *)v76 + (char *)v7 - v10) = v27;
          v26 = v94 + 1;
          v94 = v26;
          v26[255] = v101 * dbl_7323E0;
          *v76 = v101 * dbl_7323D8;
          ++v61;
          ++v85;
          ++v76;
        }
        while ( v61 <= 255 );
        break;
      case 9:
        v62 = 0;
        v77 = (double *)(v9 + 1024);
        v86 = v7;
        v28 = v8 + 2048;
        do
        {
          v29 = (double)v62;
          v28 += 8;
          *v86 = v29;
          *((_DWORD *)v28 - 514) = 0;
          *((_DWORD *)v28 - 513) = 0;
          v102 = v29 + v29 - dbl_732220;
          *(v77 - 512) = v102 * dbl_732448;
          *(double *)&v28[(char *)v7 - v8 - 8] = v29;
          *((double *)v28 - 1) = v102 * dbl_732440;
          *(double *)&v28[v10 - v8 - 8] = v102 * dbl_7323D0;
          *(double *)((char *)v77 + (char *)v7 - v10) = v29;
          *((double *)v28 + 255) = v102 * dbl_732430;
          *(_DWORD *)v77 = 0;
          *((_DWORD *)v77 + 1) = 0;
          ++v62;
          ++v86;
          ++v77;
        }
        while ( v62 <= 255 );
        break;
      case 12:
        v72 = (double *)(v9 + 1024);
        v57 = 0;
        v67 = (char *)v9 - v8;
        v81 = v7;
        v16 = v8 + 2048;
        do
        {
          v16 += 8;
          v96 = (double)v57;
          v17 = v96 * dbl_7324D0;
          *v81 = v96 * dbl_7324D0;
          v18 = v96 - dbl_7324C8;
          *((_DWORD *)v16 - 514) = 0;
          *((_DWORD *)v16 - 513) = 0;
          *(v72 - 512) = v18 * dbl_7324C0;
          *(double *)&v16[(char *)v7 - v8 - 8] = v17;
          v97 = v96 - dbl_7324B8;
          *((double *)v16 - 1) = v97 * dbl_7324B0;
          *(double *)&v16[v67 - 8] = v18 * dbl_7324A8;
          *(double *)((char *)v72 + (char *)v7 - v10) = v17;
          *((double *)v16 + 255) = v97 * dbl_7324A0;
          *(_DWORD *)v72 = 0;
          *((_DWORD *)v72 + 1) = 0;
          ++v57;
          ++v81;
          ++v72;
        }
        while ( v57 <= 255 );
        v11 = dbl_732498;
        v106 = dbl_732498;
        break;
      default:
        v63 = 0;
        v78 = (double *)(v9 + 1024);
        v87 = v7;
        v30 = v8 + 2048;
        do
        {
          v31 = (double)v63;
          v30 += 8;
          *v87 = v31;
          *((_DWORD *)v30 - 514) = 0;
          *((_DWORD *)v30 - 513) = 0;
          v103 = v31 + v31 - dbl_732220;
          *(v78 - 512) = v103 * dbl_7323C8;
          *(double *)&v30[(char *)v7 - v8 - 8] = v31;
          *((double *)v30 - 1) = v103 * dbl_7323C0;
          *(double *)&v30[v10 - v8 - 8] = v103 * dbl_7323B8;
          *(double *)((char *)v78 + (char *)v7 - v10) = v31;
          *((double *)v30 + 255) = v103 * dbl_7323B0;
          *(_DWORD *)v78 = 0;
          *((_DWORD *)v78 + 1) = 0;
          ++v63;
          ++v87;
          ++v78;
        }
        while ( v63 <= 255 );
        break;
    }
    v32 = a1;
    if ( a1[840] != 2 )
    {
      for ( i = 0; i < a1[844]; ++i )
      {
        v68 = (unsigned __int8 *)sub_6F6DD0((int)a1, 0, i, a1[843], 1);
        if ( !v68 )
          break;
        v66 = 0;
        if ( (int)a1[843] > 0 )
        {
          while ( 1 )
          {
            v33 = v68[1];
            v88 = v68[2];
            v34 = *v68;
            v35 = *(double *)&v10[8 * v34] + *(double *)&v8[8 * v33] + v7[v88];
            if ( v35 >= ZERO_2 )
            {
              if ( v35 > v106 )
                v35 = v106;
            }
            else
            {
              v35 = ZERO_2;
            }
            v36 = *(double *)&v10[8 * v34 + 2048] + *(double *)&v8[8 * v33 + 2048] + v7[v88 + 256];
            v104 = v36;
            if ( v38 )
            {
              v104 = 0.0;
            }
            else if ( v36 > v106 )
            {
              v104 = v106;
            }
            v39 = *(double *)&v10[8 * v34 + 4096] + *(double *)&v8[8 * v33 + 4096] + v7[v88 + 512];
            v89 = v39;
            if ( v41 )
            {
              v89 = 0.0;
            }
            else if ( v39 > v106 )
            {
              v89 = v106;
            }
            if ( a2 == 12 )
              break;
            if ( a2 == 7 )
            {
              v35 = (double)(unsigned __int8)byte_732094[(__int64)v35];
              v104 = (double)(unsigned __int8)byte_732094[(__int64)v104];
              v42 = (double)(unsigned __int8)byte_732094[(__int64)v89];
              goto LABEL_70;
            }
LABEL_71:
            v68[2] = (__int64)v35;
            v68[1] = (__int64)v104;
            *v68 = (__int64)v89;
            v68 += 4;
            if ( ++v66 >= a1[843] )
              goto LABEL_72;
          }
          v35 = (double)(unsigned __int8)byte_731F34[(__int64)v35];
          v104 = (double)(unsigned __int8)byte_731F34[(__int64)v104];
          v42 = (double)(unsigned __int8)byte_731F34[(__int64)v89];
LABEL_70:
          v89 = v42;
          goto LABEL_71;
        }
LABEL_72:
        if ( !imageSyncPixelRegion((int)a1) )
          break;
        if ( (~(a1[844] - i - 1) & (a1[844] - i - 2)) + 1 == a1[844] - i - 1 )
          seekdir();
      }
LABEL_99:
      sub_6FAB60(v10);
      sub_6FAB60(v8);
      sub_6FAB60(v7);
      return;
    }
    v64 = 0;
    if ( (int)a1[856] <= 0 )
    {
LABEL_98:
      sub_6E9920(a1);
      goto LABEL_99;
    }
    while ( 1 )
    {
      v43 = v32[855];
      v44 = *(unsigned __int8 *)(v43 + 4 * v64 + 1);
      v45 = (unsigned __int8 *)(v43 + 4 * v64);
      v90 = v44;
      v46 = v45[2];
      v47 = *v45;
      v48 = v7[v46] + *(double *)&v8[8 * v90] + *(double *)&v10[8 * v47];
      if ( v48 >= ZERO_2 )
      {
        if ( v48 > v11 )
          v48 = v11;
      }
      else
      {
        v48 = ZERO_2;
      }
      v49 = v7[v46 + 256] + *(double *)&v8[8 * v90 + 2048] + *(double *)&v10[8 * v47 + 2048];
      v105 = v49;
      if ( v51 )
      {
        v105 = 0.0;
      }
      else if ( v49 > v11 )
      {
        v105 = v11;
      }
      v52 = v7[v46 + 512] + *(double *)&v8[8 * v90 + 4096] + *(double *)&v10[8 * v47 + 4096];
      v91 = v52;
      if ( v54 )
      {
        v91 = 0.0;
      }
      else if ( v52 > v11 )
      {
        v91 = v11;
      }
      if ( a2 == 12 )
      {
        v48 = (double)(unsigned __int8)byte_731F34[(__int64)v48];
        v105 = (double)(unsigned __int8)byte_731F34[(__int64)v105];
        v55 = (double)(unsigned __int8)byte_731F34[(__int64)v91];
      }
      else
      {
        if ( a2 != 7 )
          goto LABEL_97;
        v48 = (double)(unsigned __int8)byte_732094[(__int64)v48];
        v105 = (double)(unsigned __int8)byte_732094[(__int64)v105];
        v55 = (double)(unsigned __int8)byte_732094[(__int64)v91];
      }
      v91 = v55;
LABEL_97:
      v45[2] = (__int64)v48;
      *(_BYTE *)(a1[855] + 4 * v64 + 1) = (__int64)v105;
      *(_BYTE *)(a1[855] + 4 * v64++) = (__int64)v91;
      if ( v64 >= a1[856] )
        goto LABEL_98;
      v32 = a1;
    }
  }
  a1[857] = 1;
  v2 = 0;
  for ( j = 0; v2 < a1[844]; j = ++v2 )
  {
    v3 = (unsigned __int8 *)sub_6F6DD0((int)a1, 0, v2, a1[843], 1);
    if ( !v3 )
      break;
    v65 = 0;
    if ( (int)a1[843] > 0 )
    {
      do
      {
        v4 = v3[2];
        v79 = *v3;
        v5 = v3[3];
        v6 = v3[1];
        if ( (unsigned int)(v5 + v4) <= 0xFF )
          v3[2] = -1 - v5 - v4;
        else
          v3[2] = 0;
        if ( (unsigned int)(v5 + v6) <= 0xFF )
          v3[1] = -1 - v6 - v5;
        else
          v3[1] = 0;
        if ( (unsigned int)(v5 + v79) <= 0xFF )
          *v3 = -1 - v79 - v5;
        else
          *v3 = 0;
        v3[3] = 0;
        v3 += 4;
        ++v65;
      }
      while ( v65 < a1[843] );
      v2 = j;
    }
    if ( !imageSyncPixelRegion((int)a1) )
      break;
  }
}
#endif

#if 0  // Decompiled pseudocode reference
/*=== WriteImage (0x6EA820) ===*/
int __cdecl WriteImage(const char **src, int a2)
{
  char *v2; // ebx
  char *v3; // esi
  signed int v4; // esi
  int v6; // eax
  int (__cdecl *v7)(char *, int); // eax
  int v8; // eax
  signed int v9; // edi
  char *lpTempFileName; // [esp+10h] [ebp-D1Ch]
  char *v11; // [esp+14h] [ebp-D18h]
  int v12; // [esp+18h] [ebp-D14h]
  char v13[3332]; // [esp+1Ch] [ebp-D10h] BYREF
  int v14; // [esp+D20h] [ebp-Ch]

  v2 = (char *)CloneImageInfo(src);
  lpTempFileName = (char *)(a2 + 12);
  strcpy(v2 + 28, (const char *)(a2 + 12));
  v11 = (char *)(a2 + 1688);
  strcpy(v2 + 1692, (const char *)(a2 + 1688));
  SetImageInfo(v2, 1);
  strcpy((char *)(a2 + 12), v2 + 28);
  if ( *(_DWORD *)(a2 + 5472) && !*((_DWORD *)v2 + 1672)
    || *(_DWORD *)(a2 + 5464)
    || sub_6E7640(a2)
    || !sub_6F58F0((char *)(a2 + 3680)) )
  {
    v3 = (char *)(a2 + 1688);
  }
  else
  {
    v3 = (char *)(a2 + 1688);
    if ( sub_6FC740(v11, v2 + 1692, v13) && !v14 )
    {
      strcpy(lpTempFileName, (const char *)(a2 + 3680));
      v4 = sub_6FCDC0((int)v2, a2, v11, v2 + 1692);
      DestroyImageInfo(v2);
      return v4;
    }
  }
  v12 = 0;
  v6 = sub_6FABF0(v2 + 1692);
  if ( v6 )
  {
    v7 = *(int (__cdecl **)(char *, int))(v6 + 8);
    if ( v7 )
      goto LABEL_12;
  }
  if ( sub_6FC740(0, v2 + 1692, v13) )
  {
    sub_6F6590(lpTempFileName);
    v9 = sub_6FCDC0((int)v2, a2, 0, v2 + 1692);
    remove(lpTempFileName);
    DestroyImageInfo(v2);
    return v9;
  }
  sub_6FD910(320, (int)aNoEncodeDelega, (int)(v2 + 1692));
  v8 = sub_6FABF0(v3);
  if ( v8 && (v7 = *(int (__cdecl **)(char *, int))(v8 + 8)) != 0 )
LABEL_12:
    v12 = v7(v2, a2);
  else
    sub_6FD910(320, (int)aNoEncodeDelega, (int)v3);
  if ( *(_DWORD *)(a2 + 4) )
  {
    sub_6FD910(325, (int)aAnErrorHasOccu_0, (int)lpTempFileName);
    DestroyImageInfo(v2);
    return 0;
  }
  else
  {
    strcpy(v11, v2 + 1692);
    DestroyImageInfo(v2);
    return v12;
  }
}
#endif

bmpStdString *GeometryInfoToString(bmpGeometryInfo *geometryInfo, bmpStdString *outString)
{
    size_t length;
    ubyte *allocation;
    char text[96];
    char *cursor = text;

    if (geometryInfo->width != 0)
    {
        cursor += sprintf(cursor, "%d", geometryInfo->width);
    }
    if (geometryInfo->width != 0 && geometryInfo->height != 0)
    {
        cursor += sprintf(cursor, "x%d", geometryInfo->height);
    }
    if (geometryInfo->xOffset != 0 || geometryInfo->yOffset != 0)
    {
        cursor += sprintf(
            cursor,
            "%c%d%c%d",
            geometryInfo->xNegative ? '-' : '+',
            geometryInfo->xOffset,
            geometryInfo->yNegative ? '-' : '+',
            geometryInfo->yOffset);
    }
    if (geometryInfo->percent)
    {
        *cursor++ = '%';
    }
    if (geometryInfo->aspect)
    {
        *cursor++ = '!';
    }
    if (geometryInfo->less)
    {
        *cursor++ = '>';
    }
    if (geometryInfo->greater)
    {
        *cursor++ = '<';
    }
    *cursor = '\0';

    memset(outString, 0, sizeof(*outString));
    length = strlen(text);
    if (length == 0)
    {
        return outString;
    }

    allocation = malloc(length + 2);
    if (allocation == NULL)
    {
        return outString;
    }

    allocation[0] = 0;
    memcpy(allocation + 1, text, length + 1);
    outString->data = (char *)(allocation + 1);
    outString->length = (udword)length;
    outString->capacity = (udword)length;
    return outString;
}
