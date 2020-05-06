#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <jim-cpp-api.h>
#include <errno.h>
#include <sys/stat.h>
#include <direct.h> // #NonPortHeader
#if 0
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#include <tuple>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <string_view>
#include <variant>

#include <prj_compat.h>

#if defined(PRJ_OS_LINUX)
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#elif defined(PRJ_OS_WIN)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <io.h>
#  include <fcntl.h>
#else
#endif

BEGIN_JIM_NAMESPACE

using namespace std;

template<typename T1, typename T2>
struct val2 {
    T1 v1;
    T2 v2;
};

template<typename T1, typename T2, typename T3>
struct val3 {
    T1 v1;
    T2 v2;
    T3 v3;
};


template<typename T1, typename T2, typename T3, typename T4>
struct val4 {
    T1 v1;
    T2 v2;
    T3 v3;
    T4 v4;
};

#if 0
// Tries to take care of everything you need to do to wrap a function.
struct JimCmd {
    string cmd_;
    string description_;

    // Wrap all the checking of arguments in one object to keep JimCmd simpler.
    int16_t minNumArgs_ = -1;
    int16_t maxNumArgs_ = -1;


    JimCmd(void) : cmd_("cmdNone"), description_("arg1") { }

    Retval jimcmdCaller(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) {
        Retval ret = JIM_OK;
        try {
            JimArgs  args(interp, argc, argv);
            jimcmd(args);
            //if (!args.setResults_) args.return_();
        } catch (JimObjError& joe) {
            if (joe.code_ == JIMOBJ_ERROR_JUSTARETURN) {
            } else {
                ret = JIM_ERR;
            }
        }
        return ret;
    }
    virtual void jimcmd(JimArgs& args) { // Actual specialization.
        JimObj   path(args.arg(0));

        // args.return_(fileSize1((const char*) path));
    }
    JimCmd(const string& cmd, const string& description) : cmd_(cmd), description_(description) {
    }
};
#endif



// ======================================================

int get_errno(void) {
#ifdef PRJ_OS_WIN
    return *::_errno();
#else
    return = ::errno;
#endif
}
int& get_errno_ref(void) {
#ifdef PRJ_OS_WIN
    return *::_errno();
#else
    return = ::errno;
#endif
}

bool isWindows(void) {
#ifdef PRJ_OS_WIN
    return true;
#else
    return false;
#endif
}
struct FileStatus {
    struct stat   fileStatus_;
    int errno_ = 0;

    struct Info {
        const char* name_ = "none";
        int64_t value_ = -1;
    };
    enum FIELDS {
        ST_DEV, ST_INO, ST_MODE, ST_NLINK, ST_UID, ST_GID, ST_RDEV, ST_SIZE, ST_ATIME, ST_MTIME, ST_CTIME,
        ST_BLKSIZE, ST_BLOCKS, NUM_FIELDS
    };
    vector<Info>   values_ = {
        {"dev_t", -1},
        {"ino", -1},
        {"nlink", -1},
        {"uid", -1},
        {"gid", -1},
        {"rdev", -1},
        {"size", -1},
        {"atime", -1},
        {"mtime", -1},
        {"ctime", -1},
        {"blksize", -1},
        {"blocks", -1},
    };

    FileStatus(string_view filepath) {
        int ret = ::stat((const char*)filepath.data(), &fileStatus_);
        if (ret == -1) {
            errno_ = get_errno();

        } else {
            values_[ST_DEV].value_ = fileStatus_.st_dev;
            values_[ST_INO].value_ = fileStatus_.st_ino;
            values_[ST_NLINK].value_ = fileStatus_.st_nlink;
            values_[ST_UID].value_ = fileStatus_.st_uid;
            values_[ST_GID].value_ = fileStatus_.st_gid;
            values_[ST_RDEV].value_ = fileStatus_.st_rdev;
            values_[ST_SIZE].value_ = fileStatus_.st_size;
            values_[ST_ATIME].value_ = fileStatus_.st_atime;
            values_[ST_MTIME].value_ = fileStatus_.st_mtime;
            values_[ST_CTIME].value_ = fileStatus_.st_ctime;
#ifdef PRJ_OS_LINUX
            values_[ST_BLKSIZE].value_ = fileStatus_.st_blksize;
            values_[ST_BLOCKS].value_ = fileStatus_.st_blocks;
#endif
        }

    }
};

