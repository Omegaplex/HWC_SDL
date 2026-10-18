/*=============================================================================
    Name    : KASHashes.h
    Purpose : Pre-computed hash constants for KAS label strings.

    The Cataclysm binary's KAS compiler (kas2c) replaced all label string
    references with their integer hashes at compile time. HW1's compiler
    emitted string literals directly (e.g. kasAITeamShipsPtr("TaiidanBombers")).

    All values are computed by kasHashLabel() in KAS.c, a case-insensitive
    ELF/PJW variant. The macro name is the original KAS label string from the
    .missphere or .kas source file.

    Usage:
        kasAITeamPtrIfExists(KH_FleetIntel)  -- instead of 0x0A8A880C

    Adding new constants:
        1. Identify the label string from the .missphere file or KAS source.
        2. Verify with Python:
               python -c "
               def h(s):
                   r=0
                   for c in s:
                       r=(16*r+(ord(c)&0xDF))&0xFFFFFFFF
                       if r&0xF0000000: r^=(r&0xF0000000)^((r&0xF0000000)>>24)
                   return r-0x100000000 if r>=0x80000000 else r
               print(hex(h('MyLabel')))"
        3. Add a #define with the format:
               #define KH_LabelName  0xXXXXXXXX
        4. Replace bare hex/decimal literals with the constant in .c files.

    For labels whose original string is unknown, the constant is named
    KH_<CONTEXT>_UNKNOWN_<hint> with a TODO comment noting its usage.
    Replace once the original string is identified.
=============================================================================*/

#ifndef KASHASHES_H
#define KASHASHES_H

/* -------------------------------------------------------------------------
 * Mission scope identifiers (stored in kasCurrentScopeHash / "scope" in KAS.c)
 * ------------------------------------------------------------------------- */
#define KH_mission01        0x087E6EF1  /* kasHashLabel("mission01") */
#define KH_mission02        0x087E6EF2  /* kasHashLabel("mission02") */
#define KH_mission03        0x087E6EF3  /* kasHashLabel("mission03") */
#define KH_mission04        0x087E6EF4  /* kasHashLabel("mission04") */
#define KH_mission05        0x087E6EF5  /* kasHashLabel("mission05") */
#define KH_mission06        0x087E6EF6  /* kasHashLabel("mission06") */
#define KH_mission07        0x087E6EF7  /* kasHashLabel("mission07") */
#define KH_mission08        0x087E6EF8  /* kasHashLabel("mission08") */
#define KH_mission09        0x087E6EF9  /* kasHashLabel("mission09") */
#define KH_mission10        0x087E6EC0  /* kasHashLabel("mission10") */
#define KH_mission11        0x087E6EC1  /* kasHashLabel("mission11") */
#define KH_mission12        0x087E6EC2  /* kasHashLabel("mission12") */
#define KH_mission13        0x087E6EC3  /* kasHashLabel("mission13") */
#define KH_mission14        0x087E6EC4  /* kasHashLabel("mission14") */
#define KH_mission15        0x087E6EC5  /* kasHashLabel("mission15") */
#define KH_mission16        0x087E6EC6  /* kasHashLabel("mission16") */

/* -------------------------------------------------------------------------
 * Shared team labels — used across multiple missions
 * ------------------------------------------------------------------------- */
#define KH_FleetIntel       0x0A8A880C  /* kasHashLabel("FleetIntel") */
#define KH_Objectives       0x098DDC43  /* kasHashLabel("Objectives") */

/* -------------------------------------------------------------------------
 * Shared volume / grow-selection labels
 *
 * "Everything" is the AIBox covering the entire mission sphere.
 * "AllShips"   is the full-sphere ship scan selection.
 * "Enemy" / "Friendly" are from the NPC FSM perspective:
 *   Enemy    = player (Somtaaw / playerIndex 0)
 *   Friendly = NPC allied ships (playerIndex != 0)
 * "Mothership"  = grow-selection for the player Somtaaw mothership.
 * "sMothership" = grow-selection for an enemy Sect mothership unit.
 * ------------------------------------------------------------------------- */
