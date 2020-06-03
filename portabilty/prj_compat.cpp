// g++ -x c++ -c prj_compat.c -I. -g3 -DPRJ_COMPAT_MAIN -ldl
// gcc prj_compat.c -I. -g3 -DPRJ_COMPAT_MAIN -ldl
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#define LOCAL_NOTUSED(V) ((void) V)

#ifdef _WIN32 // #optionalCode
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h> // #NonPortHeader
#include <errno.h>
#endif

#include <jim.h>
#include <jim-config.h>
#include <prj_compat.h>

#ifdef PRJ_OS_WIN
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h> // #NonPortHeader
#  include <winsock.h> // #NonPortHeader
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
#  include <unistd.h> // #NonPortHeader
#  include <sys/stat.h> // #NonPortHeader
#endif

#if defined(HAVE_SYS_SOCKET_H) && defined(HAVE_SELECT) && defined(HAVE_NETINET_IN_H) && defined(HAVE_NETDB_H) && defined(HAVE_ARPA_INET_H) // #optionalCode #WinOff
#  include <sys/socket.h> // #NonPortHeader
#  include <netinet/in.h> // #NonPortHeader
#  include <netinet/tcp.h> // #NonPortHeader
#  include <arpa/inet.h> // #NonPortHeader
#  include <netdb.h> // #NonPortHeader
#  ifdef HAVE_SYS_UN_H // #optionalCode #WinOff
#    include <sys/un.h> // #NonPortHeader
#  endif
#endif

#ifdef HAVE_SYS_TIME_H // #optionalCode
#  include <sys/time.h>
#endif

#ifdef HAVE_BACKTRACE // #optionalCode
#  include <execinfo.h> // #NonPortHeader
#endif

#ifdef HAVE_CRT_EXTERNS_H // #optionalCode
#  include <crt_externs.h> // #NonPortHeader
#endif

#if defined(HAVE_SYS_SYSINFO_H) && !defined(_WIN32)
#  include <sys/sysinfo.h> // #NonPortHeader
#endif

#if defined(__MINGW32__) // #optionalCode #WinOff
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#else
#  include <sys/types.h> // #NonPortHeader
#  ifdef HAVE_SYS_SELECT_H // #optionalCode #NonPortHeader
#    include <sys/select.h> // #NonPortHeader
#  endif
#endif

#ifdef HAVE_DIRENT_H // #optionalCode
#  include <dirent.h> // #NonPortHeader
#endif

#ifdef HAVE_DLOPEN // #optionalCode
#  include <dlfcn.h> // #NonPortHeader
#endif

#if defined(HAVE_WAITPID) && !defined(_WIN32)
#  include <sys/types.h> // #NonPortHeader
#  include <sys/wait.h> // #NonPortHeader
#endif

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
static_assert(sizeof(mode_t) <= sizeof(prj_mode_t), "ERROR: prj_mode_t size");
static_assert(std::is_signed<mode_t>::value == std::is_signed<prj_mode_t>::value, "ERROR: prj_mode_t sign");
#endif

#ifdef HAVE_STRUCT_TM_TYPE
//static_assert(sizeof(struct tm) =< sizeof(struct prj_tm), "ERROR: prj_tm size_");
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
prj_forkFp prj_fork = nullptr;
#endif

#ifdef HAVE_FSYNC
// int fsync(int fd_);
prj_fsyncFp prj_fsync = fsync;
#else
prj_fsyncFp prj_fsync = nullptr;
#endif

#ifdef HAVE_FSEEKO
//        int fseeko(FILE *stream, off_t offset, int whence);
prj_fseekoFp prj_fseeko = (prj_fseekoFp)fseeko;
#else
prj_fseekoFp prj_fseeko = nullptr;
#endif

// uid_t getuid(void);

#ifdef HAVE_GETEUID
// uid_t geteuid(void);
prj_geteuidFp prj_geteuid = geteuid;
#else
prj_geteuidFp prj_geteuid = nullptr;
#endif

#ifdef HAVE_LINK
//int link(const char *oldpath, const char *newpath);
prj_linkFp prj_link = (prj_linkFp)link;
#else
prj_linkFp prj_link = nullptr;
#endif

#ifdef HAVE_SYMLINK
// int symlink(const char *target, const char *linkpath);
prj_symlinkFp prj_symlink = (prj_linkFp)symlink;
#else
prj_symlinkFp prj_symlink = nullptr;
#endif


