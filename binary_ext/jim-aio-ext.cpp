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
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <jimautoconf.h>
#include <prj_compat.h>
#include <jim.h>
#include <jim-eventloop.h>
#include <jim-subcmd.h>
#include <jim-aio-ext.h>

#if jim_ext_aio

#if defined(__MINGW32__) || defined(_MSC_VER) // #optionalCode
#  ifndef HAVE_PIPE 
#    define HAVE_PIPE
#  endif
#endif

#ifdef HAVE_UNISTD_H
#  include <fcntl.h>
#endif

#ifdef JIM_WIDE_4BYTE
#    define JIM_WIDE_MIN LONG_MIN
#    define JIM_WIDE_MAX LONG_MAX
#endif
#ifdef JIM_WIDE_8BYTE
#    define JIM_WIDE_MIN LLONG_MIN
#    define JIM_WIDE_MAX LLONG_MAX
#endif

BEGIN_JIM_NAMESPACE 

enum {
    AIO_CMD_LEN = 32,     /* e.g. aio.handleXXXXXX */
    AIO_BUF_LEN = 256     /* Can keep this small and rely on stdio buffering */
};

enum { AIO_KEEPOPEN = 1 };

#if defined(JIM_IPV6) // #optionalCode #WinOff
#define IPV6 1
#else
#define IPV6 0
#ifndef PF_INET6
#define PF_INET6 0
#endif
#endif

#ifdef JIM_ANSIC // #optionalCode
/* no fdopen() with ANSIC, so can't support these */
#undef HAVE_PIPE
#undef HAVE_SOCKETPAIR
#endif

static Retval compat_fseeko(FILE *stream, prj_off_t offset, int whence) {
    if (prj_funcDef(prj_fseeko)) { // #Unsupported #NonPortFuncFix
        return prj_fseeko(stream, offset, whence);
    } 
    return fseek(stream, testConv<prj_off_t,long>(offset), whence);
}

static int compat_ftello(FILE *stream) {
    if (prj_funcDef(prj_ftello)) { // #Unsupported #NonPortFuncFix
        prj_off_t ret = prj_ftello(stream);
        return testConv<prj_off_t, int>(ret);
    }
    return ftell(stream);
}


static int stdio_writer(struct AioFile *af, const char *buf, int len)
{
    size_t ret = prj_fwrite(buf, 1, len, af->fp); // #output
    return testConv<size_t, int>(ret);
}

static int stdio_reader(struct AioFile *af, char *buf, int len)
{
    size_t ret = prj_fread(buf, 1, len, af->fp); // #input
    return testConv<size_t, int>(ret);
}

static const char *stdio_getline(struct AioFile *af, char *buf, int len)
{
    return prj_fgets(buf, len, af->fp); // #input
}

static int stdio_error(const AioFile *af)
{
    if (!ferror(af->fp)) {
        return JRET(JIM_OK);
    }
    clearerr(af->fp);
    /* EAGAIN and similar are not errorText_ conditions. Just treat them like eof_ */
    if (feof(af->fp) || errno == EAGAIN || errno == EINTR) {
        return JRET(JIM_OK);
    }
#ifdef ECONNRESET // #optionalCode
    if (errno == ECONNRESET) {
        return JRET(JIM_OK);
    }
#endif
#ifdef ECONNABORTED // #optionalCode
    if (errno == ECONNABORTED) {
        return JRET(JIM_OK);
    }
#endif
    return JRET(JIM_ERR);
}

const char *stdio_strerror(struct AioFile *af MAYBE_USED)
{
    return strerror(errno);
}

static const JimAioFopsType g_stdio_fops = {
    stdio_writer,
    stdio_reader,
    stdio_getline,
    stdio_error,
    stdio_strerror,
    nullptr
};


static int JimAioSubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
static AioFile *JimMakeChannel(Jim_InterpPtr interp, FILE *fh, int fd, Jim_ObjPtr filename,
    const char *hdlfmt, int family, const char *mode);


static const char *JimAioErrorString(AioFile *af)
{
    if (af && af->fops)
        return af->fops->strerror(af);

    return strerror(errno);
}

static void JimAioSetError(Jim_InterpPtr interp, Jim_ObjPtr name)
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    if (name) {
        Jim_SetResultFormatted(interp, "%#s: %s", name, JimAioErrorString(af));
    }
    else {
        Jim_SetResultString(interp, JimAioErrorString(af), -1);
    }
}

static int JimCheckStreamError(Jim_InterpPtr interp, AioFile *af)
{
	int ret = af->fops->error(af);
	if (ret) {
		JimAioSetError(interp, af->filename);
	}
	return ret;
}

static void JimAioDelProc(Jim_InterpPtr interp, void *privData)
{
    AioFile *af = (AioFile*)privData;

    JIM_NOTUSED(interp);

    Jim_DecrRefCount(interp, af->filename);

#ifdef jim_ext_eventloop // #optionalCode
    /* remove all existing EventHandlers */
    Jim_DeleteFileHandler(interp, af->fd, JIM_EVENT_READABLE | JIM_EVENT_WRITABLE | JIM_EVENT_EXCEPTION);
#endif /* jim_ext_eventloop*/

#if defined(JIM_SSL) // #optionalCode #WinOff
    if (af->ssl != nullptr) {
        SSL_free((SSL*)af->ssl);
    }
#endif /* defined(JIM_SSL) */
    if (!(af->openFlags & AIO_KEEPOPEN)) {
        fclose(af->fp);
    }

    free_AioFile(af); // #FreeF 
}

