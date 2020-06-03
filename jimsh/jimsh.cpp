/*
 * jimsh - An interactive shell for Jim
 *
 * Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
 * Copyright 2009 Steve Bennett <steveb@workware.net.au>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE JIM TCL PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * JIM TCL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the Jim Tcl Project.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jim.h>
#include <jim-config.h>

#ifndef _WIN32
extern char** environ;
#endif

BEGIN_JIM_NAMESPACE 

/* From initjimsh.tcl */
extern int Jim_initjimshInit(Jim_InterpPtr interp);

static void JimSetArgv(Jim_InterpPtr interp, int argc, char *const argv[])
{
    int n;
    Jim_ObjPtr listObj = Jim_NewListObj(interp, nullptr, 0);

    /* Populate argv global var */
    for (n = 0; n < argc; n++) {
        Jim_ObjPtr obj = Jim_NewStringObj(interp, argv[n], -1);

        Jim_ListAppendElement(interp, listObj, obj);
    }

    IGNOREJIMRET Jim_SetVariableStr(interp, "argv", listObj);
    IGNOREJIMRET Jim_SetVariableStr(interp, "argc", Jim_NewIntObj(interp, argc));
}

static void JimPrintErrorMessage(Jim_InterpPtr interp)
{
    Jim_MakeErrorMessage(interp);
    fprintf(stderr, "%s\n", Jim_String(Jim_GetResult(interp)));
}


#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const int version[] = { MAJOR , MINOR };
#include <jimtclpp-version.h>

void usage(const char* executable_name)
{

    const char* format = 
    R"helptext(jimsh version %d.%d\n
Usage: %s
or   : %s [options] [filename]

Without options: Interactive mode

Options:
      --version  : prints the version string
      --help     : prints this text
      -e CMD     : executes command CMD
                   NOTE: all subsequent options will be passed as arguments to the command
    [filename|-] : executes the script contained in the named file, or from stdin if "-"
                   NOTE: all subsequent options will be passed to the script
    )helptext";
    IGNOREPOSIXRET printf(format, version[0], version[1], executable_name, executable_name); // #stdoutput
}

END_JIM_NAMESPACE

using namespace Jim;

int main(int argc, char *const argv[])
{
    int retcode = JRET(JIM_ERR);
    Jim_InterpPtr interp;
    char *const orig_argv0 = argv[0];

    /* Parse initial arguments before interpreter is started */
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        IGNOREPOSIXRET printf("%d.%d\n", version[0], version[1]); // #stdoutput
        return 0;
    }
    else if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usage(argv[0]);
        return 0;
    }

    try { // #try
        /* Create and initialize the interpreter */
        interp = Jim_CreateInterp();
        Jim_RegisterCoreCommands(interp);

        /* Register static extensions */
        if (Jim_InitStaticExtensions(interp) != JRET(JIM_OK)) {
            JimPrintErrorMessage(interp);
        }

        IGNORE_NOREAL_ERROR Jim_SetVariableStrWithStr(interp, "jim::argv0", orig_argv0);
        IGNORE_NOREAL_ERROR Jim_SetVariableStrWithStr(interp, JIM_INTERACTIVE, argc == 1 ? "1" : "0");
        retcode = Jim_initjimshInit(interp);

        if (argc == 1) {
            /* Executable name_ is the only argument - start interactive prompt */
            if (retcode == JRET(JIM_ERR)) {
                JimPrintErrorMessage(interp);
            }
            if (retcode != JRET(JIM_EXIT)) {
                JimSetArgv(interp, 0, nullptr);
                retcode = Jim_InteractivePrompt(interp);
            }
        } else {
            /* Additional arguments - interpret them */
            if (argc > 2 && strcmp(argv[1], "-e") == 0) {
                /* Evaluate code in subsequent argument */
                JimSetArgv(interp, argc - 3, argv + 3);
                retcode = Jim_Eval(interp, argv[2]);
                if (retcode != JRET(JIM_ERR)) {
                    IGNOREPOSIXRET printf("%s\n", Jim_String(Jim_GetResult(interp))); // #stdoutput
                }
            } else {
                IGNOREJIMRET Jim_SetVariableStr(interp, "argv0", Jim_NewStringObj(interp, argv[1], -1));
                JimSetArgv(interp, argc - 2, argv + 2);
                if (strcmp(argv[1], "-") == 0) {
                    retcode = Jim_Eval(interp, "eval [info source [stdin read] stdin 1]");
                } else {
                    retcode = Jim_EvalFile(interp, argv[1]);
                }
            }
            if (retcode == JRET(JIM_ERR)) {
                JimPrintErrorMessage(interp);
            }
        }
        if (retcode == JRET(JIM_EXIT)) {
            retcode = Jim_GetExitCode(interp);
        } else if (retcode == JRET(JIM_ERR)) {
            retcode = 1;
        } else {
            retcode = 0;
        }
        Jim_FreeInterp(interp);
    } catch (std::exception& e) { // #catch 
        printf("Exception %s\n", e.what());
    } catch (...) { // #catch 
        printf("Unknown exception caught!\n");
    }
    return retcode;
}
