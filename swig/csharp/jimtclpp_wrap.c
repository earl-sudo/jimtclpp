/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.1
 *
 * This file is not intended to be easily readable and contains a number of
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG
 * interface file instead.
 * ----------------------------------------------------------------------------- */


#ifndef SWIGCSHARP
#define SWIGCSHARP
#endif


/* -----------------------------------------------------------------------------
 *  This section contains generic SWIG labels for method/variable
 *  declarations/attributes, and other compiler dependent labels.
 * ----------------------------------------------------------------------------- */

/* template workaround for compilers that cannot correctly implement the C++ standard */
#ifndef SWIGTEMPLATEDISAMBIGUATOR
# if defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x560)
#  define SWIGTEMPLATEDISAMBIGUATOR template
# elif defined(__HP_aCC)
/* Needed even with `aCC -AA' when `aCC -V' reports HP ANSI C++ B3910B A.03.55 */
/* If we find a maximum version that requires this, the test would be __HP_aCC <= 35500 for A.03.55 */
#  define SWIGTEMPLATEDISAMBIGUATOR template
# else
#  define SWIGTEMPLATEDISAMBIGUATOR
# endif
#endif

/* inline attribute */
#ifndef SWIGINLINE
# if defined(__cplusplus) || (defined(__GNUC__) && !defined(__STRICT_ANSI__))
#   define SWIGINLINE inline
# else
#   define SWIGINLINE
# endif
#endif

/* attribute recognised by some compilers to avoid 'unused' warnings */
#ifndef SWIGUNUSED
# if defined(__GNUC__)
#   if !(defined(__cplusplus)) || (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#     define SWIGUNUSED __attribute__ ((__unused__))
#   else
#     define SWIGUNUSED
#   endif
# elif defined(__ICC)
#   define SWIGUNUSED __attribute__ ((__unused__))
# else
#   define SWIGUNUSED
# endif
#endif

#ifndef SWIG_MSC_UNSUPPRESS_4505
# if defined(_MSC_VER)
#   pragma warning(disable : 4505) /* unreferenced local function has been removed */
# endif
#endif

#ifndef SWIGUNUSEDPARM
# ifdef __cplusplus
#   define SWIGUNUSEDPARM(p)
# else
#   define SWIGUNUSEDPARM(p) p SWIGUNUSED
# endif
#endif

/* internal SWIG method */
#ifndef SWIGINTERN
# define SWIGINTERN static SWIGUNUSED
#endif

/* internal inline SWIG method */
#ifndef SWIGINTERNINLINE
# define SWIGINTERNINLINE SWIGINTERN SWIGINLINE
#endif

/* exporting methods */
#if defined(__GNUC__)
#  if (__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#    ifndef GCC_HASCLASSVISIBILITY
#      define GCC_HASCLASSVISIBILITY
#    endif
#  endif
#endif

#ifndef SWIGEXPORT
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   if defined(STATIC_LINKED)
#     define SWIGEXPORT
#   else
#     define SWIGEXPORT __declspec(dllexport)
#   endif
# else
#   if defined(__GNUC__) && defined(GCC_HASCLASSVISIBILITY)
#     define SWIGEXPORT __attribute__ ((visibility("default")))
#   else
#     define SWIGEXPORT
#   endif
# endif
#endif

/* calling conventions for Windows */
#ifndef SWIGSTDCALL
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   define SWIGSTDCALL __stdcall
# else
#   define SWIGSTDCALL
# endif
#endif

/* Deal with Microsoft's attempt at deprecating C standard runtime functions */
#if !defined(SWIG_NO_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE
#endif

/* Deal with Microsoft's attempt at deprecating methods in the standard C++ library */
#if !defined(SWIG_NO_SCL_SECURE_NO_DEPRECATE) && defined(_MSC_VER) && !defined(_SCL_SECURE_NO_DEPRECATE)
# define _SCL_SECURE_NO_DEPRECATE
#endif

/* Deal with Apple's deprecated 'AssertMacros.h' from Carbon-framework */
#if defined(__APPLE__) && !defined(__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES)
# define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#endif

/* Intel's compiler complains if a variable which was never initialised is
 * cast to void, which is a common idiom which we use to indicate that we
 * are aware a variable isn't used.  So we just silence that warning.
 * See: https://github.com/swig/swig/issues/192 for more discussion.
 */
#ifdef __INTEL_COMPILER
# pragma warning disable 592
#endif


#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/* Support for throwing C# exceptions from C/C++. There are two types: 
 * Exceptions that take a message and ArgumentExceptions that take a message and a parameter name. */
typedef enum {
  SWIG_CSharpApplicationException,
  SWIG_CSharpArithmeticException,
  SWIG_CSharpDivideByZeroException,
  SWIG_CSharpIndexOutOfRangeException,
  SWIG_CSharpInvalidCastException,
  SWIG_CSharpInvalidOperationException,
  SWIG_CSharpIOException,
  SWIG_CSharpNullReferenceException,
  SWIG_CSharpOutOfMemoryException,
  SWIG_CSharpOverflowException,
  SWIG_CSharpSystemException
} SWIG_CSharpExceptionCodes;

