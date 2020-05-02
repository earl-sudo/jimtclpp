// g++ -x c++ -c prj_compat.c -I. -g3 -DPRJ_COMPAT_MAIN -ldl
// gcc prj_compat.c -I. -g3 -DPRJ_COMPAT_MAIN -ldl
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#ifdef _WIN32 // #optionalCode
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h> // #NonPortHeader
#include <errno.h>
#endif

#include "jim-config.h"
#include "jim.h"
#include <jimautoconf.h>

#ifdef _WIN32 // #optionalCode #WinOff
#undef HAVE_DLOPEN

#else
#define HAVE_PID_T_TYPE 1
#define HAVE_UID_T_TYPE 1
#define HAVE_OFF_T_TYPE 1
#define HAVE_USECONDS_T_TYPE 1
#define HAVE_SSIZE_T_TYPE 1
#define HAVE_MODE_T_TYPE 1
#define HAVE_STRUCT_TIMEVAL_TYPE 1
#define HAVE_STRUCT_TM_TYPE 1
#endif

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> // #NonPortHeader

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <setjmp.h>
#include <math.h>


#ifdef HAVE_UNISTD_H // #optionalCode #WinOff
#include <unistd.h> // #NonPortHeader
#include <sys/stat.h> // #NonPortHeader
#endif

#if defined(HAVE_SYS_SOCKET_H) && defined(HAVE_SELECT) && defined(HAVE_NETINET_IN_H) && defined(HAVE_NETDB_H) && defined(HAVE_ARPA_INET_H) // #optionalCode #WinOff
#include <sys/socket.h> // #NonPortHeader
#include <netinet/in.h> // #NonPortHeader
#include <netinet/tcp.h> // #NonPortHeader
#include <arpa/inet.h> // #NonPortHeader
#include <netdb.h> // #NonPortHeader
#ifdef HAVE_SYS_UN_H // #optionalCode #WinOff
#include <sys/un.h> // #NonPortHeader
#endif
#else
#undef HAVE_SHUTDOWN
#endif

#ifdef HAVE_SYS_TIME_H // #optionalCode
#include <sys/time.h>
#endif

#ifdef HAVE_BACKTRACE // #optionalCode
#include <execinfo.h> // #NonPortHeader
#endif

#ifdef HAVE_CRT_EXTERNS_H // #optionalCode
#include <crt_externs.h> // #NonPortHeader
#endif

#if defined(HAVE_SYS_SYSINFO_H) && !defined(_WIN32)
#include <sys/sysinfo.h> // #NonPortHeader
#endif

#if defined(__MINGW32__) // #optionalCode #WinOff
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // #NonPortHeader
#include <winsock.h> // #NonPortHeader
#ifndef HAVE_USLEEP // #optionalCode
#define HAVE_USLEEP
#endif
#else
#include <sys/types.h> // #NonPortHeader
#ifdef HAVE_SYS_SELECT_H // #optionalCode #NonPortHeader
#include <sys/select.h> // #NonPortHeader
#endif
#endif

#ifdef HAVE_DIRENT_H // #optionalCode
#include <dirent.h> // #NonPortHeader
#endif

#ifdef HAVE_DLOPEN // #optionalCode
#include <dlfcn.h> // #NonPortHeader
#endif

#if defined(HAVE_WAITPID) && !defined(_WIN32)
#include <sys/types.h> // #NonPortHeader
#include <sys/wait.h> // #NonPortHeader
#endif

#include <prj_compat.h>


#ifdef __cplusplus
#include <type_traits>

#ifdef HAVE_PID_T_TYPE
static_assert(sizeof(pid_t) <= sizeof(prj_pid_t), "ERROR: pid_t size");
static_assert(std::is_signed<pid_t>::value == std::is_signed<prj_pid_t>::value, "ERROR: pid_t sign");
#endif


#ifdef HAVE_UID_T_TYPE
static_assert(sizeof(uid_t) == sizeof(prj_uid_t), "ERROR: prj_uid_t size");
static_assert(std::is_signed<uid_t>::value == std::is_signed<prj_uid_t>::value, "ERROR: prj_uid_t sign");
#endif

#ifdef HAVE_OFF_T_TYPE
static_assert(sizeof(off_t) == sizeof(prj_off_t), "ERROR: prj_off_t size");
static_assert(std::is_signed<off_t>::value == std::is_signed<prj_off_t>::value, "ERROR: prj_uid_t sign");
#endif

