/*=============================================================================
    Name    : Battle.c
    Purpose : Code to evaluate and update battle chatter.

    Created 12/14/1998 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include <math.h>
#include "types.h"
#include "debug.h"
#include "memory.h"
#include "spaceobj.h"
#include "shipdefs.h"
#include "statscript.h"
#include "vector.h"
#include "matrix.h"
#include "prim2d.h"
#include "camera.h"
#include "soundEvent.h"
#include "randy.h"
#include "mainrgn.h"
#include "fastmath.h"
#include "alliance.h"
#include "universe.h"
#include "flightman.h"
#include "ping.h"
#include "gun.h"
#include "select.h"
#include "singleplayer.h"
#include "blobs.h"
#include "battle.h"

// HW1 ship type alias: Mothership maps to sMothership in Cataclysm's ship type system
#define Mothership sMothership

/*=============================================================================
    Data:
=============================================================================*/
#if BATTLE_TEXT
#if 0
char *battleString_Fighter_ManeuverGrunts[] =
{
    "...oh boy...",
    "...go...go...go...",
    "...come on...come on...",
    "...that's it, that's it...",
    "...yeah!...",
    "...[stomach punch grunt]...",
    "...[stomach twisting groan]...",
    "...[steady heavy breathing]...",
    "...[shaky fast breathing]...",
    "...[hyperventilating rapid breathing]...",
    "...[G-force induced moan - short]...",
    "...[G-force induced moan - long]...",
    NULL
};
#endif
char *battleString_WingmanChased[] =
{
    "There's one on your tail. Check six. Check six.",
    "You're being followed.",
    "You've got one closing behind.",
    "You've gotta tail.",
    "You've picked one up.",
    "There's one on you.",
    "Break! Break!",
    NULL
};
char *battleString_WingmanChased_Rsp[] =
{
    "He's on tight.",
    "...Can't break it...",
    "...I see 'em. Cover me.",
    "Roger that trailer, I see him.",
    "Get him off me!",
    "Gotcha.",
    NULL
};
char *battleString_WingmanHit[] =
{
    "Wing hit!",
    "...Cover me dammit.",
    "I've been hit.",
    "I'm hit, but not too bad.",
    "Okay, I'm still with you.",
    "(to self) Damn! Okay, alright. I'm alright...",
    NULL
};
char *battleString_WingmanHit_Rsp[] =
{
    "Alright, hang in there.",
    "Looks OK.",
    "Hold formation. Let's keep it together now...",
    "You're looking fine.",
    "Hang on ... I'm coming in.",
    "On my way.",
    NULL
};
char *battleString_WingmanLethal[] =
{
    "Can't stabilize... I'm... <static>",
    "I'm losing the... <static>",
    "(to self) (a big scream cut short)... <static>",
    "(to self) (breathes in hard and fast)... <static>",
    "aaargh (effort) ... <static>",
    "ahhhhhhhh (burning) ... <static>",
    "hunngh! (depressurization) .. <static>",
    "I'm folding! ...<static>",
    "Mayday, I got no...<static>",
    NULL
};
char *battleString_WingmanDies[] =
{
    "Wingman down",
    "I've lost my wingman",
    "Wing down.",
    "Lost the wing.",
    NULL
};
char *battleString_LeaderChased[] =
{
    "Group leader, you've got a tail.",
    "Group lead, you've picked up a trailer.",
    "Hold on group leader, I'm coming.",
    NULL
};
char *battleString_Wingman[] =
{
    "Wing, watch your course.",
    "You're sliding off-line. Correct.",
    "You're off target vector... compensate now.",
    "Stay within targeting threshold.",
    "Counter that angle, wing.",
    "We're losing wing lock... reset.",
    "You're too close, watch the gap.",
    "Close the gap your too far!",
    "Wingman, tighten it up.",
    "Stay on target.",
    NULL
};
char *battleString_PositiveBattle[] =
{
    "Nice move.",
    "I saw it, yeah, very smooth.",
    "OK, looking fine over here.",
    "Cover me.",
    "I got you.",
    "I'm with you.",
    "That's affirmative, I have it here.",
    "We're on it.",
    "Confirmed visual. Locked.",
    "Target assignments confirmed.",
    "Formation secured.",
    "Coordinates entered.",
    "Got it.",
    "Once again, the way it's done.",
    "Boom. Right there.",
    "Oh yeah.",
    "We're in. Yes.",
    NULL
};
char *battleString_NegativeBattle[] =
{
    "Redirect fire! Redirect fire!",
    "What's happening?",
    "I can't see! Scopes are dead!",
    "Negative! Left! Go left!",
    "Say again?",
    "Behind you!",
    "What's the position?",
    "Can't lock in!",
    "Alarms! I got alarms!",
    "Sound off!",
    "Dammit!",
    "Fall back... ",
    "Respond! ",
    "Talk to me...",
    "Hold on...",
    "What are you doing?",
    "Come on!",
    "Let's move!",
    "Go around!",
    "No! Negative!",
    "Regroup! Regroup!",
    "This ain't happening, man.",
    NULL
};
char *battleString_NeutralBattle[] =
{
    "Tactical even. It's flat.",
    "We're this far, push it home!",
    "Lay it on! Over the top!",
    "Lead your targets...",
    "Collision avoidance... lock it up.",
    "It's pitched. Nudge it! Nudge it our way!",
    "Battle stats level.",
    "Who has it?",
    "Watch the targets, not tactical.",
    "Pick them up fast.",
    "Plug in! Plug in!",
    "We're holding positions...",
    "Flanking ships stay close.",
    NULL
};
char *battleString_FriendlyFire[] =
{
    "Check your fire!  Check your fire!",
    "Check your targets!  Friendly fire!",
    "Watch it!",
    "Watch that friendly fire!",
    NULL
};
char *battleString_Kamikaze[] =
{

    "[increasing pitch] ...mmmmmmmnnahh!!!",
    "....nnnnn....aaahh!",
    "..aaaarrrgh!",
    "FOR THE HOMEWORLD!!!!",
    "...uh....understood...  (hesitant)",
    "EAT THIS!!!",

    NULL
};
char *battleString_StartBattleDisadvantaged[] =
{

    "We're outnumbered.",
    "I have a bad feeling about this...",
    "This isn't lookin' good Fleet...",
    "(to self quietly) ..ohh man...",

    NULL
};
char *battleString_StartBattleDisadvantaged_Rsp[] =
{
    "Yeah, but we can do it.",
    "Just keep it tight.",
    "We've got our orders.",
    "Shut it, we're goin' in.",
    NULL
};
char *battleString_StartBattleFair[] =
{

    "Watch your six.",
    "Pick your targets.",
    "It looks even, let's do it.",

    NULL
};
char *battleString_StartBattleFair_Rsp[] =
{

    "Maintain positions.",
    "Right behind you.",
    "Acknowledged. On your wing.",
    "Coming in at <NumberList>.",
    "Look for the trap.",

    NULL
};
char *battleString_StartBattleAdvantaged[] =
{

    "We caught 'em napping.",
    "I almost feel bad about this.",
    "It's in our favor.",
    "Keep one eye on the clock.",
    "Looks like we got the upper hand.",
    "Let's make this quick and painless.",

    NULL
};
char *battleString_StartBattleAdvantaged_Rsp[] =
{

    "Stay on top of them.",
    "Don't let them regroup.",
    "It's not over yet.",
    "Get on their tails, and stay there.",
    "Copy that ... let's take 'em down one by one.",
    "They're up against the wall.",

    NULL
};
char *battleString_Retreat[] =
{

    "All units retreat, repeat, all units retreat.",
    "Get out of there now! Abort. Abort.",
    "Abort mission, repeat, abort mission.",
    "Retreat. Retreat. Abort all offensive action.",
    "Begin abort sequence.",
    "Abort code <NumberList>.",
    "Break! Break!",
    "Disengage immediately.",

    NULL
};
char *battleString_Retreat_Rsp[] =
{

    "<Roger>. On our way.",
    "<Roger>. We're outta here!",
    "<Roger>. It's about time!",
    "Abort code received and confirmed.",
    "<Roger>. Disengaging.",
    "Let's get the hell outta Dodge!",
    "Wing leaders... head count, report all losses.",

    NULL
};
char *battleString_WinningBattle[] =
{

    "We're on top of it.",
    "This is looking good. Stay tight.",
    "It's ours so far. Watch the counter.",
    "Spreading at <Degrees>. Good work.",
    "Just a little more and it's over.",
    "Tactical full, looks good.",
    "Let's wrap this up.",

    NULL
};
char *battleString_WinningBattle_Rsp[] =
{

    "Yup.",
    "Roger that sweep.",
    "Holding attack pattern <Letter>.",
    "That's it. There we go.",
    "They're going down.",

    NULL
};
char *battleString_KickingButt[] =
{

    "Let's end this.",
    "It's over. Let's clean it up.",
    "We have them.",
    "This is finished.",
    "Pick off the stragglers.",
    "OK that'll do it.",
    "The enemy is failing.",

    NULL
};
char *battleString_KickingButt_Rsp[] =
{
    "Roger that. Cleaning it up.",
    "Affirmative. It'll be no problem.",
    "No contest.",
    "They're almost down.",
    "Watch for debris.",
    "Reset targeting and inertials.",
    "Let's do them a favour.",
    "They're routed.",
    NULL
};
char *battleString_BattleWon[] =
{

    "The enemy is defeated.",
    "We've won. We've done it.",
    "Stand down from combat stations.",
    "The area is secure.",
    "Mission accomplished. Standing by.",
    "Enemy neutralized, awaiting further instructions.",
    "OK. That's it.  Nice work.",

    NULL
};
char *battleString_LosingBattle[] =
{
    "We have a situation developing here.",
    "Requesting back up... repeat... requesting back up.",
    "Switch to battle code <NumberList>.",
    "The enemy has it.",
    "We're taking heavy losses.",
    "We are being overwhelmed.",
    "We're losing grip here.",
    "Head count, head count! Report in!",
    "Links failing, we have losses.",
    "Tactical is thinning out here, we've got a problem.",
    "... ah ... this is not looking good Fleet.",
    NULL
};
char *battleString_LosingBattle_Rsp[] =
{

    "OK, keep it together now.",
    "Return the fight to them.",
    "Roger that contact, we see it.",
    "Hang in there. Hold your positions.",
    "Losses confirmed.",
    "Get it together and fight back.",
    "Assume defense lock <Letter>.",
    "Open up gaps. Look for the gaps.",
    "<Roger>. Stay close. Keep it close.",

    NULL
};
char *battleString_LosingBadly[] =
{

    "Situation critical Fleet.",
    "We're getting our asses kicked here Fleet!",
    "We're down to <Number> units.",
    "We're pinned. We need relief.",
    "Hits! We got hits all over here!",
    "Contact everywhere! Heavy contact!",
    "Is anyone responding here?",
    "Revise head counts! Updates!",
    "Where are the links?",
    "Tactical is almost clear. It's getting a bit lonely out here...",
    "Now it's really not looking good Fleet...",
    "Check your fire! Check your fire!",

    NULL
};
char *battleString_LosingBadly_Rsp[] =
{
    "Emergency procedures.",
    "Rally! Rally!",
    "Get us out of here!",
    "Maintain positions!",
    "We're running out of time.",
    "Can't ... shake 'em... Can't...",
    "It's too much! I can't.... damn!",
    "Come ON!  Let's GO!",
    "If you're gonna act, act now.",
    "This is murder out here.",
    NULL
};
#endif //BATTLE_TEXT

