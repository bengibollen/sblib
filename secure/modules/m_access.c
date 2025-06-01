/*
 * $Revision: 1.2 $
 * $Date: 2004/05/10 20:05:38 $
 *
 * Purpose: The Master ACCESS Module.
 *          Copyright Tubmud 1997
 *          Changes for Evermore 1999
 *          Changes for Heaven7 2002
 *
 *
 */

#pragma strong_types
#pragma save_types

#include "/sys/object_info.h"

/**** PUBLIC INTERFACE *******************************************************/

nomask static void set_privilege(mixed priv);
nomask mixed query_privilege();
nomask mixed query_unguarded_privilege();
nomask int verify_privilege_granted(mixed *handle);

nomask varargs static mixed unguarded(mixed priv,closure code,mixed args);

/**** PRIVATE INTERFACE ******************************************************/

private static mixed privilege;
private static mixed unguarded_privilege;
private static mixed *security_handle;

/**** PUBLIC IMPLEMENTATION **************************************************/

nomask static void set_privilege(mixed priv) 
{
  if (!extern_call())
  {
    if (object_name(this_object()) == SECURITY)
    {
      privilege = priv;
      return;
    }
    else if (SECURITY -> valid_privilege(priv)) 
    {
      if (stringp(priv) && priv[0] == '#') 
      {        
        if ((object_info(this_object(), OI_ONCE_INTERACTIVE)) && 
            (object_name(this_object())[<sizeof(priv)..] == priv))
        {
          privilege = priv;
        }
        return;
      }
      else 
      {
        privilege = SECURITY -> 
          reduced_privilege(priv, SECURITY -> 
            query_protection(this_object(), 1));
      }
    }
  }
}

nomask mixed query_privilege() 
{
  return privilege;
}

nomask mixed query_unguarded_privilege() 
{
  return unguarded_privilege;
}

nomask int verify_privilege_granted(mixed *handle) 
{
  return security_handle == handle;
}

nomask varargs static mixed unguarded(mixed priv,closure code,mixed args) 
{
  if (!extern_call())
  {
    unguarded_privilege = priv;
    return SECURITY -> call_unguarded(code, args, security_handle = allocate(1));
  }
  else
  {
    return 0;
  }
}