#define KH_Everything       0x07EC6887  /* kasHashLabel("Everything") */
#define KH_AllShips         0x0117CA33  /* kasHashLabel("AllShips")   */
#define KH_Enemy            0x004A2A29  /* kasHashLabel("Enemy")      */
#define KH_Friendly         0x06DA2DA9  /* kasHashLabel("Friendly")   */
#define KH_Mothership       0x0CA25860  /* kasHashLabel("Mothership") */
#define KH_sMothership      0x0CFF5860  /* kasHashLabel("sMothership") */

/* Ship-type grow-selection labels reused as KAS variable names */
#define KH_sMothershipDockingBay    0x07CA3A49  /* kasHashLabel("sMothershipDockingBay") */
#define KH_sMothershipMicro         0x069E2DEF  /* kasHashLabel("sMothershipMicro") */

/* -------------------------------------------------------------------------
 * Mission 01 team / timer / flag labels  (resourcesphere_1.missphere)
 * ------------------------------------------------------------------------- */

/* Timer whose original KAS string is unknown.
   Replace with exact label once .kas source is found. */
#define KH_M01_SCAN_TIMER           0x030719C2  /* TODO: identify. Repeating scan-and-select timer in Watch_Mission01. */

/* Mission 1 post-load NIS / arrival handoff flags. */
#define KH_INTEL_FLOW_ACTIVE         0x8AFA30C4  /* Global hash. Matches G_PlayNislet. */
#define KH_INTEL_FLOW_READY          0x8C641D25  /* Global hash. Matches G_Nislet1Complete; retail Watch_Mission01_TutorialFlow waits on this before resuming. */
#define KH_M01_INIT_COMPLETE        0x87DEFCE4  /* Global hash. Matches G_ShipsHaveArrived, set from singleplayer.c on hyperspace arrival completion. */
#define KH_M01_MS_FOUND             0x0443CCB7  /* TODO: identify. Signed: 71552183. Set after first enemy-mothership lookup in Watch_Mission01 / Watch_Mission02. */

/* -------------------------------------------------------------------------
 * Mission 02 team labels  (resourcesphere_1/2/3.missphere)
 * KH_FleetIntel and KH_Objectives are shared — see above.
 * ------------------------------------------------------------------------- */

/* Pirate faction (resourcesphere_2) */
#define KH_M02_Pirates          0x04E668F3  /* kasHashLabel("Pirates") */
#define KH_M02_Kamikaze         0x061DF215  /* kasHashLabel("Kamikaze") */
#define KH_M02_pCarrier         0x076768E2  /* kasHashLabel("pCarrier") */
#define KH_M02_pFighter1        0x0DBDDE91  /* kasHashLabel("pFighter1") */
#define KH_M02_pFighter2        0x0DBDDE92  /* kasHashLabel("pFighter2") */
#define KH_M02_pFighter3        0x0DBDDE93  /* kasHashLabel("pFighter3") */
#define KH_M02_pSalvage         0x0761A0E5  /* kasHashLabel("pSalvage") */
#define KH_M02_pTricky          0x0596D859  /* kasHashLabel("pTricky") */
#define KH_M02_pGuard           0x054C9664  /* kasHashLabel("pGuard") */
#define KH_M02_pPatrol0         0x04697680  /* kasHashLabel("pPatrol0") */
#define KH_M02_pPatrol1         0x04697681  /* kasHashLabel("pPatrol1") */
#define KH_M02_pPatrol2         0x04697682  /* kasHashLabel("pPatrol2") */
#define KH_M02_pPatrol3         0x04697683  /* kasHashLabel("pPatrol3") */
#define KH_M02_pPatrol4         0x04697684  /* kasHashLabel("pPatrol4") */
#define KH_M02_pPatrol5         0x04697685  /* kasHashLabel("pPatrol5") */
#define KH_M02_Ambush1          0x0617A7D1  /* kasHashLabel("Ambush1") */
#define KH_M02_Ambush2          0x0617A7D2  /* kasHashLabel("Ambush2") */
#define KH_M02_Ambush3          0x0617A7D3  /* kasHashLabel("Ambush3") */
#define KH_M02_Decoy            0x00489849  /* kasHashLabel("Decoy")   */

