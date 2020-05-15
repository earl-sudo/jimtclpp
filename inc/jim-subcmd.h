#pragma once

/* Provides a common approach to implementing Tcl commands
 * which implement subcommands
 */

#include <jim-api.h>

BEGIN_JIM_NAMESPACE 

#define JIM_MODFLAG_HIDDEN   0x0001		/* Don't show the subcommand in usage or commands */
#define JIM_MODFLAG_FULLARGV 0x0002		/* Subcmd proc gets called with full argv */

/* Custom flags start at 0x0100 */

/**
 * Returns JIM_OK if OK, JIM_ERR (etc.) on errorText_, break, continue, etc.
 * Returns -1 if invalid args.
 */
typedef int jim_subcmd_function(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv); 

struct jim_subcmd_type {
	const char *cmd;				/* Name of the (sub)command_ */
	const char *args;				/* Textual description of allowed args */
	jim_subcmd_function *function;	/* Function implementing the subcommand */
	short minargs;					/* Minimum required arguments */
	short maxargs;					/* Maximum allowed arguments or -1 if no limit */
	unsigned_short flags;			/* JIM_MODFLAG_... plus custom flags */
};

/**
 * Looks up the appropriate subcommand in the given command_ table and return
 * the command_ function which implements the subcommand.
 * NULL will be returned and an appropriate errorText_ will be set if the subcommand or
 * arguments are invalid.
 *
 * Typical usage is:
 *  {
 *    const jim_subcmd_type *ct = Jim_ParseSubCmd(interp_, command_table, argc, argv);
 *
 *    return Jim_CallSubCmd(interp_, ct, argc, argv);
 *  }
 *
 */
const jim_subcmd_type *
Jim_ParseSubCmd(Jim_InterpPtr interp, const jim_subcmd_type *command_table, int argc, Jim_ObjConstArray argv);

/**
 * Parses the args against the given command_ table and executes the subcommand if found
 * or sets an appropriate errorText_ if the subcommand or arguments is invalid.
 *
 * Can be used directly with Jim_CreateCommand() where the ClientData is the command_ table.
 *
 * e.g. Jim_CreateCommand(interp_, "mycmd", Jim_SubCmdProc, command_table, NULL);
 */
Retval Jim_SubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);

/**
 * Invokes the given subcmd with the given args as returned
 * by Jim_ParseSubCmd()
 *
 * If ct is NULL, returns JIM_ERR, leaving any message.
 * Otherwise invokes ct->function
 *
 * If ct->function returns -1, sets an errorText_ message and returns JIM_ERR.
 * Otherwise returns the result of ct->function.
 */
Retval Jim_CallSubCmd(Jim_InterpPtr interp, const jim_subcmd_type *ct, int argc, Jim_ObjConstArray argv);

END_JIM_NAMESPACE