#ifdef HAVE_FTELLO
//off_t ftello(FILE *stream);
prj_ftelloFp prj_ftello = (prj_ftelloFp)ftello;
#else
prj_ftelloFp prj_ftello = nullptr;
#endif

#ifdef HAVE_BACKTRACE
// char **backtrace_symbols(void *const *buffer, int size_);
// void backtrace_symbols_fd(void *const *buffer, int size_, int fd_);
// char **backtrace_symbols(void *const *buffer, int size_);
prj_backtrace_symbolsFp prj_backtrace_symbols = (prj_backtrace_symbolsFp)backtrace_symbols;
prj_backtrace_symbols_fdFp prj_backtrace_symbols_fd = (prj_backtrace_symbols_fdFp)backtrace_symbols_fd;
prj_backtraceFp prj_backtrace = (prj_backtraceFp)backtrace;
#else
prj_backtrace_symbolsFp prj_backtrace_symbols = nullptr;
prj_backtrace_symbols_fdFp prj_backtrace_symbols_fd = nullptr;
prj_backtraceFp prj_backtrace = nullptr;
#endif

#ifdef HAVE_REALPATH
prj_realpathFp prj_realpath = (prj_realpathFp)realpath;
#else
prj_realpathFp prj_realpath = nullptr;
#endif

#ifdef HAVE_UMASK
prj_umaskFp prj_umask = (prj_umaskFp)umask;
#else
prj_umaskFp prj_umask = nullptr;
#endif

#ifdef HAVE_MKSTEMP
prj_mkstempFp prj_mkstemp = (prj_mkstempFp)mkstemp;
#else
prj_mkstempFp prj_mkstemp = nullptr;
#endif

#ifdef HAVE_UALARM
prj_ualarmFp prj_ualarm = (prj_ualarmFp)ualarm;
#else
prj_ualarmFp prj_ualarm = nullptr;
#endif

#ifdef HAVE_ISATTY
prj_isattyFp prj_isatty = (prj_isattyFp)isatty;
#else
prj_isattyFp prj_isatty = nullptr;
#endif

#ifdef HAVE_READLINK
prj_readlinkFp prj_readlink = (prj_readlinkFp)readlink;
#else
prj_readlinkFp prj_readlink = nullptr;
#endif

//#ifdef HAVE_SHUTDOWN
//prj_shutdownFp prj_shutdown = (prj_shutdownFp)shutdown;
//#else
//prj_shutdownFp prj_shutdown = nullptr;
//#endif

#ifdef HAVE_USLEEP
// usleep not defined in MinGW
prj_usleepFp prj_usleep = (prj_usleepFp)usleep;
#else
#ifdef _WIN32
static int usleep(prj_useconds_t usec) { Sleep(usec / 1000); return 0; }
prj_usleepFp prj_usleep = usleep;
#else
prj_usleepFp prj_usleep = nullptr;
#endif
#endif

#ifdef HAVE_UTIMES
prj_utimesFp prj_utimes = (prj_utimesFp)utimes;
#else
prj_utimesFp prj_utimes = nullptr;
#endif

#ifdef HAVE_VFORK
prj_vforkFp prj_vfork = (prj_vforkFp)vfork;
#else
prj_vforkFp prj_vfork = nullptr;
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
prj_fcntlFp prj_fcntl = (prj_fcntlFp) nullptr;
#endif

#ifdef HAVE_IOCTL
prj_ioctlFp prj_ioctl = (prj_ioctlFp) ioctl;
#else
prj_ioctlFp prj_ioctl = (prj_ioctlFp) nullptr;
#endif

#ifdef HAVE_KILL
prj_killFp prj_kill = (prj_killFp) kill;
#else
prj_killFp prj_kill = (prj_killFp) nullptr;
#endif

#ifdef HAVE_SLEEP
prj_sleepFp prj_sleep = (prj_sleepFp) sleep;
#else
prj_sleepFp prj_sleep = (prj_sleepFp) nullptr;
#endif

#ifdef HAVE_STRPTIME
prj_strptimeFp prj_strptime = (prj_strptimeFp) strptime;
#else
prj_strptimeFp prj_strptime = (prj_strptimeFp) nullptr;
#endif