static Retval aio_cmd_read(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    char buf[AIO_BUF_LEN];
    Jim_ObjPtr objPtr;
    int nonewline = 0;
    jim_wide neededLen = -1;         /* -1 is "read as much as possible" */

    if (argc && Jim_CompareStringImmediate(interp, argv[0], "-nonewline")) {
        nonewline = 1;
        argv++;
        argc--;
    }
    if (argc == 1) {
        if (Jim_GetWide(interp, argv[0], &neededLen) != JRET(JIM_OK))
            return JRET(JIM_ERR);
        if (neededLen < 0) {
            Jim_SetResultString(interp, "invalid parameter: negative len", -1); // #ErrStr
            return JRET(JIM_ERR);
        }
    }
    else if (argc) {
        return -1;
    }
    objPtr = Jim_NewStringObj(interp, nullptr, 0);
    while (neededLen != 0) {
        int retval;
        int readlen;

        if (neededLen == -1) {
            readlen = AIO_BUF_LEN;
        }
        else {
            int64_t val = (neededLen > AIO_BUF_LEN ? AIO_BUF_LEN : neededLen);
            readlen = testConv<int64_t, int>(val);
        }
        retval = af->fops->reader(af, buf, readlen);
        if (retval > 0) {
            Jim_AppendString(interp, objPtr, buf, retval);
            if (neededLen != -1) {
                neededLen -= retval;
            }
        }
        if (retval != readlen)
            break;
    }
    /* Check for errorText_ conditions */
    if (JimCheckStreamError(interp, af)) {
        Jim_FreeObj(interp, objPtr);
        return JRET(JIM_ERR);
    }
    if (nonewline) {
        int len;
        const char *s = Jim_GetString(objPtr, &len);

        if (len > 0 && s[len - 1] == '\n') {
            objPtr->lengthDecr();
            objPtr->bytes_NULLterminate();
        }
    }
    Jim_SetResult(interp, objPtr);
    return JRET(JIM_OK);
}

AioFile *Jim_AioFile(Jim_InterpPtr interp, Jim_ObjPtr command)
{
    Jim_CmdPtr cmdPtr = Jim_GetCommand(interp, command, JIM_ERRMSG);

    /* XXX: There ought to be a supported API for this */
    if (cmdPtr && !cmdPtr->isproc() && cmdPtr->cmdProc() == JimAioSubCmdProc) {
        return cmdPtr->getPrivData<AioFile*>();
    }
    Jim_SetResultFormatted(interp, "Not a filehandle: \"%#s\"", command); // #ErrStr
    return nullptr;
}

JIM_EXPORT FILE *Jim_AioFilehandle(Jim_InterpPtr interp, Jim_ObjPtr command)
{
    AioFile *af;

    af = Jim_AioFile(interp, command);
    if (af == nullptr) {
        return nullptr;
    }

    return af->fp;
}

static Retval aio_cmd_getfd(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    fflush(af->fp);
    Jim_SetResultInt(interp, prj_fileno(af->fp)); // #NonPortFuncFix

    return JRET(JIM_OK);
}

static Retval aio_cmd_copy(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    jim_wide count = 0;
    jim_wide maxlen = JIM_WIDE_MAX;
    AioFile *outf = Jim_AioFile(interp, argv[0]);

    if (outf == nullptr) {
        return JRET(JIM_ERR);
    }

    if (argc == 2) {
        if (Jim_GetWide(interp, argv[1], &maxlen) != JRET(JIM_OK)) {
            return JRET(JIM_ERR);
        }
    }

    while (count < maxlen) {
        char ch;

        if (af->fops->reader(af, &ch, 1) != 1) {
            break;
        }
        if (outf->fops->writer(outf, &ch, 1) != 1) {
            break;
        }
        count++;
    }

    if (JimCheckStreamError(interp, af) || JimCheckStreamError(interp, outf)) {
        return JRET(JIM_ERR);
    }

    Jim_SetResultInt(interp, count);

    return JRET(JIM_OK);
}

static Retval aio_cmd_gets(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    char buf[AIO_BUF_LEN];
    Jim_ObjPtr objPtr;
    int len;

    errno = 0;

    objPtr = Jim_NewStringObj(interp, nullptr, 0);
    while (true) {
        buf[AIO_BUF_LEN - 1] = '_';

        if (af->fops->getline(af, buf, AIO_BUF_LEN) == nullptr)
            break;

        if (buf[AIO_BUF_LEN - 1] == '\0' && buf[AIO_BUF_LEN - 2] != '\n') {
            Jim_AppendString(interp, objPtr, buf, AIO_BUF_LEN - 1);
        }
        else {
            size_t slen = strlen(buf);
            len = testConv<size_t,int>(slen);

            if (len && (buf[len - 1] == '\n')) {
                /* strip "\n" */
                len--;
            }

            Jim_AppendString(interp, objPtr, buf, len);
            break;
        }
    }

    if (JimCheckStreamError(interp, af)) {
        /* I/O errorText_ */
        Jim_FreeObj(interp, objPtr);
        return JRET(JIM_ERR);
    }

    if (argc) {
        if (Jim_SetVariable(interp, argv[0], objPtr) != JRET(JIM_OK)) {
            Jim_FreeObj(interp, objPtr);
            return JRET(JIM_ERR);
        }

        len = Jim_Length(objPtr);

        if (len == 0 && feof(af->fp)) {
            /* On EOF returns -1 if varName was specified */
            len = -1;
        }
        Jim_SetResultInt(interp, len);
    }
    else {
        Jim_SetResult(interp, objPtr);
    }
    return JRET(JIM_OK);
}

