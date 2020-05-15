/*
 * Implements the file command_ for jim
 *
 * (c) 2008 Steve Bennett <steveb@workware.net.au>
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
 * Based on code originally from Tcl 6.7:
 *
 * Copyright 1987-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <sys/stat.h>
#ifdef _WIN32 // #TODO add a HAVE_IO_H
#include <io.h>
#endif

#include <jimautoconf.h>

#include <jim-api.h>
#include <jim-cpp-api.h>

#if 0

#ifdef HAVE_UNISTD_H // #optionalCode #WinOff
#include <unistd.h> // #NonPortHeader
#elif defined(_MSC_VER)
#include <direct.h> // #NonPortHeader
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#include <prj_compat.h>

BEGIN_JIM_NAMESPACE

#ifndef MAXPATHLEN // #optionalCode
#  define MAXPATHLEN JIM_PATH_LEN
#endif

#if defined(__MINGW32__) || defined(__MSYS__) || defined(_MSC_VER) // #optionalCode #WinOff
#define ISWINDOWS 1
#else
#define ISWINDOWS 0
#endif

/* extract nanosecond resolution mtime from struct stat */
#if defined(HAVE_STRUCT_STAT_ST_MTIMESPEC) // #optionalCode #WinOff
#define STAT_MTIME_US(STAT) ((STAT).st_mtimespec.tv_sec * 1000000ll + (STAT).st_mtimespec.tv_nsec / 1000)
#elif defined(HAVE_STRUCT_STAT_ST_MTIM)
#define STAT_MTIME_US(STAT) ((STAT).st_mtim.tv_sec * 1000000ll + (STAT).st_mtim.tv_nsec / 1000)
#endif
#ifdef PRJ_OS_MACOS
#  undef STAT_MTIME_US
#endif

/*
 *----------------------------------------------------------------------
 *
 * JimGetFileType --
 *
 *  Given a mode word, returns a string identifying the tokenType_ of a
 *  file.
 *
 * Results:
 *  A static text string giving the file tokenType_ from mode.
 *
 * Side effects:
 *  None.
 *
 *----------------------------------------------------------------------
 */

    static const char* JimGetFileType(int mode) {
    if (S_ISREG(mode)) {
        return "file";
    } else if (S_ISDIR(mode)) {
        return "directory";
    }
#ifdef S_ISCHR // #optionalCode #WinOff
    else if (S_ISCHR(mode)) {
        return "characterSpecial";
    }
#endif
#ifdef S_ISBLK // #optionalCode #WinOff
    else if (S_ISBLK(mode)) {
        return "blockSpecial";
    }
#endif
#ifdef S_ISFIFO // #optionalCode #WinOff
    else if (S_ISFIFO(mode)) {
        return "fifo";
    }
#endif
#ifdef S_ISLNK // #optionalCode #WinOff
    else if (S_ISLNK(mode)) {
        return "link";
    }
#endif
#ifdef S_ISSOCK // #optionalCode #WinOff
    else if (S_ISSOCK(mode)) {
        return "socket";
    }
#endif
    return "unknown";
}

/*
 *----------------------------------------------------------------------
 *
 * StoreStatData --
 *
 *  This is a utility procedure that breaks out the fields of a
 *  "stat" structure and stores them in textual form into the
 *  elements of an associative array.
 *
 * Results:
 *  Returns a standard Tcl return value.  If an errorText_ occurs then
 *  a message is left_ in interp_->result.
 *
 * Side effects:
 *  Elements of the associative array given by "varName" are modified.
 *
 *----------------------------------------------------------------------
 */
static void AppendStatElement(Jim_InterpPtr interp_, Jim_ObjPtr listObj, const char* key, jim_wide value) {
    Jim_ListAppendElement(interp_, listObj, Jim_NewStringObj(interp_, key, -1));
    Jim_ListAppendElement(interp_, listObj, Jim_NewIntObj(interp_, value));
}

