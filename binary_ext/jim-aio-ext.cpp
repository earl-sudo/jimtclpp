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
    AIO_CMD_LEN = 32,      /* e.g. aio.handleXXXXXX */
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
        return JIM_OK;
    }
    clearerr(af->fp);
    /* EAGAIN and similar are not error conditions. Just treat them like eof */
    if (feof(af->fp) || errno == EAGAIN || errno == EINTR) {
        return JIM_OK;
    }
#ifdef ECONNRESET // #optionalCode
    if (errno == ECONNRESET) {
        return JIM_OK;
    }
#endif
#ifdef ECONNABORTED // #optionalCode
    if (errno == ECONNABORTED) {
        return JIM_OK;
    }
#endif
    return JIM_ERR;
}

const char *stdio_strerror(struct AioFile *af)
{
    return strerror(errno);
}

static const JimAioFopsType g_stdio_fops = {
    stdio_writer,
    stdio_reader,
    stdio_getline,
    stdio_error,
    stdio_strerror,
    NULL
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
    if (af->ssl != NULL) {
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
        if (Jim_GetWide(interp, argv[0], &neededLen) != JIM_OK)
            return JIM_ERR;
        if (neededLen < 0) {
            Jim_SetResultString(interp, "invalid parameter: negative len", -1); // #ErrStr
            return JIM_ERR;
        }
    }
    else if (argc) {
        return -1;
    }
    objPtr = Jim_NewStringObj(interp, NULL, 0);
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
    /* Check for error conditions */
    if (JimCheckStreamError(interp, af)) {
        Jim_FreeNewObj(interp, objPtr);
        return JIM_ERR;
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
    return JIM_OK;
}

AioFile *Jim_AioFile(Jim_InterpPtr interp, Jim_ObjPtr command)
{
    Jim_Cmd *cmdPtr = Jim_GetCommand(interp, command, JIM_ERRMSG);

    /* XXX: There ought to be a supported API for this */
    if (cmdPtr && !cmdPtr->isproc() && cmdPtr->cmdProc() == JimAioSubCmdProc) {
        return cmdPtr->getPrivData<AioFile*>();
    }
    Jim_SetResultFormatted(interp, "Not a filehandle: \"%#s\"", command); // #ErrStr
    return NULL;
}

JIM_EXPORT FILE *Jim_AioFilehandle(Jim_InterpPtr interp, Jim_ObjPtr command)
{
    AioFile *af;

    af = Jim_AioFile(interp, command);
    if (af == NULL) {
        return NULL;
    }

    return af->fp;
}

static Retval aio_cmd_getfd(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    fflush(af->fp);
    Jim_SetResultInt(interp, prj_fileno(af->fp)); // #NonPortFuncFix

    return JIM_OK;
}

static Retval aio_cmd_copy(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    jim_wide count = 0;
    jim_wide maxlen = JIM_WIDE_MAX;
    AioFile *outf = Jim_AioFile(interp, argv[0]);

    if (outf == NULL) {
        return JIM_ERR;
    }

    if (argc == 2) {
        if (Jim_GetWide(interp, argv[1], &maxlen) != JIM_OK) {
            return JIM_ERR;
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
        return JIM_ERR;
    }

    Jim_SetResultInt(interp, count);

    return JIM_OK;
}

static Retval aio_cmd_gets(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
    char buf[AIO_BUF_LEN];
    Jim_ObjPtr objPtr;
    int len;

    errno = 0;

    objPtr = Jim_NewStringObj(interp, NULL, 0);
    while (1) {
        buf[AIO_BUF_LEN - 1] = '_';

        if (af->fops->getline(af, buf, AIO_BUF_LEN) == NULL)
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
        /* I/O error */
        Jim_FreeNewObj(interp, objPtr);
        return JIM_ERR;
    }

    if (argc) {
        if (Jim_SetVariable(interp, argv[0], objPtr) != JIM_OK) {
            Jim_FreeNewObj(interp, objPtr);
            return JIM_ERR;
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
    return JIM_OK;
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
            return JIM_OK;
        }
    }
    JimAioSetError(interp, af->filename);
    return JIM_ERR;
}

static Retval aio_cmd_isatty(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    if (prj_funcDef(prj_isatty)) // #Unsupported #NonPortFuncFix
    {
        AioFile *af = (AioFile*)Jim_CmdPrivData(interp);
        Jim_SetResultInt(interp, prj_isatty(prj_fileno(af->fp))); // #NonPortFuncFix
    } else {
        Jim_SetResultInt(interp, 0);
    }

    return JIM_OK;
}


static Retval aio_cmd_flush(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    if (fflush(af->fp) == EOF) {
        JimAioSetError(interp, af->filename);
        return JIM_ERR;
    }
    return JIM_OK;
}

static Retval aio_cmd_eof(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    Jim_SetResultInt(interp, feof(af->fp));
    return JIM_OK;
}

static Retval aio_cmd_close(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    if (argc == 3) {
#if defined(HAVE_SOCKETS) && defined(HAVE_SHUTDOWN) // #optionalCode #WinOff
        static const char * const options[] = { "r", "w", NULL };
        enum { OPT_R, OPT_W, };
        int option;
        AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

        if (Jim_GetEnum(interp, argv[2], options, &option, NULL, JIM_ERRMSG) != JIM_OK) {
            return JIM_ERR;
        }
        if (prj_shutdown(af->fd, option == OPT_R ? SHUT_RD : SHUT_WR) == 0) { // #NonPortFuncFix #ConvFunc #prjFuncError
            return JIM_OK;
        }
        JimAioSetError(interp, NULL);
#else
        Jim_SetResultString(interp, "async close not supported", -1); // #ErrStr
#endif
        return JIM_ERR;
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
    if (Jim_GetWide(interp, argv[0], &offset) != JIM_OK) {
        return JIM_ERR;
    }
    if (compat_fseeko(af->fp, offset, orig) == -1) {
        JimAioSetError(interp, af->filename);
        return JIM_ERR;
    }
    return JIM_OK;
}

static Retval aio_cmd_tell(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    Jim_SetResultInt(interp, compat_ftello(af->fp));
    return JIM_OK;
}

static Retval aio_cmd_filename(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    Jim_SetResult(interp, af->filename);
    return JIM_OK;
}

#ifdef O_NDELAY // #optionalCode #WinOff
static Retval aio_cmd_ndelay(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #PosixCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    int fmode = prj_fcntl(af->fd, F_GETFL); // #NonPortFuncFix 

    if (argc) {
        long nb;

        if (Jim_GetLong(interp, argv[0], &nb) != JIM_OK) {
            return JIM_ERR;
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
    return JIM_OK;
}
#endif

static Retval aio_cmd_sync(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    fflush(af->fp);
    prj_fsync(af->fd); // #NonPortFuncFix
    return JIM_OK;
}

static Retval aio_cmd_buffering(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    AioFile *af = (AioFile*)Jim_CmdPrivData(interp);

    static const char * const options[] = {
        "none",
        "line",
        "full",
        NULL
    };
    enum
    {
        OPT_NONE,
        OPT_LINE,
        OPT_FULL,
    };
    int option;

    if (Jim_GetEnum(interp, argv[0], options, &option, NULL, JIM_ERRMSG) != JIM_OK) {
        return JIM_ERR;
    }
    switch (option) {
        case OPT_NONE:
            setvbuf(af->fp, NULL, _IONBF, 0);
            break;
        case OPT_LINE:
            setvbuf(af->fp, NULL, _IOLBF, BUFSIZ);
            break;
        case OPT_FULL:
            setvbuf(af->fp, NULL, _IOFBF, BUFSIZ);
            break;
    }
    return JIM_OK;
}

#ifdef jim_ext_eventloop // #optionalCode
static void JimAioFileEventFinalizer(Jim_InterpPtr interp, void *clientData)
{
    Jim_ObjArray* objPtrPtr = (Jim_ObjArray*)clientData;

    Jim_DecrRefCount(interp, *objPtrPtr);
    *objPtrPtr = NULL;
}

static int JimAioFileEventHandler(Jim_InterpPtr interp, void *clientData, int mask)
{
    Jim_ObjArray *objPtrPtr = (Jim_ObjArray*)clientData;

    return Jim_EvalObjBackground(interp, *objPtrPtr);
}

static Retval aio_eventinfo(Jim_InterpPtr interp, AioFile * af, unsigned_t mask, Jim_ObjArray *scriptHandlerObj,
    int argc, Jim_ObjConstArray argv)
{
    if (argc == 0) {
        /* Return current script */
        if (*scriptHandlerObj) {
            Jim_SetResult(interp, *scriptHandlerObj);
        }
        return JIM_OK;
    }

    if (*scriptHandlerObj) {
        /* Delete old handler */
        Jim_DeleteFileHandler(interp, af->fd, mask);
    }

    /* Now possibly add the new script(s) */
    if (Jim_Length(argv[0]) == 0) {
        /* Empty script, so done */
        return JIM_OK;
    }

    /* A new script to add */
    Jim_IncrRefCount(argv[0]);
    *scriptHandlerObj = argv[0];

    Jim_CreateFileHandler(interp, af->fd, mask,
        JimAioFileEventHandler, scriptHandlerObj, JimAioFileEventFinalizer);

    return JIM_OK;
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
                return JIM_ERR;
            }
            break;
        default:
            Jim_SetResultInt(interp, 0);
            break;
    }

    return JIM_OK;
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
    return JIM_OK;
}
#endif /* JIM_BOOTSTRAP */

// #removedCode aio_cmd_tty() defined(HAVE_TERMIOS_H) && !defined(JIM_BOOTSTRAP)

static const jim_subcmd_type g_aio_command_table[] = { // #JimSubCmdDef
    {   "read",
        "?-nonewline? ?len?",
        aio_cmd_read,
        0,
        2,
        /* Description: Read and return bytes from the stream. To eof if no len. */
    },
    {   "copyto",
        "handle ?size?",
        aio_cmd_copy,
        1,
        2,
        /* Description: Copy up to 'size' bytes to the given filehandle, or to eof if no size. */
    },
    {   "getfd",
        NULL,
        aio_cmd_getfd,
        0,
        0,
        /* Description: Internal command to return the underlying file descriptor. */
    },
    {   "gets",
        "?var?",
        aio_cmd_gets,
        0,
        1,
        /* Description: Read one line and return it or store it in the var */
    },
    {   "puts",
        "?-nonewline? str",
        aio_cmd_puts,
        1,
        2,
        /* Description: Write the string, with newline unless -nonewline */
    },
    {   "isatty",
        NULL,
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
        /* Description: Receive up to 'len' bytes on the socket. Sets 'addrvar' with receive address, if set */
    },
    {   "sendto",
        "str address",
        aio_cmd_sendto,
        2,
        2,
        /* Description: Send 'str' to the given address (dgram only) */
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
        NULL,
        aio_cmd_flush,
        0,
        0,
        /* Description: Flush the stream */
    },
    {   "eof",
        NULL,
        aio_cmd_eof,
        0,
        0,
        /* Description: Returns 1 if stream is at eof */
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
        NULL,
        aio_cmd_tell,
        0,
        0,
        /* Description: Returns the current seek position */
    },
    {   "filename",
        NULL,
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
        /* Description: Set O_NDELAY (if arg). Returns current/new setting. */
    },
#endif
#ifdef HAVE_FSYNC // #optionalCode #WinOff
    {   "sync",
        NULL,
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
        /* Description: Returns script, or invoke readable-script when readable, {} to remove */
    },
    {   "writable",
        "?writable-script?",
        aio_cmd_writable,
        0,
        1,
        /* Description: Returns script, or invoke writable-script when writable, {} to remove */
    },
    {   "onexception",
        "?exception-script?",
        aio_cmd_onexception,
        0,
        1,
        /* Description: Returns script, or invoke exception-script when oob data, {} to remove */
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
        NULL,
        aio_cmd_verify,
        0,
        0,
        /* Description: Verifies the certificate of a SSL/TLS channel */
    },
#endif
#if defined(HAVE_STRUCT_FLOCK) // #optionalCode #WinOff
    {   "lock",
        NULL,
        aio_cmd_lock,
        0,
        0,
        /* Description: Attempt to get a lock. */
    },
    {   "unlock",
        NULL,
        aio_cmd_unlock,
        0,
        0,
        /* Description: Relase a lock. */
    },
#endif
    // #removedCode aio_cmd_tty() defined(HAVE_TERMIOS_H) && !defined(JIM_BOOTSTRAP)
#endif /* JIM_BOOTSTRAP */
    { NULL }
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
        return JIM_ERR;
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
    return JimMakeChannel(interp, NULL, -1, argv[1], "aio.handle%ld", 0, mode) ? JIM_OK : JIM_ERR;
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
    if (ssl_ctx == NULL) {
        SSL_load_error_strings();
        SSL_library_init();
        ssl_ctx = SSL_CTX_new(TLS_method());
        if (ssl_ctx && SSL_CTX_set_default_verify_paths(ssl_ctx)) {
            SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
            Jim_SetAssocData(interp, "ssl_ctx", JimAioSslContextDelProc, ssl_ctx);
        } else {
            Jim_SetResultString(interp, ERR_error_string(ERR_get_error(), NULL), -1);
        }
    }
    return ssl_ctx;
}
#endif /* JIM_BOOTSTRAP */

/**
 * Creates a channel for fh/fd/filename.
 *
 * If fh is not NULL, uses that as the channel (and sets AIO_KEEPOPEN).
 * Otherwise, if fd is >= 0, uses that as the channel.
 * Otherwise opens 'filename' with mode 'mode'.
 *
 * hdlfmt is a sprintf format for the filehandle. Anything with %ld at the end will do.
 * mode is used for open or fdopen.
 *
 * Creates the command and sets the name as the current result.
 * Returns the AioFile pointer on sucess or NULL on failure.
 */
static AioFile *JimMakeChannel(Jim_InterpPtr interp, FILE *fh, int fd, Jim_ObjPtr filename,
    const char *hdlfmt, int family, const char *mode)
{
    AioFile *af;
    char buf[AIO_CMD_LEN];
    int openFlags = 0;

    snprintf(buf, sizeof(buf), hdlfmt, Jim_GetId(interp));

    if (fh) {
        openFlags = AIO_KEEPOPEN;
    }

    snprintf(buf, sizeof(buf), hdlfmt, Jim_GetId(interp));
    if (!filename) {
        filename = Jim_NewStringObj(interp, buf, -1);
    }

    Jim_IncrRefCount(filename);

    if (fh == NULL) {
        if (fd >= 0) {
#ifndef JIM_ANSIC // #optionalCode #WinOff
            fh = prj_fdopen(fd, mode); // #NonPortFuncFix 
#endif
        }
        else
            fh = fopen(Jim_String(filename), mode);

        if (fh == NULL) {
            JimAioSetError(interp, filename);
#ifndef JIM_ANSIC // #optionalCode #WinOff
            if (fd >= 0) {
                prj_close(fd); // #NonPortFuncFix
            }
#endif
            Jim_DecrRefCount(interp, filename);
            return NULL;
        }
    }

    /* Create the file command */
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
    af->ssl = NULL;

    Jim_CreateCommand(interp, buf, JimAioSubCmdProc, af, JimAioDelProc);

    /* Note that the command must use the global namespace, even if
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
    if (JimMakeChannel(interp, NULL, p[0], filename, hdlfmt, family, mode[0])) {
        Jim_ObjPtr objPtr = Jim_NewListObj(interp, NULL, 0);
        Jim_ListAppendElement(interp, objPtr, Jim_GetResult(interp));
        if (JimMakeChannel(interp, NULL, p[1], filename, hdlfmt, family, mode[1])) {
            Jim_ListAppendElement(interp, objPtr, Jim_GetResult(interp));
            Jim_SetResult(interp, objPtr);
            return JIM_OK;
        }
    }

    /* Can only be here if fdopen() failed */
    prj_close(p[0]); // #NonPortFuncFix
    prj_close(p[1]); // #NonPortFuncFix
    JimAioSetError(interp, NULL);
    return JIM_ERR;
}
#endif

#ifdef HAVE_PIPE // #optionalCode #WinOff
static int JimAioPipeCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int p[2];
    static const char *mode[2] = { "r", "w" };

    if (argc != 1) {
        Jim_WrongNumArgs(interp, 1, argv, "");
        return JIM_ERR;
    }

    if (prj_pipe(p) != 0) { // #NonPortFuncFix
        JimAioSetError(interp, NULL);
        return JIM_ERR;
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
        NULL
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
    const char *hostportarg = NULL;
    int res;
    int on = 1;
    const char *mode = "r+";
    int family = PF_INET;
    Jim_ObjPtr argv0 = argv[0];
    int ipv6 = 0;

    if (argc > 1 && Jim_CompareStringImmediate(interp, argv[1], "-ipv6")) {
        if (!IPV6) {
            Jim_SetResultString(interp, "ipv6 not supported", -1); // #ErrStr
            return JIM_ERR;
        }
        ipv6 = 1;
        family = PF_INET6;
    }
    argc -= ipv6;
    argv += ipv6;

    if (argc < 2) {
      wrongargs:
        Jim_WrongNumArgs(interp, 1, &argv0, "?-ipv6? type ?address?"); // #ErrStr
        return JIM_ERR;
    }

    if (Jim_GetEnum(interp, argv[1], socktypes, &socktype, "socket type", JIM_ERRMSG) != JIM_OK)
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
                    JimAioSetError(interp, NULL);
                    return JIM_ERR;
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
                    if (JimParseIPv6Address(interp, hostportarg, &sa, &salen) != JIM_OK) {
                        return JIM_ERR;
                    }
                }
                else if (JimParseIpAddress(interp, hostportarg, &sa, &salen) != JIM_OK) {
                    return JIM_ERR;
                }
                sock = prj_socket(family, (socktype == SOCK_DGRAM_CLIENT) ? SOCK_DGRAM : SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, NULL);
                    return JIM_ERR;
                }
                res = prj_connect(sock, &sa.sa, salen); // #NonPortFuncFix #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JIM_ERR;
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
                    if (JimParseIPv6Address(interp, hostportarg, &sa, &salen) != JIM_OK) {
                        return JIM_ERR;
                    }
                }
                else if (JimParseIpAddress(interp, hostportarg, &sa, &salen) != JIM_OK) {
                    return JIM_ERR;
                }
                sock = prj_socket(family, (socktype == SOCK_DGRAM_SERVER) ? SOCK_DGRAM : SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, NULL);
                    return JIM_ERR;
                }

                /* Enable address reuse */
                prj_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)); // #NonPortFuncFix #SockFunc

                res = bind(sock, &sa.sa, salen); // #NonPortFunc #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JIM_ERR;
                }
                if (socktype == SOCK_STREAM_SERVER) {
                    res = prj_listen(sock, 5); // #NonPortFuncFix #SockFunc
                    if (res) {
                        JimAioSetError(interp, NULL);
                        prj_close(sock); // #NonPortFuncFix
                        return JIM_ERR;
                    }
                }
                hdlfmt = "aio.socksrv%ld";
            }
            break;

