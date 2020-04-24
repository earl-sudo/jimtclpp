#pragma once

/*
 * Cross-platform compatibility functions and types for I/O.
 * Currently used by jim-aio.c and jim-exec.c
 */

#include <stdio.h>
#include <errno.h>

#include "jimautoconf.h"
#include <jim-api.h>
#include <jim-win32compat.h>

BEGIN_JIM_NAMESPACE 

/**
 * Set an error result based on errno and the given message.
 */
void Jim_SetResultErrno(Jim_Interp *interp, const char *msg);

/**
 * Opens the file for writing (and appending if append is true).
 * Returns the file descriptor, or -1 on failure.
 */
int Jim_OpenForWrite(const char *filename, int append);

/**
 * Opens the file for reading.
 * Returns the file descriptor, or -1 on failure.
 */
int Jim_OpenForRead(const char *filename);

#if defined(__MINGW32__) || defined(_MSC_VER) // #optionalCode
    #ifndef STRICT 
    #define STRICT
    #endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
    #include <windows.h>
    #include <fcntl.h>
    #include <io.h>
    #include <process.h>

    typedef HANDLE pidtype;
    #define JIM_BAD_PID INVALID_HANDLE_VALUE
    /* Note that this isn't a separate value on Windows since we don't have os.fork */
    #define JIM_NO_PID INVALID_HANDLE_VALUE

    /* These seem to accord with the conventions used by msys/mingw32 */
    #define WIFEXITED(STATUS) (((STATUS) & 0xff00) == 0)
    #define WEXITSTATUS(STATUS) ((STATUS) & 0x00ff)
    #define WIFSIGNALED(STATUS) (((STATUS) & 0xff00) != 0)
    #define WTERMSIG(STATUS) (((STATUS) >> 8) & 0xff)
    #define WNOHANG 1

    /**
     * Unix-compatible errno
     */
    int Jim_Errno(void);

#ifndef HAVE_PIPE 
    #define HAVE_PIPE
#endif
    #define pipe(P) _pipe((P), 0, O_NOINHERIT)

#elif defined(HAVE_UNISTD_H) // #optionalCode #WinOff
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/wait.h>
    #include <sys/stat.h>

    typedef int pidtype;
    #define Jim_Errno() errno
    #define JIM_BAD_PID -1
    #define JIM_NO_PID 0

    #ifndef HAVE_EXECVPE // #optionalCode #WinOff
        #define execvpe(ARG0, ARGV, ENV) execvp(ARG0, ARGV)
    #endif
#endif

END_JIM_NAMESPACE