#ifdef HAVE_CLOCK_GETTIME
prj_clock_gettimeFp prj_clock_gettime = (prj_clock_gettimeFp)clock_gettime;
#else
prj_clock_gettimeFp prj_clock_gettime = (prj_clock_gettimeFp)nullptr;
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
prj_fdopenFp prj_fdopen = (prj_fdopenFp) fdopen;
prj_getpidFp prj_getpid = (prj_getpidFp) getpid;
#ifdef PRJ_OS_MACOS
prj_execvpeFp prj_execvpe = (prj_execvpeFp) nullptr;
#else
prj_execvpeFp prj_execvpe = (prj_execvpeFp) execvpe;
#endif
#endif

prj_getenvFp prj_getenv = (prj_getenvFp) getenv;

#ifdef _WIN32
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
prj_openlogFp prj_openlog = (prj_openlogFp) nullptr;
prj_syslogFp prj_syslog = (prj_syslogFp) nullptr;
prj_closelogFp prj_closelog = (prj_closelogFp) nullptr;
#endif

#if defined(_WIN32)
static_assert(sizeof(HANDLE) <= sizeof(prj_pid_t), "ERROR: pid_t size");

static void* dlopen(const char* path, int mode) { // #WinSimLinux
    LOCAL_NOTUSED(mode);

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
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
                   LANG_NEUTRAL, msg, sizeof(msg) - 1, nullptr);
    return msg;
}

prj_dlopenFp prj_dlopen = (prj_dlopenFp) dlopen;
prj_dlcloseFp prj_dlclose = (prj_dlcloseFp) dlclose;
prj_dlsymFp prj_dlsym = (prj_dlsymFp) dlsym;
prj_dlerrorFp prj_dlerror = (prj_dlerrorFp) dlerror;
#else
prj_dlopenFp prj_dlopen = (prj_dlopenFp) nullptr;
prj_dlcloseFp prj_dlclose = (prj_dlcloseFp) nullptr;
prj_dlsymFp prj_dlsym = (prj_dlsymFp) nullptr;
prj_dlerrorFp prj_dlerror = (prj_dlerrorFp) nullptr;
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

prj_opendirFp prj_opendir = (prj_opendirFp)nullptr;
prj_closedirFp prj_closedir = (prj_closedirFp) nullptr;
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
prj_sysinfoFp prj_sysinfo = (prj_sysinfoFp) nullptr;
long prj_sysinfo_uptime(struct prj_sysinfo* info) {
    return 0;
}
#endif
#ifdef _WIN32
#ifndef STRICT
#define STRICT
#endif

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
    int tz_dsttime;         /* tokenType_ of DST correction */
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
 * Note: this function_ is not for Win32 high precision timing purpose. See
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
#  define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#  define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#include <time.h>

#if 0
struct timezone {
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* tokenType_ of dst correction */
};
#endif