static Retval aio_cmd_puts(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    int wlen;
    const char *wdata;
    Jim_ObjPtr strObj;

    if (argc == 2) {
        if (!Jim_CompareStringImmediate(interp, argv[0], "-nonewline")) {
            return -1;
        }
        strObj = argv[1];
    }
    else {
        strObj = argv[0];
    }

    wdata = Jim_GetString(strObj, &wlen);
    if (af->fops->writer(af, wdata, wlen) == wlen) {
        if (argc == 2 || af->fops->writer(af, "\n", 1) == 1) {
            return JRET(JIM_OK);
        }
    }
    JimAioSetError(interp, af->filename);
    return JRET(JIM_ERR);
}

static Retval aio_cmd_isatty(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    if (prj_funcDef(prj_isatty)) // #Unsupported #NonPortFuncFix
    {
        AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
        Jim_SetResultInt(interp, prj_isatty(prj_fileno(af->fp))); // #NonPortFuncFix
    } else {
        Jim_SetResultInt(interp, 0);
    }

    return JRET(JIM_OK);
}


static Retval aio_cmd_flush(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    if (fflush(af->fp) == EOF) {
        JimAioSetError(interp, af->filename);
        return JRET(JIM_ERR);
    }
    return JRET(JIM_OK);
}

static Retval aio_cmd_eof(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    Jim_SetResultInt(interp, feof(af->fp));
    return JRET(JIM_OK);
}

static Retval aio_cmd_close(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    if (argc == 3) {
#if defined(HAVE_SOCKETS) && defined(HAVE_SHUTDOWN) // #optionalCode #WinOff
        static const char * const options[] = { "r", "w", nullptr };
        enum { OPT_R, OPT_W, };
        int option;
        AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

        if (Jim_GetEnum(interp, argv[2], options, &option, nullptr, JIM_ERRMSG) != JRET(JIM_OK)) {
            return JRET(JIM_ERR);
        }
        if (prj_shutdown(af->fd, option == OPT_R ? SHUT_RD : SHUT_WR) == 0) { // #NonPortFuncFix #ConvFunc #prjFuncError
            return JRET(JIM_OK);
        }
        JimAioSetError(interp, nullptr);
#else
        Jim_SetResultString(interp, "async close not supported", -1); // #ErrStr
#endif
        return JRET(JIM_ERR);
    }

    return Jim_DeleteCommand(interp, Jim_String(argv[0]));
}

static Retval aio_cmd_seek(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    int orig = SEEK_SET;
    jim_wide offset;

    if (argc == 2) {
        if (Jim_CompareStringImmediate(interp, argv[1], "start"))
            orig = SEEK_SET;
        else if (Jim_CompareStringImmediate(interp, argv[1], "current"))
            orig = SEEK_CUR;
        else if (Jim_CompareStringImmediate(interp, argv[1], "end"))
            orig = SEEK_END;
        else {
            return -1;
        }
    }
    if (Jim_GetWide(interp, argv[0], &offset) != JRET(JIM_OK)) {
        return JRET(JIM_ERR);
    }
    if (compat_fseeko(af->fp, offset, orig) == -1) {
        JimAioSetError(interp, af->filename);
        return JRET(JIM_ERR);
    }
    return JRET(JIM_OK);
}

static Retval aio_cmd_tell(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    Jim_SetResultInt(interp, compat_ftello(af->fp));
    return JRET(JIM_OK);
}

static Retval aio_cmd_filename(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    Jim_SetResult(interp, af->filename);
    return JRET(JIM_OK);
}

#ifdef O_NDELAY // #optionalCode #WinOff
static Retval aio_cmd_ndelay(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    int fmode = prj_fcntl(af->fd, F_GETFL); // #NonPortFuncFix 

    if (argc) {
        long nb;

        if (Jim_GetLong(interp, argv[0], &nb) != JRET(JIM_OK)) {
            return JRET(JIM_ERR);
        }
        if (nb) {
            fmode |= O_NDELAY;
        }
        else {
            fmode &= ~O_NDELAY;
        }
        (void)prj_fcntl(af->fd, F_SETFL, fmode); // #NonPortFuncFix 
    }
    Jim_SetResultInt(interp, (fmode & O_NONBLOCK) ? 1 : 0);
    return JRET(JIM_OK);
}
#endif

static Retval aio_cmd_sync(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    fflush(af->fp);
    IGNOREPOSIXRET prj_fsync(af->fd); // #NonPortFuncFix
    return JRET(JIM_OK);
}

