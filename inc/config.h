#ifndef CONFIG_H
#define CONFIG_H 1

#define MUD_NAME "Silverminken"

#define ADMINS   ({ "danne" })
#define HOST_NAME "localhost"

// GMCP paths
#define GMCP_SOUND_URL       "http://www." HOST_NAME "/sound"
#define GMCP_FILES_BASE_URL     "https://www." HOST_NAME "/gmcp/file?token="

// Maximum memory limit in bytes
// If exceeded, master::reset() will call Armageddon
#define MAX_MALLOC_SIZE 0x60000000	// 1536 MB

// Where to find the master object?
// #define MASTER_OBJ __MASTER_OBJECT__

// This one is used by the map system. It defines the object
// that manages the map.
#define MAP_OB "/map/map"

// the cron object
#define CRON_OB "/apps/cron"

// the login object
#define LOGIN_OB "/secure/obj/login"

// the wizard shell object
#define WIZARD_SHELL_OB "/obj/wizard_shell"

// Player files
// #define PLAYER_FILE(x) ("/var/players/" + (x))
#define PLAYER_FILE_BEFORE_WIZ(x) ("/var/players/wizzes/" + (x))
// (also change: valid_read and valid_write in master)

#define PLAYER_SUICID_LOCATION "/var/players/suicid"

#define PLAYER_DELETER "/secure/player_deleter"

// Object that processes event calls:
#define EVENT_MASTER    "/secure/event"

// Soul
#define SOUL_SRC "/obj/soul"

// Where are the skill descriptions located SKILL_PATH_STRINGS+"/skill/offensive..."
#define SKILL_PATH_STRINGS "/static"

// Where are the online help files for the help command?
#define HELP_PATH "/doc/hilfe"

// The names of guest characters in the MUD
//
// Even indices = female names
// Odd indices = male names
#define GUEST_NAMES \
({ \
   "anne", "tim", "charlotte", "simon", "janette", \
   "herbert", "maria", "arndt", "sandra", "ulf"    \
})

/*
#define PLAYER_DAY ((shorttimestr(time())[0..4]=="01.04") && \
        (-1==member(({"sissilein","gnomilein"}), \
         (playerp(this_object())?this_object():(this_player()||this_object())) \
          ->query_real_name())))
*/

/*
 * This is an important constant. It says how many hits a fighter has
 * had with his weapon when the fighter is in between a beginner and
 * an expert user.
 *
 * If this value is changed a lot suddenly, then all players will suddenly
 * have it more easy or harder.
 */
#define AVERAGE_EXPERIENCE	3000

/*
 * To balance individual skills with others, the individual skill is not
 * limited to AVERAGE_EXPERIENCE, but to
 */
//#define MAX_SKILL      ((AVERAGE_EXPERIENCE*3)/2)
#define MAX_SKILL      4500

/*
 * Convert the experience amount into a skill value, in the range
 * 0 - 100 and the same for range 0 - 1000.
 *
 */
#define EXP_TO_PERCENT(exp)\
 ((100*((exp)<AVERAGE_EXPERIENCE?(exp):AVERAGE_EXPERIENCE))/AVERAGE_EXPERIENCE)

#define EXP_TO_PROMILLE(exp)\
 ((1000*((exp)<AVERAGE_EXPERIENCE?(exp):AVERAGE_EXPERIENCE))/AVERAGE_EXPERIENCE)

/*
 * Different skills have different learning curves. For example, a fighter
 * might have to hit 1000 times before he is an average fighter.
 * But a thief may only have to pick locks 100 times before he is
 * an average thief. All skills when used must define what learning constant
 * below to use. The administrator may later change this value to tune
 * the game.
 *
 * Always use one of the symbols below, not an absolute numeric value.
 */

#define LEARNING_1		1	/* Fighting etc */
#define LEARNING_2		5	/* Using spells in combats etc */
#define LEARNING_3		20	/* Pick locks,climb, unusual spells */
#define LEARNING_4		50	/* Start of one action weapons */
#define LEARNING_5		100	/* Very unusual actions */
#define LEARNING_6		1000	/* Extremely unusual actions */

#define LEARNING_NAHKAMPF 10
#define LEARNING_DEFENSIV 10
#define LEARNING_SCHUSS 20
#define LEARNING_WURF 50

/*
 * To prevent extensive use of SHOUT and TELL
 */

// SHOUT COST is no longer used, instead the following two are used.
#define SHOUT_COST 30
#define MIN_SHOUT_COST 20
#define SHOUT_CHARS_PER_SP 5

#define TELL_COST  3

// Communication costs for magic guilds for communication between
// guild members:
#define TELL_COST_MAGIC_GUILD 1

// Costs for mustering:
#define MUSTER_COST 3

/*
 * Max Number of scars
 */

#define MAX_SCAR 10

/*
 * Number of heart_beats to heal.
 */

#define HEALING_TIME 20

/*
 * Fight Speed, one hit per x heart beats
 *
 * 1 = normal speed, one hit each heart beat
 * 2 = half speed, one hit every two heart beats
 * 3 = and so on...
 *
 * 0 = hit only at int overflows, don't do this
 */

#define FIGHT_SPEED 1

/*
 * Mass murderer:
 *
 * How many M's before a player is considered a mass murderer?
 * Then upon death, their values will be reduced to new player level.
 *
 * If this behavior is not desired, simply don't define this constant.
 */

#define MASSENMOERDER 5

/*
 * Time of autosave in heart_beats.
 */

#define SAVE_TIME 300

#define RESET_TIME 2400

/*
 * Controlling the align-titles
 */

