#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef HAVE_NO_AUTOCONF // #optionalCode
#include <jim-config.h>
#endif

#include <jim-base.h>
#include <returns.h>
#include <prj_trace.h>

BEGIN_JIM_NAMESPACE

/* -----------------------------------------------------------------------------
 * System configuration
 * autoconf (configure) will set these
 * ---------------------------------------------------------------------------*/

 /* -----------------------------------------------------------------------------
  * Compiler specific fixes.
  * ---------------------------------------------------------------------------*/

/* Long Long tokenType_ and related issues */
typedef int64_t jim_wide;
#ifndef JIM_WIDE_MODIFIER
#  define JIM_WIDE_MODIFIER "lld"
#  define JIM_WIDE_8BYTE
#endif

 /* -----------------------------------------------------------------------------
  * Exported defines
  * ---------------------------------------------------------------------------*/
enum JIM_INTERP_FLAG_FLAGS {
    JIM_NONE = 0,           /* no flags_ set */
    JIM_ERRMSG = 1,         /* set an errorText_ message in the interpreter. */
    JIM_ENUM_ABBREV = 2,    /* Jim_GetEnum() - Allow unambiguous abbreviation */
    JIM_UNSHARED = 4,       /* Jim_GetVariable() - return unshared object */
    JIM_MUSTEXIST = 8       /* Jim_SetDictKeysVector() - fail if non-existent */
};

#define JRET(X) X

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
enum FILESYSTEM_PARAM {
    JIM_PATH_LEN = 1024 // #MagicNum
};

#define JIM_LIBPATH "auto_path" // #MagicStr

/* -----------------------------------------------------------------------------
 * Forwards
 * ---------------------------------------------------------------------------*/
#include <jim-forwards.h>

 // For now I am not building shared libraries so this is disabled.
#define JIM_EXPORT // #disabled_option #optionalCode
#ifndef JIM_EXPORT
#  ifdef PRJ_OS_WIN
 // We don't handle DLL and EXE differently.  Little know fact is you can call functions off EXE's.
#    define JIM_EXPORT __declspec(dllexport)
#  endif
#endif

#ifndef JIM_API_INLINE // #optionalCode
#  ifdef JIM_INLINE_API_SMALLFUNCS
#    define JIM_API_INLINE JIM_EXPORT inline
#  else
#    define JIM_API_INLINE JIM_EXPORT
#  endif
#endif


/* -----------------------------------------------------------------------------
 * Jim interpreter structure.
 * Fields similar to the real Tcl interpreter structure have the same names.
 * ---------------------------------------------------------------------------*/
typedef int Jim_CmdProc(Jim_InterpPtr interp, int argc,
                        Jim_ObjConstArray argv);
typedef void Jim_DelCmdProc(Jim_InterpPtr interp, void *privData);

JIM_API_INLINE long Jim_GetId(Jim_InterpPtr  i);

/* -----------------------------------------------------------------------------
 * Exported API prototypes.
 * ---------------------------------------------------------------------------*/
JIM_EXPORT void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr); // #dtor_like
JIM_API_INLINE void Jim_IncrRefCount(Jim_ObjPtr  objPtr);
JIM_API_INLINE void Jim_DecrRefCount(Jim_InterpPtr  interp, Jim_ObjPtr  objPtr);
CHKRET JIM_EXPORT int  Jim_RefCount(Jim_ObjPtr  objPtr);
CHKRET JIM_API_INLINE int Jim_IsShared(Jim_ObjPtr  objPtr);

#include <jim-alloc.h>

/* environment */
CHKRET JIM_EXPORT char **Jim_GetEnviron(void);
JIM_EXPORT void Jim_SetEnviron(char **env);
CHKRET JIM_EXPORT int Jim_MakeTempFile(Jim_InterpPtr interp,
                                const char *filename_template, int unlink_file /*bool*/);
//
/* evaluation */
CHKRET JIM_EXPORT Retval Jim_Eval(Jim_InterpPtr interp, const char *script);
/* in C code, you can do this and get better errorText_ messages */
/*   Jim_EvalSource( interp_, __FILE__, __LINE__ , "some tcl commands"); */
CHKRET JIM_EXPORT Retval Jim_EvalSource(Jim_InterpPtr interp, const char *filename,
                              int lineno, const char *script);
/* Backwards compatibility */
CHKRET inline Retval Jim_Eval_Named(Jim_InterpPtr  I, const char* S, const char* F, int L) {
    return Jim_EvalSource(I, F, L, S); 
}

