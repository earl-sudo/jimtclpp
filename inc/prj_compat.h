#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*
 * An extremely think compatibility layer.
 * 
 * Goal: Deal with non-portable functions used in Jim.
 *
 * To be included:
 ** Function is not totally portable, with it either not being defined on some platform or behaving 
 *  different than what we consider "expected".
 ** Not too complicated.  Really complicated functions don't fit with this plan.
 *
 * Any function which may not be considered totally portable can go through this layer.  The
 * goal of the layer is to mimic the function to it's best ability as defined by the OS in it's 
 * comment would define the function.  This mimic behavior makes it easy for people to find 
 * documentation and examples to work from, while potentially working on platforms where
 * this would not normally work.
 * Since all these function are called off function pointers, it is possible to leave the 
 * function as a NULL to indicate that this function makes no sense on some other platform.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t prj_pid_t;
typedef uint32_t prj_uid_t;
typedef int64_t prj_off_t;
typedef uint32_t prj_useconds_t;
typedef int64_t prj_ssize_t;
typedef uint32_t prj_mode_t;
typedef int32_t prj_clockid_t;
typedef uint64_t prj_time_t;

struct prj_timezone {
    int tz_minuteswest;     /* minutes west of Greenwich */
    int tz_dsttime;         /* type of DST correction */
};

struct prj_timespec {
    prj_time_t   tv_sec;        /* seconds */
    long         tv_nsec;       /* nanoseconds */
};

struct prj_DIR;
struct prj_dirent;

/* Definitions for 
 * backtrace_symbols()
 * backtrace_symbols_fd()
 * bracktrace()
 * clock_gettime()
 * closedir()
 * closelog()
 * dlclose()
 * dlerror()
 * dlopen()
 * dlsym()
 * dup()
 * dup2()
 * execvp()
 * execvpe()
 * fcntl()
 * fdopen()
 * fork()
 * fseeko()
 * ftello()
 * getenv()
 * geteuid()
 * getpid()
 * gettimeofday()
 * gmtime()
 * ioctl()
 * isatty()
 * kill()
 * link()
 * localtime()
 * localtime_r()
 * mkstemp()
 * mktime()
 * opendir()
 * openlog()
 * pipe()
 * raise()
 * readdir()
 * readlink()
 * realpath()
 * shutdown()
 * sigaction()
 * signal()
 * sleep()
 * strptime()
 * symlink()
 * syslog()
 * ualarm()
 * umask()
 * useconds()
 * usleep()
 * utimes()
 * vfork()
 * waitpid()
 * 
 * Extensions:
 * prj_dirent_dname()
 * prj_environ()
 */

/* Linux64_2020: pid_t fork(void); */
typedef prj_pid_t (*prj_forkFp)(void);

extern prj_forkFp prj_fork;

/* Linux64_2020: int fsync(int fd); */
typedef int (*prj_fsyncFp)(int fd);
extern prj_fsyncFp prj_fsync;

/* Linux64_2020: int fseeko(FILE *stream, off_t offset, int whence); */
typedef int (*prj_fseekoFp)(void *stream, prj_off_t offset, int whence);
extern prj_fseekoFp prj_fseeko;
 
/* Linux64_2020: uid_t geteuid(void); */
typedef prj_uid_t (*prj_geteuidFp)(void);
extern prj_geteuidFp prj_geteuid;

/* Linux64_2020: int link(const char *oldpath, const char *newpath); */
typedef int (*prj_linkFp)(const char *oldpath, const char *newpath);
extern prj_linkFp prj_link;

/* Linux64_2020: int symlink(const char *target, const char *linkpath); */
typedef int (*prj_symlinkFp)(const char *target, const char *linkpath);
extern prj_symlinkFp prj_symlink;

/* Linux64_2020: off_t ftello(FILE *stream); */
typedef prj_off_t (*prj_ftelloFp)(FILE *stream);
extern prj_ftelloFp prj_ftello;

