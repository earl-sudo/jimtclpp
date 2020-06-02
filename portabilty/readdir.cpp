#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#if defined(_WIN32)
#   define PRJ_OS_IS WIN32 1
#   define PRJ_OS_WIN32 1
#   define PRJ_OS_WIN 1
#   define PRJ_OS_32BIT 1
#   define PROJ_OS PRJ_WIN32
#elif defined(_WIN64)
#   define PRJ_OS_IS WIN64 1
#   define PRJ_OS_WIN64 1
#   define PRJ_OS_WIN 1
#   define PRJ_OS_64BIT 1
#   define PROJ_OS PRJ_WIN64
#elif defined(__ANDOID__)
#   define PRJ_OS_IS ANDROID
#   define PRJ_OS_ANDOID 1
#   define PROJ_OS PRJ_ANDOID
#elif defined(__linux__)
#   define PRJ_OS_LINUX 1
#   if defined(__x86_64__)
#     define PRJ_OS_IS LINUX64
#     define PRJ_OS_64BIT 1
#     define PROJ_OS PRJ_LINUX64
#   elif defined(__i386__)
#     define PRJ_OS_IS LINUX32
#     define PRJ_OS_32BIT 1
#     define PROJ_OS PRJ_LINUX32
#   endif
#elif defined(__APPLE__)
#   define PRJ_OS_IS MACOS
#   define PRJ_OS_MACOS 1
#   define PROJ_OS PRJ_MACOS
#else
#  warning "unknown os"
#endif

#include <string.h>

#include "readdir.h"

#ifdef PRJ_OS_WIN32
#include <Windows.h>
#include <io.h>

struct Data {
    intptr_t handle;
    struct _finddata_t finddata;
};

#elif defined(PRJ_OS_ANDOID) || defined(PRJ_OS_MACOS) || defined(PRJ_OS_LINUX)
#include <sys/types.h>
#include <dirent.h>

struct Data {
    DIR* handle;
    struct dirent* dirent;
};
#endif

using namespace std;

Readdir::Readdir(string_view dirPath) : initPath_(dirPath) {
    data_ = (void*) calloc(1, sizeof(Data));
    Data* view = (Data*) data_;
#ifdef PRJ_OS_WIN32
    /* search pattern must end with suitable wildcard */
    string path(dirPath);

    const char* all =
        strchr("/\\", path.at(path.length() - 1)) ? "*" : "/*";
    path.append(all);

    view->handle = _findfirst(path.c_str(), &view->finddata);
    if (view->handle == 0) {
        inError_ = true;
    } else {
        inError_ = false;
    }
#elif defined(PRJ_OS_ANDOID) || defined(PRJ_OS_MACOS) || defined(PRJ_OS_LINUX)
    view->handle = opendir(initPath_.c_str());
    if (view->handle != nullptr) {
        view->dirent = readdir(view->handle);
    } else {
        if (view->handle == 0) {
            inError_ = true;
        } else {
            inError_ = false;
        }
    }
#endif
}
Readdir::~Readdir(void) {
#ifdef PRJ_OS_WIN32
#elif defined(PRJ_OS_ANDOID) || defined(PRJ_OS_MACOS) || defined(PRJ_OS_LINUX)
    Data* view = (Data*) data_;
    if (view->handle) closedir(view->handle);
#endif
    free(data_);
}

std::string Readdir::getError() {
    return strerror(errno);
}
string Readdir::nextName() {
    Data* view = (Data*) data_;
    string ret;
#ifdef PRJ_OS_WIN32
    if (view->handle == 0) return "";

    ret = view->finddata.name;
    int fn = _findnext(view->handle, &view->finddata);
    if (fn) /* end of dir*/ { view->handle = 0; }
#elif defined(PRJ_OS_ANDOID) || defined(PRJ_OS_MACOS) || defined(PRJ_OS_LINUX)
    if (view->handle == 0) return "";
    if (view->dirent == nullptr) return "";
    if (view->dirent->d_name == nullptr) return "";
    ret = view->dirent->d_name;
    view->dirent = readdir(view->handle);
#endif
    return ret;
}