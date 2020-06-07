#pragma once

/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

 // A fairly flexible log facility based on "rxi" logging facility code.
 //
 // Options:
 // Totally remove log calls at compile time with DO_NOT_TRACE.
 // Totally turn off log calls at run time with "log_toggle(nullptr);"
 // Change what is logged log_add_filter()/log_remove_filter().
 // Turn on/off ASCII coloring messages "log_set_usecolor()"
 // Turn off/on all output "log_set_quiet()"
 // Change logging level "log_set_level()"
 // Change where logging goes to "log_set_fp()"

#include <stdio.h>
#include <stdarg.h>

#include <functional>

namespace PrjLogger {

    using namespace std;

    enum LEVELS { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

    typedef void (*log_LockFn)(void* udata, int lock);
    typedef bool (*logfilerFunc)(LEVELS level, const char* topics, const char* file, const char* function, int line);


#define PRJTRACE(LEVEL, TOPICS, FMT, ...)  ::PrjLogger::log(LEVEL, TOPICS, __FILE__, __FUNCTION__, __LINE__, FMT, __VA_ARGS__)

    typedef void (*logFunc)(LEVELS level, const char* topics, const char* file, const char* function, int line, const char* fmt, ...);
    extern logFunc g_logger;

#ifndef DO_NOT_TRACE
#define log_trace(TAG,...) do { if (::PrjLogger::g_logger) ::PrjLogger::g_logger(::PrjLogger::LOG_TRACE, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define log_debug(TAG,...) do { if (::PrjLogger::g_logger) ::PrjLogger::g_logger(::PrjLogger::LOG_DEBUG, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define log_info(TAG,...)  do { if (::PrjLogger::g_logger) ::PrjLogger::g_logger(::PrjLogger::LOG_INFO, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define log_warn(TAG,...)  do { if (::PrjLogger::g_logger) ::PrjLogger::g_logger(::PrjLogger::LOG_WARN, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define log_error(TAG,...) do { if (::PrjLogger::g_logger) ::PrjLogger::g_logger(::PrjLogger::LOG_ERROR, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#define log_fatal(TAG,...) do { if (::PrjLogger::g_logger) ::PrjLogger::g_logger(::PrjLogger::LOG_FATAL, TAG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); } while(0)
#else
#define log_trace(...) 
#define log_debug(...) 
#define log_info(...)  
#define log_warn(...)  
#define log_error(...) 
#define log_fatal(...) 
#endif

    void log(LEVELS level, const char* topics, const char* file, const char* function, int line, const char* fmt, ...);

    void log_toggle(logFunc logFunc = log);
    void log_set_udata(void* udata);
    void log_set_lock(log_LockFn fn);
    void log_set_fp(FILE* fp);
    void log_set_level(int level);
    void log_set_quiet(int enable);
    void log_set_usecolor(bool enable);
    void log_add_filter(logfilerFunc func);
    void log_remove_filter(logfilerFunc func);
    void log_set_usefullpath(bool enable);
    void log_use_time(bool enable);
    void log_show_topics(bool enable);

}; // namespace PrjLogger

