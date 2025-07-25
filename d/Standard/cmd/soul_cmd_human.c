/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*

  soul_cmd_ghost.c

  This is the soul command object for ghosts.
  Do not inherit this object, use the objects under /std

*/

#pragma strict_types
#pragma no_clone

inherit "/std/cmd/std/soul_cmd";

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping query_cmdlist()
{
    return ::query_cmdlist();
}

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the
 * same order as in the function name list.
 * **************************************************************************/
