#pragma once

#include <jimautoconf.h>

extern PRJ_OS g_prj_os; // Allow for runtime check of os

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

typedef int64_t prj_pid_t;
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
 * close
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

/* Maybe add these
U accept@@GLIBC_2.2.5					s
U access@@GLIBC_2.2.5
U alarm@@GLIBC_2.2.5					maybe <posix>
U chdir@@GLIBC_2.2.5					maybe <posix>
U clock@@GLIBC_2.2.5					maybe <posix>
U bind@@GLIBC_2.2.5					    s
U connect@@GLIBC_2.2.5					s
U freeaddrinfo@@GLIBC_2.2.5				s
U getaddrinfo@@GLIBC_2.2.5				s
U gethostname@@GLIBC_2.2.5				s
U getsockopt@@GLIBC_2.2.5				s
U inet_ntop@@GLIBC_2.2.5				s
U listen@@GLIBC_2.2.5					s
U lseek@@GLIBC_2.2.5					maybe <posix>
U mkdir@@GLIBC_2.2.5					maybe <posix>
U open@@GLIBC_2.2.5					    maybe <posix>
U raise@@GLIBC_2.2.5					maybe <posix> NOTUSED
U recvfrom@@GLIBC_2.2.5				    s NOTUSED
U remove@@GLIBC_2.2.5					maybe <posix>
U rmdir@@GLIBC_2.2.5					maybe <posix>
U select@@GLIBC_2.2.5					s
U sendto@@GLIBC_2.2.5					s
U setsockopt@@GLIBC_2.2.5				s
U socket@@GLIBC_2.2.5					s
U socketpair@@GLIBC_2.2.5				s
U strcasecmp@@GLIBC_2.2.5				maybe <posix>
U strncasecmp@@GLIBC_2.2.5				maybe <posix>
U strpbrk@@GLIBC_2.2.5					maybe <posix>
U sysinfo@@GLIBC_2.2.5					maybe <linux>
U unlink@@GLIBC_2.2.5					maybe <posix>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int access(const char *pathname, int mode);
unsigned int alarm(unsigned int seconds);
int chdir(const char *path);
clock_t clock(void);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
int gethostname(char *name, size_t len);
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
int listen(int sockfd, int backlog);
off_t lseek(int fd, off_t offset, int whence);
int mkdir(const char *pathname, mode_t mode);
int open(const char *pathname, int flags);
int raise(int sig);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
int remove(const char *pathname);
int rmdir(const char *pathname);
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
size_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int socket(int domain, int type, int protocol);
int socketpair(int domain, int type, int protocol, int sv[2]);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
char *strpbrk(const char *s, const char *accept);
int sysinfo(struct sysinfo *info);
int unlink(const char *pathname);
*/

/* Linux_2020: pid_t fork(void); */
typedef prj_pid_t (*prj_forkFp)(void);

extern prj_forkFp prj_fork;

/* Linux_2020: int fsync(int fd); */
typedef int (*prj_fsyncFp)(int fd);
extern prj_fsyncFp prj_fsync;

/* Linux_2020: int fseeko(FILE *stream, off_t offset, int whence); */
typedef int (*prj_fseekoFp)(void *stream, prj_off_t offset, int whence);
extern prj_fseekoFp prj_fseeko;
 
/* Linux_2020: uid_t geteuid(void); */
typedef prj_uid_t (*prj_geteuidFp)(void);
extern prj_geteuidFp prj_geteuid;

/* Linux_2020: int link(const char *oldpath, const char *newpath); */
typedef int (*prj_linkFp)(const char *oldpath, const char *newpath);
extern prj_linkFp prj_link;

/* Linux_2020: int symlink(const char *target, const char *linkpath); */
typedef int (*prj_symlinkFp)(const char *target, const char *linkpath);
extern prj_symlinkFp prj_symlink;

/* Linux_2020: off_t ftello(FILE *stream); */
typedef prj_off_t (*prj_ftelloFp)(FILE *stream);
extern prj_ftelloFp prj_ftello;

/* Linux_2020: char **backtrace_symbols(void *const *buffer, int size); */
typedef char **(*prj_backtrace_symbolsFp)(void *const *buffer, int size);
extern prj_backtrace_symbolsFp prj_backtrace_symbols;