CHKRET JIM_EXPORT Retval Jim_EvalGlobal(Jim_InterpPtr interp, const char *script);
CHKRET JIM_EXPORT Retval Jim_EvalFile(Jim_InterpPtr interp, const char *filename);
CHKRET JIM_EXPORT Retval Jim_EvalFileGlobal(Jim_InterpPtr interp, const char *filename);
CHKRET JIM_EXPORT Retval Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr);
CHKRET JIM_EXPORT Retval Jim_EvalObjVector(Jim_InterpPtr interp, int objc,
                                 Jim_ObjConstArray objv);
CHKRET JIM_EXPORT Retval Jim_EvalObjList(Jim_InterpPtr interp, Jim_ObjPtr listObj);
CHKRET JIM_EXPORT Retval Jim_EvalObjPrefix(Jim_InterpPtr interp, Jim_ObjPtr prefix,
                                 int objc, Jim_ObjConstArray objv);
CHKRET JIM_EXPORT inline Retval Jim_EvalPrefix_(Jim_InterpPtr  i, const char* p, int oc, Jim_ObjConstArray  ov);
CHKRET JIM_EXPORT Retval Jim_EvalNamespace(Jim_InterpPtr interp, Jim_ObjPtr scriptObj, Jim_ObjPtr nsObj);
CHKRET JIM_EXPORT Retval Jim_SubstObj(Jim_InterpPtr interp, Jim_ObjPtr substObjPtr,
                            Jim_ObjArray* resObjPtrPtr, int flags);

/* stack_ */
CHKRET JIM_EXPORT Jim_StackPtr  Jim_AllocStack(void);
JIM_EXPORT void Jim_InitStack(Jim_StackPtr stack); // #ctor_like Jim_StackPtr
JIM_EXPORT void Jim_FreeStack(Jim_StackPtr stack); // #dtor_like Jim_StackPtr
CHKRET JIM_EXPORT int Jim_StackLen(Jim_StackPtr stack);
JIM_EXPORT void Jim_StackPush(Jim_StackPtr stack, void *element);
CHKRET JIM_EXPORT void * Jim_StackPop(Jim_StackPtr stack);
CHKRET JIM_EXPORT void * Jim_StackPeek(Jim_StackPtr stack);
JIM_EXPORT void Jim_FreeStackElements(Jim_StackPtr stack, void(*freeFunc)(void *ptr));

/* hash table */
CHKRET JIM_EXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht, // #ctor_like Jim_HashTable
                                 const Jim_HashTableType *type, void *privdata);
JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht,
                                    unsigned_int size);
CHKRET JIM_EXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void *key,
                                void *val);
CHKRET JIM_EXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht,
                                    const void *key, void *val);
CHKRET JIM_EXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht, // #dtor_like Jim_HashTable
                                   const void *key);
CHKRET JIM_EXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht); // #dtor_like Jim_HashTable
CHKRET JIM_EXPORT Jim_HashEntryPtr  Jim_FindHashEntry(Jim_HashTablePtr ht,
                                             const void *key);
JIM_EXPORT void Jim_ResizeHashTable(Jim_HashTablePtr ht);
CHKRET JIM_EXPORT Jim_HashTableIterator *Jim_GetHashTableIterator(Jim_HashTablePtr ht);
CHKRET JIM_EXPORT Jim_HashEntryPtr  Jim_NextHashEntry(Jim_HashTableIterator *iter);
CHKRET JIM_EXPORT const char* Jim_KeyAsStr(Jim_HashEntryPtr  he);
CHKRET JIM_EXPORT const void* Jim_KeyAsVoid(Jim_HashEntryPtr  he);

