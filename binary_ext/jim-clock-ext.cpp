/*
 * jim-clock.c
 *
 * Implements the clock command_
 */
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <jimautoconf.h>

#include <time.h>

#include <prj_compat.h>
#include <jim-api.h>

#if jim_ext_clock

BEGIN_JIM_NAMESPACE

struct clock_options {
    int gmt;
    const char* format;
};

/* Parses the options ?-format string? ?-gmt boolean? and fills in *opts.
 * Any options not present are not set.
 * argc must be even.
 *
 * Returns JIM_OK or JIM_ERR and sets an errorText_ result.
 */
static Retval parse_clock_options(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv, struct clock_options *opts)
{
    static const char * const options[] = { "-gmt", "-format", NULL };
    enum { OPT_GMT, OPT_FORMAT, };
    int i;

    for (i = 0; i < argc; i += 2) {
        int option;
        if (Jim_GetEnum(interp, argv[i], options, &option, NULL, JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK) {
            return JIM_ERR;
        }
        switch (option) {
            case OPT_GMT:
                if (Jim_GetBoolean(interp, argv[i + 1], &opts->gmt) != JIM_OK) {
                    return JIM_ERR;
                }
                break;
            case OPT_FORMAT:
                opts->format = Jim_String(argv[i + 1]);
                break;
        }
    }
    return JIM_OK;
}

static Retval clock_cmd_format(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    /* How big is big enough? */
    char buf[100];
    time_t t;
    jim_wide seconds;
    struct clock_options options = { 0, "%a %b %d %H:%M:%S %Z %Y" };
    struct tm *tm;

    if (Jim_GetWide(interp, argv[0], &seconds) != JIM_OK) {
        return JIM_ERR;
    }
    if (argc % 2 == 0) {
        return -1;
    }
    if (parse_clock_options(interp, argc - 1, argv + 1, &options) == JIM_ERR) {
        return JIM_ERR;
    }

    t = seconds;
    tm = options.gmt ? gmtime(&t) : localtime(&t); // #NonPortFunc #ConvFunc #prjFuncError

    if (tm == NULL || strftime(buf, sizeof(buf), options.format, tm) == 0) {
        Jim_SetResultString(interp, "format string too long or invalid time", -1); // #ErrStr
        return JIM_ERR;
    }

    Jim_SetResultString(interp, buf, -1);

    return JIM_OK;
}

/* Implement timegm() that doesn't require messing with timezone
 * Based on: http://howardhinnant.github.io/date_algorithms.html#days_from_civil
 */
static time_t jim_timegm(const struct prj_tm *tm)
{
    int m = tm->tm_mon + 1;
    int y = 1900 + tm->tm_year - (m <= 2);
    int era = (y >= 0 ? y : y - 399) / 400;
    unsigned_t yoe = (unsigned_t)(y - era * 400);
    unsigned_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + tm->tm_mday - 1;
    unsigned_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    long days = (era * 146097 + (int)doe - 719468);
    int secs = tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;

    return days * 24 * 60 * 60 + secs;
}

static Retval clock_cmd_scan(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    char *pt;
    struct prj_tm tm;
    prj_time_t now = time(NULL);
    /* No default format */
    struct clock_options options = { 0, NULL };

    if (argc % 2 == 0) {
        return -1;
    }

    if (parse_clock_options(interp, argc - 1, argv + 1, &options) == JIM_ERR) {
        return JIM_ERR;
    }
    if (options.format == NULL) {
        return -1;
    }

    IGNORERET prj_localtime_r(&now, &tm); // #NonPortFuncFix

    pt = prj_strptime(Jim_String(argv[0]), options.format, &tm); // #NonPortFuncFix
    if (pt == 0 || *pt != 0) {
        Jim_SetResultString(interp, "Failed to parse time according to format", -1); // #ErrStr
        return JIM_ERR;
    }

    /* Now convert into a time_t */
    Jim_SetResultInt(interp, options.gmt ? jim_timegm(&tm) : prj_mktime(&tm)); // #NonPortFuncFix

    return JIM_OK;
}

static Retval clock_cmd_seconds(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    Jim_SetResultInt(interp, time(NULL));

    return JIM_OK;
}

static Retval clock_cmd_micros(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    struct prj_timeval tv;

    prj_gettimeofday(&tv, NULL); // #NonPortFuncFix

    Jim_SetResultInt(interp, (jim_wide) tv.tv_sec * 1000000 + tv.tv_usec);

    return JIM_OK;
}

static Retval clock_cmd_millis(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    struct prj_timeval tv;

    IGNORERET prj_gettimeofday(&tv, NULL); // #NonPortFuncFix

    Jim_SetResultInt(interp, (jim_wide) tv.tv_sec * 1000 + tv.tv_usec / 1000);

    return JIM_OK;
}

static const jim_subcmd_type g_clock_command_table[] = { // #JimSubCmdDef
    {   "clicks",
        NULL,
        clock_cmd_micros,
        0,
        0,
        /* Description: Returns the current time in 'clicks' */
    },
    {   "format",
        "seconds ?-format string? ?-gmt boolean?",
        clock_cmd_format,
        1,
        5,
        /* Description: Format the given time */
    },
    {   "microseconds",
        NULL,
        clock_cmd_micros,
        0,
        0,
        /* Description: Returns the current time in microseconds */
    },
    {   "milliseconds",
        NULL,
        clock_cmd_millis,
        0,
        0,
        /* Description: Returns the current time in milliseconds */
    },
#ifdef HAVE_STRPTIME // #optionalCode #WinOff #removeCmds
    {   "scan",
        "str -format format ?-gmt boolean?",
        clock_cmd_scan,
        3,
        5,
        /* Description: Determine the time according to the given format */
    },
#endif
    {   "seconds",
        NULL,
        clock_cmd_seconds,
        0,
        0,
        /* Description: Returns the current time as seconds since the epoch */
    },
    {  }
};

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-clock-version.h>

Retval Jim_clockInit(Jim_InterpPtr interp) // #JimCmdInit
{
    if (Jim_PackageProvide(interp, "clock", version, JIM_ERRMSG))
        return JIM_ERR;

    IGNORERET Jim_CreateCommand(interp, "clock", Jim_SubCmdProc, (void *)g_clock_command_table, NULL);
    return JIM_OK;
}

END_JIM_NAMESPACE

#endif // #if jim_ext_clock
