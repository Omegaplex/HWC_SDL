/*=============================================================================
    Name    : beam.c
    Purpose : Sentinel forcefield beam rendering helpers.
=============================================================================*/

#include "glinc.h"

#include "beam.h"
#include "types.h"
#include "fastmath.h"
#include "debug.h"
#include "spaceobj.h"
#include "universe.h"
#include "render.h"
#include "prim3d.h"
#include "texreg.h"
#include "select.h"
#include "randy.h"

extern trhandle g_glowHandle;
void rndFilter(bool32 on);

typedef struct BeamEffectEntry
{
    SentinelSpecifics *sourceSpec;
    SentinelSpecifics *linkASpec;
    SentinelSpecifics *linkBSpec;
    vector sourcePosition;
    vector linkAPosition;
    vector linkBPosition;
    real32 distance;
    struct BeamEffectEntry *next;
} BeamEffectEntry;

#define BEAM_MAX_EFFECTS_RGL         79
#define BEAM_MAX_EFFECTS_SOFTWARE    34
#define BEAM_POOL_SIZE               BEAM_MAX_EFFECTS_RGL
#define BEAM_CHARGE_BLEND_TIME       0.75f
#define BEAM_CORE_BLEND_TIME         0.25f
#define BEAM_TRIANGLE_FADE_TIME      10.0f
#define BEAM_CIRCLE_SCALE            4.0f
#define BEAM_CIRCLE_RADIUS_SCALE     0.65f

static BeamEffectEntry beamEffectPool[BEAM_POOL_SIZE];
static BeamEffectEntry *beamEffectList = NULL;
static sdword beamEffectCount = 0;
static const color beamCoreColors[3] =
{
    colRGBA(0, 0, 255, 255),
    colRGBA(0, 255, 0, 255),
    colRGBA(255, 0, 0, 255)
};
static const color beamTriangleColors[3] =
{
    colRGBA(0, 0, 200, 100),
    colRGBA(0, 200, 0, 100),
    colRGBA(200, 0, 0, 100)
};
static const color beamCircleColor = colRGBA(255, 0, 0, 255);
static const color beamPulseColor = colRGBA(255, 255, 255, 150);

static ubyte beamColorComponent(color colorValue, sdword shift)
{
    return (ubyte)((colorValue >> shift) & 0xff);
}

static color beamBlendRGBA(color c0, color c1, real32 factor)
{
    sdword blend;
    sdword inverseBlend;

    if (factor < 0.0f)
    {
        blend = 0;
    }
    else
    {
        blend = (sdword)(factor * 255.0f + 0.5f);
        if (blend > 255)
        {
            blend = 255;
        }
    }

    inverseBlend = 255 - blend;
    return
        (((blend * colRed(c0) + inverseBlend * colRed(c1)) >> 8) & 0x000000ff) |
        (((blend * colGreen(c0) + inverseBlend * colGreen(c1))     ) & 0x0000ff00) |
        (((blend * colBlue(c0) + inverseBlend * colBlue(c1)) << 8) & 0x00ff0000) |
        (((blend * colAlpha(c0) + inverseBlend * colAlpha(c1)) << 16) & 0xff000000);
}

static sdword beamMaxEffectCount(void)
{
    return (RGLtype != 0) ? BEAM_MAX_EFFECTS_RGL : BEAM_MAX_EFFECTS_SOFTWARE;
}

static void beamGetForceFieldOriginInWorld(vector *worldPosition, Ship *ship)
{
    DockStaticInfo *dockStaticInfo = ship->staticinfo->dockStaticInfo;
    ubyte dockIndex;

    dbgAssert(dockStaticInfo != NULL);
    dockIndex = dockStaticInfo->firstPointOfType[DPT_ForceFieldOrigin];
    dbgAssert(dockIndex < dockStaticInfo->numDockPoints);

    matMultiplyMatByVec(worldPosition, &ship->rotinfo.coordsys, &dockStaticInfo->dockstaticpoints[dockIndex].position);
    worldPosition->x += ship->posinfo.position.x;
    worldPosition->y += ship->posinfo.position.y;
    worldPosition->z += ship->posinfo.position.z;
}