static Retval aio_cmd_buffering(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    static const char * const options[] = {
        "none",
        "line",
        "full",
        nullptr
    };
    enum
    {
        OPT_NONE,
        OPT_LINE,
        OPT_FULL,
    };
    int option;

    if (Jim_GetEnum(interp, argv[0], options, &option, nullptr, JIM_ERRMSG) != JRET(JIM_OK)) {
        return JRET(JIM_ERR);
    }
    switch (option) {
        case OPT_NONE:
            setvbuf(af->fp, nullptr, _IONBF, 0);
            break;
        case OPT_LINE:
            setvbuf(af->fp, nullptr, _IOLBF, BUFSIZ);
            break;
        case OPT_FULL:
            setvbuf(af->fp, nullptr, _IOFBF, BUFSIZ);
            break;
    }
    return JRET(JIM_OK);
}

#ifdef jim_ext_eventloop // #optionalCode
static void JimAioFileEventFinalizer(Jim_InterpPtr interp, void *clientData_)
{
    Jim_ObjArray* objPtrPtr = (Jim_ObjArray*)clientData_;

    Jim_DecrRefCount(interp, *objPtrPtr);
    *objPtrPtr = nullptr;
}

static int JimAioFileEventHandler(Jim_InterpPtr interp, void *clientData_, int mask_)
{
    Jim_ObjArray *objPtrPtr = (Jim_ObjArray*)clientData_;

    return Jim_EvalObjBackground(interp, *objPtrPtr);
}

static Retval aio_eventinfo(Jim_InterpPtr interp, AioFile * af, unsigned_t mask_, Jim_ObjArray *scriptHandlerObj,
    int argc, Jim_ObjConstArray argv)
{
    if (argc == 0) {
        /* Return current script */
        if (*scriptHandlerObj) {
            Jim_SetResult(interp, *scriptHandlerObj);
        }
        return JRET(JIM_OK);
    }

    if (*scriptHandlerObj) {
        /* Delete old handler */
        Jim_DeleteFileHandler(interp, af->fd, mask_);
    }

    /* Now possibly add the new script(s) */
    if (Jim_Length(argv[0]) == 0) {
        /* Empty script, so done */
        return JRET(JIM_OK);
    }

    /* A new script to add */
    Jim_IncrRefCount(argv[0]);
    *scriptHandlerObj = argv[0];

    Jim_CreateFileHandler(interp, af->fd, mask_,
        JimAioFileEventHandler, scriptHandlerObj, JimAioFileEventFinalizer);

    return JRET(JIM_OK);
}

static Retval aio_cmd_readable(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    return aio_eventinfo(interp, af, JIM_EVENT_READABLE, &af->rEvent, argc, argv);
}

static Retval aio_cmd_writable(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    return aio_eventinfo(interp, af, JIM_EVENT_WRITABLE, &af->wEvent, argc, argv);
}

static Retval aio_cmd_onexception(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    return aio_eventinfo(interp, af, JIM_EVENT_EXCEPTION, &af->eEvent, argc, argv);
}
#endif

#if defined(JIM_SSL) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff 

#endif /* JIM_BOOTSTRAP */

#if defined(HAVE_STRUCT_FLOCK) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff
static Retval aio_cmd_lock(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    struct flock fl;

    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;

    switch (prj_fcntl(af->fd, F_SETLK, &fl)) // #NonPortFuncFix
    {
        case 0:
            Jim_SetResultInt(interp, 1);
            break;
        case -1:
            if (errno == EACCES || errno == EAGAIN)
                Jim_SetResultInt(interp, 0);
            else
            {
                Jim_SetResultFormatted(interp, "lock failed: %s", // #ErrStr
                    strerror(errno));
                return JRET(JIM_ERR);
            }
            break;
        default:
            Jim_SetResultInt(interp, 0);
            break;
    }

    return JRET(JIM_OK);
}

static Retval aio_cmd_unlock(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    struct flock fl;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;

    Jim_SetResultInt(interp, prj_fcntl(af->fd, F_SETLK, &fl) == 0); // #NonPortFuncFix 
    return JRET(JIM_OK);
}
#endif /* JIM_BOOTSTRAP */

// #removedCode aio_cmd_tty() defined(HAVE_TERMIOS_H) && !defined(JIM_BOOTSTRAP)

