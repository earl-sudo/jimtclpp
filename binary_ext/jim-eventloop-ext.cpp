/* Jim - A small embeddable Tcl interpreter
 *
 * Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
 * Copyright 2005 Clemens Hintze <c.hintze@gmx.net>
 * Copyright 2005 patthoyts - Pat Thoyts <patthoyts@users.sf.net>
 * Copyright 2008 oharboe - Øyvind Harboe - oyvind.harboe@zylin.com
 * Copyright 2008 Andrew Lunn <andrew@lunn.ch>
 * Copyright 2008 Duane Ellis <openocd@duaneellis.com>
 * Copyright 2008 Uwe Klein <uklein@klein-messgeraete.de>
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
 **/

#include <jimautoconf.h>
#include <prj_compat.h>

#include <jim.h>
#include <jim-eventloop.h>

/* --- */
BEGIN_JIM_NAMESPACE

/* File event structure */
struct Jim_FileEvent
{
    int fd;
    int mask;                   /* one of JIM_EVENT_(READABLE|WRITABLE|EXCEPTION) */
    Jim_FileProc *fileProc;
    Jim_EventFinalizerProc *finalizerProc;
    void *clientData;
    struct Jim_FileEvent *next;
};

/* Time event structure */
struct Jim_TimeEvent
{
    jim_wide id;                /* time event identifier. */
    jim_wide initialus;         /* initial relative timer value in microseconds */
    jim_wide when;              /* microseconds */
    Jim_TimeProc *timeProc;
    Jim_EventFinalizerProc *finalizerProc;
    void *clientData;
    struct Jim_TimeEvent *next;
};

/* Per-interp structure containing the state of the event loop */
struct Jim_EventLoop
{
    Jim_FileEvent *fileEventHead;
    Jim_TimeEvent *timeEventHead;
    jim_wide timeEventNextId;   /* highest event id created, starting at 1 */
    time_t timeBase;
    int suppress_bgerror; /* bgerror returned break, so don't call it again */
};

static void JimAfterTimeHandler(Jim_InterpPtr interp, void *clientData);
static void JimAfterTimeEventFinalizer(Jim_InterpPtr interp, void *clientData);

int Jim_EvalObjBackground(Jim_InterpPtr interp, Jim_Obj *scriptObjPtr)
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");
    Jim_CallFrame *savedFramePtr;
    int retval;

    savedFramePtr = interp->framePtr();
    interp->framePtr(Jim_TopCallFrame(interp));
    retval = Jim_EvalObj(interp, scriptObjPtr);
    interp->framePtr(savedFramePtr);
    /* Try to report the error (if any) via the bgerror proc */
    if (retval != JIM_OK && retval != JIM_RETURN && !eventLoop->suppress_bgerror) {
        Jim_Obj *objv[2];
        int rc = JIM_ERR;

        objv[0] = Jim_NewStringObj(interp, "bgerror", -1);
        objv[1] = Jim_GetResult(interp);
        Jim_IncrRefCount(objv[0]);
        Jim_IncrRefCount(objv[1]);
        if (Jim_GetCommand(interp, objv[0], JIM_NONE) == NULL || (rc = Jim_EvalObjVector(interp, 2, objv)) != JIM_OK) {
            if (rc == JIM_BREAK) {
                /* No more bgerror calls */
                eventLoop->suppress_bgerror++;
            }
            else {
                /* Report the error to stderr. */
                Jim_MakeErrorMessage(interp);
                fprintf(stderr, "%s\n", Jim_String(Jim_GetResult(interp)));
                /* And reset the result */
                Jim_SetResultString(interp, "", -1);
            }
        }
        Jim_DecrRefCount(interp, objv[0]);
        Jim_DecrRefCount(interp, objv[1]);
    }
    return retval;
}


