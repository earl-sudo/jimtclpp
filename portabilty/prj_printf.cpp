#define _CRT_NONSTDC_NO_WARNINGS 1

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

#include <assert.h>

#include "prj_printf.h"

#include <map>
#include <vector>


namespace PrjPrintf {

    using namespace std;

    bool g_init = false; // #threadIssue
    handleError g_errorHandler;

    vector<FILE*>   g_stdOutVec; // #threadIssue
    vector<FILE*>   g_stdErrVec; // #threadIssue
    FILE*           g_stdIn; // #threadIssue
    vector<FILE*>   g_tmp; // #threadIssue

    static inline vector<FILE*>& getOutput(FILE* fp) {
        if (fp == stdout) return g_stdOutVec;
        if (fp == stderr) return g_stdErrVec;
        g_tmp.clear();
        g_tmp.push_back(fp);
        return g_tmp;
    }
    static inline FILE* getInput(FILE* fp) {
        if (fp == stdin) return g_stdIn;
        return fp;
    }

    static inline void init() {
        g_stdOutVec.push_back(stdout);
        g_stdErrVec.push_back(stderr);
        g_stdIn = stdin;
        //
        g_init = true;
    }

    void redirectStdin(FILE* stream) { 
        if (!g_init) init(); // #lazyInit
        g_stdIn = stream;
    }
    void redirectStdout(FILE* stream) { 
        if (!g_init) init(); // #lazyInit
        g_stdOutVec[0] = stream;
    }
    void redirectStderr(FILE* stream) { 
        if (!g_init) init(); // #lazyInit
        g_stdErrVec[0] = stream;
    }
    void addStdout(FILE* stream) { 
        if (!g_init) init(); // #lazyInit
        g_stdOutVec.push_back(stream);
    }
    void addStderr(FILE* stream) { 
        if (!g_init) init(); // #lazyInit
        g_stdErrVec.push_back(stream);
    }
    void resetStdin() {
        if (!g_init) init(); // #lazyInit
        g_stdIn = stdin;
    }
    void resetStdout() {
        if (!g_init) init(); // #lazyInit
        g_stdOutVec.resize(1);
        g_stdOutVec[0] = stdout;
    }
    void resetStderr() {
        if (!g_init) init(); // #lazyInit
        g_stdErrVec.resize(1);
        g_stdErrVec[0] = stderr;
    }
    void resetAll() {
        resetStdin();
        resetStdout();
        resetStderr();
    }

