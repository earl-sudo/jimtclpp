/* Syslog interface for tcl
 * Copyright Victor Wagner <vitus@ice.ru> at
 * http://www.ice.ru/~vitus/works/tcl.html#syslog
 *
 * Slightly modified by Steve Bennett <steveb@snapgear.com>
 * Ported to Jim by Steve Bennett <steveb@workware.net.au>
 */


#include <jimautoconf.h>

#ifndef _WIN32

#include <syslog.h> // #NonPortHeader
//#include <string.h>

#include <prj_compat.h>
#include <jim.h>

#if jim_ext_syslog

BEGIN_JIM_NAMESPACE

typedef struct
{
    int logOpened;
    int facility;
    int options;
    char ident[32];
} SyslogInfo;

#ifndef LOG_AUTHPRIV
# define LOG_AUTHPRIV LOG_AUTH
#endif

#if 0
static const char * const facilities[] = {
    [LOG_AUTHPRIV] = "authpriv",
    [LOG_CRON] = "cron",
    [LOG_DAEMON] = "daemon",
    [LOG_KERN] = "kernel",
    [LOG_LPR] = "lpr",

    [LOG_MAIL] = "mail",
    [LOG_NEWS] = "news",
    [LOG_SYSLOG] = "syslog",
    [LOG_USER] = "user",
    [LOG_UUCP] = "uucp",

    [LOG_LOCAL0] = "local0",
    [LOG_LOCAL1] = "local1",
    [LOG_LOCAL2] = "local2",
    [LOG_LOCAL3] = "local3",
    [LOG_LOCAL4] = "local4",

    [LOG_LOCAL5] = "local5",
    [LOG_LOCAL6] = "local6",
    [LOG_LOCAL7] = "local7",
};
#else
static const char* const facilities[] = {
    "kernel",  /* 0 */
    NULL,  /* 1 */
    NULL,  /* 2 */
    NULL,  /* 3 */
    NULL,  /* 4 */
    NULL,  /* 5 */
    NULL,  /* 6 */
    NULL,  /* 7 */
    "user",  /* 8 */
    NULL,  /* 9 */
    NULL,  /* 10 */
    NULL,  /* 11 */
    NULL,  /* 12 */
    NULL,  /* 13 */
    NULL,  /* 14 */
    NULL,  /* 15 */
    "mail",  /* 16 */
    NULL,  /* 17 */
    NULL,  /* 18 */
    NULL,  /* 19 */
    NULL,  /* 20 */
    NULL,  /* 21 */
    NULL,  /* 22 */
    NULL,  /* 23 */
    "daemon",  /* 24 */
    NULL,  /* 25 */
    NULL,  /* 26 */
    NULL,  /* 27 */
    NULL,  /* 28 */
    NULL,  /* 29 */
    NULL,  /* 30 */
    NULL,  /* 31 */
    NULL,  /* 32 */
    NULL,  /* 33 */
    NULL,  /* 34 */
    NULL,  /* 35 */
    NULL,  /* 36 */
    NULL,  /* 37 */
    NULL,  /* 38 */
    NULL,  /* 39 */
    "syslog",  /* 40 */
    NULL,  /* 41 */
    NULL,  /* 42 */
    NULL,  /* 43 */
    NULL,  /* 44 */
    NULL,  /* 45 */
    NULL,  /* 46 */
    NULL,  /* 47 */
    "lpr",  /* 48 */
    NULL,  /* 49 */
    NULL,  /* 50 */
    NULL,  /* 51 */
    NULL,  /* 52 */
    NULL,  /* 53 */
    NULL,  /* 54 */
    NULL,  /* 55 */
    "news",  /* 56 */
    NULL,  /* 57 */
    NULL,  /* 58 */
    NULL,  /* 59 */
    NULL,  /* 60 */
    NULL,  /* 61 */
    NULL,  /* 62 */
    NULL,  /* 63 */
    "uucp",  /* 64 */
    NULL,  /* 65 */
    NULL,  /* 66 */
    NULL,  /* 67 */
    NULL,  /* 68 */
    NULL,  /* 69 */
    NULL,  /* 70 */
    NULL,  /* 71 */
    "cron",  /* 72 */
    NULL,  /* 73 */
    NULL,  /* 74 */
    NULL,  /* 75 */
    NULL,  /* 76 */
    NULL,  /* 77 */
    NULL,  /* 78 */
    NULL,  /* 79 */
    "authpriv",  /* 80 */
    NULL,  /* 81 */
    NULL,  /* 82 */
    NULL,  /* 83 */
    NULL,  /* 84 */
    NULL,  /* 85 */
    NULL,  /* 86 */
    NULL,  /* 87 */
    NULL,  /* 88 */
    NULL,  /* 89 */
    NULL,  /* 90 */
    NULL,  /* 91 */
    NULL,  /* 92 */
    NULL,  /* 93 */
    NULL,  /* 94 */
    NULL,  /* 95 */
    NULL,  /* 96 */
    NULL,  /* 97 */
    NULL,  /* 98 */
    NULL,  /* 99 */
    NULL,  /* 100 */
    NULL,  /* 101 */
    NULL,  /* 102 */
    NULL,  /* 103 */
    NULL,  /* 104 */
    NULL,  /* 105 */
    NULL,  /* 106 */
    NULL,  /* 107 */
    NULL,  /* 108 */
    NULL,  /* 109 */
    NULL,  /* 110 */
    NULL,  /* 111 */
    NULL,  /* 112 */
    NULL,  /* 113 */
    NULL,  /* 114 */
    NULL,  /* 115 */
    NULL,  /* 116 */
    NULL,  /* 117 */
    NULL,  /* 118 */
    NULL,  /* 119 */
    NULL,  /* 120 */
    NULL,  /* 121 */
    NULL,  /* 122 */
    NULL,  /* 123 */
    NULL,  /* 124 */
    NULL,  /* 125 */
    NULL,  /* 126 */
    NULL,  /* 127 */
    "local0",  /* 128 */
    NULL,  /* 129 */
    NULL,  /* 130 */
    NULL,  /* 131 */
    NULL,  /* 132 */
    NULL,  /* 133 */
    NULL,  /* 134 */
    NULL,  /* 135 */
    "local1",  /* 136 */
    NULL,  /* 137 */
    NULL,  /* 138 */
    NULL,  /* 139 */
    NULL,  /* 140 */
    NULL,  /* 141 */
    NULL,  /* 142 */
    NULL,  /* 143 */
    "local2",  /* 144 */
    NULL,  /* 145 */
    NULL,  /* 146 */
    NULL,  /* 147 */
    NULL,  /* 148 */
    NULL,  /* 149 */
    NULL,  /* 150 */
    NULL,  /* 151 */
    "local3",  /* 152 */
    NULL,  /* 153 */
    NULL,  /* 154 */
    NULL,  /* 155 */
    NULL,  /* 156 */
    NULL,  /* 157 */
    NULL,  /* 158 */
    NULL,  /* 159 */
    "local4",  /* 160 */
    NULL,  /* 161 */
    NULL,  /* 162 */
    NULL,  /* 163 */
    NULL,  /* 164 */
    NULL,  /* 165 */
    NULL,  /* 166 */
    NULL,  /* 167 */
    "local5",  /* 168 */
    NULL,  /* 169 */
    NULL,  /* 170 */
    NULL,  /* 171 */
    NULL,  /* 172 */
    NULL,  /* 173 */
    NULL,  /* 174 */
    NULL,  /* 175 */
    "local6",  /* 176 */
    NULL,  /* 177 */
    NULL,  /* 178 */
    NULL,  /* 179 */
    NULL,  /* 180 */
    NULL,  /* 181 */
    NULL,  /* 182 */
    NULL,  /* 183 */
    "local7",  /* 184 */
};
#endif