/* Allied ships (resourcesphere_2/3) */
#define KH_M02_kDestroyer       0x0893B502  /* kasHashLabel("kDestroyer") */
#define KH_M02_dMessagePod      0x0712BFB4  /* kasHashLabel("dMessagePod") */

/* -------------------------------------------------------------------------
 * Mission 02 FSM name labels — first argument to kasFSMCreate().
 *
 * Each FSM is registered under a thematic KAS label that other FSMs use to
 * send messages to it. FSM names may differ from team labels.
 * ------------------------------------------------------------------------- */
#define KH_M02_FSM_Destroyer    0x08970502  /* kasHashLabel("Destroyer")  — DamagedDestroyer FSM */
#define KH_M02_FSM_Salvage      0x0761A5E5  /* kasHashLabel("Salvage")    — PirateSalvageCorvette FSM */
#define KH_M02_FSM_Strike       0x05896DF5  /* kasHashLabel("Strike")     — PirateStrikeWave FSM */
#define KH_M02_FSM_MessagePod   0x075EBFB4  /* kasHashLabel("MessagePod") — SalvagePodApproach FSM */
#define KH_M02_FSM_Ambush       0x04617A78  /* kasHashLabel("Ambush")     — PirateTeamReinforcement FSM */

/* FSM names whose original KAS strings are not yet identified */
#define KH_M02_FSM_UNKNOWN_PATROL   0x04697443  /* TODO: identify. FSM name for all 6 HonourGuardSwarmPatrol instances (teams pPatrol0..5). */
#define KH_M02_FSM_UNKNOWN_ASSAULT  0x07676DE2  /* TODO: identify. FSM name for PirateAssaultTeam (team = pCarrier). */
#define KH_M02_FSM_UNKNOWN_HEAVY    0x04C66683  /* TODO: identify. FSM name for PirateHeavyStrikeTeam (team = pFighter3). */
#define KH_M02_FSM_UNKNOWN_MISSILE  0x0E1AE0B4  /* TODO: identify. FSM name for PirateMissileCorvetteTeam (team = pGuard). */
#define KH_M02_FSM_UNKNOWN_STRIKE   0x05896DF5  /* kasHashLabel("Strike")  — reuse of KH_M02_FSM_Strike */

/* -------------------------------------------------------------------------
 * Mission 02 volume / grow-selection labels
 * ------------------------------------------------------------------------- */

/* Beast ship area used in Init_Mission02. Both the volume and the growsel
   share this label (same label used for both entity types in KAS). */
#define KH_M02_BEAST_AREA           0x00004C34  /* TODO: identify. Signed: 19508. Volume + growsel for initial beast ship scan in Init_Mission02. */

/* -------------------------------------------------------------------------
 * Mission 02 timer labels (original strings not confirmed)
 * ------------------------------------------------------------------------- */
#define KH_M02_DIFFICULTY_TIMER     0x0004CE34  /* TODO: identify. Signed: 314932. Difficulty-scaled patrol start delay (120/180/360 s). Also checked in Watch_Mission02. */
#define KH_M02_SCAN_TIMER           0x06381513  /* TODO: identify. Signed: 104338707. Recurring ship scan/select timer in Watch_Mission02. */
#define KH_M02_CLOAK_DETECT_TIMER   0x0BE10CA3  /* TODO: identify. Signed: 199298211. Cloaked-ship detection poll timer in Watch_Mission02. */

/* -------------------------------------------------------------------------
 * Mission 02 entry / state flags (original strings not confirmed)
 * ------------------------------------------------------------------------- */

