#pragma once

#include <jim-api.h>

BEGIN_JIM_NAMESPACE

struct AioFile;

struct JimAioFopsType {
    int (*writer)(struct AioFile* af, const char* buf, int len);
    int (*reader)(struct AioFile* af, char* buf, int len);
    const char* (*getline)(struct AioFile* af, char* buf, int len);
    int (*error)(const struct AioFile* af);
    const char* (*strerror)(struct AioFile* af);
    int (*verify)(struct AioFile* af);
};

struct AioFile {
    FILE* fp;
    Jim_ObjPtr  filename;
    int type;
    int openFlags;              /* AIO_KEEPOPEN? keep FILE* */
    int fd;
    Jim_ObjPtr  rEvent;
    Jim_ObjPtr  wEvent;
    Jim_ObjPtr  eEvent;
    int addr_family;
    void* ssl;
    const JimAioFopsType* fops;
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_AioFile         Jim_TAllocZ<AioFile>()
#define free_AioFile(ptr)   Jim_TFree<AioFile>(ptr)

const char* stdio_strerror(struct AioFile* af);

END_JIM_NAMESPACE
