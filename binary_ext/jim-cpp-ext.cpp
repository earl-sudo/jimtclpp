#include <string.h>
#include <jim-cpp-api.h>
#include <errno.h>
#include <sys/stat.h>
#include <direct.h> // #NonPortHeader
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)

#include <tuple>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <string_view>

#define MKDIR_DEFAULT(PATHNAME) _mkdir(PATHNAME) // #NonPortFunc #TODO

BEGIN_JIM_NAMESPACE

using namespace std;

/**
 * Create directory, creating all intermediate paths if necessary.
 *
 * Returns 0 if OK or -1 on failure (and sets errno)
 *
 * Note: The path may be modified.
 */
static int mkdir_all(char* path) {
    int ok = 1;

    /* First time just try to make the dir */
    goto first;

    while (ok--) {
        /* Must have failed the first time, so recursively make the parent and try again */
        {
            char* slash = strrchr(path, '/');

            if (slash && slash != path) {
                *slash = 0;
                if (mkdir_all(path) != 0) {
                    return -1;
                }
                *slash = '/';
            }
        }
first:
        if (MKDIR_DEFAULT(path) == 0) {
            return 0;
        }
        if (errno == ENOENT) {
            /* Create the parent and try again */
            continue;
        }
        /* Maybe it already exists as a directory */
        if (errno == EEXIST) {
            struct stat sb;

            if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                return 0;
            }
            /* Restore errno */
            errno = EEXIST;
        }
        /* Failed */
        break;
    }
    return -1;
}

#ifdef ORIG
static Retval file_cmd_mkdir(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    while (argc--) {
        char* path = Jim_StrDup(Jim_String(argv[0]));
        int rc = mkdir_all(path);

        Jim_TFree<char>(path); // #FreeF 
        if (rc != 0) {
            Jim_SetResultFormatted(interp, "can't create directory \"%#s\": %s", argv[0],
                                   strerror(errno));
            return JIM_ERR;
        }
        argv++;
    }
    return JIM_OK;
}

static Retval file_cmd_exists(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    return file_access(interp, argv[0], F_OK);
}

static Retval file_cmd_delete(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    int force = Jim_CompareStringImmediate(interp, argv[0], "-force");

    if (force || Jim_CompareStringImmediate(interp, argv[0], "--")) {
        argc++;
        argv--;
    }

    while (argc--) {
        const char* path = Jim_String(argv[0]);

        if (prj_unlink(path) == -1 && errno != ENOENT) { // #NonPortFuncFix
            if (prj_rmdir(path) == -1) { // #NonPortFuncFix
                /* Maybe try using the script helper */
                if (!force || Jim_EvalPrefix(interp, "file delete force", 1, argv) != JIM_OK) {
                    Jim_SetResultFormatted(interp, "couldn't delete file \"%s\": %s", path,
                                           strerror(errno));
                    return JIM_ERR;
                }
            }
        }
        argv++;
    }
    return JIM_OK;
}

static Retval file_stat(Jim_InterpPtr  interp, Jim_ObjPtr  filename, struct stat* sb) {
    const char* path = Jim_String(filename);

    if (stat(path, sb) == -1) {
        Jim_SetResultFormatted(interp, "could not read \"%#s\": %s", filename, strerror(errno));
        return JIM_ERR;
    }
    return JIM_OK;
}

static Retval file_cmd_size(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    struct stat sb;

    if (file_stat(interp, argv[0], &sb) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp, sb.st_size);
    return JIM_OK;
}
#endif

static tuple<Retval, _off_t> fileSize(const char* path) {
    struct stat sb;

    if (stat(path, &sb) != -1) {
        return { JIM_OK, sb.st_size };
    }
    return { JIM_ERR, 0 };
}

static Retval fileSize(const char* path, _off_t& sz) {
    struct stat sb;

    if (stat(path, &sb) != -1) {
        sz = sb.st_size;
        return JIM_OK;
    }
    return JIM_ERR;
}

static int64_t fileSize1(const char* path) {
    struct stat sb;

    if (stat(path, &sb) != -1) {
        return (int64_t) sb.st_size;
    }
    return -1;
}

template<typename tocheck> 
bool negativeIsError(tocheck v) { return v < 0; }

