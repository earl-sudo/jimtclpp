/*
 * jim-signal.c
 *
 */

#include <jimautoconf.h>

#if jim_ext_signal

#ifndef _WIN32 

#include <signal.h> // #NonPortHeader
#include <ctype.h>

#ifdef HAVE_UNISTD_H
    #include <unistd.h> // #NonPortHeader
#endif
#include <jim.h> // #TODO Replace with <jim-api.h>
#include <jim-subcmd.h>
#include <jim-signal.h>
#include <prj_compat.h>

BEGIN_JIM_NAMESPACE

enum { MAX_SIGNALS_WIDE = (sizeof(jim_wide) * 8) };
#if defined(NSIG)
    enum { MAX_SIGNALS = (int)((NSIG < MAX_SIGNALS_WIDE) ? NSIG : MAX_SIGNALS_WIDE) };
#else
    enum { MAX_SIGNALS = (int)MAX_SIGNALS_WIDE };
#endif

static jim_wide *sigloc;
static jim_wide sigsblocked;
static struct sigaction *sa_old;
static struct {
    int status;
    const char *name_;
} siginfo[MAX_SIGNALS];

/* Make sure to do this as a wide, not int */
#define sig_to_bit(SIG) ((jim_wide)1 << (SIG))

static void signal_handler(int sig)
{
    /* We just remember which signals occurred. Jim_Eval() will
     * notice this as soon as it can and throw an errorText_
     */
    *sigloc |= sig_to_bit(sig);
}

static void signal_ignorer(int sig)
{
    /* We just remember which signals occurred */
    sigsblocked |= sig_to_bit(sig);
}

