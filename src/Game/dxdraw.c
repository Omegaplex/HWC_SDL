/*=============================================================================
    Name    : dxdraw.c
    Purpose : Decompiled functions from cataclysm.exe (dxdraw module)
=============================================================================*/

#include "types.h"
#include "debug.h"
#include "spaceobj.h"


// ============================================================================
// Decompiled from cataclysm.exe @ 0x5567F0
BOOL ddCreateWindow(int hWnd, DWORD width, DWORD height, DWORD depth)
{
    LPDIRECTDRAW dd;
    BOOL result;
    signed int hr1;
    signed int hr2;

    ddHWnd = hWnd;
    if ( fullScreen )
    {
        dd = lpDD;
        if ( !lpDD )
        {
            result = DirectDrawCreate(0, &lpDD, 0) == 0;
            if ( !result )
                return result;
            dd = lpDD;
        }
        hr1 = dd->lpVtbl->SetCooperativeLevel(dd, (HWND)ddHWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
        if ( hr1 )
        {
            ddReportError(hr1);
            hr2 = lpDD->lpVtbl->SetCooperativeLevel(lpDD, (HWND)ddHWnd, DDSCL_NORMAL);
            if ( hr2 )
            {
                ddReportError(hr2);
                return 0;
            }
        }
        if ( lpDD->lpVtbl->SetDisplayMode(lpDD, width, height, depth) )
            return 0;
        ddWidth = width;
        ddHeight = height;
        ddDepth = depth;
        ddModeSet = 1;
        lpDD->lpVtbl->SetCooperativeLevel(lpDD, (HWND)ddHWnd, DDSCL_NORMAL);
    }
    return 1;
}

// ============================================================================
// Decompiled from cataclysm.exe @ 0x556990
char *ddReportError(HRESULT hr)
{
    char *result;

    // Large switch statement over DirectDraw error codes
    // Organized as nested if/switch for compiler optimization
    if ( hr > DDERR_OVERLAYCOLORKEYRANGEFIXED )
    {
        switch ( hr )
        {
            case DDERR_PALETTEBUSY: result = "DDERR_PALETTEBUSY"; break;
            case DDERR_COLORKEYNOTSET: result = "DDERR_COLORKEYNOTSET"; break;
            case DDERR_SURFACEALREADYATTACHED: result = "DDERR_SURFACEALREADYATTACHED"; break;
            case DDERR_SURFACEALREADYDEPENDENT: result = "DDERR_SURFACEALREADYDEPENDENT"; break;
            case DDERR_SURFACEBUSY: result = "DDERR_SURFACEBUSY"; break;
            case DDERR_CANTLOCKSURFACE: result = "DDERR_CANTLOCKSURFACE"; break;
            case DDERR_SURFACEISOBSCURED: result = "DDERR_SURFACEISOBSCURED"; break;
            case DDERR_SURFACELOST: result = "DDERR_SURFACELOST"; break;
            case DDERR_SURFACENOTATTACHED: result = "DDERR_SURFACENOTATTACHED"; break;
            case DDERR_TOOBIGHEIGHT: result = "DDERR_TOOBIGHEIGHT"; break;
            case DDERR_TOOBIGSIZE: result = "DDERR_TOOBIGSIZE"; break;
            case DDERR_TOOBIGWIDTH: result = "DDERR_TOOBIGWIDTH"; break;
            case DDERR_UNSUPPORTEDFORMAT: result = "DDERR_UNSUPPORTEDFORMAT"; break;
            case DDERR_UNSUPPORTEDMASK: result = "DDERR_UNSUPPORTEDMASK"; break;
            case DDERR_INVALIDSTREAM: result = "DDERR_INVALIDSTREAM"; break;
            case DDERR_VERTICALBLANKINPROGRESS: result = "DDERR_VERTICALBLANKINPROGRESS"; break;
            case DDERR_WASSTILLDRAWING: result = "DDERR_WASSTILLDRAWING"; break;
            case DDERR_XALIGN: result = "DDERR_XALIGN"; break;
            case DDERR_INVALIDDIRECTDRAWGUID: result = "DDERR_INVALIDDIRECTDRAWGUID"; break;
            case DDERR_DIRECTDRAWALREADYCREATED: result = "DDERR_DIRECTDRAWALREADYCREATED"; break;
            case DDERR_NODIRECTDRAWHW: result = "DDERR_NODIRECTDRAWHW"; break;
            case DDERR_PRIMARYSURFACEALREADYEXISTS: result = "DDERR_PRIMARYSURFACEALREADYEXISTS"; break;
            case DDERR_NOEMULATION: result = "DDERR_NOEMULATION"; break;
            case DDERR_REGIONTOOSMALL: result = "DDERR_REGIONTOOSMALL"; break;
            case DDERR_CLIPPERISUSINGHWND: result = "DDERR_CLIPPERISUSINGHWND"; break;
            case DDERR_NOCLIPPERATTACHED: result = "DDERR_NOCLIPPERATTACHED"; break;
            case DDERR_NOHWND: result = "DDERR_NOHWND"; break;
            case DDERR_HWNDSUBCLASSED: result = "DDERR_HWNDSUBCLASSED"; break;
            case DDERR_HWNDALREADYSET: result = "DDERR_HWNDALREADYSET"; break;
            case DDERR_NOPALETTEATTACHED: result = "DDERR_NOPALETTEATTACHED"; break;
            case DDERR_NOPALETTEHW: result = "DDERR_NOPALETTEHW"; break;
            case DDERR_BLTFASTCANTCLIP: result = "DDERR_BLTFASTCANTCLIP"; break;
            case DDERR_NOBLTHW: result = "DDERR_NOBLTHW"; break;
            case DDERR_NODDROPSHW: result = "DDERR_NODDROPSHW"; break;
            case DDERR_OVERLAYNOTVISIBLE: result = "DDERR_OVERLAYNOTVISIBLE"; break;
            case DDERR_NOOVERLAYDEST: result = "DDERR_NOOVERLAYDEST"; break;
            case DDERR_INVALIDPOSITION: result = "DDERR_INVALIDPOSITION"; break;
            case DDERR_NOTAOVERLAYSURFACE: result = "DDERR_NOTAOVERLAYSURFACE"; break;
            case DDERR_EXCLUSIVEMODEALREADYSET: result = "DDERR_EXCLUSIVEMODEALREADYSET"; break;
            case DDERR_NOTFLIPPABLE: result = "DDERR_NOTFLIPPABLE"; break;
            case DDERR_CANTDUPLICATE: result = "DDERR_CANTDUPLICATE"; break;
            case DDERR_NOTLOCKED: result = "DDERR_NOTLOCKED"; break;
            case DDERR_CANTCREATEDC: result = "DDERR_CANTCREATEDC"; break;
            case DDERR_NODC: result = "DDERR_NODC"; break;
            case DDERR_WRONGMODE: result = "DDERR_WRONGMODE"; break;
            default: return "???";
        }
    }
    else if ( hr == DDERR_OVERLAYCOLORKEYRANGEFIXED ) return "DDERR_OVERLAYCOLORKEYRANGEFIXED";
    // ... (additional nested switch blocks for lower error codes)
    // DDERR_GENERIC, DDERR_UNSUPPORTED, DDERR_OUTOFMEMORY, DDERR_INVALIDPARAMS,
    // DDERR_ALREADYINITIALIZED, DDERR_CANNOTATTACHSURFACE, DDERR_CURRENTLYNOTAVAIL,
    // DDERR_EXCEPTION, DDERR_HEIGHTALIGN, DDERR_INCOMPATIBLEPRIMARY, etc.
    else return "???";
    return result;
}
