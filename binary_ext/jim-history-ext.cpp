
#include <jimautoconf.h>
#include <jim-api.h>

#if jim_ext_history

BEGIN_JIM_NAMESPACE

static Retval history_cmd_getline(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_ObjPtr objPtr;
    char *line = Jim_HistoryGetline(interp, Jim_String(argv[0]));

    /* On EOF returns -1 if varName was specified; otherwise the empty string. */
    if (line == NULL) {
        if (argc == 2) {
            Jim_SetResultInt(interp, -1);
        }
        return JIM_OK;
    }

    objPtr = Jim_NewStringObjNoAlloc(interp, line, -1);

    /* Returns the length of the string if varName was specified */
    if (argc == 2) {
        if (Jim_SetVariable(interp, argv[1], objPtr) != JIM_OK) {
            Jim_FreeObj(interp, objPtr);
            return JIM_ERR;
        }
        Jim_SetResultInt(interp, Jim_Length(objPtr));
    }
    else {
        Jim_SetResult(interp, objPtr);
    }
    return JIM_OK;
}

static Retval history_cmd_setcompletion(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_HistorySetCompletion(interp, Jim_Length(argv[0]) ? argv[0] : NULL);
    return JIM_OK;
}

static Retval history_cmd_load(Jim_InterpPtr interp MAYBE_USED, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_HistoryLoad(Jim_String(argv[0]));
    return JIM_OK;
}

static Retval history_cmd_save(Jim_InterpPtr interp MAYBE_USED, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_HistorySave(Jim_String(argv[0]));
    return JIM_OK;
}

static Retval history_cmd_add(Jim_InterpPtr interp MAYBE_USED, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_HistoryAdd(Jim_String(argv[0]));
    return JIM_OK;
}

static Retval history_cmd_show(Jim_InterpPtr interp MAYBE_USED, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    Jim_HistoryShow();
    return JIM_OK;
}

static const jim_subcmd_type g_history_command_table[] = { // #JimSubCmdDef
    {   "getline",
        "prompt ?varname?",
        history_cmd_getline,
        1,
        2,
        /* Description: Reads one lineNum_ from the user. Similar to gets. */
    },
    {   "completion",
        "command",
        history_cmd_setcompletion,
        1,
        1,
        /* Description: Sets an autocompletion callback command_, or none if "" */
    },
    {   "load",
        "filename",
        history_cmd_load,
        1,
        1,
        /* Description: Loads history from the given file, if possible */
    },
    {   "save",
        "filename",
        history_cmd_save,
        1,
        1,
        /* Description: Saves history to the given file */
    },
    {   "add",
        "line",
        history_cmd_add,
        1,
        1,
        /* Description: Adds the lineNum_ to the history ands saves */
    },
    {   "show",
        NULL,
        history_cmd_show,
        0,
        0,
        /* Description: Displays the history */
    },
    {  }
};

static int JimHistorySubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_CallSubCmd(interp, Jim_ParseSubCmd(interp, g_history_command_table, argc, argv), argc, argv);
}

static void JimHistoryDelProc(Jim_InterpPtr interp MAYBE_USED, void *privData)
{
    Jim_TFree<void>(privData,"void"); // #FreeF 
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-history-version.h>

Retval Jim_historyInit(Jim_InterpPtr interp) // #JimCmdInit
{
    VoidPtrArray*  history;
    if (Jim_PackageProvide(interp, "history", version, JIM_ERRMSG))
        return JIM_ERR;

    history = Jim_TAlloc<VoidPtrArray>(1,"VoidPtrArray"); // #AllocF 
    *history = NULL;

    IGNORERET Jim_CreateCommand(interp, "history", JimHistorySubCmdProc, history, JimHistoryDelProc);
    return JIM_OK;
}

END_JIM_NAMESPACE

#endif // #if jim_ext_history
