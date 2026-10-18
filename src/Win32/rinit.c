/*=============================================================================
    Name    : rinit.c
    Purpose : Display device/mode enumeration for modern Windows
              Based on CET's approach: 32-bit modes only, min 640x480

    Created for Cataclysm source reconstruction
=============================================================================*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "memory.h"
#include "crc32.h"
#include "rinit.h"
#include "devstats.h"

extern udword loadedDevcaps;
extern udword loadedDevcaps2;
extern udword opDeviceCRC;
extern udword strCurLanguage;

static rdevice *deviceList = NULL;

int rinFreeDevices(void);

static void rinApplyPersistedDeviceCaps(rdevice *dev)
{
    udword deviceCRC;

    if (loadedDevcaps == 0xFFFFFFFF || loadedDevcaps2 == 0xFFFFFFFF)
    {
        return;
    }

    dev->devcaps = loadedDevcaps;
    dev->devcaps2 = loadedDevcaps2;

    deviceCRC = crc32Compute((ubyte *)dev, sizeof(rdevice));
    if (opDeviceCRC != 0 && opDeviceCRC != deviceCRC)
    {
        dev->devcaps = 0;
        dev->devcaps2 = 0;
    }
}

static void rinAddMode(rdevice *dev, int width, int height, int depth)
{
    rmode *mode = (rmode *)malloc(sizeof(rmode));
    mode->width = width;
    mode->height = height;
    mode->depth = depth;
    mode->next = NULL;

    if (dev->modes == NULL)
    {
        dev->modes = mode;
    }
    else
    {
        rmode *tail = dev->modes;
        while (tail->next != NULL) tail = tail->next;
        tail->next = mode;
    }
}

static void rinCopyModes(rdevice *devOut, rdevice *devIn)
{
    rmode *mode;

    if (devIn == NULL)
    {
        devOut->modes = NULL;
        return;
    }

    for (mode = devIn->modes; mode != NULL; mode = mode->next)
    {
        rinAddMode(devOut, mode->width, mode->height, mode->depth);
    }
}

static void rinNameDefaultOpenGL(rdevice *dev)
{
    if (strCurLanguage == 1)
    {
        strncpy(dev->name, "OpenGL Standard", 63);
    }
    else if (strCurLanguage == 2)
    {
        strncpy(dev->name, "Standard-OpenGL", 63);
    }
    else if (strCurLanguage == 3)
    {
        strncpy(dev->name, "OpenGL predeterminado", 63);
    }
    else if (strCurLanguage == 4)
    {
        strncpy(dev->name, "OpenGL di Default", 63);
    }
    else
    {
        strncpy(dev->name, "Default OpenGL", 63);
    }
    dev->name[63] = '\0';
}

static rdevice *rinAddDevice(char *name, char *data, int type)
{
    rdevice *dev = (rdevice *)malloc(sizeof(rdevice));
    memset(dev, 0, sizeof(rdevice));
    dev->type = type;
    strncpy(dev->name, name, 63);
    strncpy(dev->data, data, 63);
    dev->modes = NULL;
    dev->next = NULL;

    if (deviceList == NULL)
    {
        deviceList = dev;
    }
    else
    {
        rdevice *tail = deviceList;
        while (tail->next != NULL) tail = tail->next;
        tail->next = dev;
    }
    return dev;
}

static int rinCountDevices(void)
{
    int count = 0;
    rdevice *dev = deviceList;

    while (dev != NULL)
    {
        count++;
        dev = dev->next;
    }

    return count;
}

int rinEnumerateDevices(void)
{
    DEVMODE dm;
    rdevice *dev;
    rdevice *primaryD3D;

    rinFreeDevices();

    // Binary options code compares the D3D device's data string against the
    // currently selected D3D payload. The default renderer startup path uses an
    // empty payload, so our synthetic primary D3D entry must do the same.
    dev = rinAddDevice("Direct3D", "", RIN_TYPE_DIRECT3D);
    rinApplyPersistedDeviceCaps(dev);
    primaryD3D = dev;

    // Enumerate display modes via Win32 API
    memset(&dm, 0, sizeof(dm));
    dm.dmSize = sizeof(dm);

    for (int i = 0; EnumDisplaySettings(NULL, i, &dm); i++)
    {
        // Only accept 32-bit modes at 640x480 or higher
        if (dm.dmBitsPerPel == 32 &&
            dm.dmPelsWidth >= 640 &&
            dm.dmPelsHeight >= 480)
        {
            // Check for duplicates
            int duplicate = 0;
            rmode *m = dev->modes;
            while (m != NULL)
            {
                if (m->width == (int)dm.dmPelsWidth &&
                    m->height == (int)dm.dmPelsHeight &&
                    m->depth == (int)dm.dmBitsPerPel)
                {
                    duplicate = 1;
                    break;
                }
                m = m->next;
            }
            if (!duplicate)
            {
                rinAddMode(dev, dm.dmPelsWidth, dm.dmPelsHeight, dm.dmBitsPerPel);
            }
        }
    }

    // HW1 always exposed a default OpenGL renderer backed by the primary
    // display's modes. Cataclysm's options UI still expects that synthetic
    // OpenGL entry to exist.
    dev = rinAddDevice("", "", RIN_TYPE_OPENGL);
    dev->devcaps = primaryD3D->devcaps;
    dev->devcaps2 = primaryD3D->devcaps2;
    rinNameDefaultOpenGL(dev);
    rinCopyModes(dev, primaryD3D);
    if (dev->modes == NULL)
    {
        rinAddMode(dev, 640, 480, 32);
        rinAddMode(dev, 800, 600, 32);
        rinAddMode(dev, 1024, 768, 32);
    }

    return 1;
}

int rinFreeDevices(void)
{
    rdevice *dev = deviceList;
    while (dev != NULL)
    {
        rdevice *next = dev->next;
        rmode *mode = dev->modes;
        while (mode != NULL)
        {
            rmode *nextMode = mode->next;
            free(mode);
            mode = nextMode;
        }
        free(dev);
        dev = next;
    }
    deviceList = NULL;
    return 1;
}

rdevice *rinGetDeviceList(void)
{
    return deviceList;
}

unsigned int rinDeviceCRC(void)
{
    unsigned int crc;
    int deviceCount;
    size_t bufferSize;
    rdevice *copy;
    rdevice *dev;
    rdevice *nextCopy;

    deviceCount = rinCountDevices();
    if (deviceCount == 0 || deviceList == NULL)
    {
        return 0;
    }

    bufferSize = (size_t)deviceCount * sizeof(rdevice);
    copy = (rdevice *)malloc(bufferSize);
    if (copy == NULL)
    {
        return 0;
    }

    memset(copy, 0, bufferSize);

    dev = deviceList;
    nextCopy = copy;
    while (dev != NULL)
    {
        memcpy(nextCopy, dev, sizeof(rdevice));
        nextCopy->modes = NULL;
        nextCopy->next = NULL;
        nextCopy++;
        dev = dev->next;
    }

    crc = crc32Compute((ubyte *)copy, (udword)bufferSize);
    free(copy);
    return crc;
}

#if 0  // Cat2: Old DirectDraw/D3D5 device list management; live rinit.c replaces this with D3D9 enumeration (0x57DFF0)
/*=== rinAddDeviceToList (0X57DFF0) ===*/
_DWORD *__cdecl rinAddDeviceToList(_DWORD *a1)
{
_DWORD *result; // eax

result = a1;
if ( rDeviceList )
{
a1[36] = rDeviceList;
rDeviceList = a1;
}
else
{
rDeviceList = a1;
a1[36] = 0;
}
return result;
}
#endif