#define ALIGN_STRETCH 10
#define A_HEILIG     ( 100*ALIGN_STRETCH) // over 1000  holy
#define A_GUT        (  20*ALIGN_STRETCH) // over  200  good
#define A_NETT       (   4*ALIGN_STRETCH) // over   40  nice
                                          // -40..40     neutral
#define A_MIES       (  -4*ALIGN_STRETCH) // under -40   mean
#define A_TEUFLISCH  ( -20*ALIGN_STRETCH) // under -200  devilish
#define A_DAEMONISCH (-100*ALIGN_STRETCH) // under -1000 demonic

/*
 * The MAX_Values of the Parameters which are necessary to
 * compute the stats. If a player exceeds these Limits, his
 * stats are set to 100. For Details see:
 * update_stats() in  /i/player/skills.c
 */

#define TIME_TO_WIZ	2160000	/* Expected minimum time to become wizard:
				   25 days				   */

// #define MAX_SUM_WEIGHT	10000000
#define MAX_SUM_WEIGHT  6500000

// #define MAX_SUM_MOVE	70000
#define MAX_SUM_MOVE	60000

// There are 17 weapon skills (Max is 17 * MAX_EXP)
// one is currently not available for mages...
// three are given to the players
#define MAX_SUM_WEAPON	(AVERAGE_EXPERIENCE*13)

#define ADULT_AGE	TIME_TO_WIZ

// #define MAX_SUM_HP	45000
#define MAX_SUM_HP	35000

// #define MAX_SUM_SP	75000
#define MAX_SUM_SP	50000

// #define MAX_SUM_COMM    15000
#define MAX_SUM_COMM    12000

// #define MAX_SUM_FEEL	8500
#define MAX_SUM_FEEL	6500

// There are 7 magic skills (Max is 7 * MAX_EXP)
#define MAX_SUM_ZAUBER	(AVERAGE_EXPERIENCE*4)

// Number of lines in the room description:
#define MAX_ROOM_INVENTORY_LIST 22

/*
 *  To log all shutdowns
 */

#define LOG_SHUTDOWN

/*
 * The requested experience to become wizard.
 *
 * (With 10 puzzles and 5 games factor 30 ???
 *  Base factor 10, one point per puzzle/game)
 */
#define TOTAL_EXPERIENCE (AVERAGE_EXPERIENCE*44)

/*
 * Show experience in promille?
 */

#define SHOW_EXPERIENCE_IN_PROMILLE

/*
 * The Default-Room, where a new player starts.
 */
#define DEFAULT_START_ROOM "/room/church"

#define DEFAULT_ROOM_AFTER_DEATH "/room/church"

#define DEFAULT_NIRVANA_EXIT "/room/church"

/*
 * The room where all the statues are located
 */
// #define STATUE_ROOM "/room/statue"

/*
 * Should players keep their items after logging out?
 */

#define RETAIN_PLAYER_INVENTORY ja
#define PLAYER_INVENTORY_CONTAINER "/secure/player_container/"

/*
 * After STATUE_TIME, the statue will be removed at the next reset()
 * if it's in STATUE_ROOM
 * 0 means never
 */
#define STATUE_TIME 3600

/*
 * How much can gods carry?
 */
#define WIZ_ENCUMBRANCE 100

/*
 * After how many seconds should a NON-INTERACTIVE player be deleted
 * or moved to the statue room if it's defined.
 * 0 means never
 */
#define NET_DEAD_TIME 3600

/*
 * After how many seconds should an idle player become a statue
 * 0 means never
 */
#define MAX_IDLE_TIME 18000

/*
 * After how many seconds should an idle god become a statue
 * 0 means never
 */
#define MAX_WIZ_IDLE_TIME 0

/*
 * The product of max_hp, compute_damage() and armour_level of a monster
 * is divided by a divisor and recorded as experience in the skill
 * "kill skill". A distinction is made between small creatures and large game.
 * The boundary between them is BEGIN_GROSSWILD, the divisors are
 * TEILE_KLEINGETIER and TEILER_GROSSWILD.
 */

#define BEGIN_GROSSWILD 6000

#define TEILER_KLEINGETIER 60

#define TEILER_GROSSWILD 100

/* Enable debugging of /i/living/hands, /i/weapons/...
 *                           /i/armour/...
 */
#undef FIGHT_DEBUG

/*
 * Log characteristics to /apps/object_stats
 */
// #define LOG_OBJECT_STATS

// Use simul efun for call out to prevent MUD slowdown through
// incorrectly programmed call outs
#define USE_CALL_OUT_SIMUL_EFUN ja

// At what call out count should first rescue attempts be made?
#define CALLOUT_LIMIT_ONE 600	  // From here ONE_OBJECT limit applies
#define CALLOUT_LIMIT_TWO 800	  // From here ALL_CLONES limit applies
#define CALLOUT_LIMIT_THREE 1000  // From here PER_UID limit applies

// How many call outs may a single object have before it's considered faulty?
#define MAX_CALLOUTS_FOR_ONE_OBJECT 50

#define MAX_CALLOUTS_FOR_ALL_CLONES 100

#define MAX_CALLOUTS_PER_UID 100

// Size of the donut
#define MAP_MIN_X	__INT_MIN__
#define MAP_MIN_Y	__INT_MIN__
#define MAP_MAX_X	__INT_MAX__
#define MAP_MAX_Y	__INT_MAX__


// UIDs
#define ROOT_UID "ROOT"

// Settings
#define false 			        0
#define true 			        1
#define null 			        0

// Log file
#define LOG_FILE "/lib/log"

#endif // CONFIG_H