/* Linux64_2020: char **backtrace_symbols(void *const *buffer, int size); */
typedef char **(*prj_backtrace_symbolsFp)(void *const *buffer, int size);
extern prj_backtrace_symbolsFp prj_backtrace_symbols;

/* Linux64_2020: void backtrace_symbols_fd(void *const *buffer, int size, int fd); */
typedef void (*prj_backtrace_symbols_fdFp)(void *const *buffer, int size, int fd);
extern prj_backtrace_symbols_fdFp prj_backtrace_symbols_fd;

/* Linux64_2020: char **backtrace_symbols(void *const *buffer, int size); */
typedef int (*prj_backtraceFp)(void **buffer, int size);;
extern prj_backtraceFp prj_backtrace;

/* Linux64_2020: char *realpath(const char *path, char *resolved_path); */
typedef char *(*prj_realpathFp)(const char *path, char *resolved_path);
extern prj_realpathFp prj_realpath;

/* Linux64_2020: mode_t umask(mode_t mask); */
typedef prj_mode_t (*prj_umaskFp)(prj_mode_t mask);
extern prj_umaskFp prj_umask;

/* Linux64_2020: int mkstemp(char *template); */
typedef int (*prj_mkstempFp)(char *template_);
extern prj_mkstempFp prj_mkstemp; 

/* Linux64_2020: useconds_t ualarm(useconds_t usecs, useconds_t interval); */
typedef prj_useconds_t (*prj_ualarmFp)(prj_useconds_t usecs, prj_useconds_t interval);
extern prj_ualarmFp prj_ualarm;

/* Linux64_2020: int usleep(useconds_t usec); */
typedef int (*usleepFp)(prj_useconds_t usec);

/* Linux64_2020: int isatty(int fd); */
typedef int (*prj_isattyFp)(int fd);
extern prj_isattyFp prj_isatty;

/* Linux64_2020: ssize_t readlink(const char *pathname, char *buf, size_t bufsiz); */
typedef prj_ssize_t (*prj_readlinkFp)(const char *pathname, char *buf, size_t bufsiz);
extern prj_readlinkFp prj_readlink;

/* Linux64_2020: int shutdown(int sockfd, int how); */
typedef int (*prj_shutdownFp)(int sockfd, int how);
extern prj_shutdownFp prj_shutdown;

/* Linux64_2020: int usleep(useconds_t usec); */
typedef int (*prj_usleepFp)(prj_useconds_t usec);
extern prj_usleepFp prj_usleep;

/* Linux64_2020: int utimes(const char *filename, const struct timeval times[2]);
           struct utimbuf {
               time_t actime;       
               time_t modtime;      
           };
*/
typedef uint64_t prj_time_t;
struct prj_timeval {
    long tv_sec;       
    long tv_usec;      
};
typedef int (*prj_utimesFp)(const char *filename, const struct prj_timeval times[2]);
extern prj_utimesFp prj_utimes;

/* Linux64_2020: pid_t vfork(void); */
typedef prj_pid_t (*prj_vforkFp)(void);
extern prj_vforkFp prj_vfork;

/* Linux64_2020: int dup(int oldfd); */
typedef int (*prj_dupFp)(int oldfd);
extern prj_dupFp prj_dup;

/* Linux64_2020: int dup2(int oldfd, int newfd); */
typedef int (*prj_dup2Fp)(int oldfd, int newfd);
extern prj_dup2Fp prj_dup2;

/* Linux64_2020: int execvp(const char *file, char *const argv[]); */
typedef int (*prj_execvpFp)(const char *file, char *const argv[]);
extern prj_execvpFp prj_execvp;

/* Linux64_2020: int execvpe(const char *file, char *const argv[], char *const envp[]); */
typedef int (*prj_execvpeFp)(const char *file, char *const argv[], char *const envp[]);
extern prj_execvpeFp prj_execvpe;