typedef enum {
  SWIG_CSharpArgumentException,
  SWIG_CSharpArgumentNullException,
  SWIG_CSharpArgumentOutOfRangeException
} SWIG_CSharpExceptionArgumentCodes;

typedef void (SWIGSTDCALL* SWIG_CSharpExceptionCallback_t)(const char *);
typedef void (SWIGSTDCALL* SWIG_CSharpExceptionArgumentCallback_t)(const char *, const char *);

typedef struct {
  SWIG_CSharpExceptionCodes code;
  SWIG_CSharpExceptionCallback_t callback;
} SWIG_CSharpException_t;

typedef struct {
  SWIG_CSharpExceptionArgumentCodes code;
  SWIG_CSharpExceptionArgumentCallback_t callback;
} SWIG_CSharpExceptionArgument_t;

static SWIG_CSharpException_t SWIG_csharp_exceptions[] = {
  { SWIG_CSharpApplicationException, NULL },
  { SWIG_CSharpArithmeticException, NULL },
  { SWIG_CSharpDivideByZeroException, NULL },
  { SWIG_CSharpIndexOutOfRangeException, NULL },
  { SWIG_CSharpInvalidCastException, NULL },
  { SWIG_CSharpInvalidOperationException, NULL },
  { SWIG_CSharpIOException, NULL },
  { SWIG_CSharpNullReferenceException, NULL },
  { SWIG_CSharpOutOfMemoryException, NULL },
  { SWIG_CSharpOverflowException, NULL },
  { SWIG_CSharpSystemException, NULL }
};

static SWIG_CSharpExceptionArgument_t SWIG_csharp_exceptions_argument[] = {
  { SWIG_CSharpArgumentException, NULL },
  { SWIG_CSharpArgumentNullException, NULL },
  { SWIG_CSharpArgumentOutOfRangeException, NULL }
};

static void SWIGUNUSED SWIG_CSharpSetPendingException(SWIG_CSharpExceptionCodes code, const char *msg) {
  SWIG_CSharpExceptionCallback_t callback = SWIG_csharp_exceptions[SWIG_CSharpApplicationException].callback;
  if ((size_t)code < sizeof(SWIG_csharp_exceptions)/sizeof(SWIG_CSharpException_t)) {
    callback = SWIG_csharp_exceptions[code].callback;
  }
  callback(msg);
}

static void SWIGUNUSED SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpExceptionArgumentCodes code, const char *msg, const char *param_name) {
  SWIG_CSharpExceptionArgumentCallback_t callback = SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentException].callback;
  if ((size_t)code < sizeof(SWIG_csharp_exceptions_argument)/sizeof(SWIG_CSharpExceptionArgument_t)) {
    callback = SWIG_csharp_exceptions_argument[code].callback;
  }
  callback(msg, param_name);
}


#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionCallbacks_jimtclpp(
                                                SWIG_CSharpExceptionCallback_t applicationCallback,
                                                SWIG_CSharpExceptionCallback_t arithmeticCallback,
                                                SWIG_CSharpExceptionCallback_t divideByZeroCallback, 
                                                SWIG_CSharpExceptionCallback_t indexOutOfRangeCallback, 
                                                SWIG_CSharpExceptionCallback_t invalidCastCallback,
                                                SWIG_CSharpExceptionCallback_t invalidOperationCallback,
                                                SWIG_CSharpExceptionCallback_t ioCallback,
                                                SWIG_CSharpExceptionCallback_t nullReferenceCallback,
                                                SWIG_CSharpExceptionCallback_t outOfMemoryCallback, 
                                                SWIG_CSharpExceptionCallback_t overflowCallback, 
                                                SWIG_CSharpExceptionCallback_t systemCallback) {
  SWIG_csharp_exceptions[SWIG_CSharpApplicationException].callback = applicationCallback;
  SWIG_csharp_exceptions[SWIG_CSharpArithmeticException].callback = arithmeticCallback;
  SWIG_csharp_exceptions[SWIG_CSharpDivideByZeroException].callback = divideByZeroCallback;
  SWIG_csharp_exceptions[SWIG_CSharpIndexOutOfRangeException].callback = indexOutOfRangeCallback;
  SWIG_csharp_exceptions[SWIG_CSharpInvalidCastException].callback = invalidCastCallback;
  SWIG_csharp_exceptions[SWIG_CSharpInvalidOperationException].callback = invalidOperationCallback;
  SWIG_csharp_exceptions[SWIG_CSharpIOException].callback = ioCallback;
  SWIG_csharp_exceptions[SWIG_CSharpNullReferenceException].callback = nullReferenceCallback;
  SWIG_csharp_exceptions[SWIG_CSharpOutOfMemoryException].callback = outOfMemoryCallback;
  SWIG_csharp_exceptions[SWIG_CSharpOverflowException].callback = overflowCallback;
  SWIG_csharp_exceptions[SWIG_CSharpSystemException].callback = systemCallback;
}