static Retval StoreStatData(Jim_InterpPtr interp_, Jim_ObjPtr varName, const struct stat* sb) {
    /* Just use a list to store the data_ */
    Jim_ObjPtr listObj = Jim_NewListObj(interp_, NULL, 0);

    AppendStatElement(interp_, listObj, "dev", sb->st_dev);
    AppendStatElement(interp_, listObj, "ino", sb->st_ino);
    AppendStatElement(interp_, listObj, "mode", sb->st_mode);
    AppendStatElement(interp_, listObj, "nlink", sb->st_nlink);
    AppendStatElement(interp_, listObj, "uid", sb->st_uid);
    AppendStatElement(interp_, listObj, "gid", sb->st_gid);
    AppendStatElement(interp_, listObj, "size", sb->st_size);
    AppendStatElement(interp_, listObj, "atime", sb->st_atime);
    AppendStatElement(interp_, listObj, "mtime", sb->st_mtime);
    AppendStatElement(interp_, listObj, "ctime", sb->st_ctime);
#ifdef STAT_MTIME_US // #optionalCode #WinOff
    AppendStatElement(interp_, listObj, "mtimeus", STAT_MTIME_US(*sb));
#endif
    Jim_ListAppendElement(interp_, listObj, Jim_NewStringObj(interp_, "type", -1));
    Jim_ListAppendElement(interp_, listObj, Jim_NewStringObj(interp_, JimGetFileType((int) sb->st_mode), -1));

    /* Was a variable specified? */
    if (varName) {
        Jim_ObjPtr objPtr_;
        objPtr_ = Jim_GetVariable(interp_, varName, JIM_NONE);

        if (objPtr_) {
            Jim_ObjPtr objv[2];

            objv[0] = objPtr_;
            objv[1] = listObj;

            objPtr_ = Jim_DictMerge(interp_, 2, objv);
            if (objPtr_ == NULL) {
                /* This message matches the one from Tcl */
                Jim_SetResultFormatted(interp_, "can't set \"%#s(dev)\": variable isn't array", varName);
                Jim_FreeNewObj(interp_, listObj);
                return JIM_ERR;
            }

            Jim_InvalidateStringRep(objPtr_);

            Jim_FreeNewObj(interp_, listObj);
            listObj = objPtr_;
        }
        Jim_SetVariable(interp_, varName, listObj);
    }

    /* And also return the value */
    Jim_SetResult(interp_, listObj);

    return JIM_OK;
}

static Retval file_cmd_dirname(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* path = Jim_String(argv[0]);
    const char* p = strrchr(path, '/');

    if (!p && path[0] == '.' && path[1] == '.' && path[2] == '\0') {
        Jim_SetResultString(interp_, "..", -1);
    } else if (!p) {
        Jim_SetResultString(interp_, ".", -1);
    } else if (p == path) {
        Jim_SetResultString(interp_, "/", -1);
    } else if (ISWINDOWS && p[-1] == ':') {
        /* z:/dir => z:/ */
        Jim_SetResultString(interp_, path, (int) (p - path + 1));
    } else {
        Jim_SetResultString(interp_, path, (int) (p - path));
    }
    return JIM_OK;
}

static Retval file_cmd_rootname(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* path = Jim_String(argv[0]);
    const char* lastSlash = strrchr(path, '/');
    const char* p = strrchr(path, '.');

    if (p == NULL || (lastSlash != NULL && lastSlash > p)) {
        Jim_SetResult(interp_, argv[0]);
    } else {
        Jim_SetResultString(interp_, path, (int) (p - path));
    }
    return JIM_OK;
}

static Retval file_cmd_extension(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* path = Jim_String(argv[0]);
    const char* lastSlash = strrchr(path, '/');
    const char* p = strrchr(path, '.');

    if (p == NULL || (lastSlash != NULL && lastSlash >= p)) {
        p = "";
    }
    Jim_SetResultString(interp_, p, -1);
    return JIM_OK;
}

