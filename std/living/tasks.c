/*
 * /std/living/tasks.c
 *
 * See /doc/man/general/tasks for information on tasks.
 *
 * One public function is defined:
 *
 * nomask varargs int * resolve_task(int difficulty, int *skill_list,
 *     object opponent, int * opp_skill_list)
 *
 * This should be called to determine the success of a given task.
 */

#include <tasks.h>
#include <macros.h>
#include <ss_types.h>

/*
 * Function: find_listval
 * Description: Finds the value of one list_member  in a skill list
 * Arguments: Either a stat, skill, property or a VBFC
 * Returns: Value of the list_member
 */
private int find_listval(mixed list_member)
{
    if (closurep(list_member))
        return funcall(list_member);

    if (stringp(list_member) && sizeof(list_member) && list_member[0] == '@')
    	return ({int}) this_object()->check_call(list_member);
    else if (stringp(list_member))
    	return ({int}) this_object()->query_prop(list_member);

    else if (!intp(list_member))
	    return 0;

/*
 * Kludge to figure out which stat.  This depends on the constants in tasks.h
 * -1 -> 0    (TS_STR    -> SS_STR) until
 * -10 -> 9   (TS_CRAFT -> SS_CRAFT)
 */
    else if (list_member < 0 && list_member > -11)
	    return ({int}) this_object()->query_stat((-list_member) - 1);

	return ({int}) this_object()->query_skill(list_member);
}

/*
 * Function: find_drm
 * Description: Finds the die roll modifiers for a this living, given the
 *              list of applicable skills, stats and modifiers.
 * Arguments:
 *            'skill_list' is a list of integers or VBFC's, as described above.
 * Returns: a positive integer containing the total die roll modifier (drm)
 *          (Zero on error)
 */
public int find_drm(mixed *skill_list)
{
    int mod, weight, count, i, drm, n, tmod;

    n = sizeof(skill_list);
    i = 0;
    weight = 100;

    while(i < n)
    {
        if (mod != 0)
            weight = 100;

        mod = 0;

        /* this added because the switch below can only take an
         * integer argument.
         */
        if (closurep(skill_list[i]) || stringp(skill_list[i]))
        {
            mod = find_listval(skill_list[i++]);
        }
        else
        {
     	    switch (skill_list[i])
            {

                case SKILL_MIN:
                    i++;
    	            if (skill_list[i] != SKILL_END)
	        	        mod = find_listval(skill_list[i++]);
	                else
		                mod = 0;

                    for (; i < n && skill_list[i] != SKILL_END; i++)
                    {
                        tmod = find_listval(skill_list[i]);
                        mod = MIN(tmod, mod);
                    }
                    break;

                case SKILL_MAX:
                    i++;

                    if (skill_list[i] != SKILL_END)
                        mod = find_listval(skill_list[i++]);
                    else
                        mod = 0;

                    for (; i < n && skill_list[i] != SKILL_END; i++)
                    {
                        tmod = find_listval(skill_list[i]);
                        mod = MAX(tmod, mod);
                    }

                    break;

                case SKILL_AVG:
                    i++;

                    if (skill_list[i] != SKILL_END)
                    {
                        mod = find_listval(skill_list[i++]);
                        count = 1;
                    }
                    else
                    {
                        mod = 0;
                        count = 0;
                    }

                    for (; skill_list[i] != SKILL_END; i++)
                    {
        		        mod += find_listval(skill_list[i]);
                        count++;
                    }
        
                    if (count)
                        mod /= count;
                    
                    break;

                case SKILL_WEIGHT:
                    i++;
                    weight = skill_list[i++];
                    mod = 0;
                break;

                case SKILL_VALUE:
                    i++;
                    mod = skill_list[i++];
                break;

                case SKILL_END:
                    i++;
                    break;

                default:
                    mod = find_listval(skill_list[i++]);
    	            break;
            }
        }

        drm += weight * mod / 100;
    }

    return 2 * drm;
}


/*
 * Function:    resolve_task
 * Description: Determines the success of a task
 * Arguments:   'difficulty' is a positive integer, the difficulty of the task
 *              'skill_list' is a list describing the skills and stats to
 *              use in determining success, for the living attempting the task.
 *              'opponent' is the living object which the task works against,
 *              in a competitive task.
 *              'opp_skill_list' describes the skills the opponent uses.
 * Returns:     The percentage of the difficulty that was achieved.
 *              Negative implies failure, positive implies success.
 */
nomask varargs int resolve_task(int difficulty, mixed *skill_list,
    object opponent, mixed * opp_skill_list)
{
    int dr, drm;

    dr = random(500) - random(500) + 500;
    drm = 0;

    if (pointerp(skill_list))
        drm += find_drm(skill_list);

    if (objectp(opponent) && (pointerp(opp_skill_list)))
        drm -= ({int}) opponent->find_drm(opp_skill_list);

    if (difficulty != 0)
        return 100 * (dr + drm - difficulty) / difficulty;
    return dr + drm - difficulty;

}

/*
 * Function name: check_skill
 * Description:   This is a pretty general skill check that some like better
 * Arguments:	  skilllist - Same type of skill list as in resolve_task()
 *		  player    - The player to test skills on (or any living)
 * Returns:	  1 if success, 0 if not
 */
public int check_skill(mixed *skilllist, object player)
{
    int i, skill;

    i = ({int}) player->find_drm(skilllist);
    skill = (i / sizeof(skilllist));
    if (random(100) + 1 <= skill)
	return 1;
    else
	return 0;
}

