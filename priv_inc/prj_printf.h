#pragma once
// This is a very thin wrapper around some io functions.
// Prevents me from having to recode all IO from old code, while giving me more control.
//
// It allows for an error handler to be registered to test all these functions.
//      Most of the time programmers never check returns from say "printf()".
// It allows for non-return version of these functions.  
// It also allows me to redirect and/or fork stdin/stdout/stderr to different destinations.

#include <stdint.h>
#include <stdio.h>
#include <io.h>

namespace PrjPrintf {
    typedef void (*handleError)();
    extern handleError g_errorHandler;

    void redirectStdin(FILE* stream);
    void redirectStdout(FILE* stream);
    void redirectStderr(FILE* stream);
    void addStdout(FILE* stream);
    void addStderr(FILE* stream);
    void resetStdin();
    void resetStdout();
    void resetStderr();
    void resetAll();

    int printf(const char* fmt, ...);
    int fprintf(FILE* stream, const char* format, ...);
    int fputs(const char* s, FILE* stream);
    int putchar(int c);

    size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

    size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
    char *fgets(char *s, int size, FILE *stream);

    namespace NoRet {
        void printf(const char* fmt, ...);
        void fprintf(FILE* stream, const char* format, ...);
        void fputs(const char* s, FILE* stream);
        void putchar(int c);

        void fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

        void fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
        void fgets(char* s, int size, FILE* stream);

    }
}; // namespace PrjPrintf