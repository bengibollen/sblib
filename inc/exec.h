/*
 * /sys/exec.h
 *
 * This is the default include file with some prefedefined macros for the
 * command exec. It is included in your exec_obj.c by default _after_ the
 * exec.h from your home directory has been included. Provisions are made
 * so that definitions from this file, generic as they are, are only
 * created if they are not defined before to guard against redefinition
 * errors.
 *
 * The following definitions are included:
 *
 * ME      Always returns the objectpointer to you, the person executing
 *         the exec command, even if you switched this_player().
 * TP      this_player() [may have been altered using set_this_player(o)]
 * TI      this_interactive() [probably always the same as ME]
 * TO      this_object()
 *
 * FP(s)   find_player(s)
 * FL(s)   find_living(s)
 * ENV(o)  environment(o)
 * INV(o)  all_inventory(o)
 * CAP(s)  capitalize(s)
 * LOW(s)  lower_case(s)
 * HERE    The room you are in.
 */

#ifndef DEFAULT_EXEC_INCLUDE
#define DEFAULT_EXEC_INCLUDE

#ifndef ME
#define ME     (find_player(getuid()))
#endif

#ifndef TP
#define TP     (this_player())
#endif

#ifndef TI
#define TI     (this_interactive())
#endif

#ifndef TO
#define TO     (this_object())
#endif

#ifndef FP
#define FP(s)  (find_player(s))
#endif

#ifndef FL
#define FL(s)  (find_living(s))
#endif

#ifndef ENV
#define ENV(o) (environment(o))
#endif

#ifndef INV
#define INV(o) (all_inventory(o))
#endif

#ifndef CAP
#define CAP(s) (capitalize(s))
#endif

#ifndef LOW
#define LOW(s) (lower_case(s))
#endif

#ifndef HERE
#define HERE (environment(this_interactive()))
#endif

/* No definitions beyond this line. */
#endif
