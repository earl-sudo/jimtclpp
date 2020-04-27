#pragma once

#include <stdint.h>
#include <stdio.h>

#ifndef HAVE_NO_AUTOCONF // #optionalCode
#include <jim-config.h>
#endif

#include <jim-base.h>


BEGIN_JIM_NAMESPACE

/* -----------------------------------------------------------------------------
 * System configuration
 * autoconf (configure) will set these
 * ---------------------------------------------------------------------------*/

 /* -----------------------------------------------------------------------------
  * Compiler specific fixes.
  * ---------------------------------------------------------------------------*/

/* Long Long type and related issues */
#ifndef jim_wide // #optionalCode #WinOff
#  ifdef HAVE_LONG_LONG
#    define jim_wide long long
#    define JIM_WIDE_MODIFIER "lld"
#    define JIM_WIDE_8BYTE 1
#else
#    define jim_wide long
#    define JIM_WIDE_MODIFIER "ld"
#    define JIM_WIDE_4BYTE 1
#  endif
#endif

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
typedef Jim_HashEntry*          Jim_HashEntryArray;
typedef void*                   VoidPtrArrray;
typedef Jim_Obj*                Jim_ObjArray;

#define JIM_EXPORT

#ifndef JIM_API_INLINE // #optionalCode
#  ifdef JIM_INLINE_API_SMALLFUNCS
#    define JIM_API_INLINE JIM_EXPORT inline
#  else
#    define JIM_API_INLINE JIM_EXPORT
#  endif
#endif

/* -----------------------------------------------------------------------------
 * Stack
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Hash table
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Jim_Obj structure
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Call frame, vars, commands structures
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Jim interpreter structure.
 * Fields similar to the real Tcl interpreter structure have the same names.
 * ---------------------------------------------------------------------------*/
typedef int Jim_CmdProc(Jim_Interp *interp, int argc,
                        Jim_Obj *const *argv);
typedef void Jim_DelCmdProc(Jim_Interp *interp, void *privData);

JIM_API_INLINE long Jim_GetId(Jim_Interp* i);

/* -----------------------------------------------------------------------------
 * Exported API prototypes.
 * ---------------------------------------------------------------------------*/

#define Jim_FreeNewObj Jim_FreeObj

void Jim_FreeObj(Jim_Interp *interp, Jim_Obj *objPtr); /* EJ HACK #TODO */
JIM_API_INLINE void Jim_IncrRefCount(Jim_Obj* objPtr);
JIM_API_INLINE void Jim_DecrRefCount(Jim_Interp* interp, Jim_Obj* objPtr);
JIM_EXPORT int  Jim_RefCount(Jim_Obj* objPtr);
JIM_API_INLINE int Jim_IsShared(Jim_Obj* objPtr);

/* Memory allocation */
JIM_EXPORT void *Jim_Alloc(int sizeInBytes);
JIM_EXPORT void *Jim_Realloc(void *ptr, int sizeInBytes);
JIM_EXPORT void Jim_Free(void *ptr);
JIM_EXPORT char * Jim_StrDup(const char *s);
JIM_EXPORT char *Jim_StrDupLen(const char *s, int l /* num 1 byte characters */);

//template<typename T>
//T* Jim_TAlloc(T* v, int N = 1) { return (T*) Jim_Alloc(N * sizeof(T)); }

//template<typename T>
//T* Jim_TAllocZ(T* v, int N = 1) { auto v = (T*) Jim_Alloc(N * sizeof(T)); memset(v, 0, sizeof(T) * N);  }

template<typename T>
T* Jim_TAllocZ(int N = 1) { auto v = (T*) Jim_Alloc(N * sizeof(T)); memset(v, 0, sizeof(T) * N); return v;  }

template<typename T>
T* Jim_TAlloc(int N = 1) { return (T*) Jim_Alloc(N * sizeof(T));  }

template<typename T>
void Jim_TFree(T* p) { Jim_Free(p); }

template<typename T>
T* Jim_TRealloc(T* ptr, int N) {
    return (T*)Jim_Realloc(ptr, N * sizeof(T));
}

/* environment */
JIM_EXPORT char **Jim_GetEnviron(void);
JIM_EXPORT void Jim_SetEnviron(char **env);
JIM_EXPORT int Jim_MakeTempFile(Jim_Interp *interp, 
                                const char *filename_template, int unlink_file /*bool*/);