static const jim_subcmd_type g_aio_command_table[] = { // #JimSubCmdDef
    {   "read",
        "?-nonewline? ?len?",
        aio_cmd_read,
        0,
        2,
        /* Description: Read and return bytes from the stream. To eof_ if no len_. */
    },
    {   "copyto",
        "handle ?size?",
        aio_cmd_copy,
        1,
        2,
        /* Description: Copy up to 'size_' bytes to the given filehandle, or to eof_ if no size_. */
    },
    {   "getfd",
        nullptr,
        aio_cmd_getfd,
        0,
        0,
        /* Description: Internal command_ to return the underlying file descriptor. */
    },
    {   "gets",
        "?var?",
        aio_cmd_gets,
        0,
        1,
        /* Description: Read one lineNum_ and return it or store it in the var */
    },
    {   "puts",
        "?-nonewline? str",
        aio_cmd_puts,
        1,
        2,
        /* Description: Write the string, with newline unless -nonewline */
    },
    {   "isatty",
        nullptr,
        aio_cmd_isatty,
        0,
        0,
        /* Description: Is the file descriptor a tty? */
    },
#if defined(HAVE_SOCKETS) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff #removeCmds
    {   "recvfrom",
        "len ?addrvar?",
        aio_cmd_recvfrom,
        1,
        2,
        /* Description: Receive up to 'len_' bytes on the socket. Sets 'addrvar' with receive address, if set */
    },
    {   "sendto",
        "str address",
        aio_cmd_sendto,
        2,
        2,
        /* Description: Send 'getStr' to the given address (dgram only) */
    },
    {   "accept",
        "?addrvar?",
        aio_cmd_accept,
        0,
        1,
        /* Description: Server socket only: Accept a connection and return stream */
    },
    {   "listen",
        "backlog",
        aio_cmd_listen,
        1,
        1,
        /* Description: Set the listen backlog for server socket */
    },
    {   "sockopt",
        "?opt 0|1?",
        aio_cmd_sockopt,
        0,
        2,
        /* Description: Return a dictionary of sockopts, or set the value of a sockopt */
    },
#endif /* JIM_BOOTSTRAP */
    {   "flush",
        nullptr,
        aio_cmd_flush,
        0,
        0,
        /* Description: Flush the stream */
    },
    {   "eof",
        nullptr,
        aio_cmd_eof,
        0,
        0,
        /* Description: Returns 1 if stream is at eof_ */
    },
    {   "close",
        "?r(ead)|w(rite)?",
        aio_cmd_close,
        0,
        1,
        JIM_MODFLAG_FULLARGV,
        /* Description: Closes the stream. */
    },
    {   "seek",
        "offset ?start|current|end",
        aio_cmd_seek,
        1,
        2,
        /* Description: Seeks in the stream (default 'current') */
    },
    {   "tell",
        nullptr,
        aio_cmd_tell,
        0,
        0,
        /* Description: Returns the current seek position */
    },
    {   "filename",
        nullptr,
        aio_cmd_filename,
        0,
        0,
        /* Description: Returns the original filename */
    },
#ifdef O_NDELAY // #optionalCode #WinOff
    {   "ndelay",
        "?0|1?",
        aio_cmd_ndelay,
        0,
        1,
        /* Description: Set O_NDELAY (if arg_). Returns current/new setting. */
    },
#endif
#ifdef HAVE_FSYNC // #optionalCode #WinOff
    {   "sync",
        nullptr,
        aio_cmd_sync,
        0,
        0,
        /* Description: Flush and fsync() the stream */
    },
#endif
    {   "buffering",
        "none|line|full",
        aio_cmd_buffering,
        1,
        1,
        /* Description: Sets buffering */
    },
#ifdef jim_ext_eventloop // #optionalCode
    {   "readable",
        "?readable-script?",
        aio_cmd_readable,
        0,
        1,
        /* Description: Returns script, or invoke readable-script when_ readable, {} to remove */
    },
    {   "writable",
        "?writable-script?",
        aio_cmd_writable,
        0,
        1,
        /* Description: Returns script, or invoke writable-script when_ writable, {} to remove */
    },
    {   "onexception",
        "?exception-script?",
        aio_cmd_onexception,
        0,
        1,
        /* Description: Returns script, or invoke exception-script when_ oob data_, {} to remove */
    },
#endif
#if !defined(JIM_BOOTSTRAP) // #optionalCode 
#if defined(JIM_SSL) // #optionalCode #WinOff
    {   "ssl",
        "?-server cert priv?",
        aio_cmd_ssl,
        0,
        3,
        JIM_MODFLAG_FULLARGV
        /* Description: Wraps a stream socket with SSL/TLS and returns a new channel */
    },
    {   "verify",
        nullptr,
        aio_cmd_verify,
        0,
        0,
        /* Description: Verifies the certificate of a SSL/TLS channel */
    },
#endif
#if defined(HAVE_STRUCT_FLOCK) // #optionalCode #WinOff
    {   "lock",
        nullptr,
        aio_cmd_lock,
        0,
        0,
        /* Description: Attempt to get a lock. */
    },
    {   "unlock",
        nullptr,
        aio_cmd_unlock,
        0,
        0,
        /* Description: Relase a lock. */
    },
#endif
    // #removedCode aio_cmd_tty() defined(HAVE_TERMIOS_H) && !defined(JIM_BOOTSTRAP)
#endif /* JIM_BOOTSTRAP */
    {  }
};

static int JimAioSubCmdProc(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    return Jim_CallSubCmd(interp, Jim_ParseSubCmd(interp, g_aio_command_table, argc, argv), argc, argv);
}

static int JimAioOpenCommand(Jim_InterpPtr interp, int argc,
        Jim_ObjConstArray argv)
{
    const char *mode;

    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "filename ?mode?"); // #ErrStr
        return JRET(JIM_ERR);
    }

    mode = (argc == 3) ? Jim_String(argv[2]) : "r";

#ifdef jim_ext_tclcompat // #optionalCode
    {
        const char *filename = Jim_String(argv[1]);

        /* If the filename starts with '|', use popen instead */
        if (*filename == '|') {
            Jim_ObjPtr evalObj[3];

            evalObj[0] = Jim_NewStringObj(interp, "::popen", -1);
            evalObj[1] = Jim_NewStringObj(interp, filename + 1, -1);
            evalObj[2] = Jim_NewStringObj(interp, mode, -1);

            return Jim_EvalObjVector(interp, 3, evalObj);
        }
    }
#endif
    return JimMakeChannel(interp, nullptr, -1, argv[1], "aio.handle%ld", 0, mode) ? JRET(JIM_OK) : JRET(JIM_ERR);
}