#ifdef HAVE_SYS_UN_H // #optionalCode #WinOff
        case SOCK_UNIX:
            {
                struct sockaddr_un sa;
                socklen_t len;

                if (argc != 3 || ipv6) {
                    goto wrongargs;
                }

                if (JimParseDomainAddress(interp, hostportarg, &sa) != JIM_OK) {
                    JimAioSetError(interp, argv[2]);
                    return JIM_ERR;
                }
                family = PF_UNIX;
                sock = prj_socket(PF_UNIX, SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, NULL);
                    return JIM_ERR;
                }
                len = strlen(sa.sun_path) + 1 + sizeof(sa.sun_family);
                res = prj_connect(sock, (struct sockaddr *)&sa, len); // #NonPortFuncFix #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JIM_ERR;
                }
                hdlfmt = "aio.sockunix%ld";
                break;
            }

        case SOCK_UNIX_SERVER:
            {
                struct sockaddr_un sa;
                socklen_t len;

                if (argc != 3 || ipv6) {
                    goto wrongargs;
                }

                if (JimParseDomainAddress(interp, hostportarg, &sa) != JIM_OK) {
                    JimAioSetError(interp, argv[2]);
                    return JIM_ERR;
                }
                family = PF_UNIX;
                sock = prj_socket(PF_UNIX, SOCK_STREAM, 0); // #NonPortFuncFix #SockFunc
                if (sock < 0) {
                    JimAioSetError(interp, NULL);
                    return JIM_ERR;
                }
                len = strlen(sa.sun_path) + 1 + sizeof(sa.sun_family);
                res = bind(sock, (struct sockaddr *)&sa, len); // #NonPortFunc #SockFunc
                if (res) {
                    JimAioSetError(interp, argv[2]);
                    prj_close(sock); // #NonPortFuncFix
                    return JIM_ERR;
                }
                res = prj_listen(sock, 5); // #NonPortFuncFix #SockFunc
                if (res) {
                    JimAioSetError(interp, NULL);
                    prj_close(sock); // #NonPortFuncFix
                    return JIM_ERR;
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
                    JimAioSetError(interp, NULL);
                    return JIM_ERR;
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
            return JIM_ERR;
    }

    return JimMakeChannel(interp, NULL, sock, argv[1], hdlfmt, family, mode) ? JIM_OK : JIM_ERR;
}
#endif /* JIM_BOOTSTRAP */