static Retval file_cmd_tail(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* path = Jim_String(argv[0]);
    const char* lastSlash = strrchr(path, '/');

    if (lastSlash) {
        Jim_SetResultString(interp_, lastSlash + 1, -1);
    } else {
        Jim_SetResult(interp_, argv[0]);
    }
    return JIM_OK;
}

static Retval file_cmd_normalize(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    if (prj_funcDef(prj_realpath)) // #Unsupported #NonPortFuncFix
    {
        const char* path = Jim_String(argv[0]);
        char* newname = new_CharArray(MAXPATHLEN + 1); // #AllocF 

        if (prj_realpath(path, newname)) {
            Jim_SetResult(interp_, Jim_NewStringObjNoAlloc(interp_, newname, -1));
            return JIM_OK;
        } else {
            free_CharArray(newname); // #FreeF 
            Jim_SetResultFormatted(interp_, "can't normalize \"%#s\": %s", argv[0], strerror(errno));
            return JIM_ERR;
        }
    } else {
        Jim_SetResultString(interp_, "Not implemented", -1);
    }
    return JIM_ERR;
}

static Retval file_cmd_join(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int i;
    char* newname = new_CharArray(MAXPATHLEN + 1); // #AllocF 
    char* last = newname;

    *newname = 0;

    /* Simple implementation for now */
    for (i = 0; i < argc; i++) {
        int len_;
        const char* part = Jim_GetString(argv[i], &len_);

        if (*part == '/') {
            /* Absolute component, so go back to the start */
            last = newname;
        } else if (ISWINDOWS && strchr(part, ':')) {
            /* Absolute component on mingw, so go back to the start */
            last = newname;
        } else if (part[0] == '.') {
            if (part[1] == '/') {
                part += 2;
                len_ -= 2;
            } else if (part[1] == 0 && last != newname) {
                /* Adding '.' to an existing path does nothing */
                continue;
            }
        }

        /* Add a slash if needed */
        if (last != newname && last[-1] != '/') {
            *last++ = '/';
        }

        if (len_) {
            if (last + len_ - newname >= MAXPATHLEN) {
                free_CharArray(newname); // #FreeF 
                Jim_SetResultString(interp_, "Path too long", -1);
                return JIM_ERR;
            }
            memcpy(last, part, len_);
            last += len_;
        }

        /* Remove a slash if needed */
        if (last > newname + 1 && last[-1] == '/') {
            /* but on on Windows, leave the trailing slash on "c:/ " */
            if (!ISWINDOWS || !(last > newname + 2 && last[-2] == ':')) {
                *--last = 0;
            }
        }
    }

    *last = 0;

    /* Probably need to handle some special cases ... */

    Jim_SetResult(interp_, Jim_NewStringObjNoAlloc(interp_, newname,
                                                  (int) (last - newname)));

    return JIM_OK;
}

static Retval file_access(Jim_InterpPtr interp_, Jim_ObjPtr filename, int mode) {
    Jim_SetResultBool(interp_, prj_access(Jim_String(filename), mode) != -1); // #NonPortFuncFix

    return JIM_OK;
}

static Retval file_cmd_readable(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return file_access(interp_, argv[0], R_OK);
}

static Retval file_cmd_writable(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return file_access(interp_, argv[0], W_OK);
}

static Retval file_cmd_executable(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
#ifdef X_OK // #optionalCode #WinOff
    return file_access(interp_, argv[0], X_OK);
#else
    /* If no X_OK, just assume true. */
    Jim_SetResultBool(interp_, 1);
    return JIM_OK;
#endif
}

static Retval file_cmd_exists(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return file_access(interp_, argv[0], F_OK);
}