void Jim_CreateFileHandler(Jim_InterpPtr interp, int fd, int mask,
    Jim_FileProc * proc, void *clientData, Jim_EventFinalizerProc * finalizerProc)
{
    Jim_FileEvent *fe;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");

    fe = Jim_TAlloc<Jim_FileEvent>(); // #AllocF 
    fe->fd = fd;
    fe->mask = mask;
    fe->fileProc = proc;
    fe->finalizerProc = finalizerProc;
    fe->clientData = clientData;
    fe->next = eventLoop->fileEventHead;
    eventLoop->fileEventHead = fe;
}

/**
 * Removes all event handlers for 'handle' that match 'mask'.
 */
void Jim_DeleteFileHandler(Jim_InterpPtr interp, int fd, int mask)
{
    Jim_FileEvent *fe, *next, *prev = NULL;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");

    for (fe = eventLoop->fileEventHead; fe; fe = next) {
        next = fe->next;
        if (fe->fd == fd && (fe->mask & mask)) {
            /* Remove this entry from the list */
            if (prev == NULL)
                eventLoop->fileEventHead = next;
            else
                prev->next = next;
            if (fe->finalizerProc)
                fe->finalizerProc(interp, fe->clientData);
            Jim_TFree<Jim_FileEvent>(fe); // #FreeF 
            continue;
        }
        prev = fe;
    }
}

#ifdef CLOCK_MONOTONIC_RAW
int g_CLOCK_MONOTONIC_RAW_VAL = 1;
#else
int g_CLOCK_MONOTONIC_RAW_VAL = 0;
#define CLOCK_MONOTONIC_RAW 0
#endif

/**
 * Returns the time since interp creation in microseconds.
 */
static jim_wide JimGetTimeUsec(Jim_EventLoop *eventLoop)
{
    long_long now;
    struct prj_timeval tv;

    struct prj_timespec ts;

    if (g_CLOCK_MONOTONIC_RAW_VAL && prj_clock_gettime &&  // #NonPortFuncFix
        prj_clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0) {
        now = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
    }
    else
    {
        prj_gettimeofday(&tv, NULL); // #NonPortFuncFix

        now = tv.tv_sec * 1000000LL + tv.tv_usec;
    }

    return now - eventLoop->timeBase;
}

jim_wide Jim_CreateTimeHandler(Jim_InterpPtr interp, jim_wide us,
    Jim_TimeProc * proc, void *clientData, Jim_EventFinalizerProc * finalizerProc)
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");
    jim_wide id = ++eventLoop->timeEventNextId;
    Jim_TimeEvent *te, *e, *prev;

    te = Jim_TAlloc<Jim_TimeEvent>(); // #AllocF 
    te->id = id;
    te->initialus = us;
    te->when = JimGetTimeUsec(eventLoop) + us;
    te->timeProc = proc;
    te->finalizerProc = finalizerProc;
    te->clientData = clientData;

    /* Add to the appropriate place in the list */
    prev = NULL;
    for (e = eventLoop->timeEventHead; e; e = e->next) {
        if (te->when < e->when) {
            break;
        }
        prev = e;
    }
    if (prev) {
        te->next = prev->next;
        prev->next = te;
    }
    else {
        te->next = eventLoop->timeEventHead;
        eventLoop->timeEventHead = te;
    }

    return id;
}

static jim_wide JimParseAfterId(Jim_Obj *idObj)
{
    const char *tok = Jim_String(idObj);
    jim_wide id;

    if (strncmp(tok, "after#", 6) == 0 && Jim_StringToWide(tok + 6, &id, 10) == JIM_OK) {
        /* Got an event by id */
        return id;
    }
    return -1;
}

static jim_wide JimFindAfterByScript(Jim_EventLoop *eventLoop, Jim_Obj *scriptObj)
{
    Jim_TimeEvent *te;

    for (te = eventLoop->timeEventHead; te; te = te->next) {
        /* Is this an 'after' event? */
        if (te->timeProc == JimAfterTimeHandler) {
            if (Jim_StringEqObj(scriptObj, (Jim_Obj*)te->clientData)) {
                return te->id;
            }
        }
    }
    return -1;                  /* NO event with the specified ID found */
}