namespace CppFile {

    struct File {
        FILE* fp = NULL;
        bool blocking = true; // -blocking
        int64_t bufferingSize = -1; // -buffering
        bool buffering = true; // -buffering
        string encoding; // -encoding
        string in_oefchar; // -eofchar
        string out_eofchar;
        int translationMode = 1; // -translation
        int64_t lastPos = 0;

        string openOp;
        string closeOp;
        string writeOp;
        string readOp;
    };
    typedef val2<string_view /*option*/, variant<string_view,int64_t,double,bool>> Option;

    template<typename VALUETYPE>
    struct PredefOption {
        string_view     name_;
        VALUETYPE  value_;
        PredefOption(void) { }
    };

    enum BUFFERING { BUFFERING_NONE, BUFFERING_FULL, BUFFERING_LINE };
    enum TRANSLATION { TRANSLATION_AUTO, TRANSLATION_BINARY, TRANSLATION_CR, TRANSLATION_CRLF, TRANSLATION_LF };

    typedef PredefOption<bool>       forced;
    typedef PredefOption<bool>       blocking;
    typedef PredefOption<BUFFERING>  buffering;
    typedef PredefOption<int64_t>    buffersize;
    typedef PredefOption<string>     encoding;
    typedef PredefOption<string>     eofcharIn;
    typedef PredefOption<string>     eofcharOut;
    typedef PredefOption<TRANSLATION>   translationIn;
    typedef PredefOption<TRANSLATION>   translationOut;

    typedef PredefOption<int64_t>    size;
    typedef PredefOption<string>     command;

    enum FILE_FLAGS {
        FILE_RDONLY, FILE_WRONLY, FILE_RDWR, FILE_APPEND, FILE_BINARY, FILE_CREAT, FILE_EXEC, FILE_NOCTTY, FILE_NONBLOCK, FILE_TRUNC
    };

    typedef PredefOption<FILE_FLAGS> access;
    typedef PredefOption<int64_t>    permission;
    typedef PredefOption<bool>       nonewline;

    enum SEEK_FLAGS {
        SEEK_START_, SEEK_CURRENT_, SEEK_END_
    };
    typedef PredefOption< SEEK_FLAGS>   seekop;

    void test_Option() {
        Option      o1{ "op1", (int64_t)1 };
        Option      o2{ "op2", (double) 3.14 };
        Option      o3{ "op3", "value3" };
        Option      o4{ "op4", true };

        printf("op1 %s %d %d\n", o1.v1.data(), o1.v2.index(), (int)get<int64_t>(o1.v2));
        printf("op2 %s %d %f\n", o2.v1.data(), o2.v2.index(), get<double>(o2.v2));
        printf("op3 %s %d %s\n", o3.v1.data(), o3.v2.index(), get<string_view>(o3.v2).data());
        printf("op4 %s %d %d\n", o4.v1.data(), o4.v2.index(), get<bool>(o4.v2));

    }
    int         errno_;
    char        errorMsg_[128 + JIM_PATH_LEN];