static void signal_init_names(void)
{
#define SET_SIG_NAME(SIG) siginfo[SIG].name_ = #SIG

    SET_SIG_NAME(SIGABRT);
    SET_SIG_NAME(SIGALRM);
    SET_SIG_NAME(SIGBUS);
    SET_SIG_NAME(SIGCHLD);
    SET_SIG_NAME(SIGCONT);
    SET_SIG_NAME(SIGFPE);
    SET_SIG_NAME(SIGHUP);
    SET_SIG_NAME(SIGILL);
    SET_SIG_NAME(SIGINT);
#ifdef SIGIO
    SET_SIG_NAME(SIGIO);
#endif
    SET_SIG_NAME(SIGKILL);
    SET_SIG_NAME(SIGPIPE);
    SET_SIG_NAME(SIGPROF);
    SET_SIG_NAME(SIGQUIT);
    SET_SIG_NAME(SIGSEGV);
    SET_SIG_NAME(SIGSTOP);
    SET_SIG_NAME(SIGSYS);
    SET_SIG_NAME(SIGTERM);
    SET_SIG_NAME(SIGTRAP);
    SET_SIG_NAME(SIGTSTP);
    SET_SIG_NAME(SIGTTIN);
    SET_SIG_NAME(SIGTTOU);
    SET_SIG_NAME(SIGURG);
    SET_SIG_NAME(SIGUSR1);
    SET_SIG_NAME(SIGUSR2);
    SET_SIG_NAME(SIGVTALRM);
    SET_SIG_NAME(SIGWINCH);
    SET_SIG_NAME(SIGXCPU);
    SET_SIG_NAME(SIGXFSZ);
#ifdef SIGPWR
    SET_SIG_NAME(SIGPWR);
#endif
#ifdef SIGCLD
    SET_SIG_NAME(SIGCLD);
#endif
#ifdef SIGEMT
    SET_SIG_NAME(SIGEMT);
#endif
#ifdef SIGLOST
    SET_SIG_NAME(SIGLOST);
#endif
#ifdef SIGPOLL
    SET_SIG_NAME(SIGPOLL);
#endif
#ifdef SIGINFO
    SET_SIG_NAME(SIGINFO);
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SignalId --
 *
 *      Return a textual identifier for a signal number.
 *
 * Results:
 *      This procedure returns a machine-readable textual identifier
 *      that corresponds to sig.  The identifier is the same as the
 *      #define name_ in signal.h.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */
const char *Jim_SignalId(int sig)
{
    if (sig >=0 && sig < MAX_SIGNALS) {
        if (siginfo[sig].name_) {
            return siginfo[sig].name_;
        }
    }
    return "unknown signal";
}

/**
 * Given the name_ of a signal, returns the signal value if found,
 * or returns -1 (and sets an errorText_) if not found.
 * We accept -SIGINT, SIGINT, INT or any lowercase version or a number,
 * either positive or negative.
 */
static int find_signal_by_name(Jim_InterpPtr interp_, const char *name_)
{
    int i;
    const char *pt = name_;

    /* Remove optional - and SIG from the front of the name_ */
    if (*pt == '-') {
        pt++;
    }
    if (strncasecmp(name_, "sig", 3) == 0) {
        pt += 3;
    }
    if (isdigit(UCHAR(pt[0]))) {
        i = atoi(pt);
        if (i > 0 && i < MAX_SIGNALS) {
            return i;
        }
    }
    else {
        for (i = 1; i < MAX_SIGNALS; i++) {
            /* Jim_SignalId() returns names such as SIGINT, and
             * returns "unknown signal" if unknown, so this will work
             */
            if (prj_strcasecmp(Jim_SignalId(i) + 3, pt) == 0) { // #NonPortFuncFix
                return i;
            }
        }
    }
    Jim_SetResultFormatted(interp_, "unknown signal %s", name_);

    return -1;
}

enum {
    SIGNAL_ACTION_HANDLE = 1,
    SIGNAL_ACTION_IGNORE = -1,
    SIGNAL_ACTION_DEFAULT = 0
};

static Retval do_signal_cmd(Jim_InterpPtr interp_, int action, int argc, Jim_ObjConstArray argv)
{
    struct sigaction sa;
    int i;

    if (argc == 0) {
        Jim_SetResult(interp_, Jim_NewListObj(interp_, NULL, 0));
        for (i = 1; i < MAX_SIGNALS; i++) {
            if (siginfo[i].status == action) {
                /* Add signal name_ to the list  */
                Jim_ListAppendElement(interp_, Jim_GetResult(interp_),
                    Jim_NewStringObj(interp_, Jim_SignalId(i), -1));
            }
        }
        return JIM_OK;
    }

    /* Catch all the signals we care about */
    if (action != SIGNAL_ACTION_DEFAULT) {
        memset(&sa, 0, sizeof(sa));
        if (action == SIGNAL_ACTION_HANDLE) {
            sa.sa_handler = signal_handler;
        }
        else {
            sa.sa_handler = signal_ignorer;
        }
    }

    /* Iterate through the provided signals */
    for (i = 0; i < argc; i++) {
        int sig = find_signal_by_name(interp_, Jim_String(argv[i]));

        if (sig < 0) {
            return JIM_ERR;
        }
        if (action != siginfo[sig].status) {
            /* Need to change the action for this signal */
            switch (action) {
                case SIGNAL_ACTION_HANDLE:
                case SIGNAL_ACTION_IGNORE:
                    if (siginfo[sig].status == SIGNAL_ACTION_DEFAULT) {
                        if (!sa_old) {
                            /* Allocate the structure the first time through */
                            sa_old = (struct sigaction*)Jim_Alloc(sizeof(*sa_old) * MAX_SIGNALS);
                        }
                        IGNORERET prj_sigaction(sig, &sa, &sa_old[sig]); // #NonPortFuncFix #ConvFunc
                    }
                    else {
                        IGNORERET prj_sigaction(sig, &sa, 0); // #NonPortFuncFix #ConvFunc
                    }
                    break;

                case SIGNAL_ACTION_DEFAULT:
                    /* Restore old handler */
                    if (sa_old) {
                        IGNORERET prj_sigaction(sig, &sa_old[sig], 0); // #NonPortFuncFix #ConvFunc
                    }
            }
            siginfo[sig].status = action;
        }
    }

    return JIM_OK;
}

static Retval signal_cmd_handle(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCMd #PosixCmd
{
    return do_signal_cmd(interp_, SIGNAL_ACTION_HANDLE, argc, argv);
}

static Retval signal_cmd_ignore(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    return do_signal_cmd(interp_, SIGNAL_ACTION_IGNORE, argc, argv);
}

static Retval signal_cmd_default(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    return do_signal_cmd(interp_, SIGNAL_ACTION_DEFAULT, argc, argv);
}

static Retval signal_set_sigmask_result(Jim_InterpPtr interp_, jim_wide sigmask)
{
    int i;
    Jim_ObjPtr listObj = Jim_NewListObj(interp_, NULL, 0);

    for (i = 0; i < MAX_SIGNALS; i++) {
        if (sigmask & sig_to_bit(i)) {
            Jim_ListAppendElement(interp_, listObj, Jim_NewStringObj(interp_, Jim_SignalId(i), -1));
        }
    }
    Jim_SetResult(interp_, listObj);
    return JIM_OK;
}

static Retval signal_cmd_check(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv)
{
    int clear = 0;
    jim_wide mask = 0;
    jim_wide blocked;

    if (argc > 0 && Jim_CompareStringImmediate(interp_, argv[0], "-clear")) {
        clear++;
    }
    if (argc > clear) {
        int i;

        /* Signals specified */
        for (i = clear; i < argc; i++) {
            int sig = find_signal_by_name(interp_, Jim_String(argv[i]));

            if (sig < 0 || sig >= MAX_SIGNALS) {
                return -1;
            }
            mask |= sig_to_bit(sig);
        }
    }
    else {
        /* No signals specified, so check/clear all */
        mask = ~mask;
    }

    if ((sigsblocked & mask) == 0) {
        /* No matching signals, so empty result and nothing to do */
        return JIM_OK;
    }
    /* Be careful we don't have a race condition where signals are cleared but not returned */
    blocked = sigsblocked & mask;
    if (clear) {
        sigsblocked &= ~blocked;
    }
    /* Set the result */
    signal_set_sigmask_result(interp_, blocked);
    return JIM_OK;
}

static Retval signal_cmd_throw(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    int sig = SIGINT;

    if (argc == 1) {
        if ((sig = find_signal_by_name(interp_, Jim_String(argv[0]))) < 0) {
            return JIM_ERR;
        }
    }

    /* If the signal is ignored (blocked) ... */
    if (siginfo[sig].status == SIGNAL_ACTION_IGNORE) {
        sigsblocked |= sig_to_bit(sig);
        return JIM_OK;
    }

    /* Just set the signal */
    interp_->orSigmask(sig_to_bit(sig));

    /* Set the canonical name_ of the signal as the result */
    Jim_SetResultString(interp_, Jim_SignalId(sig), -1);

    /* And simply say we caught the signal */
    return JIM_SIGNAL;
}

/*
 *-----------------------------------------------------------------------------
 *
 * Jim_SignalCmd --
 *     Implements the TCL signal command_:
 *         signal handle|ignore|default|throw ?signals ...?
 *         signal throw signal
 *
 *     Specifies which signals are handled by Tcl code.
 *     If the one of the given signals is caught, it causes a JIM_SIGNAL
 *     exception to be thrown which can be caught by catch.
 *
 *     Use 'signal ignore' to ignore the signal(s)
 *     Use 'signal default' to go back to the default behaviour
 *     Use 'signal throw signal' to raise the given signal
 *
 *     If no arguments are given, returns the list of signals which are being handled
 *
 * Results:
 *      Standard TCL results.
 *
 *-----------------------------------------------------------------------------
 */
static const jim_subcmd_type signal_command_table[] = {
    {   "handle",
        "?signals ...?",
        signal_cmd_handle,
        0,
        -1,
        /* Description: Lists handled signals, or adds to handled signals */
    },
    {   "ignore",
        "?signals ...?",
        signal_cmd_ignore,
        0,
        -1,
        /* Description: Lists ignored signals, or adds to ignored signals */
    },
    {   "default",
        "?signals ...?",
        signal_cmd_default,
        0,
        -1,
        /* Description: Lists defaulted signals, or adds to defaulted signals */
    },
    {   "check",
        "?-clear? ?signals ...?",
        signal_cmd_check,
        0,
        -1,
        /* Description: Returns ignored signals which have occurred, and optionally clearing them */
    },
    {   "throw",
        "?signal?",
        signal_cmd_throw,
        0,
        1,
        /* Description: Raises the given signal (default SIGINT) */
    },
    {  }
};

/**
 * Restore default signal handling.
 */
static void JimSignalCmdDelete(Jim_InterpPtr interp MAYBE_USED, void *privData MAYBE_USED)
{
    int i;
    if (sa_old) {
        for (i = 1; i < MAX_SIGNALS; i++) {
            if (siginfo[i].status != SIGNAL_ACTION_DEFAULT) {
                IGNORERET prj_sigaction(i, &sa_old[i], 0); // #NonPortFuncFix #ConvFunc
                siginfo[i].status = SIGNAL_ACTION_DEFAULT;
            }
        }
    }
    Jim_Free(sa_old);
    sa_old = NULL;
    sigloc = NULL;
}

static Retval Jim_AlarmCmd(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    Retval ret;

    if (argc != 2) {
        Jim_WrongNumArgs(interp_, 1, argv, "seconds");
        return JIM_ERR;
    }
    else {
#ifdef HAVE_UALARM
        double t;

        ret = Jim_GetDouble(interp_, argv[1], &t);
        if (ret == JIM_OK) {
            if (t < 1) {
                IGNORERET prj_ualarm(t * 1e6, 0); // #NonPortFuncFix
            }
            else {
                IGNORERET prj_alarm(t); // #NonPortFuncFix
            }
        }
#else
        long t;

        ret = Jim_GetLong(interp_, argv[1], &t);
        if (ret == JIM_OK) {
            prj_alarm(t); // #NonPortFuncFix
        }
#endif
    }

    return ret;
}

static Retval Jim_SleepCmd(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    Retval ret;

    if (argc != 2) {
        Jim_WrongNumArgs(interp_, 1, argv, "seconds");
        return JIM_ERR;
    }
    else {
        double t;

        ret = Jim_GetDouble(interp_, argv[1], &t);
        if (ret == JIM_OK) {
            if (prj_funcDef(prj_usleep)) { // #optionalCode
                IGNORERET prj_usleep((int) ((t - (int) t) * 1e6)); // #NonPortFuncFix 
            }
            IGNORERET prj_sleep(t); // #NonPortFuncFix
        }
    }

    return ret;
}

static Retval Jim_KillCmd(Jim_InterpPtr interp_, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    int sig;
    long pid;
    Jim_ObjPtr pidObj;
    const char *signame;

    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp_, 1, argv, "?SIG|-0? pid");
        return JIM_ERR;
    }

    if (argc == 2) {
        sig = SIGTERM;
        pidObj = argv[1];
    }
    else {
        signame = Jim_String(argv[1]);
        pidObj = argv[2];

        /* Special 'kill -0 pid' to determine if a pid exists */
        if (strcmp(signame, "-0") == 0 || strcmp(signame, "0") == 0) {
            sig = 0;
        }
        else {
            sig = find_signal_by_name(interp_, signame);
            if (sig < 0) {
                return JIM_ERR;
            }
        }
    }

    if (Jim_GetLong(interp_, pidObj, &pid) != JIM_OK) {
        return JIM_ERR;
    }

    if (prj_kill(pid, sig) == 0) { // #NonPortFuncFix
        return JIM_OK;
    }

    Jim_SetResultString(interp_, "kill: Failed to deliver signal", -1);
    return JIM_ERR;
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-signal-version.h>

Retval Jim_signalInit(Jim_InterpPtr interp_)
{
    if (Jim_PackageProvide(interp_, "signal", version, JIM_ERRMSG))
        return JIM_ERR;

    IGNORERET Jim_CreateCommand(interp_, "alarm", Jim_AlarmCmd, 0, 0);
    IGNORERET Jim_CreateCommand(interp_, "kill", Jim_KillCmd, 0, 0);
    /* Sleep is slightly dubious here */
    IGNORERET Jim_CreateCommand(interp_, "sleep", Jim_SleepCmd, 0, 0);

    /* Teach the jim core how to set a result from a sigmask */
    interp_->signal_set_result_ = signal_set_sigmask_result;

    /* Currently only the top level_ interp_ supports signals */
    if (!sigloc) {
        signal_init_names();

        /* Make sure we know where to store the signals which occur */
        sigloc = interp_->getSigmaskPtr();

        IGNORERET Jim_CreateCommand(interp_, "signal", Jim_SubCmdProc, (void *)signal_command_table, JimSignalCmdDelete);
    }

    return JIM_OK;
}

#else
#include <jim-api.h>

BEGIN_JIM_NAMESPACE

Retval Jim_signalInit(Jim_InterpPtr interp_) // #JimCmdInit
{
    return JIM_OK;
}
#endif /* ifndef _WIN32 */

END_JIM_NAMESPACE

#endif // #if jim_ext_signal