//master list of chatter events
battleevent battleChatterEvent[] =
{
/*BCE_Wingman,                  */ newEvent("CHAT_Fighter_Wingman", 0x10008C4, battleString_Wingman),
/*BCE_PositiveBattle            */ newEvent("CHAT_Intel_Grp_PositiveBattle", 0x100207F, battleString_PositiveBattle),
/*BCE_PositiveBattle_More       */ newEvent("CHAT_Ship_Grp_PositiveBattleMore", 0x1000816, battleString_PositiveBattle),
/*BCE_NegativeBattle            */ newEvent("CHAT_Intel_Grp_NegativeBattle", 0x1002080, battleString_NegativeBattle),
/*BCE_NegativeBattle_More       */ newEvent("CHAT_Ship_Grp_NegativeBattleMore", 0x1000817, battleString_NegativeBattle),
/*BCE_NegativeBattle_More2      */ newEvent("CHAT_Intel_Grp_NegativeBattleMore2", 0x100206F, battleString_NegativeBattle),
/*BCE_NeutralBattle,            */ newEvent("CHAT_Intel_Grp_NeutralBattle", 0x1002081, battleString_NeutralBattle),
/*BCE_NeutralBattle_More,       */ newEvent("CHAT_Intel_Grp_NeutralBattleMore", 0x1002071, battleString_NeutralBattle),
/*BCE_FriendlyFire,             */ newEvent("CHAT_Intel_Grp_FriendlyFire", 0x1002082, battleString_FriendlyFire),
/*BCE_GroupFriendlyFire,        */ newEvent("STAT_MoShip_AssGrp_FriendlyFire", 0x200105F, battleString_FriendlyFire),
/*BCE_WingmanChased,            */ newEvent("STAT_Fighter_WingmanChased", 0x20008BE, battleString_WingmanChased),
/*BCE_WingmanChasedR            */ newEvent("STAT_Fighter_WingmanChasedRsp", 0x20008BF, battleString_WingmanChased_Rsp),
/*BCE_WingmanHit,               */ newEvent("STAT_Fighter_WingmanHit", 0x20008C0, battleString_WingmanHit),
/*BCE_WingmanHitRsp             */ newEvent("STAT_Fighter_WingmanHitRsp", 0x20008C1, battleString_WingmanHit_Rsp),
/*BCE_WingmanLethal,            */ newEvent("STAT_Spec_ShipBeingBeastified", 0x10008A1, NULL),
/*BCE_WingmanDies,              */ newEvent("STAT_Fighter_WingmanDies", 0x20008C2, battleString_WingmanDies),
/*BCE_LeaderChased,             */ newEvent("STAT_Fighter_LeaderChased", 0x20008C3, battleString_LeaderChased),
/*BCE_Kamikaze,                 */ newEvent("COMM_LInt_Kamikaze", 0x40008C5, battleString_Kamikaze),
/*BCE_StartBattleDisadvant,     */ newEvent("STAT_Intel_Grp_StartBattleDisadvant", 0x2002077, battleString_StartBattleDisadvantaged),
/*BCE_StartBattleDisadvantRsp,  */ newEvent("STAT_Intel_Grp_StartBattleDisadvantRsp", 0x200206C, battleString_StartBattleDisadvantaged_Rsp),
/*BCE_StartBattleFair,          */ newEvent("STAT_Intel_Grp_StartBattleFair", 0x2002078, battleString_StartBattleFair),
/*BCE_StartBattleFairRsp,       */ newEvent("STAT_Intel_Grp_StartBattleFairRsp", 0x200206D, battleString_StartBattleFair_Rsp),
/*BCE_StartBattleAdvantaged,    */ newEvent("STAT_Intel_Grp_StartBattleAdvantaged", 0x2002079, battleString_StartBattleAdvantaged),
/*BCE_StartBattleAdvantagedRsp, */ newEvent("STAT_Intel_Grp_StartBattleAdvantagedRsp", 0x200206E, battleString_StartBattleAdvantaged_Rsp),
/*BCE_Retreat,                  */ newEvent("COMM_Intel_Grp_Retreat", 0x4002072, battleString_Retreat),
/*BCE_RetreatRsp,               */ newEvent("COMM_Intel_Grp_RetreatRsp", 0x400206B, battleString_Retreat_Rsp),
/*BCE_WinningBattle,            */ newEvent("STAT_Intel_Grp_WinningBattle", 0x200207A, battleString_WinningBattle),
/*BCE_WinningBattleRsp,         */ newEvent("STAT_Ship_Grp_WinningBattleRsp", 0x1000812, battleString_WinningBattle_Rsp),
/*BCE_KickingButt,              */ newEvent("STAT_Intel_Grp_KickingButt", 0x200207B, battleString_KickingButt),
/*BCE_KickingButtRsp,           */ newEvent("STAT_Ship_Grp_KickingButtRsp", 0x1000813, battleString_KickingButt_Rsp),
/*BCE_BattleWon,                */ newEvent("STAT_Intel_Grp_BattleWon", 0x200207C, battleString_BattleWon),
/*BCE_GroupVictory,             */ newEvent("STAT_Intel_AssGrp_Victory", 0x2002075, battleString_BattleWon),
/*BCE_LosingBattle,             */ newEvent("STAT_Intel_Grp_LosingBattle", 0x200207D, battleString_LosingBattle),
/*BCE_LosingBattle_More,        */ newEvent("STAT_Intel_Grp_LosingBattleMore", 0x2002070, battleString_LosingBattle),
/*BCE_LosingBattleRsp,          */ newEvent("STAT_Ship_Grp_LosingBattleRsp", 0x1000814, battleString_LosingBattle_Rsp),
/*BCE_LosingBadly,              */ newEvent("STAT_Intel_Grp_LosingBadly", 0x200207E, battleString_LosingBadly),
/*BCE_LosingBadly_More,         */ newEvent("STAT_Ship_Grp_LosingBadlyMore", 0x1000819, battleString_LosingBadly),
/*BCE_LosingBadlyRsp,           */ newEvent("STAT_Ship_Grp_LosingBadlyRsp", 0x1000815, battleString_LosingBadly_Rsp),
/*COMM_CannotComply,            */ newEvent("COMM_CannotComply", 0x4000810, NULL),
/*STAT_Strike_StuckInGrav,      */ newEvent("STAT_MoShip_GravWellHasShips", 0x2001051, NULL),
/*COMM_Grav_Off,                */ newEvent("COMM_Spec_GravWellOff", 0x40008A0, NULL),
/*STAT_Strike_AttackComplete,   */ newEvent("STAT_MoShip_StrikeAttackComplete", 0x200105A, NULL),
/*COMM_Kamikaze_NoTargets,      */ newEvent("COMM_Kamikaze_NoTargets", 0x4000809, NULL),
/*COMM_HVette_BurstAttack,      */ newEvent("COMM_Spec_FighterBurstAttack", 0x400089A, NULL),
/*COMM_MLVette_ForceDrop,       */ newEvent("COMM_Spec_MineLayerForceDrop", 0x400089B, NULL),
/*COMM_MissleDest_VolleyAttack, */ newEvent("STAT_Spec_HiveSwarmersUnderway", 0x20008AB, NULL),
/*STAT_Grp_RetreatSuccessful,   */ newEvent("STAT_Intel_Grp_RetreatSuccessful", 0x2002083, NULL),
/*STAT_AssGrp_RetreatSuccessful,*/ newEvent("STAT_MoShip_AssGrp_RetreatSuccessful", 0x200105D, NULL),
/*STAT_Grp_EnemyRetreated,      */ newEvent("STAT_Intel_Grp_EnemyRetreated", 0x2002084, NULL),
/*STAT_AssGrp_EnemyRetreated,   */ newEvent("STAT_MoShip_AssGrp_EnemyRetreated", 0x200105E, NULL),
/*STAT_SCVette_DroppingOff,     */ newEvent("STAT_Spec_SalvageDropOff", 0x2000897, NULL),
/*STAT_F_ShipStolen,            */ newEvent("STAT_MoShip_ShipStolen", 0x2001041, NULL),
/*STAT_Grp_InMineField,         */ newEvent("STAT_Intel_Grp_InMineField", 0x2002087, NULL),
/*STAT_AssGrp_InMineField,      */ newEvent("STAT_MoShip_AssGrp_InMineField", 0x2001060, NULL),
/*BCE_STAT_F_CloakedShipsDete   */ newEvent("STAT_MoShip_CloakedShipsDetected", 0x200104C, NULL),
/*BCE_STAT_Grp_AttackRetaliat   */ newEvent("STAT_Intel_Grp_AttackRetaliate", 0x2002086, NULL),
/*BCE_STAT_Grp_AttackRetaliat,  */ newEvent("STAT_Ship_Grp_AttackRetaliateRepeat", 0x1000818, NULL),
/*BCE_STAT_AssGrp_AttackRetal   */ newEvent("STAT_Intel_AssGrp_AttackRetaliate", 0x2002073, NULL),
/*BCE_STAT_AssGrp_AttackRetaleat*/ newEvent("STAT_Intel_AssGrp_AttackRetaliateRepeat", 0x2002074, NULL),
/* BCE_AttackFromAlly,          */ newEvent("STAT_MoShip_UnderAttackFromAlly", 0x2001069, NULL),
/* BCE_GroupAttackFromAlly,     */ newEvent("STAT_Intel_Grp_AttackFromAlly", 0x2002085, NULL),
/* BCE_ProbeDetected,           */ newEvent("STAT_MoShip_EnemyProbeDetected", 0x200104B, NULL),
/* BCE_GravwellDetected,        */ newEvent("STAT_MoShip_GravWellDetected", 0x2001050, NULL),
/* BCE_CapitalShipDamaged,      */ newEvent("STAT_Ship_Damaged", 0x100080B, NULL),
/* BCE_CapitalShipDies,         */ newEvent("STAT_Cap_Dies", 0x100080C, NULL),
/* BCE_CapitalShipDiesReport,   */ newEvent("STAT_MoShip_ShipDiesReport", 0x2001065, NULL),
/* BCE_HyperspaceAbandoned      */ newEvent("STAT_Spec_RammerRamComplete", 0x20008AE, NULL),
/* BCE_MothershipLargeAttack,   */ newEvent("STAT_MoShip_MoShipLargeAttack", 0x2001059, NULL),
/* BCE_MothershipLargeAttackResp*/ newEvent("STAT_MoShip_MoShipUnderLargeAttackRsp", 0x2001057, NULL),
/* BCE_CapitalShipLaunched,     */ newEvent("STAT_MoShip_CapShipLaunched", 0x200103F, NULL),
/* BCE_CapitalShipWelcomed,     */ newEvent("STAT_MoShip_CapShipWelcomed", 0x200101E, NULL),
/* BCE_CapitalDamagedSpecific,  */ newEvent("STAT_MoShip_DamagedSpecificShip", 0x2001066, NULL),
/* BCE_CapitalShipDocked,       */ newEvent("STAT_Cap_ShipDocked", 0x81000804, NULL),
/* BCE_DockingChatter,          */ newEvent("STAT_MoShip_EMPPulseDetected", 0x200104F, NULL),
/* BCE_DockingChatterMore,      */ newEvent("CHAT_Docking_More", 0x81000804, NULL),
/* BCE_CloakingOn,              */ newEvent("STAT_Spec_WorkerFull", 0x20008A4, NULL),
/* BCE_CloakingOnResponse,      */ newEvent("STAT_Cloak_CloakingOn_Resp", 0x0, NULL),
/* BCE_CloakingInsufficientPower*/ newEvent("COMM_Cloak_InsufficientPower", 0x0, NULL),
/* BCE_CloakingPowerLow,        */ newEvent("STAT_Spec_CloakCloakPowerLow", 0x200088C, NULL),
/* BCE_Decloaking,              */ newEvent("COMM_Spec_CloakDecloak", 0x400088D, NULL),
/* BCE_SalvageTargetAcquired,   */ newEvent("STAT_Spec_SalvageTargetAcquired", 0x2000899, NULL),
/* BCE_RepairStarted,           */ newEvent("STAT_RepVette_StartedRepairs", 0x0, NULL),
/* BCE_RepairFinished,          */ newEvent("STAT_Spec_FinishedRepairs", 0x2000893, NULL),
/* BCE_FuelLow,                 */ newEvent("STAT_Spec_ResourcesTransferred", 0x200088E, NULL),
/* BCE_OutOfFuel,               */ newEvent("COMM_Spec_RammerStartingRam", 0x40008AD, NULL),
/* BCE_COMM_F_ScuttleReaffirm   */ newEvent("COMM_F_ScuttleReaffirm", 0x400080A, NULL),
/* BCE_COMM_F_HyperspaceDetected*/ newEvent("COMM_MoShip_HyperspaceDetected", 0x400104E, NULL),
/* BCE_STAT_F_ProxSensor_Detection*/ newEvent("STAT_MoShip_ProxSensorDetection", 0x0, NULL),

/*COMM_F_Grp_Enemy_Fighters_Decl*/ //newEvent("COMM_F_Grp_Enemy_Fighters_Decloaking", COMM_F_Grp_Enemy_Fighters_Decloaking , NULL),
/*COMM_F_Cloakgen_Decloaking,   */ //newEvent("COMM_F_Cloakgen_Decloaking", COMM_F_Cloakgen_Decloaking , NULL),
#if 0
/**/newEvent(),
#endif
};

