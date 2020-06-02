#pragma once

/* C Version of jim-api.h for those that really want C. */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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

    enum JIM_RETURNS {
        JRET(JIM_OK),
        JRET(JIM_ERR),
        JRET(JIM_RETURN),
        JRET(JIM_BREAK),
        JRET(JIM_CONTINUE),
        JRET(JIM_SIGNAL),
        JRET(JIM_EXIT),
        /* The following are internal codes and should never been seen/used */
        JRET(JIM_EVAL)
    };

    /* Filesystem related */
    enum {
        JIM_PATH_LEN = 1024 // #MagicNum
    };

#define JIM_LIBPATH "auto_path" // #MagicStr

    /* -----------------------------------------------------------------------------
     * Forwards
     * ---------------------------------------------------------------------------*/
#include <jim-forwards.h>

// For now I am not building shared libraries so this is disabled.
#define JIM_CEXPORT // #disabled_option #optionalCode
#ifndef JIM_CEXPORT
#  ifdef PRJ_OS_WIN
     // We don't handle DLL and EXE differently.  Little know fact is you can call functions off EXE's.
#    define JIM_CEXPORT __declspec(dllexport)
#  endif
#endif

#ifndef JIM_CAPI_INLINE // #optionalCode
#  ifdef JIM_INLINE_API_SMALLFUNCS
#    define JIM_CAPI_INLINE JIM_CEXPORT inline
#  else
#    define JIM_CAPI_INLINE JIM_CEXPORT
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
    typedef int Jim_CmdProc(Jim_InterpPtr interp, int argc,
                            Jim_ObjConstArray argv);
    typedef void Jim_DelCmdProc(Jim_InterpPtr interp, void* privData);

    JIM_CAPI_INLINE long Jim_GetId(Jim_InterpPtr  i);

    /* -----------------------------------------------------------------------------
     * Exported API prototypes.
     * ---------------------------------------------------------------------------*/

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
    /* in C code, you can do this and get better errorText_ messages */
    /*   Jim_EvalSource( interp_, __FILE__, __LINE__ , "some tcl commands"); */
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

    /* stack_ */
    JIM_CEXPORT Jim_StackPtr  Jim_AllocStack(void);
    JIM_CEXPORT void Jim_InitStack(Jim_StackPtr stack);
    JIM_CEXPORT void Jim_FreeStack(Jim_StackPtr stack);
    JIM_CEXPORT int Jim_StackLen(Jim_StackPtr stack);
    JIM_CEXPORT void Jim_StackPush(Jim_StackPtr stack, void* element);
    JIM_CEXPORT void* Jim_StackPop(Jim_StackPtr stack);
    JIM_CEXPORT void* Jim_StackPeek(Jim_StackPtr stack);
    JIM_CEXPORT void Jim_FreeStackElements(Jim_StackPtr stack, void(*freeFunc)(void* ptr));

    /* hash table */
    JIM_CEXPORT void Jim_InitHashTable(Jim_HashTablePtr ht,
                                        const Jim_HashTableType* type, void* privdata);
    JIM_CEXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht,
                                        unsigned_int size);
    JIM_CEXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void* key,
                                       void* val);
    JIM_CEXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht,
                                        const void* key, void* val);
    JIM_CEXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht,
                                          const void* key);
    JIM_CEXPORT void Jim_FreeHashTable(Jim_HashTablePtr ht);
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
    JIM_CEXPORT Jim_ReferencePtr  Jim_GetReference(Jim_InterpPtr interp,
                                               Jim_ObjPtr  objPtr);
    JIM_CEXPORT Retval Jim_SetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjPtr  cmdNamePtr);
    JIM_CEXPORT Retval Jim_GetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjArray* cmdNamePtrPtr);

    /* interpreter */
    JIM_CEXPORT Jim_InterpPtr  Jim_CreateInterp(void);
    JIM_CEXPORT void Jim_FreeInterp(Jim_InterpPtr i);
    JIM_CEXPORT int Jim_GetExitCode(Jim_InterpPtr interp);
    JIM_CEXPORT const char* Jim_ReturnCode(int code);
    JIM_CEXPORT void Jim_SetResultFormatted(Jim_InterpPtr interp, const char* format, ...);
    JIM_CEXPORT Jim_CallFramePtr  Jim_TopCallFrame(Jim_InterpPtr  interp);
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
    JIM_CEXPORT Jim_CmdPtr  Jim_GetCommand(Jim_InterpPtr interp,
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
                                          Jim_CallFramePtr  targetCallFrame);
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
    JIM_CEXPORT Jim_CallFramePtr  Jim_GetCallFrameByLevel(Jim_InterpPtr interp,
                                                      Jim_ObjPtr  levelObjPtr);

    /* garbage collection */
    JIM_CEXPORT int Jim_Collect(Jim_InterpPtr interp);
    JIM_CEXPORT void Jim_CollectIfNeeded(Jim_InterpPtr interp);

    /* index object */
    JIM_CEXPORT Retval Jim_GetIndex(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                   int* indexPtr /* on errorText_ set INT_MAX/-INT_MAX */);

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
     * Find a matching name_ in the array of the given length.
     *
     * nullptr entries are ignored.
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

    /* errorText_ messages */
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

    /* tokenType_ inspection - avoid where possible */
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
     * Set an errorText_ result based on errno and the given message.
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
};
#endif