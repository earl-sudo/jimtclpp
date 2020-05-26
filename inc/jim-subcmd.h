#pragma once

/* Provides a common approach to implementing Tcl commands
 * which implement subcommands
 */

#include <jim-api.h>

BEGIN_JIM_NAMESPACE 

#define JIM_MODFLAG_HIDDEN   0x0001		/* Don't show the subcommand in usage or commands */
#define JIM_MODFLAG_FULLARGV 0x0002		/* Subcmd proc gets called with full argv */

/* Custom flags_ start at 0x0100 */

/**
 * Returns JRET(JIM_OK) if OK, JRET(JIM_ERR) (etc.) on errorText_, break, continue, etc.
 * Returns -1 if invalid args_.
 */
typedef int jim_subcmd_function(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv); 

struct jim_subcmd_type {
	const char *cmd_ = NULL;	 	    /* Name of the (sub)command_ */
	const char *args_ = NULL;	    /* Textual description of allowed args_ */
	jim_subcmd_function *function_ = NULL;	/* Function implementing the subcommand */
	short minargs_ = 0;				/* Minimum required arguments */
	short maxargs_ = 0;				/* Maximum allowed arguments or -1 if no limit */
	unsigned_short flags_ = 0;		/* JIM_MODFLAG_... plus custom flags_ */

	jim_subcmd_type(
		const char* cmdD, const char* argsD, 
		jim_subcmd_function funcD, 
		short minargsD, short maxargsD,
		unsigned_short flagsD = 0
	) : cmd_(cmdD), args_(argsD), function_(funcD), 
		minargs_(minargsD), maxargs_(maxargsD), flags_(flagsD) { }
	jim_subcmd_type() { }
};

/**
 * Looks up the appropriate subcommand in the given command_ table and return
 * the command_ function_ which implements the subcommand.
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
 * Parses the args_ against the given command_ table and executes the subcommand if found
 * or sets an appropriate errorText_ if the subcommand or arguments is invalid.
 *
 * Can be used directly with Jim_CreateCommand() where the ClientData is the command_ table.
 *
 * e.g. Jim_CreateCommand(interp_, "mycmd", Jim_SubCmdProc, command_table, NULL);
 */
Retval Jim_SubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);

/**
 * Invokes the given subcmd with the given args_ as returned
 * by Jim_ParseSubCmd()
 *
 * If ct is NULL, returns JRET(JIM_ERR), leaving any message.
 * Otherwise invokes ct->function_
 *
 * If ct->function_ returns -1, sets an errorText_ message and returns JRET(JIM_ERR).
 * Otherwise returns the result of ct->function_.
 */
Retval Jim_CallSubCmd(Jim_InterpPtr interp, const jim_subcmd_type *ct, int argc, Jim_ObjConstArray argv);

END_JIM_NAMESPACE