#if 0  // Cat2: Old D3D5 device capability checks; live rinit.c replaces this with D3D9 enumeration (0x57E090)
/*=== rinCheckDeviceCaps (0X57E090) ===*/
bool __cdecl rinCheckDeviceCaps(int a1, int a2, _BYTE *a3)
{
qmemcpy(&dword_A75DF0, a3, 0xFCu);
*(_DWORD *)&dword_A75DF0 = 0;
dword_A75DF4 = 0;
dword_A75DF8 = 0;
dword_A75DFC = 0;
dword_A75E08 = 0;
if ( (a3[116] & 0x12) == 0 )
return 0;
if ( (a3[120] & 0x23) == 0 )
return 0;
if ( (a3[112] & 0xA) == 0 )
return 0;
if ( (a3[136] & 3) != 0 )
return (a3[144] & 5) != 0;
return 0;
}
#endif

#if 0  // Cat2: Old D3D5 device enumeration callback; live rinit.c replaces this with D3D9 enumeration (0x57E680)
/*=== rinEnumD3DDevices_cb (0X57E680) ===*/
signed int __stdcall rinEnumD3DDevices_cb(int a1, int a2, int a3, int a4, int a5, int a6)
{
if ( !a1 || !a4 || !a5 )
return 0;
if ( *(_DWORD *)(a4 + 4) )
{
if ( sub_57E090(a2, a3, (_BYTE *)a4) )
++D3Ddevs;
}
return 1;
}
#endif

#if 0  // Cat2: Old DirectDraw primary device enumeration callback; live rinit.c replaces this with D3D9 enumeration (0x57E7E0)
/*=== rinEnumPrimaryDevice_cb (0X57E7E0) ===*/
BOOL __stdcall rinEnumPrimaryDevice_cb(GUID *lpGUID, const char *a2, const char *a3, char *a4)
{
char *v5; // esi
const char *v6; // [esp+0h] [ebp-Ch]

if ( !lpGUID )
return 1;
if ( DirectDrawCreate(lpGUID, (LPDIRECTDRAW *)&lpGUID, 0) < 0 )
return 1;
v5 = a4;
(*(void (__stdcall **)(GUID *, _DWORD, _DWORD, char *, signed int (__stdcall *)(_DWORD *, int)))(lpGUID->Data1 + 32))(
lpGUID,
0,
0,
a4,
rinEnumPrimaryDisplayModes_cb);
if ( v5 )
{
v6 = a3;
*(_DWORD *)v5 = 1;
strncpy(v5 + 4, v6, 0x3Fu);
strncpy(v5 + 68, a2, 0x3Fu);
v5[67] = 0;
v5[131] = 0;
}
return 0;
}
#endif
