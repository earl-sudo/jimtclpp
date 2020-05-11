#define _CRT_SECURE_NO_WARNINGS 1

#define USE_FILESYSTEM 1

#include <tuple>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <string_view>
#include <variant>
#if USE_FILESYSTEM
#  include <filesystem>
#endif

#include <stdlib.h>
#include <jimautoconf.h>

#ifdef PRJ_OS_WIN
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif
#include "prj_wrap.h"

#include <errno.h>
#include <sys/stat.h>
#ifdef PRJ_OS_WIN
#  include <direct.h> // #NonPortHeader
#  include <io.h>
#  include <fcntl.h>
#endif
#ifdef PRJ_OS_LINUX
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#endif

#include <prj_compat.h>

#ifndef JIM_PATH_LEN
#  define JIM_PATH_LEN 1024
#endif

namespace prj_wrap {

    using namespace std;

    int get_errno(void) {
#ifdef PRJ_OS_WIN
        return *::_errno();
#else
        return errno;
#endif
    }
    int& get_errno_ref(void) {
#ifdef PRJ_OS_WIN
        return *::_errno();
#else
        return errno;
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
            int ret = ::stat((const char*) filepath.data(), &fileStatus_);
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

#if 0
        template<typename VALUETYPE>
        struct PredefOption {
            string_view     name_;
            VALUETYPE  value_;
            PredefOption(void) {}
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
            FILE_RDONLY, FILE_WRONLY, FILE_RDWR, FILE_APPEND, 
            FILE_BINARY, FILE_CREAT, FILE_EXEC, FILE_NOCTTY, FILE_NONBLOCK, FILE_TRUNC
        };

        typedef PredefOption<FILE_FLAGS> access;
        typedef PredefOption<int64_t>    permission;
        typedef PredefOption<bool>       nonewline;

        enum SEEK_FLAGS {
            SEEK_START_, SEEK_CURRENT_, SEEK_END_
        };
        typedef PredefOption< SEEK_FLAGS>   seekop;

        void test_Option() {
            Option      o1{ "op1", (int64_t) 1 };
            Option      o2{ "op2", (double) 3.14 };
            Option      o3{ "op3", "value3" };
            Option      o4{ "op4", true };

            ::printf("op1 %s %d %d\n", o1.v1.data(), o1.v2.index(), (int) get<int64_t>(o1.v2));
            ::printf("op2 %s %d %f\n", o2.v1.data(), o2.v2.index(), get<double>(o2.v2));
            ::printf("op3 %s %d %s\n", o3.v1.data(), o3.v2.index(), get<string_view>(o3.v2).data());
            ::printf("op4 %s %d %d\n", o4.v1.data(), o4.v2.index(), get<bool>(o4.v2));
        }
#endif

        int         errno_;
        char        errorMsg_[128 + JIM_PATH_LEN];
        string_view getErrorMsg() { return errorMsg_; }
        void        resetErrorMsg() { errorMsg_[0] = 0; }
        void        invalidOp() { ::snprintf(errorMsg_, sizeof(errorMsg_), "command invalid"); }
#define PRJ_NOT_IMPLEMENTED  { \
        ::snprintf(errorMsg_, sizeof(errorMsg_), "command not implemented");\
      }

#ifdef PRJ_OS_WIN
         val4<Retval, HANDLE, std::string, int> file_win_maketempfile(std::string_view filePath, bool unlink_file = true, const char* templateD = NULL);
#endif

         int getFileStatusErrorCode(FileStatus& fs) { errno_ = fs.errno_; return errno_; }

        struct Files {
            // ===== Local routines
            bool path_hasDrivePrefix(string_view fileName) {
                return fileName[1] == ':';
            }
            bool path_rootName(string_view fileName) {
                return fileName[0] == '/' || path_hasDrivePrefix(fileName);
            }
            bool path_hereRelative(string_view fileName) {
                return fileName[0] == '.' && fileName[1] != '.';
            }
            bool path_upOne(string_view fileName, int pos = 0) {
                return fileName[pos] == '.' && fileName[pos + 1] == '.';
            }
            bool path_isRelative(string_view fileName) {
                // ./.././ but not /.tom or /file.fil
                if (fileName.find("/../") != string::npos) return true;
                if (fileName.find("/./") != string::npos) return true;
                if (fileName.find("../") == 0) return true;
                if (fileName.find("./") == 0) return true;
                return false;
            }
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
            Retval file_assocate(string_view file, string_view openOp, 
                                 string_view closeOp, string_view readOp, string_view writeOp); // #cppExtNeed
            Retval file_assocate_path(string_view file, string_view openOp, 
                                      string_view closeOp, string_view readOp, 
                                      string_view writeOp); // #cppExtNeed
            Retval file_changed(File& handle); // #cppExtNeed
        };