static sdword beamRenderEffect(BeamEffectEntry *beamEffect)
{
    SentinelSpecifics *sourceSpec = beamEffect->sourceSpec;
    real32 edgeBlend;
    real32 centerBlend;
    color outerColor;
    color innerColor;
    real32 circleBlend;
    color triangleColor;
    color fadedTriangleColor;
    color alphaColor;
    sdword formationLevel;
    vector midpointA;
    vector midpointB;
    vector origin = { 0.0f, 0.0f, 0.0f };
    real32 elapsedSincePulseStart = universe.totaltimeelapsed - sourceSpec->forcefieldPulseStartTime;
    sdword depthTestingState;

    formationLevel = sourceSpec->formationLevel;
    dbgAssert(formationLevel >= 0 && formationLevel < 3);

    edgeBlend = sourceSpec->forcefieldChargeTimer / BEAM_CHARGE_BLEND_TIME;
    if (edgeBlend > 1.0f)
    {
        edgeBlend = 1.0f;
    }
    outerColor = beamBlendRGBA(0, beamCoreColors[formationLevel], 1.0f - edgeBlend);

    centerBlend = (sourceSpec->forcefieldChargeTimer - BEAM_CHARGE_BLEND_TIME) / BEAM_CORE_BLEND_TIME;
    if (centerBlend > 1.0f)
    {
        centerBlend = 1.0f;
    }
    else if (centerBlend < 0.0f)
    {
        centerBlend = 0.0f;
    }
    innerColor = beamBlendRGBA(0, beamCoreColors[formationLevel], 1.0f - centerBlend);

    midpointA.x = beamEffect->sourcePosition.x + (beamEffect->linkAPosition.x - beamEffect->sourcePosition.x) * 0.5f;
    midpointA.y = beamEffect->sourcePosition.y + (beamEffect->linkAPosition.y - beamEffect->sourcePosition.y) * 0.5f;
    midpointA.z = beamEffect->sourcePosition.z + (beamEffect->linkAPosition.z - beamEffect->sourcePosition.z) * 0.5f;
    midpointB.x = beamEffect->sourcePosition.x + (beamEffect->linkBPosition.x - beamEffect->sourcePosition.x) * 0.5f;
    midpointB.y = beamEffect->sourcePosition.y + (beamEffect->linkBPosition.y - beamEffect->sourcePosition.y) * 0.5f;
    midpointB.z = beamEffect->sourcePosition.z + (beamEffect->linkBPosition.z - beamEffect->sourcePosition.z) * 0.5f;

    rndAdditiveBlends(TRUE);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    rndLightingEnable(FALSE);
    rndTextureEnable(FALSE);

    glBegin(GL_LINES);
    glColor4ub(
        beamColorComponent(outerColor, 0),
        beamColorComponent(outerColor, 8),
        beamColorComponent(outerColor, 16),
        beamColorComponent(outerColor, 24));
    glVertex3f(beamEffect->sourcePosition.x, beamEffect->sourcePosition.y, beamEffect->sourcePosition.z);
    glColor4ub(
        beamColorComponent(innerColor, 0),
        beamColorComponent(innerColor, 8),
        beamColorComponent(innerColor, 16),
        beamColorComponent(innerColor, 24));
    glVertex3f(midpointA.x, midpointA.y, midpointA.z);
    glVertex3f(midpointA.x, midpointA.y, midpointA.z);
    glColor4ub(
        beamColorComponent(outerColor, 0),
        beamColorComponent(outerColor, 8),
        beamColorComponent(outerColor, 16),
        beamColorComponent(outerColor, 24));
    glVertex3f(beamEffect->linkAPosition.x, beamEffect->linkAPosition.y, beamEffect->linkAPosition.z);
    glColor4ub(
        beamColorComponent(outerColor, 0),
        beamColorComponent(outerColor, 8),
        beamColorComponent(outerColor, 16),
        beamColorComponent(outerColor, 24));
    glVertex3f(beamEffect->sourcePosition.x, beamEffect->sourcePosition.y, beamEffect->sourcePosition.z);
    glColor4ub(
        beamColorComponent(innerColor, 0),
        beamColorComponent(innerColor, 8),
        beamColorComponent(innerColor, 16),
        beamColorComponent(innerColor, 24));
    glVertex3f(midpointB.x, midpointB.y, midpointB.z);
    glVertex3f(midpointB.x, midpointB.y, midpointB.z);
    glColor4ub(
        beamColorComponent(outerColor, 0),
        beamColorComponent(outerColor, 8),
        beamColorComponent(outerColor, 16),
        beamColorComponent(outerColor, 24));
    glVertex3f(beamEffect->linkBPosition.x, beamEffect->linkBPosition.y, beamEffect->linkBPosition.z);
    glEnd();

    if (sourceSpec->forcefieldChargeTimer == 1.0f)
    {
        alphaColor = beamTriangleColors[formationLevel];
        circleBlend = 1.0f - sourceSpec->forcefieldPulseScale;
        if (circleBlend >= 0.5f)
        {
            if (circleBlend >= 0.8f)
            {
                triangleColor = beamBlendRGBA(beamPulseColor, alphaColor, (1.0f - circleBlend) * 5.0f);
            }
            else
            {
                triangleColor = beamPulseColor;
            }
        }
        else
        {
            triangleColor = beamBlendRGBA(beamPulseColor, alphaColor, frandom(1.0f));
        }

        if (elapsedSincePulseStart <= BEAM_TRIANGLE_FADE_TIME)
        {
            real32 fade = (BEAM_TRIANGLE_FADE_TIME - elapsedSincePulseStart) / BEAM_TRIANGLE_FADE_TIME;
            if (fade > 0.0f)
            {
                fadedTriangleColor = beamBlendRGBA(beamPulseColor, alphaColor, fade);
                triangleColor = fadedTriangleColor;
            }
        }

        glColor4ub(
            beamColorComponent(triangleColor, 0),
            beamColorComponent(triangleColor, 8),
            beamColorComponent(triangleColor, 16),
            beamColorComponent(triangleColor, 24));
        glBegin(GL_TRIANGLES);
        glVertex3f(beamEffect->sourcePosition.x, beamEffect->sourcePosition.y, beamEffect->sourcePosition.z);
        glVertex3f(beamEffect->linkAPosition.x, beamEffect->linkAPosition.y, beamEffect->linkAPosition.z);
        glVertex3f(beamEffect->linkBPosition.x, beamEffect->linkBPosition.y, beamEffect->linkBPosition.z);
        glEnd();

        circleBlend = sourceSpec->forcefieldChargeTimer / BEAM_CIRCLE_RADIUS_SCALE;
        if (circleBlend > 1.0f)
        {
            circleBlend = 1.0f;
        }
        circleBlend *= BEAM_CIRCLE_SCALE;

        depthTestingState = rndSetDepthTesting(FALSE);
        rndTextureEnvironment(RTE_Modulate);
        if (g_glowHandle == TR_InvalidHandle)
        {
            rndBillboardEnable(&beamEffect->sourcePosition);
            primCircleSolid3(&origin, circleBlend, 16, beamCircleColor);
            rndBillboardDisable();
            rndBillboardEnable(&beamEffect->linkAPosition);
            primCircleSolid3(&origin, circleBlend, 16, beamCircleColor);
            rndBillboardDisable();
            rndBillboardEnable(&beamEffect->linkBPosition);
            primCircleSolid3(&origin, circleBlend, 16, beamCircleColor);
        }
        else
        {
            trMakeCurrent(g_glowHandle);
            rndFilter(TRUE);
            rndBillboardEnable(&beamEffect->sourcePosition);
            primSolidTexture3(&origin, circleBlend, beamCircleColor, g_glowHandle);
            rndBillboardDisable();
            rndBillboardEnable(&beamEffect->linkAPosition);
            primSolidTexture3(&origin, circleBlend, beamCircleColor, g_glowHandle);
            rndBillboardDisable();
            rndBillboardEnable(&beamEffect->linkBPosition);
            primSolidTexture3(&origin, circleBlend, beamCircleColor, g_glowHandle);
        }
        rndBillboardDisable();
        rndSetDepthTesting(depthTestingState);
    }

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    rndLightingEnable(TRUE);

    return 0;
}

