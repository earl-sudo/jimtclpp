#pragma once

enum PRJ_COMPILERS {
    PRJ_MSVC, PRJ_CLANG, PRJ_GCC, PRJ_MINGW32, PRJ_MINGW64
};

// Discover the Compiler we are using.
#if defined(_MSC_VER)
#  define PRJ_COMPILER_IS PRJ_MSVC
#  define PRJ_COMPILER_MSVC 1
#  define PRJ_COMPILER PRJ_MSVC
#elif defined(__GNUC__)
#  define PRJ_COMPILER_IS GCC
#  define PRJ_COMPILER_GCC 1
#  define PRJ_COMPILER PRJ_GCC
#elif defined(__clang) 
#  define PRJ_COMPILER_IS CLANG
#  define PRJ_COMPILER_CLANG 1
#  define PRJ_COMPILER PRJ_CLANG
#elif defined(__MINGW32__)
#  define PRJ_COMPILER_MINGW 1
#  define PRJ_COMPILER_IS MINGW32 
#  define PRJ_COMPILER_MINGW32 1
#  define PRJ_COMPILER PRJ_MINGW32
#elif defined(__MINGW64)
#  define PRJ_COMPILER_MINGW 1
#  define PRJ_COMPILER_IS MINGW64
#  define PRJ_COMPILER_MINGW64
#  define PRJ_COMPILER PRJ_MINGW64
#else
#  warning "Unknown compiler"
#endif

extern PRJ_COMPILERS g_prj_compiler; // Allow for runtime check of compiler

enum PRJ_OS {
    PRJ_WIN32, PRJ_WIN64, PRJ_ANDOID, PRJ_LINUX32, PRJ_LINUX64, PRJ_MACOS
};

// Discover the OS we are compiling for.
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

// Turn on/off compiler/os portablity issues.
#ifdef PRJ_OS_LINUX
#  include <linux_gcc_1.h>
#elif PRJ_OS_MACOS
#  include <macos_gcc_1.h>
#elif PRJ_OS_WIN
#  ifdef PRJ_COMPILER_MINGW
#    include <win_mingw64_1.h>
#  elif PRJ_COMPILER_MSVC
#    include <win_msvc_1.h>
#  endif
#else
#  warning "no known configuration"
#endif

// Turn on/off jim extensions.
#include <jim_max_exts.h>

// Turn on/off jim features.
#include <jim_norm_features.h>

// Make some standard C headers global.
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