#ifdef HAVE_USECONDS_T_TYPE
static_assert(sizeof(useconds_t) == sizeof(prj_useconds_t), "ERROR: prj_useconds_t size");
static_assert(std::is_signed<useconds_t>::value == std::is_signed<prj_useconds_t>::value, "ERROR: prj_uid_t sign");
#endif

#ifdef HAVE_MODE_T_TYPE
static_assert(sizeof(mode_t) == sizeof(prj_mode_t), "ERROR: prj_mode_t size");
static_assert(std::is_signed<mode_t>::value == std::is_signed<prj_mode_t>::value, "ERROR: prj_mode_t sign");
#endif

#ifdef HAVE_STRUCT_TM_TYPE
//static_assert(sizeof(struct tm) =< sizeof(struct prj_tm), "ERROR: prj_tm size");
static_assert(offsetof(struct tm,tm_sec) == offsetof(struct prj_tm,tm_sec), "ERROR: prj_tm tm_sec offset");
static_assert(offsetof(struct tm,tm_min) == offsetof(struct prj_tm,tm_min), "ERROR: prj_tm tm_min offset");
static_assert(offsetof(struct tm, tm_hour) == offsetof(struct prj_tm, tm_hour), "ERROR: prj_tm tm_hour offset");
static_assert(offsetof(struct tm, tm_mday) == offsetof(struct prj_tm, tm_mday), "ERROR: prj_tm tm_mday offset");
static_assert(offsetof(struct tm, tm_mon) == offsetof(struct prj_tm, tm_mon), "ERROR: prj_tm tm_mon offset");
static_assert(offsetof(struct tm, tm_year) == offsetof(struct prj_tm, tm_year), "ERROR: prj_tm tm_year offset");
static_assert(offsetof(struct tm, tm_wday) == offsetof(struct prj_tm, tm_wday), "ERROR: prj_tm tm_wday offset");
static_assert(offsetof(struct tm, tm_yday) == offsetof(struct prj_tm, tm_yday), "ERROR: prj_tm tm_yday offset");
static_assert(offsetof(struct tm, tm_isdst) == offsetof(struct prj_tm, tm_isdst), "ERROR: prj_tm tm_isdst offset");
#endif

#ifndef HAVE_SSIZE_T_TYPE
//static_assert(sizeof(ssize_t)) == sizeof(prj_ssize_t), "ERROR: prj_ssize_t");
static_assert(std::is_signed<prj_ssize_t>::value, "ERROR: prj_ssize_t sign");
#endif

#ifdef HAVE_STRUCT_TM_TYPE
static_assert(sizeof(struct timeval) == sizeof(struct prj_timeval), "ERROR: timeval size");
#endif

#ifdef HAVE_GETTIMEOFDAY
static_assert(sizeof(time_t) == sizeof(prj_time_t), "ERROR: time_t size");
static_assert(offsetof(struct timespec, tv_sec) == offsetof(struct prj_timespec, tv_sec), "ERROR: prj_timespec tv_sec offset");
static_assert(offsetof(struct timespec, tv_nsec) == offsetof(struct prj_timespec, tv_nsec), "ERROR: prj_timespec tv_nsec offset");
#endif

#else
#endif

#ifdef HAVE_FORK
// pid_t fork(void);
prj_forkFp prj_fork = (prj_forkFp)fork;
#else
prj_forkFp prj_fork = NULL;
#endif

#ifdef HAVE_FSYNC
// int fsync(int fd);
prj_fsyncFp prj_fsync = fsync;
#else
prj_fsyncFp prj_fsync = NULL;
#endif

#ifdef HAVE_FSEEKO
//        int fseeko(FILE *stream, off_t offset, int whence);
prj_fseekoFp prj_fseeko = (prj_fseekoFp)fseeko;
#else
prj_fseekoFp prj_fseeko = NULL;
#endif

// uid_t getuid(void);

#ifdef HAVE_GETEUID
// uid_t geteuid(void);
prj_geteuidFp prj_geteuid = geteuid;
#else
prj_geteuidFp prj_geteuid = NULL;
#endif

#ifdef HAVE_LINK
//int link(const char *oldpath, const char *newpath);
prj_linkFp prj_link = (prj_linkFp)link;
#else
prj_linkFp prj_link = NULL;
#endif