static void beamAddEffectEntry(
    SentinelSpecifics *sourceSpec,
    SentinelSpecifics *linkASpec,
    SentinelSpecifics *linkBSpec,
    const vector *sourcePosition,
    const vector *linkAPosition,
    const vector *linkBPosition,
    real32 distance)
{
    BeamEffectEntry *entry;
    BeamEffectEntry *previous;
    BeamEffectEntry *current;

    if (beamEffectCount >= beamMaxEffectCount())
    {
        entry = beamEffectList;
        if (entry->distance <= distance)
        {
            return;
        }
        beamEffectList = entry->next;
    }
    else
    {
        entry = &beamEffectPool[beamEffectCount++];
    }

    previous = NULL;
    current = beamEffectList;
    while (current != NULL && current->distance >= distance)
    {
        previous = current;
        current = current->next;
    }

    entry->next = current;
    if (previous == NULL)
    {
        beamEffectList = entry;
    }
    else
    {
        previous->next = entry;
    }

    entry->sourceSpec = sourceSpec;
    entry->linkASpec = linkASpec;
    entry->linkBSpec = linkBSpec;
    entry->sourcePosition = *sourcePosition;
    entry->linkAPosition = *linkAPosition;
    entry->linkBPosition = *linkBPosition;
    entry->distance = distance;
}