#if defined(JIM_SSL) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff
static void JimAioSslContextDelProc(Jim_InterpPtr interp, void *privData)
{
    SSL_CTX_free((SSL_CTX *)privData);
    ERR_free_strings();
}

#ifdef USE_TLSv1_2_method // #optionalCode #WinOff
#define TLS_method TLSv1_2_method
#endif

static SSL_CTX *JimAioSslCtx(Jim_InterpPtr interp)
{
    SSL_CTX *ssl_ctx = (SSL_CTX *)Jim_GetAssocData(interp, "ssl_ctx");
    if (ssl_ctx == nullptr) {
        SSL_load_error_strings();
        SSL_library_init();
        ssl_ctx = SSL_CTX_new(TLS_method());
        if (ssl_ctx && SSL_CTX_set_default_verify_paths(ssl_ctx)) {
            SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, nullptr);
            Jim_SetAssocData(interp, "ssl_ctx", JimAioSslContextDelProc, ssl_ctx);
        } else {
            Jim_SetResultString(interp, ERR_error_string(ERR_get_error(), nullptr), -1);
        }
    }
    return ssl_ctx;
}
#endif /* JIM_BOOTSTRAP */

/**
 * Creates a channel for fh/fd/filename.
 *
 * If fh is not nullptr, uses that as the channel (and sets AIO_KEEPOPEN).
 * Otherwise, if fd is >= 0, uses that as the channel.
 * Otherwise opens 'filename' with mode 'mode'.
 *
 * hdlfmt is a sprintf format for the filehandle. Anything with %ld at the end will do.
 * mode is used for open or fdopen.
 *
 * Creates the command_ and sets the name_ as the current result.
 * Returns the AioFile pointer on sucess or nullptr on failure.
 */
static AioFile *JimMakeChannel(Jim_InterpPtr interp, FILE *fh, int fd, Jim_ObjPtr filename,
    const char *hdlfmt, int family, const char *mode)
{
    AioFile *af;
    char buf[AIO_CMD_LEN];
    int openFlags = 0;
    Retval ret = JIM_ERR;

    snprintf(buf, sizeof(buf), hdlfmt, Jim_GetId(interp));

    if (fh) {
        openFlags = AIO_KEEPOPEN;
    }

    snprintf(buf, sizeof(buf), hdlfmt, Jim_GetId(interp));
    if (!filename) {
        filename = Jim_NewStringObj(interp, buf, -1);
    }

    Jim_IncrRefCount(filename);

    if (fh == nullptr) {
        if (fd >= 0) {
#ifndef JIM_ANSIC // #optionalCode #WinOff
            fh = prj_fdopen(fd, mode); // #NonPortFuncFix 
#endif
        }
        else
            fh = fopen(Jim_String(filename), mode);

        if (fh == nullptr) {
            JimAioSetError(interp, filename);
#ifndef JIM_ANSIC // #optionalCode #WinOff
            if (fd >= 0) {
                prj_close(fd); // #NonPortFuncFix
            }
#endif
            Jim_DecrRefCount(interp, filename);
            return nullptr;
        }
    }

    /* Create the file command_ */
    af = new_AioFile; // #AllocF 
    //memset(af, 0, sizeof(*af));
    af->fp = fh;
    af->filename = filename;
    af->openFlags = openFlags; 
#ifndef JIM_ANSIC // #optionalCode #WinOff
    af->fd = prj_fileno(fh);
#ifdef FD_CLOEXEC // #optionalCode #WinOff
    if ((openFlags & AIO_KEEPOPEN) == 0) {
        (void)prj_fcntl(af->fd, F_SETFD, FD_CLOEXEC); // #NonPortFuncFix
    }
#endif
#endif
    af->addr_family = family;
    af->fops = &g_stdio_fops;
    af->ssl = nullptr;

    ret = Jim_CreateCommand(interp, buf, JimAioSubCmdProc, af, JimAioDelProc);
    if (ret != JIM_OK) return nullptr;

    /* Note that the command_ must use the global namespace, even if
     * the current namespace is something different
     */
    Jim_SetResult(interp, Jim_MakeGlobalNamespaceName(interp, Jim_NewStringObj(interp, buf, -1)));

    return af;
}

#if defined(HAVE_PIPE) || (defined(HAVE_SOCKETPAIR) && defined(HAVE_SYS_UN_H)) // #optionalCode #WinOff
/**
 * Create a pair of channels. e.g. from pipe() or socketpair()
 */
static int JimMakeChannelPair(Jim_InterpPtr interp, int p[2], Jim_ObjPtr filename,
    const char *hdlfmt, int family, const char *mode[2])
{
    if (JimMakeChannel(interp, nullptr, p[0], filename, hdlfmt, family, mode[0])) {
        Jim_ObjPtr objPtr = Jim_NewListObj(interp, nullptr, 0);
        Jim_ListAppendElement(interp, objPtr, Jim_GetResult(interp));
        if (JimMakeChannel(interp, nullptr, p[1], filename, hdlfmt, family, mode[1])) {
            Jim_ListAppendElement(interp, objPtr, Jim_GetResult(interp));
            Jim_SetResult(interp, objPtr);
            return JRET(JIM_OK);
        }
    }

    /* Can only be here if fdopen() failed */
    prj_close(p[0]); // #NonPortFuncFix
    prj_close(p[1]); // #NonPortFuncFix
    JimAioSetError(interp, nullptr);
    return JRET(JIM_ERR);
}
#endif