/* Linux_2020: void backtrace_symbols_fd(void *const *buffer, int size, int fd); */
typedef void (*prj_backtrace_symbols_fdFp)(void *const *buffer, int size, int fd);
extern prj_backtrace_symbols_fdFp prj_backtrace_symbols_fd;

/* Linux_2020: char **backtrace_symbols(void *const *buffer, int size); */
typedef int (*prj_backtraceFp)(void **buffer, int size);;
extern prj_backtraceFp prj_backtrace;

/* Linux_2020: char *realpath(const char *path, char *resolved_path); */
typedef char *(*prj_realpathFp)(const char *path, char *resolved_path);
extern prj_realpathFp prj_realpath;

/* Linux_2020: mode_t umask(mode_t mask); */
typedef prj_mode_t (*prj_umaskFp)(prj_mode_t mask);
extern prj_umaskFp prj_umask;

/* Linux_2020: int mkstemp(char *template); */
typedef int (*prj_mkstempFp)(char *template_);
extern prj_mkstempFp prj_mkstemp; 

/* Linux_2020: useconds_t ualarm(useconds_t usecs, useconds_t interval); */
typedef prj_useconds_t (*prj_ualarmFp)(prj_useconds_t usecs, prj_useconds_t interval);
extern prj_ualarmFp prj_ualarm;

/* Linux_2020: int usleep(useconds_t usec); */
typedef int (*usleepFp)(prj_useconds_t usec); 

/* Linux_2020: int isatty(int fd); */
typedef int (*prj_isattyFp)(int fd);
extern prj_isattyFp prj_isatty;

/* Linux_2020: ssize_t readlink(const char *pathname, char *buf, size_t bufsiz); */
typedef prj_ssize_t (*prj_readlinkFp)(const char *pathname, char *buf, size_t bufsiz);
extern prj_readlinkFp prj_readlink;

/* Linux_2020: int shutdown(int sockfd, int how); */
//typedef int (*prj_shutdownFp)(int sockfd, int how);
//extern prj_shutdownFp prj_shutdown;

/* Linux_2020: int usleep(useconds_t usec); */
typedef int (*prj_usleepFp)(prj_useconds_t usec);
extern prj_usleepFp prj_usleep;

/* Linux_2020: int utimes(const char *filename, const struct timeval times[2]);
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

/* Linux_2020: pid_t vfork(void); */
typedef prj_pid_t (*prj_vforkFp)(void);
extern prj_vforkFp prj_vfork;

/* Linux_2020: int dup(int oldfd); */
typedef int (*prj_dupFp)(int oldfd);
extern prj_dupFp prj_dup;

/* Linux_2020: int dup2(int oldfd, int newfd); */
typedef int (*prj_dup2Fp)(int oldfd, int newfd);
extern prj_dup2Fp prj_dup2;

/* Linux_2020: int execvp(const char *file, char *const argv[]); */
typedef int (*prj_execvpFp)(const char *file, char *const argv[]);
extern prj_execvpFp prj_execvp;

/* Linux_2020: int execvpe(const char *file, char *const argv[], char *const envp[]); */
typedef int (*prj_execvpeFp)(const char *file, char *const argv[], char *const envp[]);
extern prj_execvpeFp prj_execvpe;

/* Linux_2020: int fcntl(int fd, int cmd, ...  ); */
typedef int (*prj_fcntlFp)(int fd, int cmd, ...  );
extern prj_fcntlFp prj_fcntl;

/* Linux_2020: FILE *fdopen(int fd, const char *mode); */
typedef FILE *(*prj_fdopenFp)(int fd, const char *mode);
extern prj_fdopenFp prj_fdopen;

/* Linux_2020: char *getenv(const char *name); */
typedef char *(*prj_getenvFp)(const char *name);
extern prj_getenvFp prj_getenv;

/* Linux_2020: pid_t getpid(void); */
typedef prj_pid_t (*prj_getpidFp)(void);
extern prj_getpidFp prj_getpid;

/* Linux_2020: int ioctl(int fd, unsigned long request, ...); */
typedef int (*prj_ioctlFp)(int fd, unsigned long request, ...);
extern prj_ioctlFp prj_ioctl;

/* Linux_2020: int kill(pid_t pid, int sig); */
typedef int (*prj_killFp)(prj_pid_t pid, int sig);
extern prj_killFp prj_kill;