        // BODY =====================================================================
        val2<Retval, int64_t> file_atime(string_view filename) {
            resetErrorMsg();
            FileStatus fileStatus(filename);
            if (getFileStatusErrorCode(fileStatus)) return val2<Retval, int64_t>{1, -1};

            int64_t ret = fileStatus.values_[FileStatus::ST_ATIME].value_;
            if (getFileStatusErrorCode(fileStatus) != 0) {
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "could not read \"%#s\": %s", filename.data(), ::strerror(errno_));
            }
            return val2<Retval, int64_t>{0, ret};
        }
        Retval file_attributes(string_view filename, vector <Option>& options) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval file_channels(string_view pattern) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval file_copy(string_view source, string_view dest, bool forced, bool destDir) {
            if (!file_exists(source)) {
                ::snprintf(errorMsg_, sizeof(errorMsg_),
                           "could not read \"%#s\": %s", source.data(), ::strerror(errno_));
                return 1;
            }
            if (!file_exists(dest) && !forced && !destDir) {
                ::snprintf(errorMsg_, sizeof(errorMsg_),
                           "could not write \"%#s\": %s", dest.data(), ::strerror(errno_));
                return 1;
            }
            FILE* inFile = fopen(source.data(), "r");

            
            return 1;
        } // #cppExtNeedDef
        Retval file_copy(vector<string_view/*source*/>& sources, string_view dest, bool forced) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        val2<Retval, int64_t> file_ctime(string_view filename) {
            resetErrorMsg();
            FileStatus fileStatus(filename);
            if (getFileStatusErrorCode(fileStatus)) return val2<Retval, int64_t>{1, -1};

            int64_t ret = fileStatus.values_[FileStatus::ST_CTIME].value_;
            if (errno_ != 0) {
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "could not read \"%#s\": %s", filename.data(), strerror(errno_));
            }
            return val2<Retval, int64_t>{0, ret};
        }
        Retval file_delete(string_view pathname, bool forced) {
            resetErrorMsg();
            if (prj_unlink(pathname.data()) == -1 && get_errno() != ENOENT) {
                errno_ = get_errno();
                if (prj_rmdir(pathname.data()) == -1) {
                    errno_ = get_errno();
                    if (!forced) {
                        ::snprintf(errorMsg_, sizeof(errorMsg_), 
                                   "couldn't delete file \"%s\": %s", pathname.data(),
                                 strerror(errno_));
                        return 1;
                    }
                }
            }
            errno_ = get_errno();
            return 0;
        }
        Retval file_delete(vector<string_view/*filename*/>& pathnames, bool forced = false) {
            resetErrorMsg();
            for (string_view str : pathnames) {
                if (file_delete(str, forced)) return 1;
            }
            return 0;
        }
        val2<Retval, string> file_dirname(string_view pathname) {
            resetErrorMsg();
            string path;
            const char* p = ::strrchr(path.data(), '/');

            if (!(p != NULL) && (path[0] == '.') && (path[1] == '.') && (path[2] = '\0')) { // #Review
                return val2<Retval, string>{0, ".."};
            } else if (!p) {
                return val2<Retval, string>{0, "."};
            } else if (p == path.data()) {
                return val2<Retval, string>{0, "/"};
            } else if (isWindows() && p[-1] == ':') {
                return val2<Retval, string>{0, 
                                            string(path.data(), 
                                            (string::size_type) (p - path.data() + 1))}; // #Review
            } else {

            }
            return val2<Retval, string>{0, string(path.data(), (string::size_type)(p - path.data()))};
        }
        bool exists(string_view pathname) {
            resetErrorMsg();
            if (isWindows()) {
                auto ret = prj_access(pathname.data(), 00);
                errno_ = get_errno();
                return ret==0;
            } else {
#ifndef F_OK 
#  define F_OK 1
#endif
                auto ret = prj_access(pathname.data(), F_OK);
                errno_ = get_errno();
                return ret==0;
            }
        }
        bool isReadable(string_view pathname) {
            resetErrorMsg();
            if (isWindows()) {
                auto ret = prj_access(pathname.data(), 04);
                errno_ = get_errno();
                return ret==0;
            } else {
#ifndef R_OK 
#  define R_OK 1
#endif
                auto ret = prj_access(pathname.data(), R_OK);
                errno_ = get_errno();
                return ret==0;
            }
        }
        bool isWritable(string_view pathname) {
            resetErrorMsg();
            if (isWindows()) {
                auto ret = prj_access(pathname.data(), 02);
                errno_ = get_errno();
                return ret==0;
            } else {
#ifndef W_OK 
#  define W_OK 1
#endif
                auto ret = prj_access(pathname.data(), W_OK);
                errno_ = get_errno();
                return ret==0;
            }
        }
        bool isExecutable(string_view pathname) {
            resetErrorMsg();
            if (isWindows()) {
                // TODO How?
                return true;
            } else {
#ifndef X_OK 
#  define X_OK 1
#endif
                auto ret = prj_access(pathname.data(), X_OK);
                errno_ = get_errno();
                return ret==0;
            }
        }
        Retval file_executable(string_view pathname) {
            resetErrorMsg();
            return isExecutable(pathname);
        }
        Retval file_exists(string_view pathname) { resetErrorMsg(); return exists(pathname); }
        val2<Retval, string> file_extension(string_view pathname) {
            resetErrorMsg();
            const char* lastSlash = ::strrchr(pathname.data(), '/');
            const char* p = ::strrchr(pathname.data(), '.');

            if ((p == NULL) || (lastSlash != NULL && lastSlash >= p)) {
                p = "";
            }
            return val2<Retval, string>{0, p};
        }
        Retval file_isdirectory(string_view pathname) {
            resetErrorMsg();
            FileStatus fileStatus(pathname);
            if (getFileStatusErrorCode(fileStatus)) return 1;
            int v = (int) fileStatus.values_[FileStatus::ST_MODE].value_;
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
            resetErrorMsg();
            FileStatus fileStatus(pathname);
            if (getFileStatusErrorCode(fileStatus)) return 1;
            int v = (int) fileStatus.values_[FileStatus::ST_MODE].value_;
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
            resetErrorMsg();
            int i;
            char newname[JIM_PATH_LEN + 1];
            char* last = newname;

            *newname = 0;

            /* Simple implementation for now */
            for (i = 0; i < (int) argv.size(); i++) {
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
                        ::snprintf(errorMsg_, sizeof(errorMsg_), "Path too long");
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
            resetErrorMsg();
            if (!prj_funcDef(prj_link)) { invalidOp(); return 1; }
            auto ret = prj_link(source.data(), dest.data());
            errno_ = get_errno();
            return ret!=0;
        }
        Retval file_link_hard(string_view pathname, vector<string_view/*dest*/>& dest) {
            resetErrorMsg();
            for (string_view& path : dest) {
                if (file_link_hard(pathname, path)) return 1;
            }
            return 0;
        }
        Retval file_link_symbolic(string_view source, string_view& dest) {
            resetErrorMsg();
            if (!prj_funcDef(prj_symlink)) { invalidOp(); return 1; }
            auto ret =  prj_symlink(source.data(), dest.data());
            errno_ = get_errno();
            return ret != 0;
        }
        Retval file_link_symbolic(string_view pathname, vector<string_view/*dest*/>& dest) {
            resetErrorMsg();
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
            resetErrorMsg();
            FileStatus  fileStatus(name);
            file_lstat_struct fieldInfo;
            if (getFileStatusErrorCode(fileStatus)) 
                return val2<Retval, vector<file_lstat_struct>>{1, vector<file_lstat_struct>()};

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
            resetErrorMsg();
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
                errno_ = get_errno();
                if (errno_ == ENOENT) {
                    /* Create the parent and try again */
                    continue;
                }
                /* Maybe it already exists as a directory */
                if (errno_ == EEXIST) {
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
            resetErrorMsg();
            for (string_view& d : dirs) {
                if (file_mkdir(d)) return 1;
            }
            return 0;
        }
        val2<Retval, int64_t> file_mtime(string_view pathname) {
            resetErrorMsg();
            FileStatus fileStatus(pathname);
            if (getFileStatusErrorCode(fileStatus)) return val2<Retval, int64_t>{1, -1};

            int64_t ret = fileStatus.values_[FileStatus::ST_MTIME].value_;
            if (getFileStatusErrorCode(fileStatus)) {
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "could not read \"%#s\": %s", pathname.data(), ::strerror(errno_));
            }
            return val2<Retval, int64_t>{0, ret};
        }
        Retval file_mtime(string_view filename, int64_t us) {
            resetErrorMsg();
            if (prj_funcDef(prj_utimes)) {
                struct prj_timeval times[2];

                times[1].tv_sec = (long) (times[0].tv_sec = (long) (us / 1000000));
                times[1].tv_usec = times[0].tv_usec = us % 1000000;

                if (prj_utimes(filename.data(), times) != 0) {
                    errno_ = get_errno();
                    ::snprintf(errorMsg_, sizeof(errorMsg_), 
                               "can't set time on \"%s\": %s", filename.data(), strerror(errno_));
                    return 1;
                }
                return 0;
            } else {
                invalidOp();
            }
            return 1;
        }
        Retval file_nativename(string_view pathname) { // #cppExtNeedDef
            resetErrorMsg();
            PRJ_NOT_IMPLEMENTED;  
            return 1;
        }
        val2<Retval, string> file_normalize(string_view path) {
            resetErrorMsg();
            if (!prj_funcDef(prj_realpath)) {
                invalidOp();
                return val2<Retval, string>{1, ""};
            }

            char newname[JIM_PATH_LEN + 1];

            if (prj_realpath(path.data(), newname)) {
                return val2<Retval, string>{0, newname};
            } else {
                errno_ = get_errno();
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "can't normalize \"%#s\": %s", path.data(), strerror(errno_));
                return val2<Retval, string>{1, ""};
            }

            return val2<Retval, string>{1, ""};
        }
        Retval file_owned(string_view pathname) {
            resetErrorMsg();
            if (!prj_funcDef(prj_geteuid)) {
                invalidOp();
                return 1;
            }
            if (!file_exists(pathname)) return 1;
            FileStatus fileStatus(pathname);
            if (getFileStatusErrorCode(fileStatus)) return 1;

            return prj_geteuid() == fileStatus.values_[FileStatus::ST_UID].value_;
        }
        Retval file_pathtype(string_view pathname) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval file_readable(string_view pathname) { return isReadable(pathname); }
        val2<Retval, string> file_readlink(string_view pathname) {
            resetErrorMsg();
            char linkValue[JIM_PATH_LEN + 1];
            int linkLength = (int) prj_readlink(pathname.data(), linkValue, sizeof(linkValue));
            if (linkLength == -1) {
                errno_ = get_errno();
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "couldn't readlink \"%#s\": %s", pathname.data(), strerror(errno_));
                return val2<Retval, string>{1, ""};
            }
            return val2<Retval, string>{0, linkValue};
        }
        Retval file_rename(string_view source, string_view target, bool forced) {
            resetErrorMsg();
            if (!forced && file_exists(target)) {
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "error renaming \"%#s\" to \"%#s\": target exists", source.data(),
                         target.data());
                return 1;
            }
            if (::rename(source.data(), target.data()) != 0) {
                errno_ = get_errno();
                ::snprintf(errorMsg_, sizeof(errorMsg_), 
                           "error renaming \"%#s\" to \"%#s\": %s", source.data(), target.data(),
                         strerror(errno_));
            }
            return 0;
        }
        Retval file_rootname(string_view pathname) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval file_seperator(string_view name) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        val2<Retval,int64_t> file_size(string_view name) {
            resetErrorMsg();
            FileStatus fileStatus(name);
            if (getFileStatusErrorCode(fileStatus)) 
                return val2<Retval,int64_t>{ 1, (int64_t)0 };
            int64_t v = (int) fileStatus.values_[FileStatus::ST_SIZE].value_;
            return val2<Retval, int64_t>{ 0, v };
        }
        Retval file_split(string_view name) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval file_stat(string_view name, string_view varName) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval file_system(string_view name) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        val2<Retval, string> file_tail(string_view name) {
            resetErrorMsg();
            const char* lastSlash = ::strrchr(name.data(), '/');
            if (lastSlash == NULL) return val2<Retval, string>{1, ""};
            return val2<Retval, string>{0, (lastSlash + 1)};
        }