static Jim_TimeEvent *JimFindTimeHandlerById(Jim_EventLoop *eventLoop, jim_wide id)
{
    Jim_TimeEvent *te;

    for (te = eventLoop->timeEventHead; te; te = te->next) {
        if (te->id == id) {
            return te;
        }
    }
    return NULL;
}

static Jim_TimeEvent *Jim_RemoveTimeHandler(Jim_EventLoop *eventLoop, jim_wide id)
{
    Jim_TimeEvent *te, *prev = NULL;

    for (te = eventLoop->timeEventHead; te; te = te->next) {
        if (te->id == id) {
            if (prev == NULL)
                eventLoop->timeEventHead = te->next;
            else
                prev->next = te->next;
            return te;
        }
        prev = te;
    }
    return NULL;
}

static void Jim_FreeTimeHandler(Jim_InterpPtr interp, Jim_TimeEvent *te)
{
    if (te->finalizerProc)
        te->finalizerProc(interp, te->clientData);
    Jim_TFree<Jim_TimeEvent>(te); // #FreeF 
}

jim_wide Jim_DeleteTimeHandler(Jim_InterpPtr interp, jim_wide id)
{
    Jim_TimeEvent *te;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");

    if (id > eventLoop->timeEventNextId) {
        return -2;              /* wrong event ID */
    }

    te = Jim_RemoveTimeHandler(eventLoop, id);
    if (te) {
        jim_wide remain;

        remain = te->when - JimGetTimeUsec(eventLoop);
        remain = (remain < 0) ? 0 : remain;

        Jim_FreeTimeHandler(interp, te);
        return remain;
    }
    return -1;                  /* NO event with the specified ID found */
}

/* --- POSIX version of Jim_ProcessEvents, for now the only available --- */

/* Process every pending time event, then every pending file event
 * (that may be registered by time event callbacks just processed).
 * The behaviour depends upon the setting of flags:
 *
 * If flags is 0, the function does nothing and returns.
 * if flags has JIM_ALL_EVENTS set, all event types are processed.
 * if flags has JIM_FILE_EVENTS set, file events are processed.
 * if flags has JIM_TIME_EVENTS set, time events are processed.
 * if flags has JIM_DONT_WAIT set, the function returns as soon as all
 * the events that are possible to process without waiting are processed.
 *
 * Returns the number of events processed or -1 if
 * there are no matching handlers, or -2 on error.
 */