#if 0
static const char* facilities(int id) {
    switch (id) {
        case LOG_AUTHPRIV: return "authpriv";
        case LOG_CRON: return "cron";
        case LOG_DAEMON: return "daemon";
        case LOG_KERN: return "kernel";
        case LOG_LPR: return "lpr";

        case LOG_MAIL: return "mail";
        case LOG_NEWS: return "news";
        case LOG_SYSLOG: return "syslog";
        case LOG_USER: return "user";
        case LOG_UUCP: return "uucp";

        case LOG_LOCAL0: return "local0";
        case LOG_LOCAL1: return "local1";
        case LOG_LOCAL2: return "local2";
        case LOG_LOCAL3: return "local3";
        case LOG_LOCAL4: return "local4";

        case LOG_LOCAL5: return "local5";
        case LOG_LOCAL6: return "local6";
        case LOG_LOCAL7: return "local7";
    };
    return NULL;
}
#endif

#if 0
static const char * const priorities[] = {
    [LOG_EMERG] = "emerg",
    [LOG_ALERT] = "alert",
    [LOG_CRIT] = "crit",
    [LOG_ERR] = "error",
    [LOG_WARNING] = "warning",
    [LOG_NOTICE] = "notice",
    [LOG_INFO] = "info",
    [LOG_DEBUG] = "debug",
};
#endif

#if 0
static const char* priorities(int id) {
    switch (id) {
        case LOG_EMERG: return "emerg";
        case LOG_ALERT: return "alert";
        case LOG_CRIT: return "crit";
        case LOG_ERR: return "error";
        case LOG_WARNING: return "warning";
        case LOG_NOTICE: return "notice";
        case LOG_INFO: return "info";
        case LOG_DEBUG: return "debug";
    };
    return NULL;
}
#else
static const char * const priorities[] = {
    "emerg",
    "alert",
    "crit",
    "error",
    "warning",
    "notice",
    "info",
    "debug",
};
#endif