#ifdef PRJ_OS_LINUX
#if 0
        val3<Retval, int, string> file_maketempfile(const char* filename_template, bool unlink_file) {
            int fd;
            mode_t mask;
            string filename;

            resetErrorMsg();
            if (filename_template == NULL) {
                const char* tempdir = prj_getenv("TMPDIR");
                if (tempdir == NULL || *tempdir == '\0' || access(tempdir, W_OK) != 0) {
                    tempdir = "/tmp/";
                }
                filename = tempdir;
                if (tempdir[0] && tempdir[strlen(tempdir) - 1] != '/') {
                    filename.append("tcl.tmp.XXXXXX");
                }
            } else {
                filename = filename_template;
            }

            mask = prj_umask(S_IXUSR | S_IRWXG | S_IRWXO);
            errno_ = get_errno();

            char filenameArray[JIM_PATH_LEN];
            strncpy(filenameArray, filename.data(), sizeof(filenameArray));

            if (prj_funcDef(prj_mkstemp)) {
                fd = prj_mkstemp(filenameArray);
                errno_ = get_errno();
            } else {
                if (::mktemp(filenameArray) == NULL) {
                    errno_ = get_errno();
                    fd = -1;
                } else {
                    fd = prj_open(filenameArray, O_RDWR | O_CREAT | O_TRUNC);
                }
            }
            prj_umask(mask);
            errno_ = get_errno();
            if (fd < 0) {
                return val3<Retval, int, string>{1, -1, ""};
            }
            if (unlink_file) {
                ::remove(filenameArray);
                errno_ = get_errno();
            }
            return val3<Retval, int, string>{0, fd, filenameArray};
        }