    int printf(const char* fmt, ...) {
        if (!g_init) init(); // #lazyInit

        int ret;

        if (fmt == nullptr) {
            /* Invalid format pointer */
            ret = -1;
        } else {
            va_list args;
            int len;

            va_start(args, fmt);

            /* Get length of format including arguments */
            len = ::vsnprintf(nullptr, 0, fmt, args);

            if (len < 0) {
                /* vsnprintf failed */
                ret = -1;
            } else {
                /* Declare a character buffer and write the formatted string */
                char  buffer[1024];
                int formatted_size = sizeof(buffer);
                char* formatted = buffer;
                if (len > sizeof(buffer)) {
                    formatted = (char*) malloc(len + 1);
                    formatted_size = len;
                }
                ret = vsnprintf(formatted, formatted_size, fmt, args); // #ignoredRet

                /* Call the wrapped function using the formatted output and return */
                //::fprintf(stderr, "Calling printf with fmt %s", fmt);
                for (auto o : getOutput(stdout)) {
                    ret = ::fprintf(o, formatted);
                    if (ret < 0) break;
                }

                if (len > sizeof(buffer))   free(formatted);
            }

            va_end(args);
        }
        if (ret > 0 && g_errorHandler) g_errorHandler();

        return ret;
    }
    int fprintf(FILE* stream, const char* fmt, ...) {
        if (!g_init) init(); // #lazyInit

        int ret;

        if (fmt == nullptr) {
            /* Invalid format pointer */
            ret = -1;
        } else {
            va_list args;
            int len;

            va_start(args, fmt);

            /* Get length of format including arguments */
            len = ::vsnprintf(nullptr, 0, fmt, args);

            if (len < 0) {
                /* vsnprintf failed */
                ret = -1;
            } else {
                /* Declare a character buffer and write the formatted string */
                char  buffer[1024];
                int formatted_size = sizeof(buffer);
                char* formatted = buffer;
                if (len > sizeof(buffer)) {
                    formatted = (char*) malloc(len + 1);
                    formatted_size = len;
                }
                vsnprintf(formatted, formatted_size, fmt, args);

                /* Call the wrapped function using the formatted output and return */
                //::fprintf(stderr, "Calling printf with fmt %s", fmt);
                for (auto o: getOutput(stream)) {
                    ret = ::fprintf(o, formatted);
                    if (ret < 0) break;
                }

                if (len > sizeof(buffer))   free(formatted);
            }

            va_end(args);
        }
        if (ret > 0 && g_errorHandler) g_errorHandler();

        return ret;
    }
    int fputs(const char* s, FILE* stream) {
        if (!g_init) init(); // #lazyInit
        int ret;
        for (auto o : getOutput(stream)) {
            ret = ::fputs(s, o);
            if (ret < 0) break;
        }
        if (ret < 0 && g_errorHandler) g_errorHandler();
        return ret;
    }
    int putchar(int c) {
        if (!g_init) init(); // #lazyInit
        int ret;
        for (auto o : getOutput(stdout)) {
            ret = ::fputc(c, o);
            if (ret < 0) break;
        }
        if (ret > 0 && g_errorHandler) g_errorHandler();
        return ret;
    }
    size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
        if (!g_init) init(); // #lazyInit
        size_t ret;
        for (auto o : getOutput(stream)) {
            ret = ::fwrite(ptr, size, nmemb, o);
            if (ret != nmemb) break;
        }
        if (ret != (size * nmemb) && g_errorHandler) g_errorHandler();
        return ret;
    }

    size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
        if (!g_init) init(); // #lazyInit
        size_t ret = ::fread(ptr, size, nmemb, getInput(stream));
        if (ret != (size*nmemb) && g_errorHandler) g_errorHandler();
        return ret;
    }
    char* fgets(char* s, int size, FILE* stream) {
        if (!g_init) init(); // #lazyInit
        char* ret = ::fgets(s, size, getInput(stream));
        if (ret == nullptr && g_errorHandler) g_errorHandler();
        return ret;
    }

    int64_t write(int fd, const void* buf, size_t count) {
        if (!g_init) init(); // #lazyInit
        auto ret = ::write(fd, buf, (unsigned int)count);
        if (ret < 0 && g_errorHandler) g_errorHandler();
        return (int64_t)ret;
    }

    namespace NoRet {
        void printf(const char* fmt, ...) {
            if (!g_init) init(); // #lazyInit
            int ret;

            if (fmt == nullptr) {
                /* Invalid format pointer */
                ret = -1;
            } else {
                va_list args;
                int len;

                va_start(args, fmt);

                /* Get length of format including arguments */
                len = ::vsnprintf(nullptr, 0, fmt, args);

                if (len < 0) {
                    /* vsnprintf failed */
                    ret = -1;
                } else {
                    /* Declare a character buffer and write the formatted string */
                    char  buffer[1024];
                    int formatted_size = sizeof(buffer);
                    char* formatted = buffer;
                    if (len > sizeof(buffer)) {
                        formatted = (char*) malloc(len + 1);
                        formatted_size = (len);
                    }
                    vsnprintf(formatted, formatted_size, fmt, args);

                    /* Call the wrapped function using the formatted output and return */
                    //::fprintf(stderr, "Calling printf with fmt %s", fmt);
                    ret = ::PrjPrintf::fprintf(stdout, formatted);

                    if (len > sizeof(buffer))   free(formatted);
                }

                va_end(args);
            }

            assert(ret > 0);
            if (g_errorHandler) g_errorHandler();

            return;
        }        
        void fprintf(FILE* stream, const char* fmt, ...) {
            if (!g_init) init(); // #lazyInit
            int ret;

            if (fmt == nullptr) {
                /* Invalid format pointer */
                ret = -1;
            } else {
                va_list args;
                int len;

                va_start(args, fmt);

                /* Get length of format including arguments */
                len = ::vsnprintf(nullptr, 0, fmt, args);

                if (len < 0) {
                    /* vsnprintf failed */
                    ret = -1;
                } else {
                    /* Declare a character buffer and write the formatted string */
                    char  buffer[1024];
                    int formatted_size = sizeof(buffer);
                    char* formatted = buffer;
                    if (len > sizeof(buffer)) {
                        formatted = (char*) malloc(len + 1);
                        formatted_size = len;
                    }
                    vsnprintf(formatted, formatted_size, fmt, args);

                    /* Call the wrapped function using the formatted output and return */
                    //::fprintf(stderr, "Calling printf with fmt %s", fmt);
                    for (auto o : getOutput(stream)) {
                        ret = ::fprintf(o, formatted);
                        if (ret < 0) break;
                    }

                    if (len > sizeof(buffer))   free(formatted);
                }

                va_end(args);
            }
            assert(ret >= 0);
            if (g_errorHandler) g_errorHandler();

            return;
        }
        void fputs(const char* s, FILE* stream) {
            if (!g_init) init(); // #lazyInit
            int ret = ::PrjPrintf::fputs(s, stream);
            assert(ret >= 0);
        }
        void putchar(int c) {
            if (!g_init) init(); // #lazyInit
            int ret = ::PrjPrintf::putchar(c);
            assert(ret >= 0);

        }
        void fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
            if (!g_init) init(); // #lazyInit
            size_t ret = ::PrjPrintf::fread(ptr, size, nmemb, stream);
            assert(ret != (size * nmemb));
            return;
        }
        void fgets(char* s, int size, FILE* stream) {
            if (!g_init) init(); // #lazyInit
            char* ret = ::PrjPrintf::fgets(s, size, stream);
            assert(ret != nullptr);
            return;
        }
        void fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
            if (!g_init) init(); // #lazyInit
            size_t ret = ::PrjPrintf::fwrite(ptr, size, nmemb, stream);
            assert(ret == nmemb);
            return;
        }
        void write(int fd, const void* buf, size_t count) {
            if (!g_init) init(); // #lazyInit
            int64_t ret = ::PrjPrintf::write(fd, buf, count);
            assert(ret >= 0);
            return;
        }
    }
}; // namespace PrjPrintf