static Retval file_cmd_delete(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int force = Jim_CompareStringImmediate(interp_, argv[0], "-force");

    if (force || Jim_CompareStringImmediate(interp_, argv[0], "--")) {
        argc++;
        argv--;
    }

    while (argc--) {
        const char* path = Jim_String(argv[0]);

        if (prj_unlink(path) == -1 && errno != ENOENT) { // #NonPortFuncFix
            if (prj_rmdir(path) == -1) { // #NonPortFuncFix
                /* Maybe try using the script helper */
                if (!force || Jim_EvalPrefix(interp_, "file delete force", 1, argv) != JIM_OK) {
                    Jim_SetResultFormatted(interp_, "couldn't delete file \"%s\": %s", path,
                                           strerror(errno));
                    return JIM_ERR;
                }
            }
        }
        argv++;
    }
    return JIM_OK;
}

/**
 * Create directory, creating all intermediate paths if necessary.
 *
 * Returns 0 if OK or -1 on failure (and sets errno)
 *
 * Note: The path may be modified.
 */
static int mkdir_all(char* path) {
    int ok = 1;

    /* First time just try to make the dir */
    goto first;

    while (ok--) {
        /* Must have failed the first time, so recursively make the parent and try again */
        {
            char* slash = strrchr(path, '/');

            if (slash && slash != path) {
                *slash = 0;
                if (mkdir_all(path) != 0) {
                    return -1;
                }
                *slash = '/';
            }
        }
first:
        if (prj_mkdir2(path, 0755) == 0) {
            return 0;
        }
        if (errno == ENOENT) {
            /* Create the parent and try again */
            continue;
        }
        /* Maybe it already exists as a directory */
        if (errno == EEXIST) {
            struct stat sb;

            if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                return 0;
            }
            /* Restore errno */
            errno = EEXIST;
        }
        /* Failed */
        break;
    }
    return -1;
}

static Retval file_cmd_mkdir(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    while (argc--) {
        char* path = Jim_StrDup(Jim_String(argv[0]));
        int rc = mkdir_all(path);

        free_CharArray(path); // #FreeF 
        if (rc != 0) {
            Jim_SetResultFormatted(interp_, "can't create directory \"%#s\": %s", argv[0],
                                   strerror(errno));
            return JIM_ERR;
        }
        argv++;
    }
    return JIM_OK;
}

static Retval file_cmd_tempfile(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int fd = Jim_MakeTempFile(interp_, (argc >= 1) ? Jim_String(argv[0]) : NULL, 0);

    if (fd < 0) {
        return JIM_ERR;
    }
    prj_close(fd); // #NonPortFuncFix

    return JIM_OK;
}

static Retval file_cmd_rename(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* source;
    const char* dest;
    int force = 0;

    if (argc == 3) {
        if (!Jim_CompareStringImmediate(interp_, argv[0], "-force")) {
            return -1;
        }
        force++;
        argv++;
        argc--;
    }

    source = Jim_String(argv[0]);
    dest = Jim_String(argv[1]);

    if (!force && prj_access(dest, F_OK) == 0) { // #NonPortFuncFix
        Jim_SetResultFormatted(interp_, "error renaming \"%#s\" to \"%#s\": target exists", argv[0],
                               argv[1]);
        return JIM_ERR;
    }

    if (rename(source, dest) != 0) {
        Jim_SetResultFormatted(interp_, "error renaming \"%#s\" to \"%#s\": %s", argv[0], argv[1],
                               strerror(errno));
        return JIM_ERR;
    }

    return JIM_OK;
}

static Retval file_cmd_link(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int ret;
    const char* source;
    const char* dest;
    static const char* const options[] = { "-hard", "-symbolic", NULL };
    enum { OPT_HARD, OPT_SYMBOLIC, };
    int option = OPT_HARD;

    if (argc == 3) {
        if (Jim_GetEnum(interp_, argv[0], options, &option, NULL, JIM_ENUM_ABBREV | JIM_ERRMSG) != JIM_OK) {
            return JIM_ERR;
        }
        argv++;
        argc--;
    }

    dest = Jim_String(argv[0]);
    source = Jim_String(argv[1]);

    if (option == OPT_HARD) {
        ret = prj_link(source, dest); // #NonPortFuncFix
    } else {
        ret = prj_symlink(source, dest); // #NonPortFuncFix
    }

    if (ret != 0) {
        Jim_SetResultFormatted(interp_, "error linking \"%#s\" to \"%#s\": %s", argv[0], argv[1],
                               strerror(errno));
        return JIM_ERR;
    }

    return JIM_OK;
}

