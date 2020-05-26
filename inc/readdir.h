#pragma once

#include <stdint.h>
#include <string>
#include <string_view>

#if 0

// Posix type of directory listing.

struct dirent* readdir(DIR* dirp);
DIR* opendir(const char* name);
struct dirent {
    ino_t          d_ino;       /* Inode number */
    off_t          d_off;       /* Not an offset; see below */
    unsigned short d_reclen;    /* Length of this record */
    unsigned char  d_type;      /* Type of file; not supported
                                   by all filesystem types */
    char           d_name[256]; /* Null-terminated filename */
};

// Windows directory listing
int _findnext(intptr_t handle, struct _finddata_t* fileinfo);
intptr_t _findfirst(const char* filespec,struct _finddata_t* fileinfo);

struct _finddata64i32_t {
    unsigned    attrib;
    __time64_t  time_create;    // -1 for FAT file systems
    __time64_t  time_access;    // -1 for FAT file systems
    __time64_t  time_write;
    _fsize_t    size;
    char        name[260];
};
#endif

struct Readdir {
private:
    const std::string initPath_;
    void* data_; 
public:
    Readdir(std::string_view dirPath);
    ~Readdir(void);
    std::string nextName();
    bool inError_ = true;
    std::string getError();
};