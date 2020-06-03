/*
 * Jim - POSIX extension
 *
 * Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
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

#include <jim-config.h>

#if jim_ext_posix

#ifndef _WIN32

#ifdef HAVE_UNISTD_H
#  include <unistd.h> // #NonPortHeader
#endif


#include <jim.h>

#include <jim.h> // #TODO replace with <jim-api.h>
#include <prj_compat.h>

BEGIN_JIM_NAMESPACE

static void Jim_PosixSetError(Jim_InterpPtr interp_)
{
    Jim_SetResultString(interp_, strerror(errno), -1);
}

static Retval Jim_PosixForkCommand(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    pid_t pid = 0;

    JIM_NOTUSED(argv);

    if (argc != 1) {
        Jim_WrongNumArgs(interp_, 1, argv, "");
        return JRET(JIM_ERR);
    }
    if (prj_funcDef(prj_fork)) { // #NonPortFuncFix
        if ((pid = prj_fork()) == -1) {
            Jim_PosixSetError(interp_);
            return JRET(JIM_ERR);
        }
    }
    Jim_SetResultInt(interp_, (jim_wide) pid);
    return JRET(JIM_OK);
}

static Retval Jim_PosixGetidsCommand(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    Jim_ObjPtr objv[8];

    if (argc != 1) {
        Jim_WrongNumArgs(interp_, 1, argv, "");
        return JRET(JIM_ERR);
    }
    objv[0] = Jim_NewStringObj(interp_, "uid", -1);
    objv[1] = Jim_NewIntObj(interp_, getuid());
    objv[2] = Jim_NewStringObj(interp_, "euid", -1);
    objv[3] = Jim_NewIntObj(interp_, prj_geteuid()); // #NonPortFuncFix
    objv[4] = Jim_NewStringObj(interp_, "gid", -1);
    objv[5] = Jim_NewIntObj(interp_, getgid());
    objv[6] = Jim_NewStringObj(interp_, "egid", -1);
    objv[7] = Jim_NewIntObj(interp_, getegid());
    Jim_SetResult(interp_, Jim_NewListObj(interp_, objv, 8));
    return JRET(JIM_OK);
}

#define JIM_HOST_NAME_MAX 1024
static Retval Jim_PosixGethostnameCommand(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    char *buf;
    int rc = JRET(JIM_OK);

    if (argc != 1) {
        Jim_WrongNumArgs(interp_, 1, argv, "");
        return JRET(JIM_ERR);
    }
    buf = (char*)Jim_Alloc(JIM_HOST_NAME_MAX);
    if (prj_gethostname(buf, JIM_HOST_NAME_MAX) == -1) { // #NonPortFuncFix #SockFunc
        Jim_PosixSetError(interp_);
        Jim_Free(buf);
        rc = JRET(JIM_ERR);
    }
    else {
        Jim_SetResult(interp_, Jim_NewStringObjNoAlloc(interp_, buf, -1));
    }
    return rc;
}

static Retval Jim_PosixUptimeCommand(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    if (prj_funcDef(prj_sysinfo)) { // #optionalCode
        struct prj_sysinfo info;

        if (argc != 1) {
            Jim_WrongNumArgs(interp_, 1, argv, "");
            return JRET(JIM_ERR);
        }

        if (prj_sysinfo(&info) == -1) { // #NonPortFuncFix
            Jim_PosixSetError(interp_);
            return JRET(JIM_ERR);
        }

        Jim_SetResultInt(interp_, prj_sysinfo_uptime(&info));
    } else {
        Jim_SetResultInt(interp_, (long) time(nullptr));
    }
    return JRET(JIM_OK);
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-posix-version.h>

JIM_EXPORT Retval Jim_posixInit(Jim_InterpPtr interp_) // #JimCmdInit
{
    if (Jim_PackageProvide(interp_, "posix", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

    Retval ret = JIM_ERR;

    if (prj_funcDef(prj_fork)) { // #NonPortFuncFix
        ret = Jim_CreateCommand(interp_, "os.fork", Jim_PosixForkCommand, nullptr, nullptr);
        if (ret != JIM_OK) return ret;
    }
    ret = Jim_CreateCommand(interp_, "os.getids", Jim_PosixGetidsCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;

    ret = Jim_CreateCommand(interp_, "os.gethostname", Jim_PosixGethostnameCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;

    ret = Jim_CreateCommand(interp_, "os.uptime", Jim_PosixUptimeCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;

    return JRET(JIM_OK);
}


#else
#include <jim-api.h>

BEGIN_JIM_NAMESPACE

JIM_EXPORT Retval Jim_posixInit(Jim_InterpPtr interp_) // #JimCmdInit
{
    return JRET(JIM_OK);
}
#endif /* ifndef _WIN32 */

END_JIM_NAMESPACE

#endif // #if jim_ext_posix