/* objects */
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewObj(Jim_InterpPtr interp); // #ctor_like Jim_Obj
JIM_EXPORT void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr); // #dtor_like Jim_Obj
JIM_EXPORT void Jim_InvalidateStringRep(Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_DuplicateObj(Jim_InterpPtr interp, // #copy_ctor_like
                                      Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT const char * Jim_GetString(Jim_ObjPtr objPtr,
                                      int *lenPtr);
CHKRET JIM_EXPORT const char *Jim_String(Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT int Jim_Length(Jim_ObjPtr objPtr);

/* string object */
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewStringObj(Jim_InterpPtr interp, // #ctor_like Jim_Obj
                                      const char *s, int len /* -1 means strlen(s) */);
CHKRET JIM_EXPORT Jim_ObjPtr Jim_NewStringObjUtf8(Jim_InterpPtr interp, // #ctor_like Jim_Obj
                                         const char *s, int charlen /* num chars */);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewStringObjNoAlloc(Jim_InterpPtr interp, // #ctor_like Jim_Obj
                                             char *s, int len /* -1 means strlen(s) */);
JIM_EXPORT void Jim_AppendString(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                 const char *str, int len /* -1 means strlen(s) */);
JIM_EXPORT void Jim_AppendObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                              Jim_ObjPtr appendObjPtr);
JIM_EXPORT void Jim_AppendStrings(Jim_InterpPtr interp,
                                  Jim_ObjPtr objPtr, ...);
CHKRET JIM_EXPORT int Jim_StringEqObj(Jim_ObjPtr aObjPtr, Jim_ObjPtr bObjPtr);
CHKRET JIM_EXPORT int Jim_StringMatchObj(Jim_InterpPtr interp, Jim_ObjPtr patternObjPtr,
                                  Jim_ObjPtr objPtr, int nocase /*bool*/);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_StringRangeObj(Jim_InterpPtr interp,
                                        Jim_ObjPtr strObjPtr, Jim_ObjPtr firstObjPtr,
                                        Jim_ObjPtr lastObjPtr);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_FormatString(Jim_InterpPtr interp,
                                      Jim_ObjPtr fmtObjPtr, int objc, Jim_ObjConstArray objv);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_ScanString(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr,
                                    Jim_ObjPtr fmtObjPtr, int flags);
CHKRET JIM_EXPORT int Jim_CompareStringImmediate(Jim_InterpPtr interp,
                                          Jim_ObjPtr objPtr, const char *str);
CHKRET JIM_EXPORT int Jim_StringCompareObj(Jim_InterpPtr interp, Jim_ObjPtr firstObjPtr,
                                    Jim_ObjPtr secondObjPtr, int nocase /*bool*/);
CHKRET JIM_EXPORT int Jim_StringCompareLenObj(Jim_InterpPtr interp, Jim_ObjPtr firstObjPtr,
                                       Jim_ObjPtr secondObjPtr, int nocase /*bool*/);
CHKRET JIM_EXPORT int Jim_Utf8Length(Jim_InterpPtr interp, Jim_ObjPtr objPtr);

/* reference object */
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewReference(Jim_InterpPtr interp, 
                                      Jim_ObjPtr objPtr, Jim_ObjPtr tagPtr, Jim_ObjPtr cmdNamePtr);
CHKRET JIM_EXPORT Jim_ReferencePtr  Jim_GetReference(Jim_InterpPtr interp,
                                            Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Retval Jim_SetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr cmdNamePtr);
CHKRET JIM_EXPORT Retval Jim_GetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjArray* cmdNamePtrPtr);

/* interpreter */
CHKRET JIM_EXPORT Jim_InterpPtr  Jim_CreateInterp(void); // #ctor_like Jim_Interp
JIM_EXPORT void Jim_FreeInterp(Jim_InterpPtr i); // #dtor_like Jim_Interp
CHKRET JIM_EXPORT int Jim_GetExitCode(Jim_InterpPtr interp);
CHKRET JIM_EXPORT const char *Jim_ReturnCode(int code);
JIM_EXPORT void Jim_SetResultFormatted(Jim_InterpPtr interp, const char *format, ...);
CHKRET JIM_EXPORT Jim_CallFramePtr  Jim_TopCallFrame(Jim_InterpPtr  interp);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_CurrentNamespace(Jim_InterpPtr  interp);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_EmptyObj(Jim_InterpPtr  interp);
CHKRET JIM_EXPORT int Jim_CurrentLevel(Jim_InterpPtr  interp);
CHKRET JIM_EXPORT Jim_HashTablePtr  Jim_PackagesHT(Jim_InterpPtr  interp);
JIM_EXPORT void Jim_IncrStackTrace(Jim_InterpPtr  interp);

/* commands */
JIM_EXPORT void Jim_RegisterCoreCommands(Jim_InterpPtr interp);
CHKRET JIM_EXPORT Retval Jim_CreateCommand(Jim_InterpPtr interp, // #ctor_like
                                 const char *cmdName, Jim_CmdProc *cmdProc, void *privData,
                                 Jim_DelCmdProc *delProc);
CHKRET JIM_EXPORT Retval Jim_DeleteCommand(Jim_InterpPtr interp, // #dtor_like
                                 const char *cmdName);
CHKRET JIM_EXPORT Retval Jim_RenameCommand(Jim_InterpPtr interp,
                                 const char *oldName, const char *newName);
CHKRET JIM_EXPORT Jim_CmdPtr  Jim_GetCommand(Jim_InterpPtr interp,
                                    Jim_ObjPtr objPtr, int flags);
CHKRET JIM_EXPORT Retval Jim_SetVariable(Jim_InterpPtr interp,
                               Jim_ObjPtr nameObjPtr, Jim_ObjPtr valObjPtr);
CHKRET JIM_EXPORT Retval Jim_SetVariableStr(Jim_InterpPtr interp,
                                  const char *name, Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Retval Jim_SetGlobalVariableStr(Jim_InterpPtr interp,
                                        const char *name, Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Retval Jim_SetVariableStrWithStr(Jim_InterpPtr interp,
                                         const char *name, const char *val);
CHKRET JIM_EXPORT Retval Jim_SetVariableLink(Jim_InterpPtr interp,
                                   Jim_ObjPtr nameObjPtr, Jim_ObjPtr targetNameObjPtr,
                                   Jim_CallFramePtr targetCallFrame);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_MakeGlobalNamespaceName(Jim_InterpPtr interp,
                                                 Jim_ObjPtr nameObjPtr);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_GetVariable(Jim_InterpPtr interp,
                                     Jim_ObjPtr nameObjPtr, int flags);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_GetGlobalVariable(Jim_InterpPtr interp,
                                           Jim_ObjPtr nameObjPtr, int flags);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_GetVariableStr(Jim_InterpPtr interp,
                                        const char *name, int flags);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_GetGlobalVariableStr(Jim_InterpPtr interp,
                                              const char *name, int flags);
CHKRET JIM_EXPORT Retval Jim_UnsetVariable(Jim_InterpPtr interp,
                                 Jim_ObjPtr nameObjPtr, int flags);
// Don't care if the variable actually exists.
JIM_EXPORT void Jim_UnsetVariableIgnoreErr(Jim_InterpPtr interp,
                                           Jim_ObjPtr nameObjPtr, int flags);
/* call frame */
CHKRET JIM_EXPORT Jim_CallFramePtr Jim_GetCallFrameByLevel(Jim_InterpPtr interp,
                                                  Jim_ObjPtr levelObjPtr);

/* garbage collection */
CHKRET JIM_EXPORT int Jim_Collect(Jim_InterpPtr interp);
JIM_EXPORT void Jim_CollectIfNeeded(Jim_InterpPtr interp);

/* index object */
CHKRET JIM_EXPORT Retval Jim_GetIndex(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                            int *indexPtr /* on errorText_ set INT_MAX/-INT_MAX */);

/* list object */
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewListObj(Jim_InterpPtr interp, // #ctor_like Jim_Obj
                                    Jim_ObjConstArray elements, int len);
JIM_EXPORT void Jim_ListInsertElements(Jim_InterpPtr interp,
                                       Jim_ObjPtr listPtr, int listindex, int objc, 
                                       Jim_ObjConstArray objVec);
JIM_EXPORT void Jim_ListAppendElement(Jim_InterpPtr interp,
                                      Jim_ObjPtr listPtr, Jim_ObjPtr objPtr);
JIM_EXPORT void Jim_ListAppendList(Jim_InterpPtr interp,
                                   Jim_ObjPtr listPtr, Jim_ObjPtr appendListPtr);
JIM_EXPORT int Jim_ListLength(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Retval Jim_ListIndex(Jim_InterpPtr interp, Jim_ObjPtr listPrt,
                             int listindex, Jim_ObjArray* objPtrPtr, int seterr);
CHKRET JIM_EXPORT Jim_ObjPtr Jim_ListGetIndex(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int idx);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_ConcatObj(Jim_InterpPtr interp, int objc,
                                   Jim_ObjConstArray objv);
CHKRET JIM_EXPORT Jim_ObjPtr Jim_ListJoin(Jim_InterpPtr interp,
                                 Jim_ObjPtr listObjPtr, const char *joinStr, int joinStrLen);

/* dict object */
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewDictObj(Jim_InterpPtr interp, // #ctor_like Jim_Obj
                                    Jim_ObjConstArray elements, int len);
CHKRET JIM_EXPORT Retval Jim_DictKey(Jim_InterpPtr interp, Jim_ObjPtr dictPtr,
                           Jim_ObjPtr keyPtr, Jim_ObjArray* objPtrPtr, int flags);
CHKRET JIM_EXPORT Retval Jim_DictKeysVector(Jim_InterpPtr interp,
                                  Jim_ObjPtr dictPtr, Jim_ObjConstArray keyv, int keyc,
                                  Jim_ObjArray* objPtrPtr, int flags);
CHKRET JIM_EXPORT Retval Jim_SetDictKeysVector (Jim_InterpPtr interp,
                                     Jim_ObjPtr varNamePtr, Jim_ObjConstArray keyv, int keyc,
                                     Jim_ObjPtr newObjPtr, int flags);
CHKRET JIM_EXPORT Retval Jim_DictPairs(Jim_InterpPtr interp,
                             Jim_ObjPtr dictPtr, Jim_ObjArray* *objPtrPtr, int *len);
CHKRET JIM_EXPORT Retval Jim_DictAddElement(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                  Jim_ObjPtr keyObjPtr, Jim_ObjPtr valueObjPtr);

enum JIM_DICTMATCH {
    JIM_DICTMATCH_KEYS = 0x0001,
    JIM_DICTMATCH_VALUES = 0x002
};

CHKRET JIM_EXPORT Retval Jim_DictMatchTypes(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr patternObj,
                                  int match_type, int return_types);
CHKRET JIM_EXPORT int Jim_DictSize(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Retval Jim_DictInfo(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT Jim_ObjPtr Jim_DictMerge(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv);

/* return code object */
CHKRET JIM_EXPORT Retval Jim_GetReturnCode(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                 int *intPtr);

/* expression object */
CHKRET JIM_EXPORT Retval Jim_EvalExpression(Jim_InterpPtr interp,
                                  Jim_ObjPtr exprObjPtr);
CHKRET JIM_EXPORT Retval Jim_GetBoolFromExpr(Jim_InterpPtr interp,
                                   Jim_ObjPtr exprObjPtr, int *boolPtr);

/* boolean object */
CHKRET JIM_EXPORT Retval Jim_GetBoolean(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                              int *booleanPtr);

/* integer object */
CHKRET JIM_EXPORT Retval Jim_GetWide (Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                           jim_wide *widePtr);
CHKRET JIM_EXPORT Retval Jim_GetLong(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                           long *longPtr);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewIntObj(Jim_InterpPtr interp, // #ctor_like Jim_Obj
                                   jim_wide wideValue);

/* double object */
CHKRET JIM_EXPORT Retval Jim_GetDouble(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                             double *doublePtr);
JIM_EXPORT void Jim_SetDouble(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                              double doubleValue);
CHKRET JIM_EXPORT Jim_ObjPtr  Jim_NewDoubleObj(Jim_InterpPtr interp, double doubleValue); // #ctor_like Jim_Obj

/* commands utilities */
JIM_EXPORT void Jim_WrongNumArgs(Jim_InterpPtr interp, int argc,
                                 Jim_ObjConstArray argv, const char *msg);
CHKRET JIM_EXPORT Retval Jim_GetEnum(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                           const char * const *tablePtr, int *indexPtr, const char *name, int flags);
CHKRET JIM_EXPORT Retval Jim_CheckShowCommands(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                     const char *const *tablePtr);
CHKRET JIM_EXPORT int Jim_ScriptIsComplete(Jim_InterpPtr interp,
                                    Jim_ObjPtr scriptObj, char *stateCharPtr);

/**
 * Find a matching name_ in the array of the given length.
 *
 * NULL entries are ignored.
 *
 * Returns the matching index if found, or -1 if not.
 */
CHKRET JIM_EXPORT int Jim_FindByName(const char *name, const char * const array[], size_t len);

/* package utilities */
typedef void (Jim_InterpDeleteProc)(Jim_InterpPtr interp, void *data);
CHKRET JIM_EXPORT void * Jim_GetAssocData(Jim_InterpPtr interp, const char *key);
CHKRET JIM_EXPORT Retval Jim_SetAssocData(Jim_InterpPtr interp, const char *key, // #ctor_like
                                Jim_InterpDeleteProc *delProc, void *data);
CHKRET JIM_EXPORT Retval Jim_DeleteAssocData(Jim_InterpPtr interp, const char *key); // #dtor_like

/* Packages C API */
/* jim-package.c */
CHKRET JIM_EXPORT Retval Jim_PackageProvide(Jim_InterpPtr interp,
                                  const char *name, const char *ver, int flags);
CHKRET JIM_EXPORT Retval Jim_PackageRequire(Jim_InterpPtr interp,
                                  const char *name, int flags);

/* errorText_ messages */
JIM_EXPORT void Jim_MakeErrorMessage(Jim_InterpPtr interp);

/* interactive mode */
CHKRET JIM_EXPORT Retval Jim_InteractivePrompt(Jim_InterpPtr interp);
JIM_EXPORT void Jim_HistoryLoad(const char *filename);
JIM_EXPORT void Jim_HistorySave(const char *filename);
CHKRET JIM_EXPORT char *Jim_HistoryGetline(Jim_InterpPtr interp, const char *prompt);
JIM_EXPORT void Jim_HistorySetCompletion(Jim_InterpPtr interp, Jim_ObjPtr commandObj);
JIM_EXPORT void Jim_HistoryAdd(const char *line);
JIM_EXPORT void Jim_HistoryShow(void);

/* Misc */
CHKRET Retval Jim_InitStaticExtensions(Jim_InterpPtr interp);
CHKRET JIM_EXPORT Retval Jim_StringToWide(const char *str, jim_wide *widePtr, int base);
CHKRET JIM_EXPORT int Jim_IsBigEndian(void);

/**
 * Returns 1 if a signal has been received while
 * in a catch -signal {} clause.
 */
CHKRET JIM_API_INLINE long_long Jim_CheckSignal(Jim_InterpPtr  i);

/* jim-load.c */
CHKRET JIM_EXPORT Retval Jim_LoadLibrary(Jim_InterpPtr interp, const char *pathName);
JIM_EXPORT void Jim_FreeLoadHandles(Jim_InterpPtr interp);

/* jim-aio.c */
CHKRET JIM_EXPORT FILE *Jim_AioFilehandle(Jim_InterpPtr interp, Jim_ObjPtr command);

/* tokenType_ inspection - avoid where possible */
CHKRET JIM_EXPORT int Jim_IsDict(Jim_ObjPtr objPtr);
CHKRET JIM_EXPORT int Jim_IsList(Jim_ObjPtr objPtr);

JIM_API_INLINE void Jim_SetResult(Jim_InterpPtr  i, Jim_ObjPtr  o);
JIM_API_INLINE void Jim_InterpIncrProcEpoch(Jim_InterpPtr  i);
JIM_API_INLINE void Jim_SetResultString(Jim_InterpPtr  i, const char* s, int l /* -1 means strlen(s) */);
JIM_API_INLINE void Jim_SetResultInt(Jim_InterpPtr  i, long_long intval);
JIM_API_INLINE void Jim_SetResultBool(Jim_InterpPtr  i, long_long b);
JIM_API_INLINE void Jim_SetEmptyResult(Jim_InterpPtr  i);
CHKRET JIM_API_INLINE Jim_ObjPtr  Jim_GetResult(Jim_InterpPtr  i);
JIM_API_INLINE void* Jim_CmdPrivData(Jim_InterpPtr  i);

CHKRET JIM_API_INLINE Jim_ObjPtr  Jim_NewEmptyStringObj(Jim_InterpPtr  i);
JIM_API_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter);

CHKRET JIM_EXPORT inline Retval Jim_EvalPrefix(Jim_InterpPtr  i, const char* p, int oc, Jim_ObjConstArray  ov) {
    return Jim_EvalObjPrefix((i), Jim_NewStringObj((i), (p), -1), (oc), (ov));
}

/* from jimiocompat.cpp */
/**
 * Set an errorText_ result based on errno and the given message.
 */
JIM_EXPORT void Jim_SetResultErrno(Jim_InterpPtr interp, const char* msg);

/**
 * Opens the file for writing (and appending if append is true).
 * Returns the file descriptor, or -1 on failure.
 */
CHKRET JIM_EXPORT int Jim_OpenForWrite(const char* filename, int append);

/**
 * Opens the file for reading.
 * Returns the file descriptor, or -1 on failure.
 */
CHKRET JIM_EXPORT int Jim_OpenForRead(const char* filename);

/**
 * Unix-compatible errno
 */
CHKRET JIM_EXPORT int Jim_Errno(void);


extern int  g_JIM_MAINTAINER_VAL;

END_JIM_NAMESPACE

#include <jim-subcmd.h>