int Jim_ProcessEvents(Jim_InterpPtr interp, int flags)
{
    jim_wide sleep_us = -1;
    int processed = 0;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");
    Jim_FileEvent *fe = eventLoop->fileEventHead;
    Jim_TimeEvent *te;
    jim_wide maxId;

    if ((flags & JIM_FILE_EVENTS) == 0 || fe == NULL) {
        /* No file events */
        if ((flags & JIM_TIME_EVENTS) == 0 || eventLoop->timeEventHead == NULL) {
            /* No time events */
            return -1;
        }
    }

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in order to sleep until the next time event is ready
     * to fire. */

    if (flags & JIM_DONT_WAIT) {
        /* Wait no time */
        sleep_us = 0;
    }
    else if (flags & JIM_TIME_EVENTS) {
        /* The nearest timer is always at the head of the list */
        if (eventLoop->timeEventHead) {
            Jim_TimeEvent *shortest = eventLoop->timeEventHead;

            /* Calculate the time missing for the nearest
             * timer to fire. */
            sleep_us = shortest->when - JimGetTimeUsec(eventLoop);
            if (sleep_us < 0) {
                sleep_us = 0;
            }
        }
        else {
            /* Wait forever */
            sleep_us = -1;
        }
    }

#ifdef HAVE_SELECT // #optionalCode #WinOff
    if (flags & JIM_FILE_EVENTS) {
        int retval;
        struct timeval tv, *tvp = NULL;
        fd_set rfds, wfds, efds;
        int maxfd = -1;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&efds);

        /* Check file events */
        while (fe != NULL) {
            if (fe->mask & JIM_EVENT_READABLE)
                FD_SET(fe->fd, &rfds);
            if (fe->mask & JIM_EVENT_WRITABLE)
                FD_SET(fe->fd, &wfds);
            if (fe->mask & JIM_EVENT_EXCEPTION)
                FD_SET(fe->fd, &efds);
            if (maxfd < fe->fd)
                maxfd = fe->fd;
            fe = fe->next;
        }

        if (sleep_us >= 0) {
            tvp = &tv;
            tvp->tv_sec = sleep_us / 1000000;
            tvp->tv_usec = sleep_us % 1000000;
        }

        retval = prj_select(maxfd + 1, &rfds, &wfds, &efds, tvp); // #NonPortFunc #SockFunc

        if (retval < 0) {
            if (errno == EINVAL) {
                /* This can happen on mingw32 if a non-socket filehandle is passed */
                Jim_SetResultString(interp, "non-waitable filehandle", -1);
                return -2;
            }
        }
        else if (retval > 0) {
            fe = eventLoop->fileEventHead;
            while (fe != NULL) {
                int mask = 0;
                int fd = fe->fd;

                if ((fe->mask & JIM_EVENT_READABLE) && FD_ISSET(fd, &rfds))
                    mask |= JIM_EVENT_READABLE;
                if (fe->mask & JIM_EVENT_WRITABLE && FD_ISSET(fd, &wfds))
                    mask |= JIM_EVENT_WRITABLE;
                if (fe->mask & JIM_EVENT_EXCEPTION && FD_ISSET(fd, &efds))
                    mask |= JIM_EVENT_EXCEPTION;

                if (mask) {
                    int ret = fe->fileProc(interp, fe->clientData, mask);
                    if (ret != JIM_OK && ret != JIM_RETURN) {
                        /* Remove the element on handler error */
                        Jim_DeleteFileHandler(interp, fd, mask);
                        /* At this point fe is no longer valid - it will be assigned below */
                    }
                    processed++;
                    /* After an event is processed our file event list
                     * may no longer be the same, so what we do
                     * is to clear the bit for this file descriptor and
                     * restart again from the head. */
                    FD_CLR(fd, &rfds);
                    FD_CLR(fd, &wfds);
                    FD_CLR(fd, &efds);
                    fe = eventLoop->fileEventHead;
                }
                else {
                    fe = fe->next;
                }
            }
        }
    }
#else
    if (sleep_us > 0) {
        prj_usleep((prj_useconds_t)sleep_us); // #NonPortFuncFix
    }
#endif

    /* Check time events */
    te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId;
    while (te) {
        jim_wide id;

        if (te->id > maxId) {
            te = te->next;
            continue;
        }
        if (JimGetTimeUsec(eventLoop) >= te->when) {
            id = te->id;
            /* Remove from the list before executing */
            Jim_RemoveTimeHandler(eventLoop, id);
            te->timeProc(interp, te->clientData);
            /* After an event is processed our time event list may
             * no longer be the same, so we restart from head.
             * Still we make sure to don't process events registered
             * by event handlers itself in order to don't loop forever
             * even in case an [after 0] that continuously register
             * itself. To do so we saved the max ID we want to handle. */
            Jim_FreeTimeHandler(interp, te);

            te = eventLoop->timeEventHead;
            processed++;
        }
        else {
            te = te->next;
        }
    }

    return processed;
}

/* ---------------------------------------------------------------------- */

static void JimELAssocDataDeleProc(Jim_InterpPtr interp, void *data)
{
    void *next;
    Jim_FileEvent *fe;
    Jim_TimeEvent *te;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)data;

    fe = eventLoop->fileEventHead;
    while (fe) {
        next = fe->next;
        if (fe->finalizerProc)
            fe->finalizerProc(interp, fe->clientData);
        Jim_TFree<Jim_FileEvent>(fe); // #FreeF 
        fe = (Jim_FileEvent*)next;
    }

    te = eventLoop->timeEventHead;
    while (te) {
        next = te->next;
        if (te->finalizerProc)
            te->finalizerProc(interp, te->clientData);
        Jim_TFree<Jim_TimeEvent>(te); // #FreeF 
        te = (Jim_TimeEvent*)next;
    }
    Jim_TFree<void>(data); // #FreeF 
}