#ifdef HAVE_SYMLINK
// int symlink(const char *target, const char *linkpath);
prj_symlinkFp prj_symlink = (prj_linkFp)symlink;
#else
prj_symlinkFp prj_symlink = NULL;
#endif


#ifdef HAVE_FTELLO
//off_t ftello(FILE *stream);
prj_ftelloFp prj_ftello = (prj_ftelloFp)ftello;
#else
prj_ftelloFp prj_ftello = NULL;
#endif

#ifdef HAVE_BACKTRACE
// char **backtrace_symbols(void *const *buffer, int size);
// void backtrace_symbols_fd(void *const *buffer, int size, int fd);
// char **backtrace_symbols(void *const *buffer, int size);
prj_backtrace_symbolsFp prj_backtrace_symbols = (prj_backtrace_symbolsFp)backtrace_symbols;
prj_backtrace_symbols_fdFp prj_backtrace_symbols_fd = (prj_backtrace_symbols_fdFp)backtrace_symbols_fd;
prj_backtraceFp prj_backtrace = (prj_backtraceFp)backtrace;
#else
prj_backtrace_symbolsFp prj_backtrace_symbols = NULL;
prj_backtrace_symbols_fdFp prj_backtrace_symbols_fd = NULL;
prj_backtraceFp prj_backtrace = NULL;
#endif

#ifdef HAVE_REALPATH
prj_realpathFp prj_realpath = (prj_realpathFp)realpath;
#else
prj_realpathFp prj_realpath = NULL;
#endif

#ifdef HAVE_UMASK
prj_umaskFp prj_umask = (prj_umaskFp)umask;
#else
prj_umaskFp prj_umask = NULL;
#endif

#ifdef HAVE_MKSTEMP
prj_mkstempFp prj_mkstemp = (prj_mkstempFp)mkstemp;
#else
prj_mkstempFp prj_mkstemp = NULL;
#endif

#ifdef HAVE_UALARM
prj_ualarmFp prj_ualarm = (prj_ualarmFp)ualarm;
#else
prj_ualarmFp prj_ualarm = NULL;
#endif

#ifdef HAVE_ISATTY
prj_isattyFp prj_isatty = (prj_isattyFp)isatty;
#else
prj_isattyFp prj_isatty = NULL;
#endif

#ifdef HAVE_READLINK
prj_readlinkFp prj_readlink = (prj_readlinkFp)readlink;
#else
prj_readlinkFp prj_readlink = NULL;
#endif

//#ifdef HAVE_SHUTDOWN
//prj_shutdownFp prj_shutdown = (prj_shutdownFp)shutdown;
//#else
//prj_shutdownFp prj_shutdown = NULL;
//#endif

#ifdef HAVE_USLEEP
prj_usleepFp prj_usleep = (prj_usleepFp)usleep;
#else
#ifdef _WIN32
#include <Windows.h> // #NonPortFunc #WinSpecific
static int usleep(prj_useconds_t usec) { Sleep(usec / 1000); return 0; }
prj_usleepFp prj_usleep = usleep;
#else
prj_usleepFp prj_usleep = NULL;
#endif
#endif

#ifdef HAVE_UTIMES
prj_utimesFp prj_utimes = (prj_utimesFp)utimes;
#else
prj_utimesFp prj_utimes = NULL;
#endif

#ifdef HAVE_VFORK
prj_vforkFp prj_vfork = (prj_vforkFp)vfork;
#else
prj_vforkFp prj_vfork = NULL;
#endif

#ifdef HAVE_SYS_IOCTL_H 
#include <sys/ioctl.h> // #NonPortHeader
#endif
#include <sys/types.h> // #NonPortHeader
#include <signal.h> // #NonPortHeader
#ifdef HAVE_SYSLOG_H
#include <syslog.h> // #NonPortHeader
#endif

#ifdef HAVE_FCNTL
prj_fcntlFp prj_fcntl = (prj_fcntlFp) fcntl;
#else
prj_fcntlFp prj_fcntl = (prj_fcntlFp) NULL;
#endif

#ifdef HAVE_IOCTL
prj_ioctlFp prj_ioctl = (prj_ioctlFp) ioctl;
#else
prj_ioctlFp prj_ioctl = (prj_ioctlFp) NULL;
#endif