struct ArgumentCheck {
    int16_t minNumArgs_ = 0;
    int16_t maxNumArgs_ = 0;
};

struct JimObjError {
    Retval code_;
    JimObjError(Retval v) : code_(v) { }
};

enum JIMOBJ_ERRORS {
    JIMOBJ_ERROR_CONV_LONG = 100, JIMOBJ_ERROR_CONV_INT64, JIMBOBJ_ERRROR_CONV_BOOL, JIMOBJ_ERROR_CONV_DOUBLE,
    JIMOBJ_ERROR_BADINDEX, JIMOBJ_ERROR_JUSTARETURN,
    JIMOBJ_ERROR_UNKNOWN_VAR_NAME
};

// Combines Jim_InterpPtr and Jim_ObjPtr into a convent package
struct JimObj {
    Jim_InterpPtr  interp_;
    Jim_ObjPtr  obj_;

    JimObj(Jim_InterpPtr  interp, Jim_ObjPtr  obj) : interp_(interp), obj_(obj) { }
    JimObj(const JimObj& lhs) : interp_(lhs.interp_), obj_(lhs.obj_) { }
    const JimObj& operator=(const JimObj& lhs) {
        interp_ = lhs.interp_;
        obj_ = lhs.obj_;
        return *this;
    }

    // Simply exports a value or throw exception.
    operator long() const { 
        long ret = 0;  
        Retval retcode = Jim_GetLong(interp_, obj_, &ret);  
        if (retcode != JIM_OK) throw JimObjError(JIMOBJ_ERROR_CONV_LONG);
        return ret; 
    }
    operator int64_t() const {
        int64_t ret = 0;
        Retval retcode = Jim_GetWide(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMOBJ_ERROR_CONV_INT64);
        return ret;
    }
    operator bool() const {
        int ret = 0;
        Retval retcode = Jim_GetBoolean(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMBOBJ_ERRROR_CONV_BOOL);
        return (bool)ret;
    }
    operator const char* () const {
        const char* ret = 0;
        return Jim_String(obj_);
    }
    operator double () const {
        double ret = 0;
        Retval retcode = Jim_GetDouble(interp_, obj_, &ret);
        if (retcode != JIM_OK) throw JimObjError(JIMOBJ_ERROR_CONV_DOUBLE);
        return ret;
    }

    bool isList(int index) const { return Jim_IsList(obj_); }    
    int list_length() const {
        return Jim_ListLength(interp_, obj_);
    }

    // Contently list concatenation. 
    JimObj& append(const char* val, int len = -1) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewStringObj(interp_, val, len));
        return *this;
    }
    JimObj& append(long val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewIntObj(interp_, val));
        return *this;
    }
    JimObj& append(int64_t val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewIntObj(interp_, val));
        return *this;
    }
    JimObj& append(bool val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewIntObj(interp_, val));
        return *this;
    }
    JimObj& append(double val) {
        Jim_ListAppendElement(interp_, obj_, Jim_NewDoubleObj(interp_, val));
        return *this;
    }
    bool isDict(int index) const { return Jim_IsDict(obj_); }

};

// Wraps the standard args of Jim Jim_InterpPtr, argc, argv
struct JimArgs {
private:
    Jim_InterpPtr        interp_;
    int                  argc_;
    Jim_ObjConstArray    argv_;
public:
    bool setResults_ = false;

    JimArgs(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) : interp_(interp), argc_(argc), argv_(argv) { }

    // Access to arguments
    int numArgs() const { return argc_; }
    JimObj arg(int index) {
        if (index >= numArgs()) throw JimObjError(JIMOBJ_ERROR_BADINDEX);
        return JimObj(interp_, argv_[index]);
    }

    // Convert C native to a JimObj
    JimObj val(const char* val, int len = -1) {
        return  JimObj(interp_, Jim_NewStringObj(interp_, val, len));
    }
    JimObj val(long val) {
        return  JimObj(interp_, Jim_NewIntObj(interp_, val));
    }
    JimObj val(int64_t val) {
        return  JimObj(interp_, Jim_NewIntObj(interp_, val));
    }
    JimObj listVal(void) {
        return  JimObj(interp_, Jim_NewListObj(interp_, NULL, 0));
    }
    JimObj dictVal(void) {
        return  JimObj(interp_, Jim_NewDictObj(interp_, NULL, 0));
    }
#if 0
    JimObj getNamedNoErr(Jim_ObjPtr  varName) { // Create if doesn't exists.
        Jim_ObjPtr  objPtr = Jim_GetVariable(interp_, varName, JIM_NONE);
        if (objPtr) {

        }
    }
#endif