#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterExceptionArgumentCallbacks_jimtclpp(
                                                SWIG_CSharpExceptionArgumentCallback_t argumentCallback,
                                                SWIG_CSharpExceptionArgumentCallback_t argumentNullCallback,
                                                SWIG_CSharpExceptionArgumentCallback_t argumentOutOfRangeCallback) {
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentException].callback = argumentCallback;
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentNullException].callback = argumentNullCallback;
  SWIG_csharp_exceptions_argument[SWIG_CSharpArgumentOutOfRangeException].callback = argumentOutOfRangeCallback;
}


/* Callback for returning strings to C# without leaking memory */
typedef char * (SWIGSTDCALL* SWIG_CSharpStringHelperCallback)(const char *);
static SWIG_CSharpStringHelperCallback SWIG_csharp_string_callback = NULL;


#ifdef __cplusplus
extern "C" 
#endif
SWIGEXPORT void SWIGSTDCALL SWIGRegisterStringCallback_jimtclpp(SWIG_CSharpStringHelperCallback callback) {
  SWIG_csharp_string_callback = callback;
}


/* Contract support */

#define SWIG_contract_assert(nullreturn, expr, msg) if (!(expr)) {SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentOutOfRangeException, msg, ""); return nullreturn; } else



    /* -----------------------------------------------------------------------------
     * System configuration
     * autoconf (configure) will set these
     * ---------------------------------------------------------------------------*/

     /* -----------------------------------------------------------------------------
      * Compiler specific fixes.
      * ---------------------------------------------------------------------------*/

      /* Long Long type and related issues */
     typedef long long jim_wide;


 /* -----------------------------------------------------------------------------
  * Exported defines
  * ---------------------------------------------------------------------------*/
    enum JIM_INTERP_FLAG_FLAGS {
        JIM_NONE = 0,           /* no flags set */
        JIM_ERRMSG = 1,         /* set an error message in the interpreter. */
        JIM_ENUM_ABBREV = 2,    /* Jim_GetEnum() - Allow unambiguous abbreviation */
        JIM_UNSHARED = 4,       /* Jim_GetVariable() - return unshared object */
        JIM_MUSTEXIST = 8       /* Jim_SetDictKeysVector() - fail if non-existent */
    };

    enum JIM_RETURNS {
        JIM_OK,
        JIM_ERR,
        JIM_RETURN,
        JIM_BREAK,
        JIM_CONTINUE,
        JIM_SIGNAL,
        JIM_EXIT,
        /* The following are internal codes and should never been seen/used */
        JIM_EVAL
    };

    /* Filesystem related */
    enum {
        JIM_PATH_LEN = 1024
    };

#define JIM_LIBPATH "auto_path"

    /* -----------------------------------------------------------------------------
     * Forwards
     * ---------------------------------------------------------------------------*/
    struct Jim_Stack;
    struct Jim_HashEntry;
    struct Jim_HashTableType;
    struct Jim_HashTable;
    struct Jim_HashTableIterator;
    struct Jim_Var;
    struct Jim_Obj;
    struct Jim_Cmd;
    struct Jim_Reference;
    struct Jim_ObjType;
    struct Jim_Interp;
    struct Jim_CallFrame;

    typedef unsigned long long      unsigned_long_long;
    typedef long long               long_long;
    typedef unsigned short          unsigned_short;
    typedef unsigned long           unsigned_long;
    typedef unsigned char           unsigned_char;
    typedef const unsigned char     const_unsigned_char;
    typedef const unsigned long     const_unsigned_long;
    typedef unsigned int            unsigned_int;
    typedef unsigned                unsigned_t;
    typedef unsigned jim_wide       unsigned_jim_wide;
    typedef int                     Retval;
    typedef Jim_HashEntry* Jim_HashEntryArray;
    typedef Jim_HashEntry* Jim_HashEntryPtr;
    typedef void* VoidPtrArray;
    typedef Jim_Obj* Jim_ObjArray;
    typedef char* charArray;
    typedef const char* constCharArray;
    typedef Jim_Obj* const* Jim_ObjConstArray;
    typedef Jim_Stack* Jim_StackPtr;
    typedef Jim_HashTable* Jim_HashTablePtr;
    typedef Jim_Interp* Jim_InterpPtr;
    typedef Jim_Obj* Jim_ObjPtr;