#ifdef HAVE_KILL
prj_killFp prj_kill = (prj_killFp) kill;
#else
prj_killFp prj_kill = (prj_killFp) NULL;
#endif

#ifdef HAVE_SLEEP
prj_sleepFp prj_sleep = (prj_sleepFp) sleep;
#else
prj_sleepFp prj_sleep = (prj_sleepFp) NULL;
#endif

#ifdef HAVE_STRPTIME
prj_strptimeFp prj_strptime = (prj_strptimeFp) strptime;
#else
prj_strptimeFp prj_strptime = (prj_strptimeFp) NULL;
#endif

#ifdef HAVE_CLOCK_GETTIME
prj_clock_gettimeFp prj_clock_gettime = (prj_clock_gettimeFp)clock_gettime;
#else
prj_clock_gettimeFp prj_clock_gettime = (prj_clock_gettimeFp)NULL;
#endif

#ifdef _WIN32
prj_dupFp prj_dup = (prj_dupFp) _dup;
prj_dup2Fp prj_dup2 = (prj_dup2Fp) _dup2;
prj_execvpFp prj_execvp = (prj_execvpFp) _execvp;
prj_execvpeFp prj_execvpe = (prj_execvpeFp) _execvpe;
prj_fdopenFp prj_fdopen = (prj_fdopenFp) _fdopen;
prj_getpidFp prj_getpid = (prj_getpidFp) _getpid;
#else
prj_dupFp prj_dup = (prj_dupFp)dup;
prj_dup2Fp prj_dup2 = (prj_dup2Fp)dup2;
prj_execvpFp prj_execvp = (prj_execvpFp)execvp;
prj_execvpeFp prj_execvpe = (prj_execvpeFp)execvpe;
prj_fdopenFp prj_fdopen = (prj_fdopenFp) fdopen;
prj_getpidFp prj_getpid = (prj_getpidFp) getpid;
#endif

prj_getenvFp prj_getenv = (prj_getenvFp) getenv;

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // #NonPortHeader
#undef GetEnvironmentStrings

static char** prj_environImp(void) {
    static char* environmentVariables[256];
    memset(environmentVariables, 0, sizeof(environmentVariables));

    int i = 0;
    char* lpvEnv = ::GetEnvironmentStrings();
    for (char* lpszVariable = (char*) lpvEnv; *lpszVariable; lpszVariable++) {
        environmentVariables[i] = (char*) lpszVariable; i++;
        while (*lpszVariable) {
            //putchar(*lpszVariable++);
            lpszVariable++;
        }
        //putchar('\n'); 
    }
    return (char**) environmentVariables;
}
prj_environFp prj_environ = (prj_environFp) prj_environImp;
#else
extern char** environ;
static char** prj_environImp(void) {
    return (char**) ::environ;;
}
prj_environFp prj_environ = (prj_environFp) prj_environImp;
#endif


#ifdef HAVE_SYSLOG_H
prj_openlogFp prj_openlog = (prj_openlogFp)openlog;
prj_syslogFp prj_syslog = (prj_syslogFp)syslog;
prj_closelogFp prj_closelog = (prj_closelogFp)closelog;
#else
prj_openlogFp prj_openlog = (prj_openlogFp) NULL;
prj_syslogFp prj_syslog = (prj_syslogFp) NULL;
prj_closelogFp prj_closelog = (prj_closelogFp) NULL;
#endif

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // #NonPortHeader

static_assert(sizeof(HANDLE) <= sizeof(prj_pid_t), "ERROR: pid_t size");


static void* dlopen(const char* path, int mode) { // #WinSimLinux
    JIM_NOTUSED(mode);

    return (void*) LoadLibraryA(path);
}

static int dlclose(void* handle) { // #WinSimLinux
    FreeLibrary((HMODULE) handle);
    return 0;
}

static void* dlsym(void* handle, const char* symbol) { // #WinSimLinux
    return GetProcAddress((HMODULE) handle, symbol);
}

static char* dlerror(void) { // #WinSimLinux
    static char msg[121];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                   LANG_NEUTRAL, msg, sizeof(msg) - 1, NULL);
    return msg;
}

