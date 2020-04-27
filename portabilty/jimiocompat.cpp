#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <string.h>
#include <jim.h> // # TODO convert to <jim-api.h>
#include "jimiocompat.h"
#include <prj_compat.h>

BEGIN_JIM_NAMESPACE

void Jim_SetResultErrno(Jim_Interp *interp, const char *msg)
{
    Jim_SetResultFormatted(interp, "%s: %s", msg, strerror(Jim_Errno()));
}

#if defined(__MINGW32__) || defined(_MSC_VER) // #optionalCode
#include <Windows.h> // #NonPortHeader
#include <sys/stat.h> // #NonPortHeader

int Jim_Errno(void)
{
    switch (GetLastError()) {
    case ERROR_FILE_NOT_FOUND: return ENOENT;
    case ERROR_PATH_NOT_FOUND: return ENOENT;
    case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
    case ERROR_ACCESS_DENIED: return EACCES;
    case ERROR_INVALID_HANDLE: return EBADF;
    case ERROR_BAD_ENVIRONMENT: return E2BIG;
    case ERROR_BAD_FORMAT: return ENOEXEC;
    case ERROR_INVALID_ACCESS: return EACCES;
    case ERROR_INVALID_DRIVE: return ENOENT;
    case ERROR_CURRENT_DIRECTORY: return EACCES;
    case ERROR_NOT_SAME_DEVICE: return EXDEV;
    case ERROR_NO_MORE_FILES: return ENOENT;
    case ERROR_WRITE_PROTECT: return EROFS;
    case ERROR_BAD_UNIT: return ENXIO;
    case ERROR_NOT_READY: return EBUSY;
    case ERROR_BAD_COMMAND: return EIO;
    case ERROR_CRC: return EIO;
    case ERROR_BAD_LENGTH: return EIO;
    case ERROR_SEEK: return EIO;
    case ERROR_WRITE_FAULT: return EIO;
    case ERROR_READ_FAULT: return EIO;
    case ERROR_GEN_FAILURE: return EIO;
    case ERROR_SHARING_VIOLATION: return EACCES;
    case ERROR_LOCK_VIOLATION: return EACCES;
    case ERROR_SHARING_BUFFER_EXCEEDED: return ENFILE;
    case ERROR_HANDLE_DISK_FULL: return ENOSPC;
    case ERROR_NOT_SUPPORTED: return ENODEV;
    case ERROR_REM_NOT_LIST: return EBUSY;
    case ERROR_DUP_NAME: return EEXIST;
    case ERROR_BAD_NETPATH: return ENOENT;
    case ERROR_NETWORK_BUSY: return EBUSY;
    case ERROR_DEV_NOT_EXIST: return ENODEV;
    case ERROR_TOO_MANY_CMDS: return EAGAIN;
    case ERROR_ADAP_HDW_ERR: return EIO;
    case ERROR_BAD_NET_RESP: return EIO;
    case ERROR_UNEXP_NET_ERR: return EIO;
    case ERROR_NETNAME_DELETED: return ENOENT;
    case ERROR_NETWORK_ACCESS_DENIED: return EACCES;
    case ERROR_BAD_DEV_TYPE: return ENODEV;
    case ERROR_BAD_NET_NAME: return ENOENT;
    case ERROR_TOO_MANY_NAMES: return ENFILE;
    case ERROR_TOO_MANY_SESS: return EIO;
    case ERROR_SHARING_PAUSED: return EAGAIN;
    case ERROR_REDIR_PAUSED: return EAGAIN;
    case ERROR_FILE_EXISTS: return EEXIST;
    case ERROR_CANNOT_MAKE: return ENOSPC;
    case ERROR_OUT_OF_STRUCTURES: return ENFILE;
    case ERROR_ALREADY_ASSIGNED: return EEXIST;
    case ERROR_INVALID_PASSWORD: return EPERM;
    case ERROR_NET_WRITE_FAULT: return EIO;
    case ERROR_NO_PROC_SLOTS: return EAGAIN;
    case ERROR_DISK_CHANGE: return EXDEV;
    case ERROR_BROKEN_PIPE: return EPIPE;
    case ERROR_OPEN_FAILED: return ENOENT;
    case ERROR_DISK_FULL: return ENOSPC;
    case ERROR_NO_MORE_SEARCH_HANDLES: return EMFILE;
    case ERROR_INVALID_TARGET_HANDLE: return EBADF;
    case ERROR_INVALID_NAME: return ENOENT;
    case ERROR_PROC_NOT_FOUND: return ESRCH;
    case ERROR_WAIT_NO_CHILDREN: return ECHILD;
    case ERROR_CHILD_NOT_COMPLETE: return ECHILD;
    case ERROR_DIRECT_ACCESS_HANDLE: return EBADF;
    case ERROR_SEEK_ON_DEVICE: return ESPIPE;
    case ERROR_BUSY_DRIVE: return EAGAIN;
    case ERROR_DIR_NOT_EMPTY: return EEXIST;
    case ERROR_NOT_LOCKED: return EACCES;
    case ERROR_BAD_PATHNAME: return ENOENT;
    case ERROR_LOCK_FAILED: return EACCES;
    case ERROR_ALREADY_EXISTS: return EEXIST;
    case ERROR_FILENAME_EXCED_RANGE: return ENAMETOOLONG;
    case ERROR_BAD_PIPE: return EPIPE;
    case ERROR_PIPE_BUSY: return EAGAIN;
    case ERROR_PIPE_NOT_CONNECTED: return EPIPE;
    case ERROR_DIRECTORY: return ENOTDIR;
    }
    return EINVAL;
}


