/*-------------------------------------------------------------------------*/
static ident_t *
define_variable (ident_t *name, fulltype_t type)

/* Define a new global variable <name> of type <type>.
 * The references of <type> are NOT adopted.
 * Return the identifier for the variable.
 */

{
    variable_t dummy;
    typeflags_t flags = type.t_flags;
    int n;

    if (type.t_type == lpctype_void)
    {
        yyerrorf( "Illegal to define variable '%s' as type 'void'"
                , get_txt(name->name));
    }

    name = add_global_name(name);
    /* Prepare the new variable_t */

    if (flags & TYPE_MOD_NOSAVE)
    {
        /* 'nosave' is internally saved as 'static' (historical reason) */
        flags |= TYPE_MOD_STATIC;
        flags ^= TYPE_MOD_NOSAVE;
    }

    /* If the variable already exists, make sure that we can redefine it */
    n = name->u.global.variable;
    if (n != I_GLOBAL_VARIABLE_OTHER && n != I_GLOBAL_VARIABLE_WORLDWIDE)
    {
        typeflags_t vn_flags = VARIABLE(n)->type.t_flags;

        check_variable_redefinition(name, flags);

        /* Make sure that at least one of the two definitions is 'static'.
         * The variable which has not been inherited gets first pick.
         */
        if (flags & NAME_INHERITED)
        {
            flags |= ~(vn_flags) & TYPE_MOD_STATIC;
        }
        else
        {
            vn_flags |=   ~flags & TYPE_MOD_STATIC;
            VARIABLE(n)->type.t_flags = vn_flags;
        }
    }

    type.t_flags = flags;

    dummy.name = ref_mstring(name->name);
    dummy.type = ref_fulltype(type);

    if (flags & TYPE_MOD_VIRTUAL)
    {
        if (!(flags & NAME_HIDDEN))
            name->u.global.variable = VIRTUAL_VAR_TAG | V_VARIABLE_COUNT;
        ADD_VIRTUAL_VAR(&dummy);
    }
    else
    {
        if (!(flags & NAME_HIDDEN))
            name->u.global.variable = NV_VARIABLE_COUNT;
        ADD_VARIABLE(&dummy);
        ADD_GLOBAL_VARIABLE_INFO((global_variable_t){.usage = VAR_USAGE_NONE});
    }

    return name;
} /* define_variable() */