prj_dlopenFp prj_dlopen = (prj_dlopenFp) dlopen;
prj_dlcloseFp prj_dlclose = (prj_dlcloseFp) dlclose;
prj_dlsymFp prj_dlsym = (prj_dlsymFp) dlsym;
prj_dlerrorFp prj_dlerror = (prj_dlerrorFp) dlerror;
#else
prj_dlopenFp prj_dlopen = (prj_dlopenFp) NULL;
prj_dlcloseFp prj_dlclose = (prj_dlcloseFp) NULL;
prj_dlsymFp prj_dlsym = (prj_dlsymFp) NULL;
prj_dlerrorFp prj_dlerror = (prj_dlerrorFp) NULL;
#endif

#if defined(HAVE_DLOPEN_COMPAT) && defined(_WIN32)
struct dirent {
    char* d_name;
};

typedef struct DIR {
    long                handle; /* -1 for failed rewind */
    struct _finddata_t  info;
    struct dirent       result; /* d_name null iff first time */
    char* name;  /* null-terminated char string */
} DIR;

static DIR* opendir(const char* name) { // #WinSimLinux
    DIR* dir = 0;

    if (name && name[0]) {
        size_t base_length = strlen(name);
        const char* all =
            strchr("/\\", name[base_length - 1]) ? "*" : "/*";

        if ((dir = (DIR*) malloc(sizeof * dir)) != 0 && // #Alloc
            (dir->name = (char*) malloc(base_length + strlen(all) + 1)) != 0) { // #Alloc
            strcat(strcpy(dir->name, name), all);

            if ((dir->handle = (long) _findfirst(dir->name, &dir->info)) != -1)
                dir->result.d_name = 0;
            else {
                free(dir->name); // #Free
                free(dir); // #Free
                dir = 0;
            }
        } else {
            free(dir); // #Free
            dir = 0;
            errno = ENOMEM;
        }
} else {
        errno = EINVAL;
    }
    return dir;
}

static struct dirent* readdir(DIR* dir) { // #WinSimLinux
    struct dirent* result = 0;

    if (dir && dir->handle != -1) {
        if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
            result = &dir->result;
            result->d_name = dir->info.name;
        }
    } else {
        errno = EBADF;
    }
    return result;
}

static int closedir(DIR* dir) { // #WinSimLinux
    int result = -1;

    if (dir) {
        if (dir->handle != -1)
            result = _findclose(dir->handle);
        free(dir->name);
        free(dir);
}
    if (result == -1)
        errno = EBADF;
    return result;
}

static const char* prj_dirent_dnameFunc(dirent* de) {
    return de->d_name;
}

prj_opendirFp prj_opendir = (prj_opendirFp) opendir;
prj_closedirFp prj_closedir = (prj_closedirFp) closedir;
prj_readdirFp prj_readdir = (prj_readdirFp) readdir;
prj_dirent_dnameFp prj_dirent_dname = (prj_dirent_dnameFp) prj_dirent_dnameFunc;
#else
#include <dirent.h> // #NonPortHeader

static const char* prj_dirent_dnameFunc(dirent* de) {
    return de->d_name;
}
prj_opendirFp prj_opendir = (prj_opendirFp) opendir;
prj_closedirFp prj_closedir = (prj_closedirFp) closedir;
prj_readdirFp prj_readdir = (prj_readdirFp) readdir;
prj_dirent_dnameFp prj_dirent_dname = (prj_dirent_dnameFp) prj_dirent_dnameFunc;
#endif

#if 0
#ifdef HAVE_DLOPEN

static const char* prj_dirent_dnameFunc(dirent* de) {
    return de->d_name;
}

prj_opendirFp prj_opendir = (prj_opendirFp)NULL;
prj_closedirFp prj_closedir = (prj_closedirFp) NULL;
prj_dirent_dnameFp prj_dirent_dname = (prj_dirent_dnameFp) prj_dirent_dnameFunc;
#endif
#endif

#if defined(__MINGW32__) || defined(_MSC_VER) // #optionalCode

static prj_pid_t waitpid(prj_pid_t pid, int* status, int nohang) {
    DWORD ret = WaitForSingleObject((HANDLE)pid, nohang ? 0 : INFINITE);
    if (ret == WAIT_TIMEOUT || ret == WAIT_FAILED) {
        /* WAIT_TIMEOUT can only happened with WNOHANG */
        return -1;
    }
    GetExitCodeProcess((HANDLE)pid, &ret);
    *status = ret;
    CloseHandle((HANDLE)pid);
    return pid;
}
prj_waitpidFp prj_waitpid = (prj_waitpidFp) waitpid;
#else
prj_waitpidFp prj_waitpid = (prj_waitpidFp) waitpid;
#endif