extern "C"
int gettimeofday(struct prj_timeval *tv, struct prj_timezone *tz) { // #WinSimLinux
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (nullptr != tv) {
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

    if (nullptr != tz) {
        if (!tzflag) {
            _tzset();
            tzflag++;
        }
#ifdef PRJ_COMPILER_MSVC // #FIXME What to do on other compilers for tz_minuteswest tz_dsttime?
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
#endif
    }

    return 0;
}
#endif

#ifdef HAVE_GETTIMEOFDAY
  prj_gettimeofdayFp prj_gettimeofday = (prj_gettimeofdayFp) gettimeofday;
#else
  prj_gettimeofdayFp prj_gettimeofday = (prj_gettimeofdayFp) nullptr;
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


void prj_compat_status(void) {
#ifdef PRJ_OS_WIN32
    printf("PRJ_OS_WIN32 %d\n", PRJ_OS_WIN32);
#endif
#ifdef PRJ_OS_WIN
    printf("PRJ_OS_WIN %d\n", PRJ_OS_WIN);
#endif
#ifdef PRJ_OS_32BIT
    printf("PRJ_OS_32BIT %d\n", PRJ_OS_32BIT);
#endif
#ifdef PROJ_OS
    printf("PROJ_OS %d\n", PROJ_OS);
#endif
#ifdef PRJ_OS_WIN64
    printf("PRJ_OS_WIN64 %d\n", PRJ_OS_WIN64);
#endif
#ifdef PRJ_OS_64BIT
    printf("PRJ_OS_64BIT %d\n", PRJ_OS_64BIT);
#endif
#ifdef PRJ_OS_ANDOID
    printf("PRJ_OS_ANDOID %d\n", PRJ_OS_ANDOID);
#endif
#ifdef PRJ_OS_LINUX
    printf("PRJ_OS_LINUX %d\n", PRJ_OS_LINUX);
#endif
#ifdef PRJ_OS_MACOS
    printf("PRJ_OS_MACOS %d\n", PRJ_OS_MACOS);
#endif
    // ==============================
#ifdef PRJ_COMPILER
    printf("PRJ_COMPILER %d\n", PRJ_COMPILER);
#endif
#ifdef PRJ_COMPILER_GCC
    printf("PRJ_COMPILER_GCC %d\n", PRJ_COMPILER_GCC);
#endif
#ifdef PRJ_COMPILER_CLANG
    printf("PRJ_COMPILER_CLANG %d\n", PRJ_COMPILER_CLANG);
#endif
#ifdef PRJ_COMPILER_MSVC
    printf("PRJ_COMPILER_MSVC %d\n", PRJ_COMPILER_MSVC);
#endif
#ifdef PRJ_COMPILER_MINGW32
    printf("PRJ_COMPILER_MINGW32 %d\n", PRJ_COMPILER_MINGW32);
#endif
#ifdef PRJ_COMPILER_MINGW64
    printf("PRJ_COMPILER_MINGW64 %d\n", PRJ_COMPILER_MINGW64);
#endif
    // ==============================
#ifdef _MSC_VER
        printf("_MSC_VER %d\n", _MSC_VER);
#endif
#ifdef __GNUC__
        printf("__GNUC__ %d\n", __GNUC__);
#endif
#ifdef __clang
        printf("__clang %d\n", __clang);
#endif
#ifdef __MINGW32__
        printf("__MINGW32__ %d\n", __MINGW32__);
#endif
#ifdef __MINGW64__
        printf("__MINGW64__ %d\n", __MINGW64__);
#endif
#ifdef __ANDOID__
        printf("__ANDOID__ %d\n", __ANDOID__);
#endif
#ifdef __linux__
        printf("__linux__ %d\n", __linux__);
#endif
#ifdef __APPLE__
        printf("__APPLE__ %d\n", __APPLE__);
#endif
        // ==============================
#ifdef HAVE_BACKTRACE
        printf("HAVE_BACKTRACE %d\n", HAVE_BACKTRACE);
#endif
#ifdef HAVE_CLOCK_GETTIME
        printf("HAVE_CLOCK_GETTIME %d\n", HAVE_CLOCK_GETTIME);
#endif
#ifdef HAVE_CRT_EXTERNS_H
        printf("HAVE_CRT_EXTERNS_H %d\n", HAVE_CRT_EXTERNS_H);
#endif
#ifdef HAVE_DIRENT_H
        printf("HAVE_DIRENT_H %d\n", HAVE_DIRENT_H);
#endif
#ifdef HAVE_DLOPEN
        printf("HAVE_DLOPEN %d\n", HAVE_DLOPEN);
#endif
#ifdef HAVE_FCNTL
        printf("HAVE_FCNTL %d\n", HAVE_FCNTL);
#endif
#ifdef HAVE_FORK
        printf("HAVE_FORK %d\n", HAVE_FORK);
#endif
#ifdef HAVE_FSEEKO
        printf("HAVE_FSEEKO %d\n", HAVE_FSEEKO);
#endif
#ifdef HAVE_FSYNC
        printf("HAVE_FSYNC %d\n", HAVE_FSYNC);
#endif
#ifdef HAVE_FTELLO
        printf("HAVE_FTELLO %d\n", HAVE_FTELLO);
#endif
#ifdef HAVE_GETEUID
        printf("HAVE_GETEUID %d\n", HAVE_GETEUID);
#endif
#ifdef HAVE_GETTIMEOFDAY
        printf("HAVE_GETTIMEOFDAY %d\n", HAVE_GETTIMEOFDAY);
#endif
#ifdef HAVE_IOCTL
        printf("HAVE_IOCTL %d\n", HAVE_IOCTL);
#endif
#ifdef HAVE_ISATTY
        printf("HAVE_ISATTY %d\n", HAVE_ISATTY);
#endif
#ifdef HAVE_KILL
        printf("HAVE_KILL %d\n", HAVE_KILL);
#endif
#ifdef HAVE_LINK
        printf("HAVE_LINK %d\n", HAVE_LINK);
#endif
#ifdef HAVE_MKSTEMP
        printf("HAVE_MKSTEMP %d\n", HAVE_MKSTEMP);
#endif
#ifdef HAVE_MODE_T_TYPE
        printf("HAVE_MODE_T_TYPE %d\n", HAVE_MODE_T_TYPE);
#endif
#ifdef HAVE_OFF_T_TYPE
        printf("HAVE_OFF_T_TYPE %d\n", HAVE_OFF_T_TYPE);
#endif
#ifdef HAVE_PID_T_TYPE
        printf("HAVE_PID_T_TYPE %d\n", HAVE_PID_T_TYPE);
#endif
#ifdef HAVE_READLINK
        printf("HAVE_READLINK %d\n", HAVE_READLINK);
#endif
#ifdef HAVE_REALPATH
        printf("HAVE_REALPATH %d\n", HAVE_REALPATH);
#endif
#ifdef HAVE_SHUTDOWN
        printf("HAVE_SHUTDOWN %d\n", HAVE_SHUTDOWN);
#endif
#ifdef HAVE_SLEEP
        printf("HAVE_SLEEP %d\n", HAVE_SLEEP);
#endif
#ifdef HAVE_STRPTIME
        printf("HAVE_STRPTIME %d\n", HAVE_STRPTIME);
#endif
#ifdef HAVE_STRUCT_TM_TYPE
        printf("HAVE_STRUCT_TM_TYPE %d\n", HAVE_STRUCT_TM_TYPE);
#endif
#ifdef HAVE_SYMLINK
        printf("HAVE_SYMLINK %d\n", HAVE_SYMLINK);
#endif
#ifdef HAVE_SYS_IOCTL_H
        printf("HAVE_SYS_IOCTL_H %d\n", HAVE_SYS_IOCTL_H);
#endif
#ifdef HAVE_SYSLOG_H
        printf("HAVE_SYSLOG_H %d\n", HAVE_SYSLOG_H);
#endif
#ifdef HAVE_SYS_SELECT_H
        printf("HAVE_SYS_SELECT_H %d\n", HAVE_SYS_SELECT_H);
#endif
#ifdef HAVE_SYS_TIME_H
        printf("HAVE_SYS_TIME_H %d\n", HAVE_SYS_TIME_H);
#endif
#ifdef HAVE_SYS_UN_H
        printf("HAVE_SYS_UN_H %d\n", HAVE_SYS_UN_H);
#endif
#ifdef HAVE_UALARM
        printf("HAVE_UALARM %d\n", HAVE_UALARM);
#endif
#ifdef HAVE_UID_T_TYPE
        printf("HAVE_UID_T_TYPE %d\n", HAVE_UID_T_TYPE);
#endif
#ifdef HAVE_UMASK
        printf("HAVE_UMASK %d\n", HAVE_UMASK);
#endif
#ifdef HAVE_UNISTD_H
        printf("HAVE_UNISTD_H %d\n", HAVE_UNISTD_H);
#endif
#ifdef HAVE_USECONDS_T_TYPE
        printf("HAVE_USECONDS_T_TYPE %d\n", HAVE_USECONDS_T_TYPE);
#endif
#ifdef HAVE_USLEEP
        printf("HAVE_USLEEP %d\n", HAVE_USLEEP);
#endif
#ifdef HAVE_UTIMES
        printf("HAVE_UTIMES %d\n", HAVE_UTIMES);
#endif
#ifdef HAVE_VFORK
        printf("HAVE_VFORK %d\n", HAVE_VFORK);
#endif
#ifdef HAVE_DLOPEN_COMPAT
        printf("HAVE_DLOPEN_COMPAT %d\n", HAVE_DLOPEN_COMPAT);
#endif
#ifdef HAVE_STRUCT_SYSINFO_UPTIME
        printf("HAVE_STRUCT_SYSINFO_UPTIME %d\n", HAVE_STRUCT_SYSINFO_UPTIME);
#endif
#ifdef HAVE_SYS_SOCKET_H
        printf("HAVE_SYS_SOCKET_H %d\n", HAVE_SYS_SOCKET_H);
#endif
#ifdef HAVE_NETINET_IN_H
        printf("HAVE_NETINET_IN_H %d\n", HAVE_NETINET_IN_H);
#endif
#ifdef HAVE_NETDB_H
        printf("HAVE_NETDB_H %d\n", HAVE_NETDB_H);
#endif
#ifdef HAVE_ARPA_INET_H
        printf("HAVE_ARPA_INET_H %d\n", HAVE_ARPA_INET_H);
#endif
#ifdef HAVE_SYS_SYSINFO_H
        printf("HAVE_SYS_SYSINFO_H %d\n", HAVE_SYS_SYSINFO_H);
#endif
#ifdef HAVE_WAITPID
        printf("HAVE_WAITPID %d\n", HAVE_WAITPID);
#endif
#ifdef _MSC_EXTENSIONS
        printf("_MSC_EXTENSIONS %d\n", _MSC_EXTENSIONS);
#endif
        // ==============================
#ifdef JIM_IPV6 // Support for IPV6
        printf("JIM_IPV6 %d\n", JIM_IPV6);
#endif
#ifdef JIM_MAINTAINER // Extra output
        printf("JIM_MAINTAINER %d\n", JIM_MAINTAINER);
#endif
#ifdef JIM_MATH_FUNCTIONS
        printf("JIM_MATH_FUNCTIONS %d\n", JIM_MATH_FUNCTIONS);
#endif
#ifdef JIM_REFERENCES // Include References command_
        printf("JIM_REFERENCES %d\n", JIM_REFERENCES);
#endif
#ifdef JIM_REGEXP // Include regexp command_
        printf("JIM_REGEXP %d\n", JIM_REGEXP);
#endif
#ifdef JIM_STATICLIB // #Unused
        printf("JIM_STATICLIB %d\n", JIM_STATICLIB);
#endif
#ifdef JIM_UTF8
        printf("JIM_UTF8 %d\n", JIM_UTF8);
#endif
#ifdef JIM_VERSION
        printf("JIM_VERSION %d\n", JIM_VERSION);
#endif
#ifdef JIM_DOCS // #Unused
        printf("JIM_DOCS %d\n", JIM_DOCS);
#endif
        // ==============================
#ifdef jim_ext_aio 
        printf("jim_ext_aio %d\n", jim_ext_aio);
#endif
#ifdef jim_ext_array 
        printf("jim_ext_array %d\n", jim_ext_array);
#endif
#ifdef jim_ext_binary 
        printf("jim_ext_binary %d\n", jim_ext_binary);
#endif
#ifdef jim_ext_clock 
        printf("jim_ext_clock %d\n", jim_ext_clock);
#endif
#ifdef jim_ext_exec 
        printf("jim_ext_exec %d\n", jim_ext_exec);
#endif
#ifdef jim_ext_file 
        printf("jim_ext_file %d\n", jim_ext_file);
#endif
#ifdef jim_ext_glob 
        printf("jim_ext_glob %d\n", jim_ext_glob);
#endif
#ifdef jim_ext_history 
        printf("jim_ext_history %d\n", jim_ext_history);
#endif
#ifdef jim_ext_interp 
        printf("jim_ext_interp %d\n", jim_ext_interp);
#endif
#ifdef jim_ext_load 
        printf("jim_ext_load %d\n", jim_ext_load);
#endif
#ifdef jim_ext_namespace 
        printf("jim_ext_namespace %d\n", jim_ext_namespace);
#endif
#ifdef jim_ext_nshelper 
        printf("jim_ext_nshelper %d\n", jim_ext_nshelper);
#endif
#ifdef jim_ext_oo 
        printf("jim_ext_oo %d\n", jim_ext_oo);
#endif
#ifdef jim_ext_pack 
        printf("jim_ext_pack %d\n", jim_ext_pack);
#endif
#ifdef jim_ext_package 
        printf("jim_ext_package %d\n", jim_ext_package);
#endif
#ifdef jim_ext_posix 
        printf("jim_ext_posix %d\n", jim_ext_posix);
#endif
#ifdef jim_ext_readdir 
        printf("jim_ext_readdir %d\n", jim_ext_readdir);
#endif
#ifdef jim_ext_regexp 
        printf("jim_ext_regexp %d\n", jim_ext_regexp);
#endif
#ifdef jim_ext_signal 
        printf("jim_ext_signal %d\n", jim_ext_signal);
#endif
#ifdef jim_ext_stdlib 
        printf("jim_ext_stdlib %d\n", jim_ext_stdlib);
#endif
#ifdef jim_ext_syslog 
        printf("jim_ext_syslog %d\n", jim_ext_syslog);
#endif
#ifdef jim_ext_tclcompat 
        printf("jim_ext_tclcompat %d\n", jim_ext_tclcompat);
#endif
#ifdef jim_ext_tclprefix 
        printf("jim_ext_tclprefix %d\n", jim_ext_tclprefix);
#endif
#ifdef jim_ext_tree 
        printf("jim_ext_tree %d\n", jim_ext_tree);
#endif
#ifdef jim_ext_zlib 
        printf("jim_ext_zlib %d\n", jim_ext_zlib);
#endif
}