//
/* evaluation */
JIM_EXPORT Retval Jim_Eval(Jim_Interp *interp, const char *script);
/* in C code, you can do this and get better error messages */
/*   Jim_EvalSource( interp, __FILE__, __LINE__ , "some tcl commands"); */
JIM_EXPORT Retval Jim_EvalSource(Jim_Interp *interp, const char *filename, 
                              int lineno, const char *script);
/* Backwards compatibility */
#define Jim_Eval_Named(I, S, F, L) Jim_EvalSource((I), (F), (L), (S)) // #TODO

JIM_EXPORT Retval Jim_EvalGlobal(Jim_Interp *interp, const char *script);
JIM_EXPORT Retval Jim_EvalFile(Jim_Interp *interp, const char *filename);
JIM_EXPORT Retval Jim_EvalFileGlobal(Jim_Interp *interp, const char *filename);
JIM_EXPORT Retval Jim_EvalObj(Jim_Interp *interp, Jim_Obj *scriptObjPtr);
JIM_EXPORT Retval Jim_EvalObjVector(Jim_Interp *interp, int objc,
                                 Jim_Obj *const *objv);
JIM_EXPORT Retval Jim_EvalObjList(Jim_Interp *interp, Jim_Obj *listObj);
JIM_EXPORT Retval Jim_EvalObjPrefix(Jim_Interp *interp, Jim_Obj *prefix,
                                 int objc, Jim_Obj *const *objv);
#define Jim_EvalPrefix(i, p, oc, ov) Jim_EvalObjPrefix((i), Jim_NewStringObj((i), (p), -1), (oc), (ov)) // #TODO
JIM_EXPORT Retval Jim_EvalNamespace(Jim_Interp *interp, Jim_Obj *scriptObj, Jim_Obj *nsObj);
JIM_EXPORT Retval Jim_SubstObj(Jim_Interp *interp, Jim_Obj *substObjPtr,
                            Jim_Obj **resObjPtrPtr, int flags);

/* stack */
JIM_EXPORT Jim_Stack* Jim_AllocStack(void);
JIM_EXPORT void Jim_InitStack(Jim_Stack *stack);
JIM_EXPORT void Jim_FreeStack(Jim_Stack *stack);
JIM_EXPORT int Jim_StackLen(Jim_Stack *stack);
JIM_EXPORT void Jim_StackPush(Jim_Stack *stack, void *element);
JIM_EXPORT void * Jim_StackPop(Jim_Stack *stack);
JIM_EXPORT void * Jim_StackPeek(Jim_Stack *stack);
JIM_EXPORT void Jim_FreeStackElements(Jim_Stack *stack, void(*freeFunc)(void *ptr));

/* hash table */
JIM_EXPORT Retval Jim_InitHashTable(Jim_HashTable *ht,
                                 const Jim_HashTableType *type, void *privdata);
JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTable *ht,
                                    unsigned_int size);
JIM_EXPORT Retval Jim_AddHashEntry(Jim_HashTable *ht, const void *key,
                                void *val);
JIM_EXPORT int Jim_ReplaceHashEntry(Jim_HashTable *ht,
                                    const void *key, void *val);
JIM_EXPORT Retval Jim_DeleteHashEntry(Jim_HashTable *ht,
                                   const void *key);
JIM_EXPORT Retval Jim_FreeHashTable(Jim_HashTable *ht);
JIM_EXPORT Jim_HashEntry * Jim_FindHashEntry(Jim_HashTable *ht,
                                             const void *key);
JIM_EXPORT void Jim_ResizeHashTable(Jim_HashTable *ht);
JIM_EXPORT Jim_HashTableIterator *Jim_GetHashTableIterator(Jim_HashTable *ht);
JIM_EXPORT Jim_HashEntry * Jim_NextHashEntry(Jim_HashTableIterator *iter);
JIM_EXPORT const char* Jim_KeyAsStr(Jim_HashEntry* he);
JIM_EXPORT const void* Jim_KeyAsVoid(Jim_HashEntry* he);

/* objects */
JIM_EXPORT Jim_Obj * Jim_NewObj(Jim_Interp *interp);
JIM_EXPORT void Jim_FreeObj(Jim_Interp *interp, Jim_Obj *objPtr);
JIM_EXPORT void Jim_InvalidateStringRep(Jim_Obj *objPtr);
JIM_EXPORT Jim_Obj * Jim_DuplicateObj(Jim_Interp *interp,
                                      Jim_Obj *objPtr);