static void beamAddRenderEntry(const vector *cameraPosition, Ship *ship, Ship *linkA, Ship *linkB)
{
    SentinelSpecifics *sourceSpec = sentinelShipSpecifics(ship);
    vector sourcePosition;
    vector linkAPosition;
    vector linkBPosition;
    vector queueCenter;
    real32 dx;
    real32 dy;
    real32 dz;

    if (sourceSpec->forcefieldChargeTimer <= 0.2f)
    {
        return;
    }

    beamGetForceFieldOriginInWorld(&sourcePosition, ship);
    beamGetForceFieldOriginInWorld(&linkAPosition, linkA);
    beamGetForceFieldOriginInWorld(&linkBPosition, linkB);

    queueCenter.x = (sourcePosition.x + linkAPosition.x + linkBPosition.x) * (1.0f / 3.0f);
    queueCenter.y = (sourcePosition.y + linkAPosition.y + linkBPosition.y) * (1.0f / 3.0f);
    queueCenter.z = (sourcePosition.z + linkAPosition.z + linkBPosition.z) * (1.0f / 3.0f);

    dx = queueCenter.x - cameraPosition->x;
    dy = queueCenter.y - cameraPosition->y;
    dz = queueCenter.z - cameraPosition->z;
    beamAddEffectEntry(
        sourceSpec,
        sentinelShipSpecifics(linkA),
        sentinelShipSpecifics(linkB),
        &sourcePosition,
        &linkAPosition,
        &linkBPosition,
        fmathSqrtDouble(dx * dx + dy * dy + dz * dz));
}

void beamUpdateEffects(const vector *cameraPosition, SpaceObj *spaceobj)
{
    real32 distance = 0.0f;

    if (spaceobj != NULL)
    {
        real32 dx = spaceobj->posinfo.position.x - cameraPosition->x;
        real32 dy = spaceobj->posinfo.position.y - cameraPosition->y;
        real32 dz = spaceobj->posinfo.position.z - cameraPosition->z;
        distance = fsqrt(dx * dx + dy * dy + dz * dz);
    }

    while (beamEffectList != NULL && beamEffectList->distance > distance)
    {
        beamRenderEffect(beamEffectList);
        beamEffectList = beamEffectList->next;
    }
}

void beamFlushEffects(void)
{
    while (beamEffectList != NULL && beamEffectList->distance > 0.0f)
    {
        beamRenderEffect(beamEffectList);
        beamEffectList = beamEffectList->next;
    }

    beamEffectCount = 0;
    beamEffectList = NULL;
}

void beamRenderBeams(const vector *cameraPosition, SelectCommand *selection)
{
    sdword i;

    if (selection == NULL)
    {
        return;
    }

    for (i = 0; i < selection->numShips; i++)
    {
        Ship *ship = selection->ShipPtr[i];
        SentinelSpecifics *spec = sentinelShipSpecifics(ship);
        sdword linkIndex;

        for (linkIndex = 0; linkIndex < spec->formationLinkCount; linkIndex++)
        {
            Ship *linkA = spec->formationLinks[linkIndex * 2];
            Ship *linkB = spec->formationLinks[linkIndex * 2 + 1];
            if (linkA != NULL && linkB != NULL)
            {
                beamAddRenderEntry(cameraPosition, ship, linkA, linkB);
            }
        }
    }
}