prj_raiseFp prj_raise = (prj_raiseFp)raise;
prj_signalFp prj_signal = (prj_signalFp)signal;
prj_mktimeFp prj_mktime = (prj_mktimeFp)mktime;
prj_localtimeFp prj_localtime = (prj_localtimeFp)localtime;
prj_localtime_rFp prj_localtime_r = (prj_localtime_rFp)localtime;

#ifndef _WIN32
prj_closeFp prj_close = close;
prj_pipeFp prj_pipe = (prj_pipeFp)pipe;
prj_gmtimeFp prj_gmtime = (prj_gmtimeFp) gmtime;
#else
prj_closeFp prj_close = _close;
prj_pipeFp prj_pipe = (prj_pipeFp) _pipe;
prj_gmtimeFp prj_gmtime = (prj_gmtimeFp) gmtime;
#endif

#if defined(HAVE_STRUCT_SYSINFO_UPTIME) && !defined(_WIN32)
prj_sysinfoFp prj_sysinfo = (prj_sysinfoFp)sysinfo;
long prj_sysinfo_uptime(struct prj_sysinfo* info) {
    return ((struct sysinfo*)info)->uptime;
}
#else
prj_sysinfoFp prj_sysinfo = (prj_sysinfoFp) NULL;
long prj_sysinfo_uptime(struct prj_sysinfo* info) {
    return 0;
}
#endif
#ifdef _WIN32
#ifndef STRICT
#define STRICT
#endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // #NonPortHeader

#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h> // #NonPortHeader
#include <errno.h>
#endif

#ifdef _WIN32

#if 0
struct prj_timezone {
    int tz_minuteswest;     /* minutes west of Greenwich */
    int tz_dsttime;         /* type of DST correction */
};
static_assert(offsetof(struct timezone, tz_minuteswest) == offsetof(struct prj_timezone, tz_minuteswest), "ERROR: prj_timezone tz_minuteswest offset");
static_assert(offsetof(struct timezone, tz_dsttime) == offsetof(struct prj_timezone, tz_dsttime), "ERROR: prj_timezone tz_dsttime offset");

#endif



#ifdef TRY1
/* FILETIME of Jan 1 1970 00:00:00. */
static const unsigned __int64 epoch = ((unsigned __int64) 116444736000000000ULL);

/*
 * timezone information is stored outside the kernel so tzp isn't used anymore.
 *
 * Note: this function is not for Win32 high precision timing purpose. See
 * elapsed_time().
 */
extern "C" int
gettimeofday(struct prj_timeval * tp, struct prj_timezone * tzp) {
    FILETIME    file_time;
    SYSTEMTIME  system_time;
    ULARGE_INTEGER ularge;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    ularge.LowPart = file_time.dwLowDateTime;
    ularge.HighPart = file_time.dwHighDateTime;

    tp->tv_sec = (long) ((ularge.QuadPart - epoch) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);

    return 0;
}
#endif

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#include <time.h>

struct timezone {
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

extern "C"
int gettimeofday(struct prj_timeval *tv, struct prj_timezone *tz) { // #WinSimLinux
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (NULL != tv) {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        tmpres /= 10;  /*convert into microseconds*/
        /*converting file time to unix epoch*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;
        tv->tv_sec = (long) (tmpres / 1000000UL);
        tv->tv_usec = (long) (tmpres % 1000000UL);
    }

    if (NULL != tz) {
        if (!tzflag) {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}

#define HAVE_GETTIMEOFDAY 1
#endif

#ifdef HAVE_GETTIMEOFDAY
prj_gettimeofdayFp prj_gettimeofday = (prj_gettimeofdayFp) gettimeofday;
#else
prj_gettimeofdayFp prj_gettimeofday = (prj_gettimeofdayFp) NULL;
#endif

#ifdef PRJ_COMPAT_MAIN
int main(int argc, char* argv[]) {
	struct timeval  x;
    printf("%ld\n", sizeof(struct timeval));
    printf("%ld\n", sizeof(time_t));
    printf("0x%p\n", prj_fork);
}
#endif

PRJ_COMPILERS g_prj_compiler = PRJ_COMPILER; // Allow for runtime check of compiler
PRJ_OS g_prj_os = PROJ_OS; // Allow for runtime check of os