#define JIM_CEXPORT
#define JIM_CAPI_INLINE 


    typedef int Jim_CmdProc(Jim_InterpPtr interp, int argc,
                            Jim_ObjConstArray argv);
    typedef void Jim_DelCmdProc(Jim_InterpPtr interp, void* privData);

    JIM_CAPI_INLINE long Jim_GetId(Jim_InterpPtr  i);

    /* -----------------------------------------------------------------------------
     * Exported API prototypes.
     * ---------------------------------------------------------------------------*/


    void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr  objPtr); /* EJ HACK #TODO */
    JIM_CAPI_INLINE void Jim_IncrRefCount(Jim_ObjPtr  objPtr);
    JIM_CAPI_INLINE void Jim_DecrRefCount(Jim_InterpPtr  interp, Jim_ObjPtr  objPtr);
    JIM_CEXPORT int  Jim_RefCount(Jim_ObjPtr  objPtr);
    JIM_CAPI_INLINE int Jim_IsShared(Jim_ObjPtr  objPtr);

    /* Memory allocation */
    JIM_CEXPORT void* Jim_Alloc(int sizeInBytes);
    JIM_CEXPORT void* Jim_Realloc(void* ptr, int sizeInBytes);
    JIM_CEXPORT void Jim_Free(void* ptr);
    JIM_CEXPORT char* Jim_StrDup(const char* s);
    JIM_CEXPORT char* Jim_StrDupLen(const char* s, int l /* num 1 byte characters */);


    /* environment */
    JIM_CEXPORT char** Jim_GetEnviron(void);
    JIM_CEXPORT void Jim_SetEnviron(char** env);
    JIM_CEXPORT int Jim_MakeTempFile(Jim_InterpPtr interp,
                                    const char* filename_template, int unlink_file /*bool*/);
    //
    /* evaluation */
    JIM_CEXPORT Retval Jim_Eval(Jim_InterpPtr interp, const char* script);
    /* in C code, you can do this and get better error messages */
    /*   Jim_EvalSource( interp, __FILE__, __LINE__ , "some tcl commands"); */
    JIM_CEXPORT Retval Jim_EvalSource(Jim_InterpPtr interp, const char* filename,
                                     int lineno, const char* script);
    /* Backwards compatibility */
    //inline Retval Jim_Eval_Named(Jim_InterpPtr  I, const char* S, const char* F, int L) {
    //    return Jim_EvalSource(I, F, L, S);
    //}

    JIM_CEXPORT Retval Jim_EvalGlobal(Jim_InterpPtr interp, const char* script);
    JIM_CEXPORT Retval Jim_EvalFile(Jim_InterpPtr interp, const char* filename);
    JIM_CEXPORT Retval Jim_EvalFileGlobal(Jim_InterpPtr interp, const char* filename);
    JIM_CEXPORT Retval Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr  scriptObjPtr);
    JIM_CEXPORT Retval Jim_EvalObjVector(Jim_InterpPtr interp, int objc,
                                        Jim_ObjConstArray objv);
    JIM_CEXPORT Retval Jim_EvalObjList(Jim_InterpPtr interp, Jim_ObjPtr  listObj);
    JIM_CEXPORT Retval Jim_EvalObjPrefix(Jim_InterpPtr interp, Jim_ObjPtr  prefix,
                                        int objc, Jim_ObjConstArray objv);
    //inline Retval Jim_EvalPrefix_(Jim_InterpPtr  i, const char* p, int oc, Jim_ObjConstArray  ov);
    JIM_CEXPORT Retval Jim_EvalNamespace(Jim_InterpPtr interp, Jim_ObjPtr  scriptObj, Jim_ObjPtr  nsObj);
    JIM_CEXPORT Retval Jim_SubstObj(Jim_InterpPtr interp, Jim_ObjPtr  substObjPtr,
                                   Jim_ObjArray* resObjPtrPtr, int flags);

    /* stack */
    JIM_CEXPORT Jim_StackPtr  Jim_AllocStack(void);
    JIM_CEXPORT void Jim_InitStack(Jim_StackPtr stack);
    JIM_CEXPORT void Jim_FreeStack(Jim_StackPtr stack);
    JIM_CEXPORT int Jim_StackLen(Jim_StackPtr stack);
    JIM_CEXPORT void Jim_StackPush(Jim_StackPtr stack, void* element);
    JIM_CEXPORT void* Jim_StackPop(Jim_StackPtr stack);
    JIM_CEXPORT void* Jim_StackPeek(Jim_StackPtr stack);
    JIM_CEXPORT void Jim_FreeStackElements(Jim_StackPtr stack, void(*freeFunc)(void* ptr));

    /* hash table */
    JIM_CEXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht,
                                        const Jim_HashTableType* type, void* privdata);
    JIM_CEXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht,
                                        unsigned_int size);
    JIM_CEXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void* key,
                                       void* val);
    JIM_CEXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht,
                                        const void* key, void* val);
    JIM_CEXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht,
                                          const void* key);
    JIM_CEXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht);
    JIM_CEXPORT Jim_HashEntryPtr  Jim_FindHashEntry(Jim_HashTablePtr ht,
                                                   const void* key);
    JIM_CEXPORT void Jim_ResizeHashTable(Jim_HashTablePtr ht);
    JIM_CEXPORT Jim_HashTableIterator* Jim_GetHashTableIterator(Jim_HashTablePtr ht);
    JIM_CEXPORT Jim_HashEntryPtr  Jim_NextHashEntry(Jim_HashTableIterator* iter);
    JIM_CEXPORT const char* Jim_KeyAsStr(Jim_HashEntryPtr  he);
    JIM_CEXPORT const void* Jim_KeyAsVoid(Jim_HashEntryPtr  he);

    /* objects */
    JIM_CEXPORT Jim_ObjPtr  Jim_NewObj(Jim_InterpPtr interp);
    JIM_CEXPORT void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr  objPtr);
    JIM_CEXPORT void Jim_InvalidateStringRep(Jim_ObjPtr  objPtr);
    JIM_CEXPORT Jim_ObjPtr  Jim_DuplicateObj(Jim_InterpPtr interp,
                                         Jim_ObjPtr  objPtr);
    JIM_CEXPORT const char* Jim_GetString(Jim_ObjPtr  objPtr,
                                         int* lenPtr);
    JIM_CEXPORT const char* Jim_String(Jim_ObjPtr  objPtr);
    JIM_CEXPORT int Jim_Length(Jim_ObjPtr  objPtr);

    /* string object */
    JIM_CEXPORT Jim_ObjPtr  Jim_NewStringObj(Jim_InterpPtr interp,
                                         const char* s, int len /* -1 means strlen(s) */);
    JIM_CEXPORT Jim_ObjPtr  Jim_NewStringObjUtf8(Jim_InterpPtr interp,
                                             const char* s, int charlen /* num chars */);
    JIM_CEXPORT Jim_ObjPtr  Jim_NewStringObjNoAlloc(Jim_InterpPtr interp,
                                                char* s, int len /* -1 means strlen(s) */);
    JIM_CEXPORT void Jim_AppendString(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                     const char* str, int len /* -1 means strlen(s) */);
    JIM_CEXPORT void Jim_AppendObj(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  Jim_ObjPtr  appendObjPtr);
    JIM_CEXPORT void Jim_AppendStrings(Jim_InterpPtr interp,
                                      Jim_ObjPtr  objPtr, ...);
    JIM_CEXPORT int Jim_StringEqObj(Jim_ObjPtr  aObjPtr, Jim_ObjPtr  bObjPtr);
    JIM_CEXPORT int Jim_StringMatchObj(Jim_InterpPtr interp, Jim_ObjPtr  patternObjPtr,
                                      Jim_ObjPtr  objPtr, int nocase /*bool*/);
    JIM_CEXPORT Jim_ObjPtr  Jim_StringRangeObj(Jim_InterpPtr interp,
                                           Jim_ObjPtr  strObjPtr, Jim_ObjPtr  firstObjPtr,
                                           Jim_ObjPtr  lastObjPtr);
    JIM_CEXPORT Jim_ObjPtr  Jim_FormatString(Jim_InterpPtr interp,
                                         Jim_ObjPtr  fmtObjPtr, int objc, Jim_ObjConstArray objv);
    JIM_CEXPORT Jim_ObjPtr  Jim_ScanString(Jim_InterpPtr interp, Jim_ObjPtr  strObjPtr,
                                       Jim_ObjPtr  fmtObjPtr, int flags);
    JIM_CEXPORT int Jim_CompareStringImmediate(Jim_InterpPtr interp,
                                              Jim_ObjPtr  objPtr, const char* str);
    JIM_CEXPORT int Jim_StringCompareObj(Jim_InterpPtr interp, Jim_ObjPtr  firstObjPtr,
                                        Jim_ObjPtr  secondObjPtr, int nocase /*bool*/);
    JIM_CEXPORT int Jim_StringCompareLenObj(Jim_InterpPtr interp, Jim_ObjPtr  firstObjPtr,
                                           Jim_ObjPtr  secondObjPtr, int nocase /*bool*/);
    JIM_CEXPORT int Jim_Utf8Length(Jim_InterpPtr interp, Jim_ObjPtr  objPtr);

    /* reference object */
    JIM_CEXPORT Jim_ObjPtr  Jim_NewReference(Jim_InterpPtr interp,
                                         Jim_ObjPtr  objPtr, Jim_ObjPtr  tagPtr, Jim_ObjPtr  cmdNamePtr);
    JIM_CEXPORT Jim_Reference* Jim_GetReference(Jim_InterpPtr interp,
                                               Jim_ObjPtr  objPtr);
    JIM_CEXPORT Retval Jim_SetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjPtr  cmdNamePtr);
    JIM_CEXPORT Retval Jim_GetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjArray* cmdNamePtrPtr);

    /* interpreter */
    JIM_CEXPORT Jim_InterpPtr  Jim_CreateInterp(void);
    JIM_CEXPORT void Jim_FreeInterp(Jim_InterpPtr i);
    JIM_CEXPORT int Jim_GetExitCode(Jim_InterpPtr interp);
    JIM_CEXPORT const char* Jim_ReturnCode(int code);
    JIM_CEXPORT void Jim_SetResultFormatted(Jim_InterpPtr interp, const char* format, ...);
    JIM_CEXPORT Jim_CallFrame* Jim_TopCallFrame(Jim_InterpPtr  interp);
    JIM_CEXPORT Jim_ObjPtr  Jim_CurrentNamespace(Jim_InterpPtr  interp);
    JIM_CEXPORT Jim_ObjPtr  Jim_EmptyObj(Jim_InterpPtr  interp);
    JIM_CEXPORT int Jim_CurrentLevel(Jim_InterpPtr  interp);
    JIM_CEXPORT Jim_HashTablePtr  Jim_PackagesHT(Jim_InterpPtr  interp);
    JIM_CEXPORT void Jim_IncrStackTrace(Jim_InterpPtr  interp);

    /* commands */
    JIM_CEXPORT void Jim_RegisterCoreCommands(Jim_InterpPtr interp);
    JIM_CEXPORT Retval Jim_CreateCommand(Jim_InterpPtr interp,
                                        const char* cmdName, Jim_CmdProc* cmdProc, void* privData,
                                        Jim_DelCmdProc* delProc);
    JIM_CEXPORT Retval Jim_DeleteCommand(Jim_InterpPtr interp,
                                        const char* cmdName);
    JIM_CEXPORT Retval Jim_RenameCommand(Jim_InterpPtr interp,
                                        const char* oldName, const char* newName);
    JIM_CEXPORT Jim_Cmd* Jim_GetCommand(Jim_InterpPtr interp,
                                       Jim_ObjPtr  objPtr, int flags);
    JIM_CEXPORT Retval Jim_SetVariable(Jim_InterpPtr interp,
                                      Jim_ObjPtr  nameObjPtr, Jim_ObjPtr  valObjPtr);
    JIM_CEXPORT Retval Jim_SetVariableStr(Jim_InterpPtr interp,
                                         const char* name, Jim_ObjPtr  objPtr);
    JIM_CEXPORT Retval Jim_SetGlobalVariableStr(Jim_InterpPtr interp,
                                               const char* name, Jim_ObjPtr  objPtr);
    JIM_CEXPORT Retval Jim_SetVariableStrWithStr(Jim_InterpPtr interp,
                                                const char* name, const char* val);
    JIM_CEXPORT Retval Jim_SetVariableLink(Jim_InterpPtr interp,
                                          Jim_ObjPtr  nameObjPtr, Jim_ObjPtr  targetNameObjPtr,
                                          Jim_CallFrame* targetCallFrame);
    JIM_CEXPORT Jim_ObjPtr  Jim_MakeGlobalNamespaceName(Jim_InterpPtr interp,
                                                    Jim_ObjPtr  nameObjPtr);
    JIM_CEXPORT Jim_ObjPtr  Jim_GetVariable(Jim_InterpPtr interp,
                                        Jim_ObjPtr  nameObjPtr, int flags);
    JIM_CEXPORT Jim_ObjPtr  Jim_GetGlobalVariable(Jim_InterpPtr interp,
                                              Jim_ObjPtr  nameObjPtr, int flags);
    JIM_CEXPORT Jim_ObjPtr  Jim_GetVariableStr(Jim_InterpPtr interp,
                                           const char* name, int flags);
    JIM_CEXPORT Jim_ObjPtr  Jim_GetGlobalVariableStr(Jim_InterpPtr interp,
                                                 const char* name, int flags);
    JIM_CEXPORT Retval Jim_UnsetVariable(Jim_InterpPtr interp,
                                        Jim_ObjPtr  nameObjPtr, int flags);

    /* call frame */
    JIM_CEXPORT Jim_CallFrame* Jim_GetCallFrameByLevel(Jim_InterpPtr interp,
                                                      Jim_ObjPtr  levelObjPtr);

    /* garbage collection */
    JIM_CEXPORT int Jim_Collect(Jim_InterpPtr interp);
    JIM_CEXPORT void Jim_CollectIfNeeded(Jim_InterpPtr interp);

    /* index object */
    JIM_CEXPORT Retval Jim_GetIndex(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                   int* indexPtr /* on error set INT_MAX/-INT_MAX */);

    /* list object */
    JIM_CEXPORT Jim_ObjPtr  Jim_NewListObj(Jim_InterpPtr interp,
                                       Jim_ObjConstArray elements, int len);
    JIM_CEXPORT void Jim_ListInsertElements(Jim_InterpPtr interp,
                                           Jim_ObjPtr  listPtr, int listindex, int objc,
                                           Jim_ObjConstArray objVec);
    JIM_CEXPORT void Jim_ListAppendElement(Jim_InterpPtr interp,
                                          Jim_ObjPtr  listPtr, Jim_ObjPtr  objPtr);
    JIM_CEXPORT void Jim_ListAppendList(Jim_InterpPtr interp,
                                       Jim_ObjPtr  listPtr, Jim_ObjPtr  appendListPtr);
    JIM_CEXPORT int Jim_ListLength(Jim_InterpPtr interp, Jim_ObjPtr  objPtr);
    JIM_CEXPORT Retval Jim_ListIndex(Jim_InterpPtr interp, Jim_ObjPtr  listPrt,
                                    int listindex, Jim_ObjArray* objPtrPtr, int seterr);
    JIM_CEXPORT Jim_ObjPtr  Jim_ListGetIndex(Jim_InterpPtr interp, Jim_ObjPtr  listPtr, int idx);
    JIM_CEXPORT int Jim_SetListIndex(Jim_InterpPtr interp,
                                    Jim_ObjPtr  varNamePtr, Jim_ObjConstArray indexv, int indexc,
                                    Jim_ObjPtr  newObjPtr);
    JIM_CEXPORT Jim_ObjPtr  Jim_ConcatObj(Jim_InterpPtr interp, int objc,
                                      Jim_ObjConstArray objv);
    JIM_CEXPORT Jim_ObjPtr  Jim_ListJoin(Jim_InterpPtr interp,
                                     Jim_ObjPtr  listObjPtr, const char* joinStr, int joinStrLen);

    /* dict object */
    JIM_CEXPORT Jim_ObjPtr  Jim_NewDictObj(Jim_InterpPtr interp,
                                       Jim_ObjConstArray elements, int len);
    JIM_CEXPORT Retval Jim_DictKey(Jim_InterpPtr interp, Jim_ObjPtr  dictPtr,
                                  Jim_ObjPtr  keyPtr, Jim_ObjArray* objPtrPtr, int flags);
    JIM_CEXPORT Retval Jim_DictKeysVector(Jim_InterpPtr interp,
                                         Jim_ObjPtr  dictPtr, Jim_ObjConstArray keyv, int keyc,
                                         Jim_ObjArray* objPtrPtr, int flags);
    JIM_CEXPORT Retval Jim_SetDictKeysVector(Jim_InterpPtr interp,
                                            Jim_ObjPtr  varNamePtr, Jim_ObjConstArray keyv, int keyc,
                                            Jim_ObjPtr  newObjPtr, int flags);
    JIM_CEXPORT Retval Jim_DictPairs(Jim_InterpPtr interp,
                                    Jim_ObjPtr  dictPtr, Jim_ObjArray** objPtrPtr, int* len);
    JIM_CEXPORT Retval Jim_DictAddElement(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                         Jim_ObjPtr  keyObjPtr, Jim_ObjPtr  valueObjPtr);

    enum JIM_DICTMATCH {
        JIM_DICTMATCH_KEYS = 0x0001,
        JIM_DICTMATCH_VALUES = 0x002
    };

    JIM_CEXPORT Retval Jim_DictMatchTypes(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjPtr  patternObj,
                                         int match_type, int return_types);
    JIM_CEXPORT int Jim_DictSize(Jim_InterpPtr interp, Jim_ObjPtr  objPtr);
    JIM_CEXPORT Retval Jim_DictInfo(Jim_InterpPtr interp, Jim_ObjPtr  objPtr);
    JIM_CEXPORT Jim_ObjPtr  Jim_DictMerge(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv);

    /* return code object */
    JIM_CEXPORT Retval Jim_GetReturnCode(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                        int* intPtr);

    /* expression object */
    JIM_CEXPORT Retval Jim_EvalExpression(Jim_InterpPtr interp,
                                         Jim_ObjPtr  exprObjPtr);
    JIM_CEXPORT Retval Jim_GetBoolFromExpr(Jim_InterpPtr interp,
                                          Jim_ObjPtr  exprObjPtr, int* boolPtr);

    /* boolean object */
    JIM_CEXPORT Retval Jim_GetBoolean(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                     int* booleanPtr);

    /* integer object */
    JIM_CEXPORT Retval Jim_GetWide(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  jim_wide* widePtr);
    JIM_CEXPORT Retval Jim_GetLong(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  long* longPtr);

    JIM_CEXPORT Jim_ObjPtr  Jim_NewIntObj(Jim_InterpPtr interp,
                                      jim_wide wideValue);

    /* double object */
    JIM_CEXPORT Retval Jim_GetDouble(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                    double* doublePtr);
    JIM_CEXPORT void Jim_SetDouble(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  double doubleValue);
    JIM_CEXPORT Jim_ObjPtr  Jim_NewDoubleObj(Jim_InterpPtr interp, double doubleValue);

    /* commands utilities */
    JIM_CEXPORT void Jim_WrongNumArgs(Jim_InterpPtr interp, int argc,
                                     Jim_ObjConstArray argv, const char* msg);
    JIM_CEXPORT Retval Jim_GetEnum(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  const char* const* tablePtr, int* indexPtr, const char* name, int flags);
    JIM_CEXPORT Retval Jim_CheckShowCommands(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                            const char* const* tablePtr);
    JIM_CEXPORT int Jim_ScriptIsComplete(Jim_InterpPtr interp,
                                        Jim_ObjPtr  scriptObj, char* stateCharPtr);

    /**
     * Find a matching name in the array of the given length.
     *
     * NULL entries are ignored.
     *
     * Returns the matching index if found, or -1 if not.
     */
    JIM_CEXPORT int Jim_FindByName(const char* name, const char* const array[], size_t len);

    /* package utilities */
    typedef void (Jim_InterpDeleteProc)(Jim_InterpPtr interp, void* data);
    JIM_CEXPORT void* Jim_GetAssocData(Jim_InterpPtr interp, const char* key);
    JIM_CEXPORT Retval Jim_SetAssocData(Jim_InterpPtr interp, const char* key,
                                       Jim_InterpDeleteProc* delProc, void* data);
    JIM_CEXPORT Retval Jim_DeleteAssocData(Jim_InterpPtr interp, const char* key);

    /* Packages C API */
    /* jim-package.c */
    JIM_CEXPORT Retval Jim_PackageProvide(Jim_InterpPtr interp,
                                         const char* name, const char* ver, int flags);
    JIM_CEXPORT Retval Jim_PackageRequire(Jim_InterpPtr interp,
                                         const char* name, int flags);

    /* error messages */
    JIM_CEXPORT void Jim_MakeErrorMessage(Jim_InterpPtr interp);

    /* interactive mode */
    JIM_CEXPORT Retval Jim_InteractivePrompt(Jim_InterpPtr interp);
    JIM_CEXPORT void Jim_HistoryLoad(const char* filename);
    JIM_CEXPORT void Jim_HistorySave(const char* filename);
    JIM_CEXPORT char* Jim_HistoryGetline(Jim_InterpPtr interp, const char* prompt);
    JIM_CEXPORT void Jim_HistorySetCompletion(Jim_InterpPtr interp, Jim_ObjPtr  commandObj);
    JIM_CEXPORT void Jim_HistoryAdd(const char* line);
    JIM_CEXPORT void Jim_HistoryShow(void);

    /* Misc */
    JIM_CEXPORT Retval Jim_InitStaticExtensions(Jim_InterpPtr interp);
    JIM_CEXPORT Retval Jim_StringToWide(const char* str, jim_wide* widePtr, int base);
    JIM_CEXPORT int Jim_IsBigEndian(void);

    /**
     * Returns 1 if a signal has been received while
     * in a catch -signal {} clause.
     */
    JIM_CAPI_INLINE long_long Jim_CheckSignal(Jim_InterpPtr  i);

    /* jim-load.c */
    JIM_CEXPORT Retval Jim_LoadLibrary(Jim_InterpPtr interp, const char* pathName);
    JIM_CEXPORT void Jim_FreeLoadHandles(Jim_InterpPtr interp);

    /* jim-aio.c */
    JIM_CEXPORT FILE* Jim_AioFilehandle(Jim_InterpPtr interp, Jim_ObjPtr  command);

    /* type inspection - avoid where possible */
    JIM_CEXPORT int Jim_IsDict(Jim_ObjPtr  objPtr);
    JIM_CEXPORT int Jim_IsList(Jim_ObjPtr  objPtr);

    JIM_CAPI_INLINE void Jim_SetResult(Jim_InterpPtr  i, Jim_ObjPtr  o);
    JIM_CAPI_INLINE void Jim_InterpIncrProcEpoch(Jim_InterpPtr  i);
    JIM_CAPI_INLINE void Jim_SetResultString(Jim_InterpPtr  i, const char* s, int l /* -1 means strlen(s) */);
    JIM_CAPI_INLINE void Jim_SetResultInt(Jim_InterpPtr  i, long_long intval);
    JIM_CAPI_INLINE void Jim_SetResultBool(Jim_InterpPtr  i, long_long b);
    JIM_CAPI_INLINE void Jim_SetEmptyResult(Jim_InterpPtr  i);
    JIM_CAPI_INLINE Jim_ObjPtr  Jim_GetResult(Jim_InterpPtr  i);
    JIM_CAPI_INLINE void* Jim_CmdPrivData(Jim_InterpPtr  i);

    JIM_CAPI_INLINE Jim_ObjPtr  Jim_NewEmptyStringObj(Jim_InterpPtr  i);
    JIM_CAPI_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter);

    //inline Retval Jim_EvalPrefix(Jim_InterpPtr  i, const char* p, int oc, Jim_ObjConstArray  ov) {
    //    return Jim_EvalObjPrefix((i), Jim_NewStringObj((i), (p), -1), (oc), (ov));
    //}

    /* from jimiocompat.cpp */
    /**
     * Set an error result based on errno and the given message.
     */
    void Jim_SetResultErrno(Jim_InterpPtr interp, const char* msg);

    /**
     * Opens the file for writing (and appending if append is true).
     * Returns the file descriptor, or -1 on failure.
     */
    int Jim_OpenForWrite(const char* filename, int append);

    /**
     * Opens the file for reading.
     * Returns the file descriptor, or -1 on failure.
     */
    int Jim_OpenForRead(const char* filename);

    /**
     * Unix-compatible errno
     */
    int Jim_Errno(void);



#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