JIM_EXPORT const char * Jim_GetString(Jim_Obj *objPtr,
                                      int *lenPtr);
JIM_EXPORT const char *Jim_String(Jim_Obj *objPtr);
JIM_EXPORT int Jim_Length(Jim_Obj *objPtr);

/* string object */
JIM_EXPORT Jim_Obj * Jim_NewStringObj(Jim_Interp *interp,
                                      const char *s, int len /* -1 means strlen(s) */);
JIM_EXPORT Jim_Obj *Jim_NewStringObjUtf8(Jim_Interp *interp,
                                         const char *s, int charlen /* num chars */);
JIM_EXPORT Jim_Obj * Jim_NewStringObjNoAlloc(Jim_Interp *interp,
                                             char *s, int len /* -1 means strlen(s) */);
JIM_EXPORT void Jim_AppendString(Jim_Interp *interp, Jim_Obj *objPtr,
                                 const char *str, int len /* -1 means strlen(s) */);
JIM_EXPORT void Jim_AppendObj(Jim_Interp *interp, Jim_Obj *objPtr,
                              Jim_Obj *appendObjPtr);
JIM_EXPORT void Jim_AppendStrings(Jim_Interp *interp,
                                  Jim_Obj *objPtr, ...);
JIM_EXPORT int Jim_StringEqObj(Jim_Obj *aObjPtr, Jim_Obj *bObjPtr);
JIM_EXPORT int Jim_StringMatchObj(Jim_Interp *interp, Jim_Obj *patternObjPtr,
                                  Jim_Obj *objPtr, int nocase /*bool*/);
JIM_EXPORT Jim_Obj * Jim_StringRangeObj(Jim_Interp *interp,
                                        Jim_Obj *strObjPtr, Jim_Obj *firstObjPtr,
                                        Jim_Obj *lastObjPtr);
JIM_EXPORT Jim_Obj * Jim_FormatString(Jim_Interp *interp,
                                      Jim_Obj *fmtObjPtr, int objc, Jim_Obj *const *objv);
JIM_EXPORT Jim_Obj * Jim_ScanString(Jim_Interp *interp, Jim_Obj *strObjPtr,
                                    Jim_Obj *fmtObjPtr, int flags);
JIM_EXPORT int Jim_CompareStringImmediate(Jim_Interp *interp,
                                          Jim_Obj *objPtr, const char *str);
JIM_EXPORT int Jim_StringCompareObj(Jim_Interp *interp, Jim_Obj *firstObjPtr,
                                    Jim_Obj *secondObjPtr, int nocase /*bool*/);
JIM_EXPORT int Jim_StringCompareLenObj(Jim_Interp *interp, Jim_Obj *firstObjPtr,
                                       Jim_Obj *secondObjPtr, int nocase /*bool*/);
JIM_EXPORT int Jim_Utf8Length(Jim_Interp *interp, Jim_Obj *objPtr);

/* reference object */
JIM_EXPORT Jim_Obj * Jim_NewReference(Jim_Interp *interp,
                                      Jim_Obj *objPtr, Jim_Obj *tagPtr, Jim_Obj *cmdNamePtr);
JIM_EXPORT Jim_Reference * Jim_GetReference(Jim_Interp *interp,
                                            Jim_Obj *objPtr);
JIM_EXPORT Retval Jim_SetFinalizer(Jim_Interp *interp, Jim_Obj *objPtr, Jim_Obj *cmdNamePtr);
JIM_EXPORT Retval Jim_GetFinalizer(Jim_Interp *interp, Jim_Obj *objPtr, Jim_Obj **cmdNamePtrPtr);

/* interpreter */
JIM_EXPORT Jim_Interp * Jim_CreateInterp(void);
JIM_EXPORT void Jim_FreeInterp(Jim_Interp *i);
JIM_EXPORT int Jim_GetExitCode(Jim_Interp *interp);
JIM_EXPORT const char *Jim_ReturnCode(int code);
JIM_EXPORT void Jim_SetResultFormatted(Jim_Interp *interp, const char *format, ...);
JIM_EXPORT Jim_CallFrame* Jim_TopCallFrame(Jim_Interp* interp);
JIM_EXPORT Jim_Obj* Jim_CurrentNamespace(Jim_Interp* interp);
JIM_EXPORT Jim_Obj* Jim_EmptyObj(Jim_Interp* interp);
JIM_EXPORT int Jim_CurrentLevel(Jim_Interp* interp);
JIM_EXPORT Jim_HashTable* Jim_PackagesHT(Jim_Interp* interp);
JIM_EXPORT void Jim_IncrStackTrace(Jim_Interp* interp);