static Retval file_stat(Jim_InterpPtr interp_, Jim_ObjPtr filename, struct stat* sb) {
    const char* path = Jim_String(filename);

    if (stat(path, sb) == -1) {
        Jim_SetResultFormatted(interp_, "could not read \"%#s\": %s", filename, strerror(errno));
        return JIM_ERR;
    }
    return JIM_OK;
}

#ifdef HAVE_LSTAT // #optionalCode #WinOff
static Retval file_lstat(Jim_InterpPtr  interp_, Jim_ObjPtr  filename, struct stat* sb) {
    const char* path = Jim_String(filename);

    if (lstat(path, sb) == -1) {
        Jim_SetResultFormatted(interp_, "could not read \"%#s\": %s", filename, strerror(errno));
        return JIM_ERR;
    }
    return JIM_OK;
}
#else
#define file_lstat file_stat
#endif

static Retval file_cmd_atime(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;

    if (file_stat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp_, sb.st_atime);
    return JIM_OK;
}

/**
 * Set file atime/mtime to the given time in microseconds since the epoch.
 */
static Retval JimSetFileTimes(Jim_InterpPtr interp_, const char* filename, jim_wide us) {
    if (prj_funcDef(prj_utimes)) { // #Unsupported #NonPortFuncFix
        struct prj_timeval times[2];

        times[1].tv_sec = (long) (times[0].tv_sec = (long) (us / 1000000));
        times[1].tv_usec = times[0].tv_usec = us % 1000000;

        if (prj_utimes(filename, times) != 0) {
            Jim_SetResultFormatted(interp_, "can't set time on \"%s\": %s", filename, strerror(errno));
            return JIM_ERR;
        }
        return JIM_OK;
    } else {
        Jim_SetResultString(interp_, "Not implemented", -1);
    }
    return JIM_ERR;
}

static Retval file_cmd_mtime(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;

    if (argc == 2) {
        jim_wide secs;
        if (Jim_GetWide(interp_, argv[1], &secs) != JIM_OK) {
            return JIM_ERR;
        }
        return JimSetFileTimes(interp_, Jim_String(argv[0]), secs * 1000000);
    }
    if (file_stat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp_, sb.st_mtime);
    return JIM_OK;
}

#ifdef STAT_MTIME_US // #optionalCode #WinOff
static Retval file_cmd_mtimeus(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    struct stat sb;

    if (argc == 2) {
        jim_wide us;
        if (Jim_GetWide(interp_, argv[1], &us) != JIM_OK) {
            return JIM_ERR;
        }
        return JimSetFileTimes(interp_, Jim_String(argv[0]), us);
    }
    if (file_stat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp_, STAT_MTIME_US(sb));
    return JIM_OK;
}
#endif

static Retval file_cmd_copy(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_EvalPrefix(interp_, "file copy", argc, argv);
}

static Retval file_cmd_size(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;

    if (file_stat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp_, sb.st_size);
    return JIM_OK;
}

static Retval file_cmd_isdirectory(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;
    int ret = 0;

    if (file_stat(interp_, argv[0], &sb) == JIM_OK) {
        ret = S_ISDIR(sb.st_mode);
    }
    Jim_SetResultInt(interp_, ret);
    return JIM_OK;
}

static Retval file_cmd_isfile(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;
    int ret = 0;

    if (file_stat(interp_, argv[0], &sb) == JIM_OK) {
        ret = S_ISREG(sb.st_mode);
    }
    Jim_SetResultInt(interp_, ret);
    return JIM_OK;
}