static Retval JimELVwaitCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_CmdPrivData(interp);
    Jim_Obj *oldValue;
    Retval rc;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "name");
        return JIM_ERR;
    }

    oldValue = Jim_GetGlobalVariable(interp, argv[1], JIM_NONE);
    if (oldValue) {
        Jim_IncrRefCount(oldValue);
    }
    else {
        /* If a result was left, it is an error */
        if (Jim_Length(Jim_GetResult(interp))) {
            return JIM_ERR;
        }
    }

    eventLoop->suppress_bgerror = 0;

    while ((rc = Jim_ProcessEvents(interp, JIM_ALL_EVENTS)) >= 0) {
        Jim_Obj *currValue;
        currValue = Jim_GetGlobalVariable(interp, argv[1], JIM_NONE);
        /* Stop the loop if the vwait-ed variable changed value,
         * or if was unset and now is set (or the contrary)
         * or if a signal was caught
         */
        if ((oldValue && !currValue) ||
            (!oldValue && currValue) ||
            (oldValue && currValue && !Jim_StringEqObj(oldValue, currValue)) ||
            Jim_CheckSignal(interp)) {
            break;
        }
    }
    if (oldValue)
        Jim_DecrRefCount(interp, oldValue);

    if (rc == -2) {
        return JIM_ERR;
    }

    Jim_SetEmptyResult(interp);
    return JIM_OK;
}

static Retval JimELUpdateCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_CmdPrivData(interp);
    static const char * const options[] = {
        "idletasks", NULL
    };
    enum { UPDATE_IDLE, UPDATE_NONE };
    int option = UPDATE_NONE;
    int flags = JIM_TIME_EVENTS;

    if (argc == 1) {
        flags = JIM_ALL_EVENTS;
    }
    else if (argc > 2 || Jim_GetEnum(interp, argv[1], options, &option, NULL, JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK) {
        Jim_WrongNumArgs(interp, 1, argv, "?idletasks?");
        return JIM_ERR;
    }

    eventLoop->suppress_bgerror = 0;

    while (Jim_ProcessEvents(interp, flags | JIM_DONT_WAIT) > 0) {
    }

    return JIM_OK;
}

static void JimAfterTimeHandler(Jim_InterpPtr interp, void *clientData)
{
    Jim_Obj *objPtr = (Jim_Obj*)clientData;

    Jim_EvalObjBackground(interp, objPtr);
}

static void JimAfterTimeEventFinalizer(Jim_InterpPtr interp, void *clientData)
{
    Jim_Obj *objPtr = (Jim_Obj*)clientData;

    Jim_DecrRefCount(interp, objPtr);
}