/* commands */
JIM_EXPORT void Jim_RegisterCoreCommands(Jim_Interp *interp);
JIM_EXPORT Retval Jim_CreateCommand(Jim_Interp *interp,
                                 const char *cmdName, Jim_CmdProc *cmdProc, void *privData,
                                 Jim_DelCmdProc *delProc);
JIM_EXPORT Retval Jim_DeleteCommand(Jim_Interp *interp,
                                 const char *cmdName);
JIM_EXPORT Retval Jim_RenameCommand(Jim_Interp *interp,
                                 const char *oldName, const char *newName);
JIM_EXPORT Jim_Cmd * Jim_GetCommand(Jim_Interp *interp,
                                    Jim_Obj *objPtr, int flags);
JIM_EXPORT Retval Jim_SetVariable(Jim_Interp *interp,
                               Jim_Obj *nameObjPtr, Jim_Obj *valObjPtr);
JIM_EXPORT Retval Jim_SetVariableStr(Jim_Interp *interp,
                                  const char *name, Jim_Obj *objPtr);
JIM_EXPORT Retval Jim_SetGlobalVariableStr(Jim_Interp *interp,
                                        const char *name, Jim_Obj *objPtr);
JIM_EXPORT Retval Jim_SetVariableStrWithStr(Jim_Interp *interp,
                                         const char *name, const char *val);
JIM_EXPORT Retval Jim_SetVariableLink(Jim_Interp *interp,
                                   Jim_Obj *nameObjPtr, Jim_Obj *targetNameObjPtr,
                                   Jim_CallFrame *targetCallFrame);
JIM_EXPORT Jim_Obj * Jim_MakeGlobalNamespaceName(Jim_Interp *interp,
                                                 Jim_Obj *nameObjPtr);
JIM_EXPORT Jim_Obj * Jim_GetVariable(Jim_Interp *interp,
                                     Jim_Obj *nameObjPtr, int flags);
JIM_EXPORT Jim_Obj * Jim_GetGlobalVariable(Jim_Interp *interp,
                                           Jim_Obj *nameObjPtr, int flags);
JIM_EXPORT Jim_Obj * Jim_GetVariableStr(Jim_Interp *interp,
                                        const char *name, int flags);
JIM_EXPORT Jim_Obj * Jim_GetGlobalVariableStr(Jim_Interp *interp,
                                              const char *name, int flags);
JIM_EXPORT Retval Jim_UnsetVariable(Jim_Interp *interp,
                                 Jim_Obj *nameObjPtr, int flags);

/* call frame */
JIM_EXPORT Jim_CallFrame *Jim_GetCallFrameByLevel(Jim_Interp *interp,
                                                  Jim_Obj *levelObjPtr);

/* garbage collection */
JIM_EXPORT int Jim_Collect(Jim_Interp *interp);
JIM_EXPORT void Jim_CollectIfNeeded(Jim_Interp *interp);

/* index object */
JIM_EXPORT Retval Jim_GetIndex(Jim_Interp *interp, Jim_Obj *objPtr,
                            int *indexPtr /* on error set INT_MAX/-INT_MAX */);

/* list object */
JIM_EXPORT Jim_Obj * Jim_NewListObj(Jim_Interp *interp,
                                    Jim_Obj *const *elements, int len);
JIM_EXPORT void Jim_ListInsertElements(Jim_Interp *interp,
                                       Jim_Obj *listPtr, int listindex, int objc, 
                                       Jim_Obj *const *objVec);
JIM_EXPORT void Jim_ListAppendElement(Jim_Interp *interp,
                                      Jim_Obj *listPtr, Jim_Obj *objPtr);
JIM_EXPORT void Jim_ListAppendList(Jim_Interp *interp,
                                   Jim_Obj *listPtr, Jim_Obj *appendListPtr);
JIM_EXPORT int Jim_ListLength(Jim_Interp *interp, Jim_Obj *objPtr);
JIM_EXPORT Retval Jim_ListIndex(Jim_Interp *interp, Jim_Obj *listPrt,
                             int listindex, Jim_Obj **objPtrPtr, int seterr);