/* Linux64_2020: int fcntl(int fd, int cmd, ...  ); */
typedef int (*prj_fcntlFp)(int fd, int cmd, ...  );
extern prj_fcntlFp prj_fcntl;

/* Linux64_2020: FILE *fdopen(int fd, const char *mode); */
typedef FILE *(*prj_fdopenFp)(int fd, const char *mode);
extern prj_fdopenFp prj_fdopen;

/* Linux64_2020: char *getenv(const char *name); */
typedef char *(*prj_getenvFp)(const char *name);
extern prj_getenvFp prj_getenv;

/* Linux64_2020: pid_t getpid(void); */
typedef prj_pid_t (*prj_getpidFp)(void);
extern prj_getpidFp prj_getpid;

/* Linux64_2020: int ioctl(int fd, unsigned long request, ...); */
typedef int (*prj_ioctlFp)(int fd, unsigned long request, ...);
extern prj_ioctlFp prj_ioctl;

/* Linux64_2020: int kill(pid_t pid, int sig); */
typedef int (*prj_killFp)(prj_pid_t pid, int sig);
extern prj_killFp prj_kill;

/* Linux64_2020: void openlog(const char *ident, int option, int facility); */
typedef void (*prj_openlogFp)(const char *ident, int option, int facility);
extern prj_openlogFp prj_openlog;

/* Linux64_2020: void syslog(int priority, const char *format, ...); */
typedef void (*prj_syslogFp)(int priority, const char *format, ...);
extern prj_syslogFp prj_syslog;

/* Linux64_2020: void closelog(void); */
typedef void (*prj_closelogFp)(void);
extern prj_closelogFp prj_closelog;

/* Linux64_2020: int pipe(int pipefd[2]); */
typedef int (*prj_pipeFp)(int pipefd[2]);
extern prj_pipeFp prj_pipe;

/* Linux64_2020:  unsigned int sleep(unsigned int seconds); */
typedef unsigned int (*prj_sleepFp)(unsigned int seconds);
extern prj_sleepFp prj_sleep;

/* Linux64_2020: int raise(int sig); */
typedef int (*prj_raiseFp)(int sig);
extern prj_raiseFp prj_raise;

/* Linux64_2020: int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact); */
typedef void (*prj_sighandler_t)(int);	

/* Linux64_2020: sighandler_t signal(int signum, sighandler_t handler); */
typedef prj_sighandler_t (*prj_signalFp)(int signum, prj_sighandler_t handler);
extern prj_signalFp prj_signal;

struct prj_tm {
    int tm_sec;    /* Seconds (0-60) */
    int tm_min;    /* Minutes (0-59) */
    int tm_hour;   /* Hours (0-23) */
    int tm_mday;   /* Day of the month (1-31) */
    int tm_mon;    /* Month (0-11) */
    int tm_year;   /* Year - 1900 */
    int tm_wday;   /* Day of the week (0-6, Sunday = 0) */
    int tm_yday;   /* Day in the year (0-365, 1 Jan = 0) */
    int tm_isdst;  /* Daylight saving time */
    char pad[32];
};

/* Linux64_2020: time_t mktime(struct tm *tm); */
typedef prj_time_t (*prj_mktimeFp)(struct prj_tm *tm);
extern prj_mktimeFp prj_mktime;

/* Linux64_2020: struct tm *gmtime(const time_t *timep); */
typedef struct prj_tm *(*prj_gmtimeFp)(const prj_time_t *timep);
extern prj_gmtimeFp prj_gmtime;

/* Linux64_2020: struct tm *localtime(const time_t *timep); */
typedef struct prj_tm *(*prj_localtimeFp)(const prj_time_t *timep);
extern prj_localtimeFp prj_localtime;

/* Linux64_2020: struct tm *localtime_r(const time_t *timep, struct tm *result); */
typedef struct prj_tm *(*prj_localtime_rFp)(const prj_time_t *timep, struct prj_tm *result);
extern prj_localtime_rFp prj_localtime_r;

