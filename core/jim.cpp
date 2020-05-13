/* Jim - A small embeddable Tcl interpreter
 *
 * Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
 * Copyright 2005 Clemens Hintze <c.hintze@gmx.net>
 * Copyright 2005 patthoyts - Pat Thoyts <patthoyts@users.sf.net>
 * Copyright 2008,2009 oharboe - Øyvind Harboe - oyvind.harboe@zylin.com
 * Copyright 2008 Andrew Lunn <andrew@lunn.ch>
 * Copyright 2008 Duane Ellis <openocd@duaneellis.com>
 * Copyright 2008 Uwe Klein <uklein@klein-messgeraete.de>
 * Copyright 2008 Steve Bennett <steveb@workware.net.au>
 * Copyright 2009 Nico Coesel <ncoesel@dealogic.nl>
 * Copyright 2009 Zachary T Welch zw@superlucidity.net
 * Copyright 2009 David Brownell
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE JIM TCL PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * JIM TCL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the Jim Tcl Project.
 **/
#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#define JIM_OPTIMIZATION        /* comment to avoid optimizations and reduce size */
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE           /* Mostly just for environ */
#endif

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <setjmp.h>


#if defined(_DEBUG)
#  if defined(_MSC_VER)
#    include <intrin.h>
#    define BREAKPOINT __debugbreak()
#  endif
#  if defined(PRJ_COMPILER_GCC)
#  define BREAKPOINT __builtin_trap()
#  endif
#else
#  define BREAKPOINT 
#endif


#include <jim.h>
#include <jimautoconf.h>
#include <utf8.h>
#include <prj_trace.h>

#ifdef HAVE_SYS_TIME_H // #optionalCode #WinOff
#  include <sys/time.h>
#endif
#ifdef HAVE_CRT_EXTERNS_H // #optionalCode #WinOff
#  include <crt_externs.h>
#endif

#ifdef _WIN32 // #optionalCode
#  undef UNICODE
#  undef _UNICODE
#  ifndef STRICT
#    define STRICT
#  endif
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#  include <windows.h>
#  include <io.h>
#  include <stdlib.h>
#  include <stdio.h>
#  include <process.h>
#  include <errno.h>
#endif

/* A platform compatibility layer. */
#include <prj_compat.h>

/* For INFINITY, even if math functions are not enabled */
#include <math.h>


#ifndef JIM_INLINE_API_SMALLFUNCS // #optionalCode 
#  include <jim-inc.h>
#endif

BEGIN_JIM_NAMESPACE


#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const int version[] = { MAJOR , MINOR };
#include <jimtclpp-version.h>

/* We may decide to switch to using $[...] after all, so leave it as an option */
int g_EXPRSUGAR_BRACKET = 0;

/* For the no-autoconf case */
#ifndef TCL_LIBRARY
#  define TCL_LIBRARY "."
#endif
#ifndef TCL_PLATFORM_OS
#  define TCL_PLATFORM_OS "unknown"
#endif
#ifndef TCL_PLATFORM_PLATFORM
#  define TCL_PLATFORM_PLATFORM "unknown"
#endif
#ifndef TCL_PLATFORM_PATH_SEPARATOR
#  define TCL_PLATFORM_PATH_SEPARATOR ":"
#endif

int g_DEBUG_SHOW_SCRIPT = 0;
int g_DEBUG_SHOW_SCRIPT_TOKENS = 0;
int g_DEBUG_SHOW_SUBST = 0;
int g_DEBUG_SHOW_EXPR = 0;
int g_JIM_DEBUG_GC = 0;
#ifdef JIM_MAINTAINER
int  g_JIM_MAINTAINER_VAL = 1;
#else
int g_JIM_MAINTAINER_VAL = 0;
#endif
#ifdef JIM_BOOTSTRAP
int g_JIM_BOOTSTRAP_VAL = 1;
#else
int g_JIM_BOOTSTRAP_VAL = 0;
#endif
#ifdef JIM_RANDOMISE_HASH 
int g_JIM_RANDOMISE_HASH_VAL = 1;
#else
int g_JIM_RANDOMISE_HASH_VAL = 0;
#endif
#ifdef JIM_SPRINTF_DOUBLE_NEEDS_FIX
int g_JIM_SPRINTF_DOUBLE_NEEDS_FIX_VAL = 1;
#else
int g_JIM_SPRINTF_DOUBLE_NEEDS_FIX_VAL = 0;
#endif
#ifdef DEBUG_SHOW_EXPR_TOKENS
int g_DEBUG_SHOW_EXPR_TOKENS_VAL = 1;
#else
int g_DEBUG_SHOW_EXPR_TOKENS_VAL = 0;
#endif
#ifdef g_JIM_DEBUG_PANIC
int g_JIM_DEBUG_PANIC_VAL = 1;
#define JimPanic(X) JimPanicDump X
#else
int g_JIM_DEBUG_PANIC_VAL = 0;
#  define JimPanic(X) 
#endif
#ifdef JIM_UTF8
int g_JIM_UTF8_VAL = 1;
#else
int g_JIM_UTF8_VAL = 0;
#endif

int g_JIM_DEBUG_COMMAND = g_JIM_MAINTAINER_VAL;
int g_JIM_DEBUG_PANIC = g_JIM_MAINTAINER_VAL;

/* Enable this (in conjunction with valgrind) to help debug
 * reference counting issues
 */
int g_JIM_DISABLE_OBJECT_POOL = 0;

/* Maximum size of an integer */
enum { JIM_INTEGER_SPACE = 24 };  // #MagicNum

const char *jim_tt_name(int type);

static void JimPanicDump(int fail_condition, const char *fmt, ...);

#ifdef JIM_WIDE_4BYTE
#    define JIM_WIDE_MIN LONG_MIN
#    define JIM_WIDE_MAX LONG_MAX
#endif
#ifdef JIM_WIDE_8BYTE
#    define JIM_WIDE_MIN LLONG_MIN
#    define JIM_WIDE_MAX LLONG_MAX
#endif

#ifdef JIM_OPTIMIZATION // #optionalCode
#  define JIM_IF_OPTIM(X) X
int g_JIM_OPTIMIZATION_VAL = 1;
#else
#  define JIM_IF_OPTIM(X)
int g_JIM_OPTIMIZATION_VAL = 0;
#endif

/* -----------------------------------------------------------------------------
 * Global variables
 * ---------------------------------------------------------------------------*/

/* A shared empty string for the objects string representation.
 * Jim_InvalidateStringRep knows about it and doesn't try to free it. */
static char g_JimEmptyStringRep[] = "";

/* -----------------------------------------------------------------------------
 * Required prototypes of not exported functions
 * ---------------------------------------------------------------------------*/
STATIC void JimFreeCallFrame(Jim_InterpPtr interp, Jim_CallFramePtr cf, int action);
STATIC Retval ListSetIndex(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int listindex, Jim_ObjPtr newObjPtr,
    int flags);
static Retval JimDeleteLocalProcs(Jim_InterpPtr interp, Jim_StackPtr localCommands);
STATIC Jim_ObjPtr JimExpandDictSugar(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
STATIC void SetDictSubstFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
static Jim_ObjArray *JimDictPairs(Jim_ObjPtr dictPtr, int *len);
static void JimSetFailedEnumResult(Jim_InterpPtr interp, const char *arg, const char *badtype,
    const char *prefix, const char *const *tablePtr, const char *name);
STATIC Retval JimCallProcedure(Jim_InterpPtr interp, Jim_Cmd *cmd, int argc, Jim_ObjConstArray argv);
static Retval JimGetWideNoErr(Jim_InterpPtr interp, Jim_ObjPtr objPtr, jim_wide * widePtr);
static int JimSign(jim_wide w);
static Retval JimValidName(Jim_InterpPtr interp, const char *type, Jim_ObjPtr nameObjPtr);
static void JimPrngSeed(Jim_InterpPtr interp, unsigned_char *seed, int seedLen);
static void JimRandomBytes(Jim_InterpPtr interp, void *dest, unsigned_int len);


/* Fast access to the int (wide) value of an object which is known to be of int type */
static inline jim_wide JimWideValue(Jim_ObjPtr  objPtr) { return (objPtr)->getWideValue(); }

static inline const char* JimObjTypeName(Jim_ObjPtr  O) { return ((O)->typePtr() ? (O)->typePtr()->name : "none"); }

static int utf8_tounicode_case(const char *s, int *uc, int upper)
{
    PRJ_TRACE;
    int l = utf8_tounicode(s, uc);
    if (upper) {
        *uc = utf8_upper(*uc);
    }
    return l;
}

/* Short access functions */
JIM_EXPORT int  Jim_RefCount(Jim_ObjPtr  objPtr) { return objPtr->refCount(); }
JIM_EXPORT Jim_CallFramePtr  Jim_TopCallFrame(Jim_InterpPtr  interp) {
    return interp->topFramePtr();
}
JIM_EXPORT Jim_ObjPtr  Jim_CurrentNamespace(Jim_InterpPtr  interp) { return interp->framePtr()->nsObj_; }
JIM_EXPORT Jim_ObjPtr  Jim_EmptyObj(Jim_InterpPtr  interp) { return interp->emptyObj(); }
JIM_EXPORT int Jim_CurrentLevel(Jim_InterpPtr  interp) { return interp->framePtr()->level(); }
JIM_EXPORT Jim_HashTablePtr  Jim_PackagesHT(Jim_InterpPtr  interp) { return interp->getPackagesPtr(); } 
JIM_EXPORT const char* Jim_KeyAsStr(Jim_HashEntryPtr  he) { return he->keyAsStr(); }
JIM_EXPORT const void* Jim_KeyAsVoid(Jim_HashEntryPtr  he) { return he->keyAsVoid(); }
JIM_EXPORT void Jim_IncrStackTrace(Jim_InterpPtr  interp) { interp->incrAddStackTrace(); }

/* These can be used in addition to JIM_CASESENS/JIM_NOCASE */
enum JIM_CHARSET_BITS { 
    JIM_CHARSET_SCAN = 2,
    JIM_CHARSET_GLOB = 0
};

/**
 * pattern points to a string like "[^a-z\ub5]"
 *
 * The pattern may contain trailing chars, which are ignored.
 *
 * The pattern is matched against unicode char 'c'.
 *
 * If (flags & JIM_NOCASE), case is ignored when matching.
 * If (flags & JIM_CHARSET_SCAN), the considers ^ and ] special at the start
 * of the charset, per scan, rather than glob/string match.
 *
 * If the unicode char 'c' matches that set, returns a pointer to the ']' character,
 * or the null character if the ']' is missing.
 *
 * Returns NULL on no match.
 */
static const char *JimCharsetMatch(const char *pattern, int c, int flags)
{
    PRJ_TRACE;
    int not_ = 0;
    int pchar;
    int match = 0;
    int nocase = 0;

    if (flags & JIM_NOCASE) {
        nocase++;
        c = utf8_upper(c);
    }

    if (flags & JIM_CHARSET_SCAN) {
        if (*pattern == '^') {
            not_++;
            pattern++;
        }

        /* Special case. If the first char is ']', it is part of the set */
        if (*pattern == ']') {
            goto first;
        }
    }

    while (*pattern && *pattern != ']') {
        /* Exact match */
        if (pattern[0] == '\\') {
first:
            pattern += utf8_tounicode_case(pattern, &pchar, nocase);
        }
        else {
            /* Is this a range? a-z */
            int start;
            int end;

            pattern += utf8_tounicode_case(pattern, &start, nocase);
            if (pattern[0] == '-' && pattern[1]) {
                /* skip '-' */
                pattern++;
                pattern += utf8_tounicode_case(pattern, &end, nocase);

                /* Handle reversed range too */
                if ((c >= start && c <= end) || (c >= end && c <= start)) {
                    match = 1;
                }
                continue;
            }
            pchar = start;
        }

        if (pchar == c) {
            match = 1;
        }
    }
    if (not_) {
        match = !match;
    }

    return match ? pattern : NULL;
}

/* Glob-style pattern matching. */

/* Note: string *must* be valid UTF-8 sequences
 */
static int JimGlobMatch(const char *pattern, const char *string, int nocase)
{
    PRJ_TRACE;
    int c;
    int pchar;
    while (*pattern) {
        switch (pattern[0]) {
            case '*':
                while (pattern[1] == '*') {
                    pattern++;
                }
                pattern++;
                if (!pattern[0]) {
                    return 1;   /* match */
                }
                while (*string) {
                    /* Recursive call - Does the remaining pattern match anywhere? */
                    if (JimGlobMatch(pattern, string, nocase))
                        return 1;       /* match */
                    string += utf8_tounicode(string, &c);
                }
                return 0;       /* no match */

            case '?':
                string += utf8_tounicode(string, &c);
                break;

            case '[': {
                    string += utf8_tounicode(string, &c);
                    pattern = JimCharsetMatch(pattern + 1, c, nocase ? JIM_NOCASE : 0);
                    if (!pattern) {
                        return 0;
                    }
                    if (!*pattern) {
                        /* Ran out of pattern (no ']') */
                        continue;
                    }
                    break;
                }
            case '\\':
                if (pattern[1]) {
                    pattern++;
                }
                /* fall through */
            default:
                string += utf8_tounicode_case(string, &c, nocase);
                utf8_tounicode_case(pattern, &pchar, nocase);
                if (pchar != c) {
                    return 0;
                }
                break;
        }
        pattern += utf8_tounicode_case(pattern, &pchar, nocase);
        if (!*string) {
            while (*pattern == '*') {
                pattern++;
            }
            break;
        }
    }
    if (!*pattern && !*string) {
        return 1;
    }
    return 0;
}

/**
 * string comparison. Works on binary data.
 *
 * Returns -1, 0 or 1
 *
 * Note that the lengths are byte lengths, not char lengths.
 */
static int JimStringCompare(const char *s1, int l1, const char *s2, int l2)
{
    PRJ_TRACE;
    if (l1 < l2) {
        return memcmp(s1, s2, l1) <= 0 ? -1 : 1;
    }
    else if (l2 < l1) {
        return memcmp(s1, s2, l2) >= 0 ? 1 : -1;
    }
    else {
        return JimSign(memcmp(s1, s2, l1));
    }
}

/**
 * Compare null terminated strings, up to a maximum of 'maxchars' characters,
 * (or end of string if 'maxchars' is -1).
 *
 * Returns -1, 0, 1 for s1 < s2, s1 == s2, s1 > s2 respectively.
 *
 * Note: does not support embedded nulls.
 */
static int JimStringCompareLen(const char *s1, const char *s2, int maxchars, int nocase)
{
    PRJ_TRACE;
    while (*s1 && *s2 && maxchars) {
        int c1, c2;
        s1 += utf8_tounicode_case(s1, &c1, nocase);
        s2 += utf8_tounicode_case(s2, &c2, nocase);
        if (c1 != c2) {
            return JimSign(c1 - c2);
        }
        maxchars--;
    }
    if (!maxchars) {
        return 0;
    }
    /* One string or both terminated */
    if (*s1) {
        return 1;
    }
    if (*s2) {
        return -1;
    }
    return 0;
}

/* Search for 's1' inside 's2', starting to search from char 'index' of 's2'.
 * The index of the first occurrence of s1 in s2 is returned.
 * If s1 is not found inside s2, -1 is returned.
 *
 * Note: Lengths and return value are in bytes, not chars.
 */
static int JimStringFirst(const char *s1, int l1, const char *s2, int l2, int idx)
{
    PRJ_TRACE;
    int i;
    int l1bytelen;

    if (!l1 || !l2 || l1 > l2) {
        return -1;
    }
    if (idx < 0)
        idx = 0;
    s2 += utf8_index(s2, idx);

    l1bytelen = utf8_index(s1, l1);

    for (i = idx; i <= l2 - l1; i++) {
        int c;
        if (memcmp(s2, s1, l1bytelen) == 0) {
            return i;
        }
        s2 += utf8_tounicode(s2, &c);
    }
    return -1;
}

/* Search for the last occurrence 's1' inside 's2', starting to search from char 'index' of 's2'.
 * The index of the last occurrence of s1 in s2 is returned.
 * If s1 is not found inside s2, -1 is returned.
 *
 * Note: Lengths and return value are in bytes, not chars.
 */
static int JimStringLast(const char *s1, int l1, const char *s2, int l2)
{
    PRJ_TRACE;
    const char *p;

    if (!l1 || !l2 || l1 > l2)
        return -1;

    /* Now search for the needle */
    for (p = s2 + l2 - 1; p != s2 - 1; p--) {
        if (*p == *s1 && memcmp(s1, p, l1) == 0) {
            int64_t ret = (p - s2);
            return testConv<int64_t,int>(ret);
        }
    }
    return -1;
}

/**
 * Per JimStringLast but lengths and return value are in chars, not bytes.
 */
static int JimStringLastUtf8(const char *s1, int l1, const char *s2, int l2) // #UTF8Specific
{
    PRJ_TRACE;
    int n = JimStringLast(s1, utf8_index(s1, l1), s2, utf8_index(s2, l2));
    if (n > 0) {
        n = utf8_strlen(s2, n);
    }
    return n;
}

/**
 * After an strtol()/strtod()-like conversion,
 * check whether something was converted and that
 * the only thing left is white space.
 *
 * Returns JIM_OK or JIM_ERR.
 */
static Retval JimCheckConversion(const char *str, const char *endptr)
{
    PRJ_TRACE;
    if (str[0] == '\0' || str == endptr) {
        return JIM_ERR;
    }

    if (endptr[0] != '\0') {
        while (*endptr) {
            if (!isspace(UCHAR(*endptr))) {
                return JIM_ERR;
            }
            endptr++;
        }
    }
    return JIM_OK;
}

/* Parses the front of a number to determine its sign and base.
 * Returns the index to start parsing according to the given base
 */
static int JimNumberBase(const char *str, int *base, int *sign)
{
    PRJ_TRACE;
    int i = 0;

    *base = 10;

    while (isspace(UCHAR(str[i]))) {
        i++;
    }

    if (str[i] == '-') {
        *sign = -1;
        i++;
    }
    else {
        if (str[i] == '+') {
            i++;
        }
        *sign = 1;
    }

    if (str[i] != '0') {
        /* base 10 */
        return 0;
    }

    /* We have 0<x>, so see if we can convert it */
    switch (str[i + 1]) {
        case 'x': case 'X': *base = 16; break;
        case 'o': case 'O': *base = 8; break;
        case 'b': case 'B': *base = 2; break;
        default: return 0;
    }
    i += 2;
    /* Ensure that (e.g.) 0x-5 fails to parse */
    if (str[i] != '-' && str[i] != '+' && !isspace(UCHAR(str[i]))) {
        /* Parse according to this base */
        return i;
    }
    /* Parse as base 10 */
    *base = 10;
    return 0;
}

/* Converts a number as per strtol(..., 0) except leading zeros do *not*
 * imply octal. Instead, decimal is assumed unless the number begins with 0x, 0o or 0b
 */
static long jim_strtol(const char *str, char **endptr)
{
    PRJ_TRACE;
    int sign;
    int base;
    int i = JimNumberBase(str, &base, &sign);

    if (base != 10) {
        long value = strtol(str + i, endptr, base);
        if (endptr == NULL || *endptr != str + i) {
            return value * sign;
        }
    }

    /* Can just do a regular base-10 conversion */
    return strtol(str, endptr, 10);
}


/* Converts a number as per strtoull(..., 0) except leading zeros do *not*
 * imply octal. Instead, decimal is assumed unless the number begins with 0x, 0o or 0b
 */
static jim_wide jim_strtoull(const char *str, char **endptr)
{
    PRJ_TRACE;
    if (sizeof(jim_wide) == 8) {
        int sign;
        int base;
        int i = JimNumberBase(str, &base, &sign);

        if (base != 10) {
            jim_wide value = strtoull(str + i, endptr, base); // #NonPortFuncFix 
            if (endptr == NULL || *endptr != str + i) {
                return value * sign;
            }
        }

        /* Can just do a regular base-10 conversion */
        return strtoull(str, endptr, 10); // #NonPortFuncFix 
    } else if (sizeof(jim_wide) == 4) {
        return (unsigned_long) jim_strtol(str, endptr);
    }
}

JIM_EXPORT Retval Jim_StringToWide(const char *str, jim_wide *widePtr, int base)
{
    PRJ_TRACE;
    char *endptr;

    if (base) {
        if (sizeof(jim_wide) == 8) {
            *widePtr = strtoull(str, &endptr, base); // #NonPortFunc 
        } else if (sizeof(jim_wide) == 4) {
            *widePtr = strtol(str, &endptr, base);
        }
    }
    else {
        *widePtr = jim_strtoull(str, &endptr);
    }

    return JimCheckConversion(str, endptr);
}

static Retval Jim_StringToDouble(const char *str, double *doublePtr)
{
    PRJ_TRACE;
    char *endptr;

    /* Callers can check for underflow via ERANGE */
    errno = 0;

    *doublePtr = strtod(str, &endptr);

    return JimCheckConversion(str, endptr);
}

static jim_wide JimPowWide(jim_wide b, jim_wide e)
{
    PRJ_TRACE;
    jim_wide res = 1;

    /* Special cases */
    if (b == 1) {
        /* 1 ^ any = 1 */
        return 1;
    }
    if (e < 0) {
        if (b != -1) {
            return 0;
        }
        /* Only special case is -1 ^ -n
         * -1^-1 = -1
         * -1^-2 = 1
         * i.e. same as +ve n
         */
        e = -e;
    }
    while (e)
    {
        if (e & 1) {
            res *= b;
        }
        e >>= 1;
        b *= b;
    }
    return res;
}

/* -----------------------------------------------------------------------------
 * Special functions
 * ---------------------------------------------------------------------------*/
static void JimPanicDump(int condition, const char *fmt, ...)
{
    PRJ_TRACE;
    va_list ap;

    if (!g_JIM_DEBUG_PANIC_VAL) {
        return;
    }
    if (!condition) {
        return;
    }

    va_start(ap, fmt);

    fprintf(stderr, "\nJIM INTERPRETER PANIC: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n\n");
    va_end(ap);

    if (prj_funcDef(prj_backtrace)) // #Unsupported #NonPortFuncFix
    { 
        void *array[40]; // #MagicNum
        int size, i;
        char **strings;

        size = prj_backtrace(array, 40); // #MagicNum
        strings = prj_backtrace_symbols(array, size);
        for (i = 0; i < size; i++)
            fprintf(stderr, "[backtrace] %s\n", strings[i]);
        fprintf(stderr, "[backtrace] Include the above lines and the output\n");
        fprintf(stderr, "[backtrace] of 'nm <executable>' in the bug report.\n");
    }

    exit(1);
}

/* -----------------------------------------------------------------------------
 * Memory allocation
 * ---------------------------------------------------------------------------*/

JIM_EXPORT void *Jim_Alloc(int size)
{
    return size ? malloc(size) : NULL;
}

JIM_EXPORT void Jim_Free(void *ptr)
{
    free(ptr);
}

JIM_EXPORT void *Jim_Realloc(void *ptr, int size)
{
    return realloc(ptr, size);
}

JIM_EXPORT char *Jim_StrDup(const char *s)
{
    return prj_strdup(s);
}

JIM_EXPORT char *Jim_StrDupLen(const char *s, int l)
{
    char *copy = new_CharArray(l + 1); // #AllocF 

    memcpy(copy, s, l + 1);
    copy[l] = 0;                /* Just to be sure, original could be substring */
    return copy;
}

/* -----------------------------------------------------------------------------
 * Time related functions
 * ---------------------------------------------------------------------------*/

/* Returns current time in microseconds */
static jim_wide JimClock(void)
{
    PRJ_TRACE;
    struct prj_timeval tv;

    prj_gettimeofday(&tv, NULL); // #NonPortFuncFix
    return (jim_wide) tv.tv_sec * 1000000 + tv.tv_usec;
}

/* -----------------------------------------------------------------------------
 * Hash Tables
 * ---------------------------------------------------------------------------*/

/* -------------------------- private prototypes ---------------------------- */
static void JimExpandHashTableIfNeeded(Jim_HashTablePtr ht);
static unsigned_int JimHashTableNextPower(unsigned_int size);
STATIC Jim_HashEntryPtr JimInsertHashEntry(Jim_HashTablePtr ht, const void *key, int replace);

/* -------------------------- hash functions -------------------------------- */

/* Thomas Wang's 32 bit Mix Function */
static unsigned_int Jim_IntHashFunction(unsigned_int key)
{
    PRJ_TRACE;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

/* Generic hash function (we are using to multiply by 9 and add the byte
 * as Tcl) */
static unsigned_int Jim_GenHashFunction(const_unsigned_char *buf, int len)
{
    PRJ_TRACE;
    unsigned_int h = 0;

    while (len--)
        h += (h << 3) + *buf++;
    return h;
}

/* ----------------------------- API implementation ------------------------- */

/* reset a hashtable already initialized */
STATIC void JimResetHashTable(Jim_HashTablePtr ht)
{
    PRJ_TRACE;
    ht->table_ = NULL;
    ht->size_ = 0;
    ht->sizemask_ = 0;
    ht->used_ = 0;
    ht->collisions_ = 0;
    if (g_JIM_RANDOMISE_HASH_VAL) {
        /* This is initialized to a random value to avoid a hash collision attack.
         * See: n.runs-SA-2011.004
         */
        ht->uniq_ = (rand() ^ static_cast<unsigned_int>(time(NULL)) ^ clock()); // #NonPortFunc
    } else {
        ht->uniq_ = 0;
    }
}

STATIC void JimInitHashTableIterator(Jim_HashTablePtr ht, Jim_HashTableIterator *iter)
{
    PRJ_TRACE;
    iter->ht = ht;
    iter->index_ = -1;
    iter->entry_ = NULL;
    iter->nextEntry_ = NULL;
}

/* Initialize the hash table */
JIM_EXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht, const Jim_HashTableType *type, void *privdata)
{
    PRJ_TRACE;
    JimResetHashTable(ht);
    ht->type_ = type;
    ht->privdata_ = privdata;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_CREATE, __FUNCTION__, ht);
    return JIM_OK;
}

/* Resize the table to the minimal size that contains all the elements,
 * but with the invariant of a USER/BUCKETS ration near to <= 1 */
JIM_EXPORT void Jim_ResizeHashTable(Jim_HashTablePtr ht) // #MissInCoverage
{
    PRJ_TRACE;
    int minimal = ht->used();

    if (minimal < JIM_HT_INITIAL_SIZE)
        minimal = JIM_HT_INITIAL_SIZE;
    Jim_ExpandHashTable(ht, minimal);
}

/* Expand or create the hashtable */
JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size)
{
    PRJ_TRACE;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_RESIZE_PRE, __FUNCTION__, ht);

    Jim_HashTable n;            /* the new hashtable */
    unsigned_int realsize = JimHashTableNextPower(size), i;

    /* the size is invalid if it is smaller than the number of
     * elements already inside the hashtable */
     if (size <= ht->used())
        return;

    Jim_InitHashTable(&n, ht->type(), ht->privdata());
    n.size_ = realsize;
    n.sizemask_ = realsize - 1;
    n.table_ = Jim_TAllocZ<Jim_HashEntryArray>(realsize,"Jim_HashEntryArray"); // #AllocF 
    /* Keep the same 'uniq' as the original */
    n.uniq_ = ht->uniq();
    n.setTypeName(ht->getTypeName());

    /* Initialize all the pointers to NULL */
    //memset(n.table_, 0, realsize * sizeof(Jim_HashEntryArray));

    /* Copy all the elements from the old to the new table:
     * note that if the old hash table is empty ht->used is zero,
     * so Jim_ExpandHashTable just creates an empty hash table. */
    n.used_ = ht->used();
    for (i = 0; ht->used() > 0; i++) {
        Jim_HashEntryPtr he; Jim_HashEntryPtr nextHe;

        if (ht->table_[i] == NULL)
            continue;

        /* For each hash entry on this slot... */
        he = ht->table_[i];
        while (he) {
            unsigned_int h;

            nextHe = he->next();
            /* Get the new element index */
            h = Jim_HashKey(ht, he->keyAsVoid()) & n.sizemask();
            he->next_ = n.table_[h];
            n.table_[h] = he;
            ht->used_--;
            /* Pass to the next element */
            he = nextHe;
        }
    }
    assert(ht->used() == 0);
    Jim_TFree<Jim_HashEntryArray>(ht->table_,"Jim_HashEntryArray"); // #FreeF 

    /* Remap the new hashtable in the old */
    *ht = n;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_RESIZE_POST, __FUNCTION__, ht);
}

/* Add an element to the target hash table */
JIM_EXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void *key, void *val)
{
    PRJ_TRACE;
    Jim_HashEntryPtr entry;

    /* Get the index of the new element, or -1 if
     * the element already exists. */
    entry = JimInsertHashEntry(ht, key, 0);
    if (entry == NULL)
        return JIM_ERR;

    /* Set the hash entry fields. */
    Jim_SetHashKey(ht, entry, key);
    Jim_SetHashVal(ht, entry, val);
    return JIM_OK;
}

/* Add an element, discarding the old if the key already exists */
JIM_EXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht, const void *key, void *val)
{
    PRJ_TRACE;
    int existed;
    Jim_HashEntryPtr entry;

    /* Get the index of the new element, or -1 if
     * the element already exists. */
    entry = JimInsertHashEntry(ht, key, 1);
    if (entry->keyAsVoid()) {
        /* It already exists, so only replace the value.
         * Note if both a destructor and a duplicate function exist,
         * need to dup before destroy. perhaps they are the same
         * reference counted object
         */
        if (ht->type()->valDestructor && ht->type()->valDup) {
            void *newval = ht->type()->valDup(ht->privdata(), val);
            ht->type()->valDestructor(ht->privdata(), entry->voidValue());
            entry->u.val_ = newval;
        }
        else {
            Jim_FreeEntryVal(ht, entry);
            Jim_SetHashVal(ht, entry, val);
        }
        existed = 1;
    }
    else {
        /* Doesn't exist, so set the key */
        Jim_SetHashKey(ht, entry, key);
        Jim_SetHashVal(ht, entry, val);
        existed = 0;
    }

    return existed;
}

/* Search and remove an element */
JIM_EXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht, const void *key)
{
    PRJ_TRACE;
    unsigned_int h;
    Jim_HashEntryPtr  he; Jim_HashEntryPtr prevHe;

    if (ht->used() == 0)
        return JIM_ERR;
    h = Jim_HashKey(ht, key) & ht->sizemask();
    he = ht->table_[h];

    prevHe = NULL;
    while (he) {
        if (Jim_CompareHashKeys(ht, key, he->keyAsVoid())) {
            /* Unlink the element from the list */
            if (prevHe)
                prevHe->next_ = he->next();
            else
                ht->table_[h] = he->next();
            Jim_FreeEntryKey(ht, he);
            Jim_FreeEntryVal(ht, he);
            free_Jim_HashEntry(he); // #FreeF 
            ht->used_--;
            return JIM_OK;
        }
        prevHe = he;
        he = he->next();
    }
    return JIM_ERR;             /* not found */
}

/* Destroy an entire hash table and leave it ready for reuse */
JIM_EXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht)
{
    PRJ_TRACE;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_DELETE, __FUNCTION__, ht);
    unsigned_int i;

    /* Free all the elements */
    for (i = 0; ht->used() > 0; i++) {
        Jim_HashEntryPtr  he; Jim_HashEntryPtr nextHe;

        if ((he = ht->table_[i]) == NULL)
            continue;
        while (he) {
            nextHe = he->next();
            Jim_FreeEntryKey(ht, he);
            Jim_FreeEntryVal(ht, he);
            free_Jim_HashEntry(he); // #FreeF 
            ht->used_--;
            he = nextHe;
        }
    }
    /* Free the table and the allocated cache structure */
    Jim_TFree<Jim_HashEntryArray>(ht->table_,"Jim_HashEntryArray"); // #FreeF 
    /* Re-initialize the table */
    JimResetHashTable(ht);
    return JIM_OK;              /* never fails */
}

JIM_EXPORT Jim_HashEntryPtr Jim_FindHashEntry(Jim_HashTablePtr ht, const void *key)
{
    PRJ_TRACE;
    Jim_HashEntryPtr he;
    unsigned_int h;

    if (ht->used() == 0)
        return NULL;
    h = Jim_HashKey(ht, key) & ht->sizemask();
    he = ht->table_[h];
    while (he) {
        if (Jim_CompareHashKeys(ht, key, he->keyAsVoid()))
            return he;
        he = he->next();
    }
    return NULL;
}

JIM_EXPORT Jim_HashTableIterator *Jim_GetHashTableIterator(Jim_HashTablePtr ht) // #MissInCoverage
{
    PRJ_TRACE;
    Jim_HashTableIterator* iter = new_Jim_HashTableIterator; // #AllocF 
    JimInitHashTableIterator(ht, iter);
    return iter;
}

JIM_EXPORT Jim_HashEntryPtr Jim_NextHashEntry(Jim_HashTableIterator *iter)
{
    PRJ_TRACE;
    while (1) {
        if (iter->entry() == NULL) {
            iter->index_++;
            if (iter->index() >= (signed)iter->ht->size())
                break;
            iter->entry_ = iter->ht->table_[iter->index()];
        }
        else {
            iter->entry_ = iter->nextEntry();
        }
        if (iter->entry()) {
            /* We need to save the 'next' here, the iterator user
             * may delete the entry we are returning. */
            iter->nextEntry_ = iter->entry()->next();
            return iter->entry();
        }
    }
    return NULL;
}

/* ------------------------- private functions ------------------------------ */

/* Expand the hash table if needed */
static void JimExpandHashTableIfNeeded(Jim_HashTablePtr ht)
{
    PRJ_TRACE;
    /* If the hash table is empty expand it to the initial size,
     * if the table is "full" double its size. */
    if (ht->size() == 0)
        Jim_ExpandHashTable(ht, JIM_HT_INITIAL_SIZE);
    if (ht->size() == ht->used())
        Jim_ExpandHashTable(ht, ht->size() * 2);
}

/* Our hash table capability is a power of two */
static unsigned_int JimHashTableNextPower(unsigned_int size)
{
    PRJ_TRACE;
    unsigned_int i = JIM_HT_INITIAL_SIZE;

    if (size >= 2147483648U)
        return 2147483648U;
    while (1) {
        if (i >= size)
            return i;
        i *= 2;
    }
}

/* Returns the index of a free slot that can be populated with
 * a hash entry for the given 'key'.
 * If the key already exists, -1 is returned. */
STATIC Jim_HashEntryPtr JimInsertHashEntry(Jim_HashTablePtr ht, const void *key, int replace)
{
    PRJ_TRACE;
    unsigned_int h;
    Jim_HashEntryPtr he;

    /* Expand the hashtable if needed */
    JimExpandHashTableIfNeeded(ht);

    /* Compute the key hash value */
    h = Jim_HashKey(ht, key) & ht->sizemask();
    /* Search if this slot does not already contain the given key */
    he = ht->table_[h];
    while (he) {
        if (Jim_CompareHashKeys(ht, key, he->keyAsVoid()))
            return replace ? he : NULL;
        he = he->next();
    }

    /* Allocates the memory and stores key */
    he = new_Jim_HashEntry; // #AllocF 
    he->next_ = ht->table_[h];
    ht->table_[h] = he;
    ht->used_++;
    he->key_ = NULL;

    return he;
}

/* ----------------------- StringCopy Hash Table Type ------------------------*/

static unsigned_int JimStringCopyHTHashFunction(const void *key)
{
    PRJ_TRACE;
    return Jim_GenHashFunction((const_unsigned_char*)key, (int)strlen((const char*)key));
}

static void *JimStringCopyHTDup(void *privdata, const void *key)
{
    PRJ_TRACE;
    return Jim_StrDup((const char*)key);
}

static int JimStringCopyHTKeyCompare(void *privdata, const void *key1, const void *key2)
{
    PRJ_TRACE;
    return strcmp((const char*)key1, (const char*)key2) == 0;
}

static void JimStringCopyHTKeyDestructor(void *privdata, void *key)
{
    PRJ_TRACE;
    Jim_TFree<void>(key,"void"); // #FreeF 
}

static const Jim_HashTableType g_JimPackageHashTableType = {
    JimStringCopyHTHashFunction,     /* hash function */
    JimStringCopyHTDup,              /* key dup */
    NULL,                            /* val dup */
    JimStringCopyHTKeyCompare,       /* key compare */
    JimStringCopyHTKeyDestructor,    /* key destructor */
    NULL                             /* val destructor */
};

struct AssocDataValue
{
private:
    Jim_InterpDeleteProc *delProc = NULL;
    void *data = NULL;
public:
    friend STATIC void JimAssocDataHashTableValueDestructor(void *privdata, void *data);
    friend int Jim_SetAssocData(Jim_InterpPtr interp, const char *key, Jim_InterpDeleteProc * delProc,
                                void *data);
    friend void *Jim_GetAssocData(Jim_InterpPtr interp, const char *key);
};
typedef AssocDataValue* AssocDataValuePtr;

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_AssocDataValue      Jim_TAlloc<AssocDataValue>(1,"AssocDataValue")

STATIC void JimAssocDataHashTableValueDestructor(void *privdata, void *data)
{
    PRJ_TRACE;
    AssocDataValuePtr assocPtr = (AssocDataValuePtr) data;

    if (assocPtr->delProc != NULL)
        assocPtr->delProc((Jim_InterpPtr )privdata, assocPtr->data);
    Jim_TFree<void>(data,"void"); // #FreeF 
}

static const Jim_HashTableType JimAssocDataHashTableType = {
    JimStringCopyHTHashFunction,    /* hash function */
    JimStringCopyHTDup,             /* key dup */
    NULL,                           /* val dup */
    JimStringCopyHTKeyCompare,      /* key compare */
    JimStringCopyHTKeyDestructor,   /* key destructor */
    JimAssocDataHashTableValueDestructor        /* val destructor */
};

/* -----------------------------------------------------------------------------
 * Stack - This is a simple generic stack implementation. It is used for
 * example in the 'expr' expression compiler.
 * ---------------------------------------------------------------------------*/
JIM_EXPORT Jim_StackPtr  Jim_AllocStack(void) {
    PRJ_TRACE;
    return new_Jim_Stack; // #AllocF 
}
JIM_EXPORT void Jim_InitStack(Jim_StackPtr stack)
{
    PRJ_TRACE;
    stack->len_ = 0;
    stack->maxlen_ = 0;
    stack->vector_ = NULL;
}

JIM_EXPORT void Jim_FreeStack(Jim_StackPtr stack)
{
    PRJ_TRACE;
    Jim_TFree<VoidPtrArray>(stack->vector_,"VoidPtrArray"); // #FreeF 
}

JIM_EXPORT int Jim_StackLen(Jim_StackPtr stack)
{
    PRJ_TRACE;
    return stack->len();
}

JIM_EXPORT void Jim_StackPush(Jim_StackPtr stack, void *element)
{
    PRJ_TRACE;
    int neededLen = stack->len() + 1;

    if (neededLen > stack->maxlen()) {
        stack->maxlen_ = neededLen < 20 ? 20 : neededLen * 2; // #MagicNum
        stack->vector_ = Jim_TRealloc<VoidPtrArray>(stack->vector_, stack->maxlen(),"VoidPtrArray"); // #AllocF 
    }
    stack->vector_[stack->len()] = element;
    stack->lenIncr();
}

JIM_EXPORT void *Jim_StackPop(Jim_StackPtr stack)
{
    PRJ_TRACE;
    if (stack->len() == 0)
        return NULL;
    stack->lenDecr();
    return stack->vector_[stack->len()];
}

JIM_EXPORT void *Jim_StackPeek(Jim_StackPtr stack)
{
    PRJ_TRACE;
    if (stack->len() == 0)
        return NULL;
    return stack->vector_[stack->len() - 1];
}

JIM_EXPORT void Jim_FreeStackElements(Jim_StackPtr stack, void (*freeFunc) (void *ptr))
{
    PRJ_TRACE;
    int i;

    for (i = 0; i < stack->len(); i++)
        freeFunc(stack->vector_[i]);
}

/* -----------------------------------------------------------------------------
 * Tcl Parser
 * ---------------------------------------------------------------------------*/

/* Token types */
enum TOKEN_TYPES {
     JIM_TT_NONE   = 0,          /* No token returned */
     JIM_TT_STR    = 1,          /* simple string */
     JIM_TT_ESC    = 2,          /* string that needs escape chars conversion */
     JIM_TT_VAR    = 3,          /* var substitution */
     JIM_TT_DICTSUGAR  = 4,      /* Syntax sugar for [dict get], $foo(bar) */
     JIM_TT_CMD    = 5,          /* command substitution */
/* Note: Keep these three together for TOKEN_IS_SEP() */
     JIM_TT_SEP    = 6,          /* word separator (white space) */
     JIM_TT_EOL    = 7,          /* line separator */
     JIM_TT_EOF    = 8,          /* end of script */

     JIM_TT_LINE   = 9,          /* special 'start-of-line' token. arg is # of arguments to the command. -ve if {*} */
     JIM_TT_WORD   = 10,          /* special 'start-of-word' token. arg is # of tokens to combine. -ve if {*} */

/* Additional token types needed for expressions */
     JIM_TT_SUBEXPR_START  = 11,
     JIM_TT_SUBEXPR_END    = 12,
     JIM_TT_SUBEXPR_COMMA  = 13,
     JIM_TT_EXPR_INT       = 14,
     JIM_TT_EXPR_DOUBLE    = 15,
     JIM_TT_EXPR_BOOLEAN   = 16,

     JIM_TT_EXPRSUGAR      = 17,  /* $(expression) */

/* Operator token types start here */
     JIM_TT_EXPR_OP        = 20
};

static inline int TOKEN_IS_SEP(int type) { return (type >= JIM_TT_SEP && type <= JIM_TT_EOF); }
/* Can this token start an expression? */
static inline int TOKEN_IS_EXPR_START(int type) { return (type == JIM_TT_NONE || type == JIM_TT_SUBEXPR_START || type == JIM_TT_SUBEXPR_COMMA); }
/* Is this token an expression operator? */
static inline int TOKEN_IS_EXPR_OP(int type) { return (type >= JIM_TT_EXPR_OP); }

/**
 * Results of missing quotes, braces, etc. from parsing.
 */
struct JimParseMissing { /* NOT-USED */
    int ch = 0;             /* At end of parse, ' ' if complete or '{', '[', '"', '\\', '}' if incomplete */
    int line = 0;           /* Line number starting the missing token */
private:
public:
};

/* Parser context structure. The same context is used to parse
 * Tcl scripts, expressions and lists. */
struct JimParserCtx;
typedef JimParserCtx* JimParserCtxPtr;

struct JimParserCtx
{
private:
    const char *p = NULL;       /* Pointer to the point of the program we are parsing */
    int len = 0;                /* Remaining length */
    int linenr = 0;             /* Current line number */
    const char *tstart = NULL;
    const char *tend = NULL;    /* Returned token is at tstart-tend in 'prg'. */
    int tline = 0;              /* Line number of the returned token */
    int tt = 0;                /* Token type */
    int eof = 0;               /* Non zero if EOF condition is true. */
    int inquote = 0;           /* Parsing a quoted string */
    int comment = 0;           /* Non zero if the next chars may be a comment. */
    JimParseMissing missing;   /* Details of any missing quotes, etc. */
public:

    friend STATIC void JimParserInit(JimParserCtxPtr pc, const char *prg, int len, int linenr);
    friend STATIC int JimParseScript(JimParserCtxPtr pc);
    friend STATIC int JimParseSep(JimParserCtxPtr pc);
    friend STATIC int JimParseEol(JimParserCtxPtr pc);
    friend STATIC void JimParseSubBrace(JimParserCtxPtr pc);
    friend STATIC int JimParseSubQuote(JimParserCtxPtr pc);
    friend STATIC void JimParseSubCmd(JimParserCtxPtr pc);
    friend STATIC int JimParseBrace(JimParserCtxPtr pc);
    friend STATIC int JimParseStr(JimParserCtxPtr pc);;
    friend STATIC int JimParseComment(JimParserCtxPtr pc);
    friend STATIC void JimParseSubCmd(JimParserCtxPtr pc);
    friend STATIC int JimParseSubQuote(JimParserCtxPtr pc);
    friend STATIC Jim_ObjPtr JimParserGetTokenObj(Jim_InterpPtr interp, JimParserCtxPtr pc);
    friend STATIC int JimParseCmd(JimParserCtxPtr pc);
    friend STATIC int JimParseQuote(JimParserCtxPtr pc);
    friend STATIC int JimParseVar(JimParserCtxPtr pc);
    friend STATIC int JimParseList(JimParserCtxPtr pc);
    friend STATIC int JimParseListSep(JimParserCtxPtr pc);
    friend STATIC int JimParseListQuote(JimParserCtxPtr pc);
    friend STATIC int JimParseListStr(JimParserCtxPtr pc);
    friend STATIC int JimParseExpression(JimParserCtxPtr pc);
    friend STATIC int JimParseExprNumber(JimParserCtxPtr pc);
    friend STATIC int JimParseExprIrrational(JimParserCtxPtr pc);
    friend STATIC int JimParseExprBoolean(JimParserCtxPtr pc);
    friend STATIC int JimParseExprOperator(JimParserCtxPtr pc);
    friend STATIC void JimParseSubst(JimParserCtxPtr pc, int flags);
    friend STATIC void JimSetScriptFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC int SetListFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC int SetExprFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC int SetSubstFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);
};

STATIC Retval JimParseScript(JimParserCtxPtr pc);
STATIC Retval JimParseSep(JimParserCtxPtr pc);
STATIC Retval JimParseEol(JimParserCtxPtr pc);
STATIC Retval JimParseCmd(JimParserCtxPtr pc);
STATIC Retval JimParseQuote(JimParserCtxPtr pc);
STATIC Retval JimParseVar(JimParserCtxPtr pc);
STATIC Retval JimParseBrace(JimParserCtxPtr pc);
STATIC Retval JimParseStr(JimParserCtxPtr pc);
STATIC Retval JimParseComment(JimParserCtxPtr pc);
STATIC void JimParseSubCmd(JimParserCtxPtr pc);
STATIC int JimParseSubQuote(JimParserCtxPtr pc);
STATIC Jim_ObjPtr JimParserGetTokenObj(Jim_InterpPtr interp, JimParserCtxPtr pc);

/* Initialize a parser context.
 * 'prg' is a pointer to the program text, linenr is the line
 * number of the first line contained in the program. */
STATIC void JimParserInit(JimParserCtxPtr pc, const char *prg, int len, int linenr)
{
    PRJ_TRACE;
    pc->p = prg;
    pc->len = len;
    pc->tstart = NULL;
    pc->tend = NULL;
    pc->tline = 0;
    pc->tt = JIM_TT_NONE;
    pc->eof = 0;
    pc->inquote = 0;
    pc->linenr = linenr;
    pc->comment = 1;
    pc->missing.ch = ' ';
    pc->missing.line = linenr;
}

STATIC Retval JimParseScript(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    while (1) {                 /* the while is used to reiterate with continue if needed */
        if (!pc->len) {
            pc->tstart = pc->p;
            pc->tend = pc->p - 1;
            pc->tline = pc->linenr;
            pc->tt = JIM_TT_EOL;
            pc->eof = 1;
            return JIM_OK;
        }
        switch (*(pc->p)) {
            case '\\':
                if (*(pc->p + 1) == '\n' && !pc->inquote) {
                    return JimParseSep(pc);
                }
                pc->comment = 0;
                return JimParseStr(pc);
            case ' ':
            case '\t':
            case '\r':
            case '\f':
                if (!pc->inquote)
                    return JimParseSep(pc);
                pc->comment = 0;
                return JimParseStr(pc);
            case '\n':
            case ';':
                pc->comment = 1;
                if (!pc->inquote)
                    return JimParseEol(pc);
                return JimParseStr(pc);
            case '[':
                pc->comment = 0;
                return JimParseCmd(pc);
            case '$':
                pc->comment = 0;
                if (JimParseVar(pc) == JIM_ERR) {
                    /* An orphan $. Create as a separate token */
                    pc->tstart = pc->tend = pc->p++;
                    pc->len--;
                    pc->tt = JIM_TT_ESC;
                }
                return JIM_OK;
            case '#':
                if (pc->comment) {
                    JimParseComment(pc);
                    continue;
                }
                return JimParseStr(pc);
            default:
                pc->comment = 0;
                return JimParseStr(pc);
        }
        return JIM_OK;
    }
}

STATIC Retval JimParseSep(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p;
    pc->tline = pc->linenr;
    while (isspace(UCHAR(*pc->p)) || (*pc->p == '\\' && *(pc->p + 1) == '\n')) {
        if (*pc->p == '\n') {
            break;
        }
        if (*pc->p == '\\') {
            pc->p++;
            pc->len--;
            pc->linenr++;
        }
        pc->p++;
        pc->len--;
    }
    pc->tend = pc->p - 1;
    pc->tt = JIM_TT_SEP;
    return JIM_OK;
}

STATIC Retval JimParseEol(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p;
    pc->tline = pc->linenr;
    while (isspace(UCHAR(*pc->p)) || *pc->p == ';') {
        if (*pc->p == '\n')
            pc->linenr++;
        pc->p++;
        pc->len--;
    }
    pc->tend = pc->p - 1;
    pc->tt = JIM_TT_EOL;
    return JIM_OK;
}

/*
** Here are the rules for parsing:
** {braced expression}
** - Count open and closing braces
** - Backslash escapes meaning of braces but doesn't remove the backslash
**
** "quoted expression"
** - Unescaped double quote terminates the expression
** - Backslash escapes next char
** - [commands brackets] are counted/nested
** - command rules apply within [brackets], not quoting rules (i.e. brackets have their own rules)
**
** [command expression]
** - Count open and closing brackets
** - Backslash escapes next char
** - [commands brackets] are counted/nested
** - "quoted expressions" are parsed according to quoting rules
** - {braced expressions} are parsed according to brace rules
**
** For everything, backslash escapes the next char, newline increments current line
*/

/**
 * Parses a braced expression starting at pc->p.
 *
 * Positions the parser at the end of the braced expression,
 * sets pc->tend and possibly pc->missing.
 */
STATIC void JimParseSubBrace(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    int level = 1;

    /* Skip the brace */
    pc->p++;
    pc->len--;
    while (pc->len) {
        switch (*pc->p) {
            case '\\':
                if (pc->len > 1) {
                    if (*++pc->p == '\n') {
                        pc->linenr++;
                    }
                    pc->len--;
                }
                break;

            case '{':
                level++;
                break;

            case '}':
                if (--level == 0) {
                    pc->tend = pc->p - 1;
                    pc->p++;
                    pc->len--;
                    return;
                }
                break;

            case '\n':
                pc->linenr++;
                break;
        }
        pc->p++;
        pc->len--;
    }
    pc->missing.ch = '{';
    pc->missing.line = pc->tline;
    pc->tend = pc->p - 1;
}

/**
 * Parses a quoted expression starting at pc->p.
 *
 * Positions the parser at the end of the quoted expression,
 * sets pc->tend and possibly pc->missing.
 *
 * Returns the type of the token of the string,
 * either JIM_TT_ESC (if it contains values which need to be [subst]ed)
 * or JIM_TT_STR.
 */
STATIC int JimParseSubQuote(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    int tt = JIM_TT_STR;
    int line = pc->tline;

    /* Skip the quote */
    pc->p++;
    pc->len--;
    while (pc->len) {
        switch (*pc->p) {
            case '\\':
                if (pc->len > 1) {
                    if (*++pc->p == '\n') {
                        pc->linenr++;
                    }
                    pc->len--;
                    tt = JIM_TT_ESC;
                }
                break;

            case '"':
                pc->tend = pc->p - 1;
                pc->p++;
                pc->len--;
                return tt;

            case '[':
                JimParseSubCmd(pc);
                tt = JIM_TT_ESC;
                continue;

            case '\n':
                pc->linenr++;
                break;

            case '$':
                tt = JIM_TT_ESC;
                break;
        }
        pc->p++;
        pc->len--;
    }
    pc->missing.ch = '"';
    pc->missing.line = line;
    pc->tend = pc->p - 1;
    return tt;
}

/**
 * Parses a [command] expression starting at pc->p.
 *
 * Positions the parser at the end of the command expression,
 * sets pc->tend and possibly pc->missing.
 */
STATIC void JimParseSubCmd(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    int level = 1;
    int startofword = 1;
    int line = pc->tline;

    /* Skip the bracket */
    pc->p++;
    pc->len--;
    while (pc->len) {
        switch (*pc->p) {
            case '\\':
                if (pc->len > 1) {
                    if (*++pc->p == '\n') {
                        pc->linenr++;
                    }
                    pc->len--;
                }
                break;

            case '[':
                level++;
                break;

            case ']':
                if (--level == 0) {
                    pc->tend = pc->p - 1;
                    pc->p++;
                    pc->len--;
                    return;
                }
                break;

            case '"':
                if (startofword) {
                    JimParseSubQuote(pc);
                    continue;
                }
                break;

            case '{':
                JimParseSubBrace(pc);
                startofword = 0;
                continue;

            case '\n':
                pc->linenr++;
                break;
        }
        startofword = isspace(UCHAR(*pc->p));
        pc->p++;
        pc->len--;
    }
    pc->missing.ch = '[';
    pc->missing.line = line;
    pc->tend = pc->p - 1;
}

STATIC Retval JimParseBrace(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p + 1;
    pc->tline = pc->linenr;
    pc->tt = JIM_TT_STR;
    JimParseSubBrace(pc);
    return JIM_OK;
}

STATIC Retval JimParseCmd(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p + 1;
    pc->tline = pc->linenr;
    pc->tt = JIM_TT_CMD;
    JimParseSubCmd(pc);
    return JIM_OK;
}

STATIC Retval JimParseQuote(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p + 1;
    pc->tline = pc->linenr;
    pc->tt = JimParseSubQuote(pc);
    return JIM_OK;
}

STATIC Retval JimParseVar(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    /* skip the $ */
    pc->p++;
    pc->len--;

    if (g_EXPRSUGAR_BRACKET) {
        if (*pc->p == '[') { // #MissInCoverage
            /* Parse $[...] expr shorthand syntax */
            JimParseCmd(pc);
            pc->tt = JIM_TT_EXPRSUGAR;
            return JIM_OK;
        }
    }

    pc->tstart = pc->p;
    pc->tt = JIM_TT_VAR;
    pc->tline = pc->linenr;

    if (*pc->p == '{') {
        pc->tstart = ++pc->p;
        pc->len--;

        while (pc->len && *pc->p != '}') {
            if (*pc->p == '\n') {
                pc->linenr++;
            }
            pc->p++;
            pc->len--;
        }
        pc->tend = pc->p - 1;
        if (pc->len) {
            pc->p++;
            pc->len--;
        }
    }
    else {
        while (1) {
            /* Skip double colon, but not single colon! */
            if (pc->p[0] == ':' && pc->p[1] == ':') {
                while (*pc->p == ':') {
                    pc->p++;
                    pc->len--;
                }
                continue;
            }
            /* Note that any char >= 0x80 must be part of a utf-8 char.
             * We consider all unicode points outside of ASCII as letters
             */
            if (isalnum(UCHAR(*pc->p)) || *pc->p == '_' || UCHAR(*pc->p) >= 0x80) {
                pc->p++;
                pc->len--;
                continue;
            }
            break;
        }
        /* Parse [dict get] syntax sugar. */
        if (*pc->p == '(') {
            int count = 1;
            const char *paren = NULL;

            pc->tt = JIM_TT_DICTSUGAR;

            while (count && pc->len) {
                pc->p++;
                pc->len--;
                if (*pc->p == '\\' && pc->len >= 1) {
                    pc->p++;
                    pc->len--;
                }
                else if (*pc->p == '(') {
                    count++;
                }
                else if (*pc->p == ')') {
                    paren = pc->p;
                    count--;
                }
            }
            if (count == 0) {
                pc->p++;
                pc->len--;
            }
            else if (paren) {
                /* Did not find a matching paren. Back up */
                paren++;
                pc->len += (int)(pc->p - paren);
                pc->p = paren;
            }
            if (!g_EXPRSUGAR_BRACKET) {
                if (*pc->tstart == '(') {
                    pc->tt = JIM_TT_EXPRSUGAR;
                }
            }
        }
        pc->tend = pc->p - 1;
    }
    /* Check if we parsed just the '$' character.
     * That's not a variable so an error is returned
     * to tell the state machine to consider this '$' just
     * a string. */
    if (pc->tstart == pc->p) {
        pc->p--;
        pc->len++;
        return JIM_ERR;
    }
    return JIM_OK;
}

STATIC Retval JimParseStr(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    if (pc->tt == JIM_TT_SEP || pc->tt == JIM_TT_EOL ||
        pc->tt == JIM_TT_NONE || pc->tt == JIM_TT_STR) {
        /* Starting a new word */
        if (*pc->p == '{') {
            return JimParseBrace(pc);
        }
        if (*pc->p == '"') {
            pc->inquote = 1;
            pc->p++;
            pc->len--;
            /* In case the end quote is missing */
            pc->missing.line = pc->tline;
        }
    }
    pc->tstart = pc->p;
    pc->tline = pc->linenr;
    while (1) {
        if (pc->len == 0) {
            if (pc->inquote) {
                pc->missing.ch = '"';
            }
            pc->tend = pc->p - 1;
            pc->tt = JIM_TT_ESC;
            return JIM_OK;
        }
        switch (*pc->p) {
            case '\\':
                if (!pc->inquote && *(pc->p + 1) == '\n') {
                    pc->tend = pc->p - 1;
                    pc->tt = JIM_TT_ESC;
                    return JIM_OK;
                }
                if (pc->len >= 2) {
                    if (*(pc->p + 1) == '\n') {
                        pc->linenr++;
                    }
                    pc->p++;
                    pc->len--;
                }
                else if (pc->len == 1) {
                    /* End of script with trailing backslash */
                    pc->missing.ch = '\\';
                }
                break;
            case '(':
                /* If the following token is not '$' just keep going */
                if (pc->len > 1 && pc->p[1] != '$') {
                    break;
                }
                /* fall through */
            case ')':
                /* Only need a separate ')' token if the previous was a var */
                if (*pc->p == '(' || pc->tt == JIM_TT_VAR) {
                    if (pc->p == pc->tstart) {
                        /* At the start of the token, so just return this char */
                        pc->p++;
                        pc->len--;
                    }
                    pc->tend = pc->p - 1;
                    pc->tt = JIM_TT_ESC;
                    return JIM_OK;
                }
                break;

            case '$':
            case '[':
                pc->tend = pc->p - 1;
                pc->tt = JIM_TT_ESC;
                return JIM_OK;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
            case '\f':
            case ';':
                if (!pc->inquote) {
                    pc->tend = pc->p - 1;
                    pc->tt = JIM_TT_ESC;
                    return JIM_OK;
                }
                else if (*pc->p == '\n') {
                    pc->linenr++;
                }
                break;
            case '"':
                if (pc->inquote) {
                    pc->tend = pc->p - 1;
                    pc->tt = JIM_TT_ESC;
                    pc->p++;
                    pc->len--;
                    pc->inquote = 0;
                    return JIM_OK;
                }
                break;
        }
        pc->p++;
        pc->len--;
    }
    return JIM_OK;              /* unreached */
}

STATIC Retval JimParseComment(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    while (*pc->p) {
        if (*pc->p == '\\') {
            pc->p++;
            pc->len--;
            if (pc->len == 0) {
                pc->missing.ch = '\\'; // #MissInCoverage
                return JIM_OK;
            }
            if (*pc->p == '\n') {
                pc->linenr++;
            }
        }
        else if (*pc->p == '\n') {
            pc->p++;
            pc->len--;
            pc->linenr++;
            break;
        }
        pc->p++;
        pc->len--;
    }
    return JIM_OK;
}

/* xdigitval and odigitval are helper functions for JimEscape() */
static int xdigitval(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static int odigitval(int c)
{
    if (c >= '0' && c <= '7')
        return c - '0';
    return -1;
}

/* Perform Tcl escape substitution of 's', storing the result
 * string into 'dest'. The escaped string is guaranteed to
 * be the same length or shorter than the source string.
 * slen is the length of the string at 's'.
 *
 * The function returns the length of the resulting string. */
static int JimEscape(char *dest, const char *s, int slen)
{
    PRJ_TRACE;
    char *p = dest;
    int i, len;

    for (i = 0; i < slen; i++) {
        switch (s[i]) {
            case '\\':
                switch (s[i + 1]) {
                    case 'a':
                        *p++ = 0x7;
                        i++;
                        break;
                    case 'b':
                        *p++ = 0x8;
                        i++;
                        break;
                    case 'f':
                        *p++ = 0xc;
                        i++;
                        break;
                    case 'n':
                        *p++ = 0xa;
                        i++;
                        break;
                    case 'r':
                        *p++ = 0xd;
                        i++;
                        break;
                    case 't':
                        *p++ = 0x9;
                        i++;
                        break;
                    case 'u':
                    case 'U':
                    case 'x':
                        /* A unicode or hex sequence.
                         * \x Expect 1-2 hex chars and convert to hex.
                         * \u Expect 1-4 hex chars and convert to utf-8.
                         * \U Expect 1-8 hex chars and convert to utf-8.
                         * \u{NNN} supports 1-6 hex chars and convert to utf-8.
                         * An invalid sequence means simply the escaped char.
                         */
                        {
                            unsigned_t val = 0;
                            int k;
                            int maxchars = 2;

                            i++;

                            if (s[i] == 'U') {
                                maxchars = 8; // #MissInCoverage
                            }
                            else if (s[i] == 'u') {
                                if (s[i + 1] == '{') {
                                    maxchars = 6;
                                    i++;
                                }
                                else {
                                    maxchars = 4;
                                }
                            }

                            for (k = 0; k < maxchars; k++) {
                                int c = xdigitval(s[i + k + 1]);
                                if (c == -1) {
                                    break;
                                }
                                val = (val << 4) | c;
                            }
                            /* The \u{nnn} syntax supports up to 21 bit codepoints. */
                            if (s[i] == '{') {
                                if (k == 0 || val > 0x1fffff || s[i + k + 1] != '}') {
                                    /* Back up */
                                    i--; // #MissInCoverage
                                    k = 0;
                                }
                                else {
                                    /* Skip the closing brace */
                                    k++;
                                }
                            }
                            if (k) {
                                /* Got a valid sequence, so convert */
                                if (s[i] == 'x') {
                                    *p++ = val;
                                }
                                else {
                                    p += utf8_fromunicode(p, val);
                                }
                                i += k;
                                break;
                            }
                            /* Not a valid codepoint, just an escaped char */
                            *p++ = s[i];
                        }
                        break;
                    case 'v':
                        *p++ = 0xb;
                        i++;
                        break;
                    case '\0':
                        *p++ = '\\';
                        i++;
                        break;
                    case '\n':
                        /* Replace all spaces and tabs after backslash newline with a single space*/
                        *p++ = ' ';
                        do {
                            i++;
                        } while (s[i + 1] == ' ' || s[i + 1] == '\t');
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        /* octal escape */
                        {
                            int val = 0;
                            int c = odigitval(s[i + 1]);

                            val = c;
                            c = odigitval(s[i + 2]);
                            if (c == -1) {
                                *p++ = val;
                                i++;
                                break;
                            }
                            val = (val * 8) + c;
                            c = odigitval(s[i + 3]);
                            if (c == -1) {
                                *p++ = val;
                                i += 2;
                                break;
                            }
                            val = (val * 8) + c;
                            *p++ = val;
                            i += 3;
                        }
                        break;
                    default:
                        *p++ = s[i + 1];
                        i++;
                        break;
                }
                break;
            default:
                *p++ = s[i];
                break;
        }
    }
    len = (int)(p - dest);
    *p = '\0';
    return len;
}

/* Returns a dynamically allocated copy of the current token in the
 * parser context. The function performs conversion of escapes if
 * the token is of type JIM_TT_ESC.
 *
 * Note that after the conversion, tokens that are grouped with
 * braces in the source code, are always recognizable from the
 * identical string obtained in a different way from the type.
 *
 * For example the string:
 *
 * {*}$a
 *
 * will return as first token "*", of type JIM_TT_STR
 *
 * While the string:
 *
 * *$a
 *
 * will return as first token "*", of type JIM_TT_ESC
 */
STATIC Jim_ObjPtr JimParserGetTokenObj(Jim_InterpPtr interp, JimParserCtxPtr pc)
{
    PRJ_TRACE;
    const char *start, *end;
    char *token;
    int len;

    start = pc->tstart;
    end = pc->tend;
    len = (int)((end - start) + 1);
    if (len < 0) {
        len = 0;
    }
    token = new_CharArray(len + 1); // #AllocF 
    if (pc->tt != JIM_TT_ESC) {
        /* No escape conversion needed? Just copy it. */
        memcpy(token, start, len);
        token[len] = '\0';
    }
    else {
        /* Else convert the escape chars. */
        len = JimEscape(token, start, len);
    }

    return Jim_NewStringObjNoAlloc(interp, token, len);
}

/* -----------------------------------------------------------------------------
 * Tcl Lists parsing
 * ---------------------------------------------------------------------------*/
STATIC Retval JimParseListSep(JimParserCtxPtr pc);
STATIC Retval JimParseListStr(JimParserCtxPtr pc);
STATIC Retval JimParseListQuote(JimParserCtxPtr pc);

STATIC Retval JimParseList(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    if (isspace(UCHAR(*pc->p))) {
        return JimParseListSep(pc);
    }
    switch (*pc->p) {
        case '"':
            return JimParseListQuote(pc);

        case '{':
            return JimParseBrace(pc);

        default:
            if (pc->len) {
                return JimParseListStr(pc);
            }
            break;
    }

    pc->tstart = pc->tend = pc->p;
    pc->tline = pc->linenr;
    pc->tt = JIM_TT_EOL;
    pc->eof = 1;
    return JIM_OK;
}

STATIC Retval JimParseListSep(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p;
    pc->tline = pc->linenr;
    while (isspace(UCHAR(*pc->p))) {
        if (*pc->p == '\n') {
            pc->linenr++;
        }
        pc->p++;
        pc->len--;
    }
    pc->tend = pc->p - 1;
    pc->tt = JIM_TT_SEP;
    return JIM_OK;
}

STATIC Retval JimParseListQuote(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->p++;
    pc->len--;

    pc->tstart = pc->p;
    pc->tline = pc->linenr;
    pc->tt = JIM_TT_STR;

    while (pc->len) {
        switch (*pc->p) {
            case '\\':
                pc->tt = JIM_TT_ESC;
                if (--pc->len == 0) {
                    /* Trailing backslash */
                    pc->tend = pc->p;
                    return JIM_OK;
                }
                pc->p++;
                break;
            case '\n':
                pc->linenr++;
                break;
            case '"':
                pc->tend = pc->p - 1;
                pc->p++;
                pc->len--;
                return JIM_OK;
        }
        pc->p++;
        pc->len--;
    }

    pc->tend = pc->p - 1;
    return JIM_OK;
}

STATIC Retval JimParseListStr(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    pc->tstart = pc->p;
    pc->tline = pc->linenr;
    pc->tt = JIM_TT_STR;

    while (pc->len) {
        if (isspace(UCHAR(*pc->p))) {
            pc->tend = pc->p - 1;
            return JIM_OK;
        }
        if (*pc->p == '\\') {
            if (--pc->len == 0) {
                /* Trailing backslash */
                pc->tend = pc->p;
                return JIM_OK;
            }
            pc->tt = JIM_TT_ESC;
            pc->p++;
        }
        pc->p++;
        pc->len--;
    }
    pc->tend = pc->p - 1;
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * Jim_Obj related functions
 * ---------------------------------------------------------------------------*/

/* Return a new initialized object. */
JIM_EXPORT Jim_ObjPtr Jim_NewObj(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    /* -- Check if there are objects in the free list -- */
    if (interp->freeList() != NULL) {
        /* -- Unlink the object from the free list -- */
        objPtr = interp->freeList();
        interp->setFreeList(objPtr->nextObjPtr());
    }
    else {
        /* -- No ready to use objects: allocate a new one -- */
        objPtr = new_Jim_Obj; // #AllocF 
    }

    /* Object is returned with refCount of 0. Every
     * kind of GC implemented should take care to avoid
     * scanning objects with refCount == 0. */
    objPtr->setRefCount0();
    /* All the other fields are left uninitialized to save time.
     * The caller will probably want to set them to the right
     * value anyway. */

    /* -- Put the object into the live list -- */
    objPtr->setPrevObjPtr(NULL); 
    objPtr->setNextObjPtr(interp->liveList()); 
    if (interp->liveList())
        interp->liveList()->setPrevObjPtr(objPtr); 
    interp->setLiveList(objPtr);

    return objPtr;
}

/* Free an object. Actually objects are never freed, but
 * just moved to the free objects list, where they will be
 * reused by Jim_NewObj(). */
JIM_EXPORT void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    /* Check if the object was already freed, panic. */
    JimPanic((objPtr->refCount != 0, "!!!Object %p freed with bad refcount %d, type=%s", objPtr,
        objPtr->refCount, objPtr->typePtr ? objPtr->typePtr->name : "<none>"));

    /* Free the internal representation */
    Jim_FreeIntRep(interp, objPtr);
    /* Free the string representation */
    if (objPtr->bytes() != NULL) {
        if (objPtr->bytes() != g_JimEmptyStringRep)
            objPtr->freeBytes(); // #FreeF
            // free_CharArray(objPtr->bytes_); 
    }
    /* Unlink the object from the live objects list */
    if (objPtr->prevObjPtr())
        objPtr->prevObjPtr()->setNextObjPtr(objPtr->nextObjPtr());
    if (objPtr->nextObjPtr())
        objPtr->nextObjPtr()->setPrevObjPtr(objPtr->prevObjPtr()); 
    if (interp->liveList() == objPtr)
        interp->setLiveList(objPtr->nextObjPtr());
    if (g_JIM_DISABLE_OBJECT_POOL) {
        free_Jim_Obj(objPtr); // #FreeF #MissInCoverage
    } else {
        /* Link the object into the free objects list */
        objPtr->setPrevObjPtr(NULL);
        objPtr->setNextObjPtr(interp->freeList()); 
        if (interp->freeList())
            interp->freeList()->setPrevObjPtr(objPtr); 
        interp->setFreeList(objPtr);
        objPtr->decrRefCount();
    }
}

/* Invalidate the string representation of an object. */
JIM_EXPORT void Jim_InvalidateStringRep(Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (objPtr->bytes() != NULL) {
        if (objPtr->bytes() != g_JimEmptyStringRep) {
            objPtr->freeBytes(); // #FreeF 
            // free_CharArray(objPtr->bytes_); 
        }
    }
    objPtr->bytes_setNULL();
}

/* Duplicate an object. The returned object has refcount = 0. */
JIM_EXPORT Jim_ObjPtr Jim_DuplicateObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_ObjPtr dupPtr;

    dupPtr = Jim_NewObj(interp);
    if (objPtr->bytes() == NULL) {
        /* Object does not have a valid string representation. */
        dupPtr->bytes_setNULL();
    }
    else if (objPtr->length() == 0) {
        /* Zero length, so don't even bother with the type-specific dup,
         * since all zero length objects look the same
         */
        dupPtr->setBytes(g_JimEmptyStringRep);
        dupPtr->setLength(0);
        dupPtr->setTypePtr(NULL);
        return dupPtr;
    }
    else {
        dupPtr->setBytes( new_CharArray(objPtr->length() + 1)); // #AllocF 
        dupPtr->setLength(objPtr->length());
        /* Copy the null byte too */
        dupPtr->copyBytes(objPtr);
        //memcpy(dupPtr->bytes_, objPtr->bytes_, objPtr->length() + 1); 
    }

    /* By default, the new object has the same type as the old object */
    dupPtr->setTypePtr(objPtr->typePtr());
    if (objPtr->typePtr() != NULL) {
        if (objPtr->typePtr()->dupIntRepProc == NULL) {
            dupPtr->copyInterpRep(objPtr); // #MissInCoverage
            // dupPtr->internalRep = objPtr->internalRep; 
        }
        else {
            /* The dup proc may set a different type, e.g. NULL */
            objPtr->typePtr()->dupIntRepProc(interp, objPtr, dupPtr);
        }
    }
    return dupPtr;
}

/* Return the string representation for objPtr. If the object's
 * string representation is invalid, calls the updateStringProc method to create
 * a new one from the internal representation of the object.
 */
JIM_EXPORT const char *Jim_GetString(Jim_ObjPtr objPtr, int *lenPtr)
{
    PRJ_TRACE;
    if (objPtr->bytes() == NULL) {
        /* Invalid string repr. Generate it. */
        JimPanic((objPtr->typePtr()->updateStringProc == NULL, "UpdateStringProc called against '%s' type.", objPtr->typePtr->name));
        objPtr->typePtr()->updateStringProc(objPtr);
    }
    if (lenPtr)
        *lenPtr = objPtr->length();
    return objPtr->bytes();
}

/* Just returns the length (in bytes) of the object's string rep */
JIM_EXPORT int Jim_Length(Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (objPtr->bytes() == NULL) {
        /* Invalid string repr. Generate it. */
        Jim_GetString(objPtr, NULL);
    }
    return objPtr->length();
}

/* Just returns object's string rep */
JIM_EXPORT const char *Jim_String(Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (objPtr->bytes() == NULL) {
        /* Invalid string repr. Generate it. */
        Jim_GetString(objPtr, NULL);
    }
    return objPtr->bytes();
}

STATIC void JimSetStringBytes(Jim_ObjPtr objPtr, const char *str)
{
    PRJ_TRACE;
    objPtr->setBytes(Jim_StrDup(str));
    objPtr->setLength((int)strlen(str));
}

STATIC void FreeDictSubstInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
STATIC void DupDictSubstInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);

static const Jim_ObjType g_dictSubstObjType = { // #JimType
    "dict-substitution",
    FreeDictSubstInternalRepCB,
    DupDictSubstInternalRepCB,
    NULL,
    JIM_TYPE_NONE,
};
const Jim_ObjType& dictSubstType() { return g_dictSubstObjType;  }

STATIC void FreeInterpolatedInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
STATIC void DupInterpolatedInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);

static const Jim_ObjType g_interpolatedObjType = { // #JimType
    "interpolated",
    FreeInterpolatedInternalRepCB,
    DupInterpolatedInternalRepCB,
    NULL,
    JIM_TYPE_NONE,
};
const Jim_ObjType& interpolatedType() { return g_interpolatedObjType; }

STATIC void FreeInterpolatedInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_DecrRefCount(interp, objPtr->get_dictSubstValue_index());
}

STATIC void DupInterpolatedInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #MissInCoverage
{
    PRJ_TRACE;
    /* Copy the internal rep */
    dupPtr->copyInterpRep(srcPtr); 
    /* Need to increment the key ref count */
    Jim_IncrRefCount(dupPtr->get_dictSubstValue_index());
}

/* -----------------------------------------------------------------------------
 * String Object
 * ---------------------------------------------------------------------------*/
STATIC void DupStringInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
STATIC Retval SetStringFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

static const Jim_ObjType g_stringObjType = { // #JimType #JimStr
    "string",
    NULL,
    DupStringInternalRepCB,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& stringType() { return g_stringObjType; }


STATIC void DupStringInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #JimStr
{
    PRJ_TRACE;
    JIM_NOTUSED(interp);

    /* This is a bit subtle: the only caller of this function
     * should be Jim_DuplicateObj(), that will copy the
     * string representation. After the copy, the duplicated
     * object will not have more room in the buffer than
     * srcPtr->length bytes. So we just set it to length. */
    dupPtr->setStrValue(srcPtr->length(), srcPtr->get_strValue_charLen());
    //dupPtr->internalRep.strValue_.maxLength = srcPtr->length();
    //dupPtr->internalRep.strValue_.charLength = srcPtr->internalRep.strValue_.charLength;
}

STATIC Retval SetStringFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimStr
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_stringObjType) {
        /* Get a fresh string representation. */
        if (objPtr->bytes() == NULL) {
            /* Invalid string repr. Generate it. */
            JimPanic((objPtr->typePtr->updateStringProc == NULL, "UpdateStringProc called against '%s' type.", objPtr->typePtr->name));
            objPtr->typePtr()->updateStringProc(objPtr);
        }
        /* Free any other internal representation. */
        Jim_FreeIntRep(interp, objPtr);
        /* Set it as string, i.e. just set the maxLength field. */
        objPtr->setTypePtr(&g_stringObjType);
        objPtr->setStrValue(
            objPtr->length(),
            -1 /* Don't know the utf-8 length yet */);
        //objPtr->internalRep.strValue_.maxLength = objPtr->length();
        ///* Don't know the utf-8 length yet */
        //objPtr->internalRep.strValue_.charLength = -1;
    }
    return JIM_OK;
}

/**
 * Returns the length of the object string in chars, not bytes.
 *
 * These may be different for a utf-8 string.
 */
JIM_EXPORT int Jim_Utf8Length(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #BiModeFunc #JimStr
{
    PRJ_TRACE;
    if (g_JIM_UTF8_VAL) {
        SetStringFromAny(interp, objPtr);

        if (objPtr->get_strValue_charLen() < 0) {
            objPtr->setStrValue_charLen( utf8_strlen(objPtr->bytes(), objPtr->length()));
        }
        return objPtr->get_strValue_charLen();
    }
    return Jim_Length(objPtr); // #MissInCoverage
}

/* len is in bytes -- see also Jim_NewStringObjUtf8() */
JIM_EXPORT Jim_ObjPtr Jim_NewStringObj(Jim_InterpPtr interp, const char *s, int len) // #JimStr
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr = Jim_NewObj(interp);

    /* Need to find out how many bytes the string requires */
    if (len == -1)
        len = (int)strlen(s);
    /* Alloc/Set the string rep. */
    if (len == 0) {
        objPtr->setBytes( g_JimEmptyStringRep);
    }
    else {
        objPtr->setBytes( Jim_StrDupLen(s, len));
    }
    objPtr->setLength(len);

    /* No typePtr field for the vanilla string object. */
    objPtr->setTypePtr( NULL);
    return objPtr;
}

/* charlen is in characters -- see also Jim_NewStringObj() */
JIM_EXPORT Jim_ObjPtr Jim_NewStringObjUtf8(Jim_InterpPtr interp, const char *s, int charlen) // #BiModeFunc #JimStr
{
    PRJ_TRACE;
    if (g_JIM_UTF8_VAL) {
        /* Need to find out how many bytes the string requires */
        int bytelen = utf8_index(s, charlen);

        Jim_ObjPtr  objPtr = Jim_NewStringObj(interp, s, bytelen);

        /* Remember the utf8 length, so set the type */
        objPtr->setTypePtr(&g_stringObjType);
        objPtr->setStrValue_maxLen( bytelen);
        objPtr->setStrValue_charLen( charlen);

        return objPtr;
    }

    return Jim_NewStringObj(interp, s, charlen); // #MissInCoverage

}

/* This version does not try to duplicate the 's' pointer, but
 * use it directly. */
JIM_EXPORT Jim_ObjPtr Jim_NewStringObjNoAlloc(Jim_InterpPtr interp, char *s, int len) // #JimStr
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr = Jim_NewObj(interp);

    objPtr->setBytes( s);
    objPtr->setLength( (int)((len == -1) ? strlen(s) : len));
    objPtr->setTypePtr(NULL);
    return objPtr;
}

/* Low-level string append. Use it only against unshared objects
 * of type "string". */
STATIC void StringAppendString(Jim_ObjPtr objPtr, const char *str, int len) // #JimStr
{
    PRJ_TRACE;
    int needlen;

    if (len == -1)
        len = (int)strlen(str);
    needlen = objPtr->length() + len;
    if (objPtr->get_strValue_maxLen() < needlen ||
        objPtr->get_strValue_maxLen() == 0) {
        needlen *= 2;
        /* Inefficient to malloc() for less than 8 bytes */
        if (needlen < 7) {
            needlen = 7; // #MagicNum
        }
        if (objPtr->bytes() == g_JimEmptyStringRep) {
            objPtr->setBytes( new_CharArray(needlen + 1)); // #AllocF 
        }
        else {
            objPtr->setBytes((char*) realloc_CharArray(objPtr->bytes(), needlen + 1)); // #AllocF 
        }
        objPtr->setStrValue_maxLen( needlen);
    }
    objPtr->copyBytesAt(objPtr->length(), str, len);
    //memcpy(objPtr->bytes_ + objPtr->length(), str, len); 
    objPtr->setBytes(objPtr->length() + len,'\0');

    if (objPtr->get_strValue_charLen() >= 0) {
        /* Update the utf-8 char length */
        objPtr->incrStrValue_charLen( utf8_strlen(objPtr->bytes() + objPtr->length(), len));
    }
    objPtr->lengthIncr(len);
}

/* Higher level API to append strings to objects.
 * Object must not be unshared for each of these.
 */
JIM_EXPORT void Jim_AppendString(Jim_InterpPtr interp, Jim_ObjPtr objPtr, const char *str, int len) // #JimStr
{
    PRJ_TRACE;
    JimPanic((Jim_IsShared(objPtr), "Jim_AppendString called with shared object"));
    SetStringFromAny(interp, objPtr);
    StringAppendString(objPtr, str, len);
}

JIM_EXPORT void Jim_AppendObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr appendObjPtr) // #JimStr
{
    PRJ_TRACE;
    int len;
    const char *str = Jim_GetString(appendObjPtr, &len);
    Jim_AppendString(interp, objPtr, str, len);
}

JIM_EXPORT void Jim_AppendStrings(Jim_InterpPtr interp, Jim_ObjPtr objPtr, ...) // #JimStr
{
    PRJ_TRACE;
    va_list ap;

    SetStringFromAny(interp, objPtr);
    va_start(ap, objPtr);
    while (1) {
        const char *s = va_arg(ap, const char *);

        if (s == NULL)
            break;
        Jim_AppendString(interp, objPtr, s, -1);
    }
    va_end(ap);
}

JIM_EXPORT int Jim_StringEqObj(Jim_ObjPtr aObjPtr, Jim_ObjPtr bObjPtr) // #JimStr
{
    PRJ_TRACE;
    if (aObjPtr == bObjPtr) {
        return 1;
    }
    else {
        int Alen, Blen;
        const char *sA = Jim_GetString(aObjPtr, &Alen);
        const char *sB = Jim_GetString(bObjPtr, &Blen);

        return Alen == Blen && memcmp(sA, sB, Alen) == 0;
    }
}

/**
 * Note. Does not support embedded nulls in either the pattern or the object.
 */
JIM_EXPORT int Jim_StringMatchObj(Jim_InterpPtr interp, Jim_ObjPtr patternObjPtr, Jim_ObjPtr objPtr, int nocase) // #JimStr
{
    PRJ_TRACE;
    return JimGlobMatch(Jim_String(patternObjPtr), Jim_String(objPtr), nocase);
}

/*
 * Note: does not support embedded nulls for the nocase option.
 */
JIM_EXPORT int Jim_StringCompareObj(Jim_InterpPtr interp, Jim_ObjPtr firstObjPtr, Jim_ObjPtr secondObjPtr, int nocase) // #JimStr
{
    PRJ_TRACE;
    int l1, l2;
    const char *s1 = Jim_GetString(firstObjPtr, &l1);
    const char *s2 = Jim_GetString(secondObjPtr, &l2);

    if (nocase) {
        /* Do a character compare for nocase */
        return JimStringCompareLen(s1, s2, -1, nocase);
    }
    return JimStringCompare(s1, l1, s2, l2);
}

/**
 * Like Jim_StringCompareObj() except compares to a maximum of the length of firstObjPtr.
 *
 * Note: does not support embedded nulls
 */
JIM_EXPORT int Jim_StringCompareLenObj(Jim_InterpPtr interp, Jim_ObjPtr firstObjPtr, Jim_ObjPtr secondObjPtr, int nocase) // #JimStr
{
    PRJ_TRACE;
    const char *s1 = Jim_String(firstObjPtr);
    const char *s2 = Jim_String(secondObjPtr);

    return JimStringCompareLen(s1, s2, Jim_Utf8Length(interp, firstObjPtr), nocase);
}

/* Convert a range, as returned by Jim_GetRange(), into
 * an absolute index into an object of the specified length.
 * This function may return negative values, or values
 * greater than or equal to the length of the list if the index
 * is out of range. */
static int JimRelToAbsIndex(int len, int idx)
{
    PRJ_TRACE;
    if (idx < 0)
        return len + idx;
    return idx;
}

/* Convert a pair of indexes (*firstPtr, *lastPtr) as normalized by JimRelToAbsIndex(),
 * into a form suitable for implementation of commands like [string range] and [lrange].
 *
 * The resulting range is guaranteed to address valid elements of
 * the structure.
 */
static void JimRelToAbsRange(int len, int *firstPtr, int *lastPtr, int *rangeLenPtr)
{
    PRJ_TRACE;
    int rangeLen;

    if (*firstPtr > *lastPtr) {
        rangeLen = 0;
    }
    else {
        rangeLen = *lastPtr - *firstPtr + 1;
        if (rangeLen) {
            if (*firstPtr < 0) {
                rangeLen += *firstPtr;
                *firstPtr = 0;
            }
            if (*lastPtr >= len) {
                rangeLen -= (*lastPtr - (len - 1));
                *lastPtr = len - 1;
            }
        }
    }
    if (rangeLen < 0)
        rangeLen = 0;

    *rangeLenPtr = rangeLen;
}

static Retval JimStringGetRange(Jim_InterpPtr interp, Jim_ObjPtr firstObjPtr, Jim_ObjPtr lastObjPtr, // #JimStr
    int len, int *first, int *last, int *range)
{
    PRJ_TRACE;
    if (Jim_GetIndex(interp, firstObjPtr, first) != JIM_OK) {
        return JIM_ERR;
    }
    if (Jim_GetIndex(interp, lastObjPtr, last) != JIM_OK) {
        return JIM_ERR;
    }
    *first = JimRelToAbsIndex(len, *first);
    *last = JimRelToAbsIndex(len, *last);
    JimRelToAbsRange(len, first, last, range);
    return JIM_OK;
}

static Jim_ObjPtr Jim_StringByteRangeObj(Jim_InterpPtr interp,       // #MissInCoverage #JimStr
    Jim_ObjPtr strObjPtr, Jim_ObjPtr firstObjPtr, Jim_ObjPtr lastObjPtr)
{
    PRJ_TRACE;
    int first, last;
    const char *str;
    int rangeLen;
    int bytelen;

    str = Jim_GetString(strObjPtr, &bytelen);

    if (JimStringGetRange(interp, firstObjPtr, lastObjPtr, bytelen, &first, &last, &rangeLen) != JIM_OK) {
        return NULL;
    }

    if (first == 0 && rangeLen == bytelen) {
        return strObjPtr;
    }
    return Jim_NewStringObj(interp, str + first, rangeLen);
}

JIM_EXPORT Jim_ObjPtr Jim_StringRangeObj(Jim_InterpPtr interp, // #BiModeFunc #JimStr
    Jim_ObjPtr strObjPtr, Jim_ObjPtr firstObjPtr, Jim_ObjPtr lastObjPtr) 
{
    PRJ_TRACE;
    if (g_JIM_UTF8_VAL) {
        int first, last;
        const char* str;
        int len, rangeLen;
        int bytelen;

        str = Jim_GetString(strObjPtr, &bytelen);
        len = Jim_Utf8Length(interp, strObjPtr);

        if (JimStringGetRange(interp, firstObjPtr, lastObjPtr, len, &first, &last, &rangeLen) != JIM_OK) {
            return NULL;
        }

        if (first == 0 && rangeLen == len) {
            return strObjPtr;
        }
        if (len == bytelen) {
            /* ASCII optimization */
            return Jim_NewStringObj(interp, str + first, rangeLen);
        }
        return Jim_NewStringObjUtf8(interp, str + utf8_index(str, first), rangeLen);
    }

    return Jim_StringByteRangeObj(interp, strObjPtr, firstObjPtr, lastObjPtr); // #MissInCoverage

}

static Jim_ObjPtr JimStringReplaceObj(Jim_InterpPtr interp, // #JimStr
    Jim_ObjPtr strObjPtr, Jim_ObjPtr firstObjPtr, Jim_ObjPtr lastObjPtr, Jim_ObjPtr newStrObj)
{
    PRJ_TRACE;
    int first, last;
    const char *str;
    int len, rangeLen;
    Jim_ObjPtr objPtr;

    len = Jim_Utf8Length(interp, strObjPtr);

    if (JimStringGetRange(interp, firstObjPtr, lastObjPtr, len, &first, &last, &rangeLen) != JIM_OK) {
        return NULL;
    }

    if (last < first) {
        return strObjPtr;
    }

    str = Jim_String(strObjPtr);

    /* Before part */
    objPtr = Jim_NewStringObjUtf8(interp, str, first);

    /* Replacement */
    if (newStrObj) {
        Jim_AppendObj(interp, objPtr, newStrObj);
    }

    /* After part */
    Jim_AppendString(interp, objPtr, str + utf8_index(str, last + 1), len - last - 1);

    return objPtr;
}

/**
 * Note: does not support embedded nulls.
 */
static void JimStrCopyUpperLower(char *dest, const char *str, int uc) // #JimStr
{
    PRJ_TRACE;
    while (*str) {
        int c;
        str += utf8_tounicode(str, &c);
        dest += utf8_getchars(dest, uc ? utf8_upper(c) : utf8_lower(c));
    }
    *dest = 0;
}

/**
 * Note: does not support embedded nulls.
 */
static Jim_ObjPtr  JimStringToLower(Jim_InterpPtr  interp, Jim_ObjPtr  strObjPtr) { // #BiModeFunc #JimStr
    PRJ_TRACE;
    char* buf;
    int len;
    const char* str;

    str = Jim_GetString(strObjPtr, &len);

    if (g_JIM_UTF8_VAL) {
        /* Case mapping can change the utf-8 length of the string.
         * But at worst it will be by one extra byte per char
         */
        len *= 2;
    }
    buf = new_CharArray(len + 1); // #AllocF 
    JimStrCopyUpperLower(buf, str, 0);
    return Jim_NewStringObjNoAlloc(interp, buf, -1);
}

/**
 * Note: does not support embedded nulls.
 */
static Jim_ObjPtr JimStringToUpper(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr) // #BiModeFunc #JimStr
{
    PRJ_TRACE;
    char *buf;
    const char *str;
    int len;

    str = Jim_GetString(strObjPtr, &len);

    if (g_JIM_UTF8_VAL) {
        /* Case mapping can change the utf-8 length of the string.
         * But at worst it will be by one extra byte per char
         */
        len *= 2;
    }
    buf = new_CharArray(len + 1); // #AllocF 
    JimStrCopyUpperLower(buf, str, 1);
    return Jim_NewStringObjNoAlloc(interp, buf, -1);
}

/**
 * Note: does not support embedded nulls.
 */
static Jim_ObjPtr JimStringToTitle(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr) // #BiModeFunc  #JimStr
{
    PRJ_TRACE;
    char *buf, *p;
    int len;
    int c;
    const char *str;

    str = Jim_GetString(strObjPtr, &len);

    if (g_JIM_UTF8_VAL) {
        /* Case mapping can change the utf-8 length of the string.
         * But at worst it will be by one extra byte per char
         */
        len *= 2;
    }

    buf = p = new_CharArray(len+1); // #AllocF 

    str += utf8_tounicode(str, &c);
    p += utf8_getchars(p, utf8_title(c));

    JimStrCopyUpperLower(p, str, 0);

    return Jim_NewStringObjNoAlloc(interp, buf, -1);
}

/* Similar to memchr() except searches a UTF-8 string 'str' of byte length 'len'
 * for unicode character 'c'.
 * Returns the position if found or NULL if not
 */
static const char *utf8_memchr(const char *str, int len, int c) // #BiModeFunc #JimStr
{
    PRJ_TRACE;
    if (g_JIM_UTF8_VAL) {
        while (len) {
            int sc;
            int n = utf8_tounicode(str, &sc);
            if (sc == c) {
                return str;
            }
            str += n;
            len -= n;
        }
        return NULL;
    }
    return (const char*)memchr(str, c, len); // #MissInCoverage
}

/**
 * Searches for the first non-trim char in string (str, len)
 *
 * If none is found, returns just past the last char.
 *
 * Lengths are in bytes.
 */
static const char *JimFindTrimLeft(const char *str, int len, const char *trimchars, int trimlen) // #JimStr 
{
    PRJ_TRACE;
    while (len) {
        int c;
        int n = utf8_tounicode(str, &c);

        if (utf8_memchr(trimchars, trimlen, c) == NULL) {
            /* Not a trim char, so stop */
            break;
        }
        str += n;
        len -= n;
    }
    return str;
}

/**
 * Searches backwards for a non-trim char in string (str, len).
 *
 * Returns a pointer to just after the non-trim char, or NULL if not found.
 *
 * Lengths are in bytes.
 */
static const char *JimFindTrimRight(const char *str, int len, const char *trimchars, int trimlen) // #JimStr
{
    PRJ_TRACE;
    str += len;

    while (len) {
        int c;
        int n = utf8_prev_len(str, len);

        len -= n;
        str -= n;

        n = utf8_tounicode(str, &c);

        if (utf8_memchr(trimchars, trimlen, c) == NULL) {
            return str + n;
        }
    }

    return NULL;
}

static const char g_default_trim_chars[] = " \t\n\r";
/* sizeof() here includes the null byte */
static int g_default_trim_chars_len = sizeof(g_default_trim_chars);

static Jim_ObjPtr JimStringTrimLeft(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr trimcharsObjPtr) // #JimStr
{
    PRJ_TRACE;
    int len;
    const char *str = Jim_GetString(strObjPtr, &len);
    const char *trimchars = g_default_trim_chars;
    int trimcharslen = g_default_trim_chars_len;
    const char *newstr;

    if (trimcharsObjPtr) {
        trimchars = Jim_GetString(trimcharsObjPtr, &trimcharslen);
    }

    newstr = JimFindTrimLeft(str, len, trimchars, trimcharslen);
    if (newstr == str) {
        return strObjPtr;
    }

    return Jim_NewStringObj(interp, newstr, (int)(len - (newstr - str)));
}

STATIC Jim_ObjPtr JimStringTrimRight(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr trimcharsObjPtr) // #JimStr
{
    PRJ_TRACE;
    int len;
    const char *trimchars = g_default_trim_chars;
    int trimcharslen = g_default_trim_chars_len;
    const char *nontrim;

    if (trimcharsObjPtr) {
        trimchars = Jim_GetString(trimcharsObjPtr, &trimcharslen);
    }

    SetStringFromAny(interp, strObjPtr);

    len = Jim_Length(strObjPtr);
    nontrim = JimFindTrimRight(strObjPtr->bytes(), len, trimchars, trimcharslen);

    if (nontrim == NULL) {
        /* All trim, so return a zero-length string */
        return Jim_NewEmptyStringObj(interp);
    }
    if (nontrim == strObjPtr->bytes() + len) {
        /* All non-trim, so return the original object */
        return strObjPtr;
    }

    if (Jim_IsShared(strObjPtr)) {
        strObjPtr = Jim_NewStringObj(interp, strObjPtr->bytes(), (int)(nontrim - strObjPtr->bytes()));
    }
    else {
        /* Can modify this string in place */
        strObjPtr->setBytes(nontrim - strObjPtr->bytes(), 0);
        strObjPtr->setLength((int)(nontrim - strObjPtr->bytes()));
    }

    return strObjPtr;
}

static Jim_ObjPtr JimStringTrim(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr trimcharsObjPtr) // #JimStr
{
    PRJ_TRACE;
    /* First trim left. */
    Jim_ObjPtr objPtr = JimStringTrimLeft(interp, strObjPtr, trimcharsObjPtr);

    /* Now trim right */
    strObjPtr = JimStringTrimRight(interp, objPtr, trimcharsObjPtr);

    /* Note: refCount check is needed since objPtr may be emptyObj */
    if (objPtr != strObjPtr && objPtr->refCount() == 0) {
        /* We don't want this object to be leaked */
        Jim_FreeNewObj(interp, objPtr);
    }

    return strObjPtr;
}

/* Some platforms don't have isascii - need a non-macro version */
#ifdef HAVE_ISASCII // #optionalCode #WinOff
#define jim_isascii isascii
#else
static int jim_isascii(int c)
{
    return !(c & ~0x7f);
}
#endif

static Retval JimStringIs(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr strClass, int strict) // #JimStr
{
    PRJ_TRACE;
    static const char * const strclassnames[] = {
        "integer", "alpha", "alnum", "ascii", "digit",
        "double", "lower", "upper", "space", "xdigit",
        "control", "print", "graph", "punct", "boolean",
        NULL
    };
    enum {
        STR_IS_INTEGER, STR_IS_ALPHA, STR_IS_ALNUM, STR_IS_ASCII, STR_IS_DIGIT,
        STR_IS_DOUBLE, STR_IS_LOWER, STR_IS_UPPER, STR_IS_SPACE, STR_IS_XDIGIT,
        STR_IS_CONTROL, STR_IS_PRINT, STR_IS_GRAPH, STR_IS_PUNCT, STR_IS_BOOLEAN,
    };
    int strclass;
    int len;
    int i;
    const char *str;
    int (*isclassfunc)(int c) = NULL;

    if (Jim_GetEnum(interp, strClass, strclassnames, &strclass, "class", JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK) {
        return JIM_ERR;
    }

    str = Jim_GetString(strObjPtr, &len);
    if (len == 0) {
        Jim_SetResultBool(interp, !strict);
        return JIM_OK;
    }

    switch (strclass) {
        case STR_IS_INTEGER:
            {
                jim_wide w;
                Jim_SetResultBool(interp, JimGetWideNoErr(interp, strObjPtr, &w) == JIM_OK);
                return JIM_OK;
            }

        case STR_IS_DOUBLE:
            {
                double d;
                Jim_SetResultBool(interp, Jim_GetDouble(interp, strObjPtr, &d) == JIM_OK && errno != ERANGE);
                return JIM_OK;
            }

        case STR_IS_BOOLEAN:
            {
                int b;
                Jim_SetResultBool(interp, Jim_GetBoolean(interp, strObjPtr, &b) == JIM_OK);
                return JIM_OK;
            }

        case STR_IS_ALPHA: isclassfunc = isalpha; break;
        case STR_IS_ALNUM: isclassfunc = isalnum; break;
        case STR_IS_ASCII: isclassfunc = jim_isascii; break;
        case STR_IS_DIGIT: isclassfunc = isdigit; break;
        case STR_IS_LOWER: isclassfunc = islower; break;
        case STR_IS_UPPER: isclassfunc = isupper; break;
        case STR_IS_SPACE: isclassfunc = isspace; break;
        case STR_IS_XDIGIT: isclassfunc = isxdigit; break;
        case STR_IS_CONTROL: isclassfunc = iscntrl; break;
        case STR_IS_PRINT: isclassfunc = isprint; break;
        case STR_IS_GRAPH: isclassfunc = isgraph; break;
        case STR_IS_PUNCT: isclassfunc = ispunct; break;
        default:
            return JIM_ERR; // #MissInCoverage
    }

    for (i = 0; i < len; i++) {
        if (!isclassfunc(UCHAR(str[i]))) {
            Jim_SetResultBool(interp, 0);
            return JIM_OK;
        }
    }
    Jim_SetResultBool(interp, 1);
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * Compared String Object
 * ---------------------------------------------------------------------------*/

/* This is strange object that allows comparison of a C literal string
 * with a Jim object in a very short time if the same comparison is done
 * multiple times. For example every time the [if] command is executed,
 * Jim has to check if a given argument is "else".
 * If the code has no errors, this comparison is true most of the time,
 * so we can cache the pointer of the string of the last matching
 * comparison inside the object. Because most C compilers perform literal sharing,
 * so that: char *x = "foo", char *y = "foo", will lead to x == y,
 * this works pretty well even if comparisons are at different places
 * inside the C code. */

static const Jim_ObjType g_comparedStringObjType = { // #JimType #JimStrComp
    "compared-string",
    NULL,
    NULL,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& comparedStringType() { return g_comparedStringObjType; }

/* The only way this object is exposed to the API is via the following
 * function. Returns true if the string and the object string repr.
 * are the same, otherwise zero is returned.
 *
 * Note: this isn't binary safe, but it hardly needs to be.*/
JIM_EXPORT int Jim_CompareStringImmediate(Jim_InterpPtr interp, Jim_ObjPtr objPtr, const char *str) // #JimStrComp
{
    PRJ_TRACE;
    if (objPtr->typePtr() == &g_comparedStringObjType && objPtr->getVoidPtr() == str) {
        return 1;
    }
    else {
        if (strcmp(str, Jim_String(objPtr)) != 0)
            return 0;

        if (objPtr->typePtr() != &g_comparedStringObjType) {
            Jim_FreeIntRep(interp, objPtr);
            objPtr->setTypePtr(&g_comparedStringObjType);
        }
        objPtr->setPtr<char*>((char *)str);  /*ATTENTION: const cast */
        return 1;
    }
}

static int qsortCompareStringPointers(const void *a, const void *b) // #JimStrComp
{
    PRJ_TRACE;
    char *const *sa = (char *const *)a;
    char *const *sb = (char *const *)b;

    return strcmp(*sa, *sb);
}


/* -----------------------------------------------------------------------------
 * Source Object
 *
 * This object is just a string from the language point of view, but
 * the internal representation contains the filename and line number
 * where this token was read. This information is used by
 * Jim_EvalObj() if the object passed happens to be of type "source".
 *
 * This allows propagation of the information about line numbers and file
 * names and gives error messages with absolute line numbers.
 *
 * Note that this object uses the internal representation of the Jim_Object,
 * so there is almost no memory overhead. (One Jim_Obj for each filename).
 *
 * Also the object will be converted to something else if the given
 * token it represents in the source file is not something to be
 * evaluated (not a script), and will be specialized in some other way,
 * so the time overhead is also almost zero.
 * ---------------------------------------------------------------------------*/

STATIC void FreeSourceInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
STATIC void DupSourceInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);

static const Jim_ObjType g_sourceObjType = { // #JimType #JimSrc
    "source",
    FreeSourceInternalRepCB,
    DupSourceInternalRepCB,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& sourceType() { return g_sourceObjType; }


STATIC void FreeSourceInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimSrc
{
    PRJ_TRACE;
    Jim_DecrRefCount(interp, objPtr->get_sourceValue_fileName());
}

STATIC void DupSourceInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #JimSrc
{
    PRJ_TRACE;
    dupPtr->copy_sourceValue(srcPtr);
    //dupPtr->internalRep.sourceValue_ = srcPtr->internalRep.sourceValue_;
    Jim_IncrRefCount(dupPtr->get_sourceValue_fileName());
}

STATIC void JimSetSourceInfo(Jim_InterpPtr interp, Jim_ObjPtr objPtr, // #JimSrc
    Jim_ObjPtr fileNameObj, int lineNumber)
{
    PRJ_TRACE;
    JimPanic((Jim_IsShared(objPtr), "JimSetSourceInfo called with shared object"));
    JimPanic((objPtr->typePtr != NULL, "JimSetSourceInfo called with typed object"));
    Jim_IncrRefCount(fileNameObj);
    objPtr->set_sourceValue(fileNameObj, lineNumber);
    //objPtr->internalRep.sourceValue_.fileNameObj = fileNameObj;
    //objPtr->internalRep.sourceValue_.lineNumber = lineNumber;
    objPtr->setTypePtr(&g_sourceObjType);
}

/* -----------------------------------------------------------------------------
 * ScriptLine Object
 *
 * This object is used only in the Script internal representation.
 * For each line of the script, it holds the number of tokens on the line
 * and the source line number.
 */
static const Jim_ObjType g_scriptLineObjType = { // #JimType #JimScriptLine
    "scriptline",
    NULL,
    NULL,
    NULL,
    JIM_NONE,
};
const Jim_ObjType& scriptLineType() { return g_scriptLineObjType; }

STATIC Jim_ObjPtr JimNewScriptLineObj(Jim_InterpPtr interp, int argc, int line) // #JimScriptLine
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    if (g_DEBUG_SHOW_SCRIPT) {
        char buf[100]; // #MagicNum
        snprintf(buf, sizeof(buf), "line=%d, argc=%d", line, argc); // #MissInCoverage
        objPtr = Jim_NewStringObj(interp, buf, -1);
    } else {
        objPtr = Jim_NewEmptyStringObj(interp);
    }
    objPtr->setTypePtr(&g_scriptLineObjType);
    objPtr->setScriptLineValue(argc, line);
    //objPtr->internalRep.scriptLineValue_.argc = argc;
    //objPtr->internalRep.scriptLineValue_.line = line;

    return objPtr;
}

/* -----------------------------------------------------------------------------
 * Script Object
 *
 * This object holds the parsed internal representation of a script.
 * This representation is help within an allocated ScriptObj (see below)
 */
static void FreeScriptInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
static void DupScriptInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);

static const Jim_ObjType g_scriptObjType = { // #JimType #JimScript
    "script",
    FreeScriptInternalRepCB,
    DupScriptInternalRepCB,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& scriptType() { return g_scriptObjType; }

/* Each token of a script is represented by a ScriptToken.
 * The ScriptToken contains a type and a Jim_Obj. The Jim_Obj
 * can be specialized by commands operating on it.
 */
struct ScriptToken;
typedef ScriptToken* ScriptTokenPtr;

struct ScriptToken
{
private:
public:
    int type = 0;
    Jim_ObjPtr objPtr = NULL;
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_ScriptToken(sz)     Jim_TAlloc<ScriptToken>(sz,"ScriptToken")
#define free_ScriptToken(ptr)    Jim_TFree<ScriptToken>(ptr,"ScriptToken")

/* This is the script object internal representation. An array of
 * ScriptToken structures, including a pre-computed representation of the
 * command length and arguments.
 *
 * For example the script:
 *
 * puts hello
 * set $i $x$y [foo]BAR
 *
 * will produce a ScriptObj with the following ScriptToken's:
 *
 * LIN 2
 * ESC puts
 * ESC hello
 * LIN 4
 * ESC set
 * VAR i
 * WRD 2
 * VAR x
 * VAR y
 * WRD 2
 * CMD foo
 * ESC BAR
 *
 * "puts hello" has two args (LIN 2), composed of single tokens.
 * (Note that the WRD token is omitted for the common case of a single token.)
 *
 * "set $i $x$y [foo]BAR" has four (LIN 4) args, the first word
 * has 1 token (ESC SET), and the last has two tokens (WRD 2 CMD foo ESC BAR)
 *
 * The precomputation of the command structure makes Jim_Eval() faster,
 * and simpler because there aren't dynamic lengths / allocations.
 *
 * -- {expand}/{*} handling --
 *
 * Expand is handled in a special way.
 *
 *   If a "word" begins with {*}, the word token count is -ve.
 *
 * For example the command:
 *
 * list {*}{a b}
 *
 * Will produce the following cmdstruct array:
 *
 * LIN 2
 * ESC list
 * WRD -1
 * STR a b
 *
 * Note that the 'LIN' token also contains the source information for the
 * first word of the line for error reporting purposes
 *
 * -- the substFlags field of the structure --
 *
 * The scriptObj structure is used to represent both "script" objects
 * and "subst" objects. In the second case, there are no LIN and WRD
 * tokens. Instead SEP and EOL tokens are added as-is.
 * In addition, the field 'substFlags' is used to represent the flags used to turn
 * the string into the internal representation.
 * If these flags do not match what the application requires,
 * the scriptObj is created again. For example the script:
 *
 * subst -nocommands $string
 * subst -novariables $string
 *
 * Will (re)create the internal representation of the $string object
 * two times.
 */

struct ParseTokenList;
struct ParseToken;

struct ScriptObj // #JimScript
{
private:
    ScriptTokenPtr token = NULL;  /* Tokens array. */
    Jim_ObjPtr fileNameObj = NULL;  /* Filename */
    int len = 0;                  /* Length of token[] */
    int substFlags = 0;           /* flags used for the compilation of "subst" objects */
    int inUse = 0;                /* Used to share a ScriptObj. Currently
                                   only used by Jim_EvalObj() as protection against
                                   shimmering of the currently evaluated object. */
    int firstline = 0;            /* Line number of the first line */
    int linenr = 0;               /* Error line number, if any */
    int missing = 0;              /* Missing char if script failed to parse, (or space or backslash if OK) */

public:
    friend void FreeScriptInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC void ScriptObjAddTokens(Jim_InterpPtr interp, ScriptObj *script,
                                   ParseTokenListPtr tokenlist);
    friend STATIC void SubstObjAddTokens(Jim_InterpPtr interp, ScriptObj *script,
                                  ParseTokenListPtr tokenlist);
    friend int Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr);
    friend STATIC int SetSubstFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);
    friend int Jim_SubstObj(Jim_InterpPtr interp, Jim_ObjPtr substObjPtr, Jim_ObjArray *resObjPtrPtr, int flags);
    friend STATIC int Jim_ForCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend STATIC int JimCountWordTokens(ScriptObj *script, ParseTokenPtr t);
    friend int Jim_ScriptIsComplete(Jim_InterpPtr interp, Jim_ObjPtr scriptObj, char *stateCharPtr);
    friend STATIC void JimSetScriptFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC ScriptObj *JimGetScript(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC int JimScriptValid(Jim_InterpPtr interp, ScriptObj *script);
    friend STATIC void JimAddErrorToStack(Jim_InterpPtr interp, ScriptObj *script);
    friend STATIC int JimCallProcedure(Jim_InterpPtr interp, Jim_Cmd *cmd, int argc, Jim_ObjConstArray argv);
    friend STATIC ScriptObj *Jim_GetSubst(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);
    friend STATIC int Jim_DebugCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend STATIC int Jim_InfoCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_ScriptObj       Jim_TAllocZ<ScriptObj>(1,"ScriptObj")
#define free_ScriptObj(ptr) Jim_TFree<ScriptObj>(ptr,"ScriptObj")

STATIC void JimSetScriptFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
static Retval JimParseCheckMissing(Jim_InterpPtr interp, int ch);
STATIC ScriptObj *JimGetScript(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

static void FreeScriptInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimScript
{
    PRJ_TRACE;
    int i;
    ScriptObj *script = (ScriptObj *)objPtr->getVoidPtr();

    if (--script->inUse != 0)
        return;
    for (i = 0; i < script->len; i++) {
        Jim_DecrRefCount(interp, script->token[i].objPtr);
    }
    free_ScriptToken(script->token); // #FreeF 
    Jim_DecrRefCount(interp, script->fileNameObj);
    free_ScriptObj(script); // #FreeF 
}

static void DupScriptInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #MissInCoverage #JimScript
{
    PRJ_TRACE;
    JIM_NOTUSED(interp);
    JIM_NOTUSED(srcPtr);

    /* Just return a simple string. We don't try to preserve the source info
     * since in practice scripts are never duplicated
     */
    dupPtr->setTypePtr(NULL);
}

/* A simple parse token.
 * As the script is parsed, the created tokens point into the script string rep.
 */
struct ParseToken
{
private:
public:
    const char *token = NULL;   /* Pointer to the start of the token */
    int len = 0;                /* Length of this token */
    int type = 0;               /* Token type */
    int line = 0;               /* Line number */
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_ParseToken(sz)      Jim_TAlloc<ParseToken>(sz,"ParseToken")
#define free_ParseToken(ptr)    Jim_TFree<ParseToken>(ptr,"ParseToken")
#define realloc_ParseToken(orgPtr, newSz)  Jim_TRealloc<ParseToken>(orgPtr, newSz,"ParseToken")


/* A list of parsed tokens representing a script.
 * Tokens are added to this list as the script is parsed.
 * It grows as needed.
 */
struct ParseTokenList
{
private:
    ParseTokenPtr list = NULL;    /* Array of tokens */
    /* Start with a statically allocated list of tokens which will be expanded with realloc if needed */
    int size = 0;               /* Current size of the list */
    int count = 0;              /* Number of entries used */
    ParseToken static_list[20]; /* Small initial token space to avoid allocation #MagicNum */ 
public:

    friend STATIC void ScriptTokenListInit(ParseTokenListPtr tokenlist);
    friend STATIC void ScriptTokenListFree(ParseTokenListPtr tokenlist);
    friend STATIC void ScriptAddToken(ParseTokenListPtr tokenlist, const char *token, int len, int type,
                               int line);
    friend STATIC void ScriptObjAddTokens(Jim_InterpPtr interp, ScriptObj *script,
                                   ParseTokenListPtr tokenlist);
    friend STATIC void SubstObjAddTokens(Jim_InterpPtr interp, ScriptObj *script,
                                  ParseTokenListPtr tokenlist);
    friend STATIC ExprTreePtr ExprTreeCreateTree(Jim_InterpPtr interp, const ParseTokenListPtr tokenlist, Jim_ObjPtr exprObjPtr, Jim_ObjPtr fileNameObj);
    friend STATIC int SetExprFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
};

STATIC void ScriptTokenListInit(ParseTokenListPtr tokenlist)
{
    PRJ_TRACE;
    tokenlist->list = tokenlist->static_list;
    tokenlist->size = sizeof(tokenlist->static_list) / sizeof(ParseToken);
    tokenlist->count = 0;
}

STATIC void ScriptTokenListFree(ParseTokenListPtr tokenlist)
{
    PRJ_TRACE;
    if (tokenlist->list != tokenlist->static_list) {
        free_ParseToken(tokenlist->list); // #FreeF
    }
}

/**
 * Adds the new token to the tokenlist.
 * The token has the given length, type and line number.
 * The token list is resized as necessary.
 */
STATIC void ScriptAddToken(ParseTokenListPtr tokenlist, const char *token, int len, int type,
    int line)
{
    PRJ_TRACE;
    ParseTokenPtr t;

    if (tokenlist->count == tokenlist->size) {
        /* Resize the list */
        tokenlist->size *= 2;
        if (tokenlist->list != tokenlist->static_list) {
            tokenlist->list =
                realloc_ParseToken(tokenlist->list, tokenlist->size); // #AllocF 
        }
        else {
            /* The list needs to become allocated */
            tokenlist->list = new_ParseToken(tokenlist->size); // #AllocF 
            memcpy(tokenlist->list, tokenlist->static_list,
                tokenlist->count * sizeof(*tokenlist->list));
        }
    }
    t = &tokenlist->list[tokenlist->count++];
    t->token = token;
    t->len = len;
    t->type = type;
    t->line = line;
}

/* Counts the number of adjoining non-separator tokens.
 *
 * Returns -ve if the first token is the expansion
 * operator (in which case the count doesn't include
 * that token).
 */
STATIC int JimCountWordTokens(ScriptObj *script, ParseTokenPtr t)
{
    PRJ_TRACE;
    int expand = 1;
    int count = 0;

    /* Is the first word {*} or {expand}? */
    if (t->type == JIM_TT_STR && !TOKEN_IS_SEP(t[1].type)) {
        if ((t->len == 1 && *t->token == '*') || (t->len == 6 && strncmp(t->token, "expand", 6) == 0)) {
            /* Create an expand token */
            expand = -1;
            t++;
        }
        else {
            if (script->missing == ' ') {
                /* This is a "extra characters after close-brace" error. Report the first error */
                script->missing = '}';
                script->linenr = t[1].line;
            }
        }
    }

    /* Now count non-separator words */
    while (!TOKEN_IS_SEP(t->type)) {
        t++;
        count++;
    }

    return count * expand;
}

/**
 * Create a script/subst object from the given token.
 */
static Jim_ObjPtr JimMakeScriptObj(Jim_InterpPtr interp, const ParseTokenPtr t)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    if (t->type == JIM_TT_ESC && memchr(t->token, '\\', t->len) != NULL) {
        /* Convert backlash escapes. The result will never be longer than the original */
        int len = t->len;
        char* str = new_CharArray(len + 1); // #AllocF 
        len = JimEscape(str, t->token, len);
        objPtr = Jim_NewStringObjNoAlloc(interp, str, len);
    }
    else {
        /* XXX: For strict Tcl compatibility, JIM_TT_STR should replace <backslash><newline><whitespace>
         *         with a single space.
         */
        objPtr = Jim_NewStringObj(interp, t->token, t->len);
    }
    return objPtr;
}

/**
 * Takes a tokenlist and creates the allocated list of script tokens
 * in script->token, of length script->len.
 *
 * Unnecessary tokens are discarded, and LINE and WORD tokens are inserted
 * as required.
 *
 * Also sets script->line to the line number of the first token
 */
STATIC void ScriptObjAddTokens(Jim_InterpPtr interp, ScriptObj *script,
    ParseTokenListPtr tokenlist)
{
    PRJ_TRACE;
    int i;
    ScriptTokenPtr token;
    /* Number of tokens so far for the current command */
    int lineargs = 0;
    /* This is the first token for the current command */
    ScriptTokenPtr linefirst;
    int count;
    int linenr;

    if (g_DEBUG_SHOW_SCRIPT_TOKENS) {
        printf("==== Tokens ====\n"); // #stdoutput #MissInCoverage
        for (i = 0; i < tokenlist->count; i++) {
            printf("[%2d]@%d %s '%.*s'\n", i, tokenlist->list[i].line, jim_tt_name(tokenlist->list[i].type), // #stdoutput
                tokenlist->list[i].len, tokenlist->list[i].token);
        }
    }

    /* May need up to one extra script token for each EOL in the worst case */
    count = tokenlist->count;
    for (i = 0; i < tokenlist->count; i++) {
        if (tokenlist->list[i].type == JIM_TT_EOL) {
            count++;
        }
    }
    linenr = script->firstline = tokenlist->list[0].line;

    token = script->token = new_ScriptToken(count); // #AllocF 

    /* This is the first token for the current command */
    linefirst = token++;

    for (i = 0; i < tokenlist->count; ) {
        /* Look ahead to find out how many tokens make up the next word */
        int wordtokens;

        /* Skip any leading separators */
        while (tokenlist->list[i].type == JIM_TT_SEP) {
            i++;
        }

        wordtokens = JimCountWordTokens(script, tokenlist->list + i);

        if (wordtokens == 0) {
            /* None, so at end of line */
            if (lineargs) {
                linefirst->type = JIM_TT_LINE;
                linefirst->objPtr = JimNewScriptLineObj(interp, lineargs, linenr);
                Jim_IncrRefCount(linefirst->objPtr);

                /* Reset for new line */
                lineargs = 0;
                linefirst = token++;
            }
            i++;
            continue;
        }
        else if (wordtokens != 1) {
            /* More than 1, or {*}, so insert a WORD token */
            token->type = JIM_TT_WORD;
            token->objPtr = Jim_NewIntObj(interp, wordtokens);
            Jim_IncrRefCount(token->objPtr);
            token++;
            if (wordtokens < 0) {
                /* Skip the expand token */
                i++;
                wordtokens = -wordtokens - 1;
                lineargs--;
            }
        }

        if (lineargs == 0) {
            /* First real token on the line, so record the line number */
            linenr = tokenlist->list[i].line;
        }
        lineargs++;

        /* Add each non-separator word token to the line */
        while (wordtokens--) {
            const ParseTokenPtr t = &tokenlist->list[i++];

            token->type = t->type;
            token->objPtr = JimMakeScriptObj(interp, t);
            Jim_IncrRefCount(token->objPtr);

            /* Every object is initially a string of type 'source', but the
             * internal type may be specialized during execution of the
             * script. */
            JimSetSourceInfo(interp, token->objPtr, script->fileNameObj, t->line);
            token++;
        }
    }

    if (lineargs == 0) {
        token--;
    }

    script->len = (int)(token - script->token);

    JimPanic((script->len >= count, "allocated script array is too short"));

    if (g_DEBUG_SHOW_SCRIPT) { 
        printf("==== Script (%s) ====\n", Jim_String(script->fileNameObj)); // #stdoutput #MissInCoverage
        for (i = 0; i < script->len; i++) {
            const ScriptTokenPtr t = &script->token[i];
            printf("[%2d] %s %s\n", i, jim_tt_name(t->type), Jim_String(t->objPtr)); // #stdoutput
        }
    }
}

/* Parses the given string object to determine if it represents a complete script.
 *
 * This is useful for interactive shells implementation, for [info complete].
 *
 * If 'stateCharPtr' != NULL, the function stores ' ' on complete script,
 * '{' on scripts incomplete missing one or more '}' to be balanced.
 * '[' on scripts incomplete missing one or more ']' to be balanced.
 * '"' on scripts incomplete missing a '"' char.
 * '\\' on scripts with a trailing backslash.
 *
 * If the script is complete, 1 is returned, otherwise 0.
 *
 * If the script has extra characters after a close brace, this still returns 1,
 * but sets *stateCharPtr to '}'
 * Evaluating the script will give the error "extra characters after close-brace".
 */
JIM_EXPORT int Jim_ScriptIsComplete(Jim_InterpPtr interp, Jim_ObjPtr scriptObj, char *stateCharPtr)
{
    PRJ_TRACE;
    ScriptObj *script = JimGetScript(interp, scriptObj);
    if (stateCharPtr) {
        *stateCharPtr = script->missing;
    }
    return script->missing == ' ' || script->missing == '}';
}

/**
 * Sets an appropriate error message for a missing script/expression terminator.
 *
 * Returns JIM_ERR if 'ch' represents an unmatched/missing character.
 *
 * Note that a trailing backslash is not considered to be an error.
 */
static Retval JimParseCheckMissing(Jim_InterpPtr interp, int ch)
{
    PRJ_TRACE;
    const char *msg;

    switch (ch) {
        case '\\':
        case ' ':
            return JIM_OK;

        case '[':
            msg = "unmatched \"[\"";
            break;
        case '{':
            msg = "missing close-brace";
            break;
        case '}':
            msg = "extra characters after close-brace";
            break;
        case '"':
        default:
            msg = "missing quote";
            break;
    }

    Jim_SetResultString(interp, msg, -1);
    return JIM_ERR;
}

/**
 * Similar to ScriptObjAddTokens(), but for subst objects.
 */
STATIC void SubstObjAddTokens(Jim_InterpPtr interp, ScriptObj *script,
    ParseTokenListPtr tokenlist)
{
    PRJ_TRACE;
    int i;
    ScriptTokenPtr token;

    token = script->token = new_ScriptToken(tokenlist->count); // #AllocF 

    for (i = 0; i < tokenlist->count; i++) {
        const ParseTokenPtr t = &tokenlist->list[i];

        /* Create a token for 't' */
        token->type = t->type;
        token->objPtr = JimMakeScriptObj(interp, t);
        Jim_IncrRefCount(token->objPtr);
        token++;
    }

    script->len = i;
}

/* This method takes the string representation of an object
 * as a Tcl script, and generates the pre-parsed internal representation
 * of the script.
 *
 * On parse error, sets an error message and returns JIM_ERR
 * (Note: the object is still converted to a script, even if an error occurs)
 */
STATIC void JimSetScriptFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    int scriptTextLen;
    const char *scriptText = Jim_GetString(objPtr, &scriptTextLen);
    JimParserCtx parser;
    ScriptObj *script;
    ParseTokenList tokenlist;
    int line = 1;

    /* Try to get information about filename / line number */
    if (objPtr->typePtr() == &g_sourceObjType) {
        line = objPtr->get_sourceValue_lineNum();
    }

    /* Initially parse the script into tokens (in tokenlist) */
    ScriptTokenListInit(&tokenlist);

    JimParserInit(&parser, scriptText, scriptTextLen, line);
    while (!parser.eof) {
        JimParseScript(&parser);
        ScriptAddToken(&tokenlist, parser.tstart, 
            (int)(parser.tend - parser.tstart + 1), parser.tt,
            parser.tline);
    }

    /* Add a final EOF token */
    ScriptAddToken(&tokenlist, scriptText + scriptTextLen, 0, JIM_TT_EOF, 0);

    /* Create the "real" script tokens from the parsed tokens */
    script = new_ScriptObj; // #AllocF 
    //memset(script, 0, sizeof(*script));
    script->inUse = 1;
    if (objPtr->typePtr() == &g_sourceObjType) {
        script->fileNameObj = objPtr->get_sourceValue_fileName();
    }
    else {
        script->fileNameObj = interp->emptyObj();
    }
    Jim_IncrRefCount(script->fileNameObj);
    script->missing = parser.missing.ch;
    script->linenr = parser.missing.line;

    ScriptObjAddTokens(interp, script, &tokenlist);

    /* No longer need the token list */
    ScriptTokenListFree(&tokenlist);

    /* Free the old internal rep and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setPtr<ScriptObj*>(script);
    //Jim_SetIntRepPtr(objPtr, script);
    objPtr->setTypePtr(&g_scriptObjType);
}

STATIC void JimAddErrorToStack(Jim_InterpPtr interp, ScriptObj *script);

/**
 * Returns the parsed script.
 * Note that if there is any possibility that the script is not valid,
 * call JimScriptValid() to check
 */
STATIC ScriptObj *JimGetScript(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (objPtr == interp->emptyObj()) {
        /* Avoid converting emptyObj to a script. use nullScriptObj instead. */
        objPtr = interp->nullScriptObj(); // #MissInCoverage
    }

    if (objPtr->typePtr() != &g_scriptObjType || ((ScriptObj *)Jim_GetIntRepPtr(objPtr))->substFlags) {
        JimSetScriptFromAny(interp, objPtr);
    }

    return (ScriptObj *)Jim_GetIntRepPtr(objPtr);
}

/**
 * Returns 1 if the script is valid (parsed ok), otherwise returns 0
 * and leaves an error message in the interp result.
 *
 */
STATIC int JimScriptValid(Jim_InterpPtr interp, ScriptObj *script)
{
    PRJ_TRACE;
    if (JimParseCheckMissing(interp, script->missing) == JIM_ERR) {
        JimAddErrorToStack(interp, script);
        return 0;
    }
    return 1;
}


/* -----------------------------------------------------------------------------
 * Commands
 * ---------------------------------------------------------------------------*/
static void JimIncrCmdRefCount(Jim_Cmd *cmdPtr)
{
    PRJ_TRACE;
    cmdPtr->inUse_++;
}

static void JimDecrCmdRefCount(Jim_InterpPtr interp, Jim_Cmd *cmdPtr)
{
    PRJ_TRACE;
    if (--cmdPtr->inUse_ == 0) {
        if (cmdPtr->isproc()) {
            Jim_DecrRefCount(interp, cmdPtr->u.proc_.argListObjPtr);
            Jim_DecrRefCount(interp, cmdPtr->u.proc_.bodyObjPtr);
            Jim_DecrRefCount(interp, cmdPtr->u.proc_.nsObj);
            if (cmdPtr->u.proc_.staticVars) {
                Jim_FreeHashTable(cmdPtr->u.proc_.staticVars);
                free_Jim_HashTable(cmdPtr->u.proc_.staticVars); // #FreeF 
            }
        }
        else {
            /* native (C) */
            if (cmdPtr->delProc()) {
                cmdPtr->delProc()(interp, cmdPtr->u.native_.privData); // #note returns and calls functPtr.
            }
        }
        if (cmdPtr->prevCmd()) {
            /* Delete any pushed command too */
            JimDecrCmdRefCount(interp, cmdPtr->prevCmd());
        }
        free_Jim_Cmd(cmdPtr); // #FreeF 
    }
}

/* Variables HashTable Type.
 *
 * Keys are dynamically allocated strings, Values are Jim_Var structures.
 */
static void JimVariablesHTValDestructor(void *interp, void *val)
{
    PRJ_TRACE;
    Jim_DecrRefCount((Jim_InterpPtr )interp, ((Jim_Var *)val)->objPtr);
    Jim_TFree<void>(val,"void"); // #FreeF 
}

static const Jim_HashTableType g_JimVariablesHashTableType = {
    JimStringCopyHTHashFunction,        /* hash function */
    JimStringCopyHTDup,                 /* key dup */
    NULL,                               /* val dup */
    JimStringCopyHTKeyCompare,  /* key compare */
    JimStringCopyHTKeyDestructor,       /* key destructor */
    JimVariablesHTValDestructor /* val destructor */
};

/* Commands HashTable Type.
 *
 * Keys are dynamic allocated strings, Values are Jim_Cmd structures.
 */
static void JimCommandsHT_ValDestructor(void *interp, void *val)
{
    PRJ_TRACE;
    JimDecrCmdRefCount((Jim_InterpPtr )interp, (Jim_Cmd*)val);
}

static const Jim_HashTableType g_JimCommandsHashTableType = {
    JimStringCopyHTHashFunction,    /* hash function */
    JimStringCopyHTDup,             /* key dup */
    NULL,                           /* val dup */
    JimStringCopyHTKeyCompare,      /* key compare */
    JimStringCopyHTKeyDestructor,   /* key destructor */
    JimCommandsHT_ValDestructor     /* val destructor */
};

/* ------------------------- Commands related functions --------------------- */

#ifdef jim_ext_namespace // #optionalCode
/**
 * Returns the "unscoped" version of the given namespace.
 * That is, the fully qualified name without the leading ::
 * The returned value is either nsObj, or an object with a zero ref count.
 */
static Jim_ObjPtr JimQualifyNameObj(Jim_InterpPtr interp, Jim_ObjPtr nsObj)
{
    PRJ_TRACE;
    const char *name = Jim_String(nsObj);
    if (name[0] == ':' && name[1] == ':') {
        /* This command is being defined in the global namespace */
        while (*++name == ':') {
        }
        nsObj = Jim_NewStringObj(interp, name, -1);
    }
    else if (Jim_Length(interp->framePtr()->nsObj())) {
        /* This command is being defined in a non-global namespace */
        nsObj = Jim_DuplicateObj(interp, interp->framePtr()->nsObj()); // #MissInCoverage
        Jim_AppendStrings(interp, nsObj, "::", name, NULL);
    }
    return nsObj;
}

/**
 * If nameObjPtr starts with "::", returns it.
 * Otherwise returns a new object with nameObjPtr prefixed with "::".
 * In this case, decrements the ref count of nameObjPtr.
 */
JIM_EXPORT Jim_ObjPtr Jim_MakeGlobalNamespaceName(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr)
{
    PRJ_TRACE;
    Jim_ObjPtr resultObj;

    const char *name = Jim_String(nameObjPtr);
    if (name[0] == ':' && name[1] == ':') {
        return nameObjPtr; // #MissInCoverage
    }
    Jim_IncrRefCount(nameObjPtr);
    resultObj = Jim_NewStringObj(interp, "::", -1);
    Jim_AppendObj(interp, resultObj, nameObjPtr);
    Jim_DecrRefCount(interp, nameObjPtr);

    return resultObj;
}

/**
 * An efficient version of JimQualifyNameObj() where the name is
 * available (and needed) as a 'const char *'.
 * Avoids creating an object if not necessary.
 * The object stored in *objPtrPtr should be disposed of with JimFreeQualifiedName() after use.
 */
static const char *JimQualifyName(Jim_InterpPtr interp, const char *name, Jim_ObjArray *objPtrPtr)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr = interp->emptyObj();

    if (name[0] == ':' && name[1] == ':') {
        /* This command is being defined in the global namespace */
        while (*++name == ':') {
        }
    }
    else if (Jim_Length(interp->framePtr()->nsObj())) {
        /* This command is being defined in a non-global namespace */
        objPtr = Jim_DuplicateObj(interp, interp->framePtr()->nsObj());
        Jim_AppendStrings(interp, objPtr, "::", name, NULL);
        name = Jim_String(objPtr);
    }
    Jim_IncrRefCount(objPtr);
    *objPtrPtr = objPtr;
    return name;
}

    #define JimFreeQualifiedName(INTERP, OBJ) Jim_DecrRefCount((INTERP), (OBJ))

#else
    /* We can be more efficient in the no-namespace case */
    #define JimQualifyName(INTERP, NAME, DUMMY) (((NAME)[0] == ':' && (NAME)[1] == ':') ? (NAME) + 2 : (NAME))
    #define JimFreeQualifiedName(INTERP, DUMMY) (void)(DUMMY)

Jim_ObjPtr Jim_MakeGlobalNamespaceName(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr)
{
    return nameObjPtr;
}
#endif

static Retval JimCreateCommand(Jim_InterpPtr interp, const char *name, Jim_Cmd *cmd)
{
    PRJ_TRACE;
    /* It may already exist, so we try to delete the old one.
     * Note that reference count means that it won't be deleted yet if
     * it exists in the call stack.
     *
     * BUT, if 'local' is in force, instead of deleting the existing
     * proc, we stash a reference to the old proc here.
     */
    Jim_HashEntryPtr he = Jim_FindHashEntry(&interp->commands(), name);
    if (he) {
        /* There was an old cmd with the same name,
         * so this requires a 'proc epoch' update. */

        /* If a procedure with the same name didn't exist there is no need
         * to increment the 'proc epoch' because creation of a new procedure
         * can never affect existing cached commands. We don't do
         * negative caching. */
        Jim_InterpIncrProcEpoch(interp);
    }

    if (he && interp->local()) {
        /* Push this command over the top of the previous one */
        cmd->prevCmd_ = (Jim_Cmd*)Jim_GetHashEntryVal(he);
        Jim_SetHashVal(&interp->commands(), he, cmd);
    }
    else {
        if (he) {
            /* Replace the existing command */
            Jim_DeleteHashEntry(&interp->commands(), name);
        }

        Jim_AddHashEntry(&interp->commands(), name, cmd);
    }
    return JIM_OK;
}


JIM_EXPORT Retval Jim_CreateCommand(Jim_InterpPtr interp, const char *cmdName,
    Jim_CmdProc *cmdProc, void *privData, Jim_DelCmdProc *delProc)
{
    PRJ_TRACE;
    Jim_Cmd* cmdPtr = new_Jim_Cmd;  // #AllocF 

    /* Store the new details for this command */
    //memset(cmdPtr, 0, sizeof(*cmdPtr));
    cmdPtr->inUse_ = 1;
    cmdPtr->setDelProc(delProc);
    cmdPtr->setCmdProc(cmdProc);
    cmdPtr->u.native_.privData = privData;

    JimCreateCommand(interp, cmdName, cmdPtr);
    PRJ_TRACE_GEN(::prj_trace::ACTION_CMD_CREATE, cmdName, cmdPtr, NULL);

    return JIM_OK;
}

static Retval JimCreateProcedureStatics(Jim_InterpPtr interp, Jim_Cmd *cmdPtr, Jim_ObjPtr staticsListObjPtr)
{
    PRJ_TRACE;
    int len, i;

    len = Jim_ListLength(interp, staticsListObjPtr);
    if (len == 0) {
        return JIM_OK;
    }

    cmdPtr->u.proc_.staticVars = new_Jim_HashTable; // #AllocF 
    Jim_InitHashTable(cmdPtr->u.proc_.staticVars, &g_JimVariablesHashTableType, interp);
    cmdPtr->u.proc_.staticVars->setTypeName("staticVars");
    for (i = 0; i < len; i++) {
        Jim_Obj *objPtr, *initObjPtr, *nameObjPtr;
        Jim_Var *varPtr;
        int subLen;

        objPtr = Jim_ListGetIndex(interp, staticsListObjPtr, i);
        /* Check if it's composed of two elements. */
        subLen = Jim_ListLength(interp, objPtr);
        if (subLen == 1 || subLen == 2) {
            /* Try to get the variable value from the current
             * environment. */
            nameObjPtr = Jim_ListGetIndex(interp, objPtr, 0);
            if (subLen == 1) {
                initObjPtr = Jim_GetVariable(interp, nameObjPtr, JIM_NONE);
                if (initObjPtr == NULL) {
                    Jim_SetResultFormatted(interp,
                        "variable for initialization of static \"%#s\" not found in the local context",
                        nameObjPtr);
                    return JIM_ERR;
                }
            }
            else {
                initObjPtr = Jim_ListGetIndex(interp, objPtr, 1);
            }
            if (JimValidName(interp, "static variable", nameObjPtr) != JIM_OK) {
                return JIM_ERR;
            }

            varPtr = new_Jim_Var; // #AllocF 
            varPtr->objPtr = initObjPtr;
            Jim_IncrRefCount(initObjPtr);
            varPtr->linkFramePtr = NULL;
            if (Jim_AddHashEntry(cmdPtr->u.proc_.staticVars,
                Jim_String(nameObjPtr), varPtr) != JIM_OK) {
                Jim_SetResultFormatted(interp,
                    "static variable name \"%#s\" duplicated in statics list", nameObjPtr);
                Jim_DecrRefCount(interp, initObjPtr);
                free_Jim_Var(varPtr); // #FreeF 
                return JIM_ERR;
            }
        }
        else {
            Jim_SetResultFormatted(interp, "too many fields in static specifier \"%#s\"",
                objPtr);
            return JIM_ERR;
        }
    }
    return JIM_OK;
}

/**
 * If the command is a proc, sets/updates the cached namespace (nsObj)
 * based on the command name.
 */
static void JimUpdateProcNamespace(Jim_InterpPtr interp, Jim_Cmd *cmdPtr, const char *cmdname)
{
    PRJ_TRACE;
#ifdef jim_ext_namespace // #optionalCode
    if (cmdPtr->isproc()) {
        /* XXX: Really need JimNamespaceSplit() */
        const char *pt = strrchr(cmdname, ':');
        if (pt && pt != cmdname && pt[-1] == ':') {
            Jim_DecrRefCount(interp, cmdPtr->u.proc_.nsObj);
            cmdPtr->u.proc_.nsObj = Jim_NewStringObj(interp, cmdname, (int)(pt - cmdname - 1));
            Jim_IncrRefCount(cmdPtr->u.proc_.nsObj);

            if (Jim_FindHashEntry(&interp->commands(), pt + 1)) {
                /* This command shadows a global command, so a proc epoch update is required */
                Jim_InterpIncrProcEpoch(interp);
            }
        }
    }
#endif
}

static Jim_Cmd *JimCreateProcedureCmd(Jim_InterpPtr interp, Jim_ObjPtr argListObjPtr,
    Jim_ObjPtr staticsListObjPtr, Jim_ObjPtr bodyObjPtr, Jim_ObjPtr nsObj)
{
    PRJ_TRACE;
    Jim_Cmd *cmdPtr;
    int argListLen;
    int i;

    argListLen = Jim_ListLength(interp, argListObjPtr);

    /* Allocate space for both the command pointer and the arg list */
    cmdPtr = (Jim_Cmd*) new_CharArrayZ(sizeof(*cmdPtr) + sizeof(Jim_ProcArg) * argListLen); // #AllocF  #ComplicatedAlloc
    //memset(cmdPtr, 0, sizeof(*cmdPtr));
    cmdPtr->inUse_ = 1;
    cmdPtr->isproc_ = 1;
    cmdPtr->u.proc_.argListObjPtr = argListObjPtr;
    cmdPtr->u.proc_.argListLen = argListLen;
    cmdPtr->u.proc_.bodyObjPtr = bodyObjPtr;
    cmdPtr->u.proc_.argsPos = -1;
    cmdPtr->u.proc_.arglist = (Jim_ProcArg *)(cmdPtr + 1);
    cmdPtr->u.proc_.nsObj = nsObj ? nsObj : interp->emptyObj();
    Jim_IncrRefCount(argListObjPtr);
    Jim_IncrRefCount(bodyObjPtr);
    Jim_IncrRefCount(cmdPtr->u.proc_.nsObj);

    /* Create the statics hash table. */
    if (staticsListObjPtr && JimCreateProcedureStatics(interp, cmdPtr, staticsListObjPtr) != JIM_OK) {
        goto err;
    }

    /* Parse the args out into arglist, validating as we go */
    /* Examine the argument list for default parameters and 'args' */
    for (i = 0; i < argListLen; i++) {
        Jim_ObjPtr argPtr;
        Jim_ObjPtr nameObjPtr;
        Jim_ObjPtr defaultObjPtr;
        int len;

        /* Examine a parameter */
        argPtr = Jim_ListGetIndex(interp, argListObjPtr, i);
        len = Jim_ListLength(interp, argPtr);
        if (len == 0) {
            Jim_SetResultString(interp, "argument with no name", -1);
err:
            JimDecrCmdRefCount(interp, cmdPtr);
            return NULL;
        }
        if (len > 2) {
            Jim_SetResultFormatted(interp, "too many fields in argument specifier \"%#s\"", argPtr);
            goto err;
        }

        if (len == 2) {
            /* Optional parameter */
            nameObjPtr = Jim_ListGetIndex(interp, argPtr, 0);
            defaultObjPtr = Jim_ListGetIndex(interp, argPtr, 1);
        }
        else {
            /* Required parameter */
            nameObjPtr = argPtr;
            defaultObjPtr = NULL;
        }


        if (Jim_CompareStringImmediate(interp, nameObjPtr, "args")) {
            if (cmdPtr->u.proc_.argsPos >= 0) {
                Jim_SetResultString(interp, "'args' specified more than once", -1); // #MissInCoverage
                goto err;
            }
            cmdPtr->u.proc_.argsPos = i;
        }
        else {
            if (len == 2) {
                cmdPtr->u.proc_.optArity++;
            }
            else {
                cmdPtr->u.proc_.reqArity++;
            }
        }

        cmdPtr->u.proc_.arglist[i].nameObjPtr = nameObjPtr;
        cmdPtr->u.proc_.arglist[i].defaultObjPtr = defaultObjPtr;
    }
    PRJ_TRACE_GEN(::prj_trace::ACTION_PROC_CREATE, __FUNCTION__, cmdPtr, NULL);
    return cmdPtr;
}

JIM_EXPORT Retval Jim_DeleteCommand(Jim_InterpPtr interp, const char *cmdName)
{
    PRJ_TRACE;
    Retval ret = JIM_OK;
    Jim_ObjPtr qualifiedNameObj;
    const char *qualname = JimQualifyName(interp, cmdName, &qualifiedNameObj);

    if (Jim_DeleteHashEntry(&interp->commands(), qualname) == JIM_ERR) {
        Jim_SetResultFormatted(interp, "can't delete \"%s\": command doesn't exist", cmdName);
        ret = JIM_ERR;
    }
    else {
        Jim_InterpIncrProcEpoch(interp);
    }

    JimFreeQualifiedName(interp, qualifiedNameObj);

    return ret;
}

JIM_EXPORT Retval Jim_RenameCommand(Jim_InterpPtr interp, const char *oldName, const char *newName)
{
    PRJ_TRACE;
    Retval ret = JIM_ERR;
    Jim_HashEntryPtr he;
    Jim_Cmd *cmdPtr;
    Jim_ObjPtr qualifiedOldNameObj;
    Jim_ObjPtr qualifiedNewNameObj;
    const char *fqold;
    const char *fqnew;

    if (newName[0] == 0) {
        return Jim_DeleteCommand(interp, oldName);
    }

    fqold = JimQualifyName(interp, oldName, &qualifiedOldNameObj);
    fqnew = JimQualifyName(interp, newName, &qualifiedNewNameObj);

    /* Does it exist? */
    he = Jim_FindHashEntry(&interp->commands(), fqold);
    if (he == NULL) {
        Jim_SetResultFormatted(interp, "can't rename \"%s\": command doesn't exist", oldName);
    }
    else if (Jim_FindHashEntry(&interp->commands(), fqnew)) {
        Jim_SetResultFormatted(interp, "can't rename to \"%s\": command already exists", newName);
    }
    else {
        /* Add the new name first */
        cmdPtr = (Jim_Cmd*)Jim_GetHashEntryVal(he);
        JimIncrCmdRefCount(cmdPtr);
        JimUpdateProcNamespace(interp, cmdPtr, fqnew);
        Jim_AddHashEntry(&interp->commands(), fqnew, cmdPtr);

        /* Now remove the old name */
        Jim_DeleteHashEntry(&interp->commands(), fqold);

        /* Increment the epoch */
        Jim_InterpIncrProcEpoch(interp);

        ret = JIM_OK;
    }

    JimFreeQualifiedName(interp, qualifiedOldNameObj);
    JimFreeQualifiedName(interp, qualifiedNewNameObj);

    return ret;
}

/* -----------------------------------------------------------------------------
 * Command object
 * ---------------------------------------------------------------------------*/

STATIC void FreeCommandInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_DecrRefCount(interp, objPtr->get_cmdValue_nsObj());
}

STATIC void DupCommandInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr)
{
    PRJ_TRACE;
    dupPtr->setCmdValueCopy(srcPtr);
    //dupPtr->internalRep.cmdValue_ = srcPtr->internalRep.cmdValue_;
    dupPtr->setTypePtr(srcPtr->typePtr());
    Jim_IncrRefCount(dupPtr->get_cmdValue_nsObj());
}

static const Jim_ObjType g_commandObjType = { // #JimType #JimCmdObj
    "command",
    FreeCommandInternalRepCB,
    DupCommandInternalRepCB,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& commandType() { return g_commandObjType; }

/* This function returns the command structure for the command name
 * stored in objPtr. It specializes the objPtr to contain
 * cached info instead of performing the lookup into the hash table
 * every time. The information cached may not be up-to-date, in this
 * case the lookup is performed and the cache updated.
 *
 * Respects the 'upcall' setting.
 */
JIM_EXPORT Jim_Cmd *Jim_GetCommand(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags) // #JimCmdObj
{
    PRJ_TRACE;
    Jim_Cmd *cmd;

    /* In order to be valid, the proc epoch must match and
     * the lookup must have occurred in the same namespace
     */
    if (objPtr->typePtr() != &g_commandObjType ||
            objPtr->get_procEpoch_cmd() != interp->procEpoch()
#ifdef jim_ext_namespace // #optionalCode
            || !Jim_StringEqObj(objPtr->get_cmdValue_nsObj(), interp->framePtr()->nsObj())
#endif
        ) {
        /* Not cached or out of date, so lookup */

        /* Do we need to try the local namespace? */
        const char *name = Jim_String(objPtr);
        Jim_HashEntryPtr he;

        if (name[0] == ':' && name[1] == ':') {
            while (*++name == ':') {
            }
        }
#ifdef jim_ext_namespace // #optionalCode
        else if (Jim_Length(interp->framePtr()->nsObj())) {
            /* This command is being defined in a non-global namespace */
            Jim_ObjPtr nameObj = Jim_DuplicateObj(interp, interp->framePtr()->nsObj());
            Jim_AppendStrings(interp, nameObj, "::", name, NULL);
            he = Jim_FindHashEntry(&interp->commands(), Jim_String(nameObj));
            Jim_FreeNewObj(interp, nameObj);
            if (he) {
                goto found;
            }
        }
#endif

        /* Lookup in the global namespace */
        he = Jim_FindHashEntry(&interp->commands(), name);
        if (he == NULL) {
            if (flags & JIM_ERRMSG) {
                Jim_SetResultFormatted(interp, "invalid command name \"%#s\"", objPtr);
            }
            return NULL;
        }
#ifdef jim_ext_namespace // #optionalCode
found:
#endif
        cmd = (Jim_Cmd*)Jim_GetHashEntryVal(he);

        /* Free the old internal rep and set the new one. */
        Jim_FreeIntRep(interp, objPtr);
        objPtr->setTypePtr(&g_commandObjType);
        objPtr->setCmdValue(interp->framePtr()->nsObj(), cmd, interp->procEpoch());
        //objPtr->internalRep.cmdValue_.procEpoch = interp->procEpoch();
        //objPtr->internalRep.cmdValue_.cmdPtr = cmd;
        //objPtr->internalRep.cmdValue_.nsObj = interp->framePtr()->nsObj();
        Jim_IncrRefCount(interp->framePtr()->nsObj());
    }
    else {
        cmd = objPtr->get_cmdValue_cmd();
    }
    while (cmd->u.proc_.upcall) {
        cmd = cmd->prevCmd();
    }
    return cmd;
}

/* -----------------------------------------------------------------------------
 * Variables
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Variable object
 * ---------------------------------------------------------------------------*/

enum { JIM_DICT_SUGAR = 100 };     /* Only returned by SetVariableFromAny() */

STATIC Retval SetVariableFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

static const Jim_ObjType g_variableObjType = { // #JimType #JimVar
    "variable",
    NULL,
    NULL,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& variableType() { return g_variableObjType; }

/**
 * Check that the name does not contain embedded nulls.
 *
 * Variable and procedure names are manipulated as null terminated strings, so
 * don't allow names with embedded nulls.
 */
static Retval JimValidName(Jim_InterpPtr interp, const char *type, Jim_ObjPtr nameObjPtr) // #JimVar
{
    PRJ_TRACE;
    /* Variable names and proc names can't contain embedded nulls */
    if (nameObjPtr->typePtr() != &g_variableObjType) {
        int len;
        const char *str = Jim_GetString(nameObjPtr, &len);
        if (memchr(str, '\0', len)) {
            Jim_SetResultFormatted(interp, "%s name contains embedded null", type);
            return JIM_ERR;
        }
    }
    return JIM_OK;
}

/* This method should be called only by the variable API.
 * It returns JIM_OK on success (variable already exists),
 * JIM_ERR if it does not exist, JIM_DICT_SUGAR if it's not
 * a variable name, but syntax glue for [dict] i.e. the last
 * character is ')' */
STATIC Retval SetVariableFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimVar
{
    PRJ_TRACE;
    const char *varName;
    Jim_CallFramePtr framePtr;
    Jim_HashEntryPtr he;
    int global;
    int len;

    /* Check if the object is already an up to date variable */
    if (objPtr->typePtr() == &g_variableObjType) {
        framePtr = objPtr->get_varValue_global() ? interp->topFramePtr() : interp->framePtr();
        if (objPtr->get_varValue_callFrameId() == framePtr->id()) {
            /* nothing to do */
            return JIM_OK;
        }
        /* Need to re-resolve the variable in the updated callframe */
    }
    else if (objPtr->typePtr() == &g_dictSubstObjType) {
        return JIM_DICT_SUGAR;
    }
    else if (JimValidName(interp, "variable", objPtr) != JIM_OK) {
        return JIM_ERR;
    }


    varName = Jim_GetString(objPtr, &len);

    /* Make sure it's not syntax glue to get/set dict. */
    if (len && varName[len - 1] == ')' && strchr(varName, '(') != NULL) {
        return JIM_DICT_SUGAR;
    }

    if (varName[0] == ':' && varName[1] == ':') {
        while (*++varName == ':') {
        }
        global = 1;
        framePtr = interp->topFramePtr();
    }
    else {
        global = 0;
        framePtr = interp->framePtr();
    }

    /* Resolve this name in the variables hash table */
    he = Jim_FindHashEntry(&framePtr->vars(), varName);
    if (he == NULL) {
        if (!global && framePtr->staticVars()) {
            /* Try with static vars. */
            he = Jim_FindHashEntry(framePtr->staticVars(), varName);
        }
        if (he == NULL) {
            return JIM_ERR;
        }
    }

    /* Free the old internal repr and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_variableObjType);
    objPtr->setVarValue(framePtr->id(), (Jim_Var*) Jim_GetHashEntryVal(he), global);
    //objPtr->internalRep.varValue_.callFrameId = framePtr->id();
    //objPtr->internalRep.varValue_.varPtr = (Jim_Var*)Jim_GetHashEntryVal(he);
    //objPtr->internalRep.varValue_.global = global;
    return JIM_OK;
}

/* -------------------- Variables related functions ------------------------- */
STATIC Retval JimDictSugarSet(Jim_InterpPtr interp, Jim_ObjPtr ObjPtr, Jim_ObjPtr valObjPtr);
STATIC Jim_ObjPtr JimDictSugarGet(Jim_InterpPtr interp, Jim_ObjPtr ObjPtr, int flags);

STATIC Jim_Var *JimCreateVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, Jim_ObjPtr valObjPtr)
{
    PRJ_TRACE;
    const char *name;
    Jim_CallFramePtr framePtr;
    int global;

    /* New variable to create */
    Jim_Var* var = new_Jim_Var; // #AllocF 

    var->objPtr = valObjPtr;
    Jim_IncrRefCount(valObjPtr);
    var->linkFramePtr = NULL;

    name = Jim_String(nameObjPtr);
    if (name[0] == ':' && name[1] == ':') {
        while (*++name == ':') {
        }
        framePtr = interp->topFramePtr();
        global = 1;
    }
    else {
        framePtr = interp->framePtr();
        global = 0;
    }

    /* Insert the new variable */
    Jim_AddHashEntry(&framePtr->vars(), name, var);

    /* Make the object int rep a variable */
    Jim_FreeIntRep(interp, nameObjPtr);
    nameObjPtr->setTypePtr(&g_variableObjType);
    nameObjPtr->setVarValue(framePtr->id(), var, global);
    //nameObjPtr->internalRep.varValue_.callFrameId = framePtr->id();
    //nameObjPtr->internalRep.varValue_.varPtr = var;
    //nameObjPtr->internalRep.varValue_.global = global;

    return var;
}

/* For now that's dummy. Variables lookup should be optimized
 * in many ways, with caching of lookups, and possibly with
 * a table of pre-allocated vars in every CallFrame for local vars.
 * All the caching should also have an 'epoch' mechanism similar
 * to the one used by Tcl for procedures lookup caching. */

/**
 * Set the variable nameObjPtr to value valObjptr.
 */
JIM_EXPORT Retval Jim_SetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, Jim_ObjPtr valObjPtr)
{
    PRJ_TRACE;
    Retval err;
    Jim_Var *var;

    switch (SetVariableFromAny(interp, nameObjPtr)) {
        case JIM_DICT_SUGAR:
            return JimDictSugarSet(interp, nameObjPtr, valObjPtr);

        case JIM_ERR:
            if (JimValidName(interp, "variable", nameObjPtr) != JIM_OK) {
                return JIM_ERR;
            }
            JimCreateVariable(interp, nameObjPtr, valObjPtr);
            break;

        case JIM_OK:
            var = nameObjPtr->get_varValue_ptr();
            if (var->linkFramePtr == NULL) {
                Jim_IncrRefCount(valObjPtr);
                Jim_DecrRefCount(interp, var->objPtr);
                var->objPtr = valObjPtr;
            }
            else {                  /* Else handle the link */
                Jim_CallFramePtr savedCallFrame;

                savedCallFrame = interp->framePtr();
                interp->framePtr(var->linkFramePtr);
                err = Jim_SetVariable(interp, var->objPtr, valObjPtr);
                interp->framePtr(savedCallFrame);
                if (err != JIM_OK)
                    return err; // #MissInCoverage
            }
    }
    return JIM_OK;
}

JIM_EXPORT Retval Jim_SetVariableStr(Jim_InterpPtr interp, const char *name, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_ObjPtr nameObjPtr;
    Retval result;

    nameObjPtr = Jim_NewStringObj(interp, name, -1);
    Jim_IncrRefCount(nameObjPtr);
    result = Jim_SetVariable(interp, nameObjPtr, objPtr);
    Jim_DecrRefCount(interp, nameObjPtr);
    return result;
}

JIM_EXPORT Retval Jim_SetGlobalVariableStr(Jim_InterpPtr interp, const char *name, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_CallFramePtr savedFramePtr;
    Retval result;

    savedFramePtr = interp->framePtr();
    interp->framePtr(interp->topFramePtr());
    result = Jim_SetVariableStr(interp, name, objPtr);
    interp->framePtr(savedFramePtr);
    return result;
}

JIM_EXPORT Retval Jim_SetVariableStrWithStr(Jim_InterpPtr interp, const char *name, const char *val)
{
    PRJ_TRACE;
    Jim_ObjPtr valObjPtr;
    Retval result;

    valObjPtr = Jim_NewStringObj(interp, val, -1);
    Jim_IncrRefCount(valObjPtr);
    result = Jim_SetVariableStr(interp, name, valObjPtr);
    Jim_DecrRefCount(interp, valObjPtr);
    return result;
}

JIM_EXPORT Retval Jim_SetVariableLink(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr,
    Jim_ObjPtr targetNameObjPtr, Jim_CallFramePtr targetCallFrame)
{
    PRJ_TRACE;
    const char *varName;
    const char *targetName;
    Jim_CallFramePtr framePtr;
    Jim_Var *varPtr;

    /* Check for an existing variable or link */
    switch (SetVariableFromAny(interp, nameObjPtr)) {
        case JIM_DICT_SUGAR:
            /* XXX: This message seem unnecessarily verbose, but it matches Tcl */
            Jim_SetResultFormatted(interp, "bad variable name \"%#s\": upvar won't create a scalar variable that looks like an array element", nameObjPtr);
            return JIM_ERR;

        case JIM_OK:
            varPtr = nameObjPtr->get_varValue_ptr();

            if (varPtr->linkFramePtr == NULL) {
                Jim_SetResultFormatted(interp, "variable \"%#s\" already exists", nameObjPtr);
                return JIM_ERR;
            }

            /* It exists, but is a link, so first delete the link */
            varPtr->linkFramePtr = NULL;
            break;
    }

    /* Resolve the call frames for both variables */
    /* XXX: SetVariableFromAny() already did this! */
    varName = Jim_String(nameObjPtr);

    if (varName[0] == ':' && varName[1] == ':') {
        while (*++varName == ':') {
        }
        /* Linking a global var does nothing */
        framePtr = interp->topFramePtr();
    }
    else {
        framePtr = interp->framePtr();
    }

    targetName = Jim_String(targetNameObjPtr);
    if (targetName[0] == ':' && targetName[1] == ':') {
        while (*++targetName == ':') { // #MissInCoverage
        }
        targetNameObjPtr = Jim_NewStringObj(interp, targetName, -1);
        targetCallFrame = interp->topFramePtr();
    }
    Jim_IncrRefCount(targetNameObjPtr);

    if (framePtr->level() < targetCallFrame->level()) {
        Jim_SetResultFormatted(interp,
            "bad variable name \"%#s\": upvar won't create namespace variable that refers to procedure variable",
            nameObjPtr);
        Jim_DecrRefCount(interp, targetNameObjPtr);
        return JIM_ERR;
    }

    /* Check for cycles. */
    if (framePtr == targetCallFrame) {
        Jim_ObjPtr objPtr = targetNameObjPtr;

        /* Cycles are only possible with 'uplevel 0' */
        while (1) {
            if (strcmp(Jim_String(objPtr), varName) == 0) {
                Jim_SetResultString(interp, "can't upvar from variable to itself", -1);
                Jim_DecrRefCount(interp, targetNameObjPtr);
                return JIM_ERR;
            }
            if (SetVariableFromAny(interp, objPtr) != JIM_OK)
                break;
            varPtr = objPtr->get_varValue_ptr();
            if (varPtr->linkFramePtr != targetCallFrame)
                break;
            objPtr = varPtr->objPtr;
        }
    }

    /* Perform the binding */
    Jim_SetVariable(interp, nameObjPtr, targetNameObjPtr);
    /* We are now sure 'nameObjPtr' type is variableObjType */
    nameObjPtr->get_varValue_ptr()->linkFramePtr = targetCallFrame;
    Jim_DecrRefCount(interp, targetNameObjPtr);
    return JIM_OK;
}

/* Return the Jim_Obj pointer associated with a variable name,
 * or NULL if the variable was not found in the current context.
 * The same optimization discussed in the comment to the
 * 'SetVariable' function should apply here.
 *
 * If JIM_UNSHARED is set and the variable is an array element (dict sugar)
 * in a dictionary which is shared, the array variable value is duplicated first.
 * This allows the array element to be updated (e.g. append, lappend) without
 * affecting other references to the dictionary.
 */
Jim_ObjPtr Jim_GetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags)
{
    PRJ_TRACE;
    switch (SetVariableFromAny(interp, nameObjPtr)) {
        case JIM_OK:{
                Jim_Var *varPtr = nameObjPtr->get_varValue_ptr();

                if (varPtr->linkFramePtr == NULL) {
                    return varPtr->objPtr;
                }
                else {
                    Jim_ObjPtr objPtr;

                    /* The variable is a link? Resolve it. */
                    Jim_CallFramePtr savedCallFrame = interp->framePtr();

                    interp->framePtr(varPtr->linkFramePtr);
                    objPtr = Jim_GetVariable(interp, varPtr->objPtr, flags);
                    interp->framePtr(savedCallFrame);
                    if (objPtr) {
                        return objPtr;
                    }
                    /* Error, so fall through to the error message */
                }
            }
            break;

        case JIM_DICT_SUGAR:
            /* [dict] syntax sugar. */
            return JimDictSugarGet(interp, nameObjPtr, flags);
    }
    if (flags & JIM_ERRMSG) {
        Jim_SetResultFormatted(interp, "can't read \"%#s\": no such variable", nameObjPtr);
    }
    return NULL;
}

JIM_EXPORT Jim_ObjPtr Jim_GetGlobalVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags)
{
    PRJ_TRACE;
    Jim_CallFramePtr savedFramePtr;
    Jim_ObjPtr objPtr;

    savedFramePtr = interp->framePtr();
    interp->framePtr(interp->topFramePtr());
    objPtr = Jim_GetVariable(interp, nameObjPtr, flags);
    interp->framePtr(savedFramePtr);

    return objPtr;
}

JIM_EXPORT Jim_ObjPtr Jim_GetVariableStr(Jim_InterpPtr interp, const char *name, int flags)
{
    PRJ_TRACE;
    Jim_Obj* nameObjPtr, *varObjPtr;

    nameObjPtr = Jim_NewStringObj(interp, name, -1);
    Jim_IncrRefCount(nameObjPtr);
    varObjPtr = Jim_GetVariable(interp, nameObjPtr, flags);
    Jim_DecrRefCount(interp, nameObjPtr);
    return varObjPtr;
}

JIM_EXPORT Jim_ObjPtr Jim_GetGlobalVariableStr(Jim_InterpPtr interp, const char *name, int flags)
{
    PRJ_TRACE;
    Jim_CallFramePtr savedFramePtr;
    Jim_ObjPtr objPtr;

    savedFramePtr = interp->framePtr();
    interp->framePtr(interp->topFramePtr());
    objPtr = Jim_GetVariableStr(interp, name, flags);
    interp->framePtr(savedFramePtr);

    return objPtr;
}

/* Unset a variable.
 * Note: On success unset invalidates all the (cached) variable objects
 * by incrementing callFrameEpoch
 */
JIM_EXPORT Retval Jim_UnsetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags)
{
    PRJ_TRACE;
    Jim_Var *varPtr;
    Retval retval;
    Jim_CallFramePtr framePtr;

    retval = SetVariableFromAny(interp, nameObjPtr);
    if (retval == JIM_DICT_SUGAR) {
        /* [dict] syntax sugar. */
        return JimDictSugarSet(interp, nameObjPtr, NULL);
    }
    else if (retval == JIM_OK) {
        varPtr = nameObjPtr->get_varValue_ptr();

        /* If it's a link call UnsetVariable recursively */
        if (varPtr->linkFramePtr) {
            framePtr = interp->framePtr();
            interp->framePtr(varPtr->linkFramePtr);
            retval = Jim_UnsetVariable(interp, varPtr->objPtr, JIM_NONE);
            interp->framePtr(framePtr);
        }
        else {
            const char *name = Jim_String(nameObjPtr);
            if (nameObjPtr->get_varValue_global()) {
                name += 2;
                framePtr = interp->topFramePtr();
            }
            else {
                framePtr = interp->framePtr();
            }

            retval = Jim_DeleteHashEntry(&framePtr->vars(), name);
            if (retval == JIM_OK) {
                /* Change the callframe id, invalidating var lookup caching */
                framePtr->id_ = interp->callFrameEpoch(); interp->incrCallFrameEpoch();
            }
        }
    }
    if (retval != JIM_OK && (flags & JIM_ERRMSG)) {
        Jim_SetResultFormatted(interp, "can't unset \"%#s\": no such variable", nameObjPtr);
    }
    return retval;
}

/* ----------  Dict syntax sugar (similar to array Tcl syntax) -------------- */

/* Given a variable name for [dict] operation syntax sugar,
 * this function returns two objects, the first with the name
 * of the variable to set, and the second with the respective key.
 * For example "foo(bar)" will return objects with string repr. of
 * "foo" and "bar".
 *
 * The returned objects have refcount = 1. The function can't fail. */
static void JimDictSugarParseVarKey(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                    Jim_ObjArray *varPtrPtr, Jim_ObjArray *keyPtrPtr)
{
    PRJ_TRACE;
    const char *str, *p;
    int len, keyLen;
    Jim_Obj *varObjPtr, *keyObjPtr;

    str = Jim_GetString(objPtr, &len);

    p = strchr(str, '(');
    JimPanic((p == NULL, "JimDictSugarParseVarKey() called for non-dict-sugar (%s)", str));

    varObjPtr = Jim_NewStringObj(interp, str, (int)(p - str));

    p++;
    keyLen = (int)((str + len) - p);
    if (str[len - 1] == ')') {
        keyLen--;
    }

    /* Create the objects with the variable name and key. */
    keyObjPtr = Jim_NewStringObj(interp, p, keyLen);

    Jim_IncrRefCount(varObjPtr);
    Jim_IncrRefCount(keyObjPtr);
    *varPtrPtr = varObjPtr;
    *keyPtrPtr = keyObjPtr;
}

/* Helper of Jim_SetVariable() to deal with dict-syntax variable names.
 * Also used by Jim_UnsetVariable() with valObjPtr = NULL. */
STATIC Retval JimDictSugarSet(Jim_InterpPtr interp, Jim_ObjPtr ObjPtr, Jim_ObjPtr valObjPtr)
{
    PRJ_TRACE;
    Retval err;

    SetDictSubstFromAny(interp, ObjPtr);

    err = Jim_SetDictKeysVector(interp, ObjPtr->get_dictSubstValue_varName(),
        &ObjPtr->get_dictSubstValue_indexRef(), 1, valObjPtr, JIM_MUSTEXIST);

    if (err == JIM_OK) {
        /* Don't keep an extra ref to the result */
        Jim_SetEmptyResult(interp);
    }
    else {
        if (!valObjPtr) {
            /* Better error message for unset a(2) where a exists but a(2) doesn't */
            if (Jim_GetVariable(interp, ObjPtr->get_dictSubstValue_varName(), JIM_NONE)) {
                Jim_SetResultFormatted(interp, "can't unset \"%#s\": no such element in array",
                    ObjPtr);
                return err;
            }
        }
        /* Make the error more informative and Tcl-compatible */
        Jim_SetResultFormatted(interp, "can't %s \"%#s\": variable isn't array",
            (valObjPtr ? "set" : "unset"), ObjPtr);
    }
    return err;
}

/**
 * Expands the array variable (dict sugar) and returns the result, or NULL on error.
 *
 * If JIM_UNSHARED is set and the dictionary is shared, it will be duplicated
 * and stored back to the variable before expansion.
 */
static Jim_ObjPtr JimDictExpandArrayVariable(Jim_InterpPtr interp, Jim_ObjPtr varObjPtr,
    Jim_ObjPtr keyObjPtr, int flags)
{
    PRJ_TRACE;
    Jim_ObjPtr dictObjPtr;
    Jim_ObjPtr resObjPtr = NULL;
    int ret;

    dictObjPtr = Jim_GetVariable(interp, varObjPtr, JIM_ERRMSG);
    if (!dictObjPtr) {
        return NULL;
    }

    ret = Jim_DictKey(interp, dictObjPtr, keyObjPtr, &resObjPtr, JIM_NONE);
    if (ret != JIM_OK) {
        Jim_SetResultFormatted(interp,
            "can't read \"%#s(%#s)\": %s array", varObjPtr, keyObjPtr,
            ret < 0 ? "variable isn't" : "no such element in");
    }
    else if ((flags & JIM_UNSHARED) && Jim_IsShared(dictObjPtr)) {
        /* Update the variable to have an unshared copy */
        Jim_SetVariable(interp, varObjPtr, Jim_DuplicateObj(interp, dictObjPtr));
    }

    return resObjPtr;
}

/* Helper of Jim_GetVariable() to deal with dict-syntax variable names */
STATIC Jim_ObjPtr JimDictSugarGet(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags)
{
    PRJ_TRACE;
    SetDictSubstFromAny(interp, objPtr);

    return JimDictExpandArrayVariable(interp,
        objPtr->get_dictSubstValue_varName(),
        objPtr->get_dictSubstValue_index(), flags);
}

/* --------- $var(INDEX) substitution, using a specialized object ----------- */

STATIC void FreeDictSubstInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_DecrRefCount(interp, objPtr->get_dictSubstValue_varName());
    Jim_DecrRefCount(interp, objPtr->get_dictSubstValue_index());
}

STATIC void DupDictSubstInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #MissInCoverage
{
    PRJ_TRACE;
    /* Copy the internal rep */
    dupPtr->copyInterpRep(srcPtr);
    /* Need to increment the ref counts */
    Jim_IncrRefCount(dupPtr->get_dictSubstValue_varName());
    Jim_IncrRefCount(dupPtr->get_dictSubstValue_index());
}

/* Note: The object *must* be in dict-sugar format */
STATIC void SetDictSubstFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_dictSubstObjType) {
        Jim_Obj* varObjPtr, *keyObjPtr;

        if (objPtr->typePtr() == &g_interpolatedObjType) {
            /* An interpolated object in dict-sugar form */

            varObjPtr = objPtr->get_dictSubstValue_varName();
            keyObjPtr = objPtr->get_dictSubstValue_index();

            Jim_IncrRefCount(varObjPtr);
            Jim_IncrRefCount(keyObjPtr);
        }
        else {
            JimDictSugarParseVarKey(interp, objPtr, &varObjPtr, &keyObjPtr);
        }

        Jim_FreeIntRep(interp, objPtr);
        objPtr->setTypePtr(&g_dictSubstObjType);
        objPtr->setDictSubstValue(varObjPtr, keyObjPtr);
        //objPtr->internalRep.dictSubstValue_.varNameObjPtr = varObjPtr;
        //objPtr->internalRep.dictSubstValue_.indexObjPtr = keyObjPtr;
    }
}

/* This function is used to expand [dict get] sugar in the form
 * of $var(INDEX). The function is mainly used by Jim_EvalObj()
 * to deal with tokens of type JIM_TT_DICTSUGAR. objPtr points to an
 * object that is *guaranteed* to be in the form VARNAME(INDEX).
 * The 'index' part is [subst]ituted, and is used to lookup a key inside
 * the [dict]ionary contained in variable VARNAME. */
STATIC Jim_ObjPtr JimExpandDictSugar(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_ObjPtr resObjPtr = NULL;
    Jim_ObjPtr substKeyObjPtr = NULL;

    SetDictSubstFromAny(interp, objPtr);

    if (Jim_SubstObj(interp, objPtr->get_dictSubstValue_index(),
            &substKeyObjPtr, JIM_NONE)
        != JIM_OK) {
        return NULL;
    }
    Jim_IncrRefCount(substKeyObjPtr);
    resObjPtr =
        JimDictExpandArrayVariable(interp, objPtr->get_dictSubstValue_varName(),
        substKeyObjPtr, 0);
    Jim_DecrRefCount(interp, substKeyObjPtr);

    return resObjPtr;
}

static Jim_ObjPtr JimExpandExprSugar(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (Jim_EvalExpression(interp, objPtr) == JIM_OK) {
        return Jim_GetResult(interp);
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 * CallFrame
 * ---------------------------------------------------------------------------*/

STATIC Jim_CallFramePtr JimCreateCallFrame(Jim_InterpPtr interp, Jim_CallFramePtr parent, Jim_ObjPtr nsObj)
{
    PRJ_TRACE;
    Jim_CallFramePtr cf;

    if (interp->freeFramesList()) {
        cf = interp->freeFramesList();
        interp->setFreeFramesList(cf->next);

        cf->argv_ = NULL;
        cf->argc_ = 0;
        cf->procArgsObjPtr_ = NULL;
        cf->procBodyObjPtr_ = NULL;
        cf->next = NULL;
        cf->staticVars_ = NULL;
        cf->localCommands_ = NULL;
        cf->tailcallObj_ = NULL;
        cf->tailcallCmd_ = NULL;
    }
    else {
        cf = new_Jim_CallFrame; // #AllocF 
        //memset(cf, 0, sizeof(*cf));

        Jim_InitHashTable(&cf->vars(), &g_JimVariablesHashTableType, interp);
        cf->vars().setTypeName("variables");
    }

    cf->id_ = interp->callFrameEpoch(); interp->incrCallFrameEpoch();
    cf->parent_ = parent;
    cf->level_ = parent ? parent->level() + 1 : 0;
    cf->nsObj_ = nsObj;
    Jim_IncrRefCount(nsObj);
    PRJ_TRACE_GEN(::prj_trace::ACTION_CALLFRAME_CREATE, __FUNCTION__, cf, NULL);

    return cf;
}

static Retval JimDeleteLocalProcs(Jim_InterpPtr interp, Jim_StackPtr localCommands)
{
    PRJ_TRACE;
    /* Delete any local procs */
    if (localCommands) {
        Jim_ObjPtr cmdNameObj;

        while ((cmdNameObj = (Jim_ObjPtr )Jim_StackPop(localCommands)) != NULL) {
            Jim_HashEntryPtr he;
            Jim_ObjPtr fqObjName;
            Jim_HashTablePtr ht = &interp->commands();

            const char *fqname = JimQualifyName(interp, Jim_String(cmdNameObj), &fqObjName);

            he = Jim_FindHashEntry(ht, fqname);

            if (he) {
                Jim_Cmd *cmd = (Jim_Cmd*)Jim_GetHashEntryVal(he);
                if (cmd->prevCmd()) {
                    Jim_Cmd *prevCmd = cmd->prevCmd();
                    cmd->prevCmd_ = NULL;

                    /* Delete the old command */
                    JimDecrCmdRefCount(interp, cmd);

                    /* And restore the original */
                    Jim_SetHashVal(ht, he, prevCmd);
                }
                else {
                    Jim_DeleteHashEntry(ht, fqname);
                }
                Jim_InterpIncrProcEpoch(interp);
            }
            Jim_DecrRefCount(interp, cmdNameObj);
            JimFreeQualifiedName(interp, fqObjName);
        }
        Jim_FreeStack(localCommands);
        free_Jim_Stack(localCommands); // #FreeF 
    }
    return JIM_OK;
}

/**
 * Run any $jim::defer scripts for the current call frame.
 *
 * retcode is the return code from the current proc.
 *
 * Returns the new return code.
 */
static Retval JimInvokeDefer(Jim_InterpPtr interp, Retval retcode)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    /* Fast check for the likely case that the variable doesn't exist */
    if (Jim_FindHashEntry(&interp->framePtr()->vars(), "jim::defer") == NULL) {
        return retcode;
    }

    objPtr = Jim_GetVariableStr(interp, "jim::defer", JIM_NONE);

    if (objPtr) {
        int ret = JIM_OK;
        int i;
        int listLen = Jim_ListLength(interp, objPtr);
        Jim_ObjPtr resultObjPtr;

        Jim_IncrRefCount(objPtr);

        /* Need to save away the current interp result and
         * restore it if appropriate
         */
        resultObjPtr = Jim_GetResult(interp);
        Jim_IncrRefCount(resultObjPtr);
        Jim_SetEmptyResult(interp);

        /* Invoke in reverse order */
        for (i = listLen; i > 0; i--) {
            /* If a defer script returns an error, don't evaluate remaining scripts */
            Jim_ObjPtr scriptObjPtr = Jim_ListGetIndex(interp, objPtr, i - 1);
            ret = Jim_EvalObj(interp, scriptObjPtr);
            if (ret != JIM_OK) {
                break;
            }
        }

        if (ret == JIM_OK || retcode == JIM_ERR) {
            /* defer script had no error, or proc had an error so restore proc result */
            Jim_SetResult(interp, resultObjPtr);
        }
        else {
            retcode = ret;
        }

        Jim_DecrRefCount(interp, resultObjPtr);
        Jim_DecrRefCount(interp, objPtr);
    }
    return retcode;
}

enum {
    JIM_FCF_FULL = 0,          /* Always free the vars hash table */
    JIM_FCF_REUSE = 1         /* Reuse the vars hash table if possible */
};

STATIC void JimFreeCallFrame(Jim_InterpPtr interp, Jim_CallFramePtr cf, int action)
{
    PRJ_TRACE;
    PRJ_TRACE_GEN(::prj_trace::ACTION_CALLFRAME_DELETE, __FUNCTION__, cf, NULL);
    JimDeleteLocalProcs(interp, cf->localCommands());

    if (cf->procArgsObjPtr())
        Jim_DecrRefCount(interp, cf->procArgsObjPtr_);
    if (cf->procBodyObjPtr())
        Jim_DecrRefCount(interp, cf->procBodyObjPtr_);
    Jim_DecrRefCount(interp, cf->nsObj());
    if (action == JIM_FCF_FULL || cf->vars().size() != JIM_HT_INITIAL_SIZE)
        Jim_FreeHashTable(&cf->vars());
    else {
        int i;
        Jim_HashEntry **table = cf->vars().table_, *he;

        for (i = 0; i < JIM_HT_INITIAL_SIZE; i++) {
            he = table[i];
            while (he != NULL) {
                Jim_HashEntryPtr nextEntry = he->next();
                Jim_Var *varPtr = (Jim_Var*)Jim_GetHashEntryVal(he);

                Jim_DecrRefCount(interp, varPtr->objPtr);
                Jim_TFreeNR<void>(Jim_GetHashEntryKey(he),"void"); // #FreeF 
                free_Jim_Var(varPtr); // #FreeF 
                free_Jim_HashEntry(he); // #FreeF 
                table[i] = NULL;
                he = nextEntry;
            }
        }
        cf->vars().used_ = 0;
    }
    cf->next = interp->freeFramesList();
    interp->setFreeFramesList(cf);
}


/* -----------------------------------------------------------------------------
 * References
 * ---------------------------------------------------------------------------*/
#if defined(JIM_REFERENCES) && !defined(JIM_BOOTSTRAP) // #optionalCode

/* References HashTable Type.
 *
 * Keys are unsigned_long integers, dynamically allocated for now but in the
 * future it's worth to cache this 4 bytes objects. Values are pointers
 * to Jim_References. */
static void JimReferencesHTValDestructor(void *interp, void *val)
{
    PRJ_TRACE;
    Jim_Reference *refPtr = (Jim_Reference *)val;

    Jim_DecrRefCount((Jim_InterpPtr )interp, refPtr->objPtr);
    if (refPtr->finalizerCmdNamePtr != NULL) {
        Jim_DecrRefCount((Jim_InterpPtr )interp, refPtr->finalizerCmdNamePtr);
    }
    Jim_TFree<void>(val,"void"); // #FreeF 
}

static unsigned_int JimReferencesHTHashFunction(const void *key)
{
    PRJ_TRACE;
    /* Only the least significant bits are used. */
    const_unsigned_long *widePtr = (const_unsigned_long *)key;
    unsigned_int intValue = (unsigned_int)*widePtr;

    return Jim_IntHashFunction(intValue);
}

static void *JimReferencesHTKeyDup(void *privdata, const void *key)
{
    PRJ_TRACE;
    void* copy = (void*)Jim_TAlloc<unsigned_long>(1,"unsigned_long"); // #AllocF 

    JIM_NOTUSED(privdata);

    memcpy(copy, key, sizeof(unsigned_long));
    return copy;
}

static int JimReferencesHTKeyCompare(void *privdata, const void *key1, const void *key2)
{
    PRJ_TRACE;
    JIM_NOTUSED(privdata);

    return memcmp(key1, key2, sizeof(unsigned_long)) == 0;
}

static void JimReferencesHTKeyDestructor(void *privdata, void *key)
{
    PRJ_TRACE;
    JIM_NOTUSED(privdata);

    Jim_TFree<void>(key,"void"); // #FreeF 
}

static const Jim_HashTableType g_JimReferencesHashTableType = {
    JimReferencesHTHashFunction,        /* hash function */
    JimReferencesHTKeyDup,      /* key dup */
    NULL,                       /* val dup */
    JimReferencesHTKeyCompare,  /* key compare */
    JimReferencesHTKeyDestructor,       /* key destructor */
    JimReferencesHTValDestructor        /* val destructor */
};

/* -----------------------------------------------------------------------------
 * Reference object type and References API
 * ---------------------------------------------------------------------------*/

/* The string representation of references has two features in order
 * to make the GC faster. The first is that every reference starts
 * with a non common character '<', in order to make the string matching
 * faster. The second is that the reference string rep is 42 characters
 * in length, this means that it is not necessary to check any object with a string
 * repr < 42, and usually there aren't many of these objects. */

enum { JIM_REFERENCE_SPACE = (35+JIM_REFERENCE_TAGLEN) };

static int JimFormatReference(char *buf, Jim_Reference *refPtr, unsigned_long id)
{
    PRJ_TRACE;
    const char *fmt = "<reference.<%s>.%020lu>";

    sprintf(buf, fmt, refPtr->tag, id);
    return JIM_REFERENCE_SPACE;
}

STATIC void UpdateStringOfReferenceCB(Jim_ObjPtr objPtr);

static const Jim_ObjType g_referenceObjType = { // #JimType #JimRef
    "reference",
    NULL,
    NULL,
    UpdateStringOfReferenceCB,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& referenceType() { return g_referenceObjType; }

STATIC void UpdateStringOfReferenceCB(Jim_ObjPtr objPtr) // #JimRef
{
    PRJ_TRACE;
    char buf[JIM_REFERENCE_SPACE + 1];

    JimFormatReference(buf, objPtr->get_refValue_ref(), objPtr->get_refValue_id());
    JimSetStringBytes(objPtr, buf);
}

/* returns true if 'c' is a valid reference tag character.
 * i.e. inside the range [_a-zA-Z0-9] */
static int isrefchar(int c)
{
    return (c == '_' || isalnum(c));
}

STATIC int SetReferenceFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #MissInCoverage #JimRef
{
    PRJ_TRACE;
    unsigned_long value;
    int i, len;
    const char *str, *start, *end;
    char refId[21]; // #MagicNum
    Jim_Reference *refPtr;
    Jim_HashEntryPtr he;
    char *endptr;

    /* Get the string representation */
    str = Jim_GetString(objPtr, &len);
    /* Check if it looks like a reference */
    if (len < JIM_REFERENCE_SPACE)
        goto badformat;
    /* Trim spaces */
    start = str;
    end = str + len - 1;
    while (*start == ' ')
        start++;
    while (*end == ' ' && end > start)
        end--;
    if (end - start + 1 != JIM_REFERENCE_SPACE)
        goto badformat;
    /* <reference.<1234567>.%020> */
    if (memcmp(start, "<reference.<", 12) != 0)
        goto badformat;
    if (start[12 + JIM_REFERENCE_TAGLEN] != '>' || end[0] != '>') // #MagicNum
        goto badformat;
    /* The tag can't contain chars other than a-zA-Z0-9 + '_'. */
    for (i = 0; i < JIM_REFERENCE_TAGLEN; i++) {
        if (!isrefchar(start[12 + i])) // #MagicNum
            goto badformat;
    }
    /* Extract info from the reference. */
    memcpy(refId, start + 14 + JIM_REFERENCE_TAGLEN, 20); // #MagicNum
    refId[20] = '\0'; // #MagicNum
    /* Try to convert the ID into an unsigned_long */
    value = strtoul(refId, &endptr, 10);
    if (JimCheckConversion(refId, endptr) != JIM_OK)
        goto badformat;
    /* Check if the reference really exists! */
    he = Jim_FindHashEntry(&interp->references(), &value);
    if (he == NULL) {
        Jim_SetResultFormatted(interp, "invalid reference id \"%#s\"", objPtr);
        return JIM_ERR;
    }
    refPtr = (Jim_Reference*)Jim_GetHashEntryVal(he);
    /* Free the old internal repr and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_referenceObjType);
    objPtr->setRefValue(value, refPtr);
    //objPtr->internalRep.refValue_.id = value;
    //objPtr->internalRep.refValue_.refPtr = refPtr;
    return JIM_OK;

  badformat:
    Jim_SetResultFormatted(interp, "expected reference but got \"%#s\"", objPtr);
    return JIM_ERR;
}

/* Returns a new reference pointing to objPtr, having cmdNamePtr
 * as finalizer command (or NULL if there is no finalizer).
 * The returned reference object has refcount = 0. */
JIM_EXPORT Jim_ObjPtr Jim_NewReference(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr tagPtr, Jim_ObjPtr cmdNamePtr) // #JimRef
{
    PRJ_TRACE;
    Jim_Reference *refPtr;
    unsigned_long id;
    Jim_ObjPtr refObjPtr;
    const char *tag;
    int tagLen, i;

    /* Perform the Garbage Collection if needed. */
    Jim_CollectIfNeeded(interp);

    refPtr = new_Jim_Reference; // #AllocF 
    refPtr->objPtr = objPtr;
    Jim_IncrRefCount(objPtr);
    refPtr->finalizerCmdNamePtr = cmdNamePtr;
    if (cmdNamePtr)
        Jim_IncrRefCount(cmdNamePtr);
    id = interp->incrReferenceNextId();
    Jim_AddHashEntry(&interp->references(), &id, refPtr);
    refObjPtr = Jim_NewObj(interp);
    refObjPtr->setTypePtr(&g_referenceObjType);
    refObjPtr->bytes_setNULL();
    refObjPtr->setRefValue(id, refPtr);
    //refObjPtr->internalRep.refValue_.id = id;
    //refObjPtr->internalRep.refValue_.refPtr = refPtr;
    interp->incrReferenceNextId();
    /* Set the tag. Trimmed at JIM_REFERENCE_TAGLEN. Everything
     * that does not pass the 'isrefchar' test is replaced with '_' */
    tag = Jim_GetString(tagPtr, &tagLen);
    if (tagLen > JIM_REFERENCE_TAGLEN)
        tagLen = JIM_REFERENCE_TAGLEN;
    for (i = 0; i < JIM_REFERENCE_TAGLEN; i++) {
        if (i < tagLen && isrefchar(tag[i]))
            refPtr->tag[i] = tag[i];
        else
            refPtr->tag[i] = '_';
    }
    refPtr->tag[JIM_REFERENCE_TAGLEN] = '\0';
    PRJ_TRACE_GEN(::prj_trace::ACTION_REFERNCE_CREATE, __FUNCTION__, refPtr, NULL);

    return refObjPtr;
}

JIM_EXPORT Jim_Reference *Jim_GetReference(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimRef
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_referenceObjType && SetReferenceFromAny(interp, objPtr) == JIM_ERR)
        return NULL; // #MissInCoverage
    return objPtr->get_refValue_ref();
}

JIM_EXPORT Retval Jim_SetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr cmdNamePtr) // #MissInCoverage #JimRef
{
    PRJ_TRACE;
    Jim_Reference *refPtr;

    if ((refPtr = Jim_GetReference(interp, objPtr)) == NULL)
        return JIM_ERR;
    Jim_IncrRefCount(cmdNamePtr);
    if (refPtr->finalizerCmdNamePtr)
        Jim_DecrRefCount(interp, refPtr->finalizerCmdNamePtr);
    refPtr->finalizerCmdNamePtr = cmdNamePtr;
    return JIM_OK;
}

JIM_EXPORT Retval Jim_GetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjArray *cmdNamePtrPtr) // #MissInCoverage #JimRef
{
    PRJ_TRACE;
    Jim_Reference *refPtr;

    if ((refPtr = Jim_GetReference(interp, objPtr)) == NULL)
        return JIM_ERR;
    *cmdNamePtrPtr = refPtr->finalizerCmdNamePtr;
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * References Garbage Collection
 * ---------------------------------------------------------------------------*/

/* This the hash table type for the "MARK" phase of the GC */
static const Jim_HashTableType g_JimRefMarkHashTableType = {
    JimReferencesHTHashFunction,        /* hash function */
    JimReferencesHTKeyDup,      /* key dup */
    NULL,                       /* val dup */
    JimReferencesHTKeyCompare,  /* key compare */
    JimReferencesHTKeyDestructor,       /* key destructor */
    NULL                        /* val destructor */
};

/* Performs the garbage collection. */
JIM_EXPORT int Jim_Collect(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    PRJ_TRACE_GEN(::prj_trace::ACTION_COLLECT_PRE, __FUNCTION__, interp, NULL);

    int collected = 0;
    Jim_HashTable marks;
    Jim_HashTableIterator htiter;
    Jim_HashEntryPtr he;
    Jim_ObjPtr objPtr;

    /* Avoid recursive calls */
    if (interp->lastCollectId() == (unsigned_long)~0) { 
        /* Jim_Collect() already running. Return just now. */
        return 0; // #MissInCoverage
    }
    interp->lastCollectId_ = ~0; // #JI_access lastCollectId_

    /* Mark all the references found into the 'mark' hash table.
     * The references are searched in every live object that
     * is of a type that can contain references. */
    Jim_InitHashTable(&marks, &g_JimRefMarkHashTableType, NULL);
    marks.setTypeName("refMark");
    objPtr = interp->liveList();
    while (objPtr) {
        if (objPtr->typePtr() == NULL || objPtr->typePtr()->flags & JIM_TYPE_REFERENCES) {
            const char *str, *p;
            int len;

            /* If the object is of type reference, to get the
             * Id is simple... */
            if (objPtr->typePtr() == &g_referenceObjType) {
                Jim_AddHashEntry(&marks, objPtr->get_refValue_idPtr(), NULL); // #MissInCoverage
                if (g_JIM_DEBUG_GC) {
                    printf("MARK (reference): %d refcount: %d\n", // #stdoutput
                        (int)objPtr->get_refValue_id(), objPtr->refCount());
                }
                objPtr = objPtr->nextObjPtr();
                continue;
            }
            /* Get the string repr of the object we want
             * to scan for references. */
            p = str = Jim_GetString(objPtr, &len);
            /* Skip objects too little to contain references. */
            if (len < JIM_REFERENCE_SPACE) {
                objPtr = objPtr->nextObjPtr();
                continue;
            }
            /* Extract references from the object string repr. */
            while (1) {
                int i;
                unsigned_long id;

                if ((p = strstr(p, "<reference.<")) == NULL)
                    break;
                /* Check if it's a valid reference. */
                if (len - (p - str) < JIM_REFERENCE_SPACE)
                    break; // #MissInCoverage
                if (p[41] != '>' || p[19] != '>' || p[20] != '.') // #MagicNum
                    break;
                for (i = 21; i <= 40; i++) // #MagicNum
                    if (!isdigit(UCHAR(p[i])))
                        break; // #MissInCoverage
                /* Get the ID */
                id = strtoul(p + 21, NULL, 10);

                /* Ok, a reference for the given ID
                 * was found. Mark it. */
                Jim_AddHashEntry(&marks, &id, NULL);
                if (g_JIM_DEBUG_GC) {
                    printf("MARK: %d\n", (int)id); // #stdoutput
                }
                p += JIM_REFERENCE_SPACE;
            }
        }
        objPtr = objPtr->nextObjPtr();
    }

    /* Run the references hash table to destroy every reference that
     * is not referenced outside (not present in the mark HT). */
    JimInitHashTableIterator(&interp->references(), &htiter);
    while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
        const_unsigned_long *refId;
        Jim_Reference *refPtr;

        refId = (const_unsigned_long *)he->keyAsVoid();
        /* Check if in the mark phase we encountered
         * this reference. */
        if (Jim_FindHashEntry(&marks, refId) == NULL) {
            if (g_JIM_DEBUG_GC) {
                printf("COLLECTING %d\n", (int)*refId); // #stdoutput #MissInCoverage
            }
            collected++;
            /* Drop the reference, but call the
             * finalizer first if registered. */
            refPtr = (Jim_Reference*)Jim_GetHashEntryVal(he);
            if (refPtr->finalizerCmdNamePtr) {
                char* refstr = new_CharArray(JIM_REFERENCE_SPACE + 1); // #AllocF 
                Jim_Obj *objv[3], *oldResult;

                JimFormatReference(refstr, refPtr, *refId);

                objv[0] = refPtr->finalizerCmdNamePtr;
                objv[1] = Jim_NewStringObjNoAlloc(interp, refstr, JIM_REFERENCE_SPACE);
                objv[2] = refPtr->objPtr;

                /* Drop the reference itself */
                /* Avoid the finaliser being freed here */
                Jim_IncrRefCount(objv[0]);
                /* Don't remove the reference from the hash table just yet
                 * since that will free refPtr, and hence refPtr->objPtr
                 */

                /* Call the finalizer. Errors ignored. (should we use bgerror?) */
                oldResult = interp->result();
                Jim_IncrRefCount(oldResult);
                Jim_EvalObjVector(interp, 3, objv);
                Jim_SetResult(interp, oldResult);
                Jim_DecrRefCount(interp, oldResult);

                Jim_DecrRefCount(interp, objv[0]);
            }
            Jim_DeleteHashEntry(&interp->references(), refId);
        }
    }
    Jim_FreeHashTable(&marks);
    interp->setLastCollectedId(interp->referenceNextId());
    interp->lastCollectTime(time(NULL));
    PRJ_TRACE_GEN(::prj_trace::ACTION_COLLECT_POST, __FUNCTION__, interp, NULL);

    return collected;
}

enum {
    JIM_COLLECT_ID_PERIOD = 5000,
    JIM_COLLECT_TIME_PERIOD = 300
};

JIM_EXPORT void Jim_CollectIfNeeded(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    jim_wide elapsedId;
    int elapsedTime;

    elapsedId = interp->referenceNextId() - interp->lastCollectTime(); 
    elapsedTime = (int)(time(NULL) - interp->lastCollectTime());


    if (elapsedId > JIM_COLLECT_ID_PERIOD || elapsedTime > JIM_COLLECT_TIME_PERIOD) {
        Jim_Collect(interp); // #MissInCoverage
    }
}
#endif /* JIM_REFERENCES && !JIM_BOOTSTRAP */

JIM_EXPORT int Jim_IsBigEndian(void)
{
    PRJ_TRACE;
    union {
        unsigned_short s;
        unsigned_char c[2];
    } uval = {0x0102}; // #MagicNum

    return uval.c[0] == 1;
}

/* -----------------------------------------------------------------------------
 * Interpreter related functions
 * ---------------------------------------------------------------------------*/

JIM_EXPORT Jim_InterpPtr Jim_CreateInterp(void)
{
    PRJ_TRACE;
    Jim_InterpPtr  i = new_Jim_Interp; // #AllocF 

    //memset(i, 0, sizeof(*i));

    i->setMaxCallFrameDepth(JIM_MAX_CALLFRAME_DEPTH); // #MagicNum
    i->setMaxEvalDepth(JIM_MAX_EVAL_DEPTH); // #MagicNum
    i->lastCollectTime(time(NULL));

    /* Note that we can create objects only after the
     * interpreter liveList and freeList pointers are
     * initialized to NULL. */
    Jim_InitHashTable(i->commandsPtr(), &g_JimCommandsHashTableType, i); 
    i->commandsPtr()->setTypeName("commands"); 
#ifdef JIM_REFERENCES // #optionalCode
    Jim_InitHashTable(&i->references(), &g_JimReferencesHashTableType, i);
    i->references().setTypeName("references");
#endif
    Jim_InitHashTable(i->assocDataPtr(), &JimAssocDataHashTableType, i);
    i->assocDataPtr()->setTypeName("assocData");
    Jim_InitHashTable(i->getPackagesPtr(), &g_JimPackageHashTableType, NULL); 
    i->getPackagesPtr()->setTypeName("packages"); 
    i->emptyObj(Jim_NewEmptyStringObj(i));
    i->trueObj(Jim_NewIntObj(i, 1));
    i->falseObj(Jim_NewIntObj(i, 0));
    i->framePtr(i->topFramePtr_ = JimCreateCallFrame(i, NULL, i->emptyObj())); //#JI_access topFramePtr_
    i->setErrorFileNameObj(i->emptyObj());
    i->setResult(i->emptyObj()); 
    i->setStackTrace(Jim_NewListObj(i, NULL, 0));
    i->setUnknown(Jim_NewStringObj(i, "unknown", -1));
    i->setErrorProc(i->emptyObj());
    i->currentScriptObj(Jim_NewEmptyStringObj(i));
    i->nullScriptObj(Jim_NewEmptyStringObj(i));
    Jim_IncrRefCount(i->emptyObj());
    Jim_IncrRefCount(i->errorFileNameObj());
    Jim_IncrRefCount(i->result());
    Jim_IncrRefCount(i->stackTrace());
    Jim_IncrRefCount(i->unknown());
    Jim_IncrRefCount(i->currentScriptObj());
    Jim_IncrRefCount(i->nullScriptObj());
    Jim_IncrRefCount(i->errorProc());
    Jim_IncrRefCount(i->trueObj());
    Jim_IncrRefCount(i->falseObj());

    /* Initialize key variables every interpreter should contain */
    Jim_SetVariableStrWithStr(i, JIM_LIBPATH, TCL_LIBRARY);
    Jim_SetVariableStrWithStr(i, JIM_INTERACTIVE, "0");

    Jim_SetVariableStrWithStr(i, "tcl_platform(engine)", "Jim");
    Jim_SetVariableStrWithStr(i, "tcl_platform(os)", TCL_PLATFORM_OS);
    Jim_SetVariableStrWithStr(i, "tcl_platform(platform)", TCL_PLATFORM_PLATFORM);
    Jim_SetVariableStrWithStr(i, "tcl_platform(pathSeparator)", TCL_PLATFORM_PATH_SEPARATOR);
    Jim_SetVariableStrWithStr(i, "tcl_platform(byteOrder)", Jim_IsBigEndian() ? "bigEndian" : "littleEndian");
    Jim_SetVariableStrWithStr(i, "tcl_platform(threaded)", "0");
    Jim_SetVariableStr(i, "tcl_platform(pointerSize)", Jim_NewIntObj(i, sizeof(void *)));
    Jim_SetVariableStr(i, "tcl_platform(wordSize)", Jim_NewIntObj(i, sizeof(jim_wide)));

    return i;
}

JIM_EXPORT void Jim_FreeInterp(Jim_InterpPtr i)
{
    PRJ_TRACE;
    PRJ_TRACE_GEN(::prj_trace::ACTION_INTERP_DELETE, __FUNCTION__, i, NULL);
    Jim_CallFrame *cf, *cfx;

    Jim_Obj *objPtr, *nextObjPtr;

    /* Free the active call frames list - must be done before i->commands is destroyed */
    for (cf = i->framePtr(); cf; cf = cfx) {
        /* Note that we ignore any errors */
        JimInvokeDefer(i, JIM_OK);
        cfx = cf->parent();
        JimFreeCallFrame(i, cf, JIM_FCF_FULL);
    }

    Jim_DecrRefCount(i, i->emptyObj());
    Jim_DecrRefCount(i, i->trueObj());
    Jim_DecrRefCount(i, i->falseObj());
    Jim_DecrRefCount(i, i->result());
    Jim_DecrRefCount(i, i->stackTrace());
    Jim_DecrRefCount(i, i->errorProc());
    Jim_DecrRefCount(i, i->unknown());
    Jim_DecrRefCount(i, i->errorFileNameObj());
    Jim_DecrRefCount(i, i->currentScriptObj());
    Jim_DecrRefCount(i, i->nullScriptObj());
    Jim_FreeHashTable(i->commandsPtr()); 
#ifdef JIM_REFERENCES // #optionalCode
    Jim_FreeHashTable(i->referencesPtr()); 
#endif
    Jim_FreeHashTable(i->getPackagesPtr());
    i->prngStateFree(); // #FreeF
    //free_Jim_PrngState(i->prngState_); 
    Jim_FreeHashTable(i->assocDataPtr()); //  #FreeF

    /* Check that the live object list is empty, otherwise
     * there is a memory leak. */
    if (g_JIM_MAINTAINER_VAL) {
        if (i->liveList() != NULL) {
            objPtr = i->liveList(); // #MissInCoverage

            printf("\n-------------------------------------\n"); // #stdoutput
            printf("Objects still in the free list:\n"); // #stdoutput
            while (objPtr) {
                const char* type = objPtr->typePtr() ? objPtr->typePtr()->name : "string";
                Jim_String(objPtr);

                if (objPtr->bytes() && strlen(objPtr->bytes()) > 20) {
                    printf("%p (%d) %-10s: '%.20s...'\n", // #stdoutput
                           (void*) objPtr, objPtr->refCount(), type, objPtr->bytes());
                } else {
                    printf("%p (%d) %-10s: '%s'\n", // #stdoutput
                           (void*) objPtr, objPtr->refCount(), type, objPtr->bytes() ? objPtr->bytes() : "(null)");
                }
                if (objPtr->typePtr() == &g_sourceObjType) {
                    printf("FILE %s LINE %d\n", // #stdoutput
                           Jim_String(objPtr->get_sourceValue_fileName()),
                           objPtr->get_sourceValue_lineNum());
                }
                objPtr = objPtr->nextObjPtr();
            }
            printf("-------------------------------------\n\n"); // #stdoutput
            JimPanic((1, "Live list non empty freeing the interpreter! Leak?"));
        }
    }

    /* Free all the freed objects. */
    objPtr = i->freeList();
    while (objPtr) {
        nextObjPtr = objPtr->nextObjPtr();
        free_Jim_Obj(objPtr); // #FreeF 
        objPtr = nextObjPtr;
    }

    /* Free the free call frames list */
    for (cf = i->freeFramesList(); cf; cf = cfx) {
        cfx = cf->next;
        if (cf->vars().tableAllocated())
            Jim_FreeHashTable(&cf->vars());
        free_Jim_CallFrame(cf); // #FreeF 
    }

    /* Free the interpreter structure. */
    free_Jim_Interp(i); // #FreeF 
}

/* Returns the call frame relative to the level represented by
 * levelObjPtr. If levelObjPtr == NULL, the level is assumed to be '1'.
 *
 * This function accepts the 'level' argument in the form
 * of the commands [uplevel] and [upvar].
 *
 * Returns NULL on error.
 *
 * Note: for a function accepting a relative integer as level suitable
 * for implementation of [info level ?level?], see JimGetCallFrameByInteger()
 */
JIM_EXPORT Jim_CallFramePtr Jim_GetCallFrameByLevel(Jim_InterpPtr interp, Jim_ObjPtr levelObjPtr)
{
    PRJ_TRACE;
    long level;
    const char *str;
    Jim_CallFramePtr framePtr;

    if (levelObjPtr) {
        str = Jim_String(levelObjPtr);
        if (str[0] == '#') {
            char *endptr;

            level = jim_strtol(str + 1, &endptr);
            if (str[1] == '\0' || endptr[0] != '\0') {
                level = -1;
            }
        }
        else {
            if (Jim_GetLong(interp, levelObjPtr, &level) != JIM_OK || level < 0) {
                level = -1;
            }
            else {
                /* Convert from a relative to an absolute level */
                level = interp->framePtr()->level() - level;
            }
        }
    }
    else {
        str = "1";              /* Needed to format the error message. */
        level = interp->framePtr()->level() - 1;
    }

    if (level == 0) {
        return interp->topFramePtr();
    }
    if (level > 0) {
        /* Lookup */
        for (framePtr = interp->framePtr(); framePtr; framePtr = framePtr->parent()) {
            if (framePtr->level() == level) {
                return framePtr;
            }
        }
    }

    Jim_SetResultFormatted(interp, "bad level \"%s\"", str);
    return NULL;
}

/* Similar to Jim_GetCallFrameByLevel() but the level is specified
 * as a relative integer like in the [info level ?level?] command.
 **/
static Jim_CallFramePtr JimGetCallFrameByInteger(Jim_InterpPtr interp, Jim_ObjPtr levelObjPtr)
{
    PRJ_TRACE;
    long level;
    Jim_CallFramePtr framePtr;

    if (Jim_GetLong(interp, levelObjPtr, &level) == JIM_OK) {
        if (level <= 0) {
            /* Convert from a relative to an absolute level */
            level = interp->framePtr()->level() + level;
        }

        if (level == 0) {
            return interp->topFramePtr();
        }

        /* Lookup */
        for (framePtr = interp->framePtr(); framePtr; framePtr = framePtr->parent()) {
            if (framePtr->level() == level) {
                return framePtr;
            }
        }
    }

    Jim_SetResultFormatted(interp, "bad level \"%#s\"", levelObjPtr);
    return NULL;
}

static void JimResetStackTrace(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    Jim_DecrRefCount(interp, interp->stackTrace());
    interp->setStackTrace(Jim_NewListObj(interp, NULL, 0));
    Jim_IncrRefCount(interp->stackTrace());
}

static void JimSetStackTrace(Jim_InterpPtr interp, Jim_ObjPtr stackTraceObj)
{
    PRJ_TRACE;
    int len;

    /* Increment reference first in case these are the same object */
    Jim_IncrRefCount(stackTraceObj);
    Jim_DecrRefCount(interp, interp->stackTrace());
    interp->setStackTrace(stackTraceObj);
    interp->setErrorFlag(1);

    /* This is a bit ugly.
     * If the filename of the last entry of the stack trace is empty,
     * the next stack level should be added.
     */
    len = Jim_ListLength(interp, interp->stackTrace());
    if (len >= 3) {
        if (Jim_Length(Jim_ListGetIndex(interp, interp->stackTrace(), len - 2)) == 0) {
            interp->setAddStackTrace(1); // #MissInCoverage
        }
    }
}

static void JimAppendStackTrace(Jim_InterpPtr interp, const char *procname,
    Jim_ObjPtr fileNameObj, int linenr)
{
    PRJ_TRACE;
    if (strcmp(procname, "unknown") == 0) {
        procname = "";
    }
    if (!*procname && !Jim_Length(fileNameObj)) {
        /* No useful info here */
        return;
    }

    if (Jim_IsShared(interp->stackTrace())) {
        Jim_DecrRefCount(interp, interp->stackTrace()); // #MissInCoverage
        interp->setStackTrace(Jim_DuplicateObj(interp, interp->stackTrace()));
        Jim_IncrRefCount(interp->stackTrace());
    }

    /* If we have no procname but the previous element did, merge with that frame */
    if (!*procname && Jim_Length(fileNameObj)) {
        /* Just a filename. Check the previous entry */
        int len = Jim_ListLength(interp, interp->stackTrace());

        if (len >= 3) {
            Jim_ObjPtr objPtr = Jim_ListGetIndex(interp, interp->stackTrace(), len - 3);
            if (Jim_Length(objPtr)) {
                /* Yes, the previous level had procname */
                objPtr = Jim_ListGetIndex(interp, interp->stackTrace(), len - 2);
                if (Jim_Length(objPtr) == 0) {
                    /* But no filename, so merge the new info with that frame */
                    ListSetIndex(interp, interp->stackTrace(), len - 2, fileNameObj, 0);
                    ListSetIndex(interp, interp->stackTrace(), len - 1, Jim_NewIntObj(interp, linenr), 0);
                    return;
                }
            }
        }
    }

    Jim_ListAppendElement(interp, interp->stackTrace(), Jim_NewStringObj(interp, procname, -1));
    Jim_ListAppendElement(interp, interp->stackTrace(), fileNameObj);
    Jim_ListAppendElement(interp, interp->stackTrace(), Jim_NewIntObj(interp, linenr));
}

JIM_EXPORT Retval Jim_SetAssocData(Jim_InterpPtr interp, const char *key, Jim_InterpDeleteProc *delProc,
    void *data)
{
    PRJ_TRACE;
    AssocDataValuePtr assocEntryPtr = new_AssocDataValue; // #AllocF 

    assocEntryPtr->delProc = delProc;
    assocEntryPtr->data = data;
    return Jim_AddHashEntry(interp->assocDataPtr(), key, assocEntryPtr);
}

JIM_EXPORT void *Jim_GetAssocData(Jim_InterpPtr interp, const char *key)
{
    PRJ_TRACE;
    Jim_HashEntryPtr entryPtr = Jim_FindHashEntry(interp->assocDataPtr(), key);

    if (entryPtr != NULL) {
        AssocDataValuePtr assocEntryPtr = (AssocDataValuePtr)Jim_GetHashEntryVal(entryPtr);
        return assocEntryPtr->data;
    }
    return NULL; // #MissInCoverage
}

JIM_EXPORT Retval Jim_DeleteAssocData(Jim_InterpPtr interp, const char *key) // #MissInCoverage
{
    PRJ_TRACE;
    return Jim_DeleteHashEntry(interp->assocDataPtr(), key);
}

int Jim_GetExitCode(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    return interp->exitCode();
}

/* -----------------------------------------------------------------------------
 * Integer object
 * ---------------------------------------------------------------------------*/
static void UpdateStringOfIntCB(Jim_ObjPtr objPtr);
static Retval SetIntFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);

static const Jim_ObjType g_intObjType = { // #JimType #JimInt
    "int",
    NULL,
    NULL,
    UpdateStringOfIntCB,
    JIM_TYPE_NONE,
};
const Jim_ObjType& intType() { return g_intObjType; }

/* A coerced double is closer to an int than a double.
 * It is an int value temporarily masquerading as a double value.
 * i.e. it has the same string value as an int and Jim_GetWide()
 * succeeds, but also Jim_GetDouble() returns the value directly.
 */
static const Jim_ObjType g_coercedDoubleObjType = { // #JimType #JimDouble
    "coerced-double",
    NULL,
    NULL,
    UpdateStringOfIntCB,
    JIM_TYPE_NONE,
};
const Jim_ObjType& coercedDoubleType() { return g_coercedDoubleObjType; }


static void UpdateStringOfIntCB(Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    char buf[JIM_INTEGER_SPACE + 1];
    jim_wide wideValue = JimWideValue(objPtr);
    int pos = 0;

    if (wideValue == 0) {
        buf[pos++] = '0';
    }
    else {
        char tmp[JIM_INTEGER_SPACE];
        int num = 0;
        int i;

        if (wideValue < 0) {
            buf[pos++] = '-';
            i = wideValue % 10;
            /* C89 is implementation defined as to whether (-106 % 10) is -6 or 4,
             * whereas C99 is always -6
             * coverity[dead_error_line]
             */
            tmp[num++] = (i > 0) ? (10 - i) : -i;
            wideValue /= -10;
        }

        while (wideValue) {
            tmp[num++] = wideValue % 10;
            wideValue /= 10;
        }

        for (i = 0; i < num; i++) {
            buf[pos++] = '0' + tmp[num - i - 1];
        }
    }
    buf[pos] = 0;

    JimSetStringBytes(objPtr, buf);
}

static Retval SetIntFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags) // #JimInt
{
    PRJ_TRACE;
    jim_wide wideValue;
    const char *str;

    if (objPtr->typePtr() == &g_coercedDoubleObjType) {
        /* Simple switch */
        objPtr->setTypePtr(&g_intObjType);
        return JIM_OK;
    }

    /* Get the string representation */
    str = Jim_String(objPtr);
    /* Try to convert into a jim_wide */
    if (Jim_StringToWide(str, &wideValue, 0) != JIM_OK) {
        if (flags & JIM_ERRMSG) {
            Jim_SetResultFormatted(interp, "expected integer but got \"%#s\"", objPtr);
        }
        return JIM_ERR;
    }
    if ((wideValue == JIM_WIDE_MIN || wideValue == JIM_WIDE_MAX) && errno == ERANGE) {
        Jim_SetResultString(interp, "Integer value too big to be represented", -1); // #MissInCoverage
        return JIM_ERR;
    }
    /* Free the old internal repr and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_intObjType);
    objPtr->setWideValue(wideValue);
    return JIM_OK;
}

static int JimIsWide(Jim_ObjPtr objPtr) // #JimInt
{
    PRJ_TRACE;
    return objPtr->typePtr() == &g_intObjType;
}

JIM_EXPORT Retval Jim_GetWide(Jim_InterpPtr interp, Jim_ObjPtr objPtr, jim_wide *widePtr) // #JimInt
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_intObjType && SetIntFromAny(interp, objPtr, JIM_ERRMSG) == JIM_ERR)
        return JIM_ERR;
    *widePtr = JimWideValue(objPtr);
    return JIM_OK;
}

/* Get a wide but does not set an error if the format is bad. */
static Retval JimGetWideNoErr(Jim_InterpPtr interp, Jim_ObjPtr objPtr, jim_wide * widePtr) // #JimInt
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_intObjType && SetIntFromAny(interp, objPtr, JIM_NONE) == JIM_ERR)
        return JIM_ERR;
    *widePtr = JimWideValue(objPtr);
    return JIM_OK;
}

JIM_EXPORT Retval Jim_GetLong(Jim_InterpPtr interp, Jim_ObjPtr objPtr, long *longPtr) // #JimInt
{
    PRJ_TRACE;
    jim_wide wideValue;
    int retval;

    retval = Jim_GetWide(interp, objPtr, &wideValue);
    if (retval == JIM_OK) {
        *longPtr = (long)wideValue;
        return JIM_OK;
    }
    return JIM_ERR;
}

Jim_ObjPtr Jim_NewIntObj(Jim_InterpPtr interp, jim_wide wideValue) // #JimInt
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    objPtr = Jim_NewObj(interp);
    objPtr->setTypePtr(&g_intObjType);
    objPtr->bytes_setNULL();
    objPtr->setWideValue(wideValue);
    return objPtr;
}

/* -----------------------------------------------------------------------------
 * Double object
 * ---------------------------------------------------------------------------*/
enum { JIM_DOUBLE_SPACE = 30 }; // #MagicNum

static void UpdateStringOfDouble(Jim_ObjPtr objPtr);
static Retval SetDoubleFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

static const Jim_ObjType g_doubleObjType = { // #JimType #JimDouble
    "double",
    NULL,
    NULL,
    UpdateStringOfDouble,
    JIM_TYPE_NONE,
};
const Jim_ObjType& doubleType() { return g_doubleObjType; }

#ifndef HAVE_ISNAN // #optionalCode #WinOff
#undef isnan
#define isnan(X) ((X) != (X))
#endif
#ifndef HAVE_ISINF // #optionalCode #WinOff
#undef isinf
#define isinf(X) (1.0 / (X) == 0.0)
#endif

static void UpdateStringOfDouble(Jim_ObjPtr objPtr) // #JimDouble
{
    PRJ_TRACE;
    double value = objPtr->getDoubleValue();

    if (isnan(value)) {
        JimSetStringBytes(objPtr, "NaN");
        return;
    }
    if (isinf(value)) {
        if (value < 0) {
            JimSetStringBytes(objPtr, "-Inf");
        }
        else {
            JimSetStringBytes(objPtr, "Inf");
        }
        return;
    }
    {
        char buf[JIM_DOUBLE_SPACE + 1];
        int i;
        int len = sprintf(buf, "%.12g", value);

        /* Add a final ".0" if necessary */
        for (i = 0; i < len; i++) {
            if (buf[i] == '.' || buf[i] == 'e') {

            if (g_JIM_SPRINTF_DOUBLE_NEEDS_FIX_VAL) {
                /* If 'buf' ends in e-0nn or e+0nn, remove
                 * the 0 after the + or - and reduce the length by 1
                 */
                char *e = strchr(buf, 'e'); // #MissInCoverage
                if (e && (e[1] == '-' || e[1] == '+') && e[2] == '0') {
                    /* Move it up */
                    e += 2;
                    memmove(e, e + 1, len - (e - buf));
                }
            }

                break;
            }
        }
        if (buf[i] == '\0') {
            buf[i++] = '.';
            buf[i++] = '0';
            buf[i] = '\0';
        }
        JimSetStringBytes(objPtr, buf);
    }
}

static Retval SetDoubleFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimDouble
{
    PRJ_TRACE;
    double doubleValue;
    jim_wide wideValue;
    const char *str;

#ifdef HAVE_LONG_LONG // #optionalCode
    /* Assume a 53 bit mantissa #MagicNum */
#define MIN_INT_IN_DOUBLE -(1LL << 53)
#define MAX_INT_IN_DOUBLE -(MIN_INT_IN_DOUBLE + 1)

    if (objPtr->typePtr() == &g_intObjType
        && JimWideValue(objPtr) >= MIN_INT_IN_DOUBLE
        && JimWideValue(objPtr) <= MAX_INT_IN_DOUBLE) {

        /* Direct conversion to coerced double */
        objPtr->setTypePtr(&g_coercedDoubleObjType);
        return JIM_OK;
    }
#endif
    /* Preserve the string representation.
     * Needed so we can convert back to int without loss
     */
    str = Jim_String(objPtr);

    if (Jim_StringToWide(str, &wideValue, 10) == JIM_OK) {
        /* Managed to convert to an int, so we can use this as a cooerced double */
        Jim_FreeIntRep(interp, objPtr);
        objPtr->setTypePtr(&g_coercedDoubleObjType);
        objPtr->setWideValue(wideValue);
        return JIM_OK;
    }
    else {
        /* Try to convert into a double */
        if (Jim_StringToDouble(str, &doubleValue) != JIM_OK) {
            Jim_SetResultFormatted(interp, "expected floating-point number but got \"%#s\"", objPtr);
            return JIM_ERR;
        }
        /* Free the old internal repr and set the new one. */
        Jim_FreeIntRep(interp, objPtr);
    }
    objPtr->setTypePtr(&g_doubleObjType);
    objPtr->setDoubleValue( doubleValue);
    return JIM_OK;
}



JIM_EXPORT Retval Jim_GetDouble(Jim_InterpPtr interp, Jim_ObjPtr objPtr, double *doublePtr) // #JimDouble
{
    PRJ_TRACE;
    if (objPtr->typePtr() == &g_coercedDoubleObjType) {
        *doublePtr = (double)JimWideValue(objPtr);
        return JIM_OK;
    }
    if (objPtr->typePtr() != &g_doubleObjType && SetDoubleFromAny(interp, objPtr) == JIM_ERR)
        return JIM_ERR;

    if (objPtr->typePtr() == &g_coercedDoubleObjType) {
        *doublePtr = (double)JimWideValue(objPtr);
    }
    else {
        *doublePtr = objPtr->getDoubleValue();
    }
    return JIM_OK;
}

JIM_EXPORT Jim_ObjPtr Jim_NewDoubleObj(Jim_InterpPtr interp, double doubleValue) // #JimDouble
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    objPtr = Jim_NewObj(interp);
    objPtr->setTypePtr(&g_doubleObjType);
    objPtr->bytes_setNULL();
    objPtr->setDoubleValue( doubleValue);
    return objPtr;
}

/* -----------------------------------------------------------------------------
 * Boolean conversion
 * ---------------------------------------------------------------------------*/
static Retval SetBooleanFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);

JIM_EXPORT Retval Jim_GetBoolean(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int *booleanPtr) // #JimBool
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_intObjType && SetBooleanFromAny(interp, objPtr, JIM_ERRMSG) == JIM_ERR)
        return JIM_ERR;
    *booleanPtr = (int) JimWideValue(objPtr);
    return JIM_OK;
}

static Retval SetBooleanFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags) // #JimBool
{
    PRJ_TRACE;
    static const char * const falses[] = {
        "0", "false", "no", "off", NULL
    };
    static const char * const trues[] = {
        "1", "true", "yes", "on", NULL
    };

    int boolean;

    int index;
    if (Jim_GetEnum(interp, objPtr, falses, &index, NULL, 0) == JIM_OK) {
        boolean = 0;
    } else if (Jim_GetEnum(interp, objPtr, trues, &index, NULL, 0) == JIM_OK) {
        boolean = 1;
    } else {
        if (flags & JIM_ERRMSG) {
            Jim_SetResultFormatted(interp, "expected boolean but got \"%#s\"", objPtr);
        }
        return JIM_ERR;
    }

    /* Free the old internal repr and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_intObjType);
    objPtr->setWideValue(boolean);
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * List object
 * ---------------------------------------------------------------------------*/
STATIC void ListInsertElements(Jim_ObjPtr listPtr, int idx, int elemc, Jim_ObjConstArray elemVec);
static void ListAppendElement(Jim_ObjPtr listPtr, Jim_ObjPtr objPtr);
STATIC void FreeListInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
STATIC void DupListInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
STATIC void UpdateStringOfListCB(Jim_ObjPtr objPtr);
STATIC Retval SetListFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

/* Note that while the elements of the list may contain references,
 * the list object itself can't. This basically means that the
 * list object string representation as a whole can't contain references
 * that are not presents in the single elements. */
static const Jim_ObjType g_listObjType = { // #JimType #JimList
    "list",
    FreeListInternalRepCB,
    DupListInternalRepCB,
    UpdateStringOfListCB,
    JIM_TYPE_NONE,
};
const Jim_ObjType& listType() { return g_listObjType; }

STATIC void FreeListInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimList
{
    PRJ_TRACE;
    int i;

    for (i = 0; i < objPtr->get_listValue_len(); i++) {
        Jim_DecrRefCount(interp, objPtr->get_listValue_objArray(i));
    }
    objPtr->free_listValue_ele(); // #FreeF
    //free_Jim_ObjArray(objPtr->internalRep.listValue_.ele_); 
}

STATIC void DupListInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr)  // #JimList
{
    PRJ_TRACE;
    int i;

    JIM_NOTUSED(interp);
    
    dupPtr->setListValue(
        srcPtr->get_listValue_len(),
        srcPtr->get_listValue_maxLen(),
        (Jim_ObjArray*) new_Jim_ObjArray(srcPtr->get_listValue_maxLen())); // #AllocF
    //dupPtr->internalRep.listValue_.len = srcPtr->get_listValue_len();
    //dupPtr->internalRep.listValue_.maxLen = srcPtr->get_listValue_maxLen();
    //dupPtr->internalRep.listValue_.ele = (Jim_ObjArray*) new_Jim_ObjArray(srcPtr->get_listValue_maxLen());  
    dupPtr->copy_listValue_ele(srcPtr);
    //memcpy(dupPtr->internalRep.listValue_.ele_, srcPtr->internalRep.listValue_.ele_, 
    //    sizeof(Jim_ObjPtr ) * srcPtr->get_listValue_len());
    for (i = 0; i < dupPtr->get_listValue_len(); i++) {
        Jim_IncrRefCount(dupPtr->get_listValue_objArray(i));
    }
    dupPtr->setTypePtr(&g_listObjType);
}

/* The following function checks if a given string can be encoded
 * into a list element without any kind of quoting, surrounded by braces,
 * or using escapes to quote. */
enum JIM_ELESTR {
    JIM_ELESTR_SIMPLE = 0,
    JIM_ELESTR_BRACE = 1,
    JIM_ELESTR_QUOTE = 2
};

static unsigned_char ListElementQuotingType(const char *s, int len) // #JimList
{
    PRJ_TRACE;
    int i, level, blevel, trySimple = 1;

    /* Try with the SIMPLE case */
    if (len == 0)
        return JIM_ELESTR_BRACE;
    if (s[0] == '"' || s[0] == '{') {
        trySimple = 0;
        goto testbrace;
    }
    for (i = 0; i < len; i++) {
        switch (s[i]) {
            case ' ':
            case '$':
            case '"':
            case '[':
            case ']':
            case ';':
            case '\\':
            case '\r':
            case '\n':
            case '\t':
            case '\f':
            case '\v':
                trySimple = 0;
                /* fall through */
            case '{':
            case '}':
                goto testbrace;
        }
    }
    return JIM_ELESTR_SIMPLE;

  testbrace:
    /* Test if it's possible to do with braces */
    if (s[len - 1] == '\\')
        return JIM_ELESTR_QUOTE;
    level = 0;
    blevel = 0;
    for (i = 0; i < len; i++) {
        switch (s[i]) {
            case '{':
                level++;
                break;
            case '}':
                level--;
                if (level < 0)
                    return JIM_ELESTR_QUOTE;
                break;
            case '[':
                blevel++;
                break;
            case ']':
                blevel--;
                break;
            case '\\':
                if (s[i + 1] == '\n')
                    return JIM_ELESTR_QUOTE;
                else if (s[i + 1] != '\0')
                    i++;
                break;
        }
    }
    if (blevel < 0) {
        return JIM_ELESTR_QUOTE;
    }

    if (level == 0) {
        if (!trySimple)
            return JIM_ELESTR_BRACE;
        for (i = 0; i < len; i++) {
            switch (s[i]) {
                case ' ':
                case '$':
                case '"':
                case '[':
                case ']':
                case ';':
                case '\\':
                case '\r':
                case '\n':
                case '\t':
                case '\f':
                case '\v':
                    return JIM_ELESTR_BRACE;
                    break;
            }
        }
        return JIM_ELESTR_SIMPLE;
    }
    return JIM_ELESTR_QUOTE;
}

/* Backslashes-escapes the null-terminated string 's' into the buffer at 'q'
 * The buffer must be at least strlen(s) * 2 + 1 bytes long for the worst-case
 * scenario.
 * Returns the length of the result.
 */
static int BackslashQuoteString(const char *s, int len, char *q) 
{
    PRJ_TRACE;
    char *p = q;

    while (len--) {
        switch (*s) {
            case ' ':
            case '$':
            case '"':
            case '[':
            case ']':
            case '{':
            case '}':
            case ';':
            case '\\':
                *p++ = '\\';
                *p++ = *s++;
                break;
            case '\n':
                *p++ = '\\';
                *p++ = 'n';
                s++;
                break;
            case '\r':
                *p++ = '\\';
                *p++ = 'r';
                s++;
                break;
            case '\t':
                *p++ = '\\';
                *p++ = 't';
                s++;
                break;
            case '\f':
                *p++ = '\\';
                *p++ = 'f';
                s++;
                break;
            case '\v':
                *p++ = '\\';
                *p++ = 'v';
                s++;
                break;
            default:
                *p++ = *s++;
                break;
        }
    }
    *p = '\0';

    return (int)(p - q);
}

STATIC void JimMakeListStringRep(Jim_ObjPtr objPtr, Jim_ObjArray *objv, int objc) // #JimList
{
    PRJ_TRACE;
    enum {  STATIC_QUOTING_LEN = 32 }; // #MagicNum
    int i, bufLen, realLength;
    const char *strRep;
    char *p;
    unsigned_char *quotingType, staticQuoting[STATIC_QUOTING_LEN];

    /* Estimate the space needed. */
    if (objc > STATIC_QUOTING_LEN) {
        quotingType = Jim_TAlloc<unsigned_char>(objc,"unsigned_char"); // #AllocF 
    }
    else {
        quotingType = staticQuoting;
    }
    bufLen = 0;
    for (i = 0; i < objc; i++) {
        int len;

        strRep = Jim_GetString(objv[i], &len);
        quotingType[i] = ListElementQuotingType(strRep, len);
        switch (quotingType[i]) {
            case JIM_ELESTR_SIMPLE:
                if (i != 0 || strRep[0] != '#') {
                    bufLen += len;
                    break;
                }
                /* Special case '#' on first element needs braces */
                quotingType[i] = JIM_ELESTR_BRACE;
                /* fall through */
            case JIM_ELESTR_BRACE:
                bufLen += len + 2;
                break;
            case JIM_ELESTR_QUOTE:
                bufLen += len * 2;
                break;
        }
        bufLen++;               /* elements separator. */
    }
    bufLen++;

    /* Generate the string rep. */
    p = objPtr->setBytes( new_CharArray(bufLen + 1)); // #AllocF 
    realLength = 0;
    for (i = 0; i < objc; i++) {
        int len, qlen;

        strRep = Jim_GetString(objv[i], &len);

        switch (quotingType[i]) {
            case JIM_ELESTR_SIMPLE:
                memcpy(p, strRep, len);
                p += len;
                realLength += len;
                break;
            case JIM_ELESTR_BRACE:
                *p++ = '{';
                memcpy(p, strRep, len);
                p += len;
                *p++ = '}';
                realLength += len + 2;
                break;
            case JIM_ELESTR_QUOTE:
                if (i == 0 && strRep[0] == '#') {
                    *p++ = '\\';
                    realLength++;
                }
                qlen = BackslashQuoteString(strRep, len, p);
                p += qlen;
                realLength += qlen;
                break;
        }
        /* Add a separating space */
        if (i + 1 != objc) {
            *p++ = ' ';
            realLength++;
        }
    }
    *p = '\0';                  /* nul term. */
    objPtr->setLength(realLength);

    if (quotingType != staticQuoting) {
        Jim_TFree<unsigned_char>(quotingType,"unsigned_char"); // #FreeF 
    }
}

STATIC void UpdateStringOfListCB(Jim_ObjPtr objPtr) // #JimList
{
    PRJ_TRACE;
    JimMakeListStringRep(objPtr, objPtr->get_listValue_ele(), objPtr->get_listValue_len()); 
}

STATIC Retval SetListFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimList
{
    PRJ_TRACE;
    JimParserCtx parser;
    const char *str;
    int strLen;
    Jim_ObjPtr fileNameObj;
    int linenr;

    if (objPtr->typePtr() == &g_listObjType) {
        return JIM_OK;
    }

    /* Optimize dict -> list for object with no string rep. Note that this may only save a little time, but
     * it also preserves any source location of the dict elements
     * which can be very useful
     */
    if (Jim_IsDict(objPtr) && objPtr->bytes() == NULL) {
        Jim_ObjArray *listObjPtrPtr;
        int len;
        int i;

        listObjPtrPtr = JimDictPairs(objPtr, &len);
        for (i = 0; i < len; i++) {
            Jim_IncrRefCount(listObjPtrPtr[i]);
        }

        /* Now just switch the internal rep */
        Jim_FreeIntRep(interp, objPtr);
        objPtr->setTypePtr(&g_listObjType);
        objPtr->setListValue(len, len, listObjPtrPtr);
        //objPtr->internalRep.listValue_.len = len;
        //objPtr->internalRep.listValue_.maxLen = len;
        //objPtr->internalRep.listValue_.ele = listObjPtrPtr;

        return JIM_OK;
    }

    /* Try to preserve information about filename / line number */
    if (objPtr->typePtr() == &g_sourceObjType) {
        fileNameObj = objPtr->get_sourceValue_fileName();
        linenr = objPtr->get_sourceValue_lineNum();
    }
    else {
        fileNameObj = interp->emptyObj();
        linenr = 1;
    }
    Jim_IncrRefCount(fileNameObj);

    /* Get the string representation */
    str = Jim_GetString(objPtr, &strLen);

    /* Free the old internal repr just now and initialize the
     * new one just now. The string->list conversion can't fail. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_listObjType);
    objPtr->setListValue(0, 0, NULL);
    //objPtr->internalRep.listValue_.len = 0;
    //objPtr->internalRep.listValue_.maxLen = 0;
    //objPtr->internalRep.listValue_.ele = NULL;

    /* Convert into a list */
    if (strLen) {
        JimParserInit(&parser, str, strLen, linenr);
        while (!parser.eof) {
            Jim_ObjPtr elementPtr;

            JimParseList(&parser);
            if (parser.tt != JIM_TT_STR && parser.tt != JIM_TT_ESC)
                continue;
            elementPtr = JimParserGetTokenObj(interp, &parser);
            JimSetSourceInfo(interp, elementPtr, fileNameObj, parser.tline);
            ListAppendElement(objPtr, elementPtr);
        }
    }
    Jim_DecrRefCount(interp, fileNameObj);
    return JIM_OK;
}

JIM_EXPORT Jim_ObjPtr Jim_NewListObj(Jim_InterpPtr interp, Jim_ObjConstArray elements, int len) // #JimList
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    objPtr = Jim_NewObj(interp);
    objPtr->setTypePtr(&g_listObjType);
    objPtr->bytes_setNULL();
    objPtr->setListValue(0, 0, NULL);
    //objPtr->internalRep.listValue_.ele = NULL;
    //objPtr->internalRep.listValue_.len = 0;
    //objPtr->internalRep.listValue_.maxLen = 0;

    if (len) {
        ListInsertElements(objPtr, 0, len, elements);
    }

    return objPtr;
}

/* Return a vector of Jim_Obj with the elements of a Jim list, and the
 * length of the vector. Note that the user of this function should make
 * sure that the list object can't shimmer while the vector returned
 * is in use, this vector is the one stored inside the internal representation
 * of the list object. This function is not exported, extensions should
 * always access to the List object elements using Jim_ListIndex(). */
STATIC void JimListGetElements(Jim_InterpPtr interp, Jim_ObjPtr listObj, int *listLen, // #JimList
    Jim_ObjArray* *listVec)
{
    PRJ_TRACE;
    *listLen = Jim_ListLength(interp, listObj);
    *listVec = listObj->get_listValue_ele();
}

/* Sorting uses ints, but commands may return wide */
static int JimSign(jim_wide w)
{
    if (w == 0) {
        return 0;
    }
    else if (w < 0) {
        return -1;
    }
    return 1;
}

/* ListSortElements type values */
enum JIM_LSORT_TYPE {
    JIM_LSORT_ASCII,
    JIM_LSORT_NOCASE,
    JIM_LSORT_INTEGER,
    JIM_LSORT_REAL,
    JIM_LSORT_COMMAND
};

struct lsort_info {
    jmp_buf jmpbuf;
    Jim_ObjPtr command = NULL;
    Jim_InterpPtr interp = NULL;
    enum JIM_LSORT_TYPE type;
    int order = 0;
    int index = 0;
    int indexed = 0;
    int unique = 0;
    int (*subfn)(Jim_ObjArray* , Jim_ObjArray* ) = NULL;
};

static lsort_info *g_sort_info;

static Retval ListSortIndexHelper(Jim_ObjArray* lhsObj, Jim_ObjArray* rhsObj) // #JimList
{
    PRJ_TRACE;
    Jim_Obj *lObj, *rObj;

    if (Jim_ListIndex(g_sort_info->interp, *lhsObj, g_sort_info->index, &lObj, JIM_ERRMSG) != JIM_OK ||
        Jim_ListIndex(g_sort_info->interp, *rhsObj, g_sort_info->index, &rObj, JIM_ERRMSG) != JIM_OK) {
        longjmp(g_sort_info->jmpbuf, JIM_ERR);
    }
    return g_sort_info->subfn(&lObj, &rObj);
}

/* Sort the internal rep of a list. */
static int ListSortString(Jim_ObjArray *lhsObj, Jim_ObjArray *rhsObj) // #JimList
{
    PRJ_TRACE;
    return Jim_StringCompareObj(g_sort_info->interp, *lhsObj, *rhsObj, 0) * g_sort_info->order;
}

static int ListSortStringNoCase(Jim_ObjArray *lhsObj, Jim_ObjArray *rhsObj) // #JimList
{
    PRJ_TRACE;
    return Jim_StringCompareObj(g_sort_info->interp, *lhsObj, *rhsObj, 1) * g_sort_info->order;
}

static Retval ListSortInteger(Jim_ObjArray *lhsObj, Jim_ObjArray *rhsObj) // #JimList
{
    PRJ_TRACE;
    jim_wide lhs = 0, rhs = 0;

    if (Jim_GetWide(g_sort_info->interp, *lhsObj, &lhs) != JIM_OK ||
        Jim_GetWide(g_sort_info->interp, *rhsObj, &rhs) != JIM_OK) {
        longjmp(g_sort_info->jmpbuf, JIM_ERR);
    }

    return JimSign(lhs - rhs) * g_sort_info->order;
}

static Retval ListSortReal(Jim_ObjArray *lhsObj, Jim_ObjArray *rhsObj) // #JimList
{
    PRJ_TRACE;
    double lhs = 0, rhs = 0;

    if (Jim_GetDouble(g_sort_info->interp, *lhsObj, &lhs) != JIM_OK ||
        Jim_GetDouble(g_sort_info->interp, *rhsObj, &rhs) != JIM_OK) {
        longjmp(g_sort_info->jmpbuf, JIM_ERR);
    }
    if (lhs == rhs) {
        return 0;
    }
    if (lhs > rhs) {
        return g_sort_info->order;
    }
    return -g_sort_info->order;
}

static Retval ListSortCommand(Jim_ObjArray *lhsObj, Jim_ObjArray *rhsObj) // #JimList #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr compare_script;
    Retval rc;

    jim_wide ret = 0;

    /* This must be a valid list */
    compare_script = Jim_DuplicateObj(g_sort_info->interp, g_sort_info->command);
    Jim_ListAppendElement(g_sort_info->interp, compare_script, *lhsObj);
    Jim_ListAppendElement(g_sort_info->interp, compare_script, *rhsObj);

    rc = Jim_EvalObj(g_sort_info->interp, compare_script);

    if (rc != JIM_OK || Jim_GetWide(g_sort_info->interp, Jim_GetResult(g_sort_info->interp), &ret) != JIM_OK) {
        longjmp(g_sort_info->jmpbuf, rc);
    }

    return JimSign(ret) * g_sort_info->order;
}

/* Remove duplicate elements from the (sorted) list in-place, according to the
 * comparison function, comp.
 *
 * Note that the last unique value is kept, not the first
 */
STATIC void ListRemoveDuplicates(Jim_ObjPtr listObjPtr, int (*comp)(Jim_ObjArray *lhs, Jim_ObjArray *rhs)) // #JimList
{
    PRJ_TRACE;
    int src;
    int dst = 0;
    Jim_ObjArray *ele = listObjPtr->get_listValue_ele();  

    for (src = 1; src < listObjPtr->get_listValue_len(); src++) {
        if (comp(&ele[dst], &ele[src]) == 0) {
            /* Match, so replace the dest with the current source */
            Jim_DecrRefCount(g_sort_info->interp, ele[dst]);
        }
        else {
            /* No match, so keep the current source and move to the next destination */
            dst++;
        }
        ele[dst] = ele[src];
    }

    /* At end of list, keep the final element unless all elements were kept */
    dst++;
    if (dst < listObjPtr->get_listValue_len()) {
        ele[dst] = ele[src];
    }

    /* Set the new length */
    listObjPtr->setListValueLen( dst);
}

/* Sort a list *in place*. MUST be called with a non-shared list. */
STATIC int ListSortElements(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr, lsort_info *info) // #JimList
{
    PRJ_TRACE;
    lsort_info *prev_info;

    typedef int (qsort_comparator) (const void *, const void *);
    int (*fn) (Jim_ObjArray *, Jim_ObjArray *);
    Jim_ObjArray *vector;
    int len;
    int rc;

    JimPanic((Jim_IsShared(listObjPtr), "ListSortElements called with shared object"));
    SetListFromAny(interp, listObjPtr);

    /* Allow lsort to be called reentrant */
    prev_info = g_sort_info;
    g_sort_info = info;

    vector = listObjPtr->get_listValue_ele(); 
    len = listObjPtr->get_listValue_len();
    switch (info->type) {
        case JIM_LSORT_ASCII:
            fn = ListSortString;
            break;
        case JIM_LSORT_NOCASE:
            fn = ListSortStringNoCase;
            break;
        case JIM_LSORT_INTEGER:
            fn = ListSortInteger;
            break;
        case JIM_LSORT_REAL:
            fn = ListSortReal;
            break;
        case JIM_LSORT_COMMAND:
            fn = ListSortCommand;
            break;
        default:
            fn = NULL;          /* avoid warning */ // #MissInCoverage
            JimPanic((1, "ListSort called with invalid sort type"));
            return -1; /* Should not be run but keeps static analyzers happy */
    }

    if (info->indexed) {
        /* Need to interpose a "list index" function */
        info->subfn = fn;
        fn = ListSortIndexHelper;
    }

    if ((rc = setjmp(info->jmpbuf)) == 0) {
        qsort(vector, len, sizeof(Jim_ObjPtr ), (qsort_comparator *) fn);

        if (info->unique && len > 1) {
            ListRemoveDuplicates(listObjPtr, fn);
        }

        Jim_InvalidateStringRep(listObjPtr);
    }
    g_sort_info = prev_info;

    return rc;
}

/* This is the low-level function to insert elements into a list.
 * The higher-level Jim_ListInsertElements() performs shared object
 * check and invalidates the string repr. This version is used
 * in the internals of the List Object and is not exported.
 *
 * NOTE: this function can be called only against objects
 * with internal type of List.
 *
 * An insertion point (idx) of -1 means end-of-list.
 */
STATIC void ListInsertElements(Jim_ObjPtr listPtr, int idx, int elemc, Jim_ObjConstArray elemVec) // #JimList
{
    PRJ_TRACE;
    int currentLen = listPtr->get_listValue_len();
    int requiredLen = currentLen + elemc;
    int i;
    Jim_ObjArray *point;

    if (requiredLen > listPtr->get_listValue_maxLen()) {
        if (requiredLen < 2) {
            /* Don't do allocations of under 4 pointers. */
            requiredLen = 4; // #MagicNum
        }
        else {
            requiredLen *= 2; // #MagicNum
        }

        listPtr->internalRep.listValue_.ele_ = 
            realloc_Jim_ObjArray(listPtr->get_listValue_ele(), requiredLen);  

        listPtr->setListValueMaxLen( requiredLen);
    }
    if (idx < 0) {
        idx = currentLen;
    }
    point = listPtr->get_listValue_ele() + idx; 
    memmove(point + elemc, point, (currentLen - idx) * sizeof(Jim_ObjPtr ));
    for (i = 0; i < elemc; ++i) {
        point[i] = elemVec[i];
        Jim_IncrRefCount(point[i]);
    }
    listPtr->incrListValueLen( elemc);
}

/* Convenience call to ListInsertElements() to append a single element.
 */
static void ListAppendElement(Jim_ObjPtr listPtr, Jim_ObjPtr objPtr) // #JimList
{
    PRJ_TRACE;
    ListInsertElements(listPtr, -1, 1, &objPtr);
}

/* Appends every element of appendListPtr into listPtr.
 * Both have to be of the list type.
 * Convenience call to ListInsertElements()
 */
STATIC void ListAppendList(Jim_ObjPtr listPtr, Jim_ObjPtr appendListPtr) // #JimList
{
    PRJ_TRACE;
    ListInsertElements(listPtr, -1,
        appendListPtr->get_listValue_len(), appendListPtr->get_listValue_ele()); 
}

void Jim_ListAppendElement(Jim_InterpPtr interp, Jim_ObjPtr listPtr, Jim_ObjPtr objPtr) // #JimList
{
    PRJ_TRACE;
    JimPanic((Jim_IsShared(listPtr), "Jim_ListAppendElement called with shared object"));
    SetListFromAny(interp, listPtr);
    Jim_InvalidateStringRep(listPtr);
    ListAppendElement(listPtr, objPtr);
}

void Jim_ListAppendList(Jim_InterpPtr interp, Jim_ObjPtr listPtr, Jim_ObjPtr appendListPtr) // #JimList
{
    PRJ_TRACE;
    JimPanic((Jim_IsShared(listPtr), "Jim_ListAppendList called with shared object"));
    SetListFromAny(interp, listPtr);
    SetListFromAny(interp, appendListPtr);
    Jim_InvalidateStringRep(listPtr);
    ListAppendList(listPtr, appendListPtr);
}

int Jim_ListLength(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimList
{
    PRJ_TRACE;
    SetListFromAny(interp, objPtr);
    return objPtr->get_listValue_len();
}

void Jim_ListInsertElements(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int idx, //#JimList
    int objc, Jim_ObjConstArray objVec)
{
    PRJ_TRACE;
    JimPanic((Jim_IsShared(listPtr), "Jim_ListInsertElement called with shared object"));
    SetListFromAny(interp, listPtr);
    if (idx >= 0 && idx > listPtr->get_listValue_len())
        idx = listPtr->get_listValue_len(); // #MissInCoverage
    else if (idx < 0)
        idx = 0; /// #MissInCoverage
    Jim_InvalidateStringRep(listPtr);
    ListInsertElements(listPtr, idx, objc, objVec);
}

Jim_ObjPtr Jim_ListGetIndex(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int idx) // #JimList
{
    PRJ_TRACE;
    SetListFromAny(interp, listPtr);
    if ((idx >= 0 && idx >= listPtr->get_listValue_len()) ||
        (idx < 0 && (-idx - 1) >= listPtr->get_listValue_len())) {
        return NULL;
    }
    if (idx < 0)
        idx = listPtr->get_listValue_len() + idx;
    return listPtr->get_listValue_objArray(idx);
}

JIM_EXPORT Retval Jim_ListIndex(Jim_InterpPtr interp, Jim_ObjPtr listPrt, int listindex, Jim_ObjArray *objPtrPtr, int seterr) // #JimList
{
    PRJ_TRACE;
    *objPtrPtr = Jim_ListGetIndex(interp, listPrt, listindex);
    if (*objPtrPtr == NULL) {
        if (seterr & JIM_ERRMSG) {
            Jim_SetResultString(interp, "list index out of range", -1);
        }
        return JIM_ERR;
    }
    return JIM_OK;
}

STATIC Retval ListSetIndex(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int listindex, // #JimList
    Jim_ObjPtr newObjPtr, int flags)
{
    PRJ_TRACE;
    SetListFromAny(interp, listPtr);
    if ((listindex >= 0 && listindex >= listPtr->get_listValue_len()) ||
        (listindex < 0 && (-listindex - 1) >= listPtr->get_listValue_len())) {
        if (flags & JIM_ERRMSG) {
            Jim_SetResultString(interp, "list index out of range", -1);
        }
        return JIM_ERR;
    }
    if (listindex < 0)
        listindex = listPtr->get_listValue_len() + listindex;
    Jim_DecrRefCount(interp, listPtr->get_listValue_objArray(listindex));
    listPtr->set_listValue_objArray(listindex, newObjPtr); 
    Jim_IncrRefCount(newObjPtr);
    return JIM_OK;
}

/* Modify the list stored in the variable named 'varNamePtr'
 * setting the element specified by the 'indexc' indexes objects in 'indexv',
 * with the new element 'newObjptr'. (implements the [lset] command) */
JIM_EXPORT Retval Jim_ListSetIndex(Jim_InterpPtr interp, Jim_ObjPtr varNamePtr, // #JimList
    Jim_ObjConstArray indexv, int indexc, Jim_ObjPtr newObjPtr)
{
    PRJ_TRACE;
    Jim_Obj *varObjPtr, *objPtr, *listObjPtr;
    int shared, i, idx;

    varObjPtr = objPtr = Jim_GetVariable(interp, varNamePtr, JIM_ERRMSG | JIM_UNSHARED);
    if (objPtr == NULL)
        return JIM_ERR;
    if ((shared = Jim_IsShared(objPtr)))
        varObjPtr = objPtr = Jim_DuplicateObj(interp, objPtr);
    for (i = 0; i < indexc - 1; i++) {
        listObjPtr = objPtr;
        if (Jim_GetIndex(interp, indexv[i], &idx) != JIM_OK)
            goto err; // #MissInCoverage
        if (Jim_ListIndex(interp, listObjPtr, idx, &objPtr, JIM_ERRMSG) != JIM_OK) {
            goto err; // #MissInCoverage
        }
        if (Jim_IsShared(objPtr)) {
            objPtr = Jim_DuplicateObj(interp, objPtr);
            ListSetIndex(interp, listObjPtr, idx, objPtr, JIM_NONE);
        }
        Jim_InvalidateStringRep(listObjPtr);
    }
    if (Jim_GetIndex(interp, indexv[indexc - 1], &idx) != JIM_OK)
        goto err;
    if (ListSetIndex(interp, objPtr, idx, newObjPtr, JIM_ERRMSG) == JIM_ERR)
        goto err;
    Jim_InvalidateStringRep(objPtr);
    Jim_InvalidateStringRep(varObjPtr);
    if (Jim_SetVariable(interp, varNamePtr, varObjPtr) != JIM_OK)
        goto err; // #MissInCoverage
    Jim_SetResult(interp, varObjPtr);
    return JIM_OK;
  err:
    if (shared) {
        Jim_FreeNewObj(interp, varObjPtr);
    }
    return JIM_ERR;
}

JIM_EXPORT Jim_ObjPtr Jim_ListJoin(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr, const char *joinStr, int joinStrLen)  // #JimList
{
    PRJ_TRACE;
    int i;
    int listLen = Jim_ListLength(interp, listObjPtr);
    Jim_ObjPtr resObjPtr = Jim_NewEmptyStringObj(interp);

    for (i = 0; i < listLen; ) {
        Jim_AppendObj(interp, resObjPtr, Jim_ListGetIndex(interp, listObjPtr, i));
        if (++i != listLen) {
            Jim_AppendString(interp, resObjPtr, joinStr, joinStrLen);
        }
    }
    return resObjPtr;
}

JIM_EXPORT Jim_ObjPtr Jim_ConcatObj(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv) // #JimList
{
    PRJ_TRACE;
    int i;

    /* If all the objects in objv are lists,
     * it's possible to return a list as result, that's the
     * concatenation of all the lists. */
    for (i = 0; i < objc; i++) {
        if (!Jim_IsList(objv[i]))
            break;
    }
    if (i == objc) {
        Jim_ObjPtr objPtr = Jim_NewListObj(interp, NULL, 0);

        for (i = 0; i < objc; i++)
            ListAppendList(objPtr, objv[i]);
        return objPtr;
    }
    else {
        /* Else... we have to glue strings together */
        int len = 0, objLen;
        char *bytes, *p;

        /* Compute the length */
        for (i = 0; i < objc; i++) {
            len += Jim_Length(objv[i]);
        }
        if (objc)
            len += objc - 1;
        /* Create the string rep, and a string object holding it. */
        p = bytes = new_CharArray(len + 1); // #AllocF 
        for (i = 0; i < objc; i++) {
            const char *s = Jim_GetString(objv[i], &objLen);

            /* Remove leading space */
            while (objLen && isspace(UCHAR(*s))) {
                s++;
                objLen--;
                len--;
            }
            /* And trailing space */
            while (objLen && isspace(UCHAR(s[objLen - 1]))) {
                /* Handle trailing backslash-space case */
                if (objLen > 1 && s[objLen - 2] == '\\') {
                    break;
                }
                objLen--;
                len--;
            }
            memcpy(p, s, objLen);
            p += objLen;
            if (i + 1 != objc) {
                if (objLen)
                    *p++ = ' ';
                else {
                    /* Drop the space calculated for this
                     * element that is instead null. */
                    len--;
                }
            }
        }
        *p = '\0';
        return Jim_NewStringObjNoAlloc(interp, bytes, len);
    }
}

/* Returns a list composed of the elements in the specified range.
 * first and start are directly accepted as Jim_Objects and
 * processed for the end?-index? case. */
JIM_EXPORT Jim_ObjPtr Jim_ListRange(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr, Jim_ObjPtr firstObjPtr, // #JimList
    Jim_ObjPtr lastObjPtr)
{
    PRJ_TRACE;
    int first, last;
    int len, rangeLen;

    if (Jim_GetIndex(interp, firstObjPtr, &first) != JIM_OK ||
        Jim_GetIndex(interp, lastObjPtr, &last) != JIM_OK)
        return NULL;
    len = Jim_ListLength(interp, listObjPtr);   /* will convert into list */
    first = JimRelToAbsIndex(len, first);
    last = JimRelToAbsIndex(len, last);
    JimRelToAbsRange(len, &first, &last, &rangeLen);
    if (first == 0 && last == len) {
        return listObjPtr;  // #MissInCoverage
    }
    return Jim_NewListObj(interp, listObjPtr->get_listValue_ele() + first, rangeLen); 
}

/* -----------------------------------------------------------------------------
 * Dict object
 * ---------------------------------------------------------------------------*/
static void FreeDictInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
static void DupDictInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
static void UpdateStringOfDictCB(Jim_ObjPtr objPtr);
static Retval SetDictFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

/* Dict HashTable Type.
 *
 * Keys and Values are Jim objects. */

static unsigned_int JimObjectHTHashFunction(const void *key)
{
    PRJ_TRACE;
    int len;
    const char *str = Jim_GetString((Jim_ObjPtr )key, &len);
    return Jim_GenHashFunction((const_unsigned_char *)str, len);
}

static int JimObjectHTKeyCompare(void *privdata, const void *key1, const void *key2)
{
    PRJ_TRACE;
    return Jim_StringEqObj((Jim_ObjPtr )key1, (Jim_ObjPtr )key2);
}

static void *JimObjectHTKeyValDup(void *privdata, const void *val)
{
    PRJ_TRACE;
    Jim_IncrRefCount((Jim_ObjPtr )val);
    return (void *)val;
}

static void JimObjectHTKeyValDestructor(void *interp, void *val)
{
    PRJ_TRACE;
    Jim_DecrRefCount((Jim_InterpPtr )interp, (Jim_ObjPtr )val);
}

static const Jim_HashTableType g_JimDictHashTableType = {
    JimObjectHTHashFunction,    /* hash function */
    JimObjectHTKeyValDup,       /* key dup */
    JimObjectHTKeyValDup,       /* val dup */
    JimObjectHTKeyCompare,      /* key compare */
    JimObjectHTKeyValDestructor,    /* key destructor */
    JimObjectHTKeyValDestructor /* val destructor */
};

/* Note that while the elements of the dict may contain references,
 * the list object itself can't. This basically means that the
 * dict object string representation as a whole can't contain references
 * that are not presents in the single elements. */
static const Jim_ObjType g_dictObjType = { // #JimType #JimDict
    "dict",
    FreeDictInternalRepCB,
    DupDictInternalRepCB,
    UpdateStringOfDictCB,
    JIM_TYPE_NONE,
};
const Jim_ObjType& dictType() { return g_dictObjType; }

static void FreeDictInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimDict
{
    PRJ_TRACE;
    JIM_NOTUSED(interp);

    Jim_FreeHashTable((Jim_HashTablePtr )objPtr->getVoidPtr());
    Jim_TFreeNR<void>(objPtr->getVoidPtr(),"void"); // #FreeF 
}

static void DupDictInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #JimDict
{
    PRJ_TRACE;
    Jim_HashTablePtr ht; Jim_HashTablePtr dupHt;
    Jim_HashTableIterator htiter;
    Jim_HashEntryPtr he;

    /* Create a new hash table */
    ht = (Jim_HashTablePtr )srcPtr->getVoidPtr();
    dupHt = new_Jim_HashTable; // #AllocF 
    Jim_InitHashTable(dupHt, &g_JimDictHashTableType, interp);
    dupHt->setTypeName("dict");
    if (ht->size() != 0)
        Jim_ExpandHashTable(dupHt, ht->size());
    /* Copy every element from the source to the dup hash table */
    JimInitHashTableIterator(ht, &htiter);
    while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
        Jim_AddHashEntry(dupHt, he->keyAsVoid(), he->voidValue());
    }

    dupPtr->setPtr<Jim_HashTable*>( dupHt);
    dupPtr->setTypePtr(&g_dictObjType);
}

static Jim_ObjArray *JimDictPairs(Jim_ObjPtr dictPtr, int *len) // #JimDict
{
    PRJ_TRACE;
    Jim_HashTablePtr ht;
    Jim_HashTableIterator htiter;
    Jim_HashEntryPtr he;
    Jim_ObjArray *objv;
    int i;

    ht = (Jim_HashTablePtr )dictPtr->getVoidPtr();

    /* Turn the hash table into a flat vector of Jim_Objects. */
    objv = new_Jim_ObjArray((ht->used() * 2)); // #AllocF 
    JimInitHashTableIterator(ht, &htiter);
    i = 0;
    while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
        objv[i++] = (Jim_ObjPtr )Jim_GetHashEntryKey(he);
        objv[i++] = (Jim_ObjPtr )Jim_GetHashEntryVal(he);
    }
    *len = i;
    return objv;
}

static void UpdateStringOfDictCB(Jim_ObjPtr objPtr) // #JimDict
{
    PRJ_TRACE;
    /* Turn the hash table into a flat vector of Jim_Objects. */
    int len;
    Jim_ObjArray *objv = JimDictPairs(objPtr, &len);

    /* And now generate the string rep as a list */
    JimMakeListStringRep(objPtr, objv, len);

    free_Jim_ObjArray(objv); // #FreeF 
}

static Retval SetDictFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimDict
{
    PRJ_TRACE;
    int listlen;

    if (objPtr->typePtr() == &g_dictObjType) {
        return JIM_OK;
    }

    if (Jim_IsList(objPtr) && Jim_IsShared(objPtr)) {
        /* A shared list, so get the string representation now to avoid
         * changing the order in case of fast conversion to dict.
         */
        Jim_String(objPtr);
    }

    /* For simplicity, convert a non-list object to a list and then to a dict */
    listlen = Jim_ListLength(interp, objPtr);
    if (listlen % 2) {
        Jim_SetResultString(interp, "missing value to go with key", -1);
        return JIM_ERR;
    }
    else {
        /* Converting from a list to a dict can't fail */
        Jim_HashTablePtr ht;
        int i;

        ht = new_Jim_HashTable; // #AllocF 
        Jim_InitHashTable(ht, &g_JimDictHashTableType, interp);
        ht->setTypeName("dict");

        for (i = 0; i < listlen; i += 2) {
            Jim_ObjPtr keyObjPtr = Jim_ListGetIndex(interp, objPtr, i);
            Jim_ObjPtr valObjPtr = Jim_ListGetIndex(interp, objPtr, i + 1);

            Jim_ReplaceHashEntry(ht, keyObjPtr, valObjPtr);
        }

        Jim_FreeIntRep(interp, objPtr);
        objPtr->setTypePtr(&g_dictObjType);
        objPtr->setPtr<Jim_HashTable*>( ht);

        return JIM_OK;
    }
}

/* Dict object API */

/* Add an element to a dict. objPtr must be of the "dict" type.
 * The higher-level exported function is Jim_DictAddElement().
 * If an element with the specified key already exists, the value
 * associated is replaced with the new one.
 *
 * if valueObjPtr == NULL, the key is instead removed if it exists. */
static Retval DictAddElement(Jim_InterpPtr interp, Jim_ObjPtr objPtr,  // #JimDict
    Jim_ObjPtr keyObjPtr, Jim_ObjPtr valueObjPtr)
{
    Jim_HashTablePtr ht = (Jim_HashTablePtr )objPtr->getVoidPtr();

    if (valueObjPtr == NULL) {  /* unset */
        return Jim_DeleteHashEntry(ht, keyObjPtr);
    }
    Jim_ReplaceHashEntry(ht, keyObjPtr, valueObjPtr);
    return JIM_OK;
}

/* Add an element, higher-level interface for DictAddElement().
 * If valueObjPtr == NULL, the key is removed if it exists. */
JIM_EXPORT Retval Jim_DictAddElement(Jim_InterpPtr interp, Jim_ObjPtr objPtr, // #JimDict
    Jim_ObjPtr keyObjPtr, Jim_ObjPtr valueObjPtr)
{
    PRJ_TRACE;
    JimPanic((Jim_IsShared(objPtr), "Jim_DictAddElement called with shared object"));
    if (SetDictFromAny(interp, objPtr) != JIM_OK) {
        return JIM_ERR; // #MissInCoverage
    }
    Jim_InvalidateStringRep(objPtr);
    return DictAddElement(interp, objPtr, keyObjPtr, valueObjPtr);
}

JIM_EXPORT Jim_ObjPtr Jim_NewDictObj(Jim_InterpPtr interp, Jim_ObjConstArray elements, int len) // #JimDict
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;
    int i;

    JimPanic((len % 2, "Jim_NewDictObj() 'len' argument must be even"));

    objPtr = Jim_NewObj(interp);
    objPtr->setTypePtr(&g_dictObjType);
    objPtr->bytes_setNULL();
    objPtr->setPtr<Jim_HashTable*>( new_Jim_HashTable);  // #AllocF 
    Jim_InitHashTable((Jim_HashTablePtr )objPtr->getVoidPtr(), &g_JimDictHashTableType, interp);
    ((Jim_HashTablePtr) objPtr->getVoidPtr())->setTypeName("dict");
    for (i = 0; i < len; i += 2)
        DictAddElement(interp, objPtr, elements[i], elements[i + 1]);
    return objPtr;
}

/* Return the value associated to the specified dict key
 * Returns JIM_OK if OK, JIM_ERR if entry not found or -1 if can't create dict value
 *
 * Sets *objPtrPtr to non-NULL only upon success.
 */
JIM_EXPORT Retval Jim_DictKey(Jim_InterpPtr interp, Jim_ObjPtr dictPtr, Jim_ObjPtr keyPtr, // #JimDict
                              Jim_ObjArray *objPtrPtr, int flags)
{
    PRJ_TRACE;
    Jim_HashEntryPtr he;
    Jim_HashTablePtr ht;

    if (SetDictFromAny(interp, dictPtr) != JIM_OK) {
        return -1;
    }
    ht = (Jim_HashTablePtr )dictPtr->getVoidPtr();
    if ((he = Jim_FindHashEntry(ht, keyPtr)) == NULL) {
        if (flags & JIM_ERRMSG) {
            Jim_SetResultFormatted(interp, "key \"%#s\" not known in dictionary", keyPtr);
        }
        return JIM_ERR;
    }
    else {
        *objPtrPtr = (Jim_ObjPtr )Jim_GetHashEntryVal(he);
        return JIM_OK;
    }
}

/* Return an allocated array of key/value pairs for the dictionary. Stores the length in *len */
JIM_EXPORT Retval Jim_DictPairs(Jim_InterpPtr interp, Jim_ObjPtr dictPtr, Jim_ObjArray* *objPtrPtr, int *len) // #JimDict
{
    PRJ_TRACE;
    if (SetDictFromAny(interp, dictPtr) != JIM_OK) {
        return JIM_ERR;
    }
    *objPtrPtr = JimDictPairs(dictPtr, len);

    return JIM_OK;
}


/* Return the value associated to the specified dict keys */
JIM_EXPORT Retval Jim_DictKeysVector(Jim_InterpPtr interp, Jim_ObjPtr dictPtr, // #JimDict
    Jim_ObjConstArray keyv, int keyc, Jim_ObjArray* objPtrPtr, int flags)
{
    PRJ_TRACE;
    int i;

    if (keyc == 0) {
        *objPtrPtr = dictPtr;
        return JIM_OK;
    }

    for (i = 0; i < keyc; i++) {
        Jim_ObjPtr objPtr;

        int rc = Jim_DictKey(interp, dictPtr, keyv[i], &objPtr, flags);
        if (rc != JIM_OK) {
            return rc;
        }
        dictPtr = objPtr;
    }
    *objPtrPtr = dictPtr;
    return JIM_OK;
}

/* Modify the dict stored into the variable named 'varNamePtr'
 * setting the element specified by the 'keyc' keys objects in 'keyv',
 * with the new value of the element 'newObjPtr'.
 *
 * If newObjPtr == NULL the operation is to remove the given key
 * from the dictionary.
 *
 * If flags & JIM_ERRMSG, then failure to remove the key is considered an error
 * and JIM_ERR is returned. Otherwise it is ignored and JIM_OK is returned.
 */
JIM_EXPORT Retval Jim_SetDictKeysVector(Jim_InterpPtr interp, Jim_ObjPtr varNamePtr, // #JimDict
    Jim_ObjConstArray keyv, int keyc, Jim_ObjPtr newObjPtr, int flags)
{
    PRJ_TRACE;
    Jim_Obj *varObjPtr, *objPtr, *dictObjPtr;
    int shared, i;

    varObjPtr = objPtr = Jim_GetVariable(interp, varNamePtr, flags);
    if (objPtr == NULL) {
        if (newObjPtr == NULL && (flags & JIM_MUSTEXIST)) {
            /* Cannot remove a key from non existing var */
            return JIM_ERR; // #MissInCoverage
        }
        varObjPtr = objPtr = Jim_NewDictObj(interp, NULL, 0);
        if (Jim_SetVariable(interp, varNamePtr, objPtr) != JIM_OK) {
            Jim_FreeNewObj(interp, varObjPtr); // #MissInCoverage
            return JIM_ERR;
        }
    }
    if ((shared = Jim_IsShared(objPtr)))
        varObjPtr = objPtr = Jim_DuplicateObj(interp, objPtr);
    for (i = 0; i < keyc; i++) {
        dictObjPtr = objPtr;

        /* Check if it's a valid dictionary */
        if (SetDictFromAny(interp, dictObjPtr) != JIM_OK) {
            goto err;
        }

        if (i == keyc - 1) {
            /* Last key: Note that error on unset with missing last key is OK */
            if (Jim_DictAddElement(interp, objPtr, keyv[keyc - 1], newObjPtr) != JIM_OK) {
                if (newObjPtr || (flags & JIM_MUSTEXIST)) {
                    goto err;
                }
            }
            break;
        }

        /* Check if the given key exists. */
        Jim_InvalidateStringRep(dictObjPtr);
        if (Jim_DictKey(interp, dictObjPtr, keyv[i], &objPtr,
                newObjPtr ? JIM_NONE : JIM_ERRMSG) == JIM_OK) {
            /* This key exists at the current level.
             * Make sure it's not shared!. */
            if (Jim_IsShared(objPtr)) {
                objPtr = Jim_DuplicateObj(interp, objPtr);
                DictAddElement(interp, dictObjPtr, keyv[i], objPtr);
            }
        }
        else {
            /* Key not found. If it's an [unset] operation
             * this is an error. Only the last key may not
             * exist. */
            if (newObjPtr == NULL) {
                goto err;
            }
            /* Otherwise set an empty dictionary
             * as key's value. */
            objPtr = Jim_NewDictObj(interp, NULL, 0);
            DictAddElement(interp, dictObjPtr, keyv[i], objPtr);
        }
    }
    /* XXX: Is this necessary? */
    Jim_InvalidateStringRep(objPtr);
    Jim_InvalidateStringRep(varObjPtr);
    if (Jim_SetVariable(interp, varNamePtr, varObjPtr) != JIM_OK) {
        goto err; // #MissInCoverage
    }
    Jim_SetResult(interp, varObjPtr);
    return JIM_OK;
  err:
    if (shared) {
        Jim_FreeNewObj(interp, varObjPtr);
    }
    return JIM_ERR;
}

/* -----------------------------------------------------------------------------
 * Index object
 * ---------------------------------------------------------------------------*/
static void UpdateStringOfIndexCB(Jim_ObjPtr objPtr);
static Retval SetIndexFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

static const Jim_ObjType g_indexObjType = { // #JimType #JimIndex
    "index",
    NULL,
    NULL,
    UpdateStringOfIndexCB,
    JIM_TYPE_NONE,
};
const Jim_ObjType& indexType() { return g_indexObjType; }

static void UpdateStringOfIndexCB(Jim_ObjPtr objPtr) // #MissInCoverage
{
    PRJ_TRACE;
    if (objPtr->getIntValue() == -1) {
        JimSetStringBytes(objPtr, "end");
    }
    else {
        char buf[JIM_INTEGER_SPACE + 1];
        if (objPtr->getIntValue() >= 0) {
            sprintf(buf, "%d", objPtr->getIntValue());
        }
        else {
            /* Must be <= -2 */
            sprintf(buf, "end%d", objPtr->getIntValue() + 1);
        }
        JimSetStringBytes(objPtr, buf);
    }
}

static Retval SetIndexFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimIndex
{
    PRJ_TRACE;
    int idx, end = 0;
    const char *str;
    char *endptr;

    /* Get the string representation */
    str = Jim_String(objPtr);

    /* Try to convert into an index */
    if (strncmp(str, "end", 3) == 0) { // #MagicNum
        end = 1;
        str += 3; // #MagicNum
        idx = 0;
    }
    else {
        idx = jim_strtol(str, &endptr);

        if (endptr == str) {
            goto badindex;
        }
        str = endptr;
    }

    /* Now str may include or +<num> or -<num> */
    if (*str == '+' || *str == '-') {
        int sign = (*str == '+' ? 1 : -1);

        idx += sign * jim_strtol(++str, &endptr);
        if (str == endptr || *endptr) {
            goto badindex;
        }
        str = endptr;
    }
    /* The only thing left should be spaces */
    while (isspace(UCHAR(*str))) {
        str++;
    }
    if (*str) {
        goto badindex;
    }
    if (end) {
        if (idx > 0) {
            idx = INT_MAX;
        }
        else {
            /* end-1 is represented as -2 */
            idx--;
        }
    }
    else if (idx < 0) {
        idx = -INT_MAX;
    }

    /* Free the old internal repr and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_indexObjType);
    objPtr->setIntValue( idx);
    return JIM_OK;

  badindex:
    Jim_SetResultFormatted(interp,
        "bad index \"%#s\": must be integer?[+-]integer? or end?[+-]integer?", objPtr);
    return JIM_ERR;
}

JIM_EXPORT Retval Jim_GetIndex(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int *indexPtr) // #JimIndex
{
    PRJ_TRACE;
    /* Avoid shimmering if the object is an integer. */
    if (objPtr->typePtr() == &g_intObjType) {
        jim_wide val = JimWideValue(objPtr);

        if (val < 0)
            *indexPtr = -INT_MAX;
        else if (val > INT_MAX)
            *indexPtr = INT_MAX; // #MissInCoverage
        else
            *indexPtr = (int)val;
        return JIM_OK;
    }
    if (objPtr->typePtr() != &g_indexObjType && SetIndexFromAny(interp, objPtr) == JIM_ERR)
        return JIM_ERR;
    *indexPtr = objPtr->getIntValue();
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * Return Code Object.
 * ---------------------------------------------------------------------------*/

/* NOTE: These must be kept in the same order as JIM_OK, JIM_ERR, ... */
static const char * const g_jimReturnCodes[] = {
    "ok",
    "error",
    "return",
    "break",
    "continue",
    "signal",
    "exit",
    "eval",
    NULL
};

enum { jimReturnCodesSize = (sizeof(g_jimReturnCodes)/sizeof(*g_jimReturnCodes) - 1) };

static const Jim_ObjType g_returnCodeObjType = { // #JimType #JimRet
    "return-code",
    NULL,
    NULL,
    NULL,
    JIM_TYPE_NONE,
};
const Jim_ObjType& returnCodeType() { return g_returnCodeObjType; }

/* Converts a (standard) return code to a string. Returns "?" for
 * non-standard return codes.
 */
JIM_EXPORT const char *Jim_ReturnCode(int code) // #JimRet
{
    PRJ_TRACE;
    if (code < 0 || code >= (int)jimReturnCodesSize) {
        return "?";
    }
    else {
        return g_jimReturnCodes[code];
    }
}

static Retval SetReturnCodeFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimRet
{
    PRJ_TRACE;
    int returnCode;
    jim_wide wideValue;

    /* Try to convert into an integer */
    if (JimGetWideNoErr(interp, objPtr, &wideValue) != JIM_ERR)
        returnCode = (int)wideValue;
    else if (Jim_GetEnum(interp, objPtr, g_jimReturnCodes, &returnCode, NULL, JIM_NONE) != JIM_OK) {
        Jim_SetResultFormatted(interp, "expected return code but got \"%#s\"", objPtr);
        return JIM_ERR;
    }
    /* Free the old internal repr and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setTypePtr(&g_returnCodeObjType);
    objPtr->setIntValue( returnCode);
    return JIM_OK;
}

JIM_EXPORT Retval Jim_GetReturnCode(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int *intPtr) // #JimRet
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_returnCodeObjType && SetReturnCodeFromAny(interp, objPtr) == JIM_ERR)
        return JIM_ERR;
    *intPtr = objPtr->getIntValue();
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * Expression Parsing
 * ---------------------------------------------------------------------------*/
STATIC Retval JimParseExprOperator(JimParserCtxPtr pc);
STATIC Retval JimParseExprNumber(JimParserCtxPtr pc);
STATIC Retval JimParseExprIrrational(JimParserCtxPtr pc);
STATIC Retval JimParseExprBoolean(JimParserCtxPtr pc);

/* expr operator opcodes. */
enum JIM_EXPROP
{
    /* Continues on from the JIM_TT_ space */

    /* Binary operators (numbers) */
    JIM_EXPROP_MUL = JIM_TT_EXPR_OP,             /* 20 */
    JIM_EXPROP_DIV,
    JIM_EXPROP_MOD,
    JIM_EXPROP_SUB,
    JIM_EXPROP_ADD,
    JIM_EXPROP_LSHIFT,
    JIM_EXPROP_RSHIFT,
    JIM_EXPROP_ROTL,
    JIM_EXPROP_ROTR,
    JIM_EXPROP_LT,
    JIM_EXPROP_GT,
    JIM_EXPROP_LTE,
    JIM_EXPROP_GTE,
    JIM_EXPROP_NUMEQ,
    JIM_EXPROP_NUMNE,
    JIM_EXPROP_BITAND,          /* 35 */
    JIM_EXPROP_BITXOR,
    JIM_EXPROP_BITOR,
    JIM_EXPROP_LOGICAND,        /* 38 */
    JIM_EXPROP_LOGICOR,         /* 39 */
    JIM_EXPROP_TERNARY,         /* 40 */
    JIM_EXPROP_COLON,           /* 41 */
    JIM_EXPROP_POW,             /* 42 */

    /* Binary operators (strings) */
    JIM_EXPROP_STREQ,           /* 43 */
    JIM_EXPROP_STRNE,
    JIM_EXPROP_STRIN,
    JIM_EXPROP_STRNI,

    /* Unary operators (numbers) */
    JIM_EXPROP_NOT,             /* 47 */
    JIM_EXPROP_BITNOT,
    JIM_EXPROP_UNARYMINUS,
    JIM_EXPROP_UNARYPLUS,

    /* Functions */
    JIM_EXPROP_FUNC_INT,      /* 51 */
    JIM_EXPROP_FUNC_WIDE,
    JIM_EXPROP_FUNC_ABS,
    JIM_EXPROP_FUNC_DOUBLE,
    JIM_EXPROP_FUNC_ROUND,
    JIM_EXPROP_FUNC_RAND,
    JIM_EXPROP_FUNC_SRAND,

    /* math functions from libm */
    JIM_EXPROP_FUNC_SIN,        /* 68 */
    JIM_EXPROP_FUNC_COS,
    JIM_EXPROP_FUNC_TAN,
    JIM_EXPROP_FUNC_ASIN,
    JIM_EXPROP_FUNC_ACOS,
    JIM_EXPROP_FUNC_ATAN,
    JIM_EXPROP_FUNC_ATAN2,
    JIM_EXPROP_FUNC_SINH,
    JIM_EXPROP_FUNC_COSH,
    JIM_EXPROP_FUNC_TANH,
    JIM_EXPROP_FUNC_CEIL,
    JIM_EXPROP_FUNC_FLOOR,
    JIM_EXPROP_FUNC_EXP,
    JIM_EXPROP_FUNC_LOG,
    JIM_EXPROP_FUNC_LOG10,
    JIM_EXPROP_FUNC_SQRT,
    JIM_EXPROP_FUNC_POW,
    JIM_EXPROP_FUNC_HYPOT,
    JIM_EXPROP_FUNC_FMOD,
};

/* A expression node is either a term or an operator
 * If a node is an operator, 'op' points to the details of the operator and it's terms.
 */
struct JimExprNode {
private:
    int type = 0;       /* JIM_TT_xxx */
    Jim_ObjPtr objPtr = NULL;      /* The object for a term, or NULL for an operator */

    JimExprNodePtr left = NULL;    /* For all operators */
    JimExprNodePtr right = NULL;   /* For binary operators */
    JimExprNodePtr ternary = NULL; /* For ternary operator only */
public:

    friend STATIC int JimExprOpNumUnary(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpIntUnary(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpDoubleUnary(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpIntBin(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpBin(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpStrBin(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC void JimShowExprNode(JimExprNodePtr node, int level);
    friend STATIC int ExprTreeBuildTree(Jim_InterpPtr interp, ExprBuilderPtr builder, int precedence, int flags, int exp_numterms);
    friend STATIC Jim_ObjPtr JimExprIntValOrVar(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprEvalTermNode(Jim_InterpPtr interp, JimExprNodePtr node);
    friend int Jim_EvalExpression(Jim_InterpPtr interp, Jim_ObjPtr exprObjPtr);
    friend STATIC int Jim_ForCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend STATIC Jim_ObjPtr JimGetExprAsList(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpAnd(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpOr(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC int JimExprOpTernary(Jim_InterpPtr interp, JimExprNodePtr node);
    friend STATIC void ExprTreeFreeNodes(Jim_InterpPtr interp, JimExprNodePtr nodes, int num);
};

#define free_JimExprNode(ptr)           Jim_TFree<JimExprNode>(ptr)

/* Operators table */
struct Jim_ExprOperator
{
    const char *name = NULL;
    int (*funcop) (Jim_InterpPtr interp, JimExprNodePtr opnode) = NULL;
    unsigned_char precedence = 0;
    unsigned_char arity = 0;
    unsigned_char attr = 0;
    unsigned_char namelen = 0;
};

static Retval JimExprGetTerm(Jim_InterpPtr interp, JimExprNodePtr node, Jim_ObjArray* objPtrPtr);
static int JimExprGetTermBoolean(Jim_InterpPtr interp, JimExprNodePtr node);
STATIC Retval JimExprEvalTermNode(Jim_InterpPtr interp, JimExprNodePtr node);

STATIC Retval JimExprOpNumUnary(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    int intresult = 1;
    Retval rc;
    double dA, dC = 0;
    jim_wide wA, wC = 0;
    Jim_ObjPtr A;

    if ((rc = JimExprGetTerm(interp, node->left, &A)) != JIM_OK) {
        return rc;
    }

    if ((A->typePtr() != &g_doubleObjType || A->bytes()) && JimGetWideNoErr(interp, A, &wA) == JIM_OK) {
        switch (node->type) {
            case JIM_EXPROP_FUNC_INT:
            case JIM_EXPROP_FUNC_WIDE:
            case JIM_EXPROP_FUNC_ROUND:
            case JIM_EXPROP_UNARYPLUS:
                wC = wA;
                break;
            case JIM_EXPROP_FUNC_DOUBLE:
                dC = (double)wA;
                intresult = 0;
                break;
            case JIM_EXPROP_FUNC_ABS:
                wC = wA >= 0 ? wA : -wA;
                break;
            case JIM_EXPROP_UNARYMINUS:
                wC = -wA;
                break;
            case JIM_EXPROP_NOT:
                wC = !wA;
                break;
            default:
                JIM_ABORT(); // #MissInCoverage #FIXME can't abort in all situations have to have another way!
        }
    }
    else if ((rc = Jim_GetDouble(interp, A, &dA)) == JIM_OK) {
        switch (node->type) {
            case JIM_EXPROP_FUNC_INT:
            case JIM_EXPROP_FUNC_WIDE:
                wC = (int64_t)dA;
                break;
            case JIM_EXPROP_FUNC_ROUND:
                wC = (int64_t)(dA < 0 ? (dA - 0.5) : (dA + 0.5));
                break;
            case JIM_EXPROP_FUNC_DOUBLE:
            case JIM_EXPROP_UNARYPLUS:
                dC = dA;
                intresult = 0;
                break;
            case JIM_EXPROP_FUNC_ABS:
#ifdef JIM_MATH_FUNCTIONS // #optionalCode
                dC = fabs(dA);
#else // #WinOff
                dC = dA >= 0 ? dA : -dA;
#endif
                intresult = 0;
                break;
            case JIM_EXPROP_UNARYMINUS:
                dC = -dA;
                intresult = 0;
                break;
            case JIM_EXPROP_NOT:
                wC = !dA;
                break;
            default:
                JIM_ABORT(); // #MissInCoverage #FIXME can't abort in all situations have to have another way!
        }
    }

    if (rc == JIM_OK) {
        if (intresult) {
            Jim_SetResultInt(interp, wC);
        }
        else {
            Jim_SetResult(interp, Jim_NewDoubleObj(interp, dC));
        }
    }

    Jim_DecrRefCount(interp, A);

    return rc;
}

static double JimRandDouble(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    unsigned_long x;
    JimRandomBytes(interp, &x, sizeof(x));

    return (double)x / (unsigned_long)~0;
}

STATIC Retval JimExprOpIntUnary(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    jim_wide wA;
    Jim_ObjPtr A;
    Retval rc;

    if ((rc = JimExprGetTerm(interp, node->left, &A)) != JIM_OK) {
        return rc; // #MissInCoverage
    }

    rc = Jim_GetWide(interp, A, &wA);
    if (rc == JIM_OK) {
        switch (node->type) {
            case JIM_EXPROP_BITNOT:
                Jim_SetResultInt(interp, ~wA);
                break;
            case JIM_EXPROP_FUNC_SRAND: // #MissInCoverage
                JimPrngSeed(interp, (unsigned_char *)&wA, sizeof(wA));
                Jim_SetResult(interp, Jim_NewDoubleObj(interp, JimRandDouble(interp)));
                break;
            default:
                JIM_ABORT(); // #MissInCoverage #FIXME can't abort in all situations have to have another way!
        }
    }

    Jim_DecrRefCount(interp, A);

    return rc;
}

static Retval JimExprOpNone(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    JimPanic((node->type != JIM_EXPROP_FUNC_RAND, "JimExprOpNone only support rand()"));

    Jim_SetResult(interp, Jim_NewDoubleObj(interp, JimRandDouble(interp)));

    return JIM_OK;
}

#ifdef JIM_MATH_FUNCTIONS // #optionalCode
STATIC Retval JimExprOpDoubleUnary(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    Retval rc;
    double dA, dC;
    Jim_ObjPtr A;

    if ((rc = JimExprGetTerm(interp, node->left, &A)) != JIM_OK) {
        return rc; // #MissInCoverage
    }

    rc = Jim_GetDouble(interp, A, &dA);
    if (rc == JIM_OK) {
        switch (node->type) {
            case JIM_EXPROP_FUNC_SIN:
                dC = sin(dA);
                break;
            case JIM_EXPROP_FUNC_COS:
                dC = cos(dA);
                break;
            case JIM_EXPROP_FUNC_TAN:
                dC = tan(dA);
                break;
            case JIM_EXPROP_FUNC_ASIN:
                dC = asin(dA);
                break;
            case JIM_EXPROP_FUNC_ACOS:
                dC = acos(dA);
                break;
            case JIM_EXPROP_FUNC_ATAN:
                dC = atan(dA);
                break;
            case JIM_EXPROP_FUNC_SINH:
                dC = sinh(dA);
                break;
            case JIM_EXPROP_FUNC_COSH:
                dC = cosh(dA);
                break;
            case JIM_EXPROP_FUNC_TANH:
                dC = tanh(dA);
                break;
            case JIM_EXPROP_FUNC_CEIL:
                dC = ceil(dA);
                break;
            case JIM_EXPROP_FUNC_FLOOR:
                dC = floor(dA);
                break;
            case JIM_EXPROP_FUNC_EXP:
                dC = exp(dA);
                break;
            case JIM_EXPROP_FUNC_LOG:
                dC = log(dA);
                break;
            case JIM_EXPROP_FUNC_LOG10:
                dC = log10(dA);
                break;
            case JIM_EXPROP_FUNC_SQRT:
                dC = sqrt(dA);
                break;
            default:
                JIM_ABORT(); // #MissInCoverage #FIXME can't abort in all situations have to have another way!
        }
        Jim_SetResult(interp, Jim_NewDoubleObj(interp, dC));
    }

    Jim_DecrRefCount(interp, A);

    return rc;
}
#endif

/* A binary operation on two ints */
STATIC Retval JimExprOpIntBin(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    jim_wide wA, wB;
    Retval rc;
    Jim_Obj *A, *B;

    if ((rc = JimExprGetTerm(interp, node->left, &A)) != JIM_OK) {
        return rc; // #MissInCoverage
    }
    if ((rc = JimExprGetTerm(interp, node->right, &B)) != JIM_OK) {
        Jim_DecrRefCount(interp, A); // #MissInCoverage
        return rc;
    }

    rc = JIM_ERR;

    if (Jim_GetWide(interp, A, &wA) == JIM_OK && Jim_GetWide(interp, B, &wB) == JIM_OK) {
        jim_wide wC;

        rc = JIM_OK;

        switch (node->type) {
            case JIM_EXPROP_LSHIFT:
                wC = wA << wB;
                break;
            case JIM_EXPROP_RSHIFT:
                wC = wA >> wB;
                break;
            case JIM_EXPROP_BITAND:
                wC = wA & wB;
                break;
            case JIM_EXPROP_BITXOR:
                wC = wA ^ wB;
                break;
            case JIM_EXPROP_BITOR:
                wC = wA | wB;
                break;
            case JIM_EXPROP_MOD:
                if (wB == 0) {
                    wC = 0;
                    Jim_SetResultString(interp, "Division by zero", -1);
                    rc = JIM_ERR;
                }
                else {
                    /*
                     * From Tcl 8.x
                     *
                     * This code is tricky: C doesn't guarantee much
                     * about the quotient or remainder, but Tcl does.
                     * The remainder always has the same sign as the
                     * divisor and a smaller absolute value.
                     */
                    int negative = 0;

                    if (wB < 0) {
                        wB = -wB;
                        wA = -wA;
                        negative = 1;
                    }
                    wC = wA % wB;
                    if (wC < 0) {
                        wC += wB;
                    }
                    if (negative) {
                        wC = -wC;
                    }
                }
                break;
            case JIM_EXPROP_ROTL:
            case JIM_EXPROP_ROTR:{
                    /* uint32_t would be better. But not everyone has inttypes.h? */
                    unsigned_long uA = (unsigned_long)wA;
                    unsigned_long uB = (unsigned_long)wB;
                    const unsigned_int S = sizeof(unsigned_long) * 8; // #MagicNum

                    /* Shift left by the word size or more is undefined. */
                    uB %= S;

                    if (node->type == JIM_EXPROP_ROTR) {
                        uB = S - uB;
                    }
                    wC = (unsigned_long)(uA << uB) | (uA >> (S - uB));
                    break;
                }
            default:
                JIM_ABORT(); // #MissInCoverage #FIXME can't abort in all situations have to have another way!
        }
        Jim_SetResultInt(interp, wC);
    }

    Jim_DecrRefCount(interp, A);
    Jim_DecrRefCount(interp, B);

    return rc;
}


/* A binary operation on two ints or two doubles (or two strings for some ops) */
STATIC Retval JimExprOpBin(Jim_InterpPtr interp, JimExprNodePtr node) //  #JimExpr
{
    PRJ_TRACE;
    Retval rc = JIM_OK;
    double dA, dB, dC = 0;
    jim_wide wA, wB, wC = 0;
    Jim_Obj *A, *B;

    if ((rc = JimExprGetTerm(interp, node->left, &A)) != JIM_OK) {
        return rc;
    }
    if ((rc = JimExprGetTerm(interp, node->right, &B)) != JIM_OK) {
        Jim_DecrRefCount(interp, A);
        return rc;
    }

    if ((A->typePtr() != &g_doubleObjType || A->bytes()) &&
        (B->typePtr() != &g_doubleObjType || B->bytes()) &&
        JimGetWideNoErr(interp, A, &wA) == JIM_OK && JimGetWideNoErr(interp, B, &wB) == JIM_OK) {

        /* Both are ints */

        switch (node->type) {
            case JIM_EXPROP_POW:
            case JIM_EXPROP_FUNC_POW:
                if (wA == 0 && wB < 0) {
                    Jim_SetResultString(interp, "exponentiation of zero by negative power", -1);
                    rc = JIM_ERR;
                    goto done;
                }
                wC = JimPowWide(wA, wB);
                goto intresult;
            case JIM_EXPROP_ADD:
                wC = wA + wB;
                goto intresult;
            case JIM_EXPROP_SUB:
                wC = wA - wB;
                goto intresult;
            case JIM_EXPROP_MUL:
                wC = wA * wB;
                goto intresult;
            case JIM_EXPROP_DIV:
                if (wB == 0) {
                    Jim_SetResultString(interp, "Division by zero", -1);
                    rc = JIM_ERR;
                    goto done;
                }
                else {
                    /*
                     * From Tcl 8.x
                     *
                     * This code is tricky: C doesn't guarantee much
                     * about the quotient or remainder, but Tcl does.
                     * The remainder always has the same sign as the
                     * divisor and a smaller absolute value.
                     */
                    if (wB < 0) {
                        wB = -wB;
                        wA = -wA;
                    }
                    wC = wA / wB;
                    if (wA % wB < 0) {
                        wC--;
                    }
                    goto intresult;
                }
            case JIM_EXPROP_LT:
                wC = wA < wB;
                goto intresult;
            case JIM_EXPROP_GT:
                wC = wA > wB;
                goto intresult;
            case JIM_EXPROP_LTE:
                wC = wA <= wB;
                goto intresult;
            case JIM_EXPROP_GTE:
                wC = wA >= wB;
                goto intresult;
            case JIM_EXPROP_NUMEQ:
                wC = wA == wB;
                goto intresult;
            case JIM_EXPROP_NUMNE:
                wC = wA != wB;
                goto intresult;
        }
    }
    if (Jim_GetDouble(interp, A, &dA) == JIM_OK && Jim_GetDouble(interp, B, &dB) == JIM_OK) {
        switch (node->type) {
#ifndef JIM_MATH_FUNCTIONS // #optionalCode #WinOff
            case JIM_EXPROP_POW:
            case JIM_EXPROP_FUNC_POW:
            case JIM_EXPROP_FUNC_ATAN2:
            case JIM_EXPROP_FUNC_HYPOT:
            case JIM_EXPROP_FUNC_FMOD:
                Jim_SetResultString(interp, "unsupported", -1);
                rc = JIM_ERR;
                goto done;
#else
            case JIM_EXPROP_POW:
            case JIM_EXPROP_FUNC_POW:
                dC = pow(dA, dB);
                goto doubleresult;
            case JIM_EXPROP_FUNC_ATAN2:
                dC = atan2(dA, dB);
                goto doubleresult;
            case JIM_EXPROP_FUNC_HYPOT:
                dC = hypot(dA, dB);
                goto doubleresult;
            case JIM_EXPROP_FUNC_FMOD:
                dC = fmod(dA, dB);
                goto doubleresult;
#endif
            case JIM_EXPROP_ADD:
                dC = dA + dB;
                goto doubleresult;
            case JIM_EXPROP_SUB:
                dC = dA - dB;
                goto doubleresult;
            case JIM_EXPROP_MUL:
                dC = dA * dB;
                goto doubleresult;
            case JIM_EXPROP_DIV:
                if (dB == 0) {
#ifdef INFINITY // #optionalCode
                    dC = dA < 0 ? -INFINITY : INFINITY;
#else // #WinOff
                    dC = (dA < 0 ? -1.0 : 1.0) * strtod("Inf", NULL);
#endif
                }
                else {
                    dC = dA / dB;
                }
                goto doubleresult;
            case JIM_EXPROP_LT:
                wC = dA < dB;
                goto intresult;
            case JIM_EXPROP_GT:
                wC = dA > dB;
                goto intresult;
            case JIM_EXPROP_LTE:
                wC = dA <= dB;
                goto intresult;
            case JIM_EXPROP_GTE:
                wC = dA >= dB;
                goto intresult;
            case JIM_EXPROP_NUMEQ:
                wC = dA == dB;
                goto intresult;
            case JIM_EXPROP_NUMNE:
                wC = dA != dB;
                goto intresult;
        }
    }
    else {
        /* Handle the string case */

        /* XXX: Could optimise the eq/ne case by checking lengths */
        int i = Jim_StringCompareObj(interp, A, B, 0);

        switch (node->type) {
            case JIM_EXPROP_LT:
                wC = i < 0;
                goto intresult;
            case JIM_EXPROP_GT:
                wC = i > 0;
                goto intresult;
            case JIM_EXPROP_LTE:
                wC = i <= 0;
                goto intresult;
            case JIM_EXPROP_GTE:
                wC = i >= 0;
                goto intresult;
            case JIM_EXPROP_NUMEQ:
                wC = i == 0;
                goto intresult;
            case JIM_EXPROP_NUMNE:
                wC = i != 0;
                goto intresult;
        }
    }
    /* If we get here, it is an error */
    rc = JIM_ERR;
done:
    Jim_DecrRefCount(interp, A);
    Jim_DecrRefCount(interp, B);
    return rc;
intresult:
    Jim_SetResultInt(interp, wC);
    goto done;
doubleresult:
    Jim_SetResult(interp, Jim_NewDoubleObj(interp, dC));
    goto done;
}

static int JimSearchList(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr, Jim_ObjPtr valObj)
{
    PRJ_TRACE;
    int listlen;
    int i;

    listlen = Jim_ListLength(interp, listObjPtr);
    for (i = 0; i < listlen; i++) {
        if (Jim_StringEqObj(Jim_ListGetIndex(interp, listObjPtr, i), valObj)) {
            return 1;
        }
    }
    return 0;
}



STATIC Retval JimExprOpStrBin(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    Jim_Obj *A, *B;
    jim_wide wC;
    Retval rc;

    if ((rc = JimExprGetTerm(interp, node->left, &A)) != JIM_OK) {
        return rc; // #MissInCoverage
    }
    if ((rc = JimExprGetTerm(interp, node->right, &B)) != JIM_OK) {
        Jim_DecrRefCount(interp, A); // #MissInCoverage
        return rc;
    }

    switch (node->type) {
        case JIM_EXPROP_STREQ:
        case JIM_EXPROP_STRNE:
            wC = Jim_StringEqObj(A, B);
            if (node->type == JIM_EXPROP_STRNE) {
                wC = !wC;
            }
            break;
        case JIM_EXPROP_STRIN:
            wC = JimSearchList(interp, B, A);
            break;
        case JIM_EXPROP_STRNI:
            wC = !JimSearchList(interp, B, A);
            break;
        default:
            JIM_ABORT(); // #MissInCoverage #FIXME can't abort in all situations have to have another way!
    }
    Jim_SetResultInt(interp, wC);

    Jim_DecrRefCount(interp, A);
    Jim_DecrRefCount(interp, B);

    return rc;
}

static int ExprBool(Jim_InterpPtr interp, Jim_ObjPtr obj)
{
    PRJ_TRACE;
    long l;
    double d;
    int b;
    int ret = -1;

    /* In case the object is interp->result with refcount 1*/
    Jim_IncrRefCount(obj);

    if (Jim_GetLong(interp, obj, &l) == JIM_OK) {
        ret = (l != 0);
    }
    else if (Jim_GetDouble(interp, obj, &d) == JIM_OK) {
        ret = (d != 0);
    }
    else if (Jim_GetBoolean(interp, obj, &b) == JIM_OK) {
        ret = (b != 0);
    }

    Jim_DecrRefCount(interp, obj);
    return ret;
}

STATIC Retval JimExprOpAnd(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    /* evaluate left */
    int result = JimExprGetTermBoolean(interp, node->left);

    if (result == 1) {
        /* true so evaluate right */
        result = JimExprGetTermBoolean(interp, node->right);
    }
    if (result == -1) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp, result);
    return JIM_OK;
}

STATIC Retval JimExprOpOr(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    /* evaluate left */
    int result = JimExprGetTermBoolean(interp, node->left);

    if (result == 0) {
        /* false so evaluate right */
        result = JimExprGetTermBoolean(interp, node->right);
    }
    if (result == -1) {
        return JIM_ERR;
    }
    Jim_SetResultInt(interp, result);
    return JIM_OK;
}

STATIC Retval JimExprOpTernary(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    /* evaluate left */
    int result = JimExprGetTermBoolean(interp, node->left);

    if (result == 1) {
        /* true so select right */
        return JimExprEvalTermNode(interp, node->right);
    }
    else if (result == 0) {
        /* false so select ternary */
        return JimExprEvalTermNode(interp, node->ternary);
    }
    /* error */
    return JIM_ERR;
}

enum OP_SYNTAX {
    OP_FUNC = 0x0001,        /* function syntax */
    OP_RIGHT_ASSOC = 0x0002, /* right associative */
};

/* name - precedence - arity - opcode
 *
 * This array *must* be kept in sync with the JIM_EXPROP enum.
 *
 * The following macros pre-compute the string length at compile time.
 */
#define OPRINIT_ATTR(N, P, ARITY, F, ATTR) {N, F, P, ARITY, ATTR, sizeof(N) - 1}
#define OPRINIT(N, P, ARITY, F) OPRINIT_ATTR(N, P, ARITY, F, 0)

static const struct Jim_ExprOperator g_Jim_ExprOperators[] = { // #JimExprOperators
    OPRINIT("*", 110, 2, JimExprOpBin),
    OPRINIT("/", 110, 2, JimExprOpBin),
    OPRINIT("%", 110, 2, JimExprOpIntBin),

    OPRINIT("-", 100, 2, JimExprOpBin),
    OPRINIT("+", 100, 2, JimExprOpBin),

    OPRINIT("<<", 90, 2, JimExprOpIntBin),
    OPRINIT(">>", 90, 2, JimExprOpIntBin),

    OPRINIT("<<<", 90, 2, JimExprOpIntBin),
    OPRINIT(">>>", 90, 2, JimExprOpIntBin),

    OPRINIT("<", 80, 2, JimExprOpBin),
    OPRINIT(">", 80, 2, JimExprOpBin),
    OPRINIT("<=", 80, 2, JimExprOpBin),
    OPRINIT(">=", 80, 2, JimExprOpBin),

    OPRINIT("==", 70, 2, JimExprOpBin),
    OPRINIT("!=", 70, 2, JimExprOpBin),

    OPRINIT("&", 50, 2, JimExprOpIntBin),
    OPRINIT("^", 49, 2, JimExprOpIntBin),
    OPRINIT("|", 48, 2, JimExprOpIntBin),

    OPRINIT("&&", 10, 2, JimExprOpAnd),
    OPRINIT("||", 9, 2, JimExprOpOr),
    OPRINIT_ATTR("?", 5, 3, JimExprOpTernary, OP_RIGHT_ASSOC),
    OPRINIT_ATTR(":", 5, 3, NULL, OP_RIGHT_ASSOC),

    /* Precedence is higher than * and / but lower than ! and ~, and right-associative */
    OPRINIT_ATTR("**", 120, 2, JimExprOpBin, OP_RIGHT_ASSOC),

    OPRINIT("eq", 60, 2, JimExprOpStrBin),
    OPRINIT("ne", 60, 2, JimExprOpStrBin),

    OPRINIT("in", 55, 2, JimExprOpStrBin),
    OPRINIT("ni", 55, 2, JimExprOpStrBin),

    OPRINIT_ATTR("!", 150, 1, JimExprOpNumUnary, OP_RIGHT_ASSOC),
    OPRINIT_ATTR("~", 150, 1, JimExprOpIntUnary, OP_RIGHT_ASSOC),
    OPRINIT_ATTR(" -", 150, 1, JimExprOpNumUnary, OP_RIGHT_ASSOC),
    OPRINIT_ATTR(" +", 150, 1, JimExprOpNumUnary, OP_RIGHT_ASSOC),



    OPRINIT_ATTR("int", 200, 1, JimExprOpNumUnary, OP_FUNC),
    OPRINIT_ATTR("wide", 200, 1, JimExprOpNumUnary, OP_FUNC),
    OPRINIT_ATTR("abs", 200, 1, JimExprOpNumUnary, OP_FUNC),
    OPRINIT_ATTR("double", 200, 1, JimExprOpNumUnary, OP_FUNC),
    OPRINIT_ATTR("round", 200, 1, JimExprOpNumUnary, OP_FUNC),
    OPRINIT_ATTR("rand", 200, 0, JimExprOpNone, OP_FUNC),
    OPRINIT_ATTR("srand", 200, 1, JimExprOpIntUnary, OP_FUNC),

#ifdef JIM_MATH_FUNCTIONS // #optionalCode
    OPRINIT_ATTR("sin", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("cos", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("tan", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("asin", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("acos", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("atan", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("atan2", 200, 2, JimExprOpBin, OP_FUNC),
    OPRINIT_ATTR("sinh", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("cosh", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("tanh", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("ceil", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("floor", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("exp", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("log", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("log10", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("sqrt", 200, 1, JimExprOpDoubleUnary, OP_FUNC),
    OPRINIT_ATTR("pow", 200, 2, JimExprOpBin, OP_FUNC),
    OPRINIT_ATTR("hypot", 200, 2, JimExprOpBin, OP_FUNC),
    OPRINIT_ATTR("fmod", 200, 2, JimExprOpBin, OP_FUNC),
#endif
};
#undef OPRINIT
#undef OPRINIT_ATTR

enum { JIM_EXPR_OPERATORS_NUM =
    (sizeof(g_Jim_ExprOperators)/sizeof(Jim_ExprOperator))
};

STATIC Retval JimParseExpression(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    /* Discard spaces and quoted newline */
    while (isspace(UCHAR(*pc->p)) || (*(pc->p) == '\\' && *(pc->p + 1) == '\n')) {
        if (*pc->p == '\n') {
            pc->linenr++;
        }
        pc->p++;
        pc->len--;
    }

    /* Common case */
    pc->tline = pc->linenr;
    pc->tstart = pc->p;

    if (pc->len == 0) {
        pc->tend = pc->p;
        pc->tt = JIM_TT_EOL;
        pc->eof = 1;
        return JIM_OK;
    }
    switch (*(pc->p)) {
        case '(':
                pc->tt = JIM_TT_SUBEXPR_START;
                goto singlechar;
        case ')':
                pc->tt = JIM_TT_SUBEXPR_END;
                goto singlechar;
        case ',':
            pc->tt = JIM_TT_SUBEXPR_COMMA;
singlechar:
            pc->tend = pc->p;
            pc->p++;
            pc->len--;
            break;
        case '[':
            return JimParseCmd(pc);
        case '$':
            if (JimParseVar(pc) == JIM_ERR)
                return JimParseExprOperator(pc); // #MissInCoverage
            else {
                /* Don't allow expr sugar in expressions */
                if (pc->tt == JIM_TT_EXPRSUGAR) {
                    return JIM_ERR;
                }
                return JIM_OK;
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
            return JimParseExprNumber(pc);
        case '"':
            return JimParseQuote(pc);
        case '{':
            return JimParseBrace(pc);

        case 'N':
        case 'I':
        case 'n':
        case 'i':
            if (JimParseExprIrrational(pc) == JIM_ERR)
                if (JimParseExprBoolean(pc) == JIM_ERR)
                    return JimParseExprOperator(pc);
            break;
        case 't':
        case 'f':
        case 'o':
        case 'y':
            if (JimParseExprBoolean(pc) == JIM_ERR)
                return JimParseExprOperator(pc);
            break;
        default:
            return JimParseExprOperator(pc);
            break;
    }
    return JIM_OK;
}

STATIC Retval JimParseExprNumber(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    char *end;

    /* Assume an integer for now */
    pc->tt = JIM_TT_EXPR_INT;

    jim_strtoull(pc->p, (char **)&pc->p);
    /* Tried as an integer, but perhaps it parses as a double */
    if (strchr("eENnIi.", *pc->p) || pc->p == pc->tstart) {
        /* Some stupid compilers insist they are cleverer that
         * we are. Even a (void) cast doesn't prevent this warning!
         */
        if (strtod(pc->tstart, &end)) { /* nothing */ }
        if (end == pc->tstart)
            return JIM_ERR; // #MissInCoverage
        if (end > pc->p) {
            /* Yes, double captured more chars */
            pc->tt = JIM_TT_EXPR_DOUBLE;
            pc->p = end;
        }
    }
    pc->tend = pc->p - 1;
    pc->len -= (int)(pc->p - pc->tstart);
    return JIM_OK;
}

STATIC Retval JimParseExprIrrational(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    const char *irrationals[] = { "NaN", "nan", "NAN", "Inf", "inf", "INF", NULL };
    int i;

    for (i = 0; irrationals[i]; i++) {
        const char *irr = irrationals[i];

        if (strncmp(irr, pc->p, 3) == 0) {
            pc->p += 3; // #MissInCoverage #MagicNum
            pc->len -= 3;
            pc->tend = pc->p - 1;
            pc->tt = JIM_TT_EXPR_DOUBLE;
            return JIM_OK;
        }
    }
    return JIM_ERR;
}

STATIC Retval JimParseExprBoolean(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    const char *booleans[] = { "false", "no", "off", "true", "yes", "on", NULL };
    const int lengths[] = { 5, 2, 3, 4, 3, 2, 0 }; // #MagicNum
    int i;

    for (i = 0; booleans[i]; i++) {
        const char *boolean = booleans[i];
        int length = lengths[i];

        if (strncmp(boolean, pc->p, length) == 0) {
            pc->p += length;
            pc->len -= length;
            pc->tend = pc->p - 1;
            pc->tt = JIM_TT_EXPR_BOOLEAN;
            return JIM_OK;
        }
    }
    return JIM_ERR;
}

static const_Jim_ExprOperatorPtr  JimExprOperatorInfoByOpcode(int opcode) // #JimExpr
{
    PRJ_TRACE;
    static Jim_ExprOperator dummy_op;
    if (opcode < JIM_TT_EXPR_OP) {
        return &dummy_op; // #MissInCoverage
    }
    return (const_Jim_ExprOperatorPtr)&g_Jim_ExprOperators[opcode - JIM_TT_EXPR_OP];
}

STATIC Retval JimParseExprOperator(JimParserCtxPtr pc)
{
    PRJ_TRACE;
    int i;
    const_Jim_ExprOperatorPtr  bestOp = NULL;
    int bestLen = 0;

    /* Try to get the longest match. */
    for (i = 0; i < (signed)JIM_EXPR_OPERATORS_NUM; i++) {
        const_Jim_ExprOperatorPtr  op = (const_Jim_ExprOperatorPtr)&g_Jim_ExprOperators[i];

        if (op->name[0] != pc->p[0]) {
            continue;
        }

        if (op->namelen > bestLen && strncmp(op->name, pc->p, op->namelen) == 0) {
            bestOp = op;
            bestLen = op->namelen;
        }
    }
    if (bestOp == NULL) {
        return JIM_ERR;
    }

    /* Validate parentheses around function arguments */
    if (bestOp->attr & OP_FUNC) {
        const char *p = pc->p + bestLen;
        int len = pc->len - bestLen;

        while (len && isspace(UCHAR(*p))) {
            len--;
            p++;
        }
        if (*p != '(') {
            return JIM_ERR;
        }
    }
    pc->tend = pc->p + bestLen - 1;
    pc->p += bestLen;
    pc->len -= bestLen;

    pc->tt = (int)((bestOp - g_Jim_ExprOperators) + JIM_TT_EXPR_OP);
    return JIM_OK;
}

const char *jim_tt_name(int type) // #MissInCoverage
{
    PRJ_TRACE;
    static const char * const tt_names[JIM_TT_EXPR_OP] =
        { "NIL", "STR", "ESC", "VAR", "ARY", "CMD", "SEP", "EOL", "EOF", "LIN", "WRD", "(((", ")))", ",,,", "INT",
            "DBL", "BOO", "$()" };
    if (type < JIM_TT_EXPR_OP) {
        return tt_names[type];
    }
    else if (type == JIM_EXPROP_UNARYMINUS) {
        return "-VE";
    }
    else if (type == JIM_EXPROP_UNARYPLUS) {
        return "+VE";
    }
    else {
        const_Jim_ExprOperatorPtr  op = JimExprOperatorInfoByOpcode(type);
        static char buf[20]; // #MagicNum

        if (op->name) {
            return op->name;
        }
        sprintf(buf, "(%d)", type);
        return buf;
    }
}

/* -----------------------------------------------------------------------------
 * Expression Object
 * ---------------------------------------------------------------------------*/
static void FreeExprInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
static void DupExprInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
STATIC int SetExprFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

static const Jim_ObjType g_exprObjType = { // #JimType #JimExpr
    "expression",
    FreeExprInternalRepCB,
    DupExprInternalRepCB,
    NULL,
    JIM_TYPE_REFERENCES,
};
const Jim_ObjType& exprType() { return g_exprObjType; }

/* expr tree structure */
struct ExprTree
{
    JimExprNodePtr expr = NULL;   /* The first operator or term */
    JimExprNodePtr nodes = NULL;  /* Storage of all nodes in the tree */
    int len = 0;                /* Number of nodes in use */
    int inUse = 0;              /* Used for sharing. */
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_ExprTree                Jim_TAlloc<ExprTree>(1,"ExprTree")
#define free_ExprTree(ptr)          Jim_TFree<ExprTree>(ptr,"ExprTree")

STATIC void ExprTreeFreeNodes(Jim_InterpPtr interp, JimExprNodePtr nodes, int num) // #JimExpr
{
    PRJ_TRACE;
    int i;
    for (i = 0; i < num; i++) {
        if (nodes[i].objPtr) {
            Jim_DecrRefCount(interp, nodes[i].objPtr);
        }
    }
    free_JimExprNode(nodes); // #FreeF 
}

static void ExprTreeFree(Jim_InterpPtr interp, ExprTreePtr expr) // #JimExpr
{
    PRJ_TRACE;
    ExprTreeFreeNodes(interp, expr->nodes, expr->len);
    free_ExprTree(expr); // #FreeF 
}

static void FreeExprInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimExpr
{
    PRJ_TRACE;
    ExprTreePtr expr = (ExprTreePtr )objPtr->getVoidPtr();

    if (expr) {
        if (--expr->inUse != 0) {
            return; // #MissInCoverage
        }

        ExprTreeFree(interp, expr);
    }
}

static void DupExprInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #MissInCoverage #JimExpr
{
    PRJ_TRACE;
    JIM_NOTUSED(interp);
    JIM_NOTUSED(srcPtr);

    /* Just returns an simple string. */
    dupPtr->setTypePtr(NULL);
}

struct ExprBuilder {
private:
    int parencount = 0;             /* count of outstanding parentheses */
    int level = 0;                  /* recursion depth */
    ParseTokenPtr token = NULL;       /* The current token */
    ParseTokenPtr first_token = NULL; /* The first token */
    Jim_Stack stack;                /* stack of pending terms */
    Jim_ObjPtr exprObjPtr = NULL;     /* the original expression */
    Jim_ObjPtr fileNameObj = NULL;    /* filename of the original expression */
    JimExprNodePtr nodes = NULL;      /* storage for all nodes */
    JimExprNodePtr next = NULL;       /* storage for the next node */
public:

    friend STATIC int ExprTreeBuildTree(Jim_InterpPtr interp, ExprBuilderPtr builder, int precedence, int flags, int exp_numterms);
    friend STATIC int ExprTreeBuildTree(Jim_InterpPtr interp, ExprBuilderPtr builder, int precedence, int flags, int exp_numterms);
    friend STATIC ExprTreePtr ExprTreeCreateTree(Jim_InterpPtr interp, const ParseTokenListPtr tokenlist, Jim_ObjPtr exprObjPtr, Jim_ObjPtr fileNameObj);
};

STATIC void JimShowExprNode(JimExprNodePtr node, int level) // #MissInCoverage #JimExpr
{
    PRJ_TRACE;
    int i;
    for (i = 0; i < level; i++) {
        printf("  "); // #stdoutput
    }
    if (TOKEN_IS_EXPR_OP(node->type)) {
        printf("%s\n", jim_tt_name(node->type)); // #stdoutput
        if (node->left) {
            JimShowExprNode(node->left, level + 1);
        }
        if (node->right) {
            JimShowExprNode(node->right, level + 1);
        }
        if (node->ternary) {
            JimShowExprNode(node->ternary, level + 1);
        }
    }
    else {
        printf("[%s] %s\n", jim_tt_name(node->type), Jim_String(node->objPtr)); // #stdoutput
    }
}

enum EXPR_STATE {
    EXPR_UNTIL_CLOSE = 0x0001,
    EXPR_FUNC_ARGS   = 0x0002,
    EXPR_TERNARY     = 0x0004
};

/**
 * Parse the subexpression at builder->token and return with the node on the stack.
 * builder->token is advanced to the next unconsumed token.
 * Returns JIM_OK if OK or JIM_ERR on error and leaves a message in the interpreter result.
 *
 * 'precedence' is the precedence of the current operator. Tokens are consumed until an operator
 * with an equal or lower precedence is reached (or strictly lower if right associative).
 *
 * If EXPR_UNTIL_CLOSE is set, the subexpression extends up to and including the next close parenthesis.
 * If EXPR_FUNC_ARGS is set, multiple subexpressions (terms) are expected separated by comma
 * If EXPR_TERNARY is set, two subexpressions (terms) are expected separated by colon
 *
 * 'exp_numterms' indicates how many terms are expected. Normally this is 1, but may be more for EXPR_FUNC_ARGS and EXPR_TERNARY.
 */
STATIC Retval ExprTreeBuildTree(Jim_InterpPtr interp, ExprBuilderPtr builder, int precedence, int flags, int exp_numterms) // #JimExpr
{
    PRJ_TRACE;
    int rc;
    JimExprNodePtr node;
    /* Calculate the stack length expected after pushing the number of expected terms */
    int exp_stacklen = builder->stack.len() + exp_numterms;

    if (builder->level++ > 200) {
        Jim_SetResultString(interp, "Expression too complex", -1); // #MissInCoverage
        return JIM_ERR;
    }

    while (builder->token->type != JIM_TT_EOL) {
        ParseTokenPtr t = builder->token++;
        int prevtt;

        if (t == builder->first_token) {
            prevtt = JIM_TT_NONE;
        }
        else {
            prevtt = t[-1].type;
        }

        if (t->type == JIM_TT_SUBEXPR_START) {
            if (builder->stack.len() == exp_stacklen) {
                Jim_SetResultFormatted(interp, "unexpected open parenthesis in expression: \"%#s\"", builder->exprObjPtr); // #MissInCoverage
                return JIM_ERR;
            }
            builder->parencount++;
            rc = ExprTreeBuildTree(interp, builder, 0, EXPR_UNTIL_CLOSE, 1);
            if (rc != JIM_OK) {
                return rc;
            }
            /* A complete subexpression is on the stack */
        }
        else if (t->type == JIM_TT_SUBEXPR_END) {
            if (!(flags & EXPR_UNTIL_CLOSE)) {
                if (builder->stack.len() == exp_stacklen && builder->level > 1) {
                    builder->token--;
                    builder->level--;
                    return JIM_OK;
                }
                Jim_SetResultFormatted(interp, "unexpected closing parenthesis in expression: \"%#s\"", builder->exprObjPtr);
                return JIM_ERR;
            }
            builder->parencount--;
            if (builder->stack.len() == exp_stacklen) {
                /* Return with the expected number of subexpressions on the stack */
                break;
            }
        }
        else if (t->type == JIM_TT_SUBEXPR_COMMA) {
            if (!(flags & EXPR_FUNC_ARGS)) {
                if (builder->stack.len() == exp_stacklen) {
                    /* handle the comma back at the parent level */
                    builder->token--;
                    builder->level--;
                    return JIM_OK;
                }
                Jim_SetResultFormatted(interp, "unexpected comma in expression: \"%#s\"", builder->exprObjPtr); // #MissInCoverage
                return JIM_ERR;
            }
            else {
                /* If we see more terms than expected, it is an error */
                if (builder->stack.len() > exp_stacklen) {
                    Jim_SetResultFormatted(interp, "too many arguments to math function"); // #MissInCoverage
                    return JIM_ERR;
                }
            }
            /* just go onto the next arg */
        }
        else if (t->type == JIM_EXPROP_COLON) {
            if (!(flags & EXPR_TERNARY)) {
                if (builder->level != 1) {
                    /* handle the comma back at the parent level */
                    builder->token--;
                    builder->level--;
                    return JIM_OK;
                }
                Jim_SetResultFormatted(interp, ": without ? in expression: \"%#s\"", builder->exprObjPtr);
                return JIM_ERR;
            }
            if (builder->stack.len() == exp_stacklen) {
                /* handle the comma back at the parent level */
                builder->token--;
                builder->level--;
                return JIM_OK;
            }
            /* just go onto the next term */
        }
        else if (TOKEN_IS_EXPR_OP(t->type)) {
            const_Jim_ExprOperatorPtr  op;

            /* Convert -/+ to unary minus or unary plus if necessary */
            if (TOKEN_IS_EXPR_OP(prevtt) || TOKEN_IS_EXPR_START(prevtt)) {
                if (t->type == JIM_EXPROP_SUB) {
                    t->type = JIM_EXPROP_UNARYMINUS;
                }
                else if (t->type == JIM_EXPROP_ADD) {
                    t->type = JIM_EXPROP_UNARYPLUS;
                }
            }

            op = JimExprOperatorInfoByOpcode(t->type);

            if (op->precedence < precedence || (!(op->attr & OP_RIGHT_ASSOC) && op->precedence == precedence)) {
                /* next op is lower precedence, or equal and left associative, so done here */
                builder->token--;
                break;
            }

            if (op->attr & OP_FUNC) {
                if (builder->token->type != JIM_TT_SUBEXPR_START) {
                    Jim_SetResultString(interp, "missing arguments for math function", -1); // #MissInCoverage
                    return JIM_ERR;
                }
                builder->token++;
                if (op->arity == 0) {
                    if (builder->token->type != JIM_TT_SUBEXPR_END) {
                        Jim_SetResultString(interp, "too many arguments for math function", -1);
                        return JIM_ERR;
                    }
                    builder->token++;
                    goto noargs;
                }
                builder->parencount++;

                /* This will push left and return right */
                rc = ExprTreeBuildTree(interp, builder, 0, EXPR_FUNC_ARGS | EXPR_UNTIL_CLOSE, op->arity);
            }
            else if (t->type == JIM_EXPROP_TERNARY) {
                /* Collect the two arguments to the ternary operator */
                rc = ExprTreeBuildTree(interp, builder, op->precedence, EXPR_TERNARY, 2);
            }
            else {
                /* Recursively handle everything on the right until we see a precendence <= op->precedence or == and right associative
                 * and push that on the term stack
                 */
                rc = ExprTreeBuildTree(interp, builder, op->precedence, 0, 1);
            }

            if (rc != JIM_OK) {
                return rc;
            }

noargs:
            node = builder->next++;
            node->type = t->type;

            if (op->arity >= 3) {
                node->ternary = (JimExprNodePtr )Jim_StackPop(&builder->stack);
                if (node->ternary == NULL) {
                    goto missingoperand; // #MissInCoverage
                }
            }
            if (op->arity >= 2) {
                node->right = (JimExprNodePtr )Jim_StackPop(&builder->stack);
                if (node->right == NULL) {
                    goto missingoperand; // #MissInCoverage
                }
            }
            if (op->arity >= 1) {
                node->left = (JimExprNodePtr )Jim_StackPop(&builder->stack);
                if (node->left == NULL) {
missingoperand: // #MissInCoverage
                    Jim_SetResultFormatted(interp, "missing operand to %s in expression: \"%#s\"", op->name, builder->exprObjPtr);
                    builder->next--;
                    return JIM_ERR;

                }
            }

            /* Now push the node */
            Jim_StackPush(&builder->stack, node);
        }
        else {
            Jim_ObjPtr objPtr = NULL;

            /* This is a simple non-operator term, so create and push the appropriate object */

            /* Two consecutive terms without an operator is invalid */
            if (!TOKEN_IS_EXPR_START(prevtt) && !TOKEN_IS_EXPR_OP(prevtt)) {
                Jim_SetResultFormatted(interp, "missing operator in expression: \"%#s\"", builder->exprObjPtr);
                return JIM_ERR;
            }

            /* Immediately create a double or int object? */
            if (t->type == JIM_TT_EXPR_INT || t->type == JIM_TT_EXPR_DOUBLE) {
                char *endptr;
                if (t->type == JIM_TT_EXPR_INT) {
                    objPtr = Jim_NewIntObj(interp, jim_strtoull(t->token, &endptr));
                }
                else {
                    objPtr = Jim_NewDoubleObj(interp, strtod(t->token, &endptr));
                }
                if (endptr != t->token + t->len) {
                    /* Conversion failed, so just store it as a string */
                    Jim_FreeNewObj(interp, objPtr); // #MissInCoverage
                    objPtr = NULL;
                }
            }

            if (!objPtr) {
                /* Everything else is stored a simple string term */
                objPtr = Jim_NewStringObj(interp, t->token, t->len);
                if (t->type == JIM_TT_CMD) {
                    /* Only commands need source info */
                    JimSetSourceInfo(interp, objPtr, builder->fileNameObj, t->line);
                }
            }

            /* Now push a term node */
            node = builder->next++;
            node->objPtr = objPtr;
            Jim_IncrRefCount(node->objPtr);
            node->type = t->type;
            Jim_StackPush(&builder->stack, node);
        }
    }

    if (builder->stack.len() == exp_stacklen) {
        builder->level--;
        return JIM_OK;
    }

    if ((flags & EXPR_FUNC_ARGS)) {
        Jim_SetResultFormatted(interp, "too %s arguments for math function", (builder->stack.len() < exp_stacklen) ? "few" : "many");
    }
    else {
        if (builder->stack.len() < exp_stacklen) {
            if (builder->level == 0) {
                Jim_SetResultFormatted(interp, "empty expression"); // #MissInCoverage
            }
            else {
                Jim_SetResultFormatted(interp, "syntax error in expression \"%#s\": premature end of expression", builder->exprObjPtr);
            }
        }
        else {
            Jim_SetResultFormatted(interp, "extra terms after expression"); // #MissInCoverage
        }
    }

    return JIM_ERR;
}

STATIC ExprTreePtr ExprTreeCreateTree(Jim_InterpPtr interp, const ParseTokenListPtr tokenlist, Jim_ObjPtr exprObjPtr, Jim_ObjPtr fileNameObj) // #JimExpr
{
    PRJ_TRACE;
    ExprTreePtr expr;
    ExprBuilder builder;
    int rc;
    JimExprNodePtr top = NULL;

    builder.parencount = 0;
    builder.level = 0;
    builder.token = builder.first_token = tokenlist->list;
    builder.exprObjPtr = exprObjPtr;
    builder.fileNameObj = fileNameObj;
    /* The bytecode will never produce more nodes than there are tokens - 1 (for EOL)*/
    builder.nodes = Jim_TAllocZ<JimExprNode>((tokenlist->count - 1),"JimExprNode"); // #AllocF 

    //memset(builder.nodes, 0, sizeof(JimExprNode) * (tokenlist->count - 1));
    builder.next = builder.nodes;
    Jim_InitStack(&builder.stack);

    rc = ExprTreeBuildTree(interp, &builder, 0, 0, 1);

    if (rc == JIM_OK) {
        top = (JimExprNodePtr )Jim_StackPop(&builder.stack);

        if (builder.parencount) {
            Jim_SetResultString(interp, "missing close parenthesis", -1);
            rc = JIM_ERR;
        }
    }

    /* Free the stack used for the compilation. */
    Jim_FreeStack(&builder.stack);

    if (rc != JIM_OK) {
        ExprTreeFreeNodes(interp, builder.nodes, 
                          (int)(builder.next - builder.nodes));
        return NULL;
    }

    expr = new_ExprTree; // #AllocF 
    expr->inUse = 1;
    expr->expr = top;
    expr->nodes = builder.nodes;
    expr->len = (int)(builder.next - builder.nodes);

    assert(expr->len <= tokenlist->count - 1);

    return expr;
}




/* This method takes the string representation of an expression
 * and generates a program for the expr engine */
STATIC int SetExprFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimExpr
{
    PRJ_TRACE;
    int exprTextLen;
    const char *exprText;
    JimParserCtx parser;
    ExprTreePtr expr;
    ParseTokenList tokenlist;
    int line;
    Jim_ObjPtr fileNameObj;
    int rc = JIM_ERR;

    /* Try to get information about filename / line number */
    if (objPtr->typePtr() == &g_sourceObjType) {
        fileNameObj = objPtr->get_sourceValue_fileName();
        line = objPtr->get_sourceValue_lineNum();
    }
    else {
        fileNameObj = interp->emptyObj();
        line = 1;
    }
    Jim_IncrRefCount(fileNameObj);

    exprText = Jim_GetString(objPtr, &exprTextLen);

    /* Initially tokenise the expression into tokenlist */
    ScriptTokenListInit(&tokenlist);

    JimParserInit(&parser, exprText, exprTextLen, line);
    while (!parser.eof) {
        if (JimParseExpression(&parser) != JIM_OK) {
            ScriptTokenListFree(&tokenlist);
            Jim_SetResultFormatted(interp, "syntax error in expression: \"%#s\"", objPtr);
            expr = NULL;
            goto err;
        }

        ScriptAddToken(&tokenlist, parser.tstart, 
            (int)(parser.tend - parser.tstart + 1), parser.tt,
            parser.tline);
    }

    if (g_DEBUG_SHOW_EXPR_TOKENS_VAL)
    {
        int i;
        printf("==== Expr Tokens (%s) ====\n", Jim_String(fileNameObj)); // #stdoutput #MissInCoverage
        for (i = 0; i < tokenlist.count; i++) {
            printf("[%2d]@%d %s '%.*s'\n", i, tokenlist.list[i].line, jim_tt_name(tokenlist.list[i].type),  // #stdoutput
                tokenlist.list[i].len, tokenlist.list[i].token);
        }
    }

    if (JimParseCheckMissing(interp, parser.missing.ch) == JIM_ERR) {
        ScriptTokenListFree(&tokenlist);
        Jim_DecrRefCount(interp, fileNameObj);
        return JIM_ERR;
    }

    /* Now create the expression bytecode from the tokenlist */
    expr = ExprTreeCreateTree(interp, &tokenlist, objPtr, fileNameObj);

    /* No longer need the token list */
    ScriptTokenListFree(&tokenlist);

    if (!expr) {
        goto err;
    }

    if (g_DEBUG_SHOW_EXPR) {
        printf("==== Expr ====\n"); // #stdoutput #MissInCoverage
        JimShowExprNode(expr->expr, 0);
    }

    rc = JIM_OK;
  err:
    /* Free the old internal rep and set the new one. */
    Jim_DecrRefCount(interp, fileNameObj);
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setPtr<ExprTreePtr>(expr);
    //Jim_SetIntRepPtr(objPtr, expr);
    objPtr->setTypePtr(&g_exprObjType);
    return rc;
}

static ExprTreePtr JimGetExpression(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimExpr
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_exprObjType) {
        if (SetExprFromAny(interp, objPtr) != JIM_OK) {
            return NULL;
        }
    }
    return (ExprTreePtr ) Jim_GetIntRepPtr(objPtr);
}

STATIC Jim_ObjPtr JimExprIntValOrVar(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    if (node->type == JIM_TT_EXPR_INT)
        return node->objPtr;
    else if (node->type == JIM_TT_VAR)
        return Jim_GetVariable(interp, node->objPtr, JIM_NONE);
    else if (node->type == JIM_TT_DICTSUGAR)
        return JimExpandDictSugar(interp, node->objPtr);
    else
        return NULL;
}

/* -----------------------------------------------------------------------------
 * Expressions evaluation.
 * Jim uses a recursive evaluation engine for expressions,
 * that takes advantage of the fact that expr's operators
 * can't be redefined.
 *
 * Jim_EvalExpression() uses the expression tree compiled by
 * SetExprFromAny() method of the "expression" object.
 *
 * On success a Tcl Object containing the result of the evaluation
 * is stored into expResultPtrPtr (having refcount of 1), and JIM_OK is
 * returned.
 * On error the function returns a retcode != to JIM_OK and set a suitable
 * error on the interp.
 * ---------------------------------------------------------------------------*/

STATIC Retval JimExprEvalTermNode(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    if (TOKEN_IS_EXPR_OP(node->type)) {
        const_Jim_ExprOperatorPtr  op = JimExprOperatorInfoByOpcode(node->type);
        return op->funcop(interp, node);
    }
    else {
        Jim_ObjPtr objPtr;

        /* A term */
        switch (node->type) {
            case JIM_TT_EXPR_INT:
            case JIM_TT_EXPR_DOUBLE:
            case JIM_TT_EXPR_BOOLEAN:
            case JIM_TT_STR:
                Jim_SetResult(interp, node->objPtr);
                return JIM_OK;

            case JIM_TT_VAR:
                objPtr = Jim_GetVariable(interp, node->objPtr, JIM_ERRMSG);
                if (objPtr) {
                    Jim_SetResult(interp, objPtr);
                    return JIM_OK;
                }
                return JIM_ERR;

            case JIM_TT_DICTSUGAR:
                objPtr = JimExpandDictSugar(interp, node->objPtr);
                if (objPtr) {
                    Jim_SetResult(interp, objPtr);
                    return JIM_OK;
                }
                return JIM_ERR;

            case JIM_TT_ESC:
                if (Jim_SubstObj(interp, node->objPtr, &objPtr, JIM_NONE) == JIM_OK) {
                    Jim_SetResult(interp, objPtr);
                    return JIM_OK;
                }
                return JIM_ERR;

            case JIM_TT_CMD:
                return Jim_EvalObj(interp, node->objPtr);

            default:
                /* Should never get here */
                return JIM_ERR; // #MissInCoverage
        }
    }
}

static Retval JimExprGetTerm(Jim_InterpPtr interp, JimExprNodePtr node, Jim_ObjArray* objPtrPtr) // #JimExpr
{
    PRJ_TRACE;
    Retval rc = JimExprEvalTermNode(interp, node);
    if (rc == JIM_OK) {
        *objPtrPtr = Jim_GetResult(interp);
        Jim_IncrRefCount(*objPtrPtr);
    }
    return rc;
}

static int JimExprGetTermBoolean(Jim_InterpPtr interp, JimExprNodePtr node) // #JimExpr
{
    PRJ_TRACE;
    if (JimExprEvalTermNode(interp, node) == JIM_OK) {
        return ExprBool(interp, Jim_GetResult(interp));
    }
    return -1;
}

JIM_EXPORT Retval Jim_EvalExpression(Jim_InterpPtr interp, Jim_ObjPtr exprObjPtr) // #JimExpr
{
    PRJ_TRACE;
    PRJ_TRACE_GEN(::prj_trace::ACTION_EXPR_PRE, __FUNCTION__, exprObjPtr, NULL);
    ExprTreePtr expr;
    Retval retcode = JIM_OK;

    expr = JimGetExpression(interp, exprObjPtr);
    if (!expr) {
        return JIM_ERR;         /* error in expression. */
    }

    if (g_JIM_OPTIMIZATION_VAL) {
    /* Check for one of the following common expressions used by while/for
     *
     *   CONST
     *   $a
     *   !$a
     *   $a < CONST, $a < $b
     *   $a <= CONST, $a <= $b
     *   $a > CONST, $a > $b
     *   $a >= CONST, $a >= $b
     *   $a != CONST, $a != $b
     *   $a == CONST, $a == $b
     */
    {
        Jim_ObjPtr objPtr;

        /* STEP 1 -- Check if there are the conditions to run the specialized
         * version of while */

        switch (expr->len) {
            case 1:
                objPtr = JimExprIntValOrVar(interp, expr->expr);
                if (objPtr) {
                    Jim_SetResult(interp, objPtr);
                    return JIM_OK;
                }
                break;

            case 2:
                if (expr->expr->type == JIM_EXPROP_NOT) {
                    objPtr = JimExprIntValOrVar(interp, expr->expr->left);

                    if (objPtr && JimIsWide(objPtr)) {
                        Jim_SetResult(interp, JimWideValue(objPtr) ? interp->falseObj() : interp->trueObj());
                        return JIM_OK;
                    }
                }
                break;

            case 3:
                objPtr = JimExprIntValOrVar(interp, expr->expr->left);
                if (objPtr && JimIsWide(objPtr)) {
                    Jim_ObjPtr objPtr2 = JimExprIntValOrVar(interp, expr->expr->right);
                    if (objPtr2 && JimIsWide(objPtr2)) {
                        jim_wide wideValueA = JimWideValue(objPtr);
                        jim_wide wideValueB = JimWideValue(objPtr2);
                        int cmpRes;
                        switch (expr->expr->type) {
                            case JIM_EXPROP_LT:
                                cmpRes = wideValueA < wideValueB;
                                break;
                            case JIM_EXPROP_LTE:
                                cmpRes = wideValueA <= wideValueB;
                                break;
                            case JIM_EXPROP_GT:
                                cmpRes = wideValueA > wideValueB;
                                break;
                            case JIM_EXPROP_GTE:
                                cmpRes = wideValueA >= wideValueB;
                                break;
                            case JIM_EXPROP_NUMEQ:
                                cmpRes = wideValueA == wideValueB;
                                break;
                            case JIM_EXPROP_NUMNE:
                                cmpRes = wideValueA != wideValueB;
                                break;
                            default:
                                goto noopt;
                        }
                        Jim_SetResult(interp, cmpRes ? interp->trueObj() : interp->falseObj());
                        return JIM_OK;
                    }
                }
                break;
        }
    }
noopt: ;
    }

    /* In order to avoid the internal repr being freed due to
     * shimmering of the exprObjPtr's object, we make the internal rep
     * shared. */
    expr->inUse++;

    /* Evaluate with the recursive expr engine */
    retcode = JimExprEvalTermNode(interp, expr->expr);

    expr->inUse--;
    PRJ_TRACE_GEN(::prj_trace::ACTION_EXPR_POST, __FUNCTION__, exprObjPtr, NULL);

    return retcode;
}

JIM_EXPORT Retval Jim_GetBoolFromExpr(Jim_InterpPtr interp, Jim_ObjPtr exprObjPtr, int *boolPtr) // #JimExpr
{
    PRJ_TRACE;
    Retval retcode = Jim_EvalExpression(interp, exprObjPtr);

    if (retcode == JIM_OK) {
        switch (ExprBool(interp, Jim_GetResult(interp))) {
            case 0:
                *boolPtr = 0;
                break;

            case 1:
                *boolPtr = 1;
                break;

            case -1:
                retcode = JIM_ERR;
                break;
        }
    }
    return retcode;
}

/* -----------------------------------------------------------------------------
 * ScanFormat String Object
 * ---------------------------------------------------------------------------*/

/* This Jim_Obj will held a parsed representation of a format string passed to
 * the Jim_ScanString command. For error diagnostics, the scanformat string has
 * to be parsed in its entirely first and then, if correct, can be used for
 * scanning. To avoid endless re-parsing, the parsed representation will be
 * stored in an internal representation and re-used for performance reason. */

/* A ScanFmtPartDescr will held the information of /one/ part of the whole
 * scanformat string. This part will later be used to extract information
 * out from the string to be parsed by Jim_ScanString */

struct ScanFmtPartDescr
{
private:
    const char *arg = NULL;      /* Specification of a CHARSET conversion */
    const char *prefix = NULL;   /* Prefix to be scanned literally before conversion */
    size_t width = 0;            /* Maximal width of input to be converted */
    int pos = 0;                 /* -1 - no assign, 0 - natural pos, >0 - XPG3 pos */
    char type = 0;               /* Type of conversion (e.g. c, d, f) */
    char modifier = 0;           /* Modify type (e.g. l - long, h - short */
public:

    friend STATIC int SetScanFmtFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC int ScanOneEntry(Jim_InterpPtr interp, const char *str, int pos, int strLen,
                            ScanFmtStringObjPtr  fmtObj, long idx, Jim_ObjArray* valObjPtr);
    friend Jim_ObjPtr Jim_ScanString(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr fmtObjPtr, int flags);
};

/* The ScanFmtStringObj will hold the internal representation of a scanformat
 * string parsed and separated in part descriptions. Furthermore it contains
 * the original string representation of the scanformat string to allow for
 * fast update of the Jim_Obj's string representation part.
 *
 * As an add-on the internal object representation adds some scratch pad area
 * for usage by Jim_ScanString to avoid endless allocating and freeing of
 * memory for purpose of string scanning.
 *
 * The error member points to a static allocated string in case of a mal-
 * formed scanformat string or it contains '0' (NULL) in case of a valid
 * parse representation.
 *
 * The whole memory of the internal representation is allocated as a single
 * area of memory that will be internally separated. So freeing and duplicating
 * of such an object is cheap */

struct ScanFmtStringObj
{
private:
    jim_wide size = 0;         /* Size of internal repr in bytes */
    char *stringRep = NULL;     /* Original string representation */
    size_t count = 0;           /* Number of ScanFmtPartDescr contained */
    size_t convCount = 0;       /* Number of conversions that will assign */
    size_t maxPos = 0;          /* Max position index if XPG3 is used */
    const char *error = NULL;   /* Ptr to error text (NULL if no error */
    char *scratch = NULL;       /* Some scratch pad used by Jim_ScanString */
    ScanFmtPartDescr descr[1];  /* The vector of partial descriptions */
public:

    friend STATIC void DupScanFmtInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
    friend STATIC int SetScanFmtFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend STATIC void UpdateStringOfScanFmtCB(Jim_ObjPtr objPtr);
    friend STATIC size_t FormatGetCnvCount(Jim_ObjPtr  _fo_);
    friend STATIC size_t FormatGetMaxPos(Jim_ObjPtr  _fo_);
    friend STATIC const char *FormatGetError(Jim_ObjPtr  _fo_);
    friend STATIC int ScanOneEntry(Jim_InterpPtr interp, const char *str, int pos, int strLen,
                            ScanFmtStringObjPtr  fmtObj, long idx, Jim_ObjArray* valObjPtr);
    friend Jim_ObjPtr Jim_ScanString(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr fmtObjPtr, int flags);
};


static void FreeScanFmtInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
STATIC void DupScanFmtInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
STATIC void UpdateStringOfScanFmtCB(Jim_ObjPtr objPtr);

static const Jim_ObjType g_scanFmtStringObjType = { // #JimType #JimFmt
    "scanformatstring",
    FreeScanFmtInternalRepCB,
    DupScanFmtInternalRepCB,
    UpdateStringOfScanFmtCB,
    JIM_TYPE_NONE,
};
const Jim_ObjType& scanFmtStringType() { return g_scanFmtStringObjType; }

static void FreeScanFmtInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimFmt
{
    PRJ_TRACE;
    JIM_NOTUSED(interp);
    Jim_TFreeNR<void>(objPtr->getVoidPtr(),"void"); // #FreeF 
    objPtr->setPtr<void*>( 0);
}

STATIC void DupScanFmtInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr) // #MissInCoverage #JimFmt
{
    PRJ_TRACE;
    size_t size = (size_t) ((ScanFmtStringObjPtr ) srcPtr->getVoidPtr())->size;
    ScanFmtStringObjPtr  newVec = (ScanFmtStringObjPtr ) new_CharArray((int) size); // #AllocF 

    JIM_NOTUSED(interp);
    memcpy(newVec, srcPtr->getVoidPtr(), size);
    dupPtr->setPtr<ScanFmtStringObjPtr>( newVec);
    dupPtr->setTypePtr(&g_scanFmtStringObjType);
}

STATIC void UpdateStringOfScanFmtCB(Jim_ObjPtr objPtr) // #MissInCoverage #JimFmt
{
    PRJ_TRACE;
    JimSetStringBytes(objPtr, ((ScanFmtStringObjPtr ) objPtr->getVoidPtr())->stringRep);
}

/* SetScanFmtFromAny will parse a given string and create the internal
 * representation of the format specification. In case of an error
 * the error data member of the internal representation will be set
 * to an descriptive error text and the function will be left with
 * JIM_ERR to indicate unsuccessful parsing (aka. malformed scanformat
 * specification */

STATIC Retval SetScanFmtFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr) // #JimFmt
{
    PRJ_TRACE;
    ScanFmtStringObjPtr fmtObj;
    char *buffer;
    int maxCount, i, approxSize, lastPos = -1;
    const char *fmt = Jim_String(objPtr);
    int maxFmtLen = Jim_Length(objPtr);
    const char *fmtEnd = fmt + maxFmtLen;
    int curr;

    Jim_FreeIntRep(interp, objPtr);
    /* Count how many conversions could take place maximally */
    for (i = 0, maxCount = 0; i < maxFmtLen; ++i)
        if (fmt[i] == '%')
            ++maxCount;
    /* Calculate an approximation of the memory necessary */
    approxSize = sizeof(ScanFmtStringObj)       /* Size of the container */
        +(maxCount + 1) * sizeof(ScanFmtPartDescr)      /* Size of all partials */
        +maxFmtLen * sizeof(char) + 3 + 1       /* Scratch + "%n" + '\0' #MagicNum */
        + maxFmtLen * sizeof(char) + 1  /* Original stringrep */
        + maxFmtLen * sizeof(char)      /* Arg for CHARSETs */
        +(maxCount + 1) * sizeof(char)  /* '\0' for every partial */
        +1;                     /* safety byte */
    fmtObj = (ScanFmtStringObjPtr ) new_CharArrayZ(approxSize); // #AllocF 
    //memset(fmtObj, 0, approxSize);
    fmtObj->size = approxSize;
    fmtObj->maxPos = 0;
    fmtObj->scratch = (char *)&fmtObj->descr[maxCount + 1];
    fmtObj->stringRep = fmtObj->scratch + maxFmtLen + 3 + 1; // #MagicNum
    memcpy(fmtObj->stringRep, fmt, maxFmtLen);
    buffer = fmtObj->stringRep + maxFmtLen + 1;
    objPtr->setPtr<ScanFmtStringObjPtr>( fmtObj);
    objPtr->setTypePtr(&g_scanFmtStringObjType);
    for (i = 0, curr = 0; fmt < fmtEnd; ++fmt) {
        int width = 0, skip;
        ScanFmtPartDescrPtr descr = &fmtObj->descr[curr];

        fmtObj->count++;
        descr->width = 0;       /* Assume width unspecified */
        /* Overread and store any "literal" prefix */
        if (*fmt != '%' || fmt[1] == '%') {
            descr->type = 0;
            descr->prefix = &buffer[i];
            for (; fmt < fmtEnd; ++fmt) {
                if (*fmt == '%') {
                    if (fmt[1] != '%')
                        break;
                    ++fmt;
                }
                buffer[i++] = *fmt;
            }
            buffer[i++] = 0;
        }
        /* Skip the conversion introducing '%' sign */
        ++fmt;
        /* End reached due to non-conversion literal only? */
        if (fmt >= fmtEnd)
            goto done;
        descr->pos = 0;         /* Assume "natural" positioning */
        if (*fmt == '*') {
            descr->pos = -1;    /* Okay, conversion will not be assigned */
            ++fmt;
        }
        else
            fmtObj->convCount++;        /* Otherwise count as assign-conversion */
        /* Check if next token is a number (could be width or pos */
        if (sscanf(fmt, "%d%n", &width, &skip) == 1) {
            fmt += skip;
            /* Was the number a XPG3 position specifier? */
            if (descr->pos != -1 && *fmt == '$') {
                int prev;

                ++fmt;
                descr->pos = width;
                width = 0;
                /* Look if "natural" postioning and XPG3 one was mixed */
                if ((lastPos == 0 && descr->pos > 0)
                    || (lastPos > 0 && descr->pos == 0)) {
                    fmtObj->error = "cannot mix \"%\" and \"%n$\" conversion specifiers";
                    return JIM_ERR;
                }
                /* Look if this position was already used */
                for (prev = 0; prev < curr; ++prev) {
                    if (fmtObj->descr[prev].pos == -1)
                        continue; // #MissInCoverage
                    if (fmtObj->descr[prev].pos == descr->pos) {
                        fmtObj->error =
                            "variable is assigned by multiple \"%n$\" conversion specifiers";
                        return JIM_ERR;
                    }
                }
                if (descr->pos < 0) {
                    fmtObj->error = 
                        "\"%n$\" conversion specifier is negative"; // #MissInCoverage
                    return JIM_ERR;
                }
                /* Try to find a width after the XPG3 specifier */
                if (sscanf(fmt, "%d%n", &width, &skip) == 1) {
                    descr->width = width; // #MissInCoverage
                    fmt += skip;
                }
                if (descr->pos > 0 && (size_t) descr->pos > fmtObj->maxPos)
                    fmtObj->maxPos = descr->pos;
            }
            else {
                /* Number was not a XPG3, so it has to be a width */
                descr->width = width;
            }
        }
        /* If positioning mode was undetermined yet, fix this */
        if (lastPos == -1)
            lastPos = descr->pos;
        /* Handle CHARSET conversion type ... */
        if (*fmt == '[') {
            int swapped = 1, beg = i, end, j;

            descr->type = '[';
            descr->arg = &buffer[i];
            ++fmt;
            if (*fmt == '^')
                buffer[i++] = *fmt++;
            if (*fmt == ']')
                buffer[i++] = *fmt++;
            while (*fmt && *fmt != ']')
                buffer[i++] = *fmt++;
            if (*fmt != ']') {
                fmtObj->error = "unmatched [ in format string";
                return JIM_ERR;
            }
            end = i;
            buffer[i++] = 0;
            /* In case a range fence was given "backwards", swap it */
            while (swapped) {
                swapped = 0;
                for (j = beg + 1; j < end - 1; ++j) {
                    if (buffer[j] == '-' && buffer[j - 1] > buffer[j + 1]) {
                        char tmp = buffer[j - 1];

                        buffer[j - 1] = buffer[j + 1];
                        buffer[j + 1] = tmp;
                        swapped = 1;
                    }
                }
            }
        }
        else {
            /* Remember any valid modifier if given */
            if (fmt < fmtEnd && strchr("hlL", *fmt))
                descr->modifier = tolower((int)*fmt++);

            if (fmt >= fmtEnd) {
                fmtObj->error = "missing scan conversion character"; // #MissInCoverage
                return JIM_ERR;
            }

            descr->type = *fmt;
            if (strchr("efgcsndoxui", *fmt) == 0) {
                fmtObj->error = "bad scan conversion character";
                return JIM_ERR;
            }
            else if (*fmt == 'c' && descr->width != 0) {
                fmtObj->error = "field width may not be specified in %c " "conversion";
                return JIM_ERR;
            }
            else if (*fmt == 'u' && descr->modifier == 'l') {
                fmtObj->error = "unsigned wide not supported"; // #MissInCoverage
                return JIM_ERR;
            }
        }
        curr++;
    }
  done:
    return JIM_OK;
}

/* Some accessor macros to allow lowlevel access to fields of internal repr */
STATIC inline size_t FormatGetCnvCount(Jim_ObjPtr  _fo_) { return ((ScanFmtStringObjPtr )((_fo_)->getVoidPtr()))->convCount; }
STATIC inline size_t FormatGetMaxPos(Jim_ObjPtr  _fo_) { return ((ScanFmtStringObjPtr )((_fo_)->getVoidPtr()))->maxPos; }
STATIC inline const char *FormatGetError(Jim_ObjPtr  _fo_) { return ((ScanFmtStringObjPtr )((_fo_)->getVoidPtr()))->error; }

/* JimScanAString is used to scan an unspecified string that ends with
 * next WS, or a string that is specified via a charset.
 *
 */
static Jim_ObjPtr JimScanAString(Jim_InterpPtr interp, const char *sdescr, const char *str) // #JimFmt
{
    PRJ_TRACE;
    char *buffer = Jim_StrDup(str);
    char *p = buffer;

    while (*str) {
        int c;
        int n;

        if (!sdescr && isspace(UCHAR(*str)))
            break;              /* EOS via WS if unspecified */

        n = utf8_tounicode(str, &c);
        if (sdescr && !JimCharsetMatch(sdescr, c, JIM_CHARSET_SCAN))
            break;
        while (n--)
            *p++ = *str++;
    }
    *p = 0;
    return Jim_NewStringObjNoAlloc(interp, buffer, (int)(p - buffer));
}

/* ScanOneEntry will scan one entry out of the string passed as argument.
 * It use the sscanf() function for this task. After extracting and
 * converting of the value, the count of scanned characters will be
 * returned of -1 in case of no conversion tool place and string was
 * already scanned thru */

STATIC int ScanOneEntry(Jim_InterpPtr interp, const char *str, int pos, int strLen, // #JimFmt
    ScanFmtStringObjPtr  fmtObj, long idx, Jim_ObjArray* valObjPtr)
{
    PRJ_TRACE;
    const char *tok;
    const ScanFmtPartDescrPtr descr = &fmtObj->descr[idx];
    size_t scanned = 0;
    size_t anchor = pos;
    int i;
    Jim_ObjPtr tmpObj = NULL;

    /* First pessimistically assume, we will not scan anything :-) */
    *valObjPtr = 0;
    if (descr->prefix) {
        /* There was a prefix given before the conversion, skip it and adjust
         * the string-to-be-parsed accordingly */
        for (i = 0; pos < strLen && descr->prefix[i]; ++i) {
            /* If prefix require, skip WS */
            if (isspace(UCHAR(descr->prefix[i])))
                while (pos < strLen && isspace(UCHAR(str[pos])))
                    ++pos;
            else if (descr->prefix[i] != str[pos])
                break;          /* Prefix do not match here, leave the loop */
            else
                ++pos;          /* Prefix matched so far, next round */
        }
        if (pos >= strLen) {
            return -1;          /* All of str consumed: EOF condition */
        }
        else if (descr->prefix[i] != 0)
            return 0;           /* Not whole prefix consumed, no conversion possible */
    }
    /* For all but following conversion, skip leading WS */
    if (descr->type != 'c' && descr->type != '[' && descr->type != 'n')
        while (isspace(UCHAR(str[pos])))
            ++pos;
    /* Determine how much skipped/scanned so far */
    scanned = pos - anchor;

    /* %c is a special, simple case. no width */
    if (descr->type == 'n') {
        /* Return pseudo conversion means: how much scanned so far? */
        *valObjPtr = Jim_NewIntObj(interp, anchor + scanned);
    }
    else if (pos >= strLen) {
        /* Cannot scan anything, as str is totally consumed */
        return -1;
    }
    else if (descr->type == 'c') {
            int c;
            scanned += utf8_tounicode(&str[pos], &c);
            *valObjPtr = Jim_NewIntObj(interp, c);
            return (int)scanned;
    }
    else {
        /* Processing of conversions follows ... */
        if (descr->width > 0) {
            /* Do not try to scan as fas as possible but only the given width.
             * To ensure this, we copy the part that should be scanned. */
            size_t sLen = utf8_strlen(&str[pos], strLen - pos);
            size_t tLen = descr->width > sLen ? sLen : descr->width;

            tmpObj = Jim_NewStringObjUtf8(interp, str + pos, (int)tLen);
            tok = tmpObj->bytes();
        }
        else {
            /* As no width was given, simply refer to the original string */
            tok = &str[pos];
        }
        switch (descr->type) {
            case 'd':
            case 'o':
            case 'x':
            case 'u':
            case 'i':{
                    char *endp; /* Position where the number finished */
                    jim_wide w;

                    int base = descr->type == 'o' ? 8
                        : descr->type == 'x' ? 16 : descr->type == 'i' ? 0 : 10;

                    /* Try to scan a number with the given base */
                    if (base == 0) {
                        w = jim_strtoull(tok, &endp);
                    }
                    else {
                        w = strtoull(tok, &endp, base);
                    }

                    if (endp != tok) {
                        /* There was some number successfully scanned! */
                        *valObjPtr = Jim_NewIntObj(interp, w);

                        /* Adjust the number-of-chars scanned so far */
                        scanned += endp - tok;
                    }
                    else {
                        /* Nothing was scanned. We have to determine if this
                         * happened due to e.g. prefix mismatch or input str
                         * exhausted */
                        scanned = *tok ? 0 : -1;
                    }
                    break;
                }
            case 's':
            case '[':{
                    *valObjPtr = JimScanAString(interp, descr->arg, tok);
                    scanned += Jim_Length(*valObjPtr);
                    break;
                }
            case 'e':
            case 'f':
            case 'g':{
                    char *endp;
                    double value = strtod(tok, &endp);

                    if (endp != tok) {
                        /* There was some number successfully scanned! */
                        *valObjPtr = Jim_NewDoubleObj(interp, value);
                        /* Adjust the number-of-chars scanned so far */
                        scanned += endp - tok;
                    }
                    else {
                        /* Nothing was scanned. We have to determine if this
                         * happened due to e.g. prefix mismatch or input str
                         * exhausted */
                        scanned = *tok ? 0 : -1;
                    }
                    break;
                }
        }
        /* If a substring was allocated (due to pre-defined width) do not
         * forget to free it */
        if (tmpObj) {
            Jim_FreeNewObj(interp, tmpObj);
        }
    }
    return (int)scanned;
}

/* Jim_ScanString is the workhorse of string scanning. It will scan a given
 * string and returns all converted (and not ignored) values in a list back
 * to the caller. If an error occurred, a NULL pointer will be returned */

JIM_EXPORT Jim_ObjPtr Jim_ScanString(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr fmtObjPtr, int flags) // #JimFmt
{
    PRJ_TRACE;
    size_t i, pos;
    int scanned = 1;
    const char *str = Jim_String(strObjPtr);
    int strLen = Jim_Utf8Length(interp, strObjPtr);
    Jim_ObjPtr resultList = 0;
    Jim_ObjArray* resultVec = 0;
    int resultc;
    Jim_ObjPtr emptyStr = 0;
    ScanFmtStringObjPtr fmtObj;

    /* This should never happen. The format object should already be of the correct type */
    JimPanic((fmtObjPtr->typePtr != &g_scanFmtStringObjType, "Jim_ScanString() for non-scan format"));

    fmtObj = (ScanFmtStringObjPtr ) fmtObjPtr->getVoidPtr();
    /* Check if format specification was valid */
    if (fmtObj->error != 0) {
        if (flags & JIM_ERRMSG)
            Jim_SetResultString(interp, fmtObj->error, -1);
        return 0;
    }
    /* Allocate a new "shared" empty string for all unassigned conversions */
    emptyStr = Jim_NewEmptyStringObj(interp);
    Jim_IncrRefCount(emptyStr);
    /* Create a list and fill it with empty strings up to max specified XPG3 */
    resultList = Jim_NewListObj(interp, NULL, 0);
    if (fmtObj->maxPos > 0) {
        for (i = 0; i < fmtObj->maxPos; ++i)
            Jim_ListAppendElement(interp, resultList, emptyStr);
        JimListGetElements(interp, resultList, &resultc, &resultVec);
    }
    /* Now handle every partial format description */
    for (i = 0, pos = 0; i < fmtObj->count; ++i) {
        ScanFmtPartDescrPtr descr = &(fmtObj->descr[i]);
        Jim_ObjPtr value = 0;

        /* Only last type may be "literal" w/o conversion - skip it! */
        if (descr->type == 0)
            continue;
        /* As long as any conversion could be done, we will proceed */
        if (scanned > 0)
            scanned = ScanOneEntry(interp, str, (int)pos, strLen, fmtObj, (long)i, &value);
        /* In case our first try results in EOF, we will leave */
        if (scanned == -1 && i == 0)
            goto eof;
        /* Advance next pos-to-be-scanned for the amount scanned already */
        pos += scanned;

        /* value == 0 means no conversion took place so take empty string */
        if (value == 0)
            value = Jim_NewEmptyStringObj(interp);
        /* If value is a non-assignable one, skip it */
        if (descr->pos == -1) {
            Jim_FreeNewObj(interp, value);
        }
        else if (descr->pos == 0)
            /* Otherwise append it to the result list if no XPG3 was given */
            Jim_ListAppendElement(interp, resultList, value);
        else if (resultVec != NULL && resultVec[descr->pos - 1] == emptyStr) {
            /* But due to given XPG3, put the value into the corr. slot */
            Jim_DecrRefCount(interp, resultVec[descr->pos - 1]);
            Jim_IncrRefCount(value);
            resultVec[descr->pos - 1] = value;
        }
        else {
            /* Otherwise, the slot was already used - free obj and ERROR */
            Jim_FreeNewObj(interp, value); // #MissInCoverage
            goto err;
        }
    }
    Jim_DecrRefCount(interp, emptyStr);
    return resultList;
  eof:
    Jim_DecrRefCount(interp, emptyStr);
    Jim_FreeNewObj(interp, resultList);
    return (Jim_ObjPtr )EOF;
  err:
    Jim_DecrRefCount(interp, emptyStr);  // #MissInCoverage
    Jim_FreeNewObj(interp, resultList);
    return 0;
}

/* -----------------------------------------------------------------------------
 * Pseudo Random Number Generation
 * ---------------------------------------------------------------------------*/
/* Initialize the sbox with the numbers from 0 to 255 */
static void JimPrngInit(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    enum { PRNG_SEED_SIZE = 256 };
    int i;
    unsigned_int *seed;
    time_t t = time(NULL);

    interp->prngStateAlloc(); // #AllocF 

    seed = Jim_TAlloc<unsigned_int>(PRNG_SEED_SIZE,"unsigned_int"); // #AllocF 
    for (i = 0; i < PRNG_SEED_SIZE; i++) {
        seed[i] = (unsigned_int)(rand() ^ t ^ clock()); // #NonPortFunc
    }
    JimPrngSeed(interp, (unsigned_char *)seed, PRNG_SEED_SIZE * sizeof(*seed));
    Jim_TFree<unsigned_int>(seed,"unsigned_int"); // #FreeF
}

/* Generates N bytes of random data */
static void JimRandomBytes(Jim_InterpPtr interp, void *dest, unsigned_int len)
{
    PRJ_TRACE;
    Jim_PrngState *prng;
    unsigned_char *destByte = (unsigned_char *)dest;
    unsigned_int si, sj, x;

    /* initialization, only needed the first time */
    if (interp->prngState() == NULL)
        JimPrngInit(interp);
    prng = interp->prngState();
    /* generates 'len' bytes of pseudo-random numbers */
    if (prng != NULL) {
        for (x = 0; x < len; x++) {
            prng->i = (prng->i + 1) & 0xff;
            si = prng->sbox[prng->i];
            prng->j = (prng->j + si) & 0xff;
            sj = prng->sbox[prng->j];
            prng->sbox[prng->i] = sj;
            prng->sbox[prng->j] = si;
            *destByte++ = prng->sbox[(si + sj) & 0xff];
        }
    }
}


/* Re-seed the generator with user-provided bytes */
static void JimPrngSeed(Jim_InterpPtr interp, unsigned_char *seed, int seedLen)
{
    PRJ_TRACE;
    int i = 0;
    Jim_PrngState *prng = NULL;

    /* initialization, only needed the first time */
    if (interp->prngState() == NULL)
        JimPrngInit(interp); // #MissInCoverage
    prng = interp->prngState();

    /* Set the sbox[i] with i */
    if (prng == NULL) {
        return; // #MissInCoverage
    }
    for (i = 0; i < 256; i++) // #MagicNum
        prng->sbox[i] = i;
    /* Now use the seed to perform a random permutation of the sbox */
    for (i = 0; i < seedLen; i++) {
        unsigned_char t;

        t = prng->sbox[i & 0xFF];
        prng->sbox[i & 0xFF] = prng->sbox[seed[i]];
        prng->sbox[seed[i]] = t;
    }
    prng->i = prng->j = 0;

    /* discard at least the first 256 bytes of stream.
     * borrow the seed buffer for this
     */
    for (i = 0; i < 256; i += seedLen) { // #MagicNum
        JimRandomBytes(interp, seed, seedLen);
    }
}

/* [incr] */
static Retval Jim_IncrCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    jim_wide wideValue, increment = 1;
    Jim_ObjPtr intObjPtr;

    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "varName ?increment?");
        return JIM_ERR;
    }
    if (argc == 3) {
        if (Jim_GetWide(interp, argv[2], &increment) != JIM_OK)
            return JIM_ERR;
    }
    intObjPtr = Jim_GetVariable(interp, argv[1], JIM_UNSHARED);
    if (!intObjPtr) {
        /* Set missing variable to 0 */
        wideValue = 0;
    }
    else if (Jim_GetWide(interp, intObjPtr, &wideValue) != JIM_OK) {
        return JIM_ERR;
    }
    if (!intObjPtr || Jim_IsShared(intObjPtr)) {
        intObjPtr = Jim_NewIntObj(interp, wideValue + increment);
        if (Jim_SetVariable(interp, argv[1], intObjPtr) != JIM_OK) {
            Jim_FreeNewObj(interp, intObjPtr);
            return JIM_ERR;
        }
    }
    else {
        /* Can do it the quick way */
        Jim_InvalidateStringRep(intObjPtr);
        (intObjPtr)->setWideValue( wideValue + increment);

        /* The following step is required in order to invalidate the
         * string repr of "FOO" if the var name is on the form of "FOO(IDX)" */
        if (argv[1]->typePtr() != &g_variableObjType) {
            /* Note that this can't fail since GetVariable already succeeded */
            Jim_SetVariable(interp, argv[1], intObjPtr);
        }
    }
    Jim_SetResult(interp, intObjPtr);
    return JIM_OK;
}


/* -----------------------------------------------------------------------------
 * Eval
 * ---------------------------------------------------------------------------*/
enum {
    JIM_EVAL_SARGV_LEN = 8    /* static arguments vector length #MagicNum */
};
enum {
    JIM_EVAL_SINTV_LEN = 8    /* static interpolation vector length #MagicNum */
};

/* Handle calls to the [unknown] command */
static Retval JimUnknown(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    PRJ_TRACE;
    Retval retcode;

    /* If JimUnknown() is recursively called too many times...
     * done here
     */
    if (interp->unknown_called() > 50) { // #MagicNum
        return JIM_ERR; // #MissInCoverage #ErrorCondition
    }

    /* The object interp->unknown just contains
     * the "unknown" string, it is used in order to
     * avoid to lookup the unknown command every time
     * but instead to cache the result. */

    /* If the [unknown] command does not exist ... */
    if (Jim_GetCommand(interp, interp->unknown(), JIM_NONE) == NULL)
        return JIM_ERR;

    interp->incrUnknown_called();
    /* XXX: Are we losing fileNameObj and linenr? */
    retcode = Jim_EvalObjPrefix(interp, interp->unknown(), argc, argv);
    interp->decrUnknown_called();

    return retcode;
}

#ifdef _DEBUG // #optionalCode
static int g_DEBUG_VAL = 1;
#else
static int g_DEBUG_VAL = 0;
#endif
int   g_showCmds = 0;
char  g_breakOnCommand[64] =  ""; // #MagicNum #Debug

static Retval JimInvokeCommand(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv)
{
    PRJ_TRACE;
    Retval retcode;
    Jim_Cmd *cmdPtr;
    void *prevPrivData;

    PRJ_TRACE_GEN(::prj_trace::ACTION_CMD_INVOKE, __FUNCTION__, objv, NULL);
#if 0 // #optionalCode #WinOff #Debug
    printf("invoke"); // #stdoutput
    int j;
    for (j = 0; j < objc; j++) {
        printf(" '%s'", Jim_String(objv[j])); // #stdoutput
    }
    printf("\n"); // #stdoutput
#endif

    if (interp->framePtr()->tailcallCmd_) {
        /* Special tailcall command was pre-resolved */
        cmdPtr = interp->framePtr()->tailcallCmd_;
        interp->framePtr()->tailcallCmd_ = NULL;
    }
    else {
        cmdPtr = Jim_GetCommand(interp, objv[0], JIM_ERRMSG);
        if (cmdPtr == NULL) {
            return JimUnknown(interp, objc, objv);
        }
        JimIncrCmdRefCount(cmdPtr);
    }

    if (interp->evalDepth() == interp->maxEvalDepth()) {
        Jim_SetResultString(interp, "Infinite eval recursion", -1); // #MissInCoverage
        retcode = JIM_ERR;
        goto out;
    }
    interp->incrEvalDepth();
    prevPrivData = interp->cmdPrivData();

    /* Call it -- Make sure result is an empty object. */
    Jim_SetEmptyResult(interp);
    if (cmdPtr->isproc()) {
        retcode = JimCallProcedure(interp, cmdPtr, objc, objv);
    }
    else {
        /* Stop on command named in g_breakOnCommand and the first time */ // #optionalCode
        if (g_DEBUG_VAL) {
            if (g_showCmds) { // #MissInCoverage #Debug
                printf("CMD: |%s|\n", objv[0]->bytes()); // #stdoutput
            }
            if (strcmp(objv[0]->bytes(), g_breakOnCommand) == 0) { // #Debug
                BREAKPOINT;
            }
        }
        interp->setCmdPrivData(cmdPtr->u.native_.privData);
        retcode = cmdPtr->cmdProc()(interp, objc, objv); // #note return funcPtr and calls.
    }
    interp->setCmdPrivData(prevPrivData);
    interp->decrEvalDepth();

out:
    JimDecrCmdRefCount(interp, cmdPtr);

    return retcode;
}

/* Eval the object vector 'objv' composed of 'objc' elements.
 * Every element is used as single argument.
 * Jim_EvalObj() will call this function every time its object
 * argument is of "list" type, with no string representation.
 *
 * This is possible because the string representation of a
 * list object generated by the UpdateStringOfList is made
 * in a way that ensures that every list element is a different
 * command argument. */
JIM_EXPORT Retval Jim_EvalObjVector(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv)
{
    PRJ_TRACE;
    int i; Retval retcode;

    /* Incr refcount of arguments. */
    for (i = 0; i < objc; i++)
        Jim_IncrRefCount(objv[i]);

    retcode = JimInvokeCommand(interp, objc, objv);

    /* Decr refcount of arguments and return the retcode */
    for (i = 0; i < objc; i++)
        Jim_DecrRefCount(interp, objv[i]);

    return retcode;
}

/**
 * Invokes 'prefix' as a command with the objv array as arguments.
 */
JIM_EXPORT Retval Jim_EvalObjPrefix(Jim_InterpPtr interp, Jim_ObjPtr prefix, int objc, Jim_ObjConstArray objv)
{
    PRJ_TRACE;
    Retval ret;
    Jim_ObjArray* nargv = new_Jim_ObjArray((objc + 1)); // #AllocF 

    nargv[0] = prefix;
    memcpy(&nargv[1], &objv[0], sizeof(nargv[0]) * objc);
    ret = Jim_EvalObjVector(interp, objc + 1, nargv);
    free_Jim_ObjArray(nargv); // #FreeF
    return ret;
}

STATIC void JimAddErrorToStack(Jim_InterpPtr interp, ScriptObj *script)
{
    PRJ_TRACE;
    if (!interp->errorFlag()) {
        /* This is the first error, so save the file/line information and reset the stack */
        interp->setErrorFlag(1);
        Jim_IncrRefCount(script->fileNameObj);
        Jim_DecrRefCount(interp, interp->errorFileNameObj());
        interp->setErrorFileNameObj(script->fileNameObj);
        interp->setErrorLine(script->linenr);

        JimResetStackTrace(interp);
        /* Always add a level where the error first occurs */
        interp->incrAddStackTrace();
    }

    /* Now if this is an "interesting" level, add it to the stack trace */
    if (interp->addStackTrace() > 0) {
        /* Add the stack info for the current level */

        JimAppendStackTrace(interp, Jim_String(interp->errorProc()), script->fileNameObj, script->linenr);

        /* Note: if we didn't have a filename for this level,
         * don't clear the addStackTrace flag
         * so we can pick it up at the next level
         */
        if (Jim_Length(script->fileNameObj)) {
            interp->setAddStackTrace(0);
        }

        Jim_DecrRefCount(interp, interp->errorProc());
        interp->setErrorProc(interp->emptyObj());
        Jim_IncrRefCount(interp->errorProc());
    }
}

static Retval JimSubstOneToken(Jim_InterpPtr interp, const ScriptTokenPtr token, Jim_ObjArray* objPtrPtr)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    switch (token->type) {
        case JIM_TT_STR:
        case JIM_TT_ESC:
            objPtr = token->objPtr;
            break;
        case JIM_TT_VAR:
            objPtr = Jim_GetVariable(interp, token->objPtr, JIM_ERRMSG);
            break;
        case JIM_TT_DICTSUGAR:
            objPtr = JimExpandDictSugar(interp, token->objPtr);
            break;
        case JIM_TT_EXPRSUGAR:
            objPtr = JimExpandExprSugar(interp, token->objPtr);
            break;
        case JIM_TT_CMD:
            switch (Jim_EvalObj(interp, token->objPtr)) {
                case JIM_OK:
                case JIM_RETURN:
                    objPtr = interp->result();
                    break;
                case JIM_BREAK:
                    /* Stop substituting */
                    return JIM_BREAK;
                case JIM_CONTINUE:
                    /* just skip this one */
                    return JIM_CONTINUE;
                default:
                    return JIM_ERR;
            }
            break;
        default:
            JimPanic((1,
                "default token type (%d) reached " "in Jim_SubstObj().", token->type));
            objPtr = NULL; // #MissInCoverage
            break;
    }
    if (objPtr) {
        *objPtrPtr = objPtr;
        return JIM_OK;
    }
    return JIM_ERR;
}

/* Interpolate the given tokens into a unique Jim_Obj returned by reference
 * via *objPtrPtr. This function is only called by Jim_EvalObj() and Jim_SubstObj()
 * The returned object has refcount = 0.
 */
STATIC Jim_ObjPtr JimInterpolateTokens(Jim_InterpPtr interp, const ScriptTokenPtr  token, int tokens, int flags)
{
    PRJ_TRACE;
    int totlen = 0, i;
    Jim_ObjArray* intv;
    Jim_ObjPtr sintv[JIM_EVAL_SINTV_LEN];
    Jim_ObjPtr objPtr;
    char *s;

    if (tokens <= JIM_EVAL_SINTV_LEN)
        intv = sintv;
    else
        intv = new_Jim_ObjArray(tokens); // #AllocF 

    /* Compute every token forming the argument
     * in the intv objects vector. */
    for (i = 0; i < tokens; i++) {
        switch (JimSubstOneToken(interp, &token[i], &intv[i])) {
            case JIM_OK:
            case JIM_RETURN:
                break;
            case JIM_BREAK:
                if (flags & JIM_SUBST_FLAG) {
                    /* Stop here */
                    tokens = i;
                    continue;
                }
                /* XXX: Should probably set an error about break outside loop */
                /* fall through to error */
            case JIM_CONTINUE:
                if (flags & JIM_SUBST_FLAG) {
                    intv[i] = NULL;
                    continue;
                }
                /* XXX: Ditto continue outside loop */
                /* fall through to error */
            default:
                while (i--) {
                    Jim_DecrRefCount(interp, intv[i]);
                }
                if (intv != sintv) {
                    free_Jim_ObjArray(intv); // #FreeF #MissInCoverage
                }
                return NULL;
        }
        Jim_IncrRefCount(intv[i]);
        Jim_String(intv[i]);
        totlen += intv[i]->length();
    }

    /* Fast path return for a single token */
    if (tokens == 1 && intv[0] && intv == sintv) {
        /* Reverse the Jim_IncrRefCount() above, but don't free the object */
        intv[0]->decrRefCount();
        return intv[0];
    }

    /* Concatenate every token in an unique
     * object. */
    objPtr = Jim_NewStringObjNoAlloc(interp, NULL, 0);

    if (tokens == 4 && token[0].type == JIM_TT_ESC && token[1].type == JIM_TT_ESC
        && token[2].type == JIM_TT_VAR) {
        /* May be able to do fast interpolated object -> dictSubst */
        objPtr->setTypePtr(&g_interpolatedObjType);
        objPtr->setDictSubstValue(token[0].objPtr, intv[2]);
        //objPtr->internalRep.dictSubstValue_.varNameObjPtr = token[0].objPtr;
        //objPtr->internalRep.dictSubstValue_.indexObjPtr = intv[2];
        Jim_IncrRefCount(intv[2]);
    }
    else if (tokens && intv[0] && intv[0]->typePtr() == &g_sourceObjType) {
        /* The first interpolated token is source, so preserve the source info */
        JimSetSourceInfo(interp, objPtr, intv[0]->get_sourceValue_fileName(), 
                         intv[0]->get_sourceValue_lineNum());
    }


    s = objPtr->setBytes( new_CharArray(totlen + 1)); // #AllocF 
    objPtr->setLength(totlen);
    for (i = 0; i < tokens; i++) {
        if (intv[i]) {
            memcpy(s, intv[i]->bytes(), intv[i]->length());
            s += intv[i]->length();
            Jim_DecrRefCount(interp, intv[i]);
        }
    }
    objPtr->setBytes(totlen, '\0');
    /* Free the intv vector if not static. */
    if (intv != sintv) {
        free_Jim_ObjArray(intv); // #FreeF 
    }

    return objPtr;
}


/* listPtr *must* be a list.
 * The contents of the list is evaluated with the first element as the command and
 * the remaining elements as the arguments.
 */
STATIC Retval JimEvalObjList(Jim_InterpPtr interp, Jim_ObjPtr listPtr)
{
    PRJ_TRACE;
    Retval retcode = JIM_OK;

    JimPanic((Jim_IsList(listPtr) == 0, "JimEvalObjList() invoked on non-list."));

    if (listPtr->get_listValue_len()) {
        Jim_IncrRefCount(listPtr);
        retcode = JimInvokeCommand(interp,
            listPtr->get_listValue_len(),
            listPtr->get_listValue_ele()); 
        Jim_DecrRefCount(interp, listPtr);
    }
    return retcode;
}

JIM_EXPORT Retval Jim_EvalObjList(Jim_InterpPtr interp, Jim_ObjPtr listObj)
{
    PRJ_TRACE;
    SetListFromAny(interp, listObj);
    return JimEvalObjList(interp, listObj);
}

JIM_EXPORT Retval Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr)
{
    PRJ_TRACE;
    int i;
    ScriptObj *script;
    ScriptTokenPtr token;
    Retval retcode = JIM_OK;
    Jim_Obj *sargv[JIM_EVAL_SARGV_LEN], **argv = NULL;
    Jim_ObjPtr prevScriptObj;

    /* If the object is of type "list", with no string rep we can call
     * a specialized version of Jim_EvalObj() */
    if (Jim_IsList(scriptObjPtr) && scriptObjPtr->bytes() == NULL) {
        return JimEvalObjList(interp, scriptObjPtr);
    }

    Jim_IncrRefCount(scriptObjPtr);     /* Make sure it's shared. */
    script = JimGetScript(interp, scriptObjPtr);
    if (!JimScriptValid(interp, script)) {
        Jim_DecrRefCount(interp, scriptObjPtr);
        return JIM_ERR;
    }

    /* Reset the interpreter result. This is useful to
     * return the empty result in the case of empty program. */
    Jim_SetEmptyResult(interp);

    token = script->token;

    if (g_JIM_OPTIMIZATION_VAL) {
    /* Check for one of the following common scripts used by for, while
     *
     *   {}
     *   incr a
     */
    if (script->len == 0) {
        Jim_DecrRefCount(interp, scriptObjPtr);
        return JIM_OK;
    }
    if (script->len == 3
        && token[1].objPtr->typePtr() == &g_commandObjType
        && token[1].objPtr->get_cmdValue_cmd()->isproc_ == 0
        && token[1].objPtr->get_cmdValue_cmd()->cmdProc() == Jim_IncrCoreCommand
        && token[2].objPtr->typePtr() == &g_variableObjType) {

        Jim_ObjPtr objPtr = Jim_GetVariable(interp, token[2].objPtr, JIM_NONE);

        if (objPtr && !Jim_IsShared(objPtr) && objPtr->typePtr() == &g_intObjType) {
            (objPtr)->incrWideValue();
            Jim_InvalidateStringRep(objPtr);
            Jim_DecrRefCount(interp, scriptObjPtr);
            Jim_SetResult(interp, objPtr);
            return JIM_OK;
        }
    }
    }

    /* Now we have to make sure the internal repr will not be
     * freed on shimmering.
     *
     * Think for example to this:
     *
     * set x {llength $x; ... some more code ...}; eval $x
     *
     * In order to preserve the internal rep, we increment the
     * inUse field of the script internal rep structure. */
    script->inUse++;

    /* Stash the current script */
    prevScriptObj = interp->currentScriptObj();
    interp->currentScriptObj(scriptObjPtr);

    interp->setErrorFlag(0);
    argv = sargv;

    /* Execute every command sequentially until the end of the script
     * or an error occurs.
     */
    for (i = 0; i < script->len && retcode == JIM_OK; ) {
        int argc;
        int j;

        /* First token of the line is always JIM_TT_LINE */
        argc = token[i].objPtr->get_scriptLineValue_argc();
        script->linenr = token[i].objPtr->get_scriptLineValue_line();

        /* Allocate the arguments vector if required */
        if (argc > JIM_EVAL_SARGV_LEN)
            argv = new_Jim_ObjArray(argc); // #AllocF 

        /* Skip the JIM_TT_LINE token */
        i++;

        /* Populate the arguments objects.
         * If an error occurs, retcode will be set and
         * 'j' will be set to the number of args expanded
         */
        for (j = 0; j < argc; j++) {
            long wordtokens = 1;
            int expand = 0;
            Jim_ObjPtr wordObjPtr = NULL;

            if (token[i].type == JIM_TT_WORD) {
                wordtokens = (long)JimWideValue(token[i++].objPtr);
                if (wordtokens < 0) {
                    expand = 1;
                    wordtokens = -wordtokens;
                }
            }

            if (wordtokens == 1) {
                /* Fast path if the token does not
                 * need interpolation */

                switch (token[i].type) {
                    case JIM_TT_ESC:
                    case JIM_TT_STR:
                        wordObjPtr = token[i].objPtr;
                        break;
                    case JIM_TT_VAR:
                        wordObjPtr = Jim_GetVariable(interp, token[i].objPtr, JIM_ERRMSG);
                        break;
                    case JIM_TT_EXPRSUGAR:
                        wordObjPtr = JimExpandExprSugar(interp, token[i].objPtr);
                        break;
                    case JIM_TT_DICTSUGAR:
                        wordObjPtr = JimExpandDictSugar(interp, token[i].objPtr);
                        break;
                    case JIM_TT_CMD:
                        retcode = Jim_EvalObj(interp, token[i].objPtr);
                        if (retcode == JIM_OK) {
                            wordObjPtr = Jim_GetResult(interp);
                        }
                        break;
                    default:
                        JimPanic((1, "default token type reached " "in Jim_EvalObj()."));
                }
            }
            else {
                /* For interpolation we call a helper
                 * function to do the work for us. */
                wordObjPtr = JimInterpolateTokens(interp, token + i, wordtokens, JIM_NONE);
            }

            if (!wordObjPtr) {
                if (retcode == JIM_OK) {
                    retcode = JIM_ERR;
                }
                break;
            }

            Jim_IncrRefCount(wordObjPtr);
            i += wordtokens;

            if (!expand) {
                argv[j] = wordObjPtr;
            }
            else {
                /* Need to expand wordObjPtr into multiple args from argv[j] ... */
                int len = Jim_ListLength(interp, wordObjPtr);
                int newargc = argc + len - 1;
                int k;

                if (len > 1) {
                    if (argv == sargv) {
                        if (newargc > JIM_EVAL_SARGV_LEN) {
                            argv = new_Jim_ObjArray(newargc); // #AllocF 
                            memcpy(argv, sargv, sizeof(*argv) * j);
                        }
                    }
                    else {
                        /* Need to realloc to make room for (len - 1) more entries */
                        argv = realloc_Jim_ObjArray(argv, newargc); // #AllocF 
                    }
                }

                /* Now copy in the expanded version */
                for (k = 0; k < len; k++) {
                    argv[j++] = wordObjPtr->get_listValue_objArray(k);
                    Jim_IncrRefCount(wordObjPtr->get_listValue_objArray(k));
                }

                /* The original object reference is no longer needed,
                 * after the expansion it is no longer present on
                 * the argument vector, but the single elements are
                 * in its place. */
                Jim_DecrRefCount(interp, wordObjPtr);

                /* And update the indexes */
                j--;
                argc += len - 1;
            }
        }

        if (retcode == JIM_OK && argc) {
            /* Invoke the command */
            retcode = JimInvokeCommand(interp, argc, argv);
            /* Check for a signal after each command */
            if (Jim_CheckSignal(interp)) {
                retcode = JIM_SIGNAL; // #MissInCoverage
            }
        }

        /* Finished with the command, so decrement ref counts of each argument */
        while (j-- > 0) {
            Jim_DecrRefCount(interp, argv[j]);
        }

        if (argv != sargv) {
            free_Jim_ObjArray(argv); // #FreeF
            argv = sargv;
        }
    }

    /* Possibly add to the error stack trace */
    if (retcode == JIM_ERR) {
        JimAddErrorToStack(interp, script);
    }
    /* Propagate the addStackTrace value through 'return -code error' */
    else if (retcode != JIM_RETURN || interp->returnCode() != JIM_ERR) {
        /* No need to add stack trace */
        interp->setAddStackTrace(0);
    }

    /* Restore the current script */
    interp->currentScriptObj(prevScriptObj);

    /* Note that we don't have to decrement inUse, because the
     * following code transfers our use of the reference again to
     * the script object. */
    Jim_FreeIntRep(interp, scriptObjPtr);
    scriptObjPtr->setTypePtr(&g_scriptObjType);
    scriptObjPtr->setPtr<ScriptObj*>(script);
    //Jim_SetIntRepPtr(scriptObjPtr, script);
    Jim_DecrRefCount(interp, scriptObjPtr);

    return retcode;
}

static Retval JimSetProcArg(Jim_InterpPtr interp, Jim_ObjPtr argNameObj, Jim_ObjPtr argValObj)
{
    PRJ_TRACE;
    Retval retcode;
    /* If argObjPtr begins with '&', do an automatic upvar */
    const char *varname = Jim_String(argNameObj);
    if (*varname == '&') {
        /* First check that the target variable exists */
        Jim_ObjPtr objPtr;
        Jim_CallFramePtr savedCallFrame = interp->framePtr();

        interp->framePtr(interp->framePtr()->parent());
        objPtr = Jim_GetVariable(interp, argValObj, JIM_ERRMSG);
        interp->framePtr(savedCallFrame);
        if (!objPtr) {
            return JIM_ERR;
        }

        /* It exists, so perform the binding. */
        objPtr = Jim_NewStringObj(interp, varname + 1, -1);
        Jim_IncrRefCount(objPtr);
        retcode = Jim_SetVariableLink(interp, objPtr, argValObj, interp->framePtr()->parent());
        Jim_DecrRefCount(interp, objPtr);
    }
    else {
        retcode = Jim_SetVariable(interp, argNameObj, argValObj);
    }
    return retcode;
}

/**
 * Sets the interp result to be an error message indicating the required proc args.
 */
static void JimSetProcWrongArgs(Jim_InterpPtr interp, Jim_ObjPtr procNameObj, Jim_Cmd *cmd)
{
    PRJ_TRACE;
    /* Create a nice error message, consistent with Tcl 8.5 */
    Jim_ObjPtr argmsg = Jim_NewStringObj(interp, "", 0);
    int i;

    for (i = 0; i < cmd->u.proc_.argListLen; i++) {
        Jim_AppendString(interp, argmsg, " ", 1);

        if (i == cmd->u.proc_.argsPos) {
            if (cmd->u.proc_.arglist[i].defaultObjPtr) {
                /* Renamed args */
                Jim_AppendString(interp, argmsg, "?", 1);
                Jim_AppendObj(interp, argmsg, cmd->u.proc_.arglist[i].defaultObjPtr);
                Jim_AppendString(interp, argmsg, " ...?", -1);
            }
            else {
                /* We have plain args */
                Jim_AppendString(interp, argmsg, "?arg...?", -1);
            }
        }
        else {
            if (cmd->u.proc_.arglist[i].defaultObjPtr) {
                Jim_AppendString(interp, argmsg, "?", 1);
                Jim_AppendObj(interp, argmsg, cmd->u.proc_.arglist[i].nameObjPtr);
                Jim_AppendString(interp, argmsg, "?", 1);
            }
            else {
                const char *arg = Jim_String(cmd->u.proc_.arglist[i].nameObjPtr);
                if (*arg == '&') {
                    arg++;
                }
                Jim_AppendString(interp, argmsg, arg, -1);
            }
        }
    }
    Jim_SetResultFormatted(interp, "wrong # args: should be \"%#s%#s\"", procNameObj, argmsg);
}

#ifdef jim_ext_namespace // #optionalCode
/*
 * [namespace eval]
 */
JIM_EXPORT Retval Jim_EvalNamespace(Jim_InterpPtr interp, Jim_ObjPtr scriptObj, Jim_ObjPtr nsObj)
{
    PRJ_TRACE;
    Jim_CallFramePtr callFramePtr;
    Retval retcode;

    /* Create a new callframe */
    callFramePtr = JimCreateCallFrame(interp, interp->framePtr(), nsObj);
    callFramePtr->argv_ = &interp->emptyObj_; // #JI_access emptyObj_
    callFramePtr->argc_ = 0;
    callFramePtr->procArgsObjPtr_ = NULL;
    callFramePtr->procBodyObjPtr_ = scriptObj;
    callFramePtr->staticVars_ = NULL;
    callFramePtr->fileNameObj_ = interp->emptyObj();
    callFramePtr->line = 0;
    Jim_IncrRefCount(scriptObj);
    interp->framePtr(callFramePtr);

    /* Check if there are too nested calls */
    if (interp->framePtr()->level() == interp->maxCallFrameDepth()) {
        Jim_SetResultString(interp, "Too many nested calls. Infinite recursion?", -1); // #MissInCoverage
        retcode = JIM_ERR;
    }
    else {
        /* Eval the body */
        retcode = Jim_EvalObj(interp, scriptObj);
    }

    /* Destroy the callframe */
    interp->framePtr(interp->framePtr()->parent());
    JimFreeCallFrame(interp, callFramePtr, JIM_FCF_REUSE);

    return retcode;
}
#endif

/* Call a procedure implemented in Tcl.
 * It's possible to speed-up a lot this function, currently
 * the callframes are not cached, but allocated and
 * destroyed every time. What is especially costly is
 * to create/destroy the local vars hash table every time.
 *
 * This can be fixed just implementing callframes caching
 * in JimCreateCallFrame() and JimFreeCallFrame(). */
STATIC Retval JimCallProcedure(Jim_InterpPtr interp, Jim_Cmd *cmd, int argc, Jim_ObjConstArray argv)
{
    PRJ_TRACE;
    PRJ_TRACE_GEN(::prj_trace::ACTION_PROC_INVOKE, __FUNCTION__, cmd, NULL);
    Jim_CallFramePtr callFramePtr;
    int i, d, optargs; Retval retcode;
    ScriptObj *script;

    /* Check arity */
    if (argc - 1 < cmd->u.proc_.reqArity ||
        (cmd->u.proc_.argsPos < 0 && argc - 1 > cmd->u.proc_.reqArity + cmd->u.proc_.optArity)) {
        JimSetProcWrongArgs(interp, argv[0], cmd);
        return JIM_ERR;
    }

    if (Jim_Length(cmd->u.proc_.bodyObjPtr) == 0) {
        /* Optimize for procedure with no body - useful for optional debugging */
        return JIM_OK;
    }

    /* Check if there are too nested calls */
    if (interp->framePtr()->level() == interp->maxCallFrameDepth()) {
        Jim_SetResultString(interp, "Too many nested calls. Infinite recursion?", -1); // #MissInCoverage
        return JIM_ERR;
    }

    /* Create a new callframe */
    callFramePtr = JimCreateCallFrame(interp, interp->framePtr(), cmd->u.proc_.nsObj);
    callFramePtr->argv_ = argv;
    callFramePtr->argc_ = argc;
    callFramePtr->procArgsObjPtr_ = cmd->u.proc_.argListObjPtr;
    callFramePtr->procBodyObjPtr_ = cmd->u.proc_.bodyObjPtr;
    callFramePtr->staticVars_ = cmd->u.proc_.staticVars;

    /* Remember where we were called from. */
    script = JimGetScript(interp, interp->currentScriptObj());
    callFramePtr->fileNameObj_ = script->fileNameObj;
    callFramePtr->line = script->linenr;

    Jim_IncrRefCount(cmd->u.proc_.argListObjPtr);
    Jim_IncrRefCount(cmd->u.proc_.bodyObjPtr);
    interp->framePtr(callFramePtr);

    /* How many optional args are available */
    optargs = (argc - 1 - cmd->u.proc_.reqArity);

    /* Step 'i' along the actual args, and step 'd' along the formal args */
    i = 1;
    for (d = 0; d < cmd->u.proc_.argListLen; d++) {
        Jim_ObjPtr nameObjPtr = cmd->u.proc_.arglist[d].nameObjPtr;
        if (d == cmd->u.proc_.argsPos) {
            /* assign $args */
            Jim_ObjPtr listObjPtr;
            int argsLen = 0;
            if (cmd->u.proc_.reqArity + cmd->u.proc_.optArity < argc - 1) {
                argsLen = argc - 1 - (cmd->u.proc_.reqArity + cmd->u.proc_.optArity);
            }
            listObjPtr = Jim_NewListObj(interp, &argv[i], argsLen);

            /* It is possible to rename args. */
            if (cmd->u.proc_.arglist[d].defaultObjPtr) {
                nameObjPtr =cmd->u.proc_.arglist[d].defaultObjPtr;
            }
            retcode = Jim_SetVariable(interp, nameObjPtr, listObjPtr);
            if (retcode != JIM_OK) {
                goto badargset; // #MissInCoverage
            }

            i += argsLen;
            continue;
        }

        /* Optional or required? */
        if (cmd->u.proc_.arglist[d].defaultObjPtr == NULL || optargs-- > 0) {
            retcode = JimSetProcArg(interp, nameObjPtr, argv[i++]);
        }
        else {
            /* Ran out, so use the default */
            retcode = Jim_SetVariable(interp, nameObjPtr, cmd->u.proc_.arglist[d].defaultObjPtr);
        }
        if (retcode != JIM_OK) {
            goto badargset;
        }
    }

    /* Eval the body */
    retcode = Jim_EvalObj(interp, cmd->u.proc_.bodyObjPtr);

badargset:

    /* Invoke $jim::defer then destroy the callframe */
    retcode = JimInvokeDefer(interp, retcode);
    interp->framePtr(interp->framePtr()->parent());
    JimFreeCallFrame(interp, callFramePtr, JIM_FCF_REUSE);

    /* Now chain any tailcalls in the parent frame */
    if (interp->framePtr()->tailcallObj_) {
        do {
            Jim_ObjPtr tailcallObj = interp->framePtr()->tailcallObj_;

            interp->framePtr()->tailcallObj_ = NULL;

            if (retcode == JIM_EVAL) {
                retcode = Jim_EvalObjList(interp, tailcallObj);
                if (retcode == JIM_RETURN) {
                    /* If the result of the tailcall is 'return', push
                     * it up to the caller
                     */
                    interp->incrReturnLevel();
                }
            }
            Jim_DecrRefCount(interp, tailcallObj);
        } while (interp->framePtr()->tailcallObj_);

        /* If the tailcall chain finished early, may need to manually discard the command */
        if (interp->framePtr()->tailcallCmd_) {
            JimDecrCmdRefCount(interp, interp->framePtr()->tailcallCmd_);
            interp->framePtr()->tailcallCmd_ = NULL;
        }
    }

    /* Handle the JIM_RETURN return code */
    if (retcode == JIM_RETURN) {
        if (interp->decrReturnLevel() <= 0) {
            retcode = interp->returnCode();
            interp->setReturnCode(JIM_OK);
            interp->setReturnLevel(0);
        }
    }
    else if (retcode == JIM_ERR) {
        interp->incrAddStackTrace();
        Jim_DecrRefCount(interp, interp->errorProc());
        interp->setErrorProc(argv[0]);
        Jim_IncrRefCount(interp->errorProc());
    }

    return retcode;
}

JIM_EXPORT Retval Jim_EvalSource(Jim_InterpPtr interp, const char *filename, int lineno, const char *script)
{
    PRJ_TRACE;
    int retval;
    Jim_ObjPtr scriptObjPtr;

    scriptObjPtr = Jim_NewStringObj(interp, script, -1);
    Jim_IncrRefCount(scriptObjPtr);

    if (filename) {
        Jim_ObjPtr prevScriptObj;

        JimSetSourceInfo(interp, scriptObjPtr, Jim_NewStringObj(interp, filename, -1), lineno);

        prevScriptObj = interp->currentScriptObj();
        interp->currentScriptObj(scriptObjPtr);

        retval = Jim_EvalObj(interp, scriptObjPtr);

        interp->currentScriptObj(prevScriptObj);
    }
    else {
        retval = Jim_EvalObj(interp, scriptObjPtr); // #MissInCoverage
    }
    Jim_DecrRefCount(interp, scriptObjPtr);
    return retval;
}

JIM_EXPORT Retval Jim_Eval(Jim_InterpPtr interp, const char *script)
{
    PRJ_TRACE;
    return Jim_EvalObj(interp, Jim_NewStringObj(interp, script, -1));
}

/* Execute script in the scope of the global level */
JIM_EXPORT Retval Jim_EvalGlobal(Jim_InterpPtr interp, const char *script) // #MissInCoverage
{
    PRJ_TRACE;
    int retval;
    Jim_CallFramePtr savedFramePtr = interp->framePtr();

    interp->framePtr(interp->topFramePtr());
    retval = Jim_Eval(interp, script);
    interp->framePtr(savedFramePtr);

    return retval;
}

JIM_EXPORT Retval Jim_EvalFileGlobal(Jim_InterpPtr interp, const char *filename)
{
    PRJ_TRACE;
    Retval retval;
    Jim_CallFramePtr savedFramePtr = interp->framePtr();

    interp->framePtr(interp->topFramePtr());
    retval = Jim_EvalFile(interp, filename);
    interp->framePtr(savedFramePtr);

    return retval;
}

#include <sys/stat.h>

JIM_EXPORT Retval Jim_EvalFile(Jim_InterpPtr interp, const char *filename)
{
    PRJ_TRACE;
    FILE *fp;
    char *buf;
    Jim_ObjPtr scriptObjPtr;
    Jim_ObjPtr prevScriptObj;
    struct stat sb;
    Retval retcode;
    int readlen;

    if (stat(filename, &sb) != 0 || (fp = fopen(filename, "rt")) == NULL) {
        Jim_SetResultFormatted(interp, "couldn't read file \"%s\": %s", filename, strerror(errno)); // #MissInCoverage
        return JIM_ERR;
    }
    if (sb.st_size == 0) {
        fclose(fp); // #MissInCoverage
        return JIM_OK;
    }

    buf = new_CharArray(sb.st_size + 1); // #AllocF 
    readlen = (int)prj_fread(buf, 1, sb.st_size, fp); // #input
    if (ferror(fp)) {
        fclose(fp); // #MissInCoverage
        free_CharArray(buf); // #FreeF
        Jim_SetResultFormatted(interp, "failed to load file \"%s\": %s", filename, strerror(errno));
        return JIM_ERR;
    }
    fclose(fp);
    buf[readlen] = 0;

    scriptObjPtr = Jim_NewStringObjNoAlloc(interp, buf, readlen);
    JimSetSourceInfo(interp, scriptObjPtr, Jim_NewStringObj(interp, filename, -1), 1);
    Jim_IncrRefCount(scriptObjPtr);

    prevScriptObj = interp->currentScriptObj();
    interp->currentScriptObj(scriptObjPtr);

    retcode = Jim_EvalObj(interp, scriptObjPtr);

    /* Handle the JIM_RETURN return code */
    if (retcode == JIM_RETURN) {
        if (interp->decrReturnLevel() <= 0) {
            retcode = interp->returnCode();
            interp->setReturnCode(JIM_OK);
            interp->setReturnLevel(0);
        }
    }
    if (retcode == JIM_ERR) {
        /* EvalFile changes context, so add a stack frame here */
        interp->incrAddStackTrace();
    }

    interp->currentScriptObj(prevScriptObj);

    Jim_DecrRefCount(interp, scriptObjPtr);

    return retcode;
}

/* -----------------------------------------------------------------------------
 * Subst
 * ---------------------------------------------------------------------------*/
STATIC void JimParseSubst(JimParserCtxPtr pc, int flags)
{
    PRJ_TRACE;
    pc->tstart = pc->p;
    pc->tline = pc->linenr;

    if (pc->len == 0) {
        pc->tend = pc->p;
        pc->tt = JIM_TT_EOL;
        pc->eof = 1;
        return;
    }
    if (*pc->p == '[' && !(flags & JIM_SUBST_NOCMD)) {
        JimParseCmd(pc);
        return;
    }
    if (*pc->p == '$' && !(flags & JIM_SUBST_NOVAR)) {
        if (JimParseVar(pc) == JIM_OK) {
            return;
        }
        /* Not a var, so treat as a string */
        pc->tstart = pc->p;
        flags |= JIM_SUBST_NOVAR;
    }
    while (pc->len) {
        if (*pc->p == '$' && !(flags & JIM_SUBST_NOVAR)) {
            break;
        }
        if (*pc->p == '[' && !(flags & JIM_SUBST_NOCMD)) {
            break;
        }
        if (*pc->p == '\\' && pc->len > 1) {
            pc->p++;
            pc->len--;
        }
        pc->p++;
        pc->len--;
    }
    pc->tend = pc->p - 1;
    pc->tt = (flags & JIM_SUBST_NOESC) ? JIM_TT_STR : JIM_TT_ESC;
}

/* The subst object type reuses most of the data structures and functions
 * of the script object. Script's data structures are a bit more complex
 * for what is needed for [subst]itution tasks, but the reuse helps to
 * deal with a single data structure at the cost of some more memory
 * usage for substitutions. */

/* This method takes the string representation of an object
 * as a Tcl string where to perform [subst]itution, and generates
 * the pre-parsed internal representation. */
STATIC Retval SetSubstFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags)
{
    PRJ_TRACE;
    int scriptTextLen;
    const char *scriptText = Jim_GetString(objPtr, &scriptTextLen);
    JimParserCtx parser;
    ScriptObj* script = new_ScriptObj; // #AllocF 
    ParseTokenList tokenlist;

    /* Initially parse the subst into tokens (in tokenlist) */
    ScriptTokenListInit(&tokenlist);

    JimParserInit(&parser, scriptText, scriptTextLen, 1);
    while (1) {
        JimParseSubst(&parser, flags);
        if (parser.eof) {
            /* Note that subst doesn't need the EOL token */
            break;
        }
        ScriptAddToken(&tokenlist, parser.tstart, 
            (int)(parser.tend - parser.tstart + 1), parser.tt,
            parser.tline);
    }

    /* Create the "real" subst/script tokens from the initial token list */
    script->inUse = 1;
    script->substFlags = flags;
    script->fileNameObj = interp->emptyObj();
    Jim_IncrRefCount(script->fileNameObj);
    SubstObjAddTokens(interp, script, &tokenlist);

    /* No longer need the token list */
    ScriptTokenListFree(&tokenlist);

    if (g_DEBUG_SHOW_SUBST)
    {
        int i;

        printf("==== Subst ====\n"); // #stdoutput #MissInCoverage
        for (i = 0; i < script->len; i++) {
            printf("[%2d] %s '%s'\n", i, jim_tt_name(script->token[i].type), // #stdoutput
                Jim_String(script->token[i].objPtr));
        }
    }

    /* Free the old internal rep and set the new one. */
    Jim_FreeIntRep(interp, objPtr);
    objPtr->setPtr<ScriptObj*>(script);
    //Jim_SetIntRepPtr(objPtr, script);
    objPtr->setTypePtr(&g_scriptObjType);
    return JIM_OK;
}

STATIC ScriptObj *Jim_GetSubst(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags)
{
    PRJ_TRACE;
    if (objPtr->typePtr() != &g_scriptObjType || ((ScriptObj *)Jim_GetIntRepPtr(objPtr))->substFlags != flags)
        SetSubstFromAny(interp, objPtr, flags);
    return (ScriptObj *) Jim_GetIntRepPtr(objPtr);
}

/* Performs commands,variables,blackslashes substitution,
 * storing the result object (with refcount 0) into
 * resObjPtrPtr. */
JIM_EXPORT Retval Jim_SubstObj(Jim_InterpPtr interp, Jim_ObjPtr substObjPtr, Jim_ObjArray* resObjPtrPtr, int flags)
{
    PRJ_TRACE;
    ScriptObj *script = Jim_GetSubst(interp, substObjPtr, flags);

    Jim_IncrRefCount(substObjPtr);      /* Make sure it's shared. */
    /* In order to preserve the internal rep, we increment the
     * inUse field of the script internal rep structure. */
    script->inUse++;

    *resObjPtrPtr = JimInterpolateTokens(interp, script->token, script->len, flags);

    script->inUse--;
    Jim_DecrRefCount(interp, substObjPtr);
    if (*resObjPtrPtr == NULL) {
        return JIM_ERR;
    }
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * Core commands utility functions
 * ---------------------------------------------------------------------------*/
JIM_EXPORT void Jim_WrongNumArgs(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv, const char *msg)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;
    Jim_ObjPtr listObjPtr;

    JimPanic((argc == 0, "Jim_WrongNumArgs() called with argc=0"));

    listObjPtr = Jim_NewListObj(interp, argv, argc);

    if (msg && *msg) {
        Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, msg, -1));
    }
    Jim_IncrRefCount(listObjPtr);
    objPtr = Jim_ListJoin(interp, listObjPtr, " ", 1);
    Jim_DecrRefCount(interp, listObjPtr);

    Jim_SetResultFormatted(interp, "wrong # args: should be \"%#s\"", objPtr);
}

/**
 * May add the key and/or value to the list.
 */
typedef void JimHashtableIteratorCallbackType(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr,
    Jim_HashEntryPtr he, int type);

static inline int JimTrivialMatch(const char* pattern) { return (strpbrk((pattern), "*[?\\") == NULL); }

/**
 * For each key of the hash table 'ht' (with string keys) which matches the glob pattern (all if NULL),
 * invoke the callback to add entries to a list.
 * Returns the list.
 */
static Jim_ObjPtr JimHashtablePatternMatch(Jim_InterpPtr interp, Jim_HashTablePtr ht, Jim_ObjPtr patternObjPtr,
    JimHashtableIteratorCallbackType *callback, int type)
{
    PRJ_TRACE;
    Jim_HashEntryPtr he;
    Jim_ObjPtr listObjPtr = Jim_NewListObj(interp, NULL, 0);

    /* Check for the non-pattern case. We can do this much more efficiently. */
    if (patternObjPtr && JimTrivialMatch(Jim_String(patternObjPtr))) {
        he = Jim_FindHashEntry(ht, Jim_String(patternObjPtr));
        if (he) {
            callback(interp, listObjPtr, he, type);
        }
    }
    else {
        Jim_HashTableIterator htiter;
        JimInitHashTableIterator(ht, &htiter);
        while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
            if (patternObjPtr == NULL || JimGlobMatch(Jim_String(patternObjPtr), he->keyAsStr(), 0)) {
                callback(interp, listObjPtr, he, type);
            }
        }
    }
    return listObjPtr;
}

/* Keep these in order */
enum JIM_CMDLIST {
    JIM_CMDLIST_COMMANDS = 0,
    JIM_CMDLIST_PROCS = 1,
    JIM_CMDLIST_CHANNELS = 2
};

/**
 * Adds matching command names (procs, channels) to the list.
 */
static void JimCommandMatch(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr,
    Jim_HashEntryPtr he, int type)
{
    PRJ_TRACE;
    Jim_Cmd *cmdPtr = (Jim_Cmd*)Jim_GetHashEntryVal(he);
    Jim_ObjPtr objPtr;

    if (type == JIM_CMDLIST_PROCS && !cmdPtr->isproc()) {
        /* not a proc */
        return; // #MissInCoverage
    }

    objPtr = Jim_NewStringObj(interp, he->keyAsStr(), -1);
    Jim_IncrRefCount(objPtr);

    if (type != JIM_CMDLIST_CHANNELS 
#if 1  // #optionalCode #FIXME Reference to Aio extension in main code?
    || Jim_AioFilehandle(interp, objPtr)
#endif
    ) {
        Jim_ListAppendElement(interp, listObjPtr, objPtr);
    }
    Jim_DecrRefCount(interp, objPtr);
}

/* type is JIM_CMDLIST_xxx */
static Jim_ObjPtr JimCommandsList(Jim_InterpPtr interp, Jim_ObjPtr patternObjPtr, int type)
{
    PRJ_TRACE;
    return JimHashtablePatternMatch(interp, &interp->commands(), patternObjPtr, JimCommandMatch, type);
}

/* Keep these in order */
enum JIM_VARLIST {
    JIM_VARLIST_GLOBALS = 0,
    JIM_VARLIST_LOCALS = 1,
    JIM_VARLIST_VARS = 2,

    JIM_VARLIST_VALUES = 0x1000
}; 

/**
 * Adds matching variable names to the list.
 */
static void JimVariablesMatch(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr,
    Jim_HashEntryPtr he, int type)
{
    PRJ_TRACE;
    Jim_Var *varPtr = (Jim_Var*)Jim_GetHashEntryVal(he);

    if (type != JIM_VARLIST_LOCALS || varPtr->linkFramePtr == NULL) {
        Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, he->keyAsStr(), -1));
        if (type & JIM_VARLIST_VALUES) {
            Jim_ListAppendElement(interp, listObjPtr, varPtr->objPtr);
        }
    }
}

/* mode is JIM_VARLIST_xxx */
static Jim_ObjPtr JimVariablesList(Jim_InterpPtr interp, Jim_ObjPtr patternObjPtr, int mode)
{
    PRJ_TRACE;
    if (mode == JIM_VARLIST_LOCALS && interp->framePtr() == interp->topFramePtr()) {
        /* For [info locals], if we are at top level an emtpy list
         * is returned. I don't agree, but we aim at compatibility (SS) */
        return interp->emptyObj();
    }
    else {
        Jim_CallFramePtr framePtr = (mode == JIM_VARLIST_GLOBALS) ? interp->topFramePtr() : interp->framePtr();
        return JimHashtablePatternMatch(interp, &framePtr->vars(), patternObjPtr, JimVariablesMatch, mode);
    }
}

STATIC Retval JimInfoLevel(Jim_InterpPtr interp, Jim_ObjPtr levelObjPtr,
    Jim_ObjArray* objPtrPtr, int info_level_cmd)
{
    PRJ_TRACE;
    Jim_CallFramePtr targetCallFrame;

    targetCallFrame = JimGetCallFrameByInteger(interp, levelObjPtr);
    if (targetCallFrame == NULL) {
        return JIM_ERR;
    }
    /* No proc call at toplevel callframe */
    if (targetCallFrame == interp->topFramePtr()) {
        Jim_SetResultFormatted(interp, "bad level \"%#s\"", levelObjPtr);
        return JIM_ERR;
    }
    if (info_level_cmd) {
        *objPtrPtr = Jim_NewListObj(interp, targetCallFrame->argv_, targetCallFrame->argc());
    }
    else {
        Jim_ObjPtr listObj = Jim_NewListObj(interp, NULL, 0);

        Jim_ListAppendElement(interp, listObj, targetCallFrame->argv_[0]);
        Jim_ListAppendElement(interp, listObj, targetCallFrame->fileNameObj_);
        Jim_ListAppendElement(interp, listObj, Jim_NewIntObj(interp, targetCallFrame->line));
        *objPtrPtr = listObj;
    }
    return JIM_OK;
}

/* -----------------------------------------------------------------------------
 * Core commands
 * ---------------------------------------------------------------------------*/

/* fake [puts] -- not the real puts, just for debugging. */
static Retval Jim_PutsCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #MissInCoverage #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "?-nonewline? string");
        return JIM_ERR;
    }
    if (argc == 3) {
        if (!Jim_CompareStringImmediate(interp, argv[1], "-nonewline")) {
            Jim_SetResultString(interp, "The second argument must " "be -nonewline", -1);
            return JIM_ERR;
        }
        else {
            fputs(Jim_String(argv[2]), stdout); // #stdoutput
        }
    }
    else {
        puts(Jim_String(argv[1])); // #stdoutput
    }
    return JIM_OK;
}

/* Helper for [+] and [*] */
static Retval JimAddMulHelper(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv, int op)
{
    PRJ_TRACE;
    jim_wide wideValue, res;
    double doubleValue, doubleRes;
    int i;

    res = (op == JIM_EXPROP_ADD) ? 0 : 1;

    for (i = 1; i < argc; i++) {
        if (Jim_GetWide(interp, argv[i], &wideValue) != JIM_OK)
            goto trydouble;
        if (op == JIM_EXPROP_ADD)
            res += wideValue;
        else
            res *= wideValue;
    }
    Jim_SetResultInt(interp, res);
    return JIM_OK;
  trydouble:
    doubleRes = (double)res;
    for (; i < argc; i++) {
        if (Jim_GetDouble(interp, argv[i], &doubleValue) != JIM_OK)
            return JIM_ERR;
        if (op == JIM_EXPROP_ADD)
            doubleRes += doubleValue;
        else
            doubleRes *= doubleValue;
    }
    Jim_SetResult(interp, Jim_NewDoubleObj(interp, doubleRes));
    return JIM_OK;
}

/* Helper for [-] and [/] */
static Retval JimSubDivHelper(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv, int op)
{
    PRJ_TRACE;
    jim_wide wideValue, res = 0;
    double doubleValue, doubleRes = 0;
    int i = 2;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "number ?number ... number?");
        return JIM_ERR;
    }
    else if (argc == 2) {
        /* The arity = 2 case is different. For [- x] returns -x,
         * while [/ x] returns 1/x. */
        if (Jim_GetWide(interp, argv[1], &wideValue) != JIM_OK) {
            if (Jim_GetDouble(interp, argv[1], &doubleValue) != JIM_OK) {
                return JIM_ERR;
            }
            else {
                if (op == JIM_EXPROP_SUB)
                    doubleRes = -doubleValue;
                else
                    doubleRes = 1.0 / doubleValue;
                Jim_SetResult(interp, Jim_NewDoubleObj(interp, doubleRes));
                return JIM_OK;
            }
        }
        if (op == JIM_EXPROP_SUB) {
            res = -wideValue;
            Jim_SetResultInt(interp, res);
        }
        else {
            doubleRes = 1.0 / wideValue;
            Jim_SetResult(interp, Jim_NewDoubleObj(interp, doubleRes));
        }
        return JIM_OK;
    }
    else {
        if (Jim_GetWide(interp, argv[1], &res) != JIM_OK) {
            if (Jim_GetDouble(interp, argv[1], &doubleRes)
                != JIM_OK) {
                return JIM_ERR;
            }
            else {
                goto trydouble;
            }
        }
    }
    for (i = 2; i < argc; i++) {
        if (Jim_GetWide(interp, argv[i], &wideValue) != JIM_OK) {
            doubleRes = (double)res;
            goto trydouble;
        }
        if (op == JIM_EXPROP_SUB)
            res -= wideValue;
        else {
            if (wideValue == 0) {
                Jim_SetResultString(interp, "Division by zero", -1); // #MissInCoverage
                return JIM_ERR;
            }
            res /= wideValue;
        }
    }
    Jim_SetResultInt(interp, res);
    return JIM_OK;
  trydouble:
    for (; i < argc; i++) {
        if (Jim_GetDouble(interp, argv[i], &doubleValue) != JIM_OK)
            return JIM_ERR;
        if (op == JIM_EXPROP_SUB)
            doubleRes -= doubleValue;
        else
            doubleRes /= doubleValue;
    }
    Jim_SetResult(interp, Jim_NewDoubleObj(interp, doubleRes));
    return JIM_OK;
}


/* [+] */
static Retval Jim_AddCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    return JimAddMulHelper(interp, argc, argv, JIM_EXPROP_ADD);
}

/* [*] */
static Retval Jim_MulCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    return JimAddMulHelper(interp, argc, argv, JIM_EXPROP_MUL);
}

/* [-] */
static Retval Jim_SubCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    return JimSubDivHelper(interp, argc, argv, JIM_EXPROP_SUB);
}

/* [/] */
static Retval Jim_DivCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    return JimSubDivHelper(interp, argc, argv, JIM_EXPROP_DIV);
}

/* [set] */
static Retval Jim_SetCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "varName ?newValue?");
        return JIM_ERR;
    }
    if (argc == 2) {
        Jim_ObjPtr objPtr;

        objPtr = Jim_GetVariable(interp, argv[1], JIM_ERRMSG);
        if (!objPtr)
            return JIM_ERR;
        Jim_SetResult(interp, objPtr);
        return JIM_OK;
    }
    /* argc == 3 case. */
    if (Jim_SetVariable(interp, argv[1], argv[2]) != JIM_OK)
        return JIM_ERR;
    Jim_SetResult(interp, argv[2]);
    return JIM_OK;
}

/* [unset]
 *
 * unset ?-nocomplain? ?--? ?varName ...?
 */
static Retval Jim_UnsetCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int i = 1;
    int complain = 1;

    while (i < argc) {
        if (Jim_CompareStringImmediate(interp, argv[i], "--")) {
            i++;
            break;
        }
        if (Jim_CompareStringImmediate(interp, argv[i], "-nocomplain")) {
            complain = 0;
            i++;
            continue;
        }
        break;
    }

    while (i < argc) {
        if (Jim_UnsetVariable(interp, argv[i], complain ? JIM_ERRMSG : JIM_NONE) != JIM_OK
            && complain) {
            return JIM_ERR;
        }
        i++;
    }
    return JIM_OK;
}

/* [while] */
static Retval Jim_WhileCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "condition body");
        return JIM_ERR;
    }

    /* The general purpose implementation of while starts here */
    while (1) {
        int boolean, retval;

        if ((retval = Jim_GetBoolFromExpr(interp, argv[1], &boolean)) != JIM_OK)
            return retval;
        if (!boolean)
            break;

        if ((retval = Jim_EvalObj(interp, argv[2])) != JIM_OK) {
            switch (retval) {
                case JIM_BREAK:
                    goto out;
                    break;
                case JIM_CONTINUE:
                    continue;
                    break;
                default:
                    return retval;
            }
        }
    }
  out:
    Jim_SetEmptyResult(interp);
    return JIM_OK;
}

/* [for] */
STATIC Retval Jim_ForCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Retval retval;
    int boolean = 1;
    Jim_ObjPtr varNamePtr = NULL;
    Jim_ObjPtr stopVarNamePtr = NULL;

    if (argc != 5) {
        Jim_WrongNumArgs(interp, 1, argv, "start test next body");
        return JIM_ERR;
    }

    /* Do the initialisation */
    if ((retval = Jim_EvalObj(interp, argv[1])) != JIM_OK) {
        return retval;
    }

    /* And do the first test now. Better for optimisation
     * if we can do next/test at the bottom of the loop
     */
    retval = Jim_GetBoolFromExpr(interp, argv[2], &boolean);

    /* Ready to do the body as follows:
     * while (1) {
     *     body // check retcode
     *     next // check retcode
     *     test // check retcode/test bool
     * }
     */

    if (g_JIM_OPTIMIZATION_VAL) { // #Optimization
    /* Check if the for is on the form:
     *      for ... {$i < CONST} {incr i}
     *      for ... {$i < $j} {incr i}
     */
    if (retval == JIM_OK && boolean) {
        ScriptObj *incrScript;
        ExprTreePtr expr;
        jim_wide stop, currentVal;
        Jim_ObjPtr objPtr;
        int cmpOffset;

        /* Do it only if there aren't shared arguments */
        expr = JimGetExpression(interp, argv[2]);
        incrScript = JimGetScript(interp, argv[3]);

        /* Ensure proper lengths to start */
        if (incrScript == NULL || incrScript->len != 3 || !expr || expr->len != 3) {
            goto evalstart;
        }
        /* Ensure proper token types. */
        if (incrScript->token[1].type != JIM_TT_ESC) {
            goto evalstart; // #MissInCoverage
        }

        if (expr->expr->type == JIM_EXPROP_LT) {
            cmpOffset = 0;
        }
        else if (expr->expr->type == JIM_EXPROP_LTE) {
            cmpOffset = 1;
        }
        else {
            goto evalstart; // #MissInCoverage
        }

        if (expr->expr->left->type != JIM_TT_VAR) {
            goto evalstart; // #MissInCoverage
        }

        if (expr->expr->right->type != JIM_TT_VAR && expr->expr->right->type != JIM_TT_EXPR_INT) {
            goto evalstart; // #MissInCoverage
        }

        /* Update command must be incr */
        if (!Jim_CompareStringImmediate(interp, incrScript->token[1].objPtr, "incr")) {
            goto evalstart; // #MissInCoverage
        }

        /* incr, expression must be about the same variable */
        if (!Jim_StringEqObj(incrScript->token[2].objPtr, expr->expr->left->objPtr)) {
            goto evalstart; // #MissInCoverage
        }

        /* Get the stop condition (must be a variable or integer) */
        if (expr->expr->right->type == JIM_TT_EXPR_INT) {
            if (Jim_GetWide(interp, expr->expr->right->objPtr, &stop) == JIM_ERR) {
                goto evalstart; // #MissInCoverage
            }
        }
        else {
            stopVarNamePtr = expr->expr->right->objPtr;
            Jim_IncrRefCount(stopVarNamePtr);
            /* Keep the compiler happy */
            stop = 0;
        }

        /* Initialization */
        varNamePtr = expr->expr->left->objPtr;
        Jim_IncrRefCount(varNamePtr);

        objPtr = Jim_GetVariable(interp, varNamePtr, JIM_NONE);
        if (objPtr == NULL || Jim_GetWide(interp, objPtr, &currentVal) != JIM_OK) {
            goto testcond; // #MissInCoverage
        }

        /* --- OPTIMIZED FOR --- */
        while (retval == JIM_OK) {
            /* === Check condition === */
            /* Note that currentVal is already set here */

            /* Immediate or Variable? get the 'stop' value if the latter. */
            if (stopVarNamePtr) {
                objPtr = Jim_GetVariable(interp, stopVarNamePtr, JIM_NONE);
                if (objPtr == NULL || Jim_GetWide(interp, objPtr, &stop) != JIM_OK) {
                    goto testcond; // #MissInCoverage
                }
            }

            if (currentVal >= stop + cmpOffset) {
                break;
            }

            /* Eval body */
            retval = Jim_EvalObj(interp, argv[4]);
            if (retval == JIM_OK || retval == JIM_CONTINUE) {
                retval = JIM_OK;

                objPtr = Jim_GetVariable(interp, varNamePtr, JIM_ERRMSG);

                /* Increment */
                if (objPtr == NULL) {
                    retval = JIM_ERR; // #MissInCoverage
                    goto out;
                }
                if (!Jim_IsShared(objPtr) && objPtr->typePtr() == &g_intObjType) {
                    objPtr->incrWideValue();
                    currentVal = objPtr->wideValue();
                    Jim_InvalidateStringRep(objPtr);
                }
                else {
                    if (Jim_GetWide(interp, objPtr, &currentVal) != JIM_OK ||
                        Jim_SetVariable(interp, varNamePtr, Jim_NewIntObj(interp,
                                ++currentVal)) != JIM_OK) {
                        goto evalnext; // #MissInCoverage
                    }
                }
            }
        }
        goto out;
    }
  evalstart: ;
    }

    while (boolean && (retval == JIM_OK || retval == JIM_CONTINUE)) {
        /* Body */
        retval = Jim_EvalObj(interp, argv[4]);

        if (retval == JIM_OK || retval == JIM_CONTINUE) {
            /* increment */
JIM_IF_OPTIM(evalnext:)
            retval = Jim_EvalObj(interp, argv[3]);
            if (retval == JIM_OK || retval == JIM_CONTINUE) {
                /* test */
JIM_IF_OPTIM(testcond:)
                retval = Jim_GetBoolFromExpr(interp, argv[2], &boolean);
            }
        }
    }
JIM_IF_OPTIM(out:)
    if (stopVarNamePtr) {
        Jim_DecrRefCount(interp, stopVarNamePtr);
    }
    if (varNamePtr) {
        Jim_DecrRefCount(interp, varNamePtr);
    }

    if (retval == JIM_CONTINUE || retval == JIM_BREAK || retval == JIM_OK) {
        Jim_SetEmptyResult(interp);
        return JIM_OK;
    }

    return retval;
}

/* [loop] */
static Retval Jim_LoopCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Retval retval;
    jim_wide i;
    jim_wide limit;
    jim_wide incr = 1;
    Jim_ObjPtr bodyObjPtr;

    if (argc != 5 && argc != 6) {
        Jim_WrongNumArgs(interp, 1, argv, "var first limit ?incr? body");
        return JIM_ERR;
    }

    if (Jim_GetWide(interp, argv[2], &i) != JIM_OK ||
        Jim_GetWide(interp, argv[3], &limit) != JIM_OK ||
          (argc == 6 && Jim_GetWide(interp, argv[4], &incr) != JIM_OK)) {
        return JIM_ERR; // #MissInCoverage
    }
    bodyObjPtr = (argc == 5) ? argv[4] : argv[5];

    retval = Jim_SetVariable(interp, argv[1], argv[2]);

    while (((i < limit && incr > 0) || (i > limit && incr < 0)) && retval == JIM_OK) {
        retval = Jim_EvalObj(interp, bodyObjPtr);
        if (retval == JIM_OK || retval == JIM_CONTINUE) {
            Jim_ObjPtr objPtr = Jim_GetVariable(interp, argv[1], JIM_ERRMSG);

            retval = JIM_OK;

            /* Increment */
            i += incr;

            if (objPtr && !Jim_IsShared(objPtr) && objPtr->typePtr() == &g_intObjType) {
                if (argv[1]->typePtr() != &g_variableObjType) {
                    if (Jim_SetVariable(interp, argv[1], objPtr) != JIM_OK) {
                        return JIM_ERR; // #MissInCoverage
                    }
                }
                (objPtr)->setWideValue( i);
                Jim_InvalidateStringRep(objPtr);

                /* The following step is required in order to invalidate the
                 * string repr of "FOO" if the var name is of the form of "FOO(IDX)" */
                if (argv[1]->typePtr() != &g_variableObjType) {
                    if (Jim_SetVariable(interp, argv[1], objPtr) != JIM_OK) {
                        retval = JIM_ERR; // #MissInCoverage
                        break;
                    }
                }
            }
            else {
                objPtr = Jim_NewIntObj(interp, i);
                retval = Jim_SetVariable(interp, argv[1], objPtr);
                if (retval != JIM_OK) {
                    Jim_FreeNewObj(interp, objPtr);
                }
            }
        }
    }

    if (retval == JIM_OK || retval == JIM_CONTINUE || retval == JIM_BREAK) {
        Jim_SetEmptyResult(interp);
        return JIM_OK;
    }
    return retval;
}

/* List iterators make it easy to iterate over a list.
 * At some point iterators will be expanded to support generators.
 */
struct Jim_ListIter {
private:
    Jim_ObjPtr objPtr = NULL;
    int idx = 0;
public:

    friend STATIC void JimListIterInit(Jim_ListIterPtr iter, Jim_ObjPtr objPtr);
    friend STATIC Jim_ObjPtr JimListIterNext(Jim_InterpPtr interp, Jim_ListIterPtr iter);
    friend STATIC int JimListIterDone(Jim_InterpPtr interp, Jim_ListIterPtr iter);
};

#define free_Jim_ListIter(ptr)              Jim_TFree<Jim_ListIter>(ptr,"Jim_ListIter")

/**
 * Initialize the iterator at the start of the list.
 */
STATIC void JimListIterInit(Jim_ListIterPtr iter, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    iter->objPtr = objPtr;
    iter->idx = 0;
}

/**
 * Returns the next object from the list, or NULL on end-of-list.
 */
STATIC Jim_ObjPtr JimListIterNext(Jim_InterpPtr interp, Jim_ListIterPtr iter)
{
    PRJ_TRACE;
    if (iter->idx >= Jim_ListLength(interp, iter->objPtr)) {
        return NULL;
    }
    return iter->objPtr->get_listValue_objArray(iter->idx++);
}

/**
 * Returns 1 if end-of-list has been reached.
 */
STATIC int JimListIterDone(Jim_InterpPtr interp, Jim_ListIterPtr iter)
{
    return iter->idx >= Jim_ListLength(interp, iter->objPtr);
}

/* foreach + lmap implementation. */
static Retval JimForeachMapHelper(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv, int doMap)
{
    PRJ_TRACE;
    Retval result = JIM_OK;
    int i, numargs;
    Jim_ListIter twoiters[2];   /* Avoid allocation for a single list */
    Jim_ListIterPtr iters;
    Jim_ObjPtr script;
    Jim_ObjPtr resultObj;

    if (argc < 4 || argc % 2 != 0) {
        Jim_WrongNumArgs(interp, 1, argv, "varList list ?varList list ...? script");
        return JIM_ERR;
    }
    script = argv[argc - 1];    /* Last argument is a script */
    numargs = (argc - 1 - 1);    /* argc - 'foreach' - script */

    if (numargs == 2) {
        iters = twoiters;
    }
    else {
        iters = Jim_TAlloc<Jim_ListIter>(numargs,"Jim_ListIter");  // #AllocF 
    }
    for (i = 0; i < numargs; i++) {
        JimListIterInit(&iters[i], argv[i + 1]);
        if (i % 2 == 0 && JimListIterDone(interp, &iters[i])) {
            result = JIM_ERR;
        }
    }
    if (result != JIM_OK) {
        Jim_SetResultString(interp, "foreach varlist is empty", -1);
        goto empty_varlist;
    }

    if (doMap) {
        resultObj = Jim_NewListObj(interp, NULL, 0);
    }
    else {
        resultObj = interp->emptyObj();
    }
    Jim_IncrRefCount(resultObj);

    while (1) {
        /* Have we expired all lists? */
        for (i = 0; i < numargs; i += 2) {
            if (!JimListIterDone(interp, &iters[i + 1])) {
                break;
            }
        }
        if (i == numargs) {
            /* All done */
            break;
        }

        /* For each list */
        for (i = 0; i < numargs; i += 2) {
            Jim_ObjPtr varName;

            /* foreach var */
            JimListIterInit(&iters[i], argv[i + 1]);
            while ((varName = JimListIterNext(interp, &iters[i])) != NULL) {
                Jim_ObjPtr valObj = JimListIterNext(interp, &iters[i + 1]);
                if (!valObj) {
                    /* Ran out, so store the empty string */
                    valObj = interp->emptyObj();
                }
                /* Avoid shimmering */
                Jim_IncrRefCount(valObj);
                result = Jim_SetVariable(interp, varName, valObj);
                Jim_DecrRefCount(interp, valObj);
                if (result != JIM_OK) {
                    goto err; // #MissInCoverage
                }
            }
        }
        switch (result = Jim_EvalObj(interp, script)) {
            case JIM_OK:
                if (doMap) {
                    Jim_ListAppendElement(interp, resultObj, interp->result());
                }
                break;
            case JIM_CONTINUE:
                break;
            case JIM_BREAK:
                goto out;
            default:
                goto err;
        }
    }
  out:
    result = JIM_OK;
    Jim_SetResult(interp, resultObj);
  err:
    Jim_DecrRefCount(interp, resultObj);
  empty_varlist:
    if (numargs > 2) {
        free_Jim_ListIter(iters); // #FreeF
    }
    return result;
}

/* [foreach] */
static Retval Jim_ForeachCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    return JimForeachMapHelper(interp, argc, argv, 0);
}

/* [lmap] */
static Retval Jim_LmapCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    return JimForeachMapHelper(interp, argc, argv, 1);
}

/* [lassign] */
static Retval Jim_LassignCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd#JimCoreCmd 
{
    PRJ_TRACE;
    Retval result = JIM_ERR;
    int i;
    Jim_ListIter iter;
    Jim_ObjPtr resultObj;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "varList list ?varName ...?"); // #MissInCoverage
        return JIM_ERR;
    }

    JimListIterInit(&iter, argv[1]);

    for (i = 2; i < argc; i++) {
        Jim_ObjPtr valObj = JimListIterNext(interp, &iter);
        result = Jim_SetVariable(interp, argv[i], valObj ? valObj : interp->emptyObj());
        if (result != JIM_OK) {
            return result; // #MissInCoverage
        }
    }

    resultObj = Jim_NewListObj(interp, NULL, 0);
    while (!JimListIterDone(interp, &iter)) {
        Jim_ListAppendElement(interp, resultObj, JimListIterNext(interp, &iter));
    }

    Jim_SetResult(interp, resultObj);

    return JIM_OK;
}

/* [if] */
static Retval Jim_IfCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int boolean, current = 1, falsebody = 0; Retval retval;

    if (argc >= 3) {
        while (1) {
            /* Far not enough arguments given! */
            if (current >= argc)
                goto err;
            if ((retval = Jim_GetBoolFromExpr(interp, argv[current++], &boolean))
                != JIM_OK)
                return retval;
            /* There lacks something, isn't it? */
            if (current >= argc)
                goto err;
            if (Jim_CompareStringImmediate(interp, argv[current], "then"))
                current++;
            /* Tsk tsk, no then-clause? */
            if (current >= argc)
                goto err;
            if (boolean)
                return Jim_EvalObj(interp, argv[current]);
            /* Ok: no else-clause follows */
            if (++current >= argc) {
                Jim_SetResult(interp, Jim_NewEmptyStringObj(interp));
                return JIM_OK;
            }
            falsebody = current++;
            if (Jim_CompareStringImmediate(interp, argv[falsebody], "else")) {
                /* IIICKS - else-clause isn't last cmd? */
                if (current != argc - 1)
                    goto err;
                return Jim_EvalObj(interp, argv[current]);
            }
            else if (Jim_CompareStringImmediate(interp, argv[falsebody], "elseif"))
                /* Ok: elseif follows meaning all the stuff
                 * again (how boring...) */
                continue;
            /* OOPS - else-clause is not last cmd? */
            else if (falsebody != argc - 1)
                goto err;
            return Jim_EvalObj(interp, argv[falsebody]);
        }
        return JIM_OK;
    }
  err:
    Jim_WrongNumArgs(interp, 1, argv, "condition ?then? trueBody ?elseif ...? ?else? falseBody");
    return JIM_ERR;
}


/* Returns 1 if match, 0 if no match or -<error> on error (e.g. -JIM_ERR, -JIM_BREAK)*/
static int Jim_CommandMatchObj(Jim_InterpPtr interp, Jim_ObjPtr commandObj, Jim_ObjPtr patternObj,
    Jim_ObjPtr stringObj, int nocase)
{
    PRJ_TRACE;
    Jim_ObjPtr parms[4];
    int argc = 0;
    long eq;
    int rc;

    parms[argc++] = commandObj;
    if (nocase) {
        parms[argc++] = Jim_NewStringObj(interp, "-nocase", -1);
    }
    parms[argc++] = patternObj;
    parms[argc++] = stringObj;

    rc = Jim_EvalObjVector(interp, argc, parms);

    if (rc != JIM_OK || Jim_GetLong(interp, Jim_GetResult(interp), &eq) != JIM_OK) {
        eq = -rc;
    }

    return eq;
}

/* [switch] */
static Retval Jim_SwitchCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    enum { SWITCH_EXACT, SWITCH_GLOB, SWITCH_RE, SWITCH_CMD };
    int matchOpt = SWITCH_EXACT, opt = 1, patCount, i;
    Jim_Obj *command = NULL, *scriptObj = NULL, *strObj;
    Jim_ObjArray* caseList;

    if (argc < 3) {
      wrongnumargs:
        Jim_WrongNumArgs(interp, 1, argv, "?options? string "
            "pattern body ... ?default body?   or   " "{pattern body ?pattern body ...?}");
        return JIM_ERR;
    }
    for (opt = 1; opt < argc; ++opt) {
        const char *option = Jim_String(argv[opt]);

        if (*option != '-')
            break;
        else if (strncmp(option, "--", 2) == 0) {
            ++opt;
            break;
        }
        else if (strncmp(option, "-exact", 2) == 0)
            matchOpt = SWITCH_EXACT;
        else if (strncmp(option, "-glob", 2) == 0)
            matchOpt = SWITCH_GLOB;
        else if (strncmp(option, "-regexp", 2) == 0)
            matchOpt = SWITCH_RE;
        else if (strncmp(option, "-command", 2) == 0) {
            matchOpt = SWITCH_CMD;
            if ((argc - opt) < 2)
                goto wrongnumargs; // #MissInCoverage
            command = argv[++opt];
        }
        else {
            Jim_SetResultFormatted(interp,
                "bad option \"%#s\": must be -exact, -glob, -regexp, -command procname or --",
                argv[opt]);
            return JIM_ERR;
        }
        if ((argc - opt) < 2)
            goto wrongnumargs; // #MissInCoverage
    }
    strObj = argv[opt++];
    patCount = argc - opt;
    if (patCount == 1) {
        JimListGetElements(interp, argv[opt], &patCount, &caseList);
    }
    else
        caseList = (Jim_ObjArray* )&argv[opt];
    if (patCount == 0 || patCount % 2 != 0)
        goto wrongnumargs;
    for (i = 0; scriptObj == NULL && i < patCount; i += 2) {
        Jim_ObjPtr patObj = caseList[i];

        if (!Jim_CompareStringImmediate(interp, patObj, "default")
            || i < (patCount - 2)) {
            switch (matchOpt) {
                case SWITCH_EXACT:
                    if (Jim_StringEqObj(strObj, patObj))
                        scriptObj = caseList[i + 1];
                    break;
                case SWITCH_GLOB:
                    if (Jim_StringMatchObj(interp, patObj, strObj, 0))
                        scriptObj = caseList[i + 1];
                    break;
                case SWITCH_RE:
                    command = Jim_NewStringObj(interp, "regexp", -1);
                    /* Fall thru intentionally */
                case SWITCH_CMD:{
                        int rc = Jim_CommandMatchObj(interp, command, patObj, strObj, 0);

                        /* After the execution of a command we need to
                         * make sure to reconvert the object into a list
                         * again. Only for the single-list style [switch]. */
                        if (argc - opt == 1) {
                            JimListGetElements(interp, argv[opt], &patCount, &caseList);
                        }
                        /* command is here already decref'd */
                        if (rc < 0) {
                            return -rc;
                        }
                        if (rc)
                            scriptObj = caseList[i + 1];
                        break;
                    }
            }
        }
        else {
            scriptObj = caseList[i + 1];
        }
    }
    for (; i < patCount && Jim_CompareStringImmediate(interp, scriptObj, "-"); i += 2)
        scriptObj = caseList[i + 1];
    if (scriptObj && Jim_CompareStringImmediate(interp, scriptObj, "-")) {
        Jim_SetResultFormatted(interp, "no body specified for pattern \"%#s\"", caseList[i - 2]);
        return JIM_ERR;
    }
    Jim_SetEmptyResult(interp);
    if (scriptObj) {
        return Jim_EvalObj(interp, scriptObj);
    }
    return JIM_OK;
}

/* [list] */
static Retval Jim_ListCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr listObjPtr;

    listObjPtr = Jim_NewListObj(interp, argv + 1, argc - 1);
    Jim_SetResult(interp, listObjPtr);
    return JIM_OK;
}

/* [lindex] */
static Retval Jim_LindexCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_Obj *objPtr, *listObjPtr;
    int i;
    int idx;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "list ?index ...?");
        return JIM_ERR;
    }
    objPtr = argv[1];
    Jim_IncrRefCount(objPtr);
    for (i = 2; i < argc; i++) {
        listObjPtr = objPtr;
        if (Jim_GetIndex(interp, argv[i], &idx) != JIM_OK) {
            Jim_DecrRefCount(interp, listObjPtr);
            return JIM_ERR;
        }
        if (Jim_ListIndex(interp, listObjPtr, idx, &objPtr, JIM_NONE) != JIM_OK) {
            /* Returns an empty object if the index
             * is out of range. */
            Jim_DecrRefCount(interp, listObjPtr);
            Jim_SetEmptyResult(interp);
            return JIM_OK;
        }
        Jim_IncrRefCount(objPtr);
        Jim_DecrRefCount(interp, listObjPtr);
    }
    Jim_SetResult(interp, objPtr);
    Jim_DecrRefCount(interp, objPtr);
    return JIM_OK;
}

/* [llength] */
static Retval Jim_LlengthCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "list");
        return JIM_ERR;
    }
    Jim_SetResultInt(interp, Jim_ListLength(interp, argv[1]));
    return JIM_OK;
}

/* [lsearch] */
static Retval Jim_LsearchCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    static const char * const options[] = {
        "-bool", "-not", "-nocase", "-exact", "-glob", "-regexp", "-all", "-inline", "-command",
            NULL
    };
    enum
    { OPT_BOOL, OPT_NOT, OPT_NOCASE, OPT_EXACT, OPT_GLOB, OPT_REGEXP, OPT_ALL, OPT_INLINE,
            OPT_COMMAND };
    int i;
    int opt_bool = 0;
    int opt_not = 0;
    int opt_nocase = 0;
    int opt_all = 0;
    int opt_inline = 0;
    int opt_match = OPT_EXACT;
    int listlen;
    Retval rc = JIM_OK;
    Jim_ObjPtr listObjPtr = NULL;
    Jim_ObjPtr commandObj = NULL;

    if (argc < 3) {
      wrongargs:
        Jim_WrongNumArgs(interp, 1, argv,
            "?-exact|-glob|-regexp|-command 'command'? ?-bool|-inline? ?-not? ?-nocase? ?-all? list value");
        return JIM_ERR;
    }

    for (i = 1; i < argc - 2; i++) {
        int option;

        if (Jim_GetEnum(interp, argv[i], options, &option, NULL, JIM_ERRMSG) != JIM_OK) {
            return JIM_ERR;
        }
        switch (option) {
            case OPT_BOOL:
                opt_bool = 1;
                opt_inline = 0;
                break;
            case OPT_NOT:
                opt_not = 1;
                break;
            case OPT_NOCASE:
                opt_nocase = 1;
                break;
            case OPT_INLINE:
                opt_inline = 1;
                opt_bool = 0;
                break;
            case OPT_ALL:
                opt_all = 1;
                break;
            case OPT_COMMAND:
                if (i >= argc - 2) { // #MissInCoverage
                    goto wrongargs;
                }
                commandObj = argv[++i]; 
                /* fallthru */
            case OPT_EXACT:
            case OPT_GLOB:
            case OPT_REGEXP:
                opt_match = option;
                break;
        }
    }

    argv += i;

    if (opt_all) {
        listObjPtr = Jim_NewListObj(interp, NULL, 0);
    }
    if (opt_match == OPT_REGEXP) {
        commandObj = Jim_NewStringObj(interp, "regexp", -1);
    }
    if (commandObj) {
        Jim_IncrRefCount(commandObj);
    }

    listlen = Jim_ListLength(interp, argv[0]);
    for (i = 0; i < listlen; i++) {
        int eq = 0;
        Jim_ObjPtr objPtr = Jim_ListGetIndex(interp, argv[0], i);

        switch (opt_match) {
            case OPT_EXACT:
                eq = Jim_StringCompareObj(interp, argv[1], objPtr, opt_nocase) == 0;
                break;

            case OPT_GLOB:
                eq = Jim_StringMatchObj(interp, argv[1], objPtr, opt_nocase);
                break;

            case OPT_REGEXP:
            case OPT_COMMAND:
                eq = Jim_CommandMatchObj(interp, commandObj, argv[1], objPtr, opt_nocase);
                if (eq < 0) {
                    if (listObjPtr) {
                        Jim_FreeNewObj(interp, listObjPtr);
                    }
                    rc = JIM_ERR;
                    goto done;
                }
                break;
        }

        /* If we have a non-match with opt_bool, opt_not, !opt_all, can't exit early */
        if (!eq && opt_bool && opt_not && !opt_all) {
            continue;
        }

        if ((!opt_bool && eq == !opt_not) || (opt_bool && (eq || opt_all))) {
            /* Got a match (or non-match for opt_not), or (opt_bool && opt_all) */
            Jim_ObjPtr resultObj;

            if (opt_bool) {
                resultObj = Jim_NewIntObj(interp, eq ^ opt_not);
            }
            else if (!opt_inline) {
                resultObj = Jim_NewIntObj(interp, i);
            }
            else {
                resultObj = objPtr;
            }

            if (opt_all) {
                Jim_ListAppendElement(interp, listObjPtr, resultObj);
            }
            else {
                Jim_SetResult(interp, resultObj);
                goto done;
            }
        }
    }

    if (opt_all) {
        Jim_SetResult(interp, listObjPtr);
    }
    else {
        /* No match */
        if (opt_bool) {
            Jim_SetResultBool(interp, opt_not);
        }
        else if (!opt_inline) {
            Jim_SetResultInt(interp, -1);
        }
    }

  done:
    if (commandObj) {
        Jim_DecrRefCount(interp, commandObj);
    }
    return rc;
}

/* [lappend] */
static Retval Jim_LappendCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr listObjPtr;
    int new_obj = 0;
    int i;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "varName ?value value ...?");
        return JIM_ERR;
    }
    listObjPtr = Jim_GetVariable(interp, argv[1], JIM_UNSHARED);
    if (!listObjPtr) {
        /* Create the list if it does not exist */
        listObjPtr = Jim_NewListObj(interp, NULL, 0);
        new_obj = 1;
    }
    else if (Jim_IsShared(listObjPtr)) {
        listObjPtr = Jim_DuplicateObj(interp, listObjPtr);
        new_obj = 1;
    }
    for (i = 2; i < argc; i++)
        Jim_ListAppendElement(interp, listObjPtr, argv[i]);
    if (Jim_SetVariable(interp, argv[1], listObjPtr) != JIM_OK) {
        if (new_obj)
            Jim_FreeNewObj(interp, listObjPtr);
        return JIM_ERR;
    }
    Jim_SetResult(interp, listObjPtr);
    return JIM_OK;
}

/* [linsert] */
static Retval Jim_LinsertCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int idx, len;
    Jim_ObjPtr listPtr;

    if (argc < 3) {
        Jim_WrongNumArgs(interp, 1, argv, "list index ?element ...?");
        return JIM_ERR;
    }
    listPtr = argv[1];
    if (Jim_IsShared(listPtr))
        listPtr = Jim_DuplicateObj(interp, listPtr);
    if (Jim_GetIndex(interp, argv[2], &idx) != JIM_OK)
        goto err;
    len = Jim_ListLength(interp, listPtr);
    if (idx >= len)
        idx = len;
    else if (idx < 0)
        idx = len + idx + 1;
    Jim_ListInsertElements(interp, listPtr, idx, argc - 3, &argv[3]);
    Jim_SetResult(interp, listPtr);
    return JIM_OK;
  err:
    if (listPtr != argv[1]) {
        Jim_FreeNewObj(interp, listPtr);
    }
    return JIM_ERR;
}

/* [lreplace] */
STATIC Retval Jim_LreplaceCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int first, last, len, rangeLen;
    Jim_ObjPtr listObj;
    Jim_ObjPtr newListObj;

    if (argc < 4) {
        Jim_WrongNumArgs(interp, 1, argv, "list first last ?element ...?");
        return JIM_ERR;
    }
    if (Jim_GetIndex(interp, argv[2], &first) != JIM_OK ||
        Jim_GetIndex(interp, argv[3], &last) != JIM_OK) {
        return JIM_ERR;
    }

    listObj = argv[1];
    len = Jim_ListLength(interp, listObj);

    first = JimRelToAbsIndex(len, first);
    last = JimRelToAbsIndex(len, last);
    JimRelToAbsRange(len, &first, &last, &rangeLen);

    /* Now construct a new list which consists of:
     * <elements before first> <supplied elements> <elements after last>
     */

    /* Trying to replace past the end of the list means end of list
     * See TIP #505
     */
    if (first > len) {
        first = len;
    }

    /* Add the first set of elements */
    newListObj = Jim_NewListObj(interp, listObj->get_listValue_ele(), first); 

    /* Add supplied elements */
    ListInsertElements(newListObj, -1, argc - 4, argv + 4);

    /* Add the remaining elements */
    ListInsertElements(newListObj, -1, len - first - rangeLen, listObj->get_listValue_ele() + first + rangeLen); 

    Jim_SetResult(interp, newListObj);
    return JIM_OK;
}

/* [lset] */
static Retval Jim_LsetCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc < 3) {
        Jim_WrongNumArgs(interp, 1, argv, "listVar ?index...? newVal"); // #MissInCoverage
        return JIM_ERR;
    }
    else if (argc == 3) {
        /* With no indexes, simply implements [set] */
        if (Jim_SetVariable(interp, argv[1], argv[2]) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
        Jim_SetResult(interp, argv[2]);
        return JIM_OK;
    }
    return Jim_ListSetIndex(interp, argv[1], argv + 2, argc - 3, argv[argc - 1]);
}

/* [lsort] */
static Retval Jim_LsortCoreCommand(Jim_InterpPtr interp, int argc, Jim_Obj *const argv[]) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    static const char * const options[] = {
        "-ascii", "-nocase", "-increasing", "-decreasing", "-command", "-integer", "-real", "-index", "-unique", NULL
    };
    enum
    { OPT_ASCII, OPT_NOCASE, OPT_INCREASING, OPT_DECREASING, OPT_COMMAND, OPT_INTEGER, OPT_REAL, OPT_INDEX, OPT_UNIQUE };
    Jim_ObjPtr resObj;
    int i;
    Retval retCode;
    int shared;

    lsort_info info;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "?options? list");
        return JIM_ERR;
    }

    info.type = JIM_LSORT_ASCII;
    info.order = 1;
    info.indexed = 0;
    info.unique = 0;
    info.command = NULL;
    info.interp = interp;

    for (i = 1; i < (argc - 1); i++) {
        int option;

        if (Jim_GetEnum(interp, argv[i], options, &option, NULL, JIM_ENUM_ABBREV | JIM_ERRMSG)
            != JIM_OK)
            return JIM_ERR;
        switch (option) {
            case OPT_ASCII:
                info.type = JIM_LSORT_ASCII;
                break;
            case OPT_NOCASE:
                info.type = JIM_LSORT_NOCASE;
                break;
            case OPT_INTEGER:
                info.type = JIM_LSORT_INTEGER;
                break;
            case OPT_REAL:
                info.type = JIM_LSORT_REAL;
                break;
            case OPT_INCREASING:
                info.order = 1;
                break;
            case OPT_DECREASING:
                info.order = -1;
                break;
            case OPT_UNIQUE:
                info.unique = 1;
                break;
            case OPT_COMMAND:
                if (i >= (argc - 2)) {
                    Jim_SetResultString(interp, "\"-command\" option must be followed by comparison command", -1);
                    return JIM_ERR;
                }
                info.type = JIM_LSORT_COMMAND;
                info.command = argv[i + 1];
                i++;
                break;
            case OPT_INDEX:
                if (i >= (argc - 2)) {
                    Jim_SetResultString(interp, "\"-index\" option must be followed by list index", -1);
                    return JIM_ERR;
                }
                if (Jim_GetIndex(interp, argv[i + 1], &info.index) != JIM_OK) {
                    return JIM_ERR;
                }
                info.indexed = 1;
                i++;
                break;
        }
    }
    resObj = argv[argc - 1];
    if ((shared = Jim_IsShared(resObj)))
        resObj = Jim_DuplicateObj(interp, resObj);
    retCode = ListSortElements(interp, resObj, &info);
    if (retCode == JIM_OK) {
        Jim_SetResult(interp, resObj);
    }
    else if (shared) {
        Jim_FreeNewObj(interp, resObj);
    }
    return retCode;
}

/* [append] */
static Retval Jim_AppendCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr stringObjPtr;
    int i;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "varName ?value ...?");
        return JIM_ERR;
    }
    if (argc == 2) {
        stringObjPtr = Jim_GetVariable(interp, argv[1], JIM_ERRMSG);
        if (!stringObjPtr)
            return JIM_ERR;
    }
    else {
        int new_obj = 0;
        stringObjPtr = Jim_GetVariable(interp, argv[1], JIM_UNSHARED);
        if (!stringObjPtr) {
            /* Create the string if it doesn't exist */
            stringObjPtr = Jim_NewEmptyStringObj(interp);
            new_obj = 1;
        }
        else if (Jim_IsShared(stringObjPtr)) {
            new_obj = 1;
            stringObjPtr = Jim_DuplicateObj(interp, stringObjPtr);
        }
        for (i = 2; i < argc; i++) {
            Jim_AppendObj(interp, stringObjPtr, argv[i]);
        }
        if (Jim_SetVariable(interp, argv[1], stringObjPtr) != JIM_OK) {
            if (new_obj) {
                Jim_FreeNewObj(interp, stringObjPtr);
            }
            return JIM_ERR;
        }
    }
    Jim_SetResult(interp, stringObjPtr);
    return JIM_OK;
}

/**
 * Returns a zero-refcount list describing the expression at 'node'
 */
STATIC Jim_ObjPtr JimGetExprAsList(Jim_InterpPtr interp, JimExprNodePtr node) // #MissInCoverage
{
    PRJ_TRACE;
    Jim_ObjPtr listObjPtr = Jim_NewListObj(interp, NULL, 0);

    Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, jim_tt_name(node->type), -1));
    if (TOKEN_IS_EXPR_OP(node->type)) {
        if (node->left) {
            Jim_ListAppendElement(interp, listObjPtr, JimGetExprAsList(interp, node->left));
        }
        if (node->right) {
            Jim_ListAppendElement(interp, listObjPtr, JimGetExprAsList(interp, node->right));
        }
        if (node->ternary) {
            Jim_ListAppendElement(interp, listObjPtr, JimGetExprAsList(interp, node->ternary));
        }
    }
    else {
        Jim_ListAppendElement(interp, listObjPtr, node->objPtr);
    }
    return listObjPtr;
}

/* [debug] */
STATIC Retval Jim_DebugCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #MissInCoverage #JimCoreCmd 
{
    PRJ_TRACE;
    if (g_JIM_DEBUG_COMMAND && g_JIM_BOOTSTRAP_VAL) {
    static const char * const options[] = {
        "refcount", "objcount", "objects", "invstr", "scriptlen", "exprlen",
        "exprbc", "show",
        NULL
    };
    enum
    {
        OPT_REFCOUNT, OPT_OBJCOUNT, OPT_OBJECTS, OPT_INVSTR, OPT_SCRIPTLEN,
        OPT_EXPRLEN, OPT_EXPRBC, OPT_SHOW,
    };
    int option;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "subcommand ?...?");
        return JIM_ERR;
    }
    if (Jim_GetEnum(interp, argv[1], options, &option, "subcommand", JIM_ERRMSG) != JIM_OK)
        return Jim_CheckShowCommands(interp, argv[1], options);
    if (option == OPT_REFCOUNT) {
        if (argc != 3) {
            Jim_WrongNumArgs(interp, 2, argv, "object");
            return JIM_ERR;
        }
        Jim_SetResultInt(interp, argv[2]->refCount());
        return JIM_OK;
    }
    else if (option == OPT_OBJCOUNT) {
        int freeobj = 0, liveobj = 0;
        char buf[256]; // #MagicNum
        Jim_ObjPtr objPtr;

        if (argc != 2) {
            Jim_WrongNumArgs(interp, 2, argv, "");
            return JIM_ERR;
        }
        /* Count the number of free objects. */
        objPtr = interp->freeList();
        while (objPtr) {
            freeobj++;
            objPtr = objPtr->nextObjPtr();
        }
        /* Count the number of live objects. */
        objPtr = interp->liveList();
        while (objPtr) {
            liveobj++;
            objPtr = objPtr->nextObjPtr();
        }
        /* Set the result string and return. */
        sprintf(buf, "free %d used %d", freeobj, liveobj);
        Jim_SetResultString(interp, buf, -1);
        return JIM_OK;
    }
    else if (option == OPT_OBJECTS) {
        Jim_Obj* objPtr, *listObjPtr, *subListObjPtr;

        /* Count the number of live objects. */
        objPtr = interp->liveList();
        listObjPtr = Jim_NewListObj(interp, NULL, 0);
        while (objPtr) {
            char buf[128]; // #MagicNum
            const char *type = objPtr->typePtr() ? objPtr->typePtr()->name : "";

            subListObjPtr = Jim_NewListObj(interp, NULL, 0);
            sprintf(buf, "%p", objPtr);
            Jim_ListAppendElement(interp, subListObjPtr, Jim_NewStringObj(interp, buf, -1));
            Jim_ListAppendElement(interp, subListObjPtr, Jim_NewStringObj(interp, type, -1));
            Jim_ListAppendElement(interp, subListObjPtr, Jim_NewIntObj(interp, objPtr->refCount()));
            Jim_ListAppendElement(interp, subListObjPtr, objPtr);
            Jim_ListAppendElement(interp, listObjPtr, subListObjPtr);
            objPtr = objPtr->nextObjPtr();
        }
        Jim_SetResult(interp, listObjPtr);
        return JIM_OK;
    }
    else if (option == OPT_INVSTR) {
        Jim_ObjPtr objPtr;

        if (argc != 3) {
            Jim_WrongNumArgs(interp, 2, argv, "object");
            return JIM_ERR;
        }
        objPtr = argv[2];
        if (objPtr->typePtr() != NULL)
            Jim_InvalidateStringRep(objPtr);
        Jim_SetEmptyResult(interp);
        return JIM_OK;
    }
    else if (option == OPT_SHOW) {
        const char *s;
        int len, charlen;

        if (argc != 3) {
            Jim_WrongNumArgs(interp, 2, argv, "object");
            return JIM_ERR;
        }
        s = Jim_GetString(argv[2], &len);
        if (g_JIM_UTF8_VAL) {
            charlen = utf8_strlen(s, len);
        } else {
            charlen = len;
        }
        printf("refcount: %d, type: %s\n", argv[2]->refCount(), JimObjTypeName(argv[2])); // #stdoutput
        printf("chars (%d): <<%s>>\n", charlen, s); // #stdoutput
        printf("bytes (%d):", len); // #stdoutput
        while (len--) {
            printf(" %02x", (unsigned_char)*s++); // #stdoutput
        }
        printf("\n"); // #stdoutput
        return JIM_OK;
    }
    else if (option == OPT_SCRIPTLEN) {
        ScriptObj *script;

        if (argc != 3) {
            Jim_WrongNumArgs(interp, 2, argv, "script");
            return JIM_ERR;
        }
        script = JimGetScript(interp, argv[2]);
        if (script == NULL)
            return JIM_ERR;
        Jim_SetResultInt(interp, script->len);
        return JIM_OK;
    }
    else if (option == OPT_EXPRLEN) {
        ExprTreePtr expr;

        if (argc != 3) {
            Jim_WrongNumArgs(interp, 2, argv, "expression");
            return JIM_ERR;
        }
        expr = JimGetExpression(interp, argv[2]);
        if (expr == NULL)
            return JIM_ERR;
        Jim_SetResultInt(interp, expr->len);
        return JIM_OK;
    }
    else if (option == OPT_EXPRBC) {
        ExprTreePtr expr;

        if (argc != 3) {
            Jim_WrongNumArgs(interp, 2, argv, "expression");
            return JIM_ERR;
        }
        expr = JimGetExpression(interp, argv[2]);
        if (expr == NULL)
            return JIM_ERR;
        Jim_SetResult(interp, JimGetExprAsList(interp, expr->expr));
        return JIM_OK;
    }
    else {
        Jim_SetResultString(interp,
            "bad option. Valid options are refcount, " "objcount, objects, invstr", -1);
        return JIM_ERR;
    }
    /* unreached */
    } /* JIM_DEBUG_COMMAND && !JIM_BOOTSTRAP */
    if (!g_JIM_DEBUG_COMMAND) {
        Jim_SetResultString(interp, "unsupported", -1);
    }
    return JIM_ERR;
}

/* [eval] */
static Retval Jim_EvalCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Retval rc;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "arg ?arg ...?"); // #MissInCoverage
        return JIM_ERR;
    }

    if (argc == 2) {
        rc = Jim_EvalObj(interp, argv[1]);
    }
    else {
        rc = Jim_EvalObj(interp, Jim_ConcatObj(interp, argc - 1, argv + 1));
    }

    if (rc == JIM_ERR) {
        /* eval is "interesting", so add a stack frame here */
        interp->incrAddStackTrace();
    }
    return rc;
}

/* [uplevel] */
static int Jim_UplevelCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc >= 2) {
        int retcode;
        Jim_CallFrame *savedCallFrame, *targetCallFrame;
        const char *str;

        /* Save the old callframe pointer */
        savedCallFrame = interp->framePtr();

        /* Lookup the target frame pointer */
        str = Jim_String(argv[1]);
        if ((str[0] >= '0' && str[0] <= '9') || str[0] == '#') {
            targetCallFrame = Jim_GetCallFrameByLevel(interp, argv[1]);
            argc--;
            argv++;
        }
        else {
            targetCallFrame = Jim_GetCallFrameByLevel(interp, NULL);
        }
        if (targetCallFrame == NULL) {
            return JIM_ERR;
        }
        if (argc < 2) {
            Jim_WrongNumArgs(interp, 1, argv - 1, "?level? command ?arg ...?");
            return JIM_ERR;
        }
        /* Eval the code in the target callframe. */
        interp->framePtr(targetCallFrame);
        if (argc == 2) {
            retcode = Jim_EvalObj(interp, argv[1]);
        }
        else {
            retcode = Jim_EvalObj(interp, Jim_ConcatObj(interp, argc - 1, argv + 1));
        }
        interp->framePtr(savedCallFrame);
        return retcode;
    }
    else {
        Jim_WrongNumArgs(interp, 1, argv, "?level? command ?arg ...?");
        return JIM_ERR;
    }
}

/* [expr] */
static Retval Jim_ExprCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Retval retcode;

    if (argc == 2) {
        retcode = Jim_EvalExpression(interp, argv[1]);
    }
    else if (argc > 2) {
        Jim_ObjPtr objPtr;

        objPtr = Jim_ConcatObj(interp, argc - 1, argv + 1);
        Jim_IncrRefCount(objPtr);
        retcode = Jim_EvalExpression(interp, objPtr);
        Jim_DecrRefCount(interp, objPtr);
    }
    else {
        Jim_WrongNumArgs(interp, 1, argv, "expression ?...?");
        return JIM_ERR;
    }
    if (retcode != JIM_OK)
        return retcode;
    return JIM_OK;
}

/* [break] */
static Retval Jim_BreakCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 1) {
        Jim_WrongNumArgs(interp, 1, argv, "");
        return JIM_ERR;
    }
    return JIM_BREAK;
}

/* [continue] */
static Retval Jim_ContinueCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 1) {
        Jim_WrongNumArgs(interp, 1, argv, "");
        return JIM_ERR;
    }
    return JIM_CONTINUE;
}

/* [return] */
static Retval Jim_ReturnCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int i;
    Jim_ObjPtr stackTraceObj = NULL;
    Jim_ObjPtr errorCodeObj = NULL;
    Retval returnCode = JIM_OK;
    long level = 1;

    for (i = 1; i < argc - 1; i += 2) {
        if (Jim_CompareStringImmediate(interp, argv[i], "-code")) {
            if (Jim_GetReturnCode(interp, argv[i + 1], &returnCode) == JIM_ERR) {
                return JIM_ERR;
            }
        }
        else if (Jim_CompareStringImmediate(interp, argv[i], "-errorinfo")) {
            stackTraceObj = argv[i + 1];
        }
        else if (Jim_CompareStringImmediate(interp, argv[i], "-errorcode")) {
            errorCodeObj = argv[i + 1];
        }
        else if (Jim_CompareStringImmediate(interp, argv[i], "-level")) {
            if (Jim_GetLong(interp, argv[i + 1], &level) != JIM_OK || level < 0) {
                Jim_SetResultFormatted(interp, "bad level \"%#s\"", argv[i + 1]); // #MissInCoverage
                return JIM_ERR;
            }
        }
        else {
            break; // #MissInCoverage
        }
    }

    if (i != argc - 1 && i != argc) {
        Jim_WrongNumArgs(interp, 1, argv,
            "?-code code? ?-errorinfo stacktrace? ?-level level? ?result?"); // #MissInCoverage
    }

    /* If a stack trace is supplied and code is error, set the stack trace */
    if (stackTraceObj && returnCode == JIM_ERR) {
        JimSetStackTrace(interp, stackTraceObj);
    }
    /* If an error code list is supplied, set the global $errorCode */
    if (errorCodeObj && returnCode == JIM_ERR) {
        Jim_SetGlobalVariableStr(interp, "errorCode", errorCodeObj);
    }
    interp->setReturnCode(returnCode);
    interp->setReturnLevel(level);

    if (i == argc - 1) {
        Jim_SetResult(interp, argv[i]);
    }
    return JIM_RETURN;
}

/* [tailcall] */
static Retval Jim_TailcallCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (interp->framePtr()->level() == 0) {
        Jim_SetResultString(interp, "tailcall can only be called from a proc or lambda", -1); // #MissInCoverage
        return JIM_ERR;
    }
    else if (argc >= 2) {
        /* Need to resolve the tailcall command in the current context */
        Jim_CallFramePtr cf = interp->framePtr()->parent();

        Jim_Cmd *cmdPtr = Jim_GetCommand(interp, argv[1], JIM_ERRMSG);
        if (cmdPtr == NULL) {
            return JIM_ERR;
        }

        JimPanic((cf->tailcallCmd != NULL, "Already have a tailcallCmd"));

        /* And stash this pre-resolved command */
        JimIncrCmdRefCount(cmdPtr);
        cf->tailcallCmd_ = cmdPtr;

        /* And stash the command list */
        JimPanic((cf->tailcallObj != NULL, "Already have a tailcallobj"));

        cf->tailcallObj_ = Jim_NewListObj(interp, argv + 1, argc - 1);
        Jim_IncrRefCount(cf->tailcallObj_);

        /* When the stack unwinds to the previous proc, the stashed command will be evaluated */
        return JIM_EVAL;
    }
    return JIM_OK; // #MissInCoverage
}

static Retval JimAliasCmd(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimAlias
{
    PRJ_TRACE;
    Jim_ObjPtr cmdList;
    Jim_ObjPtr prefixListObj = (Jim_ObjPtr )Jim_CmdPrivData(interp);

    /* prefixListObj is a list to which the args need to be appended */
    cmdList = Jim_DuplicateObj(interp, prefixListObj);
    Jim_ListInsertElements(interp, cmdList, Jim_ListLength(interp, cmdList), argc - 1, argv + 1);

    return JimEvalObjList(interp, cmdList);
}

static void JimAliasCmdDelete(Jim_InterpPtr interp, void *privData) // #JimAlias
{
    PRJ_TRACE;
    Jim_ObjPtr prefixListObj = (Jim_ObjPtr )privData;
    Jim_DecrRefCount(interp, prefixListObj);
}

static Retval Jim_AliasCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd #JimAlias
{
    PRJ_TRACE;
    Jim_ObjPtr prefixListObj;
    const char *newname;

    if (argc < 3) {
        Jim_WrongNumArgs(interp, 1, argv, "newname command ?args ...?"); // #MissInCoverage
        return JIM_ERR;
    }

    prefixListObj = Jim_NewListObj(interp, argv + 2, argc - 2);
    Jim_IncrRefCount(prefixListObj);
    newname = Jim_String(argv[1]);
    if (newname[0] == ':' && newname[1] == ':') {
        while (*++newname == ':') { // #MissInCoverage
        }
    }

    Jim_SetResult(interp, argv[1]);

    return Jim_CreateCommand(interp, newname, JimAliasCmd, prefixListObj, JimAliasCmdDelete);
}

/* [proc] */
static Retval Jim_ProcCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_Cmd *cmd;

    if (argc != 4 && argc != 5) {
        Jim_WrongNumArgs(interp, 1, argv, "name arglist ?statics? body");
        return JIM_ERR;
    }

    if (JimValidName(interp, "procedure", argv[1]) != JIM_OK) {
        return JIM_ERR; // #MissInCoverage
    }

    if (argc == 4) {
        cmd = JimCreateProcedureCmd(interp, argv[2], NULL, argv[3], NULL);
    }
    else {
        cmd = JimCreateProcedureCmd(interp, argv[2], argv[3], argv[4], NULL);
    }

    if (cmd) {
        /* Add the new command */
        Jim_ObjPtr qualifiedCmdNameObj;
        const char *cmdname = JimQualifyName(interp, Jim_String(argv[1]), &qualifiedCmdNameObj);

        JimCreateCommand(interp, cmdname, cmd);

        /* Calculate and set the namespace for this proc */
        JimUpdateProcNamespace(interp, cmd, cmdname);

        JimFreeQualifiedName(interp, qualifiedCmdNameObj);

        /* Unlike Tcl, set the name of the proc as the result */
        Jim_SetResult(interp, argv[1]);
        return JIM_OK;
    }
    return JIM_ERR;
}

/* [local] */
static Retval Jim_LocalCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Retval retcode;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "cmd ?args ...?"); // #MissInCoverage
        return JIM_ERR;
    }

    /* Evaluate the arguments with 'local' in force */
    interp->incrLocal();
    retcode = Jim_EvalObjVector(interp, argc - 1, argv + 1);
    interp->decrLocal();


    /* If OK, and the result is a proc, add it to the list of local procs */
    if (retcode == 0) {
        Jim_ObjPtr cmdNameObj = Jim_GetResult(interp);

        if (Jim_GetCommand(interp, cmdNameObj, JIM_ERRMSG) == NULL) {
            return JIM_ERR;
        }
        if (interp->framePtr()->localCommands() == NULL) {
            interp->framePtr()->localCommands_ = new_Jim_Stack; // #AllocF 
            Jim_InitStack(interp->framePtr()->localCommands());
        }
        Jim_IncrRefCount(cmdNameObj);
        Jim_StackPush(interp->framePtr()->localCommands(), cmdNameObj);
    }

    return retcode;
}

/* [upcall] */
static Retval Jim_UpcallCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "cmd ?args ...?"); // #MissInCoverage
        return JIM_ERR;
    }
    else {
        int retcode;

        Jim_Cmd *cmdPtr = Jim_GetCommand(interp, argv[1], JIM_ERRMSG);
        if (cmdPtr == NULL || !cmdPtr->isproc() || !cmdPtr->prevCmd()) {
            Jim_SetResultFormatted(interp, "no previous command: \"%#s\"", argv[1]);
            return JIM_ERR;
        }
        /* OK. Mark this command as being in an upcall */
        cmdPtr->u.proc_.upcall++;
        JimIncrCmdRefCount(cmdPtr);

        /* Invoke the command as normal */
        retcode = Jim_EvalObjVector(interp, argc - 1, argv + 1);

        /* No longer in an upcall */
        cmdPtr->u.proc_.upcall--;
        JimDecrCmdRefCount(interp, cmdPtr);

        return retcode;
    }
}

/* [apply] */
static Retval Jim_ApplyCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "lambdaExpr ?arg ...?");
        return JIM_ERR;
    }
    else {
        Retval ret;
        Jim_Cmd *cmd;
        Jim_ObjPtr argListObjPtr;
        Jim_ObjPtr bodyObjPtr;
        Jim_ObjPtr nsObj = NULL;
        Jim_ObjArray* nargv;

        int len = Jim_ListLength(interp, argv[1]);
        if (len != 2 && len != 3) {
            Jim_SetResultFormatted(interp, "can't interpret \"%#s\" as a lambda expression", argv[1]);
            return JIM_ERR;
        }

        if (len == 3) {
#ifdef jim_ext_namespace // #optionalCode
            /* Need to canonicalise the given namespace. */
            nsObj = JimQualifyNameObj(interp, Jim_ListGetIndex(interp, argv[1], 2));
#else // #WinOff
            Jim_SetResultString(interp, "namespaces not enabled", -1);
            return JIM_ERR;
#endif
        }
        argListObjPtr = Jim_ListGetIndex(interp, argv[1], 0);
        bodyObjPtr = Jim_ListGetIndex(interp, argv[1], 1);

        cmd = JimCreateProcedureCmd(interp, argListObjPtr, NULL, bodyObjPtr, nsObj);

        if (cmd) {
            /* Create a new argv array with a dummy argv[0], for error messages */
            nargv = new_Jim_ObjArray((argc - 2 + 1)); // #AllocF 
            nargv[0] = Jim_NewStringObj(interp, "apply lambdaExpr", -1);
            Jim_IncrRefCount(nargv[0]);
            memcpy(&nargv[1], argv + 2, (argc - 2) * sizeof(*nargv));
            ret = JimCallProcedure(interp, cmd, argc - 2 + 1, nargv);
            Jim_DecrRefCount(interp, nargv[0]);
            free_Jim_ObjArray(nargv); // #FreeF

            JimDecrCmdRefCount(interp, cmd);
            return ret;
        }
        return JIM_ERR;
    }
}


/* [concat] */
static Retval Jim_ConcatCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_SetResult(interp, Jim_ConcatObj(interp, argc - 1, argv + 1));
    return JIM_OK;
}

/* [upvar] */
static Retval Jim_UpvarCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int i;
    Jim_CallFramePtr targetCallFrame;

    /* Lookup the target frame pointer */
    if (argc > 3 && (argc % 2 == 0)) {
        targetCallFrame = Jim_GetCallFrameByLevel(interp, argv[1]);
        argc--;
        argv++;
    }
    else {
        targetCallFrame = Jim_GetCallFrameByLevel(interp, NULL);
    }
    if (targetCallFrame == NULL) {
        return JIM_ERR;
    }

    /* Check for arity */
    if (argc < 3) {
        Jim_WrongNumArgs(interp, 1, argv, "?level? otherVar localVar ?otherVar localVar ...?"); // #MissInCoverage
        return JIM_ERR;
    }

    /* Now... for every other/local couple: */
    for (i = 1; i < argc; i += 2) {
        if (Jim_SetVariableLink(interp, argv[i + 1], argv[i], targetCallFrame) != JIM_OK)
            return JIM_ERR;
    }
    return JIM_OK;
}

/* [global] */
static Retval Jim_GlobalCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int i;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "varName ?varName ...?");
        return JIM_ERR;
    }
    /* Link every var to the toplevel having the same name */
    if (interp->framePtr()->level() == 0)
        return JIM_OK;          /* global at toplevel... */
    for (i = 1; i < argc; i++) {
        /* global ::blah does nothing */
        const char *name = Jim_String(argv[i]);
        if (name[0] != ':' || name[1] != ':') {
            if (Jim_SetVariableLink(interp, argv[i], argv[i], interp->topFramePtr()) != JIM_OK)
                return JIM_ERR;
        }
    }
    return JIM_OK;
}

/* does the [string map] operation. On error NULL is returned,
 * otherwise a new string object with the result, having refcount = 0,
 * is returned. */
static Jim_ObjPtr JimStringMap(Jim_InterpPtr interp, Jim_ObjPtr mapListObjPtr,
    Jim_ObjPtr objPtr, int nocase)
{
    PRJ_TRACE;
    int numMaps;
    const char *str, *noMatchStart = NULL;
    int strLen, i;
    Jim_ObjPtr resultObjPtr;

    numMaps = Jim_ListLength(interp, mapListObjPtr);
    if (numMaps % 2) {
        Jim_SetResultString(interp, "list must contain an even number of elements", -1);
        return NULL;
    }

    str = Jim_String(objPtr);
    strLen = Jim_Utf8Length(interp, objPtr);

    /* Map it */
    resultObjPtr = Jim_NewStringObj(interp, "", 0);
    while (strLen) {
        for (i = 0; i < numMaps; i += 2) {
            Jim_ObjPtr eachObjPtr;
            const char *k;
            int kl;

            eachObjPtr = Jim_ListGetIndex(interp, mapListObjPtr, i);
            k = Jim_String(eachObjPtr);
            kl = Jim_Utf8Length(interp, eachObjPtr);

            if (strLen >= kl && kl) {
                int rc;
                rc = JimStringCompareLen(str, k, kl, nocase);
                if (rc == 0) {
                    if (noMatchStart) {
                        Jim_AppendString(interp, resultObjPtr, 
                           noMatchStart, (int)(str - noMatchStart));
                        noMatchStart = NULL;
                    }
                    Jim_AppendObj(interp, resultObjPtr, Jim_ListGetIndex(interp, mapListObjPtr, i + 1));
                    str += utf8_index(str, kl);
                    strLen -= kl;
                    break;
                }
            }
        }
        if (i == numMaps) {     /* no match */
            int c;
            if (noMatchStart == NULL)
                noMatchStart = str;
            str += utf8_tounicode(str, &c);
            strLen--;
        }
    }
    if (noMatchStart) {
        Jim_AppendString(interp, resultObjPtr, noMatchStart, 
                         (int)(str - noMatchStart));
    }
    return resultObjPtr;
}

/* [string] */
static Retval Jim_StringCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int len;
    int opt_case = 1;
    int option;
    static const char * const options[] = {
        "bytelength", "length", "compare", "match", "equal", "is", "byterange", "range", "replace",
        "map", "repeat", "reverse", "index", "first", "last", "cat",
        "trim", "trimleft", "trimright", "tolower", "toupper", "totitle", NULL
    };
    enum
    {
        OPT_BYTELENGTH, OPT_LENGTH, OPT_COMPARE, OPT_MATCH, OPT_EQUAL, OPT_IS, OPT_BYTERANGE, OPT_RANGE, OPT_REPLACE,
        OPT_MAP, OPT_REPEAT, OPT_REVERSE, OPT_INDEX, OPT_FIRST, OPT_LAST, OPT_CAT,
        OPT_TRIM, OPT_TRIMLEFT, OPT_TRIMRIGHT, OPT_TOLOWER, OPT_TOUPPER, OPT_TOTITLE
    };
    static const char * const nocase_options[] = {
        "-nocase", NULL
    };
    static const char * const nocase_length_options[] = {
        "-nocase", "-length", NULL
    };

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "option ?arguments ...?");
        return JIM_ERR;
    }
    if (Jim_GetEnum(interp, argv[1], options, &option, NULL,
            JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK)
        return Jim_CheckShowCommands(interp, argv[1], options);

    switch (option) {
        case OPT_LENGTH:
        case OPT_BYTELENGTH:
            if (argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "string");
                return JIM_ERR;
            }
            if (option == OPT_LENGTH) {
                len = Jim_Utf8Length(interp, argv[2]);
            }
            else {
                len = Jim_Length(argv[2]);
            }
            Jim_SetResultInt(interp, len);
            return JIM_OK;

        case OPT_CAT:{
                Jim_ObjPtr objPtr;
                if (argc == 3) {
                    /* optimise the one-arg case */
                    objPtr = argv[2];
                }
                else {
                    int i;

                    objPtr = Jim_NewStringObj(interp, "", 0);

                    for (i = 2; i < argc; i++) {
                        Jim_AppendObj(interp, objPtr, argv[i]);
                    }
                }
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }

        case OPT_COMPARE:
        case OPT_EQUAL:
            {
                /* n is the number of remaining option args */
                long opt_length = -1;
                int n = argc - 4;
                int i = 2;
                while (n > 0) {
                    int subopt;
                    if (Jim_GetEnum(interp, argv[i++], nocase_length_options, &subopt, NULL,
                            JIM_ENUM_ABBREV) != JIM_OK) {
badcompareargs:
                        Jim_WrongNumArgs(interp, 2, argv, "?-nocase? ?-length int? string1 string2");
                        return JIM_ERR;
                    }
                    if (subopt == 0) {
                        /* -nocase */
                        opt_case = 0;
                        n--;
                    }
                    else {
                        /* -length */
                        if (n < 2) {
                            goto badcompareargs;
                        }
                        if (Jim_GetLong(interp, argv[i++], &opt_length) != JIM_OK) {
                            return JIM_ERR;
                        }
                        n -= 2;
                    }
                }
                if (n) {
                    goto badcompareargs;
                }
                argv += argc - 2;
                if (opt_length < 0 && option != OPT_COMPARE && opt_case) {
                    /* Fast version - [string equal], case sensitive, no length */
                    Jim_SetResultBool(interp, Jim_StringEqObj(argv[0], argv[1]));
                }
                else {
                    if (opt_length >= 0) {
                        n = JimStringCompareLen(Jim_String(argv[0]), Jim_String(argv[1]), opt_length, !opt_case);
                    }
                    else {
                        n = Jim_StringCompareObj(interp, argv[0], argv[1], !opt_case);
                    }
                    Jim_SetResultInt(interp, option == OPT_COMPARE ? n : n == 0);
                }
                return JIM_OK;
            }

        case OPT_MATCH:
            if (argc != 4 &&
                (argc != 5 ||
                    Jim_GetEnum(interp, argv[2], nocase_options, &opt_case, NULL,
                        JIM_ENUM_ABBREV) != JIM_OK)) {
                Jim_WrongNumArgs(interp, 2, argv, "?-nocase? pattern string");
                return JIM_ERR;
            }
            if (opt_case == 0) {
                argv++;
            }
            Jim_SetResultBool(interp, Jim_StringMatchObj(interp, argv[2], argv[3], !opt_case));
            return JIM_OK;

        case OPT_MAP:{
                Jim_ObjPtr objPtr;

                if (argc != 4 &&
                    (argc != 5 ||
                        Jim_GetEnum(interp, argv[2], nocase_options, &opt_case, NULL,
                            JIM_ENUM_ABBREV) != JIM_OK)) {
                    Jim_WrongNumArgs(interp, 2, argv, "?-nocase? mapList string");
                    return JIM_ERR;
                }

                if (opt_case == 0) {
                    argv++;
                }
                objPtr = JimStringMap(interp, argv[2], argv[3], !opt_case);
                if (objPtr == NULL) {
                    return JIM_ERR;
                }
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }

        case OPT_RANGE:
        case OPT_BYTERANGE:{
                Jim_ObjPtr objPtr;

                if (argc != 5) {
                    Jim_WrongNumArgs(interp, 2, argv, "string first last");
                    return JIM_ERR;
                }
                if (option == OPT_RANGE) {
                    objPtr = Jim_StringRangeObj(interp, argv[2], argv[3], argv[4]);
                }
                else
                {
                    objPtr = Jim_StringByteRangeObj(interp, argv[2], argv[3], argv[4]); // #MissInCoverage
                }

                if (objPtr == NULL) {
                    return JIM_ERR;
                }
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }

        case OPT_REPLACE:{
                Jim_ObjPtr objPtr;

                if (argc != 5 && argc != 6) {
                    Jim_WrongNumArgs(interp, 2, argv, "string first last ?string?");
                    return JIM_ERR;
                }
                objPtr = JimStringReplaceObj(interp, argv[2], argv[3], argv[4], argc == 6 ? argv[5] : NULL);
                if (objPtr == NULL) {
                    return JIM_ERR;
                }
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }


        case OPT_REPEAT:{
                Jim_ObjPtr objPtr;
                jim_wide count;

                if (argc != 4) {
                    Jim_WrongNumArgs(interp, 2, argv, "string count");
                    return JIM_ERR;
                }
                if (Jim_GetWide(interp, argv[3], &count) != JIM_OK) {
                    return JIM_ERR;
                }
                objPtr = Jim_NewStringObj(interp, "", 0);
                if (count > 0) {
                    while (count--) {
                        Jim_AppendObj(interp, objPtr, argv[2]);
                    }
                }
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }

        case OPT_REVERSE:{
                char *buf, *p;
                const char *str;
                int i;

                if (argc != 3) {
                    Jim_WrongNumArgs(interp, 2, argv, "string"); // #MissInCoverage
                    return JIM_ERR;
                }

                str = Jim_GetString(argv[2], &len);
                buf = new_CharArray(len + 1); // #AllocF 
                p = buf + len;
                *p = 0;
                for (i = 0; i < len; ) {
                    int c;
                    int l = utf8_tounicode(str, &c);
                    memcpy(p - l, str, l);
                    p -= l;
                    i += l;
                    str += l;
                }
                Jim_SetResult(interp, Jim_NewStringObjNoAlloc(interp, buf, len));
                return JIM_OK;
            }

        case OPT_INDEX:{
                int idx;
                const char *str;

                if (argc != 4) {
                    Jim_WrongNumArgs(interp, 2, argv, "string index");
                    return JIM_ERR;
                }
                if (Jim_GetIndex(interp, argv[3], &idx) != JIM_OK) {
                    return JIM_ERR;
                }
                str = Jim_String(argv[2]);
                len = Jim_Utf8Length(interp, argv[2]);
                if (idx != INT_MIN && idx != INT_MAX) {
                    idx = JimRelToAbsIndex(len, idx);
                }
                if (idx < 0 || idx >= len || str == NULL) {
                    Jim_SetResultString(interp, "", 0);
                }
                else if (len == Jim_Length(argv[2])) {
                    /* ASCII optimisation */
                    Jim_SetResultString(interp, str + idx, 1);
                }
                else {
                    int c;
                    int i = utf8_index(str, idx);
                    Jim_SetResultString(interp, str + i, utf8_tounicode(str + i, &c));
                }
                return JIM_OK;
            }

        case OPT_FIRST:
        case OPT_LAST:{
                int idx = 0, l1, l2;
                const char *s1, *s2;

                if (argc != 4 && argc != 5) {
                    Jim_WrongNumArgs(interp, 2, argv, "subString string ?index?");
                    return JIM_ERR;
                }
                s1 = Jim_String(argv[2]);
                s2 = Jim_String(argv[3]);
                l1 = Jim_Utf8Length(interp, argv[2]);
                l2 = Jim_Utf8Length(interp, argv[3]);
                if (argc == 5) {
                    if (Jim_GetIndex(interp, argv[4], &idx) != JIM_OK) {
                        return JIM_ERR;
                    }
                    idx = JimRelToAbsIndex(l2, idx);
                }
                else if (option == OPT_LAST) {
                    idx = l2;
                }
                if (option == OPT_FIRST) {
                    Jim_SetResultInt(interp, JimStringFirst(s1, l1, s2, l2, idx));
                }
                else {
                    if (g_JIM_UTF8_VAL) {
                        Jim_SetResultInt(interp, JimStringLastUtf8(s1, l1, s2, idx));
                    } else {
                        Jim_SetResultInt(interp, JimStringLast(s1, l1, s2, idx)); // #MissInCoverage
                    }

                }
                return JIM_OK;
            }

        case OPT_TRIM:
        case OPT_TRIMLEFT:
        case OPT_TRIMRIGHT:{
                Jim_ObjPtr trimchars;

                if (argc != 3 && argc != 4) {
                    Jim_WrongNumArgs(interp, 2, argv, "string ?trimchars?");
                    return JIM_ERR;
                }
                trimchars = (argc == 4 ? argv[3] : NULL);
                if (option == OPT_TRIM) {
                    Jim_SetResult(interp, JimStringTrim(interp, argv[2], trimchars));
                }
                else if (option == OPT_TRIMLEFT) {
                    Jim_SetResult(interp, JimStringTrimLeft(interp, argv[2], trimchars));
                }
                else if (option == OPT_TRIMRIGHT) {
                    Jim_SetResult(interp, JimStringTrimRight(interp, argv[2], trimchars));
                }
                return JIM_OK;
            }

        case OPT_TOLOWER:
        case OPT_TOUPPER:
        case OPT_TOTITLE:
            if (argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "string");
                return JIM_ERR;
            }
            if (option == OPT_TOLOWER) {
                Jim_SetResult(interp, JimStringToLower(interp, argv[2]));
            }
            else if (option == OPT_TOUPPER) {
                Jim_SetResult(interp, JimStringToUpper(interp, argv[2]));
            }
            else {
                Jim_SetResult(interp, JimStringToTitle(interp, argv[2]));
            }
            return JIM_OK;

        case OPT_IS:
            if (argc == 4 || (argc == 5 && Jim_CompareStringImmediate(interp, argv[3], "-strict"))) {
                return JimStringIs(interp, argv[argc - 1], argv[2], argc == 5);
            }
            Jim_WrongNumArgs(interp, 2, argv, "class ?-strict? str");
            return JIM_ERR;
    }
    return JIM_OK; // #MissInCoverage
}

/* [time] */
static Retval Jim_TimeCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    long i, count = 1;
    jim_wide start, elapsed;
    char buf[60]; // #MagicNum
    const char *fmt = "%" JIM_WIDE_MODIFIER " microseconds per iteration";

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "script ?count?"); // #MissInCoverage
        return JIM_ERR;
    }
    if (argc == 3) {
        if (Jim_GetLong(interp, argv[2], &count) != JIM_OK) // #MissInCoverage
            return JIM_ERR;
    }
    if (count < 0)
        return JIM_OK; // #MissInCoverage
    i = count;
    start = JimClock();
    while (i-- > 0) {
        int retval;

        retval = Jim_EvalObj(interp, argv[1]);
        if (retval != JIM_OK) {
            return retval; // #MissInCoverage
        }
    }
    elapsed = JimClock() - start;
    sprintf(buf, fmt, count == 0 ? 0 : elapsed / count);
    Jim_SetResultString(interp, buf, -1);
    return JIM_OK;
}

/* [exit] */
static Retval Jim_ExitCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    long exitCode = 0;

    if (argc > 2) {
        Jim_WrongNumArgs(interp, 1, argv, "?exitCode?"); // #MissInCoverage
        return JIM_ERR;
    }
    if (argc == 2) {
        if (Jim_GetLong(interp, argv[1], &exitCode) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
    }
    interp->setExitCode(exitCode);
    return JIM_EXIT;
}

/* [catch] */
static Retval Jim_CatchCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int exitCode = 0;
    int i;
    int sig = 0;

    /* Which return codes are ignored (passed through)? By default, only exit, eval and signal */
    jim_wide ignore_mask = (1 << JIM_EXIT) | (1 << JIM_EVAL) | (1 << JIM_SIGNAL);
    static const int max_ignore_code = sizeof(ignore_mask) * 8;

    /* Reset the error code before catch.
     * Note that this is not strictly correct.
     */
    Jim_SetGlobalVariableStr(interp, "errorCode", Jim_NewStringObj(interp, "NONE", -1));

    for (i = 1; i < argc - 1; i++) {
        const char *arg = Jim_String(argv[i]);
        jim_wide option;
        int ignore;

        /* It's a pity we can't use Jim_GetEnum here :-( */
        if (strcmp(arg, "--") == 0) {
            i++; // #MissInCoverage
            break;
        }
        if (*arg != '-') {
            break;
        }

        if (strncmp(arg, "-no", 3) == 0) {
            arg += 3;
            ignore = 1;
        }
        else {
            arg++; 
            ignore = 0;
        }

        if (Jim_StringToWide(arg, &option, 10) != JIM_OK) {
            option = -1;
        }
        if (option < 0) {
            option = Jim_FindByName(arg, g_jimReturnCodes, jimReturnCodesSize);
        }
        if (option < 0) {
            goto wrongargs; // #MissInCoverage
        }

        if (ignore) {
            ignore_mask |= ((jim_wide)1 << option);
        }
        else {
            ignore_mask &= (~((jim_wide)1 << option));
        }
    }

    argc -= i;
    if (argc < 1 || argc > 3) {
      wrongargs:
        Jim_WrongNumArgs(interp, 1, argv,
            "?-?no?code ... --? script ?resultVarName? ?optionVarName?"); // #MissInCoverage
        return JIM_ERR;
    }
    argv += i;

    if ((ignore_mask & (1 << JIM_SIGNAL)) == 0) {
        sig++; // #MissInCoverage
    }

    interp->incrSignalLevel(sig);
    if (Jim_CheckSignal(interp)) {
        /* If a signal is set, don't even try to execute the body */
        exitCode = JIM_SIGNAL; // #MissInCoverage
    }
    else {
        exitCode = Jim_EvalObj(interp, argv[0]);
        /* Don't want any caught error included in a later stack trace */
        interp->setErrorFlag(0);
    }
    interp->decrSignalLevel(sig);

    /* Catch or pass through? Only the first 32/64 codes can be passed through */
    if (exitCode >= 0 && exitCode < max_ignore_code && (((unsigned_jim_wide)1 << exitCode) & ignore_mask)) {
        /* Not caught, pass it up */
        return exitCode;
    }

    if (sig && exitCode == JIM_SIGNAL) {
        /* Catch the signal at this level */
        if (interp->get_signal_set_result()) { // #MissInCoverage
            interp->signal_set_result_(interp, interp->getSigmask());
        }
        else {
            Jim_SetResultInt(interp, interp->getSigmask()); // #MissInCoverage
        }
        interp->setSigmask(0); // #MissInCoverage
    }

    if (argc >= 2) {
        if (Jim_SetVariable(interp, argv[1], Jim_GetResult(interp)) != JIM_OK) {
            return JIM_ERR; // #MissInCoverage
        }
        if (argc == 3) {
            Jim_ObjPtr optListObj = Jim_NewListObj(interp, NULL, 0);

            Jim_ListAppendElement(interp, optListObj, Jim_NewStringObj(interp, "-code", -1));
            Jim_ListAppendElement(interp, optListObj,
                Jim_NewIntObj(interp, exitCode == JIM_RETURN ? interp->returnCode() : exitCode));
            Jim_ListAppendElement(interp, optListObj, Jim_NewStringObj(interp, "-level", -1));
            Jim_ListAppendElement(interp, optListObj, Jim_NewIntObj(interp, interp->returnLevel()));
            if (exitCode == JIM_ERR) {
                Jim_ObjPtr errorCode;
                Jim_ListAppendElement(interp, optListObj, Jim_NewStringObj(interp, "-errorinfo",
                    -1));
                Jim_ListAppendElement(interp, optListObj, interp->stackTrace());

                errorCode = Jim_GetGlobalVariableStr(interp, "errorCode", JIM_NONE);
                if (errorCode) {
                    Jim_ListAppendElement(interp, optListObj, Jim_NewStringObj(interp, "-errorcode", -1));
                    Jim_ListAppendElement(interp, optListObj, errorCode);
                }
            }
            if (Jim_SetVariable(interp, argv[2], optListObj) != JIM_OK) {
                return JIM_ERR; // #MissInCoverage
            }
        }
    }
    Jim_SetResultInt(interp, exitCode);
    return JIM_OK;
}

#if defined(JIM_REFERENCES) && !defined(JIM_BOOTSTRAP) // #optionalCode

/* [ref] */
static Retval Jim_RefCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 3 && argc != 4) {
        Jim_WrongNumArgs(interp, 1, argv, "string tag ?finalizer?"); // #MissInCoverage
        return JIM_ERR;
    }
    if (argc == 3) {
        Jim_SetResult(interp, Jim_NewReference(interp, argv[1], argv[2], NULL));
    }
    else {
        Jim_SetResult(interp, Jim_NewReference(interp, argv[1], argv[2], argv[3]));
    }
    return JIM_OK;
}

/* [getref] */
static Retval Jim_GetrefCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_Reference *refPtr;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "reference"); // #MissInCoverage
        return JIM_ERR;
    }
    if ((refPtr = Jim_GetReference(interp, argv[1])) == NULL)
        return JIM_ERR;
    Jim_SetResult(interp, refPtr->objPtr);
    return JIM_OK;
}

/* [setref] */
static Retval Jim_SetrefCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_Reference *refPtr;

    if (argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "reference newValue"); // #MissInCoverage
        return JIM_ERR;
    }
    if ((refPtr = Jim_GetReference(interp, argv[1])) == NULL)
        return JIM_ERR;
    Jim_IncrRefCount(argv[2]);
    Jim_DecrRefCount(interp, refPtr->objPtr);
    refPtr->objPtr = argv[2];
    Jim_SetResult(interp, argv[2]);
    return JIM_OK;
}

/* [collect] */
static Retval Jim_CollectCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 1) {
        Jim_WrongNumArgs(interp, 1, argv, "");// #MissInCoverage
        return JIM_ERR;
    }
    Jim_SetResultInt(interp, Jim_Collect(interp));

    /* Free all the freed objects. */
    while (interp->freeList()) {
        Jim_ObjPtr nextObjPtr = interp->freeList()->nextObjPtr(); 
        interp->freeFreeList(); // #FreeF
        //free_Jim_Obj(interp->freeList_); // #FreeF 
        interp->setFreeList(nextObjPtr);
    }

    return JIM_OK;
}

/* [finalize] reference ?newValue? */
static Retval Jim_FinalizeCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #MissInCoverage #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "reference ?finalizerProc?");
        return JIM_ERR;
    }
    if (argc == 2) {
        Jim_ObjPtr cmdNamePtr;

        if (Jim_GetFinalizer(interp, argv[1], &cmdNamePtr) != JIM_OK)
            return JIM_ERR;
        if (cmdNamePtr != NULL) /* otherwise the null string is returned. */
            Jim_SetResult(interp, cmdNamePtr);
    }
    else {
        if (Jim_SetFinalizer(interp, argv[1], argv[2]) != JIM_OK)
            return JIM_ERR;
        Jim_SetResult(interp, argv[2]);
    }
    return JIM_OK;
}

/* [info references] */
static Retval JimInfoReferences(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #MissInCoverage
{
    PRJ_TRACE;
    Jim_ObjPtr listObjPtr;
    Jim_HashTableIterator htiter;
    Jim_HashEntryPtr he;

    listObjPtr = Jim_NewListObj(interp, NULL, 0);

    JimInitHashTableIterator(&interp->references(), &htiter);
    while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
        char buf[JIM_REFERENCE_SPACE + 1];
        Jim_Reference *refPtr = (Jim_Reference *)Jim_GetHashEntryVal(he);
        const_unsigned_long *refId = (const_unsigned_long*)he->keyAsVoid();

        JimFormatReference(buf, refPtr, *refId);
        Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, buf, -1));
    }
    Jim_SetResult(interp, listObjPtr);
    return JIM_OK;
}
#endif /* JIM_REFERENCES && !JIM_BOOTSTRAP */

/* [rename] */
static Retval Jim_RenameCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "oldName newName");
        return JIM_ERR;
    }

    if (JimValidName(interp, "new procedure", argv[2])) {
        return JIM_ERR; // #MissInCoverage
    }

    return Jim_RenameCommand(interp, Jim_String(argv[1]), Jim_String(argv[2]));
}

/**
 * match_type must be one of JIM_DICTMATCH_KEYS or JIM_DICTMATCH_VALUES
 * return_types should be either or both
 */
JIM_EXPORT Retval Jim_DictMatchTypes(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr patternObj, int match_type, int return_types)
{
    PRJ_TRACE;
    Jim_HashEntryPtr he;
    Jim_ObjPtr listObjPtr;
    Jim_HashTableIterator htiter;

    if (SetDictFromAny(interp, objPtr) != JIM_OK) {
        return JIM_ERR;
    }

    listObjPtr = Jim_NewListObj(interp, NULL, 0);

    JimInitHashTableIterator((Jim_HashTablePtr )objPtr->getVoidPtr(), &htiter);
    while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
        if (patternObj) {
            Jim_ObjPtr matchObj = (match_type == JIM_DICTMATCH_KEYS) ? (Jim_ObjPtr )he->keyAsVoid() : (Jim_ObjPtr )Jim_GetHashEntryVal(he);
            if (!JimGlobMatch(Jim_String(patternObj), Jim_String(matchObj), 0)) {
                /* no match */
                continue;
            }
        }
        if (return_types & JIM_DICTMATCH_KEYS) {
            Jim_ListAppendElement(interp, listObjPtr, he->keyAsObj());
        }
        if (return_types & JIM_DICTMATCH_VALUES) {
            Jim_ListAppendElement(interp, listObjPtr, (Jim_ObjPtr )Jim_GetHashEntryVal(he));
        }
    }

    Jim_SetResult(interp, listObjPtr);
    return JIM_OK;
}

JIM_EXPORT int Jim_DictSize(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    if (SetDictFromAny(interp, objPtr) != JIM_OK) {
        return -1;
    }
    return ((Jim_HashTablePtr )objPtr->getVoidPtr())->used();
}

/**
 * Must be called with at least one object.
 * Returns the new dictionary, or NULL on error.
 */
JIM_EXPORT Jim_ObjPtr Jim_DictMerge(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv)
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr = Jim_NewDictObj(interp, NULL, 0);
    int i;

    JimPanic((objc == 0, "Jim_DictMerge called with objc=0"));

    /* Note that we don't optimise the trivial case of a single argument */

    for (i = 0; i < objc; i++) {
        Jim_HashTablePtr ht;
        Jim_HashTableIterator htiter;
        Jim_HashEntryPtr he;

        if (SetDictFromAny(interp, objv[i]) != JIM_OK) {
            Jim_FreeNewObj(interp, objPtr);
            return NULL;
        }
        ht = (Jim_HashTablePtr )objv[i]->getVoidPtr();
        JimInitHashTableIterator(ht, &htiter);
        while ((he = Jim_NextHashEntry(&htiter)) != NULL) {
            Jim_ReplaceHashEntry((Jim_HashTablePtr )objPtr->getVoidPtr(), Jim_GetHashEntryKey(he), Jim_GetHashEntryVal(he));
        }
    }
    return objPtr;
}

JIM_EXPORT Retval Jim_DictInfo(Jim_InterpPtr interp, Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    Jim_HashTablePtr ht;
    unsigned_int i;
    char buffer[100]; // #MagicNum
    int sum = 0;
    int nonzero_count = 0;
    Jim_ObjPtr output;
    int bucket_counts[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // #MagicNum

    if (SetDictFromAny(interp, objPtr) != JIM_OK) {
        return JIM_ERR; // #MissInCoverage
    }

    ht = (Jim_HashTablePtr )objPtr->getVoidPtr();

    /* Note that this uses internal knowledge of the hash table */
    snprintf(buffer, sizeof(buffer), "%d entries in table, %d buckets\n", ht->used(), ht->size());
    output = Jim_NewStringObj(interp, buffer, -1);

    for (i = 0; i < ht->size(); i++) {
        Jim_HashEntryPtr he = ht->getEntry(i);
        int entries = 0;
        while (he) {
            entries++;
            he = he->next();
        }
        if (entries > 9) {
            bucket_counts[10]++; // #MissInCoverage
        }
        else {
            bucket_counts[entries]++;
        }
        if (entries) {
            sum += entries;
            nonzero_count++;
        }
    }
    for (i = 0; i < 10; i++) {
        snprintf(buffer, sizeof(buffer), "number of buckets with %d entries: %d\n", i, bucket_counts[i]);
        Jim_AppendString(interp, output, buffer, -1);
    }
    snprintf(buffer, sizeof(buffer), "number of buckets with 10 or more entries: %d\n", bucket_counts[10]);
    Jim_AppendString(interp, output, buffer, -1);
    snprintf(buffer, sizeof(buffer), "average search distance for entry: %.1f", nonzero_count ? (double)sum / nonzero_count : 0.0);
    Jim_AppendString(interp, output, buffer, -1);
    Jim_SetResult(interp, output);
    return JIM_OK;
}

static Retval Jim_EvalEnsemble(Jim_InterpPtr interp, const char *basecmd, const char *subcmd, int argc, Jim_ObjConstArray argv)
{
    PRJ_TRACE;
    Jim_ObjPtr prefixObj = Jim_NewStringObj(interp, basecmd, -1);

    Jim_AppendString(interp, prefixObj, " ", 1);
    Jim_AppendString(interp, prefixObj, subcmd, -1);

    return Jim_EvalObjPrefix(interp, prefixObj, argc, argv);
}

/**
 * Implements the [dict with] command
 */
static Retval JimDictWith(Jim_InterpPtr interp, Jim_ObjPtr dictVarName, Jim_ObjConstArray keyv, int keyc, Jim_ObjPtr scriptObj)
{
    PRJ_TRACE;
    int i;
    Jim_ObjPtr objPtr;
    Jim_ObjPtr dictObj;
    Jim_ObjArray* dictValues;
    int len;
    Retval ret = JIM_OK;

    /* Open up the appropriate level of the dictionary */
    dictObj = Jim_GetVariable(interp, dictVarName, JIM_ERRMSG);
    if (dictObj == NULL || Jim_DictKeysVector(interp, dictObj, keyv, keyc, &objPtr, JIM_ERRMSG) != JIM_OK) {
        return JIM_ERR;
    }
    /* Set the local variables */
    if (Jim_DictPairs(interp, objPtr, &dictValues, &len) == JIM_ERR) {
        return JIM_ERR; // #MissInCoverage
    }
    for (i = 0; i < len; i += 2) {
        if (Jim_SetVariable(interp, dictValues[i], dictValues[i + 1]) == JIM_ERR) {
            free_Jim_ObjArray(dictValues); // #FreeF #MissInCoverage
            return JIM_ERR;
        }
    }

    /* As an optimisation, if the script is empty, no need to evaluate it or update the dict */
    if (Jim_Length(scriptObj)) {
        ret = Jim_EvalObj(interp, scriptObj);

        /* Now if the dictionary still exists, update it based on the local variables */
        if (ret == JIM_OK && Jim_GetVariable(interp, dictVarName, 0) != NULL) {
            /* We need a copy of keyv with one extra element at the end for Jim_SetDictKeysVector() */
            Jim_ObjArray* newkeyv = new_Jim_ObjArray((keyc + 1)); // #AllocF 
            for (i = 0; i < keyc; i++) {
                newkeyv[i] = keyv[i];
            }

            for (i = 0; i < len; i += 2) {
                /* This will be NULL if the variable no longer exists, thus deleting the variable */
                objPtr = Jim_GetVariable(interp, dictValues[i], 0);
                newkeyv[keyc] = dictValues[i];
                Jim_SetDictKeysVector(interp, dictVarName, newkeyv, keyc + 1, objPtr, 0);
            }
            free_Jim_ObjArray(newkeyv); // #FreeF
        }
    }

    free_Jim_ObjArray(dictValues); // #FreeF

    return ret;
}

/* [dict] */
static Retval Jim_DictCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;
    int types = JIM_DICTMATCH_KEYS;
    int option;
    static const char * const options[] = {
        "create", "get", "set", "unset", "exists", "keys", "size", "info",
        "merge", "with", "append", "lappend", "incr", "remove", "values", "for",
        "replace", "update", NULL
    };
    enum
    {
        OPT_CREATE, OPT_GET, OPT_SET, OPT_UNSET, OPT_EXISTS, OPT_KEYS, OPT_SIZE, OPT_INFO,
        OPT_MERGE, OPT_WITH, OPT_APPEND, OPT_LAPPEND, OPT_INCR, OPT_REMOVE, OPT_VALUES, OPT_FOR,
        OPT_REPLACE, OPT_UPDATE,
    };

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "subcommand ?arguments ...?");
        return JIM_ERR;
    }

    if (Jim_GetEnum(interp, argv[1], options, &option, "subcommand", JIM_ERRMSG) != JIM_OK) {
        return Jim_CheckShowCommands(interp, argv[1], options);
    }

    switch (option) {
        case OPT_GET:
            if (argc < 3) {
                Jim_WrongNumArgs(interp, 2, argv, "dictionary ?key ...?");
                return JIM_ERR;
            }
            if (Jim_DictKeysVector(interp, argv[2], argv + 3, argc - 3, &objPtr,
                    JIM_ERRMSG) != JIM_OK) {
                return JIM_ERR;
            }
            Jim_SetResult(interp, objPtr);
            return JIM_OK;

        case OPT_SET:
            if (argc < 5) {
                Jim_WrongNumArgs(interp, 2, argv, "varName key ?key ...? value");
                return JIM_ERR;
            }
            return Jim_SetDictKeysVector(interp, argv[2], argv + 3, argc - 4, argv[argc - 1], JIM_ERRMSG);

        case OPT_EXISTS:
            if (argc < 4) {
                Jim_WrongNumArgs(interp, 2, argv, "dictionary key ?key ...?");
                return JIM_ERR;
            }
            else {
                int rc = Jim_DictKeysVector(interp, argv[2], argv + 3, argc - 3, &objPtr, JIM_ERRMSG);
                if (rc < 0) {
                    return JIM_ERR;
                }
                Jim_SetResultBool(interp,  rc == JIM_OK);
                return JIM_OK;
            }

        case OPT_UNSET:
            if (argc < 4) {
                Jim_WrongNumArgs(interp, 2, argv, "varName key ?key ...?");
                return JIM_ERR;
            }
            if (Jim_SetDictKeysVector(interp, argv[2], argv + 3, argc - 3, NULL, 0) != JIM_OK) {
                return JIM_ERR;
            }
            return JIM_OK;

        case OPT_VALUES:
            types = JIM_DICTMATCH_VALUES;
            /* fallthru */
        case OPT_KEYS:
            if (argc != 3 && argc != 4) {
                Jim_WrongNumArgs(interp, 2, argv, "dictionary ?pattern?");
                return JIM_ERR;
            }
            return Jim_DictMatchTypes(interp, argv[2], argc == 4 ? argv[3] : NULL, types, types);

        case OPT_SIZE:
            if (argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "dictionary");
                return JIM_ERR;
            }
            else if (Jim_DictSize(interp, argv[2]) < 0) {
                return JIM_ERR;
            }
            Jim_SetResultInt(interp, Jim_DictSize(interp, argv[2]));
            return JIM_OK;

        case OPT_MERGE:
            if (argc == 2) {
                return JIM_OK;
            }
            objPtr = Jim_DictMerge(interp, argc - 2, argv + 2);
            if (objPtr == NULL) {
                return JIM_ERR;
            }
            Jim_SetResult(interp, objPtr);
            return JIM_OK;

        case OPT_UPDATE:
            if (argc < 6 || argc % 2) {
                /* Better error message */
                argc = 2;
            }
            break;

        case OPT_CREATE:
            if (argc % 2) {
                Jim_WrongNumArgs(interp, 2, argv, "?key value ...?");
                return JIM_ERR;
            }
            objPtr = Jim_NewDictObj(interp, argv + 2, argc - 2);
            Jim_SetResult(interp, objPtr);
            return JIM_OK;

        case OPT_INFO:
            if (argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "dictionary"); // #MissInCoverage
                return JIM_ERR;
            }
            return Jim_DictInfo(interp, argv[2]); // #MissInCoverage

        case OPT_WITH:
            if (argc < 4) {
                Jim_WrongNumArgs(interp, 2, argv, "dictVar ?key ...? script");
                return JIM_ERR;
            }
            return JimDictWith(interp, argv[2], argv + 3, argc - 4, argv[argc - 1]);
    }
    /* Handle command as an ensemble */
    return Jim_EvalEnsemble(interp, "dict", options[option], argc - 2, argv + 2);
}

/* [subst] */
static Retval Jim_SubstCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    static const char * const options[] = {
        "-nobackslashes", "-nocommands", "-novariables", NULL
    };
    enum
    { OPT_NOBACKSLASHES, OPT_NOCOMMANDS, OPT_NOVARIABLES };
    int i;
    int flags = JIM_SUBST_FLAG;
    Jim_ObjPtr objPtr;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "?options? string");
        return JIM_ERR;
    }
    for (i = 1; i < (argc - 1); i++) {
        int option;

        if (Jim_GetEnum(interp, argv[i], options, &option, NULL,
                JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK) {
            return JIM_ERR;
        }
        switch (option) {
            case OPT_NOBACKSLASHES:
                flags |= JIM_SUBST_NOESC;
                break;
            case OPT_NOCOMMANDS:
                flags |= JIM_SUBST_NOCMD;
                break;
            case OPT_NOVARIABLES:
                flags |= JIM_SUBST_NOVAR;
                break;
        }
    }
    if (Jim_SubstObj(interp, argv[argc - 1], &objPtr, flags) != JIM_OK) {
        return JIM_ERR;
    }
    Jim_SetResult(interp, objPtr);
    return JIM_OK;
}

/* [info] */
STATIC Retval Jim_InfoCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    int cmd;
    Jim_ObjPtr objPtr;
    int mode = 0;

    static const char * const commands[] = {
        "body", "statics", "commands", "procs", "channels", "exists", "globals", "level", "frame", "locals",
        "vars", "version", "patchlevel", "complete", "args", "hostname",
        "script", "source", "stacktrace", "nameofexecutable", "returncodes",
        "references", "alias", NULL
    };
    enum
    { INFO_BODY, INFO_STATICS, INFO_COMMANDS, INFO_PROCS, INFO_CHANNELS, INFO_EXISTS, INFO_GLOBALS, INFO_LEVEL,
        INFO_FRAME, INFO_LOCALS, INFO_VARS, INFO_VERSION, INFO_PATCHLEVEL, INFO_COMPLETE, INFO_ARGS,
        INFO_HOSTNAME, INFO_SCRIPT, INFO_SOURCE, INFO_STACKTRACE, INFO_NAMEOFEXECUTABLE,
        INFO_RETURNCODES, INFO_REFERENCES, INFO_ALIAS,
    };

#ifdef jim_ext_namespace // #optionalCode
    int nons = 0;

    if (argc > 2 && Jim_CompareStringImmediate(interp, argv[1], "-nons")) {
        /* This is for internal use only */
        argc--;
        argv++;
        nons = 1;
    }
#endif

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "subcommand ?args ...?"); // #MissInCoverage
        return JIM_ERR;
    }
    if (Jim_GetEnum(interp, argv[1], commands, &cmd, "subcommand", JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK) {
        return Jim_CheckShowCommands(interp, argv[1], commands); // #MissInCoverage
    }

    /* Test for the most common commands first, just in case it makes a difference */
    switch (cmd) {
        case INFO_EXISTS:
            if (argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "varName"); 
                return JIM_ERR;
            }
            Jim_SetResultBool(interp, Jim_GetVariable(interp, argv[2], 0) != NULL);
            break;

        case INFO_ALIAS:{
            Jim_Cmd *cmdPtr;

            if (argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "command"); // #MissInCoverage
                return JIM_ERR;
            }
            if ((cmdPtr = Jim_GetCommand(interp, argv[2], JIM_ERRMSG)) == NULL) {
                return JIM_ERR; // #MissInCoverage
            }
            if (cmdPtr->isproc() || cmdPtr->cmdProc() != JimAliasCmd) {
                Jim_SetResultFormatted(interp, "command \"%#s\" is not an alias", argv[2]);
                return JIM_ERR;
            }
            Jim_SetResult(interp, (Jim_ObjPtr )cmdPtr->u.native_.privData);
            return JIM_OK;
        }

        case INFO_CHANNELS:
            mode++;             /* JIM_CMDLIST_CHANNELS */
#ifndef jim_ext_aio // #optionalCode #WinOff
            Jim_SetResultString(interp, "aio not enabled", -1);
            return JIM_ERR;
#endif
            /* fall through */
        case INFO_PROCS:
            mode++;             /* JIM_CMDLIST_PROCS */
            /* fall through */
        case INFO_COMMANDS:
            /* mode 0 => JIM_CMDLIST_COMMANDS */
            if (argc != 2 && argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "?pattern?");
                return JIM_ERR;
            }
#ifdef jim_ext_namespace // #optionalCode
            if (!nons) {
                if (Jim_Length(interp->framePtr()->nsObj()) || (argc == 3 && JimGlobMatch("::*", Jim_String(argv[2]), 0))) {
                    return Jim_EvalPrefix(interp, "namespace info", argc - 1, argv + 1);
                }
            }
#endif
            Jim_SetResult(interp, JimCommandsList(interp, (argc == 3) ? argv[2] : NULL, mode));
            break;

        case INFO_VARS:
            mode++;             /* JIM_VARLIST_VARS */
            /* fall through */
        case INFO_LOCALS:
            mode++;             /* JIM_VARLIST_LOCALS */
            /* fall through */
        case INFO_GLOBALS:
            /* mode 0 => JIM_VARLIST_GLOBALS */
            if (argc != 2 && argc != 3) {
                Jim_WrongNumArgs(interp, 2, argv, "?pattern?");
                return JIM_ERR;
            }
#ifdef jim_ext_namespace // #optionalCode
            if (!nons) {
                if (Jim_Length(interp->framePtr()->nsObj_) || (argc == 3 && JimGlobMatch("::*", Jim_String(argv[2]), 0))) {
                    return Jim_EvalPrefix(interp, "namespace info", argc - 1, argv + 1);
                }
            }
#endif
            Jim_SetResult(interp, JimVariablesList(interp, argc == 3 ? argv[2] : NULL, mode));
            break;

        case INFO_SCRIPT:
            if (argc != 2) {
                Jim_WrongNumArgs(interp, 2, argv, ""); // #MissInCoverage
                return JIM_ERR;
            }
            Jim_SetResult(interp, JimGetScript(interp, interp->currentScriptObj())->fileNameObj);
            break;

        case INFO_SOURCE:{
                jim_wide line;
                Jim_ObjPtr resObjPtr;
                Jim_ObjPtr fileNameObj;

                if (argc != 3 && argc != 5) {
                    Jim_WrongNumArgs(interp, 2, argv, "source ?filename line?"); // #MissInCoverage
                    return JIM_ERR;
                }
                if (argc == 5) {
                    if (Jim_GetWide(interp, argv[4], &line) != JIM_OK) { // #MissInCoverage
                        return JIM_ERR;
                    }
                    resObjPtr = Jim_NewStringObj(interp, Jim_String(argv[2]), Jim_Length(argv[2]));
                    JimSetSourceInfo(interp, resObjPtr, argv[3], (int)line);
                }
                else {
                    if (argv[2]->typePtr() == &g_sourceObjType) {
                        fileNameObj = argv[2]->get_sourceValue_fileName();
                        line = argv[2]->get_sourceValue_lineNum();
                    }
                    else if (argv[2]->typePtr() == &g_scriptObjType) {
                        ScriptObj *script = JimGetScript(interp, argv[2]);
                        fileNameObj = script->fileNameObj;
                        line = script->firstline;
                    }
                    else {
                        fileNameObj = interp->emptyObj(); // #MissInCoverage
                        line = 1;
                    }
                    resObjPtr = Jim_NewListObj(interp, NULL, 0);
                    Jim_ListAppendElement(interp, resObjPtr, fileNameObj);
                    Jim_ListAppendElement(interp, resObjPtr, Jim_NewIntObj(interp, line));
                }
                Jim_SetResult(interp, resObjPtr);
                break;
            }

        case INFO_STACKTRACE:
            Jim_SetResult(interp, interp->stackTrace());
            break;

        case INFO_LEVEL:
        case INFO_FRAME:
            switch (argc) {
                case 2:
                    Jim_SetResultInt(interp, interp->framePtr()->level());
                    break;

                case 3:
                    if (JimInfoLevel(interp, argv[2], &objPtr, cmd == INFO_LEVEL) != JIM_OK) {
                        return JIM_ERR;
                    }
                    Jim_SetResult(interp, objPtr);
                    break;

                default:
                    Jim_WrongNumArgs(interp, 2, argv, "?levelNum?");
                    return JIM_ERR;
            }
            break;

        case INFO_BODY:
        case INFO_STATICS:
        case INFO_ARGS:{
                Jim_Cmd *cmdPtr;

                if (argc != 3) {
                    Jim_WrongNumArgs(interp, 2, argv, "procname");
                    return JIM_ERR;
                }
                if ((cmdPtr = Jim_GetCommand(interp, argv[2], JIM_ERRMSG)) == NULL) {
                    return JIM_ERR; // #MissInCoverage
                }
                if (!cmdPtr->isproc()) {
                    Jim_SetResultFormatted(interp, "command \"%#s\" is not a procedure", argv[2]);
                    return JIM_ERR;
                }
                switch (cmd) {
                    case INFO_BODY:
                        Jim_SetResult(interp, cmdPtr->u.proc_.bodyObjPtr);
                        break;
                    case INFO_ARGS:
                        Jim_SetResult(interp, cmdPtr->u.proc_.argListObjPtr);
                        break;
                    case INFO_STATICS:
                        if (cmdPtr->u.proc_.staticVars) {
                            Jim_SetResult(interp, JimHashtablePatternMatch(interp, cmdPtr->u.proc_.staticVars,
                                NULL, JimVariablesMatch, JIM_VARLIST_LOCALS | JIM_VARLIST_VALUES));
                        }
                        break;
                }
                break;
            }

        case INFO_VERSION:
        case INFO_PATCHLEVEL:{
                char buf[(JIM_INTEGER_SPACE * 2) + 1];

                sprintf(buf, "%d.%d", version[0], version[1]);
                Jim_SetResultString(interp, buf, -1);
                break;
            }

        case INFO_COMPLETE:
            if (argc != 3 && argc != 4) {
                Jim_WrongNumArgs(interp, 2, argv, "script ?missing?"); // #MissInCoverage
                return JIM_ERR;
            }
            else {
                char missing;

                Jim_SetResultBool(interp, Jim_ScriptIsComplete(interp, argv[2], &missing));
                if (missing != ' ' && argc == 4) {
                    Jim_SetVariable(interp, argv[3], Jim_NewStringObj(interp, &missing, 1)); // #MissInCoverage
                }
            }
            break;

        case INFO_HOSTNAME:
            /* Redirect to os.gethostname if it exists */
            return Jim_Eval(interp, "os.gethostname"); // #MissInCoverage

        case INFO_NAMEOFEXECUTABLE:
            /* Redirect to Tcl proc */
            return Jim_Eval(interp, "{info nameofexecutable}");

        case INFO_RETURNCODES:
            if (argc == 2) {
                int i;
                Jim_ObjPtr listObjPtr = Jim_NewListObj(interp, NULL, 0);

                for (i = 0; g_jimReturnCodes[i]; i++) {
                    Jim_ListAppendElement(interp, listObjPtr, Jim_NewIntObj(interp, i));
                    Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp,
                            g_jimReturnCodes[i], -1));
                }

                Jim_SetResult(interp, listObjPtr);
            }
            else if (argc == 3) {
                long code;
                const char *name;

                if (Jim_GetLong(interp, argv[2], &code) != JIM_OK) {
                    return JIM_ERR; // #MissInCoverage
                }
                name = Jim_ReturnCode(code);
                if (*name == '?') {
                    Jim_SetResultInt(interp, code);
                }
                else {
                    Jim_SetResultString(interp, name, -1);
                }
            }
            else {
                Jim_WrongNumArgs(interp, 2, argv, "?code?"); // #MissInCoverage
                return JIM_ERR;
            }
            break;
        case INFO_REFERENCES:
#ifdef JIM_REFERENCES // #optionalCode
            return JimInfoReferences(interp, argc, argv); // #MissInCoverage
#else // #WinOff
            Jim_SetResultString(interp, "not supported", -1);
            return JIM_ERR;
#endif
    }
    return JIM_OK;
}

/* [exists] */
static Retval Jim_ExistsCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;
    int result = 0;

    static const char * const options[] = {
        "-command", "-proc", "-alias", "-var", NULL
    };
    enum
    {
        OPT_COMMAND, OPT_PROC, OPT_ALIAS, OPT_VAR
    };
    int option;

    if (argc == 2) {
        option = OPT_VAR;
        objPtr = argv[1];
    }
    else if (argc == 3) {
        if (Jim_GetEnum(interp, argv[1], options, &option, NULL, JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK) {
            return JIM_ERR; // #MissInCoverage
        }
        objPtr = argv[2];
    }
    else {
        Jim_WrongNumArgs(interp, 1, argv, "?option? name"); // #MissInCoverage
        return JIM_ERR;
    }

    if (option == OPT_VAR) {
        result = Jim_GetVariable(interp, objPtr, 0) != NULL;
    }
    else {
        /* Now different kinds of commands */
        Jim_Cmd *cmd = Jim_GetCommand(interp, objPtr, JIM_NONE);

        if (cmd) {
            switch (option) {
            case OPT_COMMAND:
                result = 1;
                break;

            case OPT_ALIAS:
                result = cmd->isproc() == 0 && cmd->cmdProc() == JimAliasCmd;
                break;

            case OPT_PROC:
                result = cmd->isproc();
                break;
            }
        }
    }
    Jim_SetResultBool(interp, result);
    return JIM_OK;
}

/* [split] */
static Retval Jim_SplitCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    const char *str, *splitChars, *noMatchStart;
    int splitLen, strLen;
    Jim_ObjPtr resObjPtr;
    int c;
    int len;

    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "string ?splitChars?");
        return JIM_ERR;
    }

    str = Jim_GetString(argv[1], &len);
    if (len == 0) {
        return JIM_OK;
    }
    strLen = Jim_Utf8Length(interp, argv[1]);

    /* Init */
    if (argc == 2) {
        splitChars = " \n\t\r";
        splitLen = 4;
    }
    else {
        splitChars = Jim_String(argv[2]);
        splitLen = Jim_Utf8Length(interp, argv[2]);
    }

    noMatchStart = str;
    resObjPtr = Jim_NewListObj(interp, NULL, 0);

    /* Split */
    if (splitLen) {
        Jim_ObjPtr objPtr;
        while (strLen--) {
            const char *sc = splitChars;
            int scLen = splitLen;
            int sl = utf8_tounicode(str, &c);
            while (scLen--) {
                int pc;
                sc += utf8_tounicode(sc, &pc);
                if (c == pc) {
                    objPtr = Jim_NewStringObj(interp, noMatchStart, 
                        (int)(str - noMatchStart));
                    Jim_ListAppendElement(interp, resObjPtr, objPtr);
                    noMatchStart = str + sl;
                    break;
                }
            }
            str += sl;
        }
        objPtr = Jim_NewStringObj(interp, noMatchStart, 
            (int)(str - noMatchStart));
        Jim_ListAppendElement(interp, resObjPtr, objPtr);
    }
    else {
        /* This handles the special case of splitchars eq {}
         * Optimize by sharing common (ASCII) characters
         */
        Jim_ObjArray* commonObj = NULL;
        enum { NUM_COMMON = (128 - 9) };
        while (strLen--) {
            int n = utf8_tounicode(str, &c);
            if (g_JIM_OPTIMIZATION_VAL) {
            if (c >= 9 && c < 128) { // #MagicNum
                /* Common ASCII char. Note that 9 is the tab character */
                c -= 9; // #MagicNum
                if (!commonObj) {
                    commonObj = new_Jim_ObjArrayZ(NUM_COMMON); // #AllocF 
                    //memset(commonObj, 0, sizeof(*commonObj) * NUM_COMMON);
                }
                if (!commonObj[c]) {
                    commonObj[c] = Jim_NewStringObj(interp, str, 1);
                }
                Jim_ListAppendElement(interp, resObjPtr, commonObj[c]);
                str++;
                continue;
            }
            }
            Jim_ListAppendElement(interp, resObjPtr, Jim_NewStringObjUtf8(interp, str, 1));
            str += n;
        }
        free_Jim_ObjArray(commonObj); // #FreeF
    }

    Jim_SetResult(interp, resObjPtr);
    return JIM_OK;
}

/* [join] */
static Retval Jim_JoinCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    const char *joinStr;
    int joinStrLen;

    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "list ?joinString?");
        return JIM_ERR;
    }
    /* Init */
    if (argc == 2) {
        joinStr = " ";
        joinStrLen = 1;
    }
    else {
        joinStr = Jim_GetString(argv[2], &joinStrLen);
    }
    Jim_SetResult(interp, Jim_ListJoin(interp, argv[1], joinStr, joinStrLen));
    return JIM_OK;
}

/* [format] */
static Retval Jim_FormatCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "formatString ?arg arg ...?");
        return JIM_ERR;
    }
    objPtr = Jim_FormatString(interp, argv[1], argc - 2, argv + 2);
    if (objPtr == NULL)
        return JIM_ERR;
    Jim_SetResult(interp, objPtr);
    return JIM_OK;
}

/* [scan] */
static Retval Jim_ScanCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_Obj* listPtr, **outVec;
    int outc, i;

    if (argc < 3) {
        Jim_WrongNumArgs(interp, 1, argv, "string format ?varName varName ...?");
        return JIM_ERR;
    }
    if (argv[2]->typePtr() != &g_scanFmtStringObjType)
        SetScanFmtFromAny(interp, argv[2]);
    if (FormatGetError(argv[2]) != 0) {
        Jim_SetResultString(interp, FormatGetError(argv[2]), -1);
        return JIM_ERR;
    }
    if (argc > 3) {
        int maxPos = (int)FormatGetMaxPos(argv[2]);
        int count = (int)FormatGetCnvCount(argv[2]);

        if (maxPos > argc - 3) {
            Jim_SetResultString(interp, "\"%n$\" argument index out of range", -1);
            return JIM_ERR;
        }
        else if (count > argc - 3) {
            Jim_SetResultString(interp, "different numbers of variable names and "
                "field specifiers", -1);
            return JIM_ERR;
        }
        else if (count < argc - 3) {
            Jim_SetResultString(interp, "variable is not assigned by any "
                "conversion specifiers", -1);
            return JIM_ERR;
        }
    }
    listPtr = Jim_ScanString(interp, argv[1], argv[2], JIM_ERRMSG);
    if (listPtr == 0)
        return JIM_ERR; // #MissInCoverage
    if (argc > 3) {
        int rc = JIM_OK;
        int count = 0;

        if (listPtr != 0 && listPtr != (Jim_ObjPtr )EOF) {
            int len = Jim_ListLength(interp, listPtr);

            if (len != 0) {
                JimListGetElements(interp, listPtr, &outc, &outVec);
                for (i = 0; i < outc; ++i) {
                    if (Jim_Length(outVec[i]) > 0) {
                        ++count;
                        if (Jim_SetVariable(interp, argv[3 + i], outVec[i]) != JIM_OK) {
                            rc = JIM_ERR;
                        }
                    }
                }
            }
            Jim_FreeNewObj(interp, listPtr);
        }
        else {
            count = -1;
        }
        if (rc == JIM_OK) {
            Jim_SetResultInt(interp, count);
        }
        return rc;
    }
    else {
        if (listPtr == (Jim_ObjPtr )EOF) {
            Jim_SetResult(interp, Jim_NewListObj(interp, 0, 0));
            return JIM_OK;
        }
        Jim_SetResult(interp, listPtr);
    }
    return JIM_OK;
}

/* [error] */
static Retval Jim_ErrorCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    if (argc != 2 && argc != 3) {
        Jim_WrongNumArgs(interp, 1, argv, "message ?stacktrace?"); // #MissInCoverage
        return JIM_ERR;
    }
    Jim_SetResult(interp, argv[1]);
    if (argc == 3) {
        JimSetStackTrace(interp, argv[2]);
        return JIM_ERR;
    }
    interp->incrAddStackTrace();
    return JIM_ERR;
}

/* [lrange] */
static Retval Jim_LrangeCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;

    if (argc != 4) {
        Jim_WrongNumArgs(interp, 1, argv, "list first last");
        return JIM_ERR;
    }
    if ((objPtr = Jim_ListRange(interp, argv[1], argv[2], argv[3])) == NULL)
        return JIM_ERR;
    Jim_SetResult(interp, objPtr);
    return JIM_OK;
}

/* [lrepeat] */
static Retval Jim_LrepeatCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_ObjPtr objPtr;
    long count;

    if (argc < 2 || Jim_GetLong(interp, argv[1], &count) != JIM_OK || count < 0) {
        Jim_WrongNumArgs(interp, 1, argv, "count ?value ...?");
        return JIM_ERR;
    }

    if (count == 0 || argc == 2) {
        return JIM_OK;
    }

    argc -= 2;
    argv += 2;

    objPtr = Jim_NewListObj(interp, argv, argc);
    while (--count) {
        ListInsertElements(objPtr, -1, argc, argv);
    }

    Jim_SetResult(interp, objPtr);
    return JIM_OK;
}

JIM_EXPORT char **Jim_GetEnviron(void)
{
    PRJ_TRACE;
    if (!prj_funcDef(prj_environ)) { // #Unsupported
        return NULL; // #MissInCoverage
    } 
    return prj_environ();
}

JIM_EXPORT void Jim_SetEnviron(char **env) // #TODO how set environment variable #broken #TmpRemoveCmd
{
    PRJ_TRACE;
#if 0
#if defined(HAVE__NSGETENVIRON)
    *_NSGetEnviron() = env;
#else
#if !defined(NO_ENVIRON_EXTERN)
    extern char** environ;
#endif

    environ = env;
#endif
#endif
}

/* [env] */
static Retval Jim_EnvCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    const char *key;
    const char *val;

    if (argc == 1) {
        char **e = Jim_GetEnviron();

        int i;
        Jim_ObjPtr listObjPtr = Jim_NewListObj(interp, NULL, 0);

        for (i = 0; e[i]; i++) {
            const char *equals = strchr(e[i], '=');

            if (equals) {
                Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, e[i],
                        (int)(equals - e[i])));
                Jim_ListAppendElement(interp, listObjPtr, Jim_NewStringObj(interp, equals + 1, -1));
            }
        }

        Jim_SetResult(interp, listObjPtr);
        return JIM_OK;
    }

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "varName ?default?"); // #MissInCoverage
        return JIM_ERR;
    }
    key = Jim_String(argv[1]);
    val = prj_getenv(key); // #NonPortFuncFix
    if (val == NULL) {
        if (argc < 3) {
            Jim_SetResultFormatted(interp, "environment variable \"%#s\" does not exist", argv[1]); // #MissInCoverage
            return JIM_ERR;
        }
        val = Jim_String(argv[2]);
    }
    Jim_SetResult(interp, Jim_NewStringObj(interp, val, -1));
    return JIM_OK;
}

/* [source] */
static Retval Jim_SourceCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    Retval retval;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "fileName"); // #MissInCoverage
        return JIM_ERR;
    }
    retval = Jim_EvalFile(interp, Jim_String(argv[1]));
    if (retval == JIM_RETURN)
        return JIM_OK; // #MissInCoverage
    return retval;
}

/* [lreverse] */
static Retval Jim_LreverseCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #MissInCoverage #JimCoreCmd 
{
    PRJ_TRACE;
    Jim_Obj *revObjPtr, **ele;
    int len;

    if (argc != 2) {
        Jim_WrongNumArgs(interp, 1, argv, "list");
        return JIM_ERR;
    }
    JimListGetElements(interp, argv[1], &len, &ele);
    len--;
    revObjPtr = Jim_NewListObj(interp, NULL, 0);
    while (len >= 0)
        ListAppendElement(revObjPtr, ele[len--]);
    Jim_SetResult(interp, revObjPtr);
    return JIM_OK;
}

static int JimRangeLen(jim_wide start, jim_wide end, jim_wide step)
{
    PRJ_TRACE;
    jim_wide len;

    if (step == 0)
        return -1; // #MissInCoverage
    if (start == end)
        return 0; 
    else if (step > 0 && start > end)
        return -1; // #MissInCoverage
    else if (step < 0 && end > start)
        return -1; // #MissInCoverage
    len = end - start;
    if (len < 0)
        len = -len;             /* abs(len) */
    if (step < 0)
        step = -step;           /* abs(step) */
    len = 1 + ((len - 1) / step);
    /* We can truncate safely to INT_MAX, the range command
     * will always return an error for a such long range
     * because Tcl lists can't be so long. */
    if (len > INT_MAX)
        len = INT_MAX; // #MissInCoverage
    return (int)((len < 0) ? -1 : len);
}

/* [range] */
static Retval Jim_RangeCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    jim_wide start = 0, end, step = 1;
    int len, i;
    Jim_ObjPtr objPtr;

    if (argc < 2 || argc > 4) {
        Jim_WrongNumArgs(interp, 1, argv, "?start? end ?step?"); // #MissInCoverage
        return JIM_ERR;
    }
    if (argc == 2) {
        if (Jim_GetWide(interp, argv[1], &end) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
    }
    else {
        if (Jim_GetWide(interp, argv[1], &start) != JIM_OK ||
            Jim_GetWide(interp, argv[2], &end) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
        if (argc == 4 && Jim_GetWide(interp, argv[3], &step) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
    }
    if ((len = JimRangeLen(start, end, step)) == -1) {
        Jim_SetResultString(interp, "Invalid (infinite?) range specified", -1); // #MissInCoverage
        return JIM_ERR;
    }
    objPtr = Jim_NewListObj(interp, NULL, 0);
    for (i = 0; i < len; i++)
        ListAppendElement(objPtr, Jim_NewIntObj(interp, start + i * step));
    Jim_SetResult(interp, objPtr);
    return JIM_OK;
}

/* [rand] */
static Retval Jim_RandCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd #JimCoreCmd 
{
    PRJ_TRACE;
    jim_wide min = 0, max = 0, len, maxMul;

    if (argc < 1 || argc > 3) {
        Jim_WrongNumArgs(interp, 1, argv, "?min? max"); // #MissInCoverage
        return JIM_ERR;
    }
    if (argc == 1) {
        max = JIM_WIDE_MAX; // #MissInCoverage
    } else if (argc == 2) {
        if (Jim_GetWide(interp, argv[1], &max) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
    } else if (argc == 3) {
        if (Jim_GetWide(interp, argv[1], &min) != JIM_OK ||
            Jim_GetWide(interp, argv[2], &max) != JIM_OK)
            return JIM_ERR; // #MissInCoverage
    }
    len = max-min;
    if (len < 0) {
        Jim_SetResultString(interp, "Invalid arguments (max < min)", -1);
        return JIM_ERR;
    }
    maxMul = JIM_WIDE_MAX - (len ? (JIM_WIDE_MAX%len) : 0);
    while (1) {
        jim_wide r;

        JimRandomBytes(interp, &r, sizeof(jim_wide));
        if (r < 0 || r >= maxMul) continue;
        r = (len == 0) ? 0 : r%len;
        Jim_SetResultInt(interp, min+r);
        return JIM_OK;
    }
}

static const struct {
    const char *name;
    Jim_CmdProc *cmdProc;
} g_Jim_CoreCommandsTable[] = {
    {"alias", Jim_AliasCoreCommand},
    {"set", Jim_SetCoreCommand},
    {"unset", Jim_UnsetCoreCommand},
    {"puts", Jim_PutsCoreCommand},
    {"+", Jim_AddCoreCommand},
    {"*", Jim_MulCoreCommand},
    {"-", Jim_SubCoreCommand},
    {"/", Jim_DivCoreCommand},
    {"incr", Jim_IncrCoreCommand},
    {"while", Jim_WhileCoreCommand},
    {"loop", Jim_LoopCoreCommand},
    {"for", Jim_ForCoreCommand},
    {"foreach", Jim_ForeachCoreCommand},
    {"lmap", Jim_LmapCoreCommand},
    {"lassign", Jim_LassignCoreCommand},
    {"if", Jim_IfCoreCommand},
    {"switch", Jim_SwitchCoreCommand},
    {"list", Jim_ListCoreCommand},
    {"lindex", Jim_LindexCoreCommand},
    {"lset", Jim_LsetCoreCommand},
    {"lsearch", Jim_LsearchCoreCommand},
    {"llength", Jim_LlengthCoreCommand},
    {"lappend", Jim_LappendCoreCommand},
    {"linsert", Jim_LinsertCoreCommand},
    {"lreplace", Jim_LreplaceCoreCommand},
    {"lsort", Jim_LsortCoreCommand},
    {"append", Jim_AppendCoreCommand},
    {"debug", Jim_DebugCoreCommand},
    {"eval", Jim_EvalCoreCommand},
    {"uplevel", Jim_UplevelCoreCommand},
    {"expr", Jim_ExprCoreCommand},
    {"break", Jim_BreakCoreCommand},
    {"continue", Jim_ContinueCoreCommand},
    {"proc", Jim_ProcCoreCommand},
    {"concat", Jim_ConcatCoreCommand},
    {"return", Jim_ReturnCoreCommand},
    {"upvar", Jim_UpvarCoreCommand},
    {"global", Jim_GlobalCoreCommand},
    {"string", Jim_StringCoreCommand},
    {"time", Jim_TimeCoreCommand},
    {"exit", Jim_ExitCoreCommand},
    {"catch", Jim_CatchCoreCommand},
#ifdef JIM_REFERENCES // #optionalCode
    {"ref", Jim_RefCoreCommand},
    {"getref", Jim_GetrefCoreCommand},
    {"setref", Jim_SetrefCoreCommand},
    {"finalize", Jim_FinalizeCoreCommand},
    {"collect", Jim_CollectCoreCommand},
#endif
    {"rename", Jim_RenameCoreCommand},
    {"dict", Jim_DictCoreCommand},
    {"subst", Jim_SubstCoreCommand},
    {"info", Jim_InfoCoreCommand},
    {"exists", Jim_ExistsCoreCommand},
    {"split", Jim_SplitCoreCommand},
    {"join", Jim_JoinCoreCommand},
    {"format", Jim_FormatCoreCommand},
    {"scan", Jim_ScanCoreCommand},
    {"error", Jim_ErrorCoreCommand},
    {"lrange", Jim_LrangeCoreCommand},
    {"lrepeat", Jim_LrepeatCoreCommand},
    {"env", Jim_EnvCoreCommand},
    {"source", Jim_SourceCoreCommand},
    {"lreverse", Jim_LreverseCoreCommand},
    {"range", Jim_RangeCoreCommand},
    {"rand", Jim_RandCoreCommand},
    {"tailcall", Jim_TailcallCoreCommand},
    {"local", Jim_LocalCoreCommand},
    {"upcall", Jim_UpcallCoreCommand},
    {"apply", Jim_ApplyCoreCommand},
    {NULL, NULL},
};

JIM_EXPORT void Jim_RegisterCoreCommands(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    int i = 0;

    while (g_Jim_CoreCommandsTable[i].name != NULL) {
        Jim_CreateCommand(interp,
            g_Jim_CoreCommandsTable[i].name, g_Jim_CoreCommandsTable[i].cmdProc, NULL, NULL);
        i++;
    }
}

/* -----------------------------------------------------------------------------
 * Interactive prompt
 * ---------------------------------------------------------------------------*/
JIM_EXPORT void Jim_MakeErrorMessage(Jim_InterpPtr interp)
{
    PRJ_TRACE;
    Jim_ObjPtr argv[2];

    argv[0] = Jim_NewStringObj(interp, "errorInfo", -1); // #MissInCoverage
    argv[1] = interp->result();

    Jim_EvalObjVector(interp, 2, argv);
}

/*
 * Given a null terminated array of strings, returns an allocated, sorted
 * copy of the array.
 */
static char **JimSortStringTable(const char *const *tablePtr)
{
    PRJ_TRACE;
    int count;
    char **tablePtrSorted;

    /* Find the size of the table */
    for (count = 0; tablePtr[count]; count++) {
    }

    /* Allocate one extra for the terminating NULL pointer */
    tablePtrSorted = Jim_TAlloc<charArray>((count + 1),"charArray"); // #AllocF 
    memcpy(tablePtrSorted, tablePtr, sizeof(char *) * count);
    qsort(tablePtrSorted, count, sizeof(char *), qsortCompareStringPointers);
    tablePtrSorted[count] = NULL;

    return tablePtrSorted;
}

static void JimSetFailedEnumResult(Jim_InterpPtr interp, const char *arg, const char *badtype,
    const char *prefix, const char *const *tablePtr, const char *name)
{
    PRJ_TRACE;
    char **tablePtrSorted;
    int i;

    if (name == NULL) {
        name = "option";
    }

    Jim_SetResultFormatted(interp, "%s%s \"%s\": must be ", badtype, name, arg);
    tablePtrSorted = JimSortStringTable(tablePtr);
    for (i = 0; tablePtrSorted[i]; i++) {
        if (tablePtrSorted[i + 1] == NULL && i > 0) {
            Jim_AppendString(interp, Jim_GetResult(interp), "or ", -1);
        }
        Jim_AppendStrings(interp, Jim_GetResult(interp), prefix, tablePtrSorted[i], NULL);
        if (tablePtrSorted[i + 1]) {
            Jim_AppendString(interp, Jim_GetResult(interp), ", ", -1);
        }
    }
    Jim_TFree<charArray>(tablePtrSorted,"charArray"); // #FreeF
}


/*
 * If objPtr is "-commands" sets the Jim result as a sorted list of options in the table
 * and returns JIM_OK.
 *
 * Otherwise returns JIM_ERR.
 */
JIM_EXPORT Retval Jim_CheckShowCommands(Jim_InterpPtr interp, Jim_ObjPtr objPtr, const char *const *tablePtr)
{
    PRJ_TRACE;
    if (Jim_CompareStringImmediate(interp, objPtr, "-commands")) {
        int i;
        char **tablePtrSorted = JimSortStringTable(tablePtr); // #MissInCoverage
        Jim_SetResult(interp, Jim_NewListObj(interp, NULL, 0));
        for (i = 0; tablePtrSorted[i]; i++) {
            Jim_ListAppendElement(interp, Jim_GetResult(interp), Jim_NewStringObj(interp, tablePtrSorted[i], -1));
        }
        Jim_TFree<charArray>(tablePtrSorted,"charArray"); // #FreeF
        return JIM_OK;
    }
    return JIM_ERR;
}

/* internal rep is stored in ptrIntvalue
 *  ptr = tablePtr
 *  int1 = flags
 *  int2 = index
 */
static const Jim_ObjType g_getEnumObjType = { // #JimType #JimEum
    "get-enum",
    NULL,
    NULL,
    NULL,
    JIM_TYPE_REFERENCES
};
const Jim_ObjType& getEnumType() { return g_getEnumObjType; }

JIM_EXPORT Retval Jim_GetEnum(Jim_InterpPtr interp, Jim_ObjPtr objPtr, // #JimEum
    const char * const *tablePtr, int *indexPtr, const char *name, int flags)
{
    PRJ_TRACE;
    const char *bad = "bad ";
    const char *const *entryPtr = NULL;
    int i;
    int match = -1;
    int arglen;
    const char *arg;

    if (objPtr->typePtr() == &g_getEnumObjType) {
        if (objPtr->get_ptrInt_ptr() == tablePtr && objPtr->get_ptrInt_int1() == flags) {
            *indexPtr = objPtr->get_ptrInt_int2();
            return JIM_OK;
        }
    }

    arg = Jim_GetString(objPtr, &arglen);

    *indexPtr = -1;

    for (entryPtr = tablePtr, i = 0; *entryPtr != NULL; entryPtr++, i++) {
        if (Jim_CompareStringImmediate(interp, objPtr, *entryPtr)) {
            /* Found an exact match */
            match = i;
            goto found;
        }
        if (flags & JIM_ENUM_ABBREV) {
            /* Accept an unambiguous abbreviation.
             * Note that '-' doesnt' constitute a valid abbreviation
             */
            if (strncmp(arg, *entryPtr, arglen) == 0) {
                if (*arg == '-' && arglen == 1) {
                    break;
                }
                if (match >= 0) {
                    bad = "ambiguous ";
                    goto ambiguous;
                }
                match = i;
            }
        }
    }

    /* If we had an unambiguous partial match */
    if (match >= 0) {
  found:
        /* Record the match in the object */
        Jim_FreeIntRep(interp, objPtr);
        objPtr->setTypePtr(&g_getEnumObjType);
        objPtr->setPtrInt2<char**>((char**)tablePtr, flags, match);
        //objPtr->internalRep.ptrIntValue_.ptr = (void *)tablePtr;
        //objPtr->internalRep.ptrIntValue_.int1 = flags;
        //objPtr->internalRep.ptrIntValue_.int2 = match;
        /* Return the result */
        *indexPtr = match;
        return JIM_OK;
    }

  ambiguous:
    if (flags & JIM_ERRMSG) {
        JimSetFailedEnumResult(interp, arg, bad, "", tablePtr, name);
    }
    return JIM_ERR;
}

JIM_EXPORT int Jim_FindByName(const char *name, const char * const array[], size_t len)
{
    PRJ_TRACE;
    int i;

    for (i = 0; i < (int)len; i++) {
        if (array[i] && strcmp(array[i], name) == 0) {
            return i;
        }
    }
    return -1; // #MissInCoverage
}

JIM_EXPORT int Jim_IsDict(Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    return objPtr->typePtr() == &g_dictObjType;
}

JIM_EXPORT int Jim_IsList(Jim_ObjPtr objPtr)
{
    PRJ_TRACE;
    return objPtr->typePtr() == &g_listObjType;
}

/**
 * Very simple printf-like formatting, designed for error messages.
 *
 * The format may contain up to 5 '%s' or '%#s', corresponding to variable arguments.
 * The resulting string is created and set as the result.
 *
 * Each '%s' should correspond to a regular string parameter.
 * Each '%#s' should correspond to a (Jim_ObjPtr ) parameter.
 * Any other printf specifier is not allowed (but %% is allowed for the % character).
 *
 * e.g. Jim_SetResultFormatted(interp, "Bad option \"%#s\" in proc \"%#s\"", optionObjPtr, procNamePtr);
 *
 * Note: We take advantage of the fact that printf has the same behavior for both %s and %#s
 *
 * Note that any Jim_Obj parameters with zero ref count will be freed as a result of this call.
 */
JIM_EXPORT void Jim_SetResultFormatted(Jim_InterpPtr interp, const char *format, ...)
{
    PRJ_TRACE;
    /* Initial space needed */
    int len = (int)strlen(format);
    int extra = 0;
    int n = 0;
    const char *params[5] = { 0 };
    int nobjparam = 0;
    Jim_ObjPtr objparam[5];
    char *buf;
    va_list args;
    int i;

    va_start(args, format);

    for (i = 0; i < len && n < 5; i++) {
        int l;

        if (strncmp(format + i, "%s", 2) == 0) {
            params[n] = va_arg(args, char *);

            l = (int)strlen(params[n]);
        }
        else if (strncmp(format + i, "%#s", 3) == 0) {
            Jim_ObjPtr objPtr = va_arg(args, Jim_ObjPtr );

            params[n] = Jim_GetString(objPtr, &l);
            objparam[nobjparam++] = objPtr;
            Jim_IncrRefCount(objPtr);
        }
        else {
            if (format[i] == '%') {
                i++; // #MissInCoverage
            }
            continue;
        }
        n++;
        extra += l;
    }

    len += extra;
    buf = new_CharArray(len + 1); // #AllocF 
    len = snprintf(buf, len + 1, format, params[0], params[1], params[2], params[3], params[4]);

    va_end(args);

    Jim_SetResult(interp, Jim_NewStringObjNoAlloc(interp, buf, len));

    for (i = 0; i < nobjparam; i++) {
        Jim_DecrRefCount(interp, objparam[i]);
    }
}

/* stubs */
#ifndef jim_ext_package // #optionalCode #WinOff
JIM_EXPORT Retval Jim_PackageProvide(Jim_InterpPtr interp, const char *name, const char *ver, int flags)
{
    PRJ_TRACE;
    return JIM_OK;
}
#endif
#ifndef jim_ext_aio // #optionalCode #WinOff
FILE *Jim_AioFilehandle(Jim_InterpPtr interp, Jim_ObjPtr fhObj)
{
    PRJ_TRACE;
    Jim_SetResultString(interp, "aio not enabled", -1);
    return NULL;
}
#endif


/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * tab-width: 4 ***
 * End: ***
 */
END_JIM_NAMESPACE