    val2<Retval, int64_t> file_atime(string_view filename);
    Retval file_attributes(string_view filename, vector <Option>& options);
    Retval file_channels(string_view pattern);
    Retval file_copy(vector<string_view/*source*/>& sources, string_view dest, bool forced = false);
    val2<Retval, int64_t> file_ctime(string_view filename);
    Retval file_delete(string_view pathname, bool forced);
    Retval file_delete(vector<string_view/*filename*/>& pathnames, bool forced);
    val2<Retval, string> file_dirname(string_view pathname);
    bool exists(string_view pathname);
    bool isReadable(string_view pathname);
    bool isWritable(string_view pathname);
    bool isExecutable(string_view pathname);
    Retval file_executable(string_view pathname);
    Retval file_exists(string_view pathname);
    val2<Retval, string> file_extension(string_view pathname);
    Retval file_isdirectory(string_view pathname);
    Retval file_isfile(string_view pathname);
    val2<Retval, string> file_join(vector<string_view>& argv);
    Retval file_link_hard(string_view source, string_view& dest);
    Retval file_link_hard(string_view pathname, vector<string_view/*dest*/>& dest);
    Retval file_link_symbolic(string_view source, string_view& dest);
    Retval file_link_symbolic(string_view pathname, vector<string_view/*dest*/>& dest);
    //val2<Retval, vector<file_lstat_struct>> file_lstat1(string_view name);
    Retval file_mkdir(string_view path);
    Retval file_mkdir(vector<string_view/*dirs*/>& dirs);
    val2<Retval, int64_t> file_mtime(string_view pathname);
    Retval file_mtime(string_view filename, int64_t us);
    Retval file_nativename(string_view pathname); // #cppExtNeed
    val2<Retval, string> file_normalize(string_view path);
    Retval file_owned(string_view pathname);
    Retval file_pathtype(string_view pathname); // #cppExtNeed
    Retval file_readable(string_view pathname);
    val2<Retval, string> file_readlink(string_view pathname);
    Retval file_rename(string_view source, string_view target, bool forced = false);
    Retval file_rootname(string_view pathname); // #cppExtNeed
    Retval file_seperator(string_view name);  // #cppExtNeed
    Retval file_size(string_view name);
    Retval file_split(string_view name); // #cppExtNeed
    Retval file_stat(string_view name, string_view varName); // #cppExtNeed
    Retval file_system(string_view name); // #cppExtNeed
    val2<Retval, string> file_tail(string_view name);
#ifdef PRJ_OS_LINUX
    val3<Retval, int, string> file_maketempfile(const char* filename_template = NULL, bool unlink_file = true);
#endif
#ifdef PRJ_OS_WIN
    val2<Retval, string> file_temppath();
    val2<Retval, string> file_tempname(string_view filePath, const char* templateD);
    val4<Retval, HANDLE, string, int> file_win_maketempfile(string_view filePath, bool unlink_file = true, const char* templateD = NULL);
#endif
    string_view file_type(string_view pathname);
    Retval file_volumes(string_view name); // #cppExtNeed
    Retval file_writable(string_view name); 

    struct Files {
        // 
        Retval file_open(string_view fileName, string_view access, string_view permissions); // #cppExtNeed
        Retval file_close(File& handle); // #cppExtNeed
        Retval file_gets(File& handle, string_view varName); // #cppExtNeed
        Retval file_read(File& handle, int64_t numChars = -1, bool nonewline = false); // #cppExtNeed
        Retval file_puts(File& handle, string_view output, bool newline = true); // #cppExtNeed
        Retval file_seek(File& handle, int64_t offset, int origin = 0); // #cppExtNeed
        Retval file_eof(File& handle); // #cppExtNeed
        Retval file_flush(File& handle); // #cppExtNeed
        Retval file_tell(File& handle, int64_t& offset); // #cppExtNeed
        Retval file_fconfigure(File& handle, vector< Option>& options); // #cppExtNeed

        //
        Retval file_readAll(File& handle); // #cppExtNeed
        Retval file_readToList(File& handle); // #cppExtNeed
        Retval file_assert(Option& option, vector<string_view>& files); // #cppExtNeed
        Retval file_append(string_view file, string_view data); // #cppExtNeed
        Retval file_ftail(File& handle); // #cppExtNeed
        Retval file_assocate(string_view file, string_view openOp, string_view closeOp, string_view readOp, string_view writeOp); // #cppExtNeed
        Retval file_assocate_path(string_view file, string_view openOp, string_view closeOp, string_view readOp, string_view writeOp); // #cppExtNeed
        Retval file_changed(File& handle); // #cppExtNeed
    };

