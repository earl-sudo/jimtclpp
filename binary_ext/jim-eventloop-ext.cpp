/* Jim - A small embeddable Tcl interpreter
 *
 * Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
 * Copyright 2005 Clemens Hintze <c.hintze@gmx.net>
 * Copyright 2005 patthoyts - Pat Thoyts <patthoyts@users.sf.net>
 * Copyright 2008 oharboe - Øyvind Harboe - oyvind.harboe@zylin.com
 * Copyright 2008 Andrew Lunn <andrew@lunn.ch_>
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

#if defined(PRJ_OS_LINUX) || defined(PRJ_OS_MACOS)
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/* --- */
BEGIN_JIM_NAMESPACE

/* File event structure */
struct Jim_FileEvent
{
    int fd_;
    int mask_;                   /* one of JIM_EVENT_(READABLE|WRITABLE|EXCEPTION) */
    Jim_FileProc *fileProc_;
    Jim_EventFinalizerProc *finalizerProc_;
    void *clientData_;
    struct Jim_FileEvent *next_;

    inline int fd() const { return fd_; }
    inline void setFd(int v) { fd_ = v; }
    inline int mask() const { return mask_; }
    inline void setMask(int v) { mask_ = v; }
    inline void* clientData() const { return clientData_; }
    inline void setClientData(void* v) { clientData_ = v; }
};

/* Time event structure */
struct Jim_TimeEvent
{
    jim_wide id_;                /* time event identifier. */
    jim_wide initialus_;         /* initial relative timer value in microseconds */
    jim_wide when_;              /* microseconds */
    Jim_TimeProc* timeProc_;
    Jim_EventFinalizerProc *finalizerProc_;
    void *clientData_;
    struct Jim_TimeEvent *next_;

    inline jim_wide id() const { return id_; }
    inline void setId(jim_wide v) { id_ = v; }
    inline jim_wide initialus() const { return initialus_; }
    inline void setInitialus(jim_wide v) { initialus_ = v; }
    inline jim_wide when() const { return when_; }
    inline void setWhen(jim_wide val) { when_ = val; }
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_Jim_FileEvent       Jim_TAlloc<Jim_FileEvent>(1,"Jim_FileEvent")
#define free_Jim_FileEvent(ptr) Jim_TFree<Jim_FileEvent>(ptr,"Jim_FileEvent")
#define new_Jim_TimeEvent       Jim_TAlloc<Jim_TimeEvent>(1,"Jim_TimeEvent")
#define free_Jim_TimeEvent(ptr) Jim_TFree<Jim_TimeEvent>(ptr,"Jim_TimeEvent")


/* Per-interp_ structure containing the state of the event loop */
struct Jim_EventLoop
{
    Jim_FileEvent *fileEventHead_;
    Jim_TimeEvent *timeEventHead_;
    jim_wide timeEventNextId_;   /* highest event id_ created, starting at 1 */
    time_t timeBase_;
    int suppress_bgerror_; /* bgerror returned break, so don't call it again */
};

static void JimAfterTimeHandler(Jim_InterpPtr interp, void *clientData);
static void JimAfterTimeEventFinalizer(Jim_InterpPtr interp, void *clientData);

int Jim_EvalObjBackground(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr)
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");
    Jim_CallFramePtr savedFramePtr;
    int retval;

