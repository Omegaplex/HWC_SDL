/*=============================================================================
    Name    : researchapi_old.c
    Purpose : archive for dead HW1-era research helpers removed from researchapi.c
=============================================================================*/

#include "researchapi.h"
#include "universe.h"
#include "statscript.h"

sdword rmNumTechRequiredForTech(Player *player, TechnologyType type)
{
    sdword i, numtech = 0;
    udword techdepend, temp;

    temp = player->researchinfo.techstat->TechNeededToResearch[type];
    techdepend = temp & player->researchinfo.HasTechnology;
    techdepend ^= temp;

    for (i = 0; i < NumTechnologies; i++)
    {
        if (bitTest(techdepend, TechToBit(i)))
        {
            if ((temp = player->researchinfo.techstat->TechNeededToResearch[i]) != 0)
            {
                numtech += rmNumTechRequiredForTech(player, i);
            }

            udword rmTechRequiredForTech(Player *player, TechnologyType type)
            {
                sdword i;
                udword techdepend;
                udword temp;

                temp = player->researchinfo.techstat->TechNeededToResearch[type];
                techdepend = temp & player->researchinfo.HasTechnology;
                techdepend ^= temp;

                for (i = NumTechnologies; i >= 0; i--)
                {
                    if (bitTest(techdepend, TechToBit(i)))
                    {
                        return i;
                    }
                }

                return 0;
            }

            udword rmAllTechRequiredforTech(Player *player, TechnologyType type)
            {
                udword i;
                udword techdepend;
                udword temp;

                techdepend = temp = player->researchinfo.techstat->TechNeededToResearch[type];

                for (i = 0; i < NumTechnologies; i--)
                {
                    if (bitTest(temp, TechToBit(i)))
                    {
                        techdepend |= rmAllTechRequiredforTech(player, i);
                    }
                }

                return techdepend;
            }

            udword rmAllTechRequredForShip(Player *player, ShipType type)
            {
                sdword i;
                udword techneeded;
                udword temp;

                temp = player->researchinfo.techstat->TechNeededToBuildShip[type];
                techneeded = temp;

                for (i = 0; i < NumTechnologies; i++)
                {
                    if (bitTest(temp, TechToBit(i)))
                    {
                        techneeded |= rmAllTechRequiredforTech(player, i);
                    }
                }

                techneeded &= ~player->researchinfo.HasTechnology;
                return techneeded;
            }
            numtech++;
        }
    }
    return numtech;
}

void rmGiveTechToPlayerByName(struct Player *player, char *techName)
{
    rmGiveTechToPlayerByType(player, StrToTechType(techName));
}

void rmRemoveUnneededTech(TechStatics *techstat)
{
    udword i, j, k, tech, techtmp;
    bool32 notneeded;

    for (i = 0; i < TOTAL_NUM_SHIPS; i++)
    {
        tech = techstat->TechNeededToBuildShip[i];
        if (bitTest(tech, RM_Disabled))
        {
            for (j = 0; j < NumTechnologies; j++)
            {
                notneeded = TRUE;

                if (bitTest(tech, TechToBit(j)))
                {
                    for (k = 0; k < TOTAL_NUM_SHIPS; k++)
                    {
                        techtmp = techstat->TechNeededToBuildShip[k];
                        if (!bitTest(techtmp, RM_Disabled) && k != i)
                        {
                            if (bitTest(techtmp, TechToBit(j)))
                            {
                                notneeded = FALSE;
                            }
                        }
                    }
                    if (notneeded)
                    {
                        techstat->TechNeededToResearch[j] |= RM_Disabled;
                    }
                }
            }
        }
    }
}

void rmRemoveAllUnneededTech(void)
{
    rmRemoveUnneededTech(&MultiPlayerSectTechStatic);
    rmRemoveUnneededTech(&MultiPlayerCarrierSectTechStatic);
    rmRemoveUnneededTech(&SinglePlayerTechStatic);
    rmRemoveUnneededTech(&MultiPlayerBeastTechStatic);
    rmRemoveUnneededTech(&MultiPlayerCarrierBeastTechStatic);
}

static bool32 rmTechResearched(udword researchMask, sdword techType)
{
    return bitTest(researchMask, TechToBit(techType));
}

void rmDeactivateLab(Player *player)
{
    sdword index, maxindex = -1;
    sdword labCount = rmPlayerResearchLabCount(player);
    real32 maxtimeleft = 0.0f;
    ResearchLab *lab;

    index = rmFindFreeLab(player, -1);

    if (index != -1)
    {
        rmPlayerResearchLab(player, index)->labstatus = LS_NORESEARCHSHIP;
    }
    else
    {
        for (index = 0; index < labCount; index++)
        {
            lab = rmPlayerResearchLab(player, index);
            if (lab->labstatus != LS_NORESEARCHSHIP && lab->topic->timeleft > maxtimeleft)
            {
                maxtimeleft = lab->topic->timeleft;
                maxindex = index;
            }
        }

        rmClearResearchlab(player, maxindex);
        rmPlayerResearchLab(player, maxindex)->labstatus = LS_NORESEARCHSHIP;
    }

    if (player == universe.curPlayerPtr && rmGUIActive)
    {
        rmUpdateTechList();
    }
}

void rmActivateFreeLab(Player *player)
{
    sdword labindex;
    sdword labCount = rmPlayerResearchLabCount(player);
    ResearchLab *lab;

    for (labindex = 0; labindex < labCount; labindex++)
    {
        lab = rmPlayerResearchLab(player, labindex);
        if (lab->labstatus == LS_NORESEARCHSHIP)
        {
            lab->labstatus = LS_NORESEARCHITEM;
            if (player == universe.curPlayerPtr && rmGUIActive)
            {
                rmUpdateTechList();
            }
            break;
        }
    }
}
