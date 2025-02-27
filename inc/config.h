#ifndef CONFIG_H
#define CONFIG_H 1

#define MUD_NAME "Silverminken"

#define ADMINS   ({ "danne" })
#define HOST_NAME "localhost"

// GMCP-Pfade
#define GMCP_SOUND_URL       "http://www." HOST_NAME "/sound"
#define GMCP_FILES_BASE_URL     "https://www." HOST_NAME "/gmcp/file?token="

// Maximales Hauptspeicherlimit in Bytes
// Bei Ueberschreiten wird von master::reset() Armageddon aufgerufen.
#define MAX_MALLOC_SIZE 0x60000000	// 1536 MB

// Wo findet man das Masterobjekt?
#define MASTER_OB __MASTER_OBJECT__

// This one is used by the map system. It defines the object
// that manages the map.
#define MAP_OB "/map/map"

// das cron object
#define CRON_OB "/apps/cron"

// das Login-Objekt
#define LOGIN_OB "/secure/obj/login"

// das Wizard-Shell-Objekt
#define WIZARD_SHELL_OB "/obj/wizard_shell"

// Playerfiles
#define PLAYER_FILE(x) ("/var/players/" + (x))
#define PLAYER_FILE_BEFORE_WIZ(x) ("/var/players/wizzes/" + (x))
// (auch zu aendern dann: valid_read und valid_write im master)

#define PLAYER_SUICID_LOCATION "/var/players/suicid"

#define PLAYER_DELETER "/secure/player_deleter"



// Objekt, das die Event-Ausfrufe verarbeitet:
#define EVENT_MASTER    "/secure/event"

// Seele
#define SOUL_SRC "/obj/soul"

// Wo liegen die Skillbeschreibungen SKILL_PATH_STRINGS+"/skill/offensiv..."
#define SKILL_PATH_STRINGS "/static"

// Wo liegen die Onlinehilfsdatein fuer den Hilfebefehl?

#define HELP_PATH "/doc/hilfe"

// Die Namen der Gastcharaktere im Mud
//
// Geradezahlige Indices = weibliche Namen
// Ungeradezahlige Indices = maennliche Namen
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
 * Um einzelne Skill mit anderen ausgleichen zu koennen, wird
 * der Einzel-Skill nicht auf AVERAGE_EXPERIENCE begrenzt, sondern auf
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
 * Different skills has different learning curves. For example, a fighter
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
#define LEARNING_6		1000	/* Extremly unusual actions */

#define LEARNING_NAHKAMPF 10
#define LEARNING_DEFENSIV 10
#define LEARNING_SCHUSS 20
#define LEARNING_WURF 50

/*
 * To prevent extensive use of SHOUT and TELL
 */

// SHOUT COST wird nicht mehr verwendet, sondern stattdessen die beiden 
// folgenden.
#define SHOUT_COST 30
#define MIN_SHOUT_COST 20
#define SHOUT_CHARS_PER_SP 5

#define TELL_COST  3

// Rede-Kosten fuer magische Gilden fuer Kommunikation zwischen
// Mitgliedern der Gilde:

#define TELL_COST_MAGIC_GUILD 1

// Kosten der Musterung:
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
 * 2 = half speed, one hit every zwo heart beats
 * 3 = and so on...
 *
 * 0 = hit only at int overflows, don't to this
 */

#define FIGHT_SPEED 1


/*
 *
 * Massenmoerder:
 *
 * Ab wieviel M's wird ein Spieler als Massenmoerder angesehen?
 * Dann wird er beim Tod von den Werten her auf neuer Spieler
 * runtergekuerzt.
 *
 * Wird dieses Verhalten nicht gewuenscht, einfach dieses Define
 * nicht definieren.
 */

#define MASSENMOERDER 5


/*
 * Time of autosave in heart_beats.
 */

#define SAVE_TIME 300

#define RESET_TIME 2400

/*
 * Controling the align-titles
 */

#define ALIGN_STRETCH 10
#define A_HEILIG     ( 100*ALIGN_STRETCH) // ueber 1000  heilig
#define A_GUT        (  20*ALIGN_STRETCH) // ueber  200  gut
#define A_NETT       (   4*ALIGN_STRETCH) // ueber   40  nett
                                          // -40..40     neutral
#define A_MIES       (  -4*ALIGN_STRETCH) // unter -40   mies
#define A_TEUFLISCH  ( -20*ALIGN_STRETCH) // unter -200  teuflisch
#define A_DAEMONISCH (-100*ALIGN_STRETCH) // unter -1000 daemonisch


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