    // Set return value 
    void return_(const char* val) { setResults_ = true;  Jim_SetResultString(interp_, val, -1); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(long val) { setResults_ = true;  Jim_SetResultInt(interp_, val); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(int64_t val) { setResults_ = true;  Jim_SetResultInt(interp_, val); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(bool val) { setResults_ = true;  Jim_SetResultBool(interp_, (long_long) val);  throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_() { setResults_ = true;  Jim_SetEmptyResult(interp_); throw JimObjError(JIMOBJ_ERROR_JUSTARETURN); }
    void return_(JimObj& obj) { setResults_ = true; Jim_SetResult(interp_, obj.obj_); }
};

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

        args.return_(fileSize1((const char*) path));
    }
    JimCmd(const string& cmd, const string& description) : cmd_(cmd), description_(description) {
    }
};

static Retval file_cmd_mkdir(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    Retval ret = JIM_OK;
    try {
        JimArgs  args(interp, argc, argv);
        JimObj   path(args.arg(0));

        args.return_(fileSize1((const char*)path));
    } catch (JimObjError& joe) {
        if (joe.code_ == JIMOBJ_ERROR_JUSTARETURN) {
        } else {
            ret = JIM_ERR;
        }
    }
    return ret;
}

static Retval file_cmd_exists(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    return JIM_OK;
}

static Retval file_cmd_delete(Jim_InterpPtr  interp, int argc, Jim_ObjConstArray  argv) // #JimCmd
{
    return JIM_OK;
}

#if 0
struct jim_subcmd_type {
    const char* cmd;				/* Name of the (sub)command */
    const char* args;				/* Textual description of allowed args */
    jim_subcmd_function* function;	/* Function implementing the subcommand */
    short minargs;					/* Minimum required arguments */
    short maxargs;					/* Maximum allowed arguments or -1 if no limit */
    unsigned_short flags;			/* JIM_MODFLAG_... plus custom flags */
};
#endif

// ====================================================================================================
static const char* JimGetFileType(int mode) {
    if (S_ISREG(mode)) {
        return "file";
    } else if (S_ISDIR(mode)) {
        return "directory";
    }
#ifdef S_ISCHR
    else if (S_ISCHR(mode)) {
        return "characterSpecial";
    }
#endif
#ifdef S_ISBLK
    else if (S_ISBLK(mode)) {
        return "blockSpecial";
    }
#endif
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode)) {
        return "fifo";
    }
#endif
#ifdef S_ISLNK
    else if (S_ISLNK(mode)) {
        return "link";
    }
#endif
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode)) {
        return "socket";
    }
#endif
    return "unknown";
}

static JimObj stat_to_jim(JimArgs& args, const struct stat& sb) {
    JimObj  listObj(args.listVal());
    listObj.append("dev").append((jim_wide) sb.st_dev);
    listObj.append("ino").append((jim_wide) sb.st_ino);
    listObj.append("mode").append((jim_wide) sb.st_mode);
    listObj.append("nlink").append((jim_wide) sb.st_nlink);
    listObj.append("uid").append((jim_wide) sb.st_uid);
    listObj.append("gid").append((jim_wide) sb.st_gid);
    listObj.append("size").append((jim_wide) sb.st_size);
    listObj.append("atime").append((jim_wide) sb.st_atime);
    listObj.append("mtime").append((jim_wide) sb.st_mtime);
    listObj.append("ctime").append((jim_wide) sb.st_ctime);
    //listObj.append("mtimeus").append((jim_wide) sb.st_ctime);
    listObj.append("type").append(JimGetFileType((int) sb.st_mode));

    return listObj;
}
static Retval stat_to_return(JimArgs& args, const struct stat& sb) {
    // #TODO args.return_(stat_to_jim(args, sb));
    return JIM_OK;
}

static Retval stat_to_var(JimArgs& args, Jim_ObjPtr  varName) {

}
// ======================================================
struct CppFile {
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
    typedef pair<string_view /*option*/, string_view/*value*/> Option;

