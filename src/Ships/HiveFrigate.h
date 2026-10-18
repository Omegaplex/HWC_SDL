#ifndef ___HIVEFRIGATE_H
#define ___HIVEFRIGATE_H

#include "spaceobj.h"

extern CustShipHeader HiveFrigateHeader;
bool32 HiveFrigateCancelSpecial(Ship *ship);

// Maximum number of swarmer drones housed by a hive frigate.
#define HIVE_MAX_SWARMERS 6

/*
 * Hive frigate swarmer-launch state machine (HiveFrigateSpecifics::launchState).
 *
 * Transitions:
 *   Idle ──(target set, in range)──► Armed ──► Launching ──► Deploying ──► Active
 *   Idle ──(target set, out of range)──► Approach ──► Armed
 *   Active ──(target lost)──► Canceling ──► Recalling ──► Idle
 *   Active ──(swarmer target changes)──► Deploying
 */
#define HiveLaunch_Idle      0  // no active special; auto-targets nearby in Aggressive mode
#define HiveLaunch_Armed     1  // in range; assigning launch timers to docked swarmers
#define HiveLaunch_Launching 2  // launch timers set; issuing dock-to-launch commands
#define HiveLaunch_Deploying 3  // swarmers deployed; issuing attack orders
#define HiveLaunch_Active    4  // swarmers attacking; monitoring, re-launching stragglers
#define HiveLaunch_Canceling 5  // cancel ordered; waiting for swarmers to enter return anim
#define HiveLaunch_Recalling 6  // ordering swarmers back to dock via clDock
#define HiveLaunch_Approach  7  // closing range to target; transitions to Armed when in range

// Swarmer drone state values (reported by hiveSwarmerGetState, stored via hiveSwarmerSetState).
#define HiveSwarmer_Docked    0  // docked and idle inside the hive frigate
#define HiveSwarmer_Launching 1  // launch delay active; dock-to-launch command issued
#define HiveSwarmer_Deployed  2  // out in space, not yet assigned an attack order
#define HiveSwarmer_Returning 3  // returning to dock (recall in progress)
#define HiveSwarmer_Attacking 4  // assigned to attack a target via clAttack

sdword hiveSwarmerGetState(Ship *ship);
void hiveSwarmerSetState(Ship *ship, sdword state);

// Runtime ShipSpecifics accessor: HiveFrigateSpecifics lives at ShipSpecifics+0x04
// (after the rebuildSwarmerAt timer). Verified byte-exact against cataclysm.exe
// HiveFrigateCanHalt @0x5B6650 (writes specialState at ship+0x1604 = spec+0xFC8).
HiveFrigateSpecifics *hiveFrigateSpec(Ship *ship);

#endif