JIM_EXPORT Jim_Obj *Jim_ListGetIndex(Jim_Interp *interp, Jim_Obj *listPtr, int idx);
JIM_EXPORT int Jim_SetListIndex(Jim_Interp *interp,
                                Jim_Obj *varNamePtr, Jim_Obj *const *indexv, int indexc,
                                Jim_Obj *newObjPtr);
JIM_EXPORT Jim_Obj * Jim_ConcatObj(Jim_Interp *interp, int objc,
                                   Jim_Obj *const *objv);
JIM_EXPORT Jim_Obj *Jim_ListJoin(Jim_Interp *interp,
                                 Jim_Obj *listObjPtr, const char *joinStr, int joinStrLen);

/* dict object */
JIM_EXPORT Jim_Obj * Jim_NewDictObj(Jim_Interp *interp,
                                    Jim_Obj *const *elements, int len);
JIM_EXPORT Retval Jim_DictKey(Jim_Interp *interp, Jim_Obj *dictPtr,
                           Jim_Obj *keyPtr, Jim_Obj **objPtrPtr, int flags);
JIM_EXPORT Retval Jim_DictKeysVector(Jim_Interp *interp,
                                  Jim_Obj *dictPtr, Jim_Obj *const *keyv, int keyc,
                                  Jim_Obj **objPtrPtr, int flags);
JIM_EXPORT Retval Jim_SetDictKeysVector (Jim_Interp *interp,
                                     Jim_Obj *varNamePtr, Jim_Obj *const *keyv, int keyc,
                                     Jim_Obj *newObjPtr, int flags);
JIM_EXPORT Retval Jim_DictPairs(Jim_Interp *interp,
                             Jim_Obj *dictPtr, Jim_Obj ***objPtrPtr, int *len);
JIM_EXPORT Retval Jim_DictAddElement(Jim_Interp *interp, Jim_Obj *objPtr,
                                  Jim_Obj *keyObjPtr, Jim_Obj *valueObjPtr);

enum JIM_DICTMATCH {
    JIM_DICTMATCH_KEYS = 0x0001,
    JIM_DICTMATCH_VALUES = 0x002
};

JIM_EXPORT Retval Jim_DictMatchTypes(Jim_Interp *interp, Jim_Obj *objPtr, Jim_Obj *patternObj, 
                                  int match_type, int return_types);
JIM_EXPORT int Jim_DictSize(Jim_Interp *interp, Jim_Obj *objPtr);
JIM_EXPORT Retval Jim_DictInfo(Jim_Interp *interp, Jim_Obj *objPtr);
JIM_EXPORT Jim_Obj *Jim_DictMerge(Jim_Interp *interp, int objc, Jim_Obj *const *objv);

/* return code object */
JIM_EXPORT Retval Jim_GetReturnCode(Jim_Interp *interp, Jim_Obj *objPtr,
                                 int *intPtr);

/* expression object */
JIM_EXPORT Retval Jim_EvalExpression(Jim_Interp *interp,
                                  Jim_Obj *exprObjPtr);
JIM_EXPORT Retval Jim_GetBoolFromExpr(Jim_Interp *interp,
                                   Jim_Obj *exprObjPtr, int *boolPtr);

/* boolean object */
JIM_EXPORT Retval Jim_GetBoolean(Jim_Interp *interp, Jim_Obj *objPtr,
                              int *booleanPtr);

/* integer object */
JIM_EXPORT Retval Jim_GetWide (Jim_Interp *interp, Jim_Obj *objPtr,
                           jim_wide *widePtr);
JIM_EXPORT Retval Jim_GetLong(Jim_Interp *interp, Jim_Obj *objPtr,
                           long *longPtr);
#define Jim_NewWideObj  Jim_NewIntObj // #TODO
JIM_EXPORT Jim_Obj * Jim_NewIntObj(Jim_Interp *interp,
                                   jim_wide wideValue);

/* double object */
JIM_EXPORT Retval Jim_GetDouble(Jim_Interp *interp, Jim_Obj *objPtr,
                             double *doublePtr);
JIM_EXPORT void Jim_SetDouble(Jim_Interp *interp, Jim_Obj *objPtr,
                              double doubleValue);
JIM_EXPORT Jim_Obj * Jim_NewDoubleObj(Jim_Interp *interp, double doubleValue);

/* commands utilities */
JIM_EXPORT void Jim_WrongNumArgs(Jim_Interp *interp, int argc,
                                 Jim_Obj *const *argv, const char *msg);