    // BODY =====================================================================
    val2<Retval, int64_t> file_atime(string_view filename) {
        FileStatus fileStatus(filename);

        int64_t ret = fileStatus.values_[FileStatus::ST_ATIME].value_;
        if (ret != 0) {
            snprintf(errorMsg_, sizeof(errorMsg_), "could not read \"%#s\": %s", filename.data(), ::strerror(fileStatus.errno_));
        }
        return val2<Retval, int64_t>{ret != -1, ret};
    }
    Retval file_attributes(string_view filename, vector <Option>& options) { return 0; } // #cppExtNeedDef
    Retval file_channels(string_view pattern) { return 0; } // #cppExtNeedDef
    Retval file_copy(vector<string_view/*source*/>& sources, string_view dest, bool forced) { return 0; } // #cppExtNeedDef
    val2<Retval, int64_t> file_ctime(string_view filename) {
        FileStatus fileStatus(filename);

        int64_t ret = fileStatus.values_[FileStatus::ST_CTIME].value_;
        if (ret != 0) {
            snprintf(errorMsg_, sizeof(errorMsg_), "could not read \"%#s\": %s", filename.data(), strerror(fileStatus.errno_));
        }
        return val2<Retval, int64_t>{ret != -1, ret};
    }
    Retval file_delete(string_view pathname, bool forced) {
        if (prj_unlink(pathname.data()) == -1 && get_errno() != ENOENT) {
            errno_ = get_errno();
            if (prj_rmdir(pathname.data()) == -1) {
                if (!forced) {
                    snprintf(errorMsg_, sizeof(errorMsg_), "couldn't delete file \"%s\": %s", pathname.data(),
                             strerror(errno_));
                    return 1;
                }
            }
        }
        return 0;
    }
    Retval file_delete(vector<string_view/*filename*/>& pathnames, bool forced = false) {
        for (string_view str : pathnames) {
            if (file_delete(str, forced)) return 1;
        }
        return 0;
    }
    val2<Retval, string> file_dirname(string_view pathname) {
        string path;
        const char* p = ::strrchr(path.data(), '/');

        if (!(p != NULL) && (path[0] == '.') && (path[1] == '.') && (path[2] = '\0')) { // #Review
            return val2<Retval, string>{0, ".."};
        } else if (!p) {
            return val2<Retval, string>{0, "."};
        } else if (p == path.data()) {
            return val2<Retval, string>{0, "/"};
        } else if (isWindows() && p[-1] == ':') {
            return val2<Retval, string>{0, string(path.data(), (string::size_type) (p - path.data() + 1))}; // #Review
        } else {

        }
        return val2<Retval, string>{0, string(path.data(), (string::size_type)(p - path.data()))};
    }
    bool exists(string_view pathname) {
        if (isWindows()) {
            return prj_access(pathname.data(), 00);
        } else {
#ifndef F_OK 
#  define F_OK 1
#endif
            return prj_access(pathname.data(), F_OK);
        }
    }
    bool isReadable(string_view pathname) {
        if (isWindows()) {
            return prj_access(pathname.data(), 04);
        } else {
#ifndef R_OK 
#  define R_OK 1
#endif
            return prj_access(pathname.data(), R_OK);
        }
    }
    bool isWritable(string_view pathname) {
        if (isWindows()) {
            return prj_access(pathname.data(), 02);
        } else {
#ifndef W_OK 
#  define W_OK 1
#endif
            return prj_access(pathname.data(), W_OK);
        }
    }
    bool isExecutable(string_view pathname) {
        if (isWindows()) {
            // TODO How?
            return true;
        } else {
#ifndef X_OK 
#  define X_OK 1
#endif
            return prj_access(pathname.data(), X_OK);
        }
    }
    Retval file_executable(string_view pathname) {
        return isExecutable(pathname);
    }
    Retval file_exists(string_view pathname) { return exists(pathname); }
    val2<Retval, string> file_extension(string_view pathname) {
        const char* lastSlash = ::strrchr(pathname.data(), '/');
        const char* p = ::strrchr(pathname.data(), '.');

        if ((p == NULL) || (lastSlash != NULL && lastSlash >= p)) {
            p = "";
        }
        return val2<Retval, string>{0, p};
    }
    Retval file_isdirectory(string_view pathname) {
        FileStatus fileStatus(pathname);
        if (fileStatus.errno_) return 1;
        int v = (int)fileStatus.values_[FileStatus::ST_MODE].value_;
        if (isWindows()) {
#ifndef _S_IFMT
#  define _S_IFMT 0xF000
#endif
#ifndef _S_IFDIR
#  define _S_IFDIR 0x4000
#endif
            return (v & _S_IFMT) == _S_IFDIR;
        }
#ifndef S_IFMT
#  define S_IFMT 1
#endif
#ifndef S_IFDIR
#  define S_IFDIR 1
#endif
        return (v & S_IFMT) == S_IFDIR;
    }
    Retval file_isfile(string_view pathname) {
        FileStatus fileStatus(pathname);
        if (fileStatus.errno_) return 1;
        int v = (int)fileStatus.values_[FileStatus::ST_MODE].value_;
        if (isWindows()) {
#ifndef _S_IFREG
#  define _S_IFREG 1
#endif
            return (v & _S_IFMT) == _S_IFREG;
        }
#ifndef S_IFREG
#  define S_IFREG 1
#endif
        return (v & S_IFMT) == _S_IFREG;
    }
    val2<Retval, string> file_join(vector<string_view>& argv) {
        int i;
        char newname[JIM_PATH_LEN + 1];
        char* last = newname;

        *newname = 0;

        /* Simple implementation for now */
        for (i = 0; i < (int)argv.size(); i++) {
            int len = 0;
            const char* part = argv[i].data();

            if (*part == '/') {
                /* Absolute component, so go back to the start */
                last = newname;
            } else if (isWindows() && ::strchr(part, ':')) {
                /* Absolute component on mingw, so go back to the start */
                last = newname;
            } else if (part[0] == '.') {
                if (part[1] == '/') {
                    part += 2;
                    len -= 2;
                } else if (part[1] == 0 && last != newname) {
                    /* Adding '.' to an existing path does nothing */
                    continue;
                }
            }

            /* Add a slash if needed */
            if (last != newname && last[-1] != '/') {
                *last++ = '/';
            }

            if (len) {
                if (last + len - newname >= JIM_PATH_LEN) {
                    snprintf(errorMsg_, sizeof(errorMsg_), "Path too long");
                    return val2<Retval, string>{1, ""};
                }
                memcpy(last, part, len);
                last += len;
            }

            /* Remove a slash if needed */
            if (last > newname + 1 && last[-1] == '/') {
                /* but on on Windows, leave the trailing slash on "c:/ " */
                if (!isWindows() || !(last > newname + 2 && last[-2] == ':')) {
                    *--last = 0;
                }
            }
        }

        *last = 0;

        /* Probably need to handle some special cases ... */
        return val2<Retval, string>{0, string(newname, (string::size_type)(last - newname))};
    }
    Retval file_link_hard(string_view source, string_view& dest) {
        if (!prj_funcDef(prj_link)) return 1;
        return prj_link(source.data(), dest.data());
    }
    Retval file_link_hard(string_view pathname, vector<string_view/*dest*/>& dest) {
        for (string_view& path : dest) {
            if (file_link_hard(pathname, path)) return 1;
        }
        return 0;
    }
    Retval file_link_symbolic(string_view source, string_view& dest) {
        if (!prj_funcDef(prj_symlink)) return 1;
        return prj_symlink(source.data(), dest.data());
    }
    Retval file_link_symbolic(string_view pathname, vector<string_view/*dest*/>& dest) {
        for (string_view& path : dest) {
            if (file_link_hard(pathname, path)) return 1;
        }
        return 0;
    }