/* Linux_2020: void openlog(const char *ident, int option, int facility); */
typedef void (*prj_openlogFp)(const char *ident, int option, int facility);
extern prj_openlogFp prj_openlog;

/* Linux_2020: void syslog(int priority, const char *format, ...); */
typedef void (*prj_syslogFp)(int priority, const char *format, ...);
extern prj_syslogFp prj_syslog;

/* Linux_2020: void closelog(void); */
typedef void (*prj_closelogFp)(void);
extern prj_closelogFp prj_closelog;

/* Linux_2020: int pipe(int pipefd[2]); */
typedef int (*prj_pipeFp)(int pipefd[2]);
extern prj_pipeFp prj_pipe;

/* Linux_2020:  unsigned int sleep(unsigned int seconds); */
typedef unsigned int (*prj_sleepFp)(unsigned int seconds);
extern prj_sleepFp prj_sleep;

/* Linux_2020: int raise(int sig); */
typedef int (*prj_raiseFp)(int sig);
extern prj_raiseFp prj_raise;

/* Linux_2020: int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact); */
typedef void (*prj_sighandler_t)(int);	

/* Linux_2020: sighandler_t signal(int signum, sighandler_t handler); */
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

/* Linux_2020: time_t mktime(struct tm *tm); */
typedef prj_time_t (*prj_mktimeFp)(struct prj_tm *tm);
extern prj_mktimeFp prj_mktime;

/* Linux_2020: struct tm *gmtime(const time_t *timep); */
typedef struct prj_tm *(*prj_gmtimeFp)(const prj_time_t *timep);
extern prj_gmtimeFp prj_gmtime;

/* Linux_2020: struct tm *localtime(const time_t *timep); */
typedef struct prj_tm *(*prj_localtimeFp)(const prj_time_t *timep);
extern prj_localtimeFp prj_localtime;

/* Linux_2020: struct tm *localtime_r(const time_t *timep, struct tm *result); */
typedef struct prj_tm *(*prj_localtime_rFp)(const prj_time_t *timep, struct prj_tm *result);
extern prj_localtime_rFp prj_localtime_r;

/* Linux_2020: char *strptime(const char *s, const char *format, struct tm *tm); */
typedef char *(*prj_strptimeFp)(const char *s, const char *format, struct prj_tm *tm);
extern prj_strptimeFp prj_strptime;

/* Linux_2020: int gettimeofday(struct prj_timeval * tp, struct prj_timezone * tzp) */
typedef int (*prj_gettimeofdayFp)(struct prj_timeval * tp, struct prj_timezone * tzp);
extern prj_gettimeofdayFp prj_gettimeofday;

/* Linux_2020: int clock_gettime(clockid_t clk_id, struct timespec *tp); */
typedef int (*prj_clock_gettimeFp)(prj_clockid_t clk_id, struct prj_timespec* tp);
extern prj_clock_gettimeFp prj_clock_gettime;

/* Linux_2020: void* dlopen(const char* path, int mode);*/
typedef void* (*prj_dlopenFp)(const char* path, int mode);
extern prj_dlopenFp prj_dlopen;

/* Linux_2020: int dlclose(void* handle); */
typedef int (*prj_dlcloseFp)(void* handle);
extern prj_dlcloseFp prj_dlclose;

/* Linux_2020: void* dlsym(void* handle, const char* symbol); */
typedef void* (*prj_dlsymFp)(void* handle, const char* symbol);
extern prj_dlsymFp prj_dlsym;

/* Linux_2020: char* dlerror(void); */
typedef char* (*prj_dlerrorFp)(void);
extern prj_dlerrorFp prj_dlerror;

/* Linux_2020: DIR* opendir(const char* name); */
typedef prj_DIR* (*prj_opendirFp)(const char* name);
extern prj_opendirFp prj_opendir;

/* Linux_2020: int closedir(DIR* dir); */
typedef int (*prj_closedirFp)(prj_DIR* dir);
extern prj_closedirFp prj_closedir;

/* Linux_2020: struct dirent* readdir(DIR* dir); */
typedef struct prj_dirent* (*prj_readdirFp)(prj_DIR* dir);
extern prj_readdirFp prj_readdir;

/* Extension */
typedef const char* (*prj_dirent_dnameFp)(prj_dirent* de);
extern prj_dirent_dnameFp prj_dirent_dname;