JIM_EXPORT Retval Jim_GetEnum(Jim_Interp *interp, Jim_Obj *objPtr,
                           const char * const *tablePtr, int *indexPtr, const char *name, int flags);
JIM_EXPORT Retval Jim_CheckShowCommands(Jim_Interp *interp, Jim_Obj *objPtr,
                                     const char *const *tablePtr);
JIM_EXPORT int Jim_ScriptIsComplete(Jim_Interp *interp,
                                    Jim_Obj *scriptObj, char *stateCharPtr);

/**
 * Find a matching name in the array of the given length.
 *
 * NULL entries are ignored.
 *
 * Returns the matching index if found, or -1 if not.
 */
JIM_EXPORT int Jim_FindByName(const char *name, const char * const array[], size_t len);

/* package utilities */
typedef void (Jim_InterpDeleteProc)(Jim_Interp *interp, void *data);
JIM_EXPORT void * Jim_GetAssocData(Jim_Interp *interp, const char *key);
JIM_EXPORT Retval Jim_SetAssocData(Jim_Interp *interp, const char *key,
                                Jim_InterpDeleteProc *delProc, void *data);
JIM_EXPORT Retval Jim_DeleteAssocData(Jim_Interp *interp, const char *key);

/* Packages C API */
/* jim-package.c */
JIM_EXPORT Retval Jim_PackageProvide(Jim_Interp *interp,
                                  const char *name, const char *ver, int flags);
JIM_EXPORT Retval Jim_PackageRequire(Jim_Interp *interp,
                                  const char *name, int flags);

/* error messages */
JIM_EXPORT void Jim_MakeErrorMessage(Jim_Interp *interp);

/* interactive mode */
JIM_EXPORT Retval Jim_InteractivePrompt(Jim_Interp *interp);
JIM_EXPORT void Jim_HistoryLoad(const char *filename);
JIM_EXPORT void Jim_HistorySave(const char *filename);
JIM_EXPORT char *Jim_HistoryGetline(Jim_Interp *interp, const char *prompt);
JIM_EXPORT void Jim_HistorySetCompletion(Jim_Interp *interp, Jim_Obj *commandObj);
JIM_EXPORT void Jim_HistoryAdd(const char *line);
JIM_EXPORT void Jim_HistoryShow(void);

/* Misc */
JIM_EXPORT Retval Jim_InitStaticExtensions(Jim_Interp *interp);
JIM_EXPORT Retval Jim_StringToWide(const char *str, jim_wide *widePtr, int base);
JIM_EXPORT int Jim_IsBigEndian(void);

/**
 * Returns 1 if a signal has been received while
 * in a catch -signal {} clause.
 */
JIM_API_INLINE long_long Jim_CheckSignal(Jim_Interp* i);

/* jim-load.c */
JIM_EXPORT Retval Jim_LoadLibrary(Jim_Interp *interp, const char *pathName);
JIM_EXPORT void Jim_FreeLoadHandles(Jim_Interp *interp);

/* jim-aio.c */
JIM_EXPORT FILE *Jim_AioFilehandle(Jim_Interp *interp, Jim_Obj *command);

/* type inspection - avoid where possible */
JIM_EXPORT int Jim_IsDict(Jim_Obj *objPtr);
JIM_EXPORT int Jim_IsList(Jim_Obj *objPtr);

JIM_API_INLINE void Jim_SetResult(Jim_Interp* i, Jim_Obj* o);
JIM_API_INLINE void Jim_InterpIncrProcEpoch(Jim_Interp* i);
JIM_API_INLINE void Jim_SetResultString(Jim_Interp* i, const char* s, int l /* -1 means strlen(s) */);
JIM_API_INLINE void Jim_SetResultInt(Jim_Interp* i, long_long intval);
JIM_API_INLINE void Jim_SetResultBool(Jim_Interp* i, long_long b);
JIM_API_INLINE void Jim_SetEmptyResult(Jim_Interp* i);
JIM_API_INLINE Jim_Obj* Jim_GetResult(Jim_Interp* i);
JIM_API_INLINE void* Jim_CmdPrivData(Jim_Interp* i);

JIM_API_INLINE Jim_Obj* Jim_NewEmptyStringObj(Jim_Interp* i);
JIM_API_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter);

extern int  g_JIM_MAINTAINER_VAL;

END_JIM_NAMESPACE

#include <jim-subcmd.h>