#if defined(JIM_SSL) && !defined(JIM_BOOTSTRAP) // #optionalCode #WinOff
static int JimAioLoadSSLCertsCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    SSL_CTX *ssl_ctx;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "dir"); // #ErrStr
        return JIM_ERR;
    }

    ssl_ctx = JimAioSslCtx(interp);
    if (!ssl_ctx) {
        return JIM_ERR;
    }
    if (SSL_CTX_load_verify_locations(ssl_ctx, NULL, Jim_String(argv[1])) == 1) {
        return JIM_OK;
    }
    Jim_SetResultString(interp, ERR_error_string(ERR_get_error(), NULL), -1);
    return JIM_ERR;
}
#endif /* JIM_BOOTSTRAP */

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-aio-ext-version.h>

Retval Jim_aioInit(Jim_InterpPtr interp) // #JimCmdInit
{
    if (Jim_PackageProvide(interp, "aio", version, JIM_ERRMSG))
        return JIM_ERR;

#if defined(JIM_SSL) // #optionalCode #WinOff
    Jim_CreateCommand(interp, "load_ssl_certs", JimAioLoadSSLCertsCommand, NULL, NULL);
#endif

    Jim_CreateCommand(interp, "open", JimAioOpenCommand, NULL, NULL);
#ifdef HAVE_SOCKETS // #optionalCode #WinOff
    Jim_CreateCommand(interp, "socket", JimAioSockCommand, NULL, NULL);
#endif
#ifdef HAVE_PIPE // #optionalCode #WinOff
    Jim_CreateCommand(interp, "pipe", JimAioPipeCommand, NULL, NULL);
#endif

    /* Create filehandles for stdin, stdout and stderr */
    JimMakeChannel(interp, stdin, -1, NULL, "stdin", 0, "r");
    JimMakeChannel(interp, stdout, -1, NULL, "stdout", 0, "w");
    JimMakeChannel(interp, stderr, -1, NULL, "stderr", 0, "w");

    return JIM_OK;
}

END_JIM_NAMESPACE

#endif // #if jim_ext_aio