    savedFramePtr = interp->framePtr();
    interp->framePtr(Jim_TopCallFrame(interp));
    retval = Jim_EvalObj(interp, scriptObjPtr);
    interp->framePtr(savedFramePtr);
    /* Try to report the errorText_ (if any) via the bgerror proc */
    if (retval != JRET(JIM_OK) && retval != JRET(JIM_RETURN) && !eventLoop->suppress_bgerror_) {
        Jim_ObjPtr objv[2];
        int rc = JRET(JIM_ERR);

        objv[0] = Jim_NewStringObj(interp, "bgerror", -1);
        objv[1] = Jim_GetResult(interp);
        Jim_IncrRefCount(objv[0]);
        Jim_IncrRefCount(objv[1]);
        if (Jim_GetCommand(interp, objv[0], JIM_NONE) == NULL || (rc = Jim_EvalObjVector(interp, 2, objv)) != JRET(JIM_OK)) {
            if (rc == JRET(JIM_BREAK)) {
                /* No more bgerror calls */
                eventLoop->suppress_bgerror_++;
            }
            else {
                /* Report the errorText_ to stderr. */
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

    fe = new_Jim_FileEvent; // #AllocF 
    fe->setFd(fd);
    fe->setMask(mask);
    fe->fileProc_ = proc;
    fe->finalizerProc_ = finalizerProc;
    fe->setClientData(clientData);
    fe->next_ = eventLoop->fileEventHead_;
    eventLoop->fileEventHead_ = fe;
}

/**
 * Removes all event handlers for 'handle' that match 'mask_'.
 */
void Jim_DeleteFileHandler(Jim_InterpPtr interp, int fd, int mask)
{
    Jim_FileEvent *fe, *next, *prev = NULL;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");

    for (fe = eventLoop->fileEventHead_; fe; fe = next) {
        next = fe->next_;
        if (fe->fd() == fd && (fe->mask() & mask)) {
            /* Remove this entry from the list */
            if (prev == NULL)
                eventLoop->fileEventHead_ = next;
            else
                prev->next_ = next;
            if (fe->finalizerProc_)
                fe->finalizerProc_(interp, fe->clientData());
            free_Jim_FileEvent(fe); // #FreeF 
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
 * Returns the time since interp_ creation in microseconds.
 */
CHKRET static jim_wide JimGetTimeUsec(Jim_EventLoop *eventLoop)
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

    return now - eventLoop->timeBase_;
}

jim_wide Jim_CreateTimeHandler(Jim_InterpPtr interp, jim_wide us,
    Jim_TimeProc * proc, void *clientData, Jim_EventFinalizerProc * finalizerProc)
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");
    jim_wide id = ++eventLoop->timeEventNextId_;
    Jim_TimeEvent *te, *e, *prev;

    te = new_Jim_TimeEvent; // #AllocF 
    te->setId(id);
    te->setInitialus(us);
    te->setWhen(JimGetTimeUsec(eventLoop) + us);
    te->timeProc_ = proc;
    te->finalizerProc_ = finalizerProc;
    te->clientData_ = clientData;

    /* Add to the appropriate place in the list */
    prev = NULL;
    for (e = eventLoop->timeEventHead_; e; e = e->next_) {
        if (te->when() < e->when()) {
            break;
        }
        prev = e;
    }
    if (prev) {
        te->next_ = prev->next_;
        prev->next_ = te;
    }
    else {
        te->next_ = eventLoop->timeEventHead_;
        eventLoop->timeEventHead_ = te;
    }

    return id;
}

CHKRET static jim_wide JimParseAfterId(Jim_ObjPtr idObj)
{
    const char *tok = Jim_String(idObj);
    jim_wide id;

    if (strncmp(tok, "after#", 6) == 0 && Jim_StringToWide(tok + 6, &id, 10) == JRET(JIM_OK)) {
        /* Got an event by id_ */
        return id;
    }
    return -1;
}

CHKRET static jim_wide JimFindAfterByScript(Jim_EventLoop *eventLoop, Jim_ObjPtr scriptObj)
{
    Jim_TimeEvent *te;

    for (te = eventLoop->timeEventHead_; te; te = te->next_) {
        /* Is this an 'after' event? */
        if (te->timeProc_ == JimAfterTimeHandler) {
            if (Jim_StringEqObj(scriptObj, (Jim_ObjPtr )te->clientData_)) {
                return te->id();
            }
        }
    }
    return -1;                  /* NO event with the specified ID found */
}

CHKRET static Jim_TimeEvent *JimFindTimeHandlerById(Jim_EventLoop *eventLoop, jim_wide id)
{
    Jim_TimeEvent *te;

    for (te = eventLoop->timeEventHead_; te; te = te->next_) {
        if (te->id() == id) {
            return te;
        }
    }
    return NULL;
}

CHKRET static Jim_TimeEvent *Jim_RemoveTimeHandler(Jim_EventLoop *eventLoop, jim_wide id)
{
    Jim_TimeEvent *te, *prev = NULL;

    for (te = eventLoop->timeEventHead_; te; te = te->next_) {
        if (te->id() == id) {
            if (prev == NULL)
                eventLoop->timeEventHead_ = te->next_;
            else
                prev->next_ = te->next_;
            return te;
        }
        prev = te;
    }
    return NULL;
}

static void Jim_FreeTimeHandler(Jim_InterpPtr interp, Jim_TimeEvent *te)
{
    if (te->finalizerProc_)
        te->finalizerProc_(interp, te->clientData_);
    free_Jim_TimeEvent(te); // #FreeF 
}

jim_wide Jim_DeleteTimeHandler(Jim_InterpPtr interp, jim_wide id)
{
    Jim_TimeEvent *te;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");

    if (id > eventLoop->timeEventNextId_) {
        return -2;              /* wrong event ID */
    }

    te = Jim_RemoveTimeHandler(eventLoop, id);
    if (te) {
        jim_wide remain;

        remain = te->when() - JimGetTimeUsec(eventLoop);
        remain = (remain < 0) ? 0 : remain;

        Jim_FreeTimeHandler(interp, te);
        return remain;
    }
    return -1;                  /* NO event with the specified ID found */
}

/* --- POSIX version of Jim_ProcessEvents, for now the only available --- */

/* Process every pending time event, then every pending file event
 * (that may be registered by time event callbacks just processed).
 * The behaviour depends upon the setting of flags_:
 *
 * If flags_ is 0, the function_ does nothing and returns.
 * if flags_ has JIM_ALL_EVENTS set, all event types are processed.
 * if flags_ has JIM_FILE_EVENTS set, file events are processed.
 * if flags_ has JIM_TIME_EVENTS set, time events are processed.
 * if flags_ has JIM_DONT_WAIT set, the function_ returns as soon as all
 * the events that are possible to process without waiting are processed.
 *
 * Returns the number of events processed or -1 if
 * there are no matching handlers, or -2 on errorText_.
 */
int Jim_ProcessEvents(Jim_InterpPtr interp, int flags)
{
    jim_wide sleep_us = -1;
    int processed = 0;
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_GetAssocData(interp, "eventloop");
    Jim_FileEvent *fe = eventLoop->fileEventHead_;
    Jim_TimeEvent *te;
    jim_wide maxId;

    if ((flags & JIM_FILE_EVENTS) == 0 || fe == NULL) {
        /* No file events */
        if ((flags & JIM_TIME_EVENTS) == 0 || eventLoop->timeEventHead_ == NULL) {
            /* No time events */
            return -1;
        }
    }

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in lsortOrder_ to sleep until the next_ time event is ready
     * to fire. */

    if (flags & JIM_DONT_WAIT) {
        /* Wait no time */
        sleep_us = 0;
    }
    else if (flags & JIM_TIME_EVENTS) {
        /* The nearest timer is always at the head of the list */
        if (eventLoop->timeEventHead_) {
            Jim_TimeEvent *shortest = eventLoop->timeEventHead_;

            /* Calculate the time missing_ for the nearest
             * timer to fire. */
            sleep_us = shortest->when() - JimGetTimeUsec(eventLoop);
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
            if (fe->mask_ & JIM_EVENT_READABLE)
                FD_SET(fe->fd_, &rfds);
            if (fe->mask_ & JIM_EVENT_WRITABLE)
                FD_SET(fe->fd_, &wfds);
            if (fe->mask_ & JIM_EVENT_EXCEPTION)
                FD_SET(fe->fd_, &efds);
            if (maxfd < fe->fd_)
                maxfd = fe->fd_;
            fe = fe->next_;
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
                Jim_SetResultString(interp, "non-waitable filehandle", -1); // #ErrStr
                return -2;
            }
        }
        else if (retval > 0) {
            fe = eventLoop->fileEventHead_;
            while (fe != NULL) {
                int mask_ = 0;
                int fd_ = fe->fd_;

                if ((fe->mask_ & JIM_EVENT_READABLE) && FD_ISSET(fd_, &rfds))
                    mask_ |= JIM_EVENT_READABLE;
                if (fe->mask_ & JIM_EVENT_WRITABLE && FD_ISSET(fd_, &wfds))
                    mask_ |= JIM_EVENT_WRITABLE;
                if (fe->mask_ & JIM_EVENT_EXCEPTION && FD_ISSET(fd_, &efds))
                    mask_ |= JIM_EVENT_EXCEPTION;

                if (mask_) {
                    int ret = fe->fileProc_(interp, fe->clientData_(), mask_);
                    if (ret != JRET(JIM_OK) && ret != JRET(JIM_RETURN)) {
                        /* Remove the element on handler errorText_ */
                        Jim_DeleteFileHandler(interp, fd_, mask_);
                        /* At this point fe is no longer valid - it will be assigned below */
                    }
                    processed++;
                    /* After an event is processed our file event list
                     * may no longer be the same, so what we do
                     * is to clear the bit for this file descriptor and
                     * restart again from the head. */
                    FD_CLR(fd_, &rfds);
                    FD_CLR(fd_, &wfds);
                    FD_CLR(fd_, &efds);
                    fe = eventLoop->fileEventHead_;
                }
                else {
                    fe = fe->next_;
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
    te = eventLoop->timeEventHead_;
    maxId = eventLoop->timeEventNextId_;
    while (te) {
        jim_wide id;

        if (te->id() > maxId) {
            te = te->next_;
            continue;
        }
        if (JimGetTimeUsec(eventLoop) >= te->when()) {
            id = te->id();
            /* Remove from the list before executing */
            IGNOREPTRRET Jim_RemoveTimeHandler(eventLoop, id);
            te->timeProc_(interp, te->clientData_);
            /* After an event is processed our time event list may
             * no longer be the same, so we restart from head.
             * Still we make sure to don't process events registered
             * by event handlers itself in lsortOrder_ to don't loop forever
             * even in case an [after 0] that continuously register
             * itself. To do so we saved the max ID we want to handle. */
            Jim_FreeTimeHandler(interp, te);

            te = eventLoop->timeEventHead_;
            processed++;
        }
        else {
            te = te->next_;
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

    fe = eventLoop->fileEventHead_;
    while (fe) {
        next = fe->next_;
        if (fe->finalizerProc_)
            fe->finalizerProc_(interp, fe->clientData());
        free_Jim_FileEvent(fe); // #FreeF 
        fe = (Jim_FileEvent*)next;
    }

    te = eventLoop->timeEventHead_;
    while (te) {
        next = te->next_;
        if (te->finalizerProc_)
            te->finalizerProc_(interp, te->clientData_);
        free_Jim_TimeEvent(te); // #FreeF 
        te = (Jim_TimeEvent*)next;
    }
    Jim_TFree<void>(data); // #FreeF 
}

CHKRET static Retval JimELVwaitCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_CmdPrivData(interp);
    Jim_ObjPtr oldValue;
    Retval rc;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "name"); // #ErrStr
        return JRET(JIM_ERR);
    }

    oldValue = Jim_GetGlobalVariable(interp, argv[1], JIM_NONE);
    if (oldValue) {
        Jim_IncrRefCount(oldValue);
    }
    else {
        /* If a result was left_, it is an errorText_ */
        if (Jim_Length(Jim_GetResult(interp))) {
            return JRET(JIM_ERR);
        }
    }

    eventLoop->suppress_bgerror_ = 0;

    while ((rc = Jim_ProcessEvents(interp, JIM_ALL_EVENTS)) >= 0) {
        Jim_ObjPtr currValue;
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
        return JRET(JIM_ERR);
    }

    Jim_SetEmptyResult(interp);
    return JRET(JIM_OK);
}

CHKRET static Retval JimELUpdateCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
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
    else if (argc > 2 || Jim_GetEnum(interp, argv[1], options, &option, NULL, JIM_ERRMSG | JIM_ENUM_ABBREV) != JRET(JIM_OK)) {
        Jim_WrongNumArgs(interp, 1, argv, "?idletasks?"); // #ErrStr
        return JRET(JIM_ERR);
    }

    eventLoop->suppress_bgerror_ = 0;

    while (Jim_ProcessEvents(interp, flags | JIM_DONT_WAIT) > 0) {
    }

    return JRET(JIM_OK);
}

static void JimAfterTimeHandler(Jim_InterpPtr interp, void *clientData)
{
    Jim_ObjPtr objPtr = (Jim_ObjPtr )clientData;

    Jim_EvalObjBackground(interp, objPtr);
}

static void JimAfterTimeEventFinalizer(Jim_InterpPtr interp, void *clientData)
{
    Jim_ObjPtr objPtr = (Jim_ObjPtr )clientData;

    Jim_DecrRefCount(interp, objPtr);
}

static Retval JimELAfterCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_EventLoop *eventLoop = (Jim_EventLoop*)Jim_CmdPrivData(interp);
    double ms = 0;
    jim_wide id;
    Jim_Obj* objPtr, *idObjPtr;
    static const char * const options[] = {
        "cancel", "info", "idle", NULL
    };
    enum
    { AFTER_CANCEL, AFTER_INFO, AFTER_IDLE, AFTER_RESTART, AFTER_EXPIRE, AFTER_CREATE };
    int option = AFTER_CREATE;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "option ?arg ...?"); // #ErrStr
        return JRET(JIM_ERR);
    }
    if (Jim_GetDouble(interp, argv[1], &ms) != JRET(JIM_OK)) {
        if (Jim_GetEnum(interp, argv[1], options, &option, "argument", JIM_ERRMSG) != JRET(JIM_OK)) {
            return JRET(JIM_ERR);
        }
        Jim_SetEmptyResult(interp);
    }
    else if (argc == 2) {
        /* Simply a sleep */
        prj_usleep((prj_useconds_t)(ms * 1000)); // #NonPortFuncFix
        return JRET(JIM_OK);
    }

    switch (option) {
        case AFTER_IDLE:
            if (argc < 3) {
                Jim_WrongNumArgs(interp, 2, argv, "script ?script ...?");
                return JRET(JIM_ERR);
            }
            /* fall through */
        case AFTER_CREATE: {
            Jim_ObjPtr scriptObj = Jim_ConcatObj(interp, argc - 2, argv + 2);
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
            return JRET(JIM_OK);
        }
        case AFTER_CANCEL:
            if (argc < 3) {
                Jim_WrongNumArgs(interp, 2, argv, "id|command");
                return JRET(JIM_ERR);
            }
            else {
                jim_wide remain = 0;

                id = JimParseAfterId(argv[2]);
                if (id <= 0) {
                    /* Not an event id_, so search by script */
                    Jim_ObjPtr scriptObj = Jim_ConcatObj(interp, argc - 2, argv + 2);
                    id = JimFindAfterByScript(eventLoop, scriptObj);
                    Jim_FreeObj(interp, scriptObj);
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
                Jim_TimeEvent *te = eventLoop->timeEventHead_;
                Jim_ObjPtr listObj = Jim_NewListObj(interp, NULL, 0);
                char buf[30];
                const char *fmt = "after#%" JIM_WIDE_MODIFIER;

                while (te) {
                    snprintf(buf, sizeof(buf), fmt, te->id());
                    Jim_ListAppendElement(interp, listObj, Jim_NewStringObj(interp, buf, -1));
                    te = te->next_;
                }
                Jim_SetResult(interp, listObj);
            }
            else if (argc == 3) {
                id = JimParseAfterId(argv[2]);
                if (id >= 0) {
                    Jim_TimeEvent *e = JimFindTimeHandlerById(eventLoop, id);
                    if (e && e->timeProc_ == JimAfterTimeHandler) {
                        Jim_ObjPtr listObj = Jim_NewListObj(interp, NULL, 0);
                        Jim_ListAppendElement(interp, listObj, (Jim_ObjPtr )e->clientData_);
                        Jim_ListAppendElement(interp, listObj, Jim_NewStringObj(interp, e->initialus() ? "timer" : "idle", -1));
                        Jim_SetResult(interp, listObj);
                        return JRET(JIM_OK);
                    }
                }
                Jim_SetResultFormatted(interp, "event \"%#s\" doesn't exist", argv[2]);
                return JRET(JIM_ERR);
            }
            else {
                Jim_WrongNumArgs(interp, 2, argv, "?id?");
                return JRET(JIM_ERR);
            }
            break;
    }
    return JRET(JIM_OK);
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-eventloop-version.h>

Retval Jim_eventloopInit(Jim_InterpPtr interp)
{
    Jim_EventLoop *eventLoop;

    if (Jim_PackageProvide(interp, "eventloop", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

    eventLoop = Jim_TAllocZ<Jim_EventLoop>(1,"Jim_EventLoop"); // #AllocF 

    Retval ret = JIM_ERR;

    ret = Jim_SetAssocData(interp, "eventloop", JimELAssocDataDeleProc, eventLoop);
    if (ret != JIM_OK) return ret;

    ret = Jim_CreateCommand(interp, "vwait", JimELVwaitCommand, eventLoop, NULL);
    if (ret != JIM_OK) return ret;

    ret = Jim_CreateCommand(interp, "update", JimELUpdateCommand, eventLoop, NULL);
    if (ret != JIM_OK) return ret;

    ret = Jim_CreateCommand(interp, "after", JimELAfterCommand, eventLoop, NULL);
    if (ret != JIM_OK) return ret;


    return JRET(JIM_OK);
}

END_JIM_NAMESPACE