//some chatter is class-specific
#define BCD_LaunchWelcome       0x01
#define BCD_Damaged             0x02
ubyte battleClassDependencies[NUM_CLASSES] =
{
    /* CLASS_Mothership   */ 0,
    /* CLASS_HeavyCruiser */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Carrier      */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Destroyer    */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Frigate      */ BCD_Damaged | BCD_LaunchWelcome,
    /* CLASS_Corvette     */ 0,
    /* CLASS_Fighter      */ 0,
    /* CLASS_Resource     */ BCD_Damaged,
    /* CLASS_NonCombat    */ 0
};

/*-----------------------------------------------------------------------------
    Battle chatter tweaks and script parsing table
-----------------------------------------------------------------------------*/
//number of times to try to find a ship to do the talking
sdword batMouthpieceTries = BAT_MouthpieceTries;

//coefficients for computing the current battle standing number (BSN)
real32 batStandingFriendlyPower =   BAT_StandingFriendlyPower;
real32 batStandingFriendlyValue =   BAT_StandingFriendlyValue;
real32 batStandingEnemyPower    =   BAT_StandingEnemyPower;
real32 batStandingEnemyValue    =   BAT_StandingEnemyValue;
/*
real32 batFriendlyLossPower     =   BAT_FriendlyLossPower;
real32 batFriendlyLossValue     =   BAT_FriendlyLossValue;
real32 batEnemyLossPower        =   BAT_EnemyLossPower;
real32 batEnemyLossValue        =   BAT_EnemyLossValue;
*/
real32 batExpLossDevalue        =   BAT_ExpLossDevalue;

//values of BSN for various statuses (BSN > x ?)
real32 batBSNKickingButt        =   BAT_BSNKickingButt;
real32 batBSNWinning            =   BAT_BSNWinning;
real32 batBSNNeutral            =   BAT_BSNNeutral;
real32 batBSNLosing             =   BAT_BSNLosing;

//coefficients for determining if a ship is really on your tail
real32 batBogeyAspect           =   BAT_BogeyAspect;

//retreat parameters
real32 batRetreatModifier       =   BAT_RetreatModifier;

//global parameter for controling how much chatter there is
udword batGlobalFrequencyModifier = BAT_GlobalFrequencyModifier;
real32 batGlobalFrequencyMin     = BAT_GlobalFrequencyMin;  //min/max in log base 2 space
real32 batGlobalFrequencyMax     = BAT_GlobalFrequencyMax;

//fleet-wide chatter throttle globals
real32 batFleetChatterTime          = 0.0f;     //last time any fleet chatter event was played
battleevent *batLastFleetChatterEvent = NULL;   //last fleet chatter event played

//battle chatter tuning pointer
#if BATTLE_TUNING
battleevent *batChatterToTune   = NULL;
#endif

//script-parsing table and function prototypes
void batMaxDistanceSetCB(char *directory,char *field,void *dataToFillIn);
void batDistanceExponentSetCB(char *directory,char *field,void *dataToFillIn);
void batRandomWeightSetCB(char *directory,char *field,void *dataToFillIn);
void batMinWavelengthSetCB(char *directory,char *field,void *dataToFillIn);
void batWavelengthExponentSetCB(char *directory,char *field,void *dataToFillIn);
void batMinRepeatProximitySetCB(char *directory,char *field,void *dataToFillIn);
void batRepeatProximityExponentSetCB(char *directory,char *field,void *dataToFillIn);
void batTuneEventSet(char *directory,char *field,void *dataToFillIn);
void batGlobalFrequencyModifierSet(char *directory,char *field,void *dataToFillIn);
scriptEntry battleTweaks[] =
{
    {"randomWeight",    batRandomWeightSetCB, NULL},
    {"maxDistance",     batMaxDistanceSetCB, NULL},
    {"expDistance",     batDistanceExponentSetCB, NULL},
    {"minWavelength",   batMinWavelengthSetCB, NULL},
    {"expWavelength",   batWavelengthExponentSetCB, NULL},
    {"minRepeatProximity",batMinRepeatProximitySetCB, NULL},
    {"expRepeatProximity",batRepeatProximityExponentSetCB, NULL},
#if BATTLE_TUNING
    {"TuneOnlyThisEvent",batTuneEventSet, NULL},
#endif
    {"batGlobalFrequencyModifier", batGlobalFrequencyModifierSet,NULL},
    makeEntry(batMouthpieceTries , scriptSetSdwordCB),
    makeEntry(batStandingFriendlyPower , scriptSetReal32CB),
    makeEntry(batStandingFriendlyValue , scriptSetReal32CB),
    makeEntry(batStandingEnemyPower    , scriptSetReal32CB),
    makeEntry(batStandingEnemyValue    , scriptSetReal32CB),
    makeEntry(batBSNKickingButt , scriptSetReal32CB),
    makeEntry(batBSNWinning     , scriptSetReal32CB),
    makeEntry(batBSNNeutral     , scriptSetReal32CB),
    makeEntry(batBSNLosing      , scriptSetReal32CB),
    makeEntry(batBogeyAspect    , scriptSetReal32CB),
    makeEntry(batRetreatModifier, scriptSetReal32CB),
    makeEntry(batGlobalFrequencyMin, scriptSetReal32CB),
    makeEntry(batGlobalFrequencyMax, scriptSetReal32CB),
    endEntry
};

/*=============================================================================
    Functions:
=============================================================================*/
/*-----------------------------------------------------------------------------
    Name        : battleChatterEventFromName
    Description : Find a battle chatter event by name reference
    Inputs      :
    Outputs     : value - value to read from the string after the name
    Return      :
----------------------------------------------------------------------------*/
battleevent *battleChatterEventFromName(char *name, double *value)
{
    sdword index, nScanned;
    char *string;
    real32 floatValue;

    string = strtok(name, BAT_Delimiters);
    for (index = 0; index < BCE_LastBCE; index++)
    {
        if (!_stricmp(name, battleChatterEvent[index].eventName))
        {
            //return(&battleChatterEvent[index]);
            goto foundName;
        }
    }
#if BATTLE_ERROR_CHECKING
    dbgFatalf(DBG_Loc, "Unable to find battle chatter event '%s'.", name);
#endif
foundName:
    string = strtok(NULL, BAT_Delimiters);
    nScanned = sscanf(string, "%f", &floatValue);
    dbgAssert(nScanned == 1);
    *value = (double)floatValue;
    return(&battleChatterEvent[index]);
}

/*-----------------------------------------------------------------------------
    Script-parsing callback functions
-----------------------------------------------------------------------------*/
void batRandomWeightSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value >= 0.0f);
    chat->randomWeight = (udword)(value * (real32)BAT_RandomTotal / (real32)PNG_TaskPeriod);
}
void batMaxDistanceSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->maxDistance = (double)value;
}
void batDistanceExponentSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value >= 0.0f);
    chat->expDistance = (double)value;
}
void batMinWavelengthSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->minWavelength = (double)value;
}
void batWavelengthExponentSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->expWavelength = (double)value;
}
void batMinRepeatProximitySetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->minRepeatProximity = (double)value;
}
void batRepeatProximityExponentSetCB(char *directory,char *field,void *dataToFillIn)
{
    battleevent *chat;
    double value;

    chat = battleChatterEventFromName(field, &value);
    dbgAssert(value > 0.0f);
    chat->expRepeatProximity = (double)value;
}
#if BATTLE_TUNING
void batTuneEventSet(char *directory,char *field,void *dataToFillIn)
{
    double value;
    batChatterToTune = battleChatterEventFromName(field, &value);
}
#endif
void batGlobalFrequencyModifierSet(char *directory,char *field,void *dataToFillIn)
{
    real32 value;
    sdword nScanned;

    nScanned = sscanf(field, "%f", &value);
    dbgAssert(nScanned == 1);
    batGlobalFrequencyModifier = (udword)(value * BIT8);
}

