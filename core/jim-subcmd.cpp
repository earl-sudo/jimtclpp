/*
 * Makes it easy to support "ensembles". i.e. commands with subcommands
 * like [string] and [array]
 *
 * (c) 2008 Steve Bennett <steveb@workware.net.au>
 *
 */
#include <stdio.h>
#include <string.h>

#include <jim.h>
#include <jim-subcmd.h>

BEGIN_JIM_NAMESPACE 

/**
 * Implements the common 'commands' subcommand
 */
static Retval subcmd_null(Jim_InterpPtr interp MAYBE_USED, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    /* Nothing to do, since the result has already been created */
    return JIM_OK;
}

/**
 * Do-nothing command_ to support -commands and -usage
 */
static const jim_subcmd_type g_dummy_subcmd = {
    "dummy", NULL, subcmd_null, 0, 0, JIM_MODFLAG_HIDDEN
};

static void add_commands(Jim_InterpPtr interp, const jim_subcmd_type * ct, const char *sep)
{
    const char *s = "";

    for (; ct->cmd; ct++) {
        if (!(ct->flags & JIM_MODFLAG_HIDDEN)) {
            Jim_AppendStrings(interp, Jim_GetResult(interp), s, ct->cmd, NULL);
            s = sep;
        }
    }
}

static void bad_subcmd(Jim_InterpPtr interp, const jim_subcmd_type * command_table, const char *type,
    Jim_ObjPtr cmd, Jim_ObjPtr subcmd)
{
    Jim_SetResultFormatted(interp, "%#s, %s command \"%#s\": should be ", cmd, type, subcmd);
    add_commands(interp, command_table, ", ");
}

static void show_cmd_usage(Jim_InterpPtr interp, const jim_subcmd_type * command_table, int argc MAYBE_USED,
    Jim_ObjConstArray argv)
{
    Jim_SetResultFormatted(interp, "Usage: \"%#s command ... \", where command is one of: ", argv[0]);
    add_commands(interp, command_table, ", ");
}

static void add_cmd_usage(Jim_InterpPtr interp, const jim_subcmd_type * ct, Jim_ObjPtr cmd)
{
    if (cmd) {
        Jim_AppendStrings(interp, Jim_GetResult(interp), Jim_String(cmd), " ", NULL);
    }
    Jim_AppendStrings(interp, Jim_GetResult(interp), ct->cmd, NULL);
    if (ct->args && *ct->args) {
        Jim_AppendStrings(interp, Jim_GetResult(interp), " ", ct->args, NULL);
    }
}

static void set_wrong_args(Jim_InterpPtr interp, const jim_subcmd_type * command_table, Jim_ObjPtr subcmd)
{
    Jim_SetResultString(interp, "wrong # args: should be \"", -1);
    add_cmd_usage(interp, command_table, subcmd);
    Jim_AppendStrings(interp, Jim_GetResult(interp), "\"", NULL);
}

/* internal rep is stored in ptrIntvalue
 *  ptr = command_table
 *  int1 = index
 */
static const Jim_ObjType g_subcmdLookupObjType = { // #JimType
    "subcmd-lookup",
    NULL,
    NULL,
    NULL,
    JIM_TYPE_REFERENCES
};
const Jim_ObjType& subcmdLookupType() { return g_subcmdLookupObjType; }