/* Linux64_2020: char *strptime(const char *s, const char *format, struct tm *tm); */
typedef char *(*prj_strptimeFp)(const char *s, const char *format, struct prj_tm *tm);
extern prj_strptimeFp prj_strptime;

/* Linux64_2020: int gettimeofday(struct prj_timeval * tp, struct prj_timezone * tzp) */
typedef int (*prj_gettimeofdayFp)(struct prj_timeval * tp, struct prj_timezone * tzp);
extern prj_gettimeofdayFp prj_gettimeofday;

/* Linux64_2020: int clock_gettime(clockid_t clk_id, struct timespec *tp); */
typedef int (*prj_clock_gettimeFp)(prj_clockid_t clk_id, struct prj_timespec* tp);
extern prj_clock_gettimeFp prj_clock_gettime;

/* Linux64_2020: void* dlopen(const char* path, int mode);*/
typedef void* (*prj_dlopenFp)(const char* path, int mode);
extern prj_dlopenFp prj_dlopen;

/* Linux64_2020: int dlclose(void* handle); */
typedef int (*prj_dlcloseFp)(void* handle);
extern prj_dlcloseFp prj_dlclose;

/* Linux64_2020: void* dlsym(void* handle, const char* symbol); */
typedef void* (*prj_dlsymFp)(void* handle, const char* symbol);
extern prj_dlsymFp prj_dlsym;

/* Linux64_2020: char* dlerror(void); */
typedef char* (*prj_dlerrorFp)(void);
extern prj_dlerrorFp prj_dlerror;

/* Linux64_2020: DIR* opendir(const char* name); */
typedef prj_DIR* (*prj_opendirFp)(const char* name);
extern prj_opendirFp prj_opendir;

/* Linux64_2020: int closedir(DIR* dir); */
typedef int (*prj_closedirFp)(prj_DIR* dir);
extern prj_closedirFp prj_closedir;

/* Linux64_2020: struct dirent* readdir(DIR* dir); */
typedef struct prj_dirent* (*prj_readdirFp)(prj_DIR* dir);
extern prj_readdirFp prj_readdir;

/* Extension */
typedef const char* (*prj_dirent_dnameFp)(prj_dirent* de);
extern prj_dirent_dnameFp prj_dirent_dname;

/* Extension */
typedef char** (*prj_environFp)(void);
extern prj_environFp prj_environ;

/* Linux64_2020: pid_t waitpid(pid_t pid, int *wstatus, int options); */
typedef prj_pid_t(*prj_waitpidFp)(prj_pid_t pid, int* wstatus, int options);
extern prj_waitpidFp prj_waitpid;

static inline int prj_funcDef(void* fp) { return NULL != fp; }

#ifdef __cplusplus
}; /* extern "C" */
#endif

#ifdef __cplusplus
template<typename T> int prj_funcDef(T* fp) { return NULL != fp; }
template<typename F, typename T> T testConv(F& v) { 
    T ret = static_cast<T>(v);
    if (ret != v) { printf("ERROR: testConv\n"); }
    return ret;
}
#endif

#ifdef _MSC_VER // #optionalCode
#  define _CRT_SECURE_NO_WARNINGS 1
#  define strdup _strdup
#  define fdopen _fdopen
#  define access _access
#  define unlink _unlink
#  define rmdir _rmdir
#  define close _close
#  define chdir _chdir
#  define getcwd _getcwd
#  define dup _dup
#  define dup2 _dup2
#  define execvp _execvp
#  define execvpe _execvpe
#ifndef pipe
#  define pipe _pipe
#endif
#  define getpid _getpid
#  define mkdir _mkdir
#  define fileno _fileno
#  define strcasecmp _stricmp
#  define strtoull _strtoui64
#  define open _open
#  define lseek _lseek
#endif