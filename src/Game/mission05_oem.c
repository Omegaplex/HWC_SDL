/*=============================================================================
    Name    : mission05_oem.c
    Purpose : Extracted Cataclysm mission-script callbacks for mission05_oem
=============================================================================*/

#include "MissionScriptSupport.h"

void Begin_Mission17_BeastSentinelReinforcementCombatLoop(void);
void Watch_Mission17_BeastSentinelAssault(void);

void Watch_Mission05_OEM_MissionFlow_DontGoThere(void)
{
    kasCurrentStep = 2;
    kasCurrentScopeHash = ((AITeam *)kasCurrentTeamPtr)->kasLabelHash;
    if (kasTimerExpiredById(0x05B5E4))
    {
        kasfStop();
        kasfSpeechEvent(
            0x0489AA24,
            (sdword)Begin_Mission17_BeastSentinelReinforcementCombatLoop,
            (sdword)Watch_Mission17_BeastSentinelAssault);
    }
}