#endif
#endif
#ifdef PRJ_OS_WIN
        val2<Retval, string> file_temppath() {
            resetErrorMsg();
            char    name[JIM_PATH_LEN];
            auto ret = ::GetTempPathA(sizeof(name), name);
            if (ret == 0) val2<Retval, string>{1, ""};
            return val2<Retval, string>{0, name};
        }
        val2<Retval, string> file_tempname(string_view filePath, const char* templateD = NULL) {
            resetErrorMsg();
            char    name[JIM_PATH_LEN];
            ::strncpy(name, filePath.data(), sizeof(name));
            auto ret = ::GetTempFileNameA(name, templateD ? templateD : "JIM", 0, name);
            if (ret == 0) val2<Retval, string>{1, ""};
            return val2<Retval, string>{1, name};
        }
        val4<Retval, HANDLE, string, int> 
            file_win_maketempfile(string_view filePath, bool unlink_file, const char* templateD) {
            resetErrorMsg();
            string name;
            auto temppath = file_temppath();
            if (temppath.v1 != 0)  return val4<Retval, HANDLE, string, int>{ 1, INVALID_HANDLE_VALUE, "", -1};
            auto tempname = file_tempname(temppath.v2, templateD);
            if (tempname.v1 != 0) return val4<Retval, HANDLE, string, int>{ 1, INVALID_HANDLE_VALUE, "", -1};

            HANDLE handle = CreateFileA(tempname.v2.data(), 
                                        GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                        CREATE_ALWAYS, 
                                        FILE_ATTRIBUTE_TEMPORARY | (unlink_file ? FILE_FLAG_DELETE_ON_CLOSE : 0),
                                        NULL);
            if (handle == INVALID_HANDLE_VALUE) {
                ::DeleteFileA(tempname.v2.data());
                return val4<Retval, HANDLE, string, int>{ 1, INVALID_HANDLE_VALUE, "", -1};
            }
            return val4<Retval, HANDLE, string, int>
                { 0, handle, tempname.v2, _open_osfhandle((intptr_t) handle, _O_RDWR | O_TEXT) };
        }