static Retval file_cmd_owned(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;
    int ret = 0;

    if (file_stat(interp_, argv[0], &sb) == JIM_OK) {
        ret = (prj_geteuid() == sb.st_uid); // #NonPortFuncFix
    }
    Jim_SetResultInt(interp_, ret);
    return JIM_OK;
}

static Retval file_cmd_readlink(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* path = Jim_String(argv[0]);
    char* linkValue = new_CharArray(MAXPATHLEN + 1); // #AllocF 

    int linkLength = (int) prj_readlink(path, linkValue, MAXPATHLEN); // #NonPortFuncFix

    if (linkLength == -1) {
        free_CharArray(linkValue); // #FreeF 
        Jim_SetResultFormatted(interp_, "couldn't readlink \"%#s\": %s", argv[0], strerror(errno));
        return JIM_ERR;
    }
    linkValue[linkLength] = 0;
    Jim_SetResult(interp_, Jim_NewStringObjNoAlloc(interp_, linkValue, linkLength));
    return JIM_OK;
}

static Retval file_cmd_type(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;

    if (file_lstat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResultString(interp_, JimGetFileType((int) sb.st_mode), -1);
    return JIM_OK;
}

#ifdef HAVE_LSTAT // #optionalCode #WinOff
static Retval file_cmd_lstat(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    struct stat sb;

    if (file_lstat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    return StoreStatData(interp_, argc == 2 ? argv[1] : NULL, &sb);
}
#else
#define file_cmd_lstat file_cmd_stat
#endif

static Retval file_cmd_stat(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    struct stat sb;

    if (file_stat(interp_, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    return StoreStatData(interp_, argc == 2 ? argv[1] : NULL, &sb);
}

static const jim_subcmd_type g_file_command_table[] = { // #JimSubCmdDef
    {   "atime",
        "name",
        file_cmd_atime,
        1,
        1,
        /* Description: Last access time */
    },
    {   "mtime",
        "name ?time?",
        file_cmd_mtime,
        1,
        2,
        /* Description: Get or set last modification time */
    },
#ifdef STAT_MTIME_US // #optionalCode #WinOff #removeCmds
    {   "mtimeus",
        "name ?time?",
        file_cmd_mtimeus,
        1,
        2,
        /* Description: Get or set last modification time in microseconds */
    },
#endif
    {   "copy",
        "?-force? source dest",
        file_cmd_copy,
        2,
        3,
        /* Description: Copy source file to destination file */
    },
    {   "dirname",
        "name",
        file_cmd_dirname,
        1,
        1,
        /* Description: Directory part of the name_ */
    },
    {   "rootname",
        "name",
        file_cmd_rootname,
        1,
        1,
        /* Description: Name without any extension */
    },
    {   "extension",
        "name",
        file_cmd_extension,
        1,
        1,
        /* Description: Last extension including the dot */
    },
    {   "tail",
        "name",
        file_cmd_tail,
        1,
        1,
        /* Description: Last component of the name_ */
    },
    {   "normalize",
        "name",
        file_cmd_normalize,
        1,
        1,
        /* Description: Normalized path of name_ */
    },
    {   "join",
        "name ?name ...?",
        file_cmd_join,
        1,
        -1,
        /* Description: Join multiple path components */
    },
    {   "readable",
        "name",
        file_cmd_readable,
        1,
        1,
        /* Description: Is file readable */
    },
    {   "writable",
        "name",
        file_cmd_writable,
        1,
        1,
        /* Description: Is file writable */
    },
    {   "executable",
        "name",
        file_cmd_executable,
        1,
        1,
        /* Description: Is file executable */
    },
    {   "exists",
        "name",
        file_cmd_exists,
        1,
        1,
        /* Description: Does file exist */
    },
    {   "delete",
        "?-force|--? name ...",
        file_cmd_delete,
        1,
        -1,
        /* Description: Deletes the files or directories (must be empty unless -force) */
    },
    {   "mkdir",
        "dir ...",
        file_cmd_mkdir,
        1,
        -1,
        /* Description: Creates the directories */
    },
    {   "tempfile",
        "?template?",
        file_cmd_tempfile,
        0,
        1,
        /* Description: Creates a temporary filename */
    },
    {   "rename",
        "?-force? source dest",
        file_cmd_rename,
        2,
        3,
        /* Description: Renames a file */
    },
#if defined(HAVE_LINK) && defined(HAVE_SYMLINK) // #optionalCode #WinOff #removeCmds
    {   "link",
        "?-symbolic|-hard? newname target",
        file_cmd_link,
        2,
        3,
        /* Description: Creates a hard or soft link */
    },
#endif
#if defined(HAVE_READLINK) // #optionalCode #WinOff #removeCmds
    {   "readlink",
        "name",
        file_cmd_readlink,
        1,
        1,
        /* Description: Value of the symbolic link */
    },
#endif
    {   "size",
        "name",
        file_cmd_size,
        1,
        1,
        /* Description: size of file */
    },
    {   "stat",
        "name ?var?",
        file_cmd_stat,
        1,
        2,
        /* Description: Returns results of stat, and may store in var array */
    },
    {   "lstat",
        "name ?var?",
        file_cmd_lstat,
        1,
        2,
        /* Description: Returns results of lstat, and may store in var array */
    },
    {   "type",
        "name",
        file_cmd_type,
        1,
        1,
        /* Description: Returns tokenType_ of the file */
    },
#ifdef HAVE_GETEUID // #optionalCode #WinOff #removeCmds
    {   "owned",
        "name",
        file_cmd_owned,
        1,
        1,
        /* Description: Returns 1 if owned by the current owner */
    },
#endif
    {   "isdirectory",
        "name",
        file_cmd_isdirectory,
        1,
        1,
        /* Description: Returns 1 if name_ is a directory */
    },
    {   "isfile",
        "name",
        file_cmd_isfile,
        1,
        1,
        /* Description: Returns 1 if name_ is a file */
    },
    {
        NULL
    }
};

static Retval Jim_CdCmd(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char* path;

    if (argc != 2) {
        Jim_WrongNumArgs(interp_, 1, argv, "dirname");
        return JIM_ERR;
    }

    path = Jim_String(argv[1]);

    if (prj_chdir(path) != 0) { // #NonPortFuncFix
        Jim_SetResultFormatted(interp_, "couldn't change working directory to \"%s\": %s", path,
                               strerror(errno));
        return JIM_ERR;
    }
    return JIM_OK;
}

static Retval Jim_PwdCmd(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    char* cwd = new_CharArray(MAXPATHLEN); // #AllocF  #Review why not (MAXPATHLEN+1)

    if (prj_getcwd(cwd, MAXPATHLEN) == NULL) { // #NonPortFuncFix
        Jim_SetResultString(interp_, "Failed to get pwd", -1);
        free_CharArray(cwd); // #FreeF 
        return JIM_ERR;
    } else if (ISWINDOWS) {
        /* Try to keep backslashes out of paths */
        char* p = cwd;
        while ((p = strchr(p, '\\')) != NULL) {
            *p++ = '/';
        }
    }

    Jim_SetResultString(interp_, cwd, -1);

    free_CharArray(cwd); // #FreeF 
    return JIM_OK;
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-file-version.h>

Retval Jim_fileInit(Jim_InterpPtr interp_) // #JimCmdInit
{
    if (Jim_PackageProvide(interp_, "file-adv", version, JIM_ERRMSG))
        return JIM_ERR;

    Jim_CreateCommand(interp_, "file", Jim_SubCmdProc, (void*) g_file_command_table, NULL);
    Jim_CreateCommand(interp_, "pwd", Jim_PwdCmd, NULL, NULL);
    Jim_CreateCommand(interp_, "cd", Jim_CdCmd, NULL, NULL);
    return JIM_OK;
}

END_JIM_NAMESPACE
#endif // #if 0
