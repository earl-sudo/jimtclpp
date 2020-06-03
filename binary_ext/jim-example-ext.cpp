
#include <jim-config.h>
#include <jim-api.h>
#include <jim-cppapi.h>

#if jim_ext_example

BEGIN_JIM_NAMESPACE

// One actual subcommand.
static Retval exCommand(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    CppApi      jim(interp);
    jim.setResult("you called example command");
    return JRET(JIM_OK);
}

// Define a commands sub-commands.
static const jim_subcmd_type g_fileadv2_subcommand_table[] = { // #JimSubCmdDef
    {   "ex", // subcommand name
        "prompt ?varname?", // prompt given in help for this option.
        exCommand, // actual implementation of command.
        1, // min number of arguments.
        2, // nax number of arguments.
        /* Description: Just an example. */
    },
    {  }
};

// Parser of subcommand
static int exSubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_CallSubCmd(interp, Jim_ParseSubCmd(interp, g_fileadv2_subcommand_table, argc, argv), argc, argv);
}

// called on removal of package.
// Registered in init routine.
static void exDelProc(Jim_InterpPtr interp MAYBE_USED, void* privData MAYBE_USED) {
}


#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-example-ext-version.h>


// Called to setup extension.
JIM_EXPORT Retval Jim_exampleInit(Jim_InterpPtr interp) // #JimCmdInit
{
    CppApi      jim(interp);

    // Give package name and version.
    if (jim.packageProvided("example", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

    // Create a command with subcommands
    jim.ret = jim.createCmd(/* name of parent command */ "ex",
                            /* pases subcommands */  exSubCmdProc, 
                            /* package private data */ nullptr, 
                            /* called on removal of pacakge */ exDelProc);
    if (jim.ret != JIM_OK) return jim.ret;

    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // #if jim_ext_history
