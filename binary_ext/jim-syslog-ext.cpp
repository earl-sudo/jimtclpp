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
    nullptr,  /* 1 */
    nullptr,  /* 2 */
    nullptr,  /* 3 */
    nullptr,  /* 4 */
    nullptr,  /* 5 */
    nullptr,  /* 6 */
    nullptr,  /* 7 */
    "user",  /* 8 */
    nullptr,  /* 9 */
    nullptr,  /* 10 */
    nullptr,  /* 11 */
    nullptr,  /* 12 */
    nullptr,  /* 13 */
    nullptr,  /* 14 */
    nullptr,  /* 15 */
    "mail",  /* 16 */
    nullptr,  /* 17 */
    nullptr,  /* 18 */
    nullptr,  /* 19 */
    nullptr,  /* 20 */
    nullptr,  /* 21 */
    nullptr,  /* 22 */
    nullptr,  /* 23 */
    "daemon",  /* 24 */
    nullptr,  /* 25 */
    nullptr,  /* 26 */
    nullptr,  /* 27 */
    nullptr,  /* 28 */
    nullptr,  /* 29 */
    nullptr,  /* 30 */
    nullptr,  /* 31 */
    nullptr,  /* 32 */
    nullptr,  /* 33 */
    nullptr,  /* 34 */
    nullptr,  /* 35 */
    nullptr,  /* 36 */
    nullptr,  /* 37 */
    nullptr,  /* 38 */
    nullptr,  /* 39 */
    "syslog",  /* 40 */
    nullptr,  /* 41 */
    nullptr,  /* 42 */
    nullptr,  /* 43 */
    nullptr,  /* 44 */
    nullptr,  /* 45 */
    nullptr,  /* 46 */
    nullptr,  /* 47 */
    "lpr",  /* 48 */
    nullptr,  /* 49 */
    nullptr,  /* 50 */
    nullptr,  /* 51 */
    nullptr,  /* 52 */
    nullptr,  /* 53 */
    nullptr,  /* 54 */
    nullptr,  /* 55 */
    "news",  /* 56 */
    nullptr,  /* 57 */
    nullptr,  /* 58 */
    nullptr,  /* 59 */
    nullptr,  /* 60 */
    nullptr,  /* 61 */
    nullptr,  /* 62 */
    nullptr,  /* 63 */
    "uucp",  /* 64 */
    nullptr,  /* 65 */
    nullptr,  /* 66 */
    nullptr,  /* 67 */
    nullptr,  /* 68 */
    nullptr,  /* 69 */
    nullptr,  /* 70 */
    nullptr,  /* 71 */
    "cron",  /* 72 */
    nullptr,  /* 73 */
    nullptr,  /* 74 */
    nullptr,  /* 75 */
    nullptr,  /* 76 */
    nullptr,  /* 77 */
    nullptr,  /* 78 */
    nullptr,  /* 79 */
    "authpriv",  /* 80 */
    nullptr,  /* 81 */
    nullptr,  /* 82 */
    nullptr,  /* 83 */
    nullptr,  /* 84 */
    nullptr,  /* 85 */
    nullptr,  /* 86 */
    nullptr,  /* 87 */
    nullptr,  /* 88 */
    nullptr,  /* 89 */
    nullptr,  /* 90 */
    nullptr,  /* 91 */
    nullptr,  /* 92 */
    nullptr,  /* 93 */
    nullptr,  /* 94 */
    nullptr,  /* 95 */
    nullptr,  /* 96 */
    nullptr,  /* 97 */
    nullptr,  /* 98 */
    nullptr,  /* 99 */
    nullptr,  /* 100 */
    nullptr,  /* 101 */
    nullptr,  /* 102 */
    nullptr,  /* 103 */
    nullptr,  /* 104 */
    nullptr,  /* 105 */
    nullptr,  /* 106 */
    nullptr,  /* 107 */
    nullptr,  /* 108 */
    nullptr,  /* 109 */
    nullptr,  /* 110 */
    nullptr,  /* 111 */
    nullptr,  /* 112 */
    nullptr,  /* 113 */
    nullptr,  /* 114 */
    nullptr,  /* 115 */
    nullptr,  /* 116 */
    nullptr,  /* 117 */
    nullptr,  /* 118 */
    nullptr,  /* 119 */
    nullptr,  /* 120 */
    nullptr,  /* 121 */
    nullptr,  /* 122 */
    nullptr,  /* 123 */
    nullptr,  /* 124 */
    nullptr,  /* 125 */
    nullptr,  /* 126 */
    nullptr,  /* 127 */
    "local0",  /* 128 */
    nullptr,  /* 129 */
    nullptr,  /* 130 */
    nullptr,  /* 131 */
    nullptr,  /* 132 */
    nullptr,  /* 133 */
    nullptr,  /* 134 */
    nullptr,  /* 135 */
    "local1",  /* 136 */
    nullptr,  /* 137 */
    nullptr,  /* 138 */
    nullptr,  /* 139 */
    nullptr,  /* 140 */
    nullptr,  /* 141 */
    nullptr,  /* 142 */
    nullptr,  /* 143 */
    "local2",  /* 144 */
    nullptr,  /* 145 */
    nullptr,  /* 146 */
    nullptr,  /* 147 */
    nullptr,  /* 148 */
    nullptr,  /* 149 */
    nullptr,  /* 150 */
    nullptr,  /* 151 */
    "local3",  /* 152 */
    nullptr,  /* 153 */
    nullptr,  /* 154 */
    nullptr,  /* 155 */
    nullptr,  /* 156 */
    nullptr,  /* 157 */
    nullptr,  /* 158 */
    nullptr,  /* 159 */
    "local4",  /* 160 */
    nullptr,  /* 161 */
    nullptr,  /* 162 */
    nullptr,  /* 163 */
    nullptr,  /* 164 */
    nullptr,  /* 165 */
    nullptr,  /* 166 */
    nullptr,  /* 167 */
    "local5",  /* 168 */
    nullptr,  /* 169 */
    nullptr,  /* 170 */
    nullptr,  /* 171 */
    nullptr,  /* 172 */
    nullptr,  /* 173 */
    nullptr,  /* 174 */
    nullptr,  /* 175 */
    "local6",  /* 176 */
    nullptr,  /* 177 */
    nullptr,  /* 178 */
    nullptr,  /* 179 */
    nullptr,  /* 180 */
    nullptr,  /* 181 */
    nullptr,  /* 182 */
    nullptr,  /* 183 */
    "local7",  /* 184 */
};
#endif