#ifdef HAVE_PIPE // #optionalCode #WinOff
static int JimAioPipeCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int p[2];
    static const char *mode[2] = { "r", "w" };

    if (argc != 1) {
        Jim_WrongNumArgs(interp, 1, argv, "");
        return JRET(JIM_ERR);
    }

    if (prj_pipe(p) != 0) { // #NonPortFuncFix
        JimAioSetError(interp, nullptr);
        return JRET(JIM_ERR);
    }

    return JimMakeChannelPair(interp, p, argv[0], "aio.pipe%ld", 0, mode);
}
#endif

#if defined(HAVE_SOCKETS) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff

static int JimAioSockCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    const char *hdlfmt = "aio.unknown%ld";
    const char *socktypes[] = {
        "unix",
        "unix.server",
        "dgram",
        "dgram.server",
        "stream",
        "stream.server",
        "pipe",
        "pair",
        nullptr
    };
    enum
    {
        SOCK_UNIX,
        SOCK_UNIX_SERVER,
        SOCK_DGRAM_CLIENT,
        SOCK_DGRAM_SERVER,
        SOCK_STREAM_CLIENT,
        SOCK_STREAM_SERVER,
        SOCK_STREAM_PIPE,
        SOCK_STREAM_SOCKETPAIR,
    };
    int socktype;
    int sock;
    const char *hostportarg = nullptr;
    int res;
    int on = 1;
    const char *mode = "r+";
    int family = PF_INET;
    Jim_ObjPtr argv0 = argv[0];
    int ipv6 = 0;

    if (argc > 1 && Jim_CompareStringImmediate(interp, argv[1], "-ipv6")) {
        if (!IPV6) {
            Jim_SetResultString(interp, "ipv6 not supported", -1); // #ErrStr
            return JRET(JIM_ERR);
        }
        ipv6 = 1;
        family = PF_INET6;
    }
    argc -= ipv6;
    argv += ipv6;

    if (argc < 2) {
      wrongargs:
        Jim_WrongNumArgs(interp, 1, &argv0, "?-ipv6? type ?address?"); // #ErrStr
        return JRET(JIM_ERR);
    }

    if (Jim_GetEnum(interp, argv[1], socktypes, &socktype, "socket type", JIM_ERRMSG) != JRET(JIM_OK))
        return Jim_CheckShowCommands(interp, argv[1], socktypes);

    Jim_SetEmptyResult(interp);

    hdlfmt = "aio.sock%ld";

    if (argc > 2) {
        hostportarg = Jim_String(argv[2]);
    }

    switch (socktype) {
        case SOCK_DGRAM_CLIENT:
            if (argc == 2) {
                /* No address, so an unconnected dgram socket */
                sock = prj_socket(family, SOCK_DGRAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, nullptr);
                    return JRET(JIM_ERR);
                }
                break;
            }
            /* fall through */
        case SOCK_STREAM_CLIENT:
            {
                union sockaddr_any sa;
                int salen;

                if (argc != 3) {
                    goto wrongargs;
                }

                if (ipv6) {
                    if (JimParseIPv6Address(interp, hostportarg, &sa, &salen) != JRET(JIM_OK)) {
                        return JRET(JIM_ERR);
                    }
                }
                else if (JimParseIpAddress(interp, hostportarg, &sa, &salen) != JRET(JIM_OK)) {
                    return JRET(JIM_ERR);
                }
                sock = prj_socket(family, (socktype == SOCK_DGRAM_CLIENT) ? SOCK_DGRAM : SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, nullptr);
                    return JRET(JIM_ERR);
                }
                res = prj_connect(sock, &sa.sa, salen); // #NonPortFuncFix #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JRET(JIM_ERR);
                }
            }
            break;

        case SOCK_STREAM_SERVER:
        case SOCK_DGRAM_SERVER:
            {
                union sockaddr_any sa;
                int salen;

                if (argc != 3) {
                    goto wrongargs;
                }

                if (ipv6) {
                    if (JimParseIPv6Address(interp, hostportarg, &sa, &salen) != JRET(JIM_OK)) {
                        return JRET(JIM_ERR);
                    }
                }
                else if (JimParseIpAddress(interp, hostportarg, &sa, &salen) != JRET(JIM_OK)) {
                    return JRET(JIM_ERR);
                }
                sock = prj_socket(family, (socktype == SOCK_DGRAM_SERVER) ? SOCK_DGRAM : SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, nullptr);
                    return JRET(JIM_ERR);
                }

                /* Enable address reuse */
                prj_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); // #NonPortFuncFix #SockFunc

                res = bind(sock, &sa.sa, salen); // #NonPortFunc #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JRET(JIM_ERR);
                }
                if (socktype == SOCK_STREAM_SERVER) {
                    res = prj_listen(sock, 5); // #NonPortFuncFix #SockFunc
                    if (res) {
                        JimAioSetError(interp, nullptr);
                        prj_close(sock); // #NonPortFuncFix
                        return JRET(JIM_ERR);
                    }
                }
                hdlfmt = "aio.socksrv%ld";
            }
            break;