static Retval JimELAfterCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_CmdPrivData(interp);
    double ms = 0;
    jim_wide id;
    Jim_Obj *objPtr, *idObjPtr;
    static const char * const options[] = {
        "cancel", "info", "idle", NULL
    };
    enum
    { AFTER_CANCEL, AFTER_INFO, AFTER_IDLE, AFTER_RESTART, AFTER_EXPIRE, AFTER_CREATE };
    int option = AFTER_CREATE;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "option ?arg ...?");
        return JIM_ERR;
    }
    if (Jim_GetDouble(interp, argv[1], &ms) != JIM_OK) {
        if (Jim_GetEnum(interp, argv[1], options, &option, "argument", JIM_ERRMSG) != JIM_OK) {
            return JIM_ERR;
        }
        Jim_SetEmptyResult(interp);
    }
    else if (argc == 2) {
        /* Simply a sleep */
        prj_usleep((prj_useconds_t)(ms * 1000)); // #NonPortFuncFix
        return JIM_OK;
    }

    switch (option) {
        case AFTER_IDLE:
            if (argc < 3) {
                Jim_WrongNumArgs(interp, 2, argv, "script ?script ...?");
                return JIM_ERR;
            }
            /* fall through */
        case AFTER_CREATE: {
            Jim_Obj *scriptObj = Jim_ConcatObj(interp, argc - 2, argv + 2);
            Jim_IncrRefCount(scriptObj);
            id = Jim_CreateTimeHandler(interp, (jim_wide)(ms * 1000), JimAfterTimeHandler, scriptObj,
                JimAfterTimeEventFinalizer);
            objPtr = Jim_NewStringObj(interp, NULL, 0);
            Jim_AppendString(interp, objPtr, "after#", -1);
            idObjPtr = Jim_NewIntObj(interp, id);
            Jim_IncrRefCount(idObjPtr);
            Jim_AppendObj(interp, objPtr, idObjPtr);
            Jim_DecrRefCount(interp, idObjPtr);
            Jim_SetResult(interp, objPtr);
            return JIM_OK;
        }
        case AFTER_CANCEL:
            if (argc < 3) {
                Jim_WrongNumArgs(interp, 2, argv, "id|command");
                return JIM_ERR;
            }
            else {
                jim_wide remain = 0;

                id = JimParseAfterId(argv[2]);
                if (id <= 0) {
                    /* Not an event id, so search by script */
                    Jim_Obj *scriptObj = Jim_ConcatObj(interp, argc - 2, argv + 2);
                    id = JimFindAfterByScript(eventLoop, scriptObj);
                    Jim_FreeNewObj(interp, scriptObj);
                    if (id <= 0) {
                        /* Not found */
                        break;
                    }
                }
                remain = Jim_DeleteTimeHandler(interp, id);
                if (remain >= 0) {
                    Jim_SetResultInt(interp, remain);
                }
            }
            break;

        case AFTER_INFO:
            if (argc == 2) {
                Jim_TimeEvent *te = eventLoop->timeEventHead;
                Jim_Obj *listObj = Jim_NewListObj(interp, NULL, 0);
                char buf[30];
                const char *fmt = "after#%" JIM_WIDE_MODIFIER;

                while (te) {
                    snprintf(buf, sizeof(buf), fmt, te->id);
                    Jim_ListAppendElement(interp, listObj, Jim_NewStringObj(interp, buf, -1));
                    te = te->next;
                }
                Jim_SetResult(interp, listObj);
            }
            else if (argc == 3) {
                id = JimParseAfterId(argv[2]);
                if (id >= 0) {
                    Jim_TimeEvent *e = JimFindTimeHandlerById(eventLoop, id);
                    if (e && e->timeProc == JimAfterTimeHandler) {
                        Jim_Obj *listObj = Jim_NewListObj(interp, NULL, 0);
                        Jim_ListAppendElement(interp, listObj, (Jim_Obj*)e->clientData);
                        Jim_ListAppendElement(interp, listObj, Jim_NewStringObj(interp, e->initialus ? "timer" : "idle", -1));
                        Jim_SetResult(interp, listObj);
                        return JIM_OK;
                    }
                }
                Jim_SetResultFormatted(interp, "event \"%#s\" doesn't exist", argv[2]);
                return JIM_ERR;
            }
            else {
                Jim_WrongNumArgs(interp, 2, argv, "?id?");
                return JIM_ERR;
            }
            break;
    }
    return JIM_OK;
}

Retval Jim_eventloopInit(Jim_InterpPtr interp)
{
    Jim_EventLoop *eventLoop;

    if (Jim_PackageProvide(interp, "eventloop", "1.0", JIM_ERRMSG))
        return JIM_ERR;

    eventLoop = Jim_TAllocZ<Jim_EventLoop>(); // #AllocF 
    //memset(eventLoop, 0, sizeof(*eventLoop));

    Jim_SetAssocData(interp, "eventloop", JimELAssocDataDeleProc, eventLoop);

    Jim_CreateCommand(interp, "vwait", JimELVwaitCommand, eventLoop, NULL);
    Jim_CreateCommand(interp, "update", JimELUpdateCommand, eventLoop, NULL);
    Jim_CreateCommand(interp, "after", JimELAfterCommand, eventLoop, NULL);

    return JIM_OK;
}

END_JIM_NAMESPACE