/* Extension */
typedef char** (*prj_environFp)(void);
extern prj_environFp prj_environ;

/* Linux_2020: pid_t waitpid(pid_t pid, int *wstatus, int options); */
typedef prj_pid_t(*prj_waitpidFp)(prj_pid_t pid, int* wstatus, int options);
extern prj_waitpidFp prj_waitpid;

/* Linux_2020: int close(int fd);*/
typedef int (*prj_closeFp)(int fd);
extern prj_closeFp prj_close;


/* Linux_2020: int sysinfo(struct sysinfo *info); */
struct prj_sysinfo {
    char        dummy_[64];
};
typedef int (*prj_sysinfoFp)(struct prj_sysinfo* info);
extern prj_sysinfoFp prj_sysinfo;

/* Extension */
long prj_sysinfo_uptime(struct prj_sysinfo* info);


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

/* Some really portable functions which for some silly reason Windows wants to rename. */
#ifdef _MSC_VER // #optionalCode
#  ifndef _CRT_SECURE_NO_WARNINGS
#    define _CRT_SECURE_NO_WARNINGS 1
#  endif

// <string.h>
#  define prj_strdup _strdup

// <stdio.h>
#  define prj_unlink _unlink
#  define prj_fileno _fileno

// <direct.h>
#  define prj_rmdir _rmdir
#  define prj_chdir _chdir
#  define prj_getcwd _getcwd
#  define prj_mkdir _mkdir
#  define prj_mkdir2(PATH, ACCESS) prj_mkdir (PATH)

// <io.h>
#  define prj_access _access
#  define prj_open _open
#  define prj_lseek _lseek

// <process.h>

// <stdlib.h>
#  define prj_strtoull _strtoui64

// sockets
#define prj_getaddrinfo getaddrinfo
#define prj_freeaddrinfo freeaddrinfo
#define prj_inet_ntop inet_ntop
#define prj_recvfrom recvfrom
#define prj_sendto sendto
#define prj_accept accept
#define prj_listen listen
#define prj_shutdown shutdown
#define prj_getsockopt getsockopt
#define prj_setsockopt setsockopt
#define prj_socket socket
#define prj_connect connect
#define prj_bind bind
#define prj_socketpair socketpair
#define prj_select select
#define prj_gethostname gethostname

// <time.h>
//#define prj_gmtime gmtime

// <unistd.h>
//#define prj_lseek lseek
#define prj_alarm alarm

// <signal.h>
//#define prj_signal signal
#define prj_sigaction sigaction

// <strings.h>
#define prj_strcasecmp strcasecmp

#else
// <string.h>
#  define prj_strdup strdup

// <stdio.h>
#  define prj_unlink unlink
#  define prj_fileno fileno

// <direct.h>
#  define prj_rmdir rmdir
#  define prj_chdir chdir
#  define prj_getcwd getcwd
#  define prj_mkdir mkdir

#ifndef HAVE_MKDIR_1ARG
#  define prj_mkdir2(PATH, ACCESS) mkdir( PATH , ACCESS )
#else
#  define prj_mkdir2(PATH, ACCESS) mkdir( PATH )
#endif

// <io.h>
#  define prj_access access
#  define prj_open open
#  define prj_lseek lseek

// <process.h>

// <stdlib.h>
#  define prj_strtoull strtoui64

// sockets
#define prj_getaddrinfo getaddrinfo
#define prj_freeaddrinfo freeaddrinfo
#define prj_inet_ntop inet_ntop
#define prj_recvfrom recvfrom
#define prj_sendto sendto
#define prj_accept accept
#define prj_listen listen
#define prj_shutdown shutdown
#define prj_getsockopt getsockopt
#define prj_setsockopt setsockopt
#define prj_socket socket
#define prj_connect connect
#define prj_bind bind
#define prj_socketpair socketpair
#define prj_select select
#define prj_gethostname gethostname

// <time.h>
//#define prj_gmtime gmtime

// <unistd.h>
//#define prj_lseek lseek
#define prj_alarm alarm

// <signal.h>
//#define prj_signal signal
#define prj_sigaction sigaction

// <strings.h>
#define prj_strcasecmp strcasecmp

#endif


// Wrappers for IO 
#define prj_fread fread
#define prj_fgets fgets
#define prj_fwrite fwrite
#define prj_write write