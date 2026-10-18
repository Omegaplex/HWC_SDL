/* defines for music tracks in the Music4Cat.wxd file */

/* Ambient/level tracks 0-16: random pool, selected via MUS_RANDOM_AMBIENT.
   Exact per-mission mappings are TBD; these are HW1 placeholders. */
#define AMB_Mission1				10
#define AMB_Mission2				0
#define AMB_Mission3				11
#define AMB_Mission4				0
#define AMB_Mission5				1
#define AMB_Mission6				2
#define AMB_Mission7				3
#define AMB_Mission8				3
#define AMB_Mission9				4
#define AMB_Mission10				5
#define AMB_Mission11				6
#define AMB_Mission12				2
#define AMB_Mission13				7
#define AMB_Mission14				8
#define AMB_Mission15				6
#define AMB_Mission16				10
#define AMB12_FrontEnd				17
#define AMB13_Tutorial				18

#define MUS_FIRST_AMBIENT			0
#define MUS_LAST_AMBIENT   			9
#define MUS_LAST_LEVELTUNE			11

/* Tracks >= MUS_FIRST_BATTLE use battle-class volume (louder/less attenuation) */
#define MUS_FIRST_BATTLE			19
#define MUS_LAST_BATTLE				24

/* Battle tracks (looping): placeholders, exact Cataclysm indices TBD */
#define B01_TuranicRaidersLong		19
#define B02_TuranicRaidersShort		20
#define B03_Swarmers				21
#define B04_EvilEmpire				22
#define NISlet01					23

/* NIS music tracks */
#define NIS01_R1					17
#define NIS01_R2					18
#define NIS02						19
#define NIS03						20
#define NIS04						21
#define NIS05						22
#define NIS06						23
#define NIS07						24
#define NIS08						25
#define NIS09						26
#define NIS10						27
#define NIS11						28

#define MUS_FIRST_NIS				17
#define MUS_LAST_NIS				28

/* Animatic music tracks */
#define ANIM00_Sierra				29
#define ANIM00_Relic				30
#define ANIM00_Opening				31
#define ANIM01_02					32
#define ANIM02_03					33
#define ANIM03_04					34
#define ANIM04_05					35
#define ANIM05_06					36
#define ANIM06_07					37
#define ANIM07_08					38
#define ANIM08_09					39
#define ANIM09_10					40
#define ANIM10_11					41
#define ANIM11_12					42
#define ANIM12_13					43
#define ANIM13_14					44
#define ANIM14_15					45
#define ANIM15_16					46
#define ANIM_Ending					47

#define MUS_FIRST_ANIMATIC			29
#define MUS_LAST_ANIMATIC			47

/* Special Cataclysm tracks */
#define MUS_CREDITS_SCREEN			40  /* Victory/credits screen with speech events */
#define RollCredits					41  /* Roll credits sequence */
#define MUS_RANDOM_AMBIENT			1102 /* Selects random track from ambient pool (0-16) */
