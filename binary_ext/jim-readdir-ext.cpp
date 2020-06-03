/*
 * Tcl readdir command_.
 *
 * (c) 2008 Steve Bennett <steveb@worware.net.au>
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
 *
 * Based on original work by:
 *-----------------------------------------------------------------------------
 * Copyright 1991-1994 Karl Lehenbauer and Mark Diekhans.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies.  Karl Lehenbauer and
 * Mark Diekhans make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *-----------------------------------------------------------------------------
 */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <jim-config.h>

#include <jim-api.h>
#include <prj_compat.h>
#include <readdir.h>

#if jim_ext_readdir

BEGIN_JIM_NAMESPACE

/*
 *-----------------------------------------------------------------------------
 *
 * Jim_ReaddirCmd --
 *     Implements the rename TCL command_:
 *         readdir ?-nocomplain? dirPath
 *
 * Results:
 *      Standard TCL result.
 *-----------------------------------------------------------------------------
 */
Retval Jim_ReaddirCmd(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char *dirPath;

    int nocomplain = 0;

    if (argc == 3 && Jim_CompareStringImmediate(interp, argv[1], "-nocomplain")) {
        nocomplain = 1;
    }
    if (argc != 2 && !nocomplain) {
        Jim_WrongNumArgs(interp, 1, argv, "?-nocomplain? dirPath");
        return JRET(JIM_ERR);
    }

    dirPath = Jim_String(argv[1 + nocomplain]);

    {
        Readdir     rd(dirPath);
        std::string name;

        if (rd.inError_) {
            if (nocomplain) {
                return JRET(JIM_OK);
            }
            // TODO errno not going to work on Windows.
            Jim_SetResultString(interp, rd.getError().c_str(), -1);
            return JRET(JIM_ERR);
        }

        Jim_ObjPtr listObj = Jim_NewListObj(interp, nullptr, 0);
        while ((name = rd.nextName()).length()) {
            Jim_ListAppendElement(interp, listObj, 
                                  Jim_NewStringObj(interp, name.c_str(), -1));
        }
        Jim_SetResult(interp, listObj);

    }

    return JRET(JIM_OK);
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-readdir-version.h>

Retval Jim_readdirInit(Jim_InterpPtr interp)
{
    if (Jim_PackageProvide(interp, "readdir", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

    Retval ret = JIM_ERR;

    ret = Jim_CreateCommand(interp, "readdir", Jim_ReaddirCmd, nullptr, nullptr);
    if (ret != JIM_OK) return ret;

    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // jim_ext_readdir