#ifdef HAVE_SYS_UN_H // #optionalCode #WinOff
        case SOCK_UNIX:
            {
                struct sockaddr_un sa;
                socklen_t len_;

                if (argc != 3 || ipv6) {
                    goto wrongargs;
                }

                if (JimParseDomainAddress(interp, hostportarg, &sa) != JRET(JIM_OK)) {
                    JimAioSetError(interp, argv[2]);
                    return JRET(JIM_ERR);
                }
                family = PF_UNIX;
                sock = prj_socket(PF_UNIX, SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, nullptr);
                    return JRET(JIM_ERR);
                }
                len_ = strlen(sa.sun_path) + 1 + sizeof(sa.sun_family);
                res = prj_connect(sock, (struct sockaddr *)&sa, len_); // #NonPortFuncFix #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JRET(JIM_ERR);
                }
                hdlfmt = "aio.sockunix%ld";
                break;
            }

        case SOCK_UNIX_SERVER:
            {
                struct sockaddr_un sa;
                socklen_t len_;

                if (argc != 3 || ipv6) {
                    goto wrongargs;
                }

                if (JimParseDomainAddress(interp, hostportarg, &sa) != JRET(JIM_OK)) {
                    JimAioSetError(interp, argv[2]);
                    return JRET(JIM_ERR);
                }
                family = PF_UNIX;
                sock = prj_socket(PF_UNIX, SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, nullptr);
                    return JRET(JIM_ERR);
                }
                len_ = strlen(sa.sun_path) + 1 + sizeof(sa.sun_family);
                res = bind(sock, (struct sockaddr *)&sa, len_); // #NonPortFunc #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JRET(JIM_ERR);
                }
                res = prj_listen(sock, 5); // #NonPortFuncFix #SockFunc
                if (res) {
                    JimAioSetError(interp, nullptr);
                    prj_close(sock); // #NonPortFuncFix
                    return JRET(JIM_ERR);
                }
                hdlfmt = "aio.sockunixsrv%ld";
                break;
            }
#endif

#if defined(HAVE_SOCKETPAIR) && defined(HAVE_SYS_UN_H) // #optionalCode #WinOff
        case SOCK_STREAM_SOCKETPAIR:
            {
                int p[2];
                static const char *mode[2] = { "r+", "r+" };

                if (argc != 2 || ipv6) {
                    goto wrongargs;
                }

                if (prj_socketpair(PF_UNIX, SOCK_STREAM, 0, p) < 0) { // #NonPortFuncFix #SockFunc
                    JimAioSetError(interp, nullptr);
                    return JRET(JIM_ERR);
                }
                return JimMakeChannelPair(interp, p, argv[1], "aio.sockpair%ld", PF_UNIX, mode);
            }
            break;
#endif

#if defined(HAVE_PIPE) // #optionalCode #WinOff
        case SOCK_STREAM_PIPE:
            if (argc != 2 || ipv6) {
                goto wrongargs;
            }
            return JimAioPipeCommand(interp, 1, &argv[1]);
#endif

        default:
            Jim_SetResultString(interp, "Unsupported socket type", -1); // #ErrStr
            return JRET(JIM_ERR);
    }

    return JimMakeChannel(interp, nullptr, sock, argv[1], hdlfmt, family, mode) ? JRET(JIM_OK) : JRET(JIM_ERR);
}
#endif /* JIM_BOOTSTRAP */

#if defined(JIM_SSL) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff
static int JimAioLoadSSLCertsCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    SSL_CTX *ssl_ctx;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "dir"); // #ErrStr
        return JRET(JIM_ERR);
    }

    ssl_ctx = JimAioSslCtx(interp);
    if (!ssl_ctx) {
        return JRET(JIM_ERR);
    }
    if (SSL_CTX_load_verify_locations(ssl_ctx, nullptr, Jim_String(argv[1])) == 1) {
        return JRET(JIM_OK);
    }
    Jim_SetResultString(interp, ERR_error_string(ERR_get_error(), nullptr), -1);
    return JRET(JIM_ERR);
}
#endif /* JIM_BOOTSTRAP */

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-aio-ext-version.h>

JIM_EXPORT Retval Jim_aioInit(Jim_InterpPtr interp) // #JimCmdInit
{
    Retval ret = JIM_ERR;
    if (Jim_PackageProvide(interp, "aio", version, JIM_ERRMSG))
        return JRET(JIM_ERR);

#if defined(JIM_SSL) // #optionalCode #WinOff
    Jim_CreateCommand(interp, "load_ssl_certs", JimAioLoadSSLCertsCommand, nullptr, nullptr);
#endif

    ret = Jim_CreateCommand(interp, "open", JimAioOpenCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;
#ifdef HAVE_SOCKETS // #optionalCode #WinOff
    ret = Jim_CreateCommand(interp, "socket", JimAioSockCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;
#endif
#ifdef HAVE_PIPE // #optionalCode #WinOff
    ret = Jim_CreateCommand(interp, "pipe", JimAioPipeCommand, nullptr, nullptr);
    if (ret != JIM_OK) return ret;
#endif

    /* Create filehandles for stdin, stdout and stderr */
    JimMakeChannel(interp, stdin, -1, nullptr, "stdin", 0, "r");
    JimMakeChannel(interp, stdout, -1, nullptr, "stdout", 0, "w");
    JimMakeChannel(interp, stderr, -1, nullptr, "stderr", 0, "w");

    return JRET(JIM_OK);
}

END_JIM_NAMESPACE

#endif // #if jim_ext_aio