#endif
        string_view file_type(string_view pathname) {
            FileStatus fileStatus(pathname);
            if (getFileStatusErrorCode(fileStatus)) return "";
            int mode = (int) fileStatus.values_[FileStatus::ST_MODE].value_;

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
        Retval file_volumes(string_view name) { 
            PRJ_NOT_IMPLEMENTED;  
            return 0; 
        } // #cppExtNeedDef
        Retval file_writable(string_view name) { return isWritable(name); }

        // =============================================================
        // File object
        // =============================================================

        // Slight change to Tcl command
        Retval Files::file_open(string_view fileName, string_view access, string_view permissions) { 
            PRJ_NOT_IMPLEMENTED;  
            return 0; 
        } // #cppExtNeedDef
        Retval Files::file_close(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_gets(File& handle, string_view varName) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_read(File& handle, int64_t numChars, bool nonewline) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_puts(File& handle, string_view output, bool newline) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_seek(File& handle, int64_t offset, int origin) { 
            PRJ_NOT_IMPLEMENTED;  
            return 0; 
        } // #cppExtNeedDef
        Retval Files::file_eof(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_flush(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_tell(File& handle, int64_t& offset) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_fconfigure(File& handle, vector< Option>& options) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef

        // Some ideas
        Retval Files::file_readAll(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_readToList(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_assert(Option& option, vector<string_view>& files) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_append(string_view file, string_view data) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_ftail(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_assocate(
            string_view file, string_view openOp, string_view closeOp, 
            string_view readOp, string_view writeOp) { 
            PRJ_NOT_IMPLEMENTED;  
            return  1; 
        } // #cppExtNeedDef
        Retval Files::file_assocate_path(
            string_view file, string_view openOp, string_view closeOp, 
            string_view readOp, string_view writeOp) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef
        Retval Files::file_changed(File& handle) { 
            PRJ_NOT_IMPLEMENTED;  
            return 1; 
        } // #cppExtNeedDef

    }; // namespace prj_wrap::CppFile

}; // end namespace prj_wrap