/*-----------------------------------------------------------------------------
    Name        : battleChatterStartup
    Description : Startup the battle chatter module by reading in BattleChatter.script
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterStartup(void)
{
    scriptSet(NULL, "BattleChatter.script", battleTweaks);  //scan in all the chatter parameters
}

/*-----------------------------------------------------------------------------
    Name        : battleChatterShutdown
    Description : Shut down the battle chatter module
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterShutdown(void)
{
}

/*-----------------------------------------------------------------------------
    Name        : battleSpeechText
    Description : Print out some text of what the battle chatter _might_ say,
                    not what it actually says.
    Inputs      : eventNum - what event is being called
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
#if BATTLE_TEXT
void battleSpeechText(sdword eventNum, char **texts)
{
    sdword nTexts, index;

    for (nTexts = 0; texts[nTexts] != NULL; nTexts++)
    {                                                       //count number of text strings
        ;
    }
    index = batRandom() % nTexts;
    dbgMessagef("\nBattle chatter event %d - '%s'.", eventNum, texts[index]);
}
#endif

/*-----------------------------------------------------------------------------
    Name        : battleChatterAttempt
    Description : Attempt to make a given chatter event chat
    Inputs      : linkTo - handle of the previously queued event that this one is to follow
                  event - event to attempt to speak
                  ship - ship that'll do the speaking
    Outputs     :
    Return      : handle of queued event, or ERROR (-1) if none queued
----------------------------------------------------------------------------*/
sdword battleChatterAttempt(sdword linkTo, sdword event, Ship *ship, sdword variable)
{
    vector dist;
    double distance, result;
    battleevent *chat;
    udword randomWeight;

    chat = &battleChatterEvent[event];
//	if (variable == SOUND_EVENT_DEFAULT)
//		variable = 0;
#if BATTLE_TUNING
    if (batChatterToTune != NULL && batChatterToTune != chat)
    {                                                       //if we're tuning only chatter event
        return(ERROR);                                      //no jouez-vous
    }
#endif
    randomWeight = chat->randomWeight * batGlobalFrequencyModifier / BIT8;
    if (batRandom() < randomWeight)
    {                                                       //if it passes random weight test
        vecSub(dist, mrCamera->eyeposition, ship->posinfo.position);
        distance = (double)vecMagnitudeSquared(dist);       //get length of vector
        distance = fmathSqrtDouble(distance);
        if (distance < chat->maxDistance)
        {                                                   //if it's not too far away from camera to hear
            result = 1.0 - distance / chat->maxDistance;
            if (chat->expDistance != 1.0)
            {
                result = pow(result, chat->expDistance);    //compute point on probability curve
            }
            if ((double)batRandom() < result * (double)BAT_RandomTotal)
            {                                               //if passes camera proximity test
#if BATTLE_TEXT
                if(chat->texts != NULL)
                {
                    battleSpeechText(chat->eventNumber, chat->texts);
                }
#endif
                chat->lastTimeSpoken = universe.totaltimeelapsed;
                chat->lastPlaceSpoken = ship->posinfo.position;
                if (chat->eventNumber == 0x100080C)
                {
                    return(speechEventQueue(NULL, chat->eventNumber,
                        variable, SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT,
                        universe.curPlayerIndex, linkTo,
                        (real32)SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT));
                }
                return(speechEventQueue(ship, chat->eventNumber,
                    variable, SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT,
                    universe.curPlayerIndex, linkTo,
                    (real32)SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT));
                return(ERROR);
                //... actual speech event
            }
#if BATTLE_VERBOSE_LEVEL >= 2
            else
            {
                dbgMessagef("\n...Event %d above distance curve.", chat->eventNumber);
            }
#endif
        }
#if BATTLE_VERBOSE_LEVEL >= 2
        else
        {
            dbgMessagef("\n...Event %d too far away.", chat->eventNumber);
        }
#endif
    }
#if BATTLE_VERBOSE_LEVEL >= 2
    else
    {
        dbgMessagef("\n...Event %d failed randomWeight check.", chat->eventNumber);
    }
#endif
    return(ERROR);
}