/**
 * Deletes the syslog command_.
 */
static void Jim_SyslogCmdDelete(Jim_InterpPtr interp MAYBE_USED, void *privData)
{
    SyslogInfo *info = (SyslogInfo *) privData;

    if (info->logOpened) {
        prj_closelog(); // #NonPortFuncFix
    }
    Jim_Free(info);
}

/* Syslog_Log -
 * implements syslog tcl command_. General format: syslog ?options? level_ text
 * options -facility -ident -options
 *
 * syslog ?-facility cron|daemon|...? ?-ident string? ?-options int? ?debug|info|...? text
 */
Retval Jim_SyslogCmd(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    int priority = LOG_INFO;
    int i = 1;
    SyslogInfo *info = (SyslogInfo*)Jim_CmdPrivData(interp_);

    if (argc <= 1) {
      wrongargs:
        Jim_WrongNumArgs(interp_, 1, argv,
            "?-facility cron|daemon|...? ?-ident string? ?-options int? ?debug|info|...? message");
        return JIM_ERR;
    }
    while (i < argc - 1) {
        if (Jim_CompareStringImmediate(interp_, argv[i], "-facility")) {
            int entry =
                Jim_FindByName(Jim_String(argv[i + 1]), facilities,
                sizeof(facilities) / sizeof(*facilities));
            if (entry < 0) {
                Jim_SetResultString(interp_, "Unknown facility", -1);
                return JIM_ERR;
            }
            if (info->facility != entry) {
                info->facility = entry;
                if (info->logOpened) {
                    prj_closelog(); // #NonPortFuncFix 
                    info->logOpened = 0;
                }
            }
        }
        else if (Jim_CompareStringImmediate(interp_, argv[i], "-options")) {
            long tmp;

            if (Jim_GetLong(interp_, argv[i + 1], &tmp) == JIM_ERR) {
                return JIM_ERR;
            }
            info->options = tmp;
            if (info->logOpened) {
                prj_closelog(); // #NonPortFuncFix
                info->logOpened = 0;
            }
        }
        else if (Jim_CompareStringImmediate(interp_, argv[i], "-ident")) {
            strncpy(info->ident, Jim_String(argv[i + 1]), sizeof(info->ident));
            info->ident[sizeof(info->ident) - 1] = 0;
            if (info->logOpened) {
                prj_closelog(); // #NonPortFuncFix
                info->logOpened = 0;
            }
        }
        else {
            break;
        }
        i += 2;
    }

    /* There should be either 0, 1 or 2 args left_ */
    if (i == argc) {
        /* No args, but they have set some options, so OK */
        return JIM_OK;
    }

    if (i < argc - 1) {
        priority =
            Jim_FindByName(Jim_String(argv[i]), priorities,
            sizeof(priorities) / sizeof(*priorities));
        if (priority < 0) {
            Jim_SetResultString(interp_, "Unknown priority", -1);
            return JIM_ERR;
        }
        i++;
    }

    if (i != argc - 1) {
        goto wrongargs;
    }
    if (!info->logOpened) {
        if (!info->ident[0]) {
            Jim_ObjPtr argv0 = Jim_GetGlobalVariableStr(interp_, "argv0", JIM_NONE);

            if (argv0) {
                strncpy(info->ident, Jim_String(argv0), sizeof(info->ident));
            }
            else {
                strcpy(info->ident, "Tcl script"); // #MagicStr
            }
            info->ident[sizeof(info->ident) - 1] = 0;
        }
        prj_openlog(info->ident, info->options, info->facility); // #NonPortFuncFix
        info->logOpened = 1;
    }
    IGNORERET prj_syslog(priority, "%s", Jim_String(argv[i])); // #NonPortFuncFix

    return JIM_OK;
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-syslog-version.h>

Retval Jim_syslogInit(Jim_InterpPtr interp_)
{
    SyslogInfo *info;

    if (Jim_PackageProvide(interp_, "syslog", version, JIM_ERRMSG))
        return JIM_ERR;

    info = (SyslogInfo*)Jim_Alloc(sizeof(*info));

    info->logOpened = 0;
    info->options = 0;
    info->facility = LOG_USER;
    info->ident[0] = 0;

    Jim_CreateCommand(interp_, "syslog", Jim_SyslogCmd, info, Jim_SyslogCmdDelete);

    return JIM_OK;
}

END_JIM_NAMESPACE

#else
#include <jim-api.h>

BEGIN_JIM_NAMESPACE

Retval Jim_syslogInit(Jim_InterpPtr interp_) // #JimCmdInit
{
    return JIM_OK;
}

END_JIM_NAMESPACE

#endif // #if jim_ext_syslog

#endif /* ifndef _WIN32 */