    struct file_lstat_struct {
        string_view name_;
        FileStatus::FIELDS field_;
        int64_t value_ = -1;
    };
    val2<Retval, vector<file_lstat_struct>> file_lstat1(string_view name) {
        FileStatus  fileStatus(name);
        file_lstat_struct fieldInfo;
        if (fileStatus.errno_) return val2<Retval, vector<file_lstat_struct>>{1, vector<file_lstat_struct>()};

        vector <file_lstat_struct> ret;
        for (int i = 0; i < FileStatus::NUM_FIELDS; i++) {
            fieldInfo.field_ = (FileStatus::FIELDS)i;
            fieldInfo.name_ = fileStatus.values_[i].name_;
            fieldInfo.value_ = fileStatus.values_[i].value_;
            ret.push_back(fieldInfo);
        }
        return val2<Retval, vector<file_lstat_struct>>{1, ret};
    }
    Retval file_mkdir(string_view path) {
        int ok = 1;

        /* First time just try to make the dir */
        goto first;

        while (ok--) {
            /* Must have failed the first time, so recursively make the parent and try again */
            {
                char* slash = (char*)::strrchr(path.data(), '/');

                if (slash && slash != path.data()) {
                    *slash = 0;
                    if (file_mkdir(path.data()) != 0) {
                        return -1;
                    }
                    *slash = '/';
                }
            }
first:
            if (prj_mkdir2(path.data(), 0755) == 0) {
                return 0;
            }
            if (get_errno() == ENOENT) {
                /* Create the parent and try again */
                continue;
            }
            /* Maybe it already exists as a directory */
            if (get_errno() == EEXIST) {
                if (exists(path) && file_isdirectory(path)) {
                    return 0;
                }
                /* Restore errno */
                get_errno_ref() = EEXIST;
            }
            /* Failed */
            break;
        }
        return -1;
    }
    Retval file_mkdir(vector<string_view/*dirs*/>& dirs) {
        for (string_view& d : dirs) {
            if (file_mkdir(d)) return 1;
        }
        return 0;
    }
    val2<Retval, int64_t> file_mtime(string_view pathname) {
        FileStatus fileStatus(pathname);
        if (fileStatus.errno_) return val2<Retval, int64_t>{1, -1};

        int64_t ret = fileStatus.values_[FileStatus::ST_MTIME].value_;
        if (ret != 0) {
            snprintf(errorMsg_, sizeof(errorMsg_), "could not read \"%#s\": %s", pathname.data(), strerror(get_errno()));
        }
        return val2 <Retval, int64_t>{ret != -1, ret};
    }
    Retval file_mtime(string_view filename, int64_t us) {
        if (prj_funcDef(prj_utimes)) {
            struct prj_timeval times[2];

            times[1].tv_sec = (long) (times[0].tv_sec = (long) (us / 1000000));
            times[1].tv_usec = times[0].tv_usec = us % 1000000;

            if (prj_utimes(filename.data(), times) != 0) {
                snprintf(errorMsg_, sizeof(errorMsg_), "can't set time on \"%s\": %s", filename.data(), strerror(get_errno()));
                return 1;
            }
            return 0;
        }
        return 1;
    }
    Retval file_nativename(string_view pathname) { // #cppExtNeedDef
        return 0;
    }
    val2<Retval, string> file_normalize(string_view path) {
        if (!prj_funcDef(prj_realpath)) return val2<Retval, string>{1, ""};

        char newname[JIM_PATH_LEN + 1];

        if (prj_realpath(path.data(), newname)) {
            return val2<Retval, string>{0, newname};
        } else {
            snprintf(errorMsg_, sizeof(errorMsg_), "can't normalize \"%#s\": %s", path.data(), strerror(get_errno()));
            return val2<Retval, string>{1, ""};
        }

        return val2<Retval, string>{1, ""};
    }
    Retval file_owned(string_view pathname) {
        if (!file_exists(pathname)) return 1;
        FileStatus fileStatus(pathname);
        if (fileStatus.errno_) return 1;

        return prj_geteuid() == fileStatus.values_[FileStatus::ST_UID].value_;
    }
    Retval file_pathtype(string_view pathname) { return 0; } // #cppExtNeedDef
    Retval file_readable(string_view pathname) { return isReadable(pathname);   }
    val2<Retval, string> file_readlink(string_view pathname) {
        char linkValue[JIM_PATH_LEN + 1];
        int linkLength = (int) prj_readlink(pathname.data(), linkValue, sizeof(linkValue));
        if (linkLength == -1) {
            snprintf(errorMsg_, sizeof(errorMsg_), "couldn't readlink \"%#s\": %s", pathname.data(), strerror(get_errno()));
            return val2<Retval, string>{1, ""};
        }
        return val2<Retval, string>{0, linkValue};
    }
    Retval file_rename(string_view source, string_view target, bool forced) {
        if (!forced && file_exists(target)) {
            snprintf(errorMsg_, sizeof(errorMsg_), "error renaming \"%#s\" to \"%#s\": target exists", source.data(),
                     target.data());
            return 1;
        }
        if (rename(source.data(), target.data()) != 0) {
            snprintf(errorMsg_, sizeof(errorMsg_), "error renaming \"%#s\" to \"%#s\": %s", source.data(), target.data(),
                     strerror(get_errno()));
        }
        return 0;
    }
    Retval file_rootname(string_view pathname) { return 0; } // #cppExtNeedDef
    Retval file_seperator(string_view name) { return 0; } // #cppExtNeedDef
    Retval file_size(string_view name) {
        FileStatus fileStatus(name);
        if (fileStatus.errno_) return 1;
        int v = (int)fileStatus.values_[FileStatus::ST_SIZE].value_;
        return v;
    }
    Retval file_split(string_view name) { return 0; } // #cppExtNeedDef
    Retval file_stat(string_view name, string_view varName) { return 0; } // #cppExtNeedDef
    Retval file_system(string_view name) { return 0; } // #cppExtNeedDef
    val2<Retval, string> file_tail(string_view name) {
        const char* lastSlash = ::strrchr(name.data(), '/');
        if (lastSlash == NULL) return val2<Retval, string>{1, ""};
        return val2<Retval, string>{0, (lastSlash + 1)};
    }
#ifdef PRJ_OS_LINUX
    val3<Retval,int,string> file_maketempfile(const char* filename_template = NULL, bool unlink_file) {
        int fd;
        mode_t mask;
        string filename;

        if (filename_template == NULL) {
            const char* tempdir = prj_getenv("TMPDIR");
            if (tmpdir == NULL || *tempdir == '\0' || access(tmpdir, W_OK) != 0) {
                tmpdir = "/tmp/";
            }
            filename = tmpdir;
            if (tmpdir[0] && tmddir[strlen(tmpdir) - 1] != '/') {
                filname.append("tcl.tmp.XXXXXX");
            }
        } else {
            filename = filename_template;
        }

        mask = prj_umask(S_IXUSR | S_IRWXG | S_IRWXO); 

        char filenameArray[JIM_PATH_LEN];
        strncpy(filenameArray, filename.data(), sizeof(filenameArray));

        if (prj_funcDef(prj_mkstemp)) {
            fd = prj_mkstemp(filenameArray);
        } else {
            if (::mktemp(filenameArray) == NULL) {
                fd = -1;
            } else {
                fd = prj_open(filenameArray, O_RDWR | O_CREAT | O_TRUNC);
            }
        }
        prj_umask(mask);
        if (fd < 0) {
            return val3<Retval, int, string>(1, -1, "");
        }
        if (unlink_file) {
            ::remove(fileNameArray);
        }
        return val3<Retval, int, string>(0, fd, filenameArray);
    }
#endif
#ifdef PRJ_OS_WIN
    val2<Retval, string> file_temppath() {
        char    name[JIM_PATH_LEN];
        auto ret = ::GetTempPathA(sizeof(name), name);
        if (ret == 0) pair<Retval, string>(1, "");
        return val2<Retval, string>{0, name};
    }
    val2<Retval, string> file_tempname(string_view filePath, const char* templateD = NULL) {
        char    name[JIM_PATH_LEN];
        ::strncpy(name, filePath.data(), sizeof(name));
        auto ret = ::GetTempFileNameA(name, templateD ? templateD : "JIM", 0, name);
        if (ret == 0) val2<Retval, string>{1, ""};
        return val2<Retval, string>{1, name};
    }
    val4<Retval,HANDLE,string,int> file_win_maketempfile(string_view filePath, bool unlink_file, const char* templateD) {
        string name;
        auto temppath = file_temppath();
        if (temppath.v1 != 0)  return val4<Retval, HANDLE, string, int>{ 1, INVALID_HANDLE_VALUE, "", -1};
        auto tempname = file_tempname(temppath.v2, templateD);
        if (tempname.v1 != 0) return val4<Retval, HANDLE, string, int>{ 1, INVALID_HANDLE_VALUE, "", -1};

        HANDLE handle = CreateFileA(tempname.v2.data(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | (unlink_file ? FILE_FLAG_DELETE_ON_CLOSE : 0),
                                    NULL);
        if (handle == INVALID_HANDLE_VALUE) {
            ::DeleteFileA(tempname.v2.data());
            return val4<Retval, HANDLE, string, int>{ 1, INVALID_HANDLE_VALUE, "", -1};
        }
        return val4<Retval, HANDLE, string, int>{ 0, handle, tempname.v2,  _open_osfhandle((intptr_t) handle, _O_RDWR | O_TEXT) };
    }
#endif
    string_view file_type(string_view pathname) { 
        FileStatus fileStatus(pathname);
        if (fileStatus.errno_) return "";
        int mode = (int)fileStatus.values_[FileStatus::ST_MODE].value_;

#ifdef PRJ_OS_WIN
        if ((mode & _S_IFMT) == _S_IFREG) {
            return "file";
        } else if ((mode & _S_IFMT) == _S_IFDIR) {
            return "directory";
        }
#else
        if (S_ISREG(mode)) {
            return "file";
        } else if (S_ISDIR(mode)) {
            return "directory";
        }
#endif
#ifdef S_ISCHR // #optionalCode #WinOff
        else if (S_ISCHR(mode)) {
            return "characterSpecial";
        }
#endif
#ifdef S_ISBLK // #optionalCode #WinOff
        else if (S_ISBLK(mode)) {
            return "blockSpecial";
        }
#endif
#ifdef S_ISFIFO // #optionalCode #WinOff
        else if (S_ISFIFO(mode)) {
            return "fifo";
        }
#endif
#ifdef S_ISLNK // #optionalCode #WinOff
        else if (S_ISLNK(mode)) {
            return "link";
        }
#endif
#ifdef S_ISSOCK // #optionalCode #WinOff
        else if (S_ISSOCK(mode)) {
            return "socket";
        }
#endif
        return 0; 
    }
    Retval file_volumes(string_view name) { return 0; } // #cppExtNeedDef
    Retval file_writable(string_view name) { return isWritable(name); }

    // Slight change to Tcl command
    Retval Files::file_open(string_view fileName, string_view access, string_view permissions) { return 0;  } // #cppExtNeedDef
    Retval Files::file_close(File& handle) { return 0;  } // #cppExtNeedDef
    Retval Files::file_gets(File& handle, string_view varName) { return 0; } // #cppExtNeedDef
    Retval Files::file_read(File& handle, int64_t numChars, bool nonewline) { return 0; } // #cppExtNeedDef
    Retval Files::file_puts(File& handle, string_view output, bool newline) { return 0; } // #cppExtNeedDef
    Retval Files::file_seek(File& handle, int64_t offset, int origin) { return 0;  } // #cppExtNeedDef
    Retval Files::file_eof(File& handle) { return 0; } // #cppExtNeedDef
    Retval Files::file_flush(File& handle) { return 0; } // #cppExtNeedDef
    Retval Files::file_tell(File& handle, int64_t& offset) { return 0; } // #cppExtNeedDef
    Retval Files::file_fconfigure(File& handle, vector< Option>& options) { return 0; } // #cppExtNeedDef

    // Some ideas
    Retval Files::file_readAll(File& handle) { return 0; } // #cppExtNeedDef
    Retval Files::file_readToList(File& handle) { return 0; } // #cppExtNeedDef
    Retval Files::file_assert(Option& option, vector<string_view>& files) { return 0;  } // #cppExtNeedDef
    Retval Files::file_append(string_view file, string_view data) { return 0; } // #cppExtNeedDef
    Retval Files::file_ftail(File& handle) { return 0; } // #cppExtNeedDef
    Retval Files::file_assocate(string_view file, string_view openOp, string_view closeOp, string_view readOp, string_view writeOp) { return  0; } // #cppExtNeedDef
    Retval Files::file_assocate_path(string_view file, string_view openOp, string_view closeOp, string_view readOp, string_view writeOp) { return 0; } // #cppExtNeedDef
    Retval Files::file_changed(File& handle) { return 0; } // #cppExtNeedDef

};
// ======================================================

static const jim_subcmd_type g_file_command_table[] = { // #JimSubCmdDef
    {   "mkdir",
        "dir ...",
        /*file_cmd_mkdir*/0,
        1,
        -1,
        /* Description: Creates the directories */
    },
    {   "exists",
        "name",
        /*file_cmd_exists*/0,
        1,
        1,
        /* Description: Does file exist */
    },
    {   "delete",
        "?-force|--? name ...",
        /*file_cmd_delete*/0,
        1,
        -1,
        /* Description: Deletes the files or directories (must be empty unless -force) */
    },
};



Retval Jim_fileppInit(Jim_InterpPtr  interp) // #JimCmdInit
{
    if (Jim_PackageProvide(interp, "filepp", "1.0", JIM_ERRMSG))
        return JIM_ERR;

    Jim_CreateCommand(interp, "filepp", Jim_SubCmdProc, (void*) g_file_command_table, NULL);
    //Jim_CreateCommand(interp, "pwd", Jim_PwdCmd, NULL, NULL);
    //Jim_CreateCommand(interp, "cd", Jim_CdCmd, NULL, NULL);
    return JIM_OK;
}


END_JIM_NAMESPACE