/*-----------------------------------------------------------------------------
    Name        : battleCanChatterAtThisTime
    Description : See if a particular event will work at this time (i.e. it
                    has not been used around here recently)
    Inputs      : event - what event to consider
                  ship - what ship we would like to play it on.  If NULL,
                  camera's position will be used for location information if needed.
    Outputs     :
    Return      : TRUE if it's OK to chatter at this time.
----------------------------------------------------------------------------*/
bool32 battleCanChatterAtThisTime(sdword event, Ship *ship)
{
    battleevent *chat;
    double timeSinceLastSpoken, result, distance;
    vector diff;

    dbgAssert(event >= 0 && event < BCE_LastBCE);
    chat = &battleChatterEvent[event];
#if BATTLE_TUNING
    if (batChatterToTune != NULL && batChatterToTune != chat)
    {                                                       //if we're tuning only one chatter event
        return(ERROR);                                      //no jouez-vous
    }
#endif
    timeSinceLastSpoken = (double)(universe.totaltimeelapsed - chat->lastTimeSpoken);

    dbgAssert(chat->minWavelength > 0.0);
    if (timeSinceLastSpoken > chat->minWavelength)
    {                                                       //if it hasn't been spoken in a coon's age
        return(TRUE);
    }
    result = timeSinceLastSpoken / chat->minWavelength;
    if (chat->expWavelength != 1.0f)
    {                                                       //find proper spot on probability curve
        result = pow(result, chat->expWavelength);
    }
    if ((double)batRandom() >= result * (double)BAT_RandomTotal)
    {
#if BATTLE_VERBOSE_LEVEL >= 2
        dbgMessagef("\n...Event %d above wavelength curve.", chat->eventNumber);
#endif
        return(FALSE);
    }
    if (chat->minRepeatProximity > 0.0f)
    {                                                       //if we should consider location of last time this event was spoken
        if (ship != NULL)
        {
            vecSub(diff, ship->posinfo.position, chat->lastPlaceSpoken);
        }
        else
        {
            vecSub(diff, mrCamera->eyeposition, chat->lastPlaceSpoken);
        }
        distance = fmathSqrtDouble((double)vecMagnitudeSquared(diff));
        if (distance < chat->minRepeatProximity)
        {
            result = distance / chat->minRepeatProximity;
            if (chat->expRepeatProximity != 1.0f)
            {
                result = pow(result, chat->expRepeatProximity);
            }
            if ((double)batRandom() >= result * (double)BAT_RandomTotal)
            {
#if BATTLE_VERBOSE_LEVEL >= 2
                dbgMessagef("\n...Event %d above proximity curve.", chat->eventNumber);
#endif
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

/*-----------------------------------------------------------------------------
    Name        : battleChatterFleetAttempt
    Description : Tries to play a fleet command battle chatter event.
    Inputs      : linkTo - what speech event to link to, or SOUND_EVENT_DEFAULT
                  event - what event to play
                  variable - variable for speech event or SOUND_EVENT_DEFAULT
                  where - position info so that we can use proximity tuning.
                    May be NULL
    Outputs     :
    Return      : handle of queued speech event, or ERROR
    Note        : This function should only be called for the correct player as
                    there is no ship parameter to determine what player this
                    event belongs to.
----------------------------------------------------------------------------*/
sdword battleChatterFleetAttempt(sdword linkTo, sdword event, sdword variable, vector *where)
{
    battleevent *chat;
    double timeSinceLastSpoken, result;
    udword randomWeight;
    vector diff;
    real32 distance;

    dbgAssert(event >= 0 && event < BCE_LastBCE);
    chat = &battleChatterEvent[event];
#if BATTLE_TUNING
    if (batChatterToTune != NULL && batChatterToTune != chat)
    {                                                       //if we're tuning only one chatter event
        return(ERROR);                                      //no jouez-vous
    }
#endif
    timeSinceLastSpoken = (double)(universe.totaltimeelapsed - chat->lastTimeSpoken);

    dbgAssert(chat->minWavelength > 0.0);
    if (timeSinceLastSpoken > chat->minWavelength)
    {                                                       //if it hasn't been spoken in a coon's age
        goto readyToPlay;
    }
    result = timeSinceLastSpoken / chat->minWavelength;
    if (chat->expWavelength != 1.0f)
    {                                                       //find proper spot on probability curve
        result = pow(result, chat->expWavelength);
    }
    if ((double)batRandom() >= result * (double)BAT_RandomTotal)
    {
#if BATTLE_VERBOSE_LEVEL >= 2
        dbgMessagef("\n...Event %d above wavelength curve.", chat->eventNumber);
#endif
        return(ERROR);
    }
readyToPlay:
    if (chat->minRepeatProximity > 0.0f && where != NULL)
    {                                                       //if we should consider location of last time this event was spoken
        vecSub(diff, *where, chat->lastPlaceSpoken);
        distance = fmathSqrtDouble((double)vecMagnitudeSquared(diff));
        if (distance < chat->minRepeatProximity)
        {
            result = distance / chat->minRepeatProximity;
            if (chat->expRepeatProximity != 1.0f)
            {
                result = pow(result, chat->expRepeatProximity);
            }
            if ((double)batRandom() >= result * (double)BAT_RandomTotal)
            {
#if BATTLE_VERBOSE_LEVEL >= 2
                dbgMessagef("\n...Event %d above proximity curve.", chat->eventNumber);
#endif
                return(ERROR);
            }
        }
    }

    randomWeight = chat->randomWeight * batGlobalFrequencyModifier / BIT8;
    if (batRandom() < randomWeight)
    {                                                       //if it passes random weight test
#if BATTLE_TEXT
        if(chat->texts != NULL)
        {
            battleSpeechText(chat->eventNumber, chat->texts);
        }
#endif
        if (where)
        {                                                   //remember where it was spoken
            chat->lastPlaceSpoken = *where;
        }
        chat->lastTimeSpoken = universe.totaltimeelapsed;
        return(speechEventQueue(NULL, chat->eventNumber,
            variable, SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT,
            universe.curPlayerIndex, linkTo,
            (real32)SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT));
        return(ERROR);
    }
#if BATTLE_VERBOSE_LEVEL >= 2
    else
    {
        dbgMessagef("\n...Event %d failed randomWeight check.", chat->eventNumber);
    }
#endif
    return(ERROR);

}

/*-----------------------------------------------------------------------------
    Name        : battleMouthPieceFind
    Description : Find a ship to do some talking.  The ship needs to be one of
                    the player's ships.
    Inputs      : ships - list of ships to look through
                  numShips - number of ships in list
    Outputs     :
    Return      : Pointer to one of the players' ships, or NULL if one could not be found.
----------------------------------------------------------------------------*/
Ship *battleMouthPieceFind(Ship **ships, sdword numShips)
{
    Ship *mouthPiece;
    sdword tries = 0;

    do
    {
        tries++;
        if (tries > batMouthpieceTries)
        {
            return(NULL);
        }
        mouthPiece = ships[batRandom() % numShips];
    }
    while ( mouthPiece->playerowner != universe.curPlayerPtr);

    dbgAssert(mouthPiece->objtype == OBJ_ShipType);
    return(mouthPiece);
}

/*-----------------------------------------------------------------------------
    Name        : battleMouthPieceFindAnother
    Description : Same as above, but it will exclude the specified ship from
                    the search.
    Inputs      : firstMouthPiece - the ship that we don't want to be talking
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
Ship *battleMouthPieceFindAnother(Ship *firstMouthPiece, Ship **ships, sdword numShips)
{
    Ship *mouthPiece;
    sdword tries = 0;

    if (numShips == 0)
    {
        return(NULL);
    }
    do
    {
        mouthPiece = ships[batRandom() % numShips];
        tries++;
        if (tries > batMouthpieceTries)
        {
            return(NULL);
        }
    }
    while ( mouthPiece->playerowner != universe.curPlayerPtr && mouthPiece != firstMouthPiece);

    dbgAssert(mouthPiece->objtype == OBJ_ShipType);
    return(mouthPiece);
}

/*-----------------------------------------------------------------------------
    Name        : battleShipOnMyTail
    Description : See if a ship is on the tail of another ship
    Inputs      : meShip - ship being followed
                  badGuyShip - ship [possibly] following us
    Outputs     :
    Return      : TRUE if the ship is on the tail
----------------------------------------------------------------------------*/
bool32 battleShipOnMyTail(Ship *meShip, Ship *badGuyShip)
{
    vector temp, bogeyPosition;
    matrix transpose;

    matCopyAndTranspose(&meShip->rotinfo.coordsys, &transpose);//transpose of coordsys
    vecSub(temp, badGuyShip->posinfo.position, meShip->posinfo.position);//vector from us to them
    matMultiplyMatByVec(&bogeyPosition, &transpose, &temp); //badguy's position in our local space
    if (bogeyPosition.z < 0.0f)
    {                                           //if they're behind us
        //sort of a "Manhattan bogey attitude" check
        if (abs(bogeyPosition.x) + abs(bogeyPosition.y) < -bogeyPosition.z * batBogeyAspect)
        {                                       //and they're on our 6
            vecNegate(temp);                    //vector from them to us
            matCopyAndTranspose(&badGuyShip->rotinfo.coordsys, &transpose);//transpose of coordsys
            matMultiplyMatByVec(&bogeyPosition, &transpose, &temp); //our position in badguy's local space
            if (bogeyPosition.z > 0.0f)
            {                                   //if they're ahead of us
                if (abs(bogeyPosition.x) + abs(bogeyPosition.y) < bogeyPosition.z * batBogeyAspect)
                {                               //and we're in their sights
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

/*-----------------------------------------------------------------------------
    Name        : battleAccountForLosses
    Description : Account for the loss of a ship on the compution of future BSN's
    Inputs      : ship - what ship was lost
                  battlePing - what ping it was lost from
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void battleAccountForLosses(Ship *ship, battleping *battlePing)
{
    real32 lossPower, lossValue;
    real32 timeSinceLoss;

    lossPower = gunShipFirePower(ship->staticinfo, ship->tacticstype);
    lossValue = (real32)ship->staticinfo->svFirePower;

    if (ship->playerowner == universe.curPlayerPtr)
    {                                                       //if it's one of our ships we're losing
        if (battlePing->friendlyValueRecentlyLost != 0.0f)
        {
            timeSinceLoss = universe.totaltimeelapsed - battlePing->whenFriendliesLost;
            if (timeSinceLoss != 0.0f)
            {
                battlePing->friendlyFirepowerRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
                battlePing->friendlyValueRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            }
        }
        battlePing->friendlyFirepowerRecentlyLost += lossPower;
        battlePing->friendlyValueRecentlyLost += lossValue;
        battlePing->whenFriendliesLost = universe.totaltimeelapsed;
    }
    else
    {                                                       //else it's an enemy that was lost
        if (battlePing->enemyValueRecentlyLost != 0.0f)
        {
            timeSinceLoss = universe.totaltimeelapsed - battlePing->whenEnemiesLost;
            if (timeSinceLoss != 0.0f)
            {
                battlePing->enemyFirepowerRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
                battlePing->enemyValueRecentlyLost /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            }
        }
        battlePing->enemyFirepowerRecentlyLost += lossPower;
        battlePing->enemyValueRecentlyLost += lossValue;
        battlePing->whenEnemiesLost = universe.totaltimeelapsed;
    }
}

#define thisPing ((ping *)voidPing)
/*-----------------------------------------------------------------------------
    Name        : battlePingEvaluateNoFriendlies
    Description : Handle a battle ping with no friendlies.
    Inputs      : thisPing - the ping who's evaluation function we are being
                    called from.
                  battlePing - the battle ping we are evaluating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battlePingEvaluateNoFriendlies(void *voidPing, battleping *battlePing)
{
    ;//nothing to play in this case
}

/*-----------------------------------------------------------------------------
    Name        : battlePingEvaluateNoEnemies
    Description : Same as regular evaluation, except there are no enemies.
                    We've probably just won a battle, or maybe it's friendly
                    or allied fire.
    Inputs      : thisPing - the ping who's evaluation function we are being
                    called from.
                  battlePing - the battle ping we are evaluating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battlePingEvaluateNoEnemies(void *voidPing, battleping *battlePing)
{
    sdword index;
    Ship *ship;
    Ship *victoryShip;
    bool32 queuedEvent = FALSE;
    sdword groupNumber;
    sdword tries;

    //do a per-ship scan for some events
    for (index = 0; index < battlePing->nShips; index++)
    {
        ship = battlePing->ship[index];
        dbgAssert(ship->objtype == OBJ_ShipType);
        dbgAssert(ship->playerowner != NULL);
        if (ship->staticinfo->shipclass && ship->playerowner == universe.curPlayerPtr)
        {                                                   //if this is a friendly ship (thus we can hear it speak)
            //Are my guys someone shooting at me?
            if (bitTest(ship->chatterBits, BCB_FriendlyFire))
            {
                groupNumber = selHotKeyGroupNumberTest(ship);
                if (groupNumber != SEL_InvalidHotKey)
                {
                    battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_GroupFriendlyFire, groupNumber, NULL);
                }
                else
                {
                    battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_FriendlyFire, SOUND_EVENT_DEFAULT, NULL);
                }
                bitClear(ship->chatterBits, BCB_FriendlyFire);
                queuedEvent = TRUE;
            }
            //Are my allies someone shooting at me?
            if (bitTest(ship->chatterBits, BCB_AlliedFire))
            {
                if ((ship->specialFlags2 & SPECIAL_2_DisabledState) == 0 &&
                    (!battleCanChatterAtThisTime(BCE_CapitalShipDamaged, ship) ||
                     battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDamaged, ship, ship->shiptype) == ERROR) &&
                    battleCanChatterAtThisTime(BCE_CapitalDamagedSpecific, ship))
                {
                    if (PrinceOfSpace)
                    {
                        speechEventQueue(NULL, 0x200135E, SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT, 0,
                            SOUND_EVENT_DEFAULT, SOUND_EVENT_DEFAULT, 5.0f, SOUND_EVENT_DEFAULT);
                    }
                    else
                    {
                        battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalDamagedSpecific, ship->shiptype, &ship->posinfo.position);
                    }
                }
                bitClear(ship->chatterBits, BCB_AlliedFire);
                queuedEvent = TRUE;
            }
        }
    }

    if ((!singlePlayerGame || singlePlayerGameInfo.currentMission != 13) && !queuedEvent && battlePing->enemyValueRecentlyLost != 0.0f)
    {
        victoryShip = NULL;
        if (batMouthpieceTries >= 1)
        {
            tries = 1;
            do
            {
                victoryShip = battlePing->ship[batRandom() % battlePing->nShips];
                if (victoryShip->playerowner == universe.curPlayerPtr)
                {
                    break;
                }
                tries++;
            }
            while (tries <= batMouthpieceTries);

            if (tries > batMouthpieceTries)
            {
                victoryShip = NULL;
            }
        }

        if (battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_BattleWon, SOUND_EVENT_DEFAULT, NULL) == ERROR && victoryShip != NULL)
        {
            groupNumber = selHotKeyGroupNumberTest(victoryShip);
            if (groupNumber != SEL_InvalidHotKey)
            {
                battleChatterFleetAttempt(SOUND_EVENT_DEFAULT, BCE_GroupVictory, groupNumber, &victoryShip->posinfo.position);
            }
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : battlePingEvaluate
    Description : Called from the corresponding ping's evaluation function,
                    evaluate the battle ping and maybe make it say something.
    Inputs      : thisPing - the ping who's evaluation function we are being
                    called from.
                  battlePing - the battle ping we are evaluating
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battlePingEvaluate(void *voidPing, battleping *battlePing)
{
    sdword index;
    Ship *ship;
    real32 enemyPower = 0.0f, friendlyPower = 0.0f;
    real32 enemyValue = 0.0f, friendlyValue = 0.0f;
    real32 enemyStanding, friendlyStanding, battleStanding;
    real32 timeSinceLoss, lossPower, lossValue;
    Ship *mouthPiece = NULL, *mothership;
    sdword handle;
    CommandToDo *command;
    SelectCommand *ships;
    vector diff;
    udword chances, saveChance[4];

#if BATTLE_VERBOSE_LEVEL >= 3
    dbgMessagef("\nEvaluating ping 0x%x", battlePing);
#endif
    //do a per-ship scan for some events
    for (index = 0; index < battlePing->nShips; index++)
    {
        ship = battlePing->ship[index];
        dbgAssert(ship->objtype == OBJ_ShipType);
        dbgAssert(ship->playerowner != NULL);
        if (ship->flags & SOF_Dead)
        {
            continue;   // dead ships shouldn't interfere
        }
        if (ship->shiptype == Mothership)
        {                                                   //don't even have the mothership talking
            continue;
        }
        if (/*ship->shiprace > R2 || */ship->playerowner == universe.curPlayerPtr)
        {                                                   //if this is a friendly ship (thus we can hear it speak)
            //count up the power and cost of your ships
            friendlyPower += gunShipFirePower(ship->staticinfo, ship->tacticstype);
//            friendlyValue += (real32)ship->staticinfo->buildCost * ship->health / ship->staticinfo->maxhealth;
            friendlyValue += (real32)ship->staticinfo->buildCost;

            // do some high-g grunting if flight maneuvers are going on
//            if (ship->flightman > FLIGHTMAN_DONOTHING)      //!!! is this the proper flightman check?
//            {
//                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_ManeuverGrunts, ship, SOUND_EVENT_DEFAULT);
//            }
#if BATTLE_VERBOSE_LEVEL >= 2
//            else
//            {
//                dbgMessagef("\n...No flightman");
//            }
#endif
            //leaders may make snaps at their wingmen
            if (ship->tacticstype == Evasive)
            {                                               //if in evasive tactics
                if (ship->attackvars.myWingmanIs != NULL)
                {                                           //if ship has a wingman
                    if (battleCanChatterAtThisTime(BCE_Wingman, ship))
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Wingman, ship, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            //wingman (or another party in the battle) may tell this ship that they have a tail
            if (ship->recentlyFiredUpon)
            {                                               //if someone has been attacking
                dbgAssert(ship->firingAtUs != NULL);
                if (battleShipOnMyTail(ship, ship->firingAtUs))
                {
                    handle = ERROR;
                    if (ship->tacticstype == Evasive && ship->attackvars.myWingmanIs != NULL)
                    {                                       //the wingman should say it if possible
                        mouthPiece = ship->attackvars.myWingmanIs;
                        if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_LeaderChased, mouthPiece))
                        {
                            handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LeaderChased, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                    else
                    {                                       //otherwise anybody else could say it
                        mouthPiece = battleMouthPieceFindAnother(ship, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_WingmanChased, mouthPiece))
                        {                                       //if there is someone to say there is a bogey
                            handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanChased, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                    if (handle != ERROR)
                    {
                        if (battleCanChatterAtThisTime(BCE_WingmanChasedRsp, ship))
                        {
                            battleChatterAttempt(handle, BCE_WingmanChasedRsp, ship, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
            //say that a ship is recently hit, someone may respond
            if (ship->recentlyAttacked > BAT_ReallyRecentAttack)
            {                                               //if attacked quite recently
                handle = ERROR;
                if (battleCanChatterAtThisTime(BCE_WingmanHit, ship))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanHit, ship, SOUND_EVENT_DEFAULT);
                }
                if (handle != ERROR)
                {
                    if (ship->tacticstype == Evasive && ship->attackvars.myWingmanIs != NULL)
                    {                                       //the wingman should say it if possible
                        mouthPiece = ship->attackvars.myWingmanIs;
                    }
                    else
                    {                                       //otherwise anybody else could say it
                        mouthPiece = battleMouthPieceFindAnother(ship, battlePing->ship, battlePing->nShips);
                    }
                    if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_WingmanHitRsp, mouthPiece))
                    {
                        battleChatterAttempt(handle, BCE_WingmanHitRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            //I've lost my wingman... bummer
            if (bitTest(ship->chatterBits, BCB_WingmanLost))
            {
                if (battleCanChatterAtThisTime(BCE_WingmanDies, ship))
                {
                    battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WingmanDies, ship, SOUND_EVENT_DEFAULT);
                }
                bitClear(ship->chatterBits, BCB_WingmanLost);
            }
            //am I really hurtin'?
            if (bitTest(ship->chatterBits, BCB_DamageInTheRed))
            {
                handle = ERROR;
                if (bitTest(battleClassDependencies[ship->staticinfo->shipclass], BCD_Damaged))
                {                                           //if this class is allowed to complain about damage
                    if (battleCanChatterAtThisTime(BCE_CapitalShipDamaged, ship))
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipDamaged, ship, ship->staticinfo->shipclass);
                    }
                    if (handle == ERROR && battleCanChatterAtThisTime(BCE_CapitalDamagedSpecific, ship))
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalDamagedSpecific, ship, ship->shiptype);
                    }
                }
                bitClear(ship->chatterBits, BCB_DamageInTheRed);
            }
            //see if we're leaving the battle area
            command = ship->command;                        //get the ship's command
            if (command != NULL)
            {
                switch (command->ordertype.order)
                {
                    case COMMAND_MOVE:
                        //... see if he's moving out of the ping
                        clGetMoveDestination(command, &diff);
                        vecSub(diff, thisPing->centre, diff);
                        if (fsqrt(vecMagnitudeSquared(diff)) > battlePing->radius * batRetreatModifier)
                        {                                   //if this ship is moving out of the ping
                            if (battleCanChatterAtThisTime(BCE_Retreat, ship))
                            {                               //if we can speak of retreat
                                handle = ERROR;
                                if (battleCanChatterAtThisTime(BCE_Retreat, ship))
                                {
                                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_Retreat, ship, SOUND_EVENT_DEFAULT);
                                }
                                if (handle != ERROR)
                                {
                                    mouthPiece = battleMouthPieceFindAnother(ship, battlePing->ship, battlePing->nShips);
                                    if (mouthPiece != NULL)
                                    {
                                        battleChatterAttempt(handle, BCE_RetreatRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                                    }
                                }
                            }
                        }
                        break;
/*
                    case COMMAND_DOCK:
                        //... see if the dock ship is far enough away
                        break;
*/
                }
            }
            //... other allied-ships tests
        }
        else
        {                                                   //not a player ship
            if (allianceIsShipAlly(ship, universe.curPlayerPtr))
            {                                               //!!! consider your allies in the battle
                friendlyPower += gunShipFirePower(ship->staticinfo, ship->tacticstype);
//                friendlyValue += (real32)ship->staticinfo->buildCost * ship->health / ship->staticinfo->maxhealth;
                friendlyValue += (real32)ship->staticinfo->buildCost;
            }
            else
            {
                enemyPower += gunShipFirePower(ship->staticinfo, ship->tacticstype);
//                enemyValue += (real32)ship->staticinfo->buildCost * ship->health / ship->staticinfo->maxhealth;
                enemyValue += (real32)ship->staticinfo->buildCost;
            }
        }
        //... other non-allied ship tests
    }

    //compute the battle status number (BSN)
    friendlyStanding = friendlyPower * batStandingFriendlyPower + friendlyValue * batStandingFriendlyValue;
    enemyStanding = enemyPower * batStandingEnemyPower + enemyValue * batStandingEnemyValue;
    //... adjust these standings by the recent losses
    //save the chances of some events because they will have their randomWeight's modified
    saveChance[0] = battleChatterEvent[BCE_LosingBattle].randomWeight;
    saveChance[1] = battleChatterEvent[BCE_LosingBadly].randomWeight;
    saveChance[2] = battleChatterEvent[BCE_WinningBattle].randomWeight;
    saveChance[3] = battleChatterEvent[BCE_KickingButt].randomWeight;
    if (battlePing->friendlyValueRecentlyLost)
    {
        timeSinceLoss = universe.totaltimeelapsed - battlePing->whenFriendliesLost;
        lossPower = battlePing->friendlyFirepowerRecentlyLost;
        lossValue = battlePing->friendlyValueRecentlyLost;
        if (timeSinceLoss != 0.0f)
        {
            lossPower /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            lossValue /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
        }
        if (friendlyValue != 0.0f)
        {
            chances = (udword)max(((lossValue / friendlyValue * batStandingFriendlyValue +
                         lossPower / friendlyPower * batStandingFriendlyValue) * (real32)BAT_RandomTotal), (real32)BAT_RandomTotal);
        }
        else
        {
            chances = BAT_RandomTotal;
        }
        battleChatterEvent[BCE_LosingBattle].randomWeight = chances;
        battleChatterEvent[BCE_LosingBadly].randomWeight = chances;
    }
    if (battlePing->enemyValueRecentlyLost)
    {
        timeSinceLoss = universe.totaltimeelapsed - battlePing->whenEnemiesLost;
        lossPower = battlePing->enemyFirepowerRecentlyLost;
        lossValue = battlePing->enemyValueRecentlyLost;
        if (timeSinceLoss != 0.0f)
        {
            lossPower /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
            lossValue /= (real32)pow((double)timeSinceLoss, (double)batExpLossDevalue);
        }
        if (enemyValue != 0.0f)
        {
            chances = (udword)max(((lossValue / enemyValue * batStandingEnemyValue +
                         lossPower / enemyPower * batStandingEnemyValue) * (real32)BAT_RandomTotal), (real32)BAT_RandomTotal);
        }
        else
        {
            chances = BAT_RandomTotal;
        }
        battleChatterEvent[BCE_WinningBattle].randomWeight = chances;
        battleChatterEvent[BCE_KickingButt].randomWeight = chances;
    }

    if (enemyStanding > 0.0f)
    {
        battleStanding = friendlyStanding / enemyStanding;
    }
    else
    {
        battleStanding = -1.0f;
        //... play the battle won event here
    }

    mouthPiece = battleMouthPieceFind(battlePing->ship, battlePing->nShips);
    if (mouthPiece != NULL)
    {                                                       //found someone to do the talking; now make him say something
        if (battleStanding > batBSNKickingButt)
        {                                                   //> kickingbutt
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            if (battleCanChatterAtThisTime(BCE_StartBattleAdvantaged, mouthPiece))
            {
                if (battleCanChatterAtThisTime(BCE_StartBattleAdvantaged, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleAdvantaged, mouthPiece, SOUND_EVENT_DEFAULT);
                }
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleAdvantagedRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                //...kicking butt speech event
                if (battleCanChatterAtThisTime(BCE_KickingButt, mouthPiece))
                {
                    handle = ERROR;
                    if (battleCanChatterAtThisTime(BCE_KickingButt, mouthPiece))
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_KickingButt, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_KickingButtRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
        }
        else if (battleStanding > batBSNWinning)
        {                                                   //winning..kickingButt
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            if (battleCanChatterAtThisTime(BCE_StartBattleAdvantaged, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleAdvantaged, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleAdvantagedRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                handle = ERROR;
                if (battleCanChatterAtThisTime(BCE_WinningBattle, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_WinningBattle, mouthPiece, SOUND_EVENT_DEFAULT);
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_WinningBattleRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
                if (handle == ERROR)
                {
                    if (battleCanChatterAtThisTime(BCE_PositiveBattle, mouthPiece))
                    {
                        if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_PositiveBattle, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                        {
                            battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_PositiveBattle_More, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
        }
        else if (battleStanding > batBSNNeutral)
        {                                                   //neutral..winning
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            if (battleCanChatterAtThisTime(BCE_StartBattleFair, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleFair, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleFairRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                if (battleCanChatterAtThisTime(BCE_NeutralBattle, mouthPiece))
                {
                    if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NeutralBattle, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                    {
                        battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NeutralBattle_More, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
        }
        else if (battleStanding > batBSNLosing)
        {                                                   //losing..neutral
            handle = ERROR;
            mothership = universe.curPlayerPtr->PlayerMothership;
            if (mothership != NULL && selShipInSelection(battlePing->ship, battlePing->nShips, mothership))
            {
                //ships = mothership->collMyBlob->blobShips;
                //dbgAssert(ships != NULL);
                //if (selShipInSelection(ships->ShipPtr, ships->numShips, mothership))
                //{                                               //if the mothership is involved in this battle
                    if (battleCanChatterAtThisTime(BCE_MothershipLargeAttack, mothership))
                    {                                           //say the mothership is under heavy fire
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_MothershipLargeAttack, mothership, SOUND_EVENT_DEFAULT);
                        if (handle != ERROR && battleCanChatterAtThisTime(BCE_MothershipLargeAttackResp, mouthPiece))
                        {
                            battleChatterAttempt(handle, BCE_MothershipLargeAttackResp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                //}
            }
            //see if the battle is just starting, and assess it if it is
            if (handle == ERROR && battleCanChatterAtThisTime(BCE_StartBattleDisadvant, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleDisadvant, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleDisadvantRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                handle = ERROR;
                if (battleCanChatterAtThisTime(BCE_LosingBattle, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBattle, mouthPiece, SOUND_EVENT_DEFAULT);
                    if (handle == ERROR)
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBattle_More, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_LosingBattleRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
                if (handle == ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL && battleCanChatterAtThisTime(BCE_NegativeBattle, mouthPiece))
                    {
                        if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NegativeBattle, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                        {
                            if (battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NegativeBattle_More, mouthPiece, SOUND_EVENT_DEFAULT) == ERROR)
                            {
                                battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_NegativeBattle_More2, mouthPiece, SOUND_EVENT_DEFAULT);
                            }
                        }
                    }
                }
            }
        }
        else if (battleStanding > 0.0f)
        {                                                   //< losing (losing badly)
            //see if the battle is just starting, and assess it if it is
            handle = ERROR;
            mothership = universe.curPlayerPtr->PlayerMothership;
            if (mothership != NULL && selShipInSelection(battlePing->ship, battlePing->nShips, mothership))
            {
                //ships = mothership->collMyBlob->blobShips;
                //dbgAssert(ships != NULL);
                //if (selShipInSelection(ships->ShipPtr, ships->numShips, mothership))
                //{                                               //if the mothership is involved in this battle
                    if (battleCanChatterAtThisTime(BCE_MothershipLargeAttack, mothership))
                    {                                           //say the mothership is under heavy fire
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_MothershipLargeAttack, mothership, SOUND_EVENT_DEFAULT);
                        if (handle != ERROR && battleCanChatterAtThisTime(BCE_MothershipLargeAttackResp, mouthPiece))
                        {
                            battleChatterAttempt(handle, BCE_MothershipLargeAttackResp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                //}
            }
            if (handle == ERROR && battleCanChatterAtThisTime(BCE_StartBattleDisadvant, mouthPiece))
            {
                handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_StartBattleDisadvant, mouthPiece, SOUND_EVENT_DEFAULT);
                if (handle != ERROR)
                {
                    mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                    if (mouthPiece != NULL)
                    {
                        battleChatterAttempt(handle, BCE_StartBattleDisadvantRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                }
            }
            if (handle == ERROR)
            {                                               //battle is not just starting
                //...losing badly speech events
                if (battleCanChatterAtThisTime(BCE_LosingBadly, mouthPiece))
                {
                    handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBadly, mouthPiece, SOUND_EVENT_DEFAULT);
                    if (handle == ERROR)
                    {
                        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_LosingBadly_More, mouthPiece, SOUND_EVENT_DEFAULT);
                    }
                    if (handle != ERROR)
                    {
                        mouthPiece = battleMouthPieceFindAnother(mouthPiece, battlePing->ship, battlePing->nShips);
                        if (mouthPiece != NULL)
                        {
                            battleChatterAttempt(handle, BCE_LosingBadlyRsp, mouthPiece, SOUND_EVENT_DEFAULT);
                        }
                    }
                }
            }
        }
    }
    //restore these previously saved randomWeight values
    battleChatterEvent[BCE_LosingBattle].randomWeight  = saveChance[0];
    battleChatterEvent[BCE_LosingBadly].randomWeight   = saveChance[1];
    battleChatterEvent[BCE_WinningBattle].randomWeight = saveChance[2];
    battleChatterEvent[BCE_KickingButt].randomWeight   = saveChance[3];
}
#undef thisPing

/*-----------------------------------------------------------------------------
    Name        : battleChatterFrequencySet
    Description : Set the battle chatter frequency
    Inputs      : freq100 - value from 0 to 100 (compatible with FE sliders)
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterFrequencySet(sdword freq100)
{
    batGlobalFrequencyModifier = (udword)(((real32)freq100 * BIT8) / 100.0f);
}

/*-----------------------------------------------------------------------------
    Name        : battleShipDyingWithTimeToScream
    Description : Attempt to play one or more death speech events for a given ship's death.
    Inputs      : ship - ship that is dying
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
extern real32 dword_A6AE4C; // sound system camera-proximity metric, updated per frame by soundEventUpdate
void battleShipDyingWithTimeToScream(Ship *ship)
{
    sdword handle;

    switch (ship->staticinfo->shipclass)
    {
        case CLASS_HeavyCruiser:
        case CLASS_Carrier:
        case CLASS_Destroyer:
        case CLASS_Frigate:
        case CLASS_Resource:
            if (ship->specialFlags2 & 0x100000)
                return;
            if (!battleCanChatterAtThisTime(BCE_CapitalShipDies, ship))
                return;
            if (mrRenderMainScreen && dword_A6AE4C >= 5000.0f)
            {
                speechEventQueue(0, 0x100080C, ship->shiprace, -1, 0,
                    ship->playerowner->playerIndex, -1, 5.0f, -1);
                return;
            }
            handle = battleChatterAttempt(-1, BCE_CapitalShipDies, ship, ship->shiprace);
            if (handle != -1)
                battleChatterFleetAttempt(handle, BCE_CapitalShipDiesReport, ship->shiprace, &ship->posinfo.position);
            return;

        case CLASS_Corvette:
        case CLASS_Fighter:
        case CLASS_Micro:
            if (ship->specialFlags2 & 0x100000)
                return;
            if (battleCanChatterAtThisTime(BCE_CapitalShipDies, ship))
            {
                if (battleChatterAttempt(-1, BCE_CapitalShipDies, ship, ship->shiprace) != -1)
                    return;
            }
            if (!battleCanChatterAtThisTime(BCE_WingmanDies, ship))
                return;
            battleChatterAttempt(-1, BCE_WingmanDies, ship, ship->shiprace);
            return;

        default:
            return;
    }
}

/*-----------------------------------------------------------------------------
    Name        : battleLaunchWelcomeExchange
    Description : Play the lauch/welcome speech event exchange on launch of a
                    new super-capital ship.
    Inputs      : ship - ship that is just launched
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleLaunchWelcomeExchange(Ship *ship)
{
    sdword handle = ERROR;
    Ship *mouthpiece;
    SelectCommand *ships;

    if (!bitTest(battleClassDependencies[ship->staticinfo->shipclass], BCD_LaunchWelcome))
    {
         return;
    }
    if (battleCanChatterAtThisTime(BCE_CapitalShipLaunched, ship))
    {
        handle = battleChatterAttempt(SOUND_EVENT_DEFAULT, BCE_CapitalShipLaunched, ship, ship->shiptype);
    }
    if (handle != ERROR && ship->collMyBlob != NULL)
    {
        ships = ship->collMyBlob->blobShips;
        dbgAssert(ships != NULL);
        mouthpiece = battleMouthPieceFindAnother(ship, ships->ShipPtr, ships->numShips);
        if (mouthpiece != NULL)
        {
            battleChatterAttempt(handle, BCE_CapitalShipWelcomed, mouthpiece, mouthpiece->shiptype);
        }
    }
}



#if 0  // Cat2: battlePingEvaluate live code differs from binary
void __cdecl battlePingEvaluate(void *thisPing, float *battlePing)
{
  int v2; // ecx
  BOOL v3; // ebp
  int v4; // edi
  float *v5; // edx
  int v6; // esi
  int v7; // eax
  int v8; // edi
  int v9; // eax
  unsigned int v10; // ebx
  int v11; // ebp
  int v12; // edi
  int v13; // edi
  int v14; // ebx
  int v15; // edi
  unsigned int v16; // ebp
  int v17; // edi
  float v18; // edx
  int v19; // ecx
  int v20; // eax
  double v21; // st7
  double v22; // st6
  double v23; // st5
  int v24; // eax
  bool32 IsObjFriendly; // eax
  double v27; // st6
  double v28; // st5
  double v29; // st5
  unsigned int v30; // edi
  int v31; // ebp
  int v32; // ebx
  int v33; // eax
  int v34; // ebx
  unsigned int v35; // ebp
  int v36; // edi
  int v37; // edx
  unsigned int v38; // ebp
  int v39; // edi
  int v40; // edx
  int v41; // eax
  int v42; // eax
  Ship *PlayerMothership; // edi
  int v44; // ebp
  int v45; // eax
  unsigned int v46; // ebp
  int v47; // edi
  int v48; // edx
  int v49; // eax
  unsigned int v50; // ebp
  int v51; // ebx
  int v52; // edi
  int v53; // ebx
  unsigned int v54; // ebp
  int v55; // edi
  int v56; // edx
  struct ShipStaticInfo *v57; // [esp+0h] [ebp-40h]
  sdword v58; // [esp+4h] [ebp-3Ch]
  float v59; // [esp+18h] [ebp-28h]
  float v60; // [esp+1Ch] [ebp-24h]
  float v61; // [esp+20h] [ebp-20h]
  float *v62; // [esp+24h] [ebp-1Ch]
  float v63; // [esp+28h] [ebp-18h]
  int v64; // [esp+2Ch] [ebp-14h]
  float v65; // [esp+30h] [ebp-10h]
  float v66; // [esp+34h] [ebp-Ch]
  float v67; // [esp+38h] [ebp-8h]
  float v68; // [esp+3Ch] [ebp-4h]
  _DWORD *thisPinga; // [esp+44h] [ebp+4h]
  float battlePinga; // [esp+48h] [ebp+8h]
  float *battlePingb; // [esp+48h] [ebp+8h]
  float *battlePingc; // [esp+48h] [ebp+8h]
  float *battlePingd; // [esp+48h] [ebp+8h]

  v63 = 0.0;
  v60 = 0.0;
  v61 = 0.0;
  v59 = 0.0;
  if ( battlePing[10] + flt_72A558 > universe.totaltimeelapsed )
    return;
  v2 = *((_DWORD *)battlePing + 3);
  v3 = 0;
  v4 = 0;
  battlePing[10] = universe.totaltimeelapsed;
  if ( v2 <= 0 )
    return;
  v5 = battlePing + 11;
  while ( !v3 )
  {
    if ( *(Player **)(*(_DWORD *)v5 + 812) == universe.curPlayerPtr )
      v3 = (*(_DWORD *)(*(_DWORD *)v5 + 460) & 0x100000) == 0;
    ++v4;
    ++v5;
    if ( v4 >= v2 )
    {
      if ( !v3 )
        return;
      break;
    }
  }
  v64 = 0;
  v62 = battlePing + 11;
  do
  {
    v6 = *(_DWORD *)v62;
    if ( (*(_DWORD *)(*(_DWORD *)v62 + 20) & 0x800) != 0 )
      goto LABEL_71;
    v7 = *(_DWORD *)(v6 + 32);
    if ( !*(_DWORD *)(v7 + 488) )
      goto LABEL_71;
    if ( *(Player **)(v6 + 812) != universe.curPlayerPtr )
    {
      IsObjFriendly = univIsObjFriendly((struct SpaceObj *)v6, universe.curPlayerPtr);
      v58 = *(_DWORD *)(v6 + 696);
      v57 = *(struct ShipStaticInfo **)(v6 + 32);
      if ( IsObjFriendly )
      {
        v60 = gunShipFirePower(v57, v58) + v60;
        v59 = (double)*(int *)(*(_DWORD *)(v6 + 32) + 568) + v59;
      }
      else
      {
        v63 = gunShipFirePower(v57, v58) + v63;
        v61 = (double)*(int *)(*(_DWORD *)(v6 + 32) + 568) + v61;
      }
      goto LABEL_71;
    }
    v60 = gunShipFirePower((struct ShipStaticInfo *)v7, *(_DWORD *)(v6 + 696)) + v60;
    v59 = (double)*(int *)(*(_DWORD *)(v6 + 32) + 568) + v59;
    if ( !*(_DWORD *)(v6 + 696) && *(_DWORD *)(v6 + 896) && battleCanChatterAtThisTime(0, (float *)v6) )
      battleChatterAttempt(-1, 0, v6, -1);
    if ( *(_BYTE *)(v6 + 859) && battleShipOnMyTail((Ship *)v6, *(Ship **)(v6 + 840)) )
    {
      if ( *(_DWORD *)(v6 + 696) || (v8 = *(_DWORD *)(v6 + 896)) == 0 )
      {
        v10 = *((_DWORD *)battlePing + 3);
        if ( !v10 )
          goto LABEL_35;
        v11 = 1;
        v12 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v10 + 11]);
        if ( dword_75071C < 1 )
          goto LABEL_35;
        while ( *(Player **)(v12 + 812) != universe.curPlayerPtr && v12 != v6 )
        {
          ++v11;
          v12 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v10 + 11]);
          if ( v11 > dword_75071C )
            goto LABEL_35;
        }
        if ( !v12 || !battleCanChatterAtThisTime(10, (float *)v12) )
          goto LABEL_35;
        v9 = battleChatterAttempt(-1, 10, v12, -1);
      }
      else
      {
        if ( !battleCanChatterAtThisTime(16, *(float **)(v6 + 896)) )
          goto LABEL_35;
        v9 = battleChatterAttempt(-1, 16, v8, -1);
      }
      v13 = v9;
      if ( v9 != -1 && battleCanChatterAtThisTime(11, (float *)v6) )
        battleChatterAttempt(v13, 11, v6, *(_DWORD *)(v6 + 688));
    }
LABEL_35:
    if ( *(_BYTE *)(v6 + 858) > 0x48u )
    {
      if ( battleCanChatterAtThisTime(12, (float *)v6) )
      {
        v14 = battleChatterAttempt(-1, 12, v6, -1);
        if ( v14 != -1 )
        {
          if ( !*(_DWORD *)(v6 + 696) )
          {
            v15 = *(_DWORD *)(v6 + 896);
            if ( v15 )
              goto LABEL_171;
          }
          v16 = *((_DWORD *)battlePing + 3);
          if ( v16 )
          {
            v17 = 1;
            v18 = battlePing[(ranRandomFn(8) & 0x7FFF) % v16 + 11];
            if ( dword_75071C >= 1 )
            {
              while ( *(Player **)(LODWORD(v18) + 812) != universe.curPlayerPtr && LODWORD(v18) != v6 )
              {
                ++v17;
                v18 = battlePing[(ranRandomFn(8) & 0x7FFF) % v16 + 11];
                if ( v17 > dword_75071C )
                  goto LABEL_49;
              }
              v15 = LODWORD(v18);
              if ( v18 != 0.0 )
              {
LABEL_171:
                if ( battleCanChatterAtThisTime(13, (float *)v15) )
                  battleChatterAttempt(v14, 13, v15, *(_DWORD *)(v15 + 688));
              }
            }
          }
        }
      }
    }
LABEL_49:
    if ( (*(_BYTE *)(v6 + 860) & 2) != 0 )
    {
      if ( battleCanChatterAtThisTime(15, (float *)v6) )
        battleChatterAttempt(-1, 15, v6, *(_DWORD *)(v6 + 688));
      *(_BYTE *)(v6 + 860) &= ~2u;
    }
    if ( (*(_BYTE *)(v6 + 860) & 8) != 0 )
    {
      if ( (byte_750710[*(_DWORD *)(*(_DWORD *)(v6 + 32) + 488)] & 2) != 0
        && (*(_DWORD *)(v6 + 460) & 0x100000) == 0
        && (!battleCanChatterAtThisTime(63, (float *)v6) || battleChatterAttempt(-1, 63, v6, *(_DWORD *)(v6 + 688)) == -1)
        && battleCanChatterAtThisTime(71, (float *)v6) )
      {
        if ( PrinceOfSpace )
          speechEventQueue(0, 33559390, -1, -1, 0, -1, -1, 5.0, -1);
        else
          battleChatterFleetAttempt(-1, 71, *(_DWORD *)(v6 + 688), (float *)(v6 + 88));
      }
      *(_BYTE *)(v6 + 860) &= ~8u;
    }
    v19 = *(_DWORD *)(v6 + 880);
    if ( v19 )
    {
      if ( *(_WORD *)(v19 + 20) == 1 )
      {
        v20 = *(_DWORD *)(v19 + 284);
        v21 = *((float *)thisPing + 8) - *(float *)(v19 + 12 * v20 + 32);
        v22 = *((float *)thisPing + 9) - *(float *)(v19 + 4 * (3 * v20 + 9));
        v23 = *((float *)thisPing + 10) - *(float *)(v19 + 12 * v20 + 40);
        if ( flt_750748 * battlePing[2] < fmathSqrtDouble(v23 * v23 + v22 * v22 + v21 * v21) )
        {
          *(_DWORD *)(v6 + 464) = 1;
          v24 = battleChatterFleetAttempt(-1, 24, -1, 0);
          if ( v24 != -1 )
            battleChatterFleetAttempt(v24, 25, -1, 0);
        }
      }
    }
LABEL_71:
    ++v62;
    ++v64;
  }
  while ( v64 < *((_DWORD *)battlePing + 3) );
  v27 = flt_75072C * v61 + flt_750728 * v63;
  v65 = dbl_74F3D8;
  v66 = dbl_74F4E0;
  v67 = dbl_74F1C8;
  v68 = dbl_74F278;
  if ( battlePing[8] != ZERO )
  {
    if ( v59 == ZERO )
      v28 = ONE;
    else
      v28 = flt_750724;
    dbl_74F3D8 = v28;
    dbl_74F4E0 = v28;
  }
  if ( battlePing[5] != ZERO )
  {
    if ( v61 == ZERO )
      v29 = ONE;
    else
      v29 = flt_75072C;
    dbl_74F1C8 = v29;
    dbl_74F278 = v29;
  }
  if ( v27 <= ZERO )
    battlePinga = -1.0;
  else
    battlePinga = (flt_750724 * v59 + flt_750720 * v60) / v27;
  v30 = *((_DWORD *)battlePing + 3);
  v31 = 1;
  if ( dword_75071C < 1 )
    goto LABEL_168;
  while ( 1 )
  {
    v32 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v30 + 11]);
    thisPinga = (_DWORD *)v32;
    if ( *(Player **)(v32 + 812) == universe.curPlayerPtr )
      break;
    if ( ++v31 > dword_75071C )
      goto LABEL_168;
  }
  if ( !v32 )
    goto LABEL_168;
  if ( battlePinga <= (double)flt_750734 )
  {
    if ( battlePinga > (double)flt_750738 )
    {
      v33 = battleChatterFleetAttempt(-1, 22, -1, 0);
      if ( v33 == -1 )
      {
        battlePingb = (float *)battleChatterFleetAttempt(-1, 26, -1, 0);
        if ( battlePingb == (float *)-1 )
        {
          v41 = battleChatterFleetAttempt(-1, 1, -1, 0);
          if ( v41 != -1 )
            battleChatterAttempt(v41, 2, v32, *(_DWORD *)(v32 + 688));
        }
        else
        {
          v38 = *((_DWORD *)battlePing + 3);
          if ( v38 )
          {
            v39 = 1;
            v40 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v38 + 11]);
            if ( dword_75071C >= 1 )
            {
              while ( *(Player **)(v40 + 812) != universe.curPlayerPtr && v40 != v32 )
              {
                ++v39;
                v40 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v38 + 11]);
                if ( v39 > dword_75071C )
                  goto LABEL_168;
              }
              if ( v40 )
                battleChatterAttempt((int)battlePingb, 27, v40, *(_DWORD *)(v40 + 688));
            }
          }
        }
      }
      else
      {
LABEL_94:
        battleChatterFleetAttempt(v33, 23, -1, 0);
      }
      goto LABEL_168;
    }
    if ( battlePinga > (double)flt_75073C )
    {
      v42 = battleChatterFleetAttempt(-1, 20, -1, 0);
      if ( v42 == -1 )
      {
        if ( battleChatterFleetAttempt(-1, 6, -1, 0) == -1 )
          battleChatterFleetAttempt(-1, 7, -1, 0);
      }
      else
      {
        battleChatterFleetAttempt(v42, 21, -1, 0);
      }
      goto LABEL_168;
    }
    if ( battlePinga <= (double)flt_750740 )
    {
      if ( battlePinga <= (double)ZERO )
        goto LABEL_168;
      PlayerMothership = universe.curPlayerPtr->PlayerMothership;
      if ( !universe.curPlayerPtr->PlayerMothership
        || !selShipInSelection(
              (Ship **)battlePing + 11,
              *((_DWORD *)battlePing + 3),
              universe.curPlayerPtr->PlayerMothership)
        || !battleCanChatterAtThisTime(67, (float *)PlayerMothership)
        || (v44 = battleChatterAttempt(-1, 67, (int)PlayerMothership, PlayerMothership->shiptype), v44 == -1) )
      {
        v45 = battleChatterFleetAttempt(-1, 18, -1, 0);
        if ( v45 == -1 )
        {
          v53 = battleChatterFleetAttempt(-1, 35, -1, 0);
          if ( v53 != -1 || (v53 = battleChatterAttempt(-1, 36, (int)thisPinga, thisPinga[172]), v53 != -1) )
          {
            v54 = *((_DWORD *)battlePing + 3);
            if ( v54 )
            {
              v55 = 1;
              v56 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v54 + 11]);
              if ( dword_75071C >= 1 )
              {
                while ( *(Player **)(v56 + 812) != universe.curPlayerPtr && (_DWORD *)v56 != thisPinga )
                {
                  ++v55;
                  v56 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v54 + 11]);
                  if ( v55 > dword_75071C )
                    goto LABEL_168;
                }
                if ( v56 )
                  battleChatterAttempt(v53, 37, v56, *(_DWORD *)(v56 + 688));
              }
            }
          }
          goto LABEL_168;
        }
LABEL_128:
        battleChatterFleetAttempt(v45, 19, -1, 0);
        goto LABEL_168;
      }
    }
    else
    {
      PlayerMothership = universe.curPlayerPtr->PlayerMothership;
      if ( !universe.curPlayerPtr->PlayerMothership
        || !selShipInSelection(
              (Ship **)battlePing + 11,
              *((_DWORD *)battlePing + 3),
              universe.curPlayerPtr->PlayerMothership)
        || !battleCanChatterAtThisTime(67, (float *)PlayerMothership)
        || (v44 = battleChatterAttempt(-1, 67, (int)PlayerMothership, PlayerMothership->shiptype), v44 == -1) )
      {
        v45 = battleChatterFleetAttempt(-1, 18, -1, 0);
        if ( v45 == -1 )
        {
          battlePingc = (float *)battleChatterFleetAttempt(-1, 32, -1, 0);
          if ( battlePingc == (float *)-1 )
          {
            if ( battleChatterFleetAttempt(-1, 33, -1, 0) == -1 )
            {
              v49 = battleChatterFleetAttempt(-1, 3, -1, 0);
              v50 = *((_DWORD *)battlePing + 3);
              battlePingd = (float *)v49;
              if ( v50 )
              {
                v51 = 1;
                v52 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v50 + 11]);
                if ( dword_75071C >= 1 )
                {
                  while ( *(Player **)(v52 + 812) != universe.curPlayerPtr && (_DWORD *)v52 != thisPinga )
                  {
                    ++v51;
                    v52 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v50 + 11]);
                    if ( v51 > dword_75071C )
                      goto LABEL_168;
                  }
                  if ( v52
                    && battlePingd != (float *)-1
                    && battleCanChatterAtThisTime(4, (float *)v52)
                    && battleChatterAttempt(-1, 4, v52, *(_DWORD *)(v52 + 688)) == -1 )
                  {
                    battleChatterFleetAttempt(-1, 5, -1, 0);
                  }
                }
              }
            }
          }
          else
          {
            v46 = *((_DWORD *)battlePing + 3);
            if ( v46 )
            {
              v47 = 1;
              v48 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v46 + 11]);
              if ( dword_75071C >= 1 )
              {
                while ( *(Player **)(v48 + 812) != universe.curPlayerPtr && v48 != v32 )
                {
                  ++v47;
                  v48 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v46 + 11]);
                  if ( v47 > dword_75071C )
                    goto LABEL_168;
                }
                if ( v48 )
                  battleChatterAttempt((int)battlePingc, 34, v48, *(_DWORD *)(v48 + 688));
              }
            }
          }
          goto LABEL_168;
        }
        goto LABEL_128;
      }
    }
    if ( battleCanChatterAtThisTime(68, (float *)v32) )
      battleChatterAttempt(v44, 68, v32, PlayerMothership->shiptype);
    goto LABEL_168;
  }
  if ( !singlePlayerGame || singlePlayerGameInfo != 13 )
  {
    v33 = battleChatterFleetAttempt(-1, 22, -1, 0);
    if ( v33 != -1 )
      goto LABEL_94;
    v34 = battleChatterFleetAttempt(-1, 28, -1, 0);
    if ( v34 != -1 )
    {
      v35 = *((_DWORD *)battlePing + 3);
      if ( v35 )
      {
        v36 = 1;
        v37 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v35 + 11]);
        if ( dword_75071C >= 1 )
        {
          while ( *(Player **)(v37 + 812) != universe.curPlayerPtr && (_DWORD *)v37 != thisPinga )
          {
            ++v36;
            v37 = LODWORD(battlePing[(ranRandomFn(8) & 0x7FFF) % v35 + 11]);
            if ( v36 > dword_75071C )
              goto LABEL_168;
          }
          if ( v37 )
            battleChatterAttempt(v34, 29, v37, *(_DWORD *)(v37 + 688));
        }
      }
    }
  }
LABEL_168:
  dbl_74F3D8 = v65;
  dbl_74F4E0 = v66;
  dbl_74F1C8 = v67;
  dbl_74F278 = v68;
}
#endif

#if 0  // Cat2: battleChatterFrequencySet live code differs from binary (binary uses flt_75074C, live uses batGlobalFrequencyModifier)
void __cdecl battleChatterFrequencySet(sdword freq100)
{
  flt_75074C = (double)freq100 * dbl_72A560;
}
#endif

/*-----------------------------------------------------------------------------
    Name        : battleChatterResetTimers
    Description : Reset all battle chatter timers, called when a sound bank loads.
    Inputs      :
    Outputs     :
    Return      :
----------------------------------------------------------------------------*/
void battleChatterResetTimers(void)
{
    sdword i;

    batFleetChatterTime = -30.0f;
    batLastFleetChatterEvent = NULL;
    for (i = 0; i < BCE_LastBCE; i++)
    {
        battleChatterEvent[i].lastTimeSpoken = 0.0f;
    }
}