// Es gibt 17 Waffenskills (Max ist 17 * MAX_EXP)
// einer ist derzeit fuer Magier nicht erhaeltlich...
// drei schenken wir den Spielern
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

// Es gibt 7 Zauber-Skills (Max ist 7 * MAX_EXP)
#define MAX_SUM_ZAUBER	(AVERAGE_EXPERIENCE*4)

// Anzahl Zeilen in der Raumbeschreibung:
#define MAX_ROOM_INVENTORY_LIST 22

/*
 *  To log all shutdowns
 */

#define LOG_SHUTDOWN

/*
 * The requested experience to become wizard.
 *
 * (Bei 10 Raetsel und 5 Spielen Faktor 30 ???
 *  Grundfaktor 10, pro Raetsel/Spiel ein Punkt dazu)
 */
#define TOTAL_EXPERIENCE (AVERAGE_EXPERIENCE*44)

/*
 * Show experience in Promille?
 */

#define SHOW_EXPERIENCE_IN_PROMILLE

/*
 * The Default-Room, where a new player starts.
 */
#define DEFAULT_START_ROOM "/room/church"


#define DEFAULT_ROOM_AFTER_DEATH "/room/church"

#define DEFAULT_NIRVANA_EXIT "/room/church"

/*
 * Der Raum, in dem die ganzen Statuen rumstehen
 */
// #define STATUE_ROOM "/room/statue"


/*
 * Sollen Spieler auch nach dem Ausloggen ihre Dinge behalten?
 */

#define RETAIN_PLAYER_INVENTORY ja
#define PLAYER_INVENTORY_CONTAINER "/secure/player_container/"


/*
 * Nach STATUE_TIME wird beim naechsten reset() die Statue
 * removed, wenn sie im STATUE_ROOM steht
 * 0 bedeutet nie
 */
#define STATUE_TIME 3600

/*
 * Wieviel duerfen Goetter mit sich herumschleppen ?
 */
#define WIZ_ENCUMBRANCE 100

/*
 * Nach wieviel Sekunden soll ein NON-INTERACTIVE-Spieler geloescht werden
 * bzw. in den Statue-Raum bewegt werden, wenn dieser definiert ist.
 * 0 bedeutet nie
 */
#define NET_DEAD_TIME 3600

/*
 * Nach wieviel Sekunden soll ein idler Spieler zur Statue werden
 * 0 bedeutet nie
 */
#define MAX_IDLE_TIME 18000

/*
 * Nach wievielen Sekunden soll ein idlender Gott zur Statue werden
 * 0 bedeutet nie
 */
#define MAX_WIZ_IDLE_TIME 0

/*
 * Das Produkt aus max_hp, compute_damage() und armour_level eines Monsters
 * wird zur durch einen Teiler dividiert und als Erfahrung im Skill
 * "skill getoetet" eingetragen. Dabei wird zwischen kleingetier und grosswild
 * unterschieden. Die Grenze zwischen beidem ist BEGIN_GROSSWILD, die Teiler
 * sind TEILE_KLEINGETIER und TEILER_GROSSWILD.
 */

#define BEGIN_GROSSWILD 6000

#define TEILER_KLEINGETIER 60

#define TEILER_GROSSWILD 100

/* Ermoeglicht debugging von /i/living/hands, /i/weapons/...
 *                           /i/armour/...
 */
#undef FIGHT_DEBUG

/*
 * Loggen von Charakteristics nach /apps/object_stats
 */
// #define LOG_OBJECT_STATS

// simul efun fuer call out verwenden, um Mudlahmlegung durch
// fehlerhaft programmierte call outs entgegenzuwirken
#define USE_CALL_OUT_SIMUL_EFUN ja

// Ab welcher call out Anzahl werden erste Rettungsversuche unternommen?
#define CALLOUT_LIMIT_ONE 600	  // Ab da gilt das ONE_OBJECT-Limit
#define CALLOUT_LIMIT_TWO 800	  // Ab da gilt das ALL_CLONES-Limit
#define CALLOUT_LIMIT_THREE 1000  // Ab da gilt das PER_UID-Limit

// Wieviele call outs darf ein einzelnes Objekt haben, bevor es als
// fehlerhaft angesehen wird?
#define MAX_CALLOUTS_FOR_ONE_OBJECT 50

#define MAX_CALLOUTS_FOR_ALL_CLONES 100

#define MAX_CALLOUTS_PER_UID 100

// Groesse des Donuts
#define MAP_MIN_X	__INT_MIN__
#define MAP_MIN_Y	__INT_MIN__
#define MAP_MAX_X	__INT_MAX__
#define MAP_MAX_Y	__INT_MAX__

#endif // CONFIG_H