const jim_subcmd_type *Jim_ParseSubCmd(Jim_InterpPtr interp, const jim_subcmd_type * command_table,
    int argc, Jim_ObjConstArray argv)
{
    const jim_subcmd_type *ct;
    const jim_subcmd_type *partial = 0;
    int cmdlen;
    Jim_ObjPtr cmd;
    const char *cmdstr;
    int help = 0;

    if (argc < 2) {
        Jim_SetResultFormatted(interp, "wrong # args: should be \"%#s command ...\"\n"
            "Use \"%#s -help ?command?\" for help", argv[0], argv[0]);
        return 0;
    }

    cmd = argv[1];

    /* Use cached lookup if possible */
    if (cmd->typePtr() == &g_subcmdLookupObjType) {
        if (cmd->get_ptrInt_ptr() == command_table) {
            ct = command_table + cmd->get_ptrInt_int1();
            goto found;
        }
    }

    /* Check for the help command_ */
    if (Jim_CompareStringImmediate(interp, cmd, "-help")) {
        if (argc == 2) {
            /* Usage for the command_, not the subcommand */
            show_cmd_usage(interp, command_table, argc, argv);
            return &g_dummy_subcmd;
        }
        help = 1;

        /* Skip the 'help' command_ */
        cmd = argv[2];
    }

    /* Check for special builtin '-commands' command_ first */
    if (Jim_CompareStringImmediate(interp, cmd, "-commands")) {
        /* Build the result here */
        Jim_SetResult(interp, Jim_NewEmptyStringObj(interp));
        add_commands(interp, command_table, " ");
        return &g_dummy_subcmd;
    }

    cmdstr = Jim_GetString(cmd, &cmdlen);

    for (ct = command_table; ct->cmd; ct++) {
        if (Jim_CompareStringImmediate(interp, cmd, ct->cmd)) {
            /* Found an exact match */
            break;
        }
        if (strncmp(cmdstr, ct->cmd, cmdlen) == 0) {
            if (partial) {
                /* Ambiguous */
                if (help) {
                    /* Just show the top level_ help here */
                    show_cmd_usage(interp, command_table, argc, argv);
                    return &g_dummy_subcmd;
                }
                bad_subcmd(interp, command_table, "ambiguous", argv[0], argv[1 + help]);
                return 0;
            }
            partial = ct;
        }
        continue;
    }

    /* If we had an unambiguous partial match */
    if (partial && !ct->cmd) {
        ct = partial;
    }

    if (!ct->cmd) {
        /* No matching command_ */
        if (help) {
            /* Just show the top level_ help here */
            show_cmd_usage(interp, command_table, argc, argv);
            return &g_dummy_subcmd;
        }
        bad_subcmd(interp, command_table, "unknown", argv[0], argv[1 + help]);
        return 0;
    }

    if (help) {
        Jim_SetResultString(interp, "Usage: ", -1);
        /* subcmd */
        add_cmd_usage(interp, ct, argv[0]);
        return &g_dummy_subcmd;
    }

    /* Cache the result for a successful non-help lookup */
    Jim_FreeIntRep(interp, cmd);
    cmd->setTypePtr(&g_subcmdLookupObjType);
    cmd->setPtrInt<jim_subcmd_type*>((jim_subcmd_type*) command_table, (int) (ct - command_table));
    //cmd->internalRep.ptrIntValue_.ptr = (void *)command_table;
    //cmd->internalRep.ptrIntValue_.int1 = (int)(ct - command_table);

found:
    /* Check the number of args */
    if (argc - 2 < ct->minargs || (ct->maxargs >= 0 && argc - 2 > ct->maxargs)) {
        Jim_SetResultString(interp, "wrong # args: should be \"", -1);
        /* subcmd */
        add_cmd_usage(interp, ct, argv[0]);
        Jim_AppendStrings(interp, Jim_GetResult(interp), "\"", NULL);

        return 0;
    }

    /* Good command_ */
    return ct;
}

Retval Jim_CallSubCmd(Jim_InterpPtr interp, const jim_subcmd_type *ct, int argc, Jim_ObjConstArray argv)
{
    int ret = JIM_ERR;

    if (ct) {
        if (ct->flags & JIM_MODFLAG_FULLARGV) {
            ret = ct->function(interp, argc, argv);
        }
        else {
            ret = ct->function(interp, argc - 2, argv + 2);
        }
        if (ret < 0) {
            set_wrong_args(interp, ct, argv[0]);
            ret = JIM_ERR;
        }
    }
    return ret;
}

Retval Jim_SubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv)
{
    const jim_subcmd_type *ct =
        Jim_ParseSubCmd(interp, (const jim_subcmd_type *)Jim_CmdPrivData(interp), argc, argv);

    return Jim_CallSubCmd(interp, ct, argc, argv);
}

END_JIM_NAMESPACE