#if 0
static const char* facilities(int id_) {
    switch (id_) {
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
    return nullptr;
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
static const char* priorities(int id_) {
    switch (id_) {
        case LOG_EMERG: return "emerg";
        case LOG_ALERT: return "alert";
        case LOG_CRIT: return "crit";
        case LOG_ERR: return "error";
        case LOG_WARNING: return "warning";
        case LOG_NOTICE: return "notice";
        case LOG_INFO: return "info";
        case LOG_DEBUG: return "debug";
    };
    return nullptr;
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
        return JRET(JIM_ERR);
    }
    while (i < argc - 1) {
        if (Jim_CompareStringImmediate(interp_, argv[i], "-facility")) {
            int entry =
                Jim_FindByName(Jim_String(argv[i + 1]), facilities,
                sizeof(facilities) / sizeof(*facilities));
            if (entry < 0) {
                Jim_SetResultString(interp_, "Unknown facility", -1);
                return JRET(JIM_ERR);
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

            if (Jim_GetLong(interp_, argv[i + 1], &tmp) == JRET(JIM_ERR)) {
                return JRET(JIM_ERR);
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

    /* There should be either 0, 1 or 2 args_ left_ */
    if (i == argc) {
        /* No args_, but they have set some options, so OK */
        return JRET(JIM_OK);
    }

    if (i < argc - 1) {
        priority =
            Jim_FindByName(Jim_String(argv[i]), priorities,
            sizeof(priorities) / sizeof(*priorities));
        if (priority < 0) {
            Jim_SetResultString(interp_, "Unknown priority", -1);
            return JRET(JIM_ERR);
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
    prj_syslog(priority, "%s", Jim_String(argv[i])); // #NonPortFuncFix

    return JRET(JIM_OK);
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-syslog-version.h>

Retval Jim_syslogInit(Jim_InterpPtr interp_)
{
    SyslogInfo *info;

    if (Jim_PackageProvide(interp_, "syslog", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

    info = (SyslogInfo*)Jim_Alloc(sizeof(*info));

    info->logOpened = 0;
    info->options = 0;
    info->facility = LOG_USER;
    info->ident[0] = 0;

    Retval ret = JIM_ERR;

    ret = Jim_CreateCommand(interp_, "syslog", Jim_SyslogCmd, info, Jim_SyslogCmdDelete);
    if (ret != JIM_OK) return ret;


    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#else
#include <jim-api.h>

BEGIN_JIM_NAMESPACE

JIM_EXPORT Retval Jim_syslogInit(Jim_InterpPtr interp_) // #JimCmdInit
{
    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // #if jim_ext_syslog

#endif /* ifndef _WIN32 */

