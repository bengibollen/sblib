/*
 * /secure/mail_checker.c
 *
 * This object can be called to check whether there is mail for a player.
 * In order to inquire whether there is mail for a certain player, you
 * can make the following call:
 *
 * int new_mail = MAIL_CHECKER->query_mail(mixed player);
 *
 * Here mixed player is the objectpointer to the player or the name of
 * the player. If omitted, this_player() is used. The result of this query
 * can be one of the following integer values:
 *
 *  0  - there is no mail at all for the player;
 *  1  - there is mail for the player, though all read;
 *  2  - there is new mail for the player;
 *  3  - there is unread mail for the player, but there is no new mail.
 */

#pragma no_clone
#pragma no_inherit

#pragma strict_types

/* If we define DEBUG, debug mail directories will be used. */

#include <mail.h>
#include <std.h>
#include <libfiles.h>
#include <configuration.h>

/*
 * Function name: create
 * Description  : Constructor. Called when creating this module.
 */
nomask void
create()
{
    configure_object(this_object(), OC_EUID, getuid());
}

/*
 * Function name: short
 * Description  : Returns the short description for this object.
 * Returns      : string - the short description.
 */
nomask string
short()
{
    return "the mail checker";
}

/*
 * Function name: query_mail
 * Description  : This function is actually used to check whether there
 *                is mail for the player.
 * Arguments    : mixed player - optional player, either the objectpointer,
 *                               or his name. If omitted: use this_player().
 * Returns      : int - 0/1/2/3 - for details, see the file-header.
 */
nomask int
query_mail(mixed player = this_player())
{
    mapping mail;
    string file_data;

    if (objectp(player))
    {
	player = ({string}) player->query_real_name();
    }

    /* Check whether the player exists. */
    if (!(({int}) SECURITY->exist_player(player)))
    {
	return 0;
    }

    /* If something is wrong with the mail-file, there is no mail for
     * the player.
     */
    if (file_data = read_file(FILE_NAME_MAIL(player)))
    {
    mail = restore_value(file_data);
        if ((sizeof(mail) != M_SIZEOF_MAIL) ||
            (member(MAIL_NEW_MAIL, m_indices(mail)) == -1))
        {
        return 0;
        }
    }
    else
    {
        return 0;
    }

    return mail[MAIL_NEW_MAIL];
}