int Jim_MakeTempFile(Jim_Interp *interp, const char *filename_template, int unlink_file)
{
    char name[MAX_PATH];
    HANDLE handle;

    if (!GetTempPathA(MAX_PATH, name) || !GetTempFileNameA(name, filename_template ? filename_template : "JIM", 0, name)) {
        return -1;
    }

    handle = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | (unlink_file ? FILE_FLAG_DELETE_ON_CLOSE : 0),
            NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    Jim_SetResultString(interp, name, -1);
    return _open_osfhandle((int)handle, _O_RDWR | _O_TEXT);

  error:
    Jim_SetResultErrno(interp, name);
    DeleteFileA(name);
    return -1;
}

int Jim_OpenForWrite(const char *filename, int append)
{
    if (strcmp(filename, "/dev/null") == 0) {
        filename = "nul:";
    }
    int fd = _open(filename, _O_WRONLY | _O_CREAT | _O_TEXT | (append ? _O_APPEND : _O_TRUNC), _S_IREAD | _S_IWRITE);
    if (fd >= 0 && append) {
        /* Note that _O_APPEND doesn't actually work. need to do it manually */
        _lseek(fd, 0L, SEEK_END);
    }
    return fd;
}

int Jim_OpenForRead(const char *filename)
{
    if (strcmp(filename, "/dev/null") == 0) {
        filename = "nul:";
    }
    return _open(filename, _O_RDONLY | _O_TEXT, 0);
}

#elif defined(HAVE_UNISTD_H) // #optionalCode #WinOff

/* Unix-specific implementation */

int Jim_MakeTempFile(Jim_Interp *interp, const char *filename_template, int unlink_file)
{
    int fd;
    mode_t mask;
    Jim_Obj *filenameObj;

    if (filename_template == NULL) {
        const char *tmpdir = prj_getenv("TMPDIR"); // #NonPortFuncFix
        if (tmpdir == NULL || *tmpdir == '\0' || access(tmpdir, W_OK) != 0) {
            tmpdir = "/tmp/";
        }
        filenameObj = Jim_NewStringObj(interp, tmpdir, -1);
        if (tmpdir[0] && tmpdir[strlen(tmpdir) - 1] != '/') {
            Jim_AppendString(interp, filenameObj, "/", 1);
        }
        Jim_AppendString(interp, filenameObj, "tcl.tmp.XXXXXX", -1);
    }
    else {
        filenameObj = Jim_NewStringObj(interp, filename_template, -1);
    }

    /* Update the template name directly with the filename */
    mask = prj_umask(S_IXUSR | S_IRWXG | S_IRWXO); // #NonPortFuncFix 
#ifdef HAVE_MKSTEMP // #optionalCode #WinOff
    fd = prj_mkstemp(filenameObj->bytes_); // #NonPortFuncFix
#else
    if (mktemp(filenameObj->bytes_) == NULL) {
        fd = -1;
    }
    else {
        fd = prj_open(filenameObj->bytes_, O_RDWR | O_CREAT | O_TRUNC); // #NonPortFuncFix
    }
#endif
    prj_umask(mask); // #NonPortFuncFix
    if (fd < 0) {
        Jim_SetResultErrno(interp, Jim_String(filenameObj));
        Jim_FreeNewObj(interp, filenameObj);
        return -1;
    }
    if (unlink_file) {
        remove(Jim_String(filenameObj)); // #NonPortFunc
    }

    Jim_SetResult(interp, filenameObj);
    return fd;
}

int Jim_OpenForWrite(const char *filename, int append)
{
    return prj_open(filename, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0666); // #NonPortFuncFix
}

int Jim_OpenForRead(const char *filename)
{
    return prj_open(filename, O_RDONLY, 0); // #NonPortFuncFix
}

#endif

END_JIM_NAMESPACE