/* Detected cloaked ships — used as both a growsel and an entry flag */
#define KH_M02_CLOAKED_FOUND        0x01A8A423  /* TODO: identify. Signed: 27829283. Entry + growsel for cloaked pirate ships detected in sensor range. */

/* Dock status for sMothershipDockingBay */
#define KH_M02_MS_DOCKED            0x098B21D9  /* TODO: identify. Signed: 160113113. Entry tracking whether Somtaaw MS has docked to upgrade bay. */

/* Flags set when key Watch_Mission02 events occur (sequential KAS label series) */
#define KH_M02_FLAG_CLOAKED_SIGHTED     0x0443CCB4  /* TODO: identify. Signed: 71552180. */
#define KH_M02_FLAG_INIT_COMPLETE       0x0443CCB5  /* TODO: identify. Signed: 71552181. */
#define KH_M02_FLAG_SHIPS_SCANNED       0x0443CCB6  /* TODO: identify. Signed: 71552182. */
#define KH_M02_FLAG_MICRO_QUEUED        0x0443CCB8  /* TODO: identify. Signed: 71552184. */

/* KH_M01_MS_FOUND (0x0443CCB7 / 71552183) is shared between M01 and M02 — see above. */

/* pPatrol detected in sensor range */
#define KH_M02_PATROL_SIGHTED       0x8D8B9E42  /* TODO: identify. Signed: -1920229822. Set when player sensor range first detects a pPatrol ship. */

/* sMothershipMicro docking-related state */
#define KH_M02_MICRO_DOCKING        0x8F624235  /* TODO: identify. Signed: -1889385931. Checked alongside sMothershipMicro dock test in Watch_Mission02. */

/* Final mission state */
#define KH_M02_MISSION_DONE         0x849928E4  /* TODO: identify. Signed: -2070337308. Checked as final condition in Watch_Mission02 return path. */

/* Intermediate grow-selections used for ship filtering in Watch_Mission02 */
#define KH_M02_SEL_PLAYER_NSAL      0x05506DA2  /* TODO: identify. Signed: 89157026. Player non-salvager ship selection. */
#define KH_M02_SEL_LINKED_SHIPS     0x04E37D85  /* TODO: identify. Signed: 82017669. Mothership-linked ship selection. */
#define KH_M02_SEL_WORKER_CHECK     0x05C46FA2  /* TODO: identify. Signed: 96759714. Used to check if a sWorker exists in player ships. */
#define KH_M02_SEL_SPAWN_WORKER     0x09379E54  /* TODO: identify. Signed: 154639956. Target selection for sWorker creation. */

/* -------------------------------------------------------------------------
 * Mission 02 KAS message labels (sent via kasfMsgSend — original strings unknown)
 * ------------------------------------------------------------------------- */
#define KH_M02_MSG_DOCK_COMPLETE    0x0FA02D75  /* TODO: identify. Signed: 262155637. Sent to Objectives team when sMothershipDockingBay dock flag is true. */
#define KH_M02_MSG_UNDOCK           0x09502D35  /* TODO: identify. Signed: 156249397. Sent to FleetIntel team when docking bay module departs. */
#define KH_M02_MSG_FLOW_START       0x093100E4  /* TODO: identify. Signed: 154206436. Sent to FleetIntel after init-complete milestone (start mission flow). */
#define KH_M02_MSG_MICRO_DOCKED     0x00479E14  /* TODO: identify. Signed: 4693524.   Sent to FleetIntel and Objectives when sMothershipMicro docks. */
#define KH_M02_MSG_PATROL_SPOTTED   0x0D8BD352  /* TODO: identify. Signed: 227267410. Sent to FleetIntel when pirate patrol first enters sensor range. */
#define KH_M02_MSG_MICRO_QUEUED     0x067AE1D2  /* TODO: identify. Signed: 108716498. Sent to FleetIntel when sMothershipMicro enters build queue. */

#endif /* KASHASHES_H */