    Retval file_atime(string_view fileNme, uint64_t time = 0) { return 0;  }
    Retval file_attributes(string_view filename, vector <Option>& options) { return 0; }
    Retval file_channels(string_view pattern) { return 0;  }
    Retval file_copy(vector<string_view/*source*/>& sources, string_view dest, bool forced = false) { return 0;  }
    Retval file_delete(vector<string_view/*filename*/>& pathnames) { return 0;  }
    Retval file_dirname(string_view pathname) { return 0;  }
    Retval file_executable(string_view pathname) { return 0;  }
    Retval file_exists(string_view pathname) { return 0; }
    Retval file_extension(string_view pathname) { return 0; }
    Retval file_isdirectory(string_view pathname) { return 0; }
    Retval file_isfile(string_view pathname) { return 0; }
    Retval file_join(string_view pathname) { return 0; }
    Retval file_link(string_view pathname, vector<string_view/*dest*/>& dest) { return 0; }
    Retval file_lstat(string_view name, string_view varName) { return 0; }
    Retval file_mkdir(vector<string_view/*dirs*/>& dirs) { return 0; }
    Retval file_mtime(string_view pathname, uint64_t time = 0) { return 0; }
    Retval file_nativename(string_view pathname) { return 0; }
    Retval file_normalize(string_view pathname) { return 0; }
    Retval file_owned(string_view pathname) { return 0; }
    Retval file_pathtype(string_view pathname) { return 0; }
    Retval file_readable(string_view pathname) { return 0; }
    Retval file_readlinke(string_view pathname) { return 0; }
    Retval file_rename(vector<string_view>& sources, string_view target, bool forced = false) { return 0; }
    Retval file_rootname(string_view pathname) { return 0; }
    Retval file_seperator(string_view name) { return 0; }
    Retval file_size(string_view name) { return 0; }
    Retval file_split(string_view name) { return 0; }
    Retval file_stat(string_view name, string_view varName) { return 0; }
    Retval file_system(string_view name) { return 0; }
    Retval file_tail(string_view name) { return 0; }
    Retval file_tempfile(string_view nameVar, string_view templateD) { return 0; }
    Retval file_type(string_view name) { return 0; }
    Retval file_volumes(string_view name) { return 0; }
    Retval file_writable(string_view name) { return 0; }

    // Slight change to Tcl command
    Retval file_open(string_view fileName, string_view access, string_view permissions) { return 0;  }
    Retval file_close(File& handle) { return 0;  }
    Retval file_gets(File& handle, string_view varName) { return 0; }
    Retval file_read(File& handle, int64_t numChars = -1, bool nonewline = false) { return 0; }
    Retval file_puts(File& handle, string_view output, bool newline = true) { return 0; }
    Retval file_seek(File& handle, int64_t offset, int origin = 0) { return 0;  }
    Retval file_eof(File& handle) { return 0; }
    Retval file_flush(File& handle) { return 0; }
    Retval file_tell(File& handle, int64_t& offset) { return 0; }
    Retval file_fconfigure(File& handle, vector< Option>& options) { return 0; }

    // Some ideas
    Retval file_readAll(File& handle) { return 0; }
    Retval file_readToList(File& handle) { return 0; }
    Retval file_assert(Option& option, vector<string_view>& files) { return 0;  }
    Retval file_append(string_view file, string_view data) { return 0; }
    Retval file_ftail(File& handle) { return 0; }
    Retval file_assocate(string_view file, string_view openOp, string_view closeOp, string_view readOp, string_view writeOp) { return 0; }
    Retval file_assocate_path(string_view file, string_view openOp, string_view closeOp, string_view readOp, string_view writeOp) { return 0; }
    Retval file_changed(File& handle) { return 0; }
};
// ======================================================

static const jim_subcmd_type g_file_command_table[] = { // #JimSubCmdDef
    {   "mkdir",
        "dir ...",
        file_cmd_mkdir,
        1,
        -1,
        /* Description: Creates the directories */
    },
    {   "exists",
        "name",
        file_cmd_exists,
        1,
        1,
        /* Description: Does file exist */
    },
    {   "delete",
        "?-force|--? name ...",
        file_cmd_delete,
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
