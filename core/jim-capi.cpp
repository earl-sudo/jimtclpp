#include <jim-api.h>
#include <jim-capi.h>

/* -----------------------------------------------------------------------------
 * Jim interpreter structure.
 * Fields similar to the real Tcl interpreter structure have the same names.
 * ---------------------------------------------------------------------------*/


JIM_CAPI_INLINE long Jim_GetId(Jim_InterpPtr  i) {
    return JIM_NAMESPACE_NAME::Jim_GetId((JIM_NAMESPACE_NAME::Jim_InterpPtr)i);
}

/* -----------------------------------------------------------------------------
 * Exported API prototypes.
 * ---------------------------------------------------------------------------*/

 // (JIM_NAMESPACE_NAME::Jim_InterpPtr)
 // (JIM_NAMESPACE_NAME::Jim_Obj*)
 // (JIM_NAMESPACE_NAME::Jim_ObjConstArray)

void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr  objPtr) {
    return JIM_NAMESPACE_NAME::Jim_FreeObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr);
}

JIM_CAPI_INLINE void Jim_IncrRefCount(Jim_ObjPtr  objPtr) { JIM_NAMESPACE_NAME::Jim_IncrRefCount((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CAPI_INLINE void Jim_DecrRefCount(Jim_InterpPtr  interp, Jim_ObjPtr  objPtr) { JIM_NAMESPACE_NAME::Jim_DecrRefCount((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT int  Jim_RefCount(Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_RefCount((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CAPI_INLINE int Jim_IsShared(Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_IsShared((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }

/* Memory allocation */
JIM_CEXPORT void* Jim_Alloc(int sizeInBytes) { return JIM_NAMESPACE_NAME::Jim_Alloc(sizeInBytes); }
JIM_CEXPORT void* Jim_Realloc(void* ptr, int sizeInBytes) { return JIM_NAMESPACE_NAME::Jim_Realloc(ptr, sizeInBytes); }
JIM_CEXPORT void Jim_Free(void* ptr) { JIM_NAMESPACE_NAME::Jim_Free(ptr); }
JIM_CEXPORT char* Jim_StrDup(const char* s) { return JIM_NAMESPACE_NAME::Jim_StrDup(s); }
JIM_CEXPORT char* Jim_StrDupLen(const char* s, int l /* num 1 byte characters */) { return JIM_NAMESPACE_NAME::Jim_StrDupLen(s, l); }


/* environment */
JIM_CEXPORT char** Jim_GetEnviron(void) { return JIM_NAMESPACE_NAME::Jim_GetEnviron(); }
JIM_CEXPORT void Jim_SetEnviron(char** env) { JIM_NAMESPACE_NAME::Jim_SetEnviron(env); }
JIM_CEXPORT int Jim_MakeTempFile(Jim_InterpPtr interp,
                                 const char* filename_template, int unlink_file /*bool*/) { return JIM_NAMESPACE_NAME::Jim_MakeTempFile((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, filename_template, unlink_file); }
//
/* evaluation */
JIM_CEXPORT Retval Jim_Eval(Jim_InterpPtr interp, const char* script) { return (Retval) JIM_NAMESPACE_NAME::Jim_Eval((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, script); }
/* in C code, you can do this and get better error messages */
/*   Jim_EvalSource( interp, __FILE__, __LINE__ , "some tcl commands") { return JIM_NAMESPACE_NAME::FFF(AAA); } */
JIM_CEXPORT Retval Jim_EvalSource(Jim_InterpPtr interp, const char* filename,
                                  int lineno, const char* script) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalSource((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, filename, lineno, script); }
/* Backwards compatibility */


JIM_CEXPORT Retval Jim_EvalGlobal(Jim_InterpPtr interp, const char* script) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalGlobal((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, script); }
JIM_CEXPORT Retval Jim_EvalFile(Jim_InterpPtr interp, const char* filename) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalFile((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, filename); }
JIM_CEXPORT Retval Jim_EvalFileGlobal(Jim_InterpPtr interp, const char* filename) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalFileGlobal((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, filename); }
JIM_CEXPORT Retval Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr  scriptObjPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )scriptObjPtr); }
JIM_CEXPORT Retval Jim_EvalObjVector(Jim_InterpPtr interp, int objc,
                                     Jim_ObjConstArray objv) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalObjVector((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, objc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)objv); }
JIM_CEXPORT Retval Jim_EvalObjList(Jim_InterpPtr interp, Jim_ObjPtr  listObj) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalObjList((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listObj); }
JIM_CEXPORT Retval Jim_EvalObjPrefix(Jim_InterpPtr interp, Jim_ObjPtr  prefix,
                                     int objc, Jim_ObjConstArray objv) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalObjPrefix((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )prefix, objc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)objv); }
//inline Retval Jim_EvalPrefix_(Jim_InterpPtr  i, const char* p, int oc, Jim_ObjConstArray  ov) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalPrefix_(i, p, oc, ov); }
JIM_CEXPORT Retval Jim_EvalNamespace(Jim_InterpPtr interp, Jim_ObjPtr  scriptObj, Jim_ObjPtr  nsObj) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalNamespace((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )scriptObj, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nsObj); }
JIM_CEXPORT Retval Jim_SubstObj(Jim_InterpPtr interp, Jim_ObjPtr  substObjPtr,
                                Jim_ObjArray* resObjPtrPtr, int flags) { return (Retval) JIM_NAMESPACE_NAME::Jim_SubstObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )substObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjArray*)resObjPtrPtr, flags); }

/* stack */
JIM_CEXPORT Jim_StackPtr  Jim_AllocStack(void) { return (Jim_StackPtr) JIM_NAMESPACE_NAME::Jim_AllocStack(); }
JIM_CEXPORT void Jim_InitStack(Jim_StackPtr stack) { JIM_NAMESPACE_NAME::Jim_InitStack((JIM_NAMESPACE_NAME::Jim_StackPtr)stack); }
JIM_CEXPORT void Jim_FreeStack(Jim_StackPtr stack) { JIM_NAMESPACE_NAME::Jim_FreeStack((JIM_NAMESPACE_NAME::Jim_StackPtr)stack); }
JIM_CEXPORT int Jim_StackLen(Jim_StackPtr stack) { return JIM_NAMESPACE_NAME::Jim_StackLen((JIM_NAMESPACE_NAME::Jim_StackPtr)stack); }
JIM_CEXPORT void Jim_StackPush(Jim_StackPtr stack, void* element) { JIM_NAMESPACE_NAME::Jim_StackPush((JIM_NAMESPACE_NAME::Jim_StackPtr)stack, element); }
JIM_CEXPORT void* Jim_StackPop(Jim_StackPtr stack) { return JIM_NAMESPACE_NAME::Jim_StackPop((JIM_NAMESPACE_NAME::Jim_StackPtr)stack); }
JIM_CEXPORT void* Jim_StackPeek(Jim_StackPtr stack) { return JIM_NAMESPACE_NAME::Jim_StackPeek((JIM_NAMESPACE_NAME::Jim_StackPtr)stack); }
JIM_CEXPORT void Jim_FreeStackElements(Jim_StackPtr stack, void(*freeFunc)(void* ptr)) { JIM_NAMESPACE_NAME::Jim_FreeStackElements((JIM_NAMESPACE_NAME::Jim_StackPtr)stack, freeFunc); }

/* hash table */
JIM_CEXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht,
                                     const Jim_HashTableType* type, void* privdata) { return (Retval) JIM_NAMESPACE_NAME::Jim_InitHashTable((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht, (const JIM_NAMESPACE_NAME::Jim_HashTableType*)type, privdata); }
JIM_CEXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht,
                                     unsigned_int size) { return JIM_NAMESPACE_NAME::Jim_ExpandHashTable((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht, size); }
JIM_CEXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void* key,
                                    void* val) { return (Retval) JIM_NAMESPACE_NAME::Jim_AddHashEntry((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht, key, val); }
JIM_CEXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht,
                                     const void* key, void* val) { return JIM_NAMESPACE_NAME::Jim_ReplaceHashEntry((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht, key, val); }
JIM_CEXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht,
                                       const void* key) { return (Retval) JIM_NAMESPACE_NAME::Jim_DeleteHashEntry((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht, key); }
JIM_CEXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht) { return (Retval) JIM_NAMESPACE_NAME::Jim_FreeHashTable((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht); }
JIM_CEXPORT Jim_HashEntryPtr  Jim_FindHashEntry(Jim_HashTablePtr ht,
                                                const void* key) { return (Jim_HashEntryPtr) JIM_NAMESPACE_NAME::Jim_FindHashEntry((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht, key); }
JIM_CEXPORT void Jim_ResizeHashTable(Jim_HashTablePtr ht) { JIM_NAMESPACE_NAME::Jim_ResizeHashTable((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht); }
JIM_CEXPORT Jim_HashTableIterator* Jim_GetHashTableIterator(Jim_HashTablePtr ht) { return (Jim_HashTableIterator*) (JIM_NAMESPACE_NAME::Jim_HashTableIterator*)JIM_NAMESPACE_NAME::Jim_GetHashTableIterator((JIM_NAMESPACE_NAME::Jim_HashTablePtr)ht); }
JIM_CEXPORT Jim_HashEntryPtr  Jim_NextHashEntry(Jim_HashTableIterator* iter) { return (Jim_HashEntryPtr) JIM_NAMESPACE_NAME::Jim_NextHashEntry((JIM_NAMESPACE_NAME::Jim_HashTableIterator*)iter); }
JIM_CEXPORT const char* Jim_KeyAsStr(Jim_HashEntryPtr  he) { return JIM_NAMESPACE_NAME::Jim_KeyAsStr((JIM_NAMESPACE_NAME::Jim_HashEntryPtr)he); }
JIM_CEXPORT const void* Jim_KeyAsVoid(Jim_HashEntryPtr  he) { return JIM_NAMESPACE_NAME::Jim_KeyAsVoid((JIM_NAMESPACE_NAME::Jim_HashEntryPtr)he); }

/* objects */
JIM_CEXPORT Jim_ObjPtr  Jim_NewObj(Jim_InterpPtr interp) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
//JIM_CEXPORT void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr  objPtr) { JIM_NAMESPACE_NAME::Jim_FreeObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT void Jim_InvalidateStringRep(Jim_ObjPtr  objPtr) { JIM_NAMESPACE_NAME::Jim_InvalidateStringRep((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Jim_ObjPtr  Jim_DuplicateObj(Jim_InterpPtr interp,
                                      Jim_ObjPtr  objPtr) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_DuplicateObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT const char* Jim_GetString(Jim_ObjPtr  objPtr,
                                      int* lenPtr) { return JIM_NAMESPACE_NAME::Jim_GetString((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, lenPtr); }
JIM_CEXPORT const char* Jim_String(Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_String((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT int Jim_Length(Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_Length((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }

/* string object */
JIM_CEXPORT Jim_ObjPtr  Jim_NewStringObj(Jim_InterpPtr interp,
                                      const char* s, int len /* -1 means strlen(s) */) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewStringObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, s, len); }
JIM_CEXPORT Jim_ObjPtr  Jim_NewStringObjUtf8(Jim_InterpPtr interp,
                                          const char* s, int charlen /* num chars */) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewStringObjUtf8((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, s, charlen); }
JIM_CEXPORT Jim_ObjPtr  Jim_NewStringObjNoAlloc(Jim_InterpPtr interp,
                                             char* s, int len /* -1 means strlen(s) */) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewStringObjNoAlloc((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, s, len); }
JIM_CEXPORT void Jim_AppendString(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  const char* str, int len /* -1 means strlen(s) */) { JIM_NAMESPACE_NAME::Jim_AppendString((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, str, len); }
JIM_CEXPORT void Jim_AppendObj(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                               Jim_ObjPtr  appendObjPtr) { JIM_NAMESPACE_NAME::Jim_AppendObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )appendObjPtr); }
//JIM_CEXPORT void Jim_AppendStrings(Jim_InterpPtr interp,
//                                  Jim_ObjPtr  objPtr, ...) {  JIM_NAMESPACE_NAME::Jim_AppendStrings(AAA); }
JIM_CEXPORT int Jim_StringEqObj(Jim_ObjPtr  aObjPtr, Jim_ObjPtr  bObjPtr) { return JIM_NAMESPACE_NAME::Jim_StringEqObj((JIM_NAMESPACE_NAME::Jim_ObjPtr )aObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )bObjPtr); }
JIM_CEXPORT int Jim_StringMatchObj(Jim_InterpPtr interp, Jim_ObjPtr  patternObjPtr,
                                   Jim_ObjPtr  objPtr, int nocase /*bool*/) { return JIM_NAMESPACE_NAME::Jim_StringMatchObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )patternObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, nocase); }
JIM_CEXPORT Jim_ObjPtr  Jim_StringRangeObj(Jim_InterpPtr interp,
                                        Jim_ObjPtr  strObjPtr, Jim_ObjPtr  firstObjPtr,
                                        Jim_ObjPtr  lastObjPtr) {
    return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_StringRangeObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )strObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )firstObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )lastObjPtr);
}
JIM_CEXPORT Jim_ObjPtr  Jim_FormatString(Jim_InterpPtr interp,
                                      Jim_ObjPtr  fmtObjPtr, int objc, Jim_ObjConstArray objv) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_FormatString((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )fmtObjPtr, objc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)objv); }
JIM_CEXPORT Jim_ObjPtr  Jim_ScanString(Jim_InterpPtr interp, Jim_ObjPtr  strObjPtr,
                                    Jim_ObjPtr  fmtObjPtr, int flags) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_ScanString((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr ) strObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )fmtObjPtr, flags); }
JIM_CEXPORT int Jim_CompareStringImmediate(Jim_InterpPtr interp,
                                           Jim_ObjPtr  objPtr, const char* str) { return JIM_NAMESPACE_NAME::Jim_CompareStringImmediate((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr ) objPtr, str); }
JIM_CEXPORT int Jim_StringCompareObj(Jim_InterpPtr interp, Jim_ObjPtr  firstObjPtr,
                                     Jim_ObjPtr  secondObjPtr, int nocase /*bool*/) { return JIM_NAMESPACE_NAME::Jim_StringCompareObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )firstObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )secondObjPtr, nocase); }
JIM_CEXPORT int Jim_StringCompareLenObj(Jim_InterpPtr interp, Jim_ObjPtr  firstObjPtr,
                                        Jim_ObjPtr  secondObjPtr, int nocase /*bool*/) { return JIM_NAMESPACE_NAME::Jim_StringCompareLenObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )firstObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )secondObjPtr, nocase); }
JIM_CEXPORT int Jim_Utf8Length(Jim_InterpPtr interp, Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_Utf8Length((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }

/* reference object */
JIM_CEXPORT Jim_ObjPtr  Jim_NewReference(Jim_InterpPtr interp,
                                      Jim_ObjPtr  objPtr, Jim_ObjPtr  tagPtr, Jim_ObjPtr  cmdNamePtr) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewReference((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )tagPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr ) cmdNamePtr); }
JIM_CEXPORT Jim_Reference* Jim_GetReference(Jim_InterpPtr interp,
                                            Jim_ObjPtr  objPtr) { return (Jim_Reference*) JIM_NAMESPACE_NAME::Jim_GetReference((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Retval Jim_SetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjPtr  cmdNamePtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_SetFinalizer((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )cmdNamePtr); }
JIM_CEXPORT Retval Jim_GetFinalizer(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjArray* cmdNamePtrPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetFinalizer((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, (JIM_NAMESPACE_NAME::Jim_ObjArray*)cmdNamePtrPtr); }

/* interpreter */
JIM_CEXPORT Jim_InterpPtr  Jim_CreateInterp(void) { return (Jim_InterpPtr) JIM_NAMESPACE_NAME::Jim_CreateInterp(); }
JIM_CEXPORT void Jim_FreeInterp(Jim_InterpPtr i) { JIM_NAMESPACE_NAME::Jim_FreeInterp((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }
JIM_CEXPORT int Jim_GetExitCode(Jim_InterpPtr interp) { return JIM_NAMESPACE_NAME::Jim_GetExitCode((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT const char* Jim_ReturnCode(int code) { return JIM_NAMESPACE_NAME::Jim_ReturnCode(code); }
//JIM_CEXPORT void Jim_SetResultFormatted(Jim_InterpPtr interp, const char* format, ...) {  JIM_NAMESPACE_NAME::Jim_SetResultFormatted(interp, format, ...); }
JIM_CEXPORT Jim_CallFrame* Jim_TopCallFrame(Jim_InterpPtr  interp) { return (Jim_CallFrame*) JIM_NAMESPACE_NAME::Jim_TopCallFrame((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT Jim_ObjPtr  Jim_CurrentNamespace(Jim_InterpPtr  interp) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_CurrentNamespace((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT Jim_ObjPtr  Jim_EmptyObj(Jim_InterpPtr  interp) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_EmptyObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT int Jim_CurrentLevel(Jim_InterpPtr  interp) { return JIM_NAMESPACE_NAME::Jim_CurrentLevel((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT Jim_HashTablePtr  Jim_PackagesHT(Jim_InterpPtr  interp) { return (Jim_HashTablePtr) JIM_NAMESPACE_NAME::Jim_PackagesHT((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT void Jim_IncrStackTrace(Jim_InterpPtr  interp) { JIM_NAMESPACE_NAME::Jim_IncrStackTrace((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }

/* commands */
JIM_CEXPORT void Jim_RegisterCoreCommands(Jim_InterpPtr interp) { JIM_NAMESPACE_NAME::Jim_RegisterCoreCommands((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT Retval Jim_CreateCommand(Jim_InterpPtr interp,
                                     const char* cmdName, Jim_CmdProc* cmdProc, void* privData,
                                     Jim_DelCmdProc* delProc) {
    return (Retval) JIM_NAMESPACE_NAME::Jim_CreateCommand((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, cmdName, (JIM_NAMESPACE_NAME::Jim_CmdProc*)cmdProc, privData, (JIM_NAMESPACE_NAME::Jim_DelCmdProc*)delProc);
}
JIM_CEXPORT Retval Jim_DeleteCommand(Jim_InterpPtr interp,
                                     const char* cmdName) { return (Retval) JIM_NAMESPACE_NAME::Jim_DeleteCommand((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, cmdName); }
JIM_CEXPORT Retval Jim_RenameCommand(Jim_InterpPtr interp,
                                     const char* oldName, const char* newName) { return (Retval) JIM_NAMESPACE_NAME::Jim_RenameCommand((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, oldName, newName); }
JIM_CEXPORT Jim_Cmd* Jim_GetCommand(Jim_InterpPtr interp,
                                    Jim_ObjPtr  objPtr, int flags) { return (Jim_Cmd*) JIM_NAMESPACE_NAME::Jim_GetCommand((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, flags); }
JIM_CEXPORT Retval Jim_SetVariable(Jim_InterpPtr interp,
                                   Jim_ObjPtr  nameObjPtr, Jim_ObjPtr  valObjPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_SetVariable((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nameObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )valObjPtr); }
JIM_CEXPORT Retval Jim_SetVariableStr(Jim_InterpPtr interp,
                                      const char* name, Jim_ObjPtr  objPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_SetVariableStr((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Retval Jim_SetGlobalVariableStr(Jim_InterpPtr interp,
                                            const char* name, Jim_ObjPtr  objPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_SetGlobalVariableStr((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Retval Jim_SetVariableStrWithStr(Jim_InterpPtr interp,
                                             const char* name, const char* val) { return (Retval) JIM_NAMESPACE_NAME::Jim_SetVariableStrWithStr((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, val); }
JIM_CEXPORT Retval Jim_SetVariableLink(Jim_InterpPtr interp,
                                       Jim_ObjPtr  nameObjPtr, Jim_ObjPtr  targetNameObjPtr,
                                       Jim_CallFrame* targetCallFrame) {
    return (Retval) JIM_NAMESPACE_NAME::Jim_SetVariableLink((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nameObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )(JIM_NAMESPACE_NAME::Jim_ObjPtr )targetNameObjPtr, (JIM_NAMESPACE_NAME::Jim_CallFrame*)targetCallFrame);
}
JIM_CEXPORT Jim_ObjPtr  Jim_MakeGlobalNamespaceName(Jim_InterpPtr interp,
                                                 Jim_ObjPtr  nameObjPtr) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_MakeGlobalNamespaceName((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nameObjPtr); }
JIM_CEXPORT Jim_ObjPtr  Jim_GetVariable(Jim_InterpPtr interp,
                                     Jim_ObjPtr  nameObjPtr, int flags) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_GetVariable((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nameObjPtr, flags); }
JIM_CEXPORT Jim_ObjPtr  Jim_GetGlobalVariable(Jim_InterpPtr interp,
                                           Jim_ObjPtr  nameObjPtr, int flags) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_GetGlobalVariable((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nameObjPtr, flags); }
JIM_CEXPORT Jim_ObjPtr  Jim_GetVariableStr(Jim_InterpPtr interp,
                                        const char* name, int flags) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_GetVariableStr((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, flags); }
JIM_CEXPORT Jim_ObjPtr  Jim_GetGlobalVariableStr(Jim_InterpPtr interp,
                                              const char* name, int flags) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_GetGlobalVariableStr((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, flags); }
JIM_CEXPORT Retval Jim_UnsetVariable(Jim_InterpPtr interp,
                                     Jim_ObjPtr  nameObjPtr, int flags) { return (Retval) JIM_NAMESPACE_NAME::Jim_UnsetVariable((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )nameObjPtr, flags); }

/* call frame */
JIM_CEXPORT Jim_CallFrame* Jim_GetCallFrameByLevel(Jim_InterpPtr interp,
                                                   Jim_ObjPtr  levelObjPtr) { return (Jim_CallFrame*) JIM_NAMESPACE_NAME::Jim_GetCallFrameByLevel((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )levelObjPtr); }

/* garbage collection */
JIM_CEXPORT int Jim_Collect(Jim_InterpPtr interp) { return JIM_NAMESPACE_NAME::Jim_Collect((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT void Jim_CollectIfNeeded(Jim_InterpPtr interp) { JIM_NAMESPACE_NAME::Jim_CollectIfNeeded((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }

/* index object */
JIM_CEXPORT Retval Jim_GetIndex(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                int* indexPtr /* on error set INT_MAX/-INT_MAX */) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetIndex((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, indexPtr); }

/* list object */
JIM_CEXPORT Jim_ObjPtr  Jim_NewListObj(Jim_InterpPtr interp,
                                    Jim_ObjConstArray elements, int len) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewListObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)elements, len); }
JIM_CEXPORT void Jim_ListInsertElements(Jim_InterpPtr interp,
                                        Jim_ObjPtr  listPtr, int listindex, int objc,
                                        Jim_ObjConstArray objVec) {
    JIM_NAMESPACE_NAME::Jim_ListInsertElements((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listPtr, listindex, objc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)objVec);
}
JIM_CEXPORT void Jim_ListAppendElement(Jim_InterpPtr interp,
                                       Jim_ObjPtr  listPtr, Jim_ObjPtr  objPtr) { JIM_NAMESPACE_NAME::Jim_ListAppendElement((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr ) objPtr); }
JIM_CEXPORT void Jim_ListAppendList(Jim_InterpPtr interp,
                                    Jim_ObjPtr  listPtr, Jim_ObjPtr  appendListPtr) { JIM_NAMESPACE_NAME::Jim_ListAppendList((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )appendListPtr); }
JIM_CEXPORT int Jim_ListLength(Jim_InterpPtr interp, Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_ListLength((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Retval Jim_ListIndex(Jim_InterpPtr interp, Jim_ObjPtr  listPrt,
                                 int listindex, Jim_ObjArray* objPtrPtr, int seterr) { return (Retval) JIM_NAMESPACE_NAME::Jim_ListIndex((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listPrt, listindex, (JIM_NAMESPACE_NAME::Jim_ObjArray*)objPtrPtr, seterr); }
JIM_CEXPORT Jim_ObjPtr  Jim_ListGetIndex(Jim_InterpPtr interp, Jim_ObjPtr  listPtr, int idx) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_ListGetIndex((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listPtr, idx); }
JIM_CEXPORT Jim_ObjPtr  Jim_ConcatObj(Jim_InterpPtr interp, int objc,
                                   Jim_ObjConstArray objv) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_ConcatObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, objc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)objv); }
JIM_CEXPORT Jim_ObjPtr  Jim_ListJoin(Jim_InterpPtr interp,
                                  Jim_ObjPtr  listObjPtr, const char* joinStr, int joinStrLen) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_ListJoin((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )listObjPtr, joinStr, joinStrLen); }

/* dict object */
JIM_CEXPORT Jim_ObjPtr  Jim_NewDictObj(Jim_InterpPtr interp,
                                    Jim_ObjConstArray elements, int len) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewDictObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)elements, len); }
JIM_CEXPORT Retval Jim_DictKey(Jim_InterpPtr interp, Jim_ObjPtr  dictPtr,
                               Jim_ObjPtr  keyPtr, Jim_ObjArray* objPtrPtr, int flags) { return (Retval) JIM_NAMESPACE_NAME::Jim_DictKey((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )dictPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr ) keyPtr, (JIM_NAMESPACE_NAME::Jim_ObjArray*)objPtrPtr, flags); }
JIM_CEXPORT Retval Jim_DictKeysVector(Jim_InterpPtr interp,
                                      Jim_ObjPtr  dictPtr, Jim_ObjConstArray keyv, int keyc,
                                      Jim_ObjArray* objPtrPtr, int flags) {
    return (Retval) JIM_NAMESPACE_NAME::Jim_DictKeysVector((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )dictPtr, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)keyv, keyc, (JIM_NAMESPACE_NAME::Jim_ObjArray*)objPtrPtr, flags);
}
JIM_CEXPORT Retval Jim_SetDictKeysVector(Jim_InterpPtr interp,
                                         Jim_ObjPtr  varNamePtr, Jim_ObjConstArray keyv, int keyc,
                                         Jim_ObjPtr  newObjPtr, int flags) {
    return (Retval) JIM_NAMESPACE_NAME::Jim_SetDictKeysVector((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )varNamePtr, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)keyv, keyc, (JIM_NAMESPACE_NAME::Jim_ObjPtr )newObjPtr, flags);
}
JIM_CEXPORT Retval Jim_DictPairs(Jim_InterpPtr interp,
                                 Jim_ObjPtr  dictPtr, Jim_ObjArray** objPtrPtr, int* len) { return (Retval) JIM_NAMESPACE_NAME::Jim_DictPairs((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )dictPtr, (JIM_NAMESPACE_NAME::Jim_ObjArray**)objPtrPtr, len); }
JIM_CEXPORT Retval Jim_DictAddElement(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                      Jim_ObjPtr  keyObjPtr, Jim_ObjPtr  valueObjPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_DictAddElement((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )keyObjPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )valueObjPtr); }

JIM_CEXPORT Retval Jim_DictMatchTypes(Jim_InterpPtr interp, Jim_ObjPtr  objPtr, Jim_ObjPtr  patternObj,
                                      int match_type, int return_types) { return (Retval) JIM_NAMESPACE_NAME::Jim_DictMatchTypes((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, (JIM_NAMESPACE_NAME::Jim_ObjPtr )patternObj, match_type, return_types); }
JIM_CEXPORT int Jim_DictSize(Jim_InterpPtr interp, Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_DictSize((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Retval Jim_DictInfo(Jim_InterpPtr interp, Jim_ObjPtr  objPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_DictInfo((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT Jim_ObjPtr  Jim_DictMerge(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_DictMerge((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, objc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)objv); }

/* return code object */
JIM_CEXPORT Retval Jim_GetReturnCode(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                     int* intPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetReturnCode((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, intPtr); }

/* expression object */
JIM_CEXPORT Retval Jim_EvalExpression(Jim_InterpPtr interp,
                                      Jim_ObjPtr  exprObjPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_EvalExpression((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )exprObjPtr); }
JIM_CEXPORT Retval Jim_GetBoolFromExpr(Jim_InterpPtr interp,
                                       Jim_ObjPtr  exprObjPtr, int* boolPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetBoolFromExpr((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )exprObjPtr, boolPtr); }

/* boolean object */
JIM_CEXPORT Retval Jim_GetBoolean(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                  int* booleanPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetBoolean((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, booleanPtr); }

/* integer object */
JIM_CEXPORT Retval Jim_GetWide(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                               jim_wide* widePtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetWide((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, widePtr); }
JIM_CEXPORT Retval Jim_GetLong(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                               long* longPtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetLong((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, longPtr); }
JIM_CEXPORT Jim_ObjPtr  Jim_NewIntObj(Jim_InterpPtr interp,
                                   jim_wide wideValue) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewIntObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, wideValue); }

/* double object */
JIM_CEXPORT Retval Jim_GetDouble(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                 double* doublePtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetDouble((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, doublePtr); }
//JIM_CEXPORT void Jim_SetDouble(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
//                               double doubleValue) { JIM_NAMESPACE_NAME::Jim_SetDouble((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, doubleValue); }
JIM_CEXPORT Jim_ObjPtr  Jim_NewDoubleObj(Jim_InterpPtr interp, double doubleValue) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewDoubleObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, doubleValue); }

/* commands utilities */
JIM_CEXPORT void Jim_WrongNumArgs(Jim_InterpPtr interp, int argc,
                                  Jim_ObjConstArray argv, const char* msg) { JIM_NAMESPACE_NAME::Jim_WrongNumArgs((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, argc, (JIM_NAMESPACE_NAME::Jim_ObjConstArray)argv, msg); }
JIM_CEXPORT Retval Jim_GetEnum(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                               const char* const* tablePtr, int* indexPtr, const char* name, int flags) { return (Retval) JIM_NAMESPACE_NAME::Jim_GetEnum((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, tablePtr, indexPtr, name, flags); }
JIM_CEXPORT Retval Jim_CheckShowCommands(Jim_InterpPtr interp, Jim_ObjPtr  objPtr,
                                         const char* const* tablePtr) { return (Retval) JIM_NAMESPACE_NAME::Jim_CheckShowCommands((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr, tablePtr); }
JIM_CEXPORT int Jim_ScriptIsComplete(Jim_InterpPtr interp,
                                     Jim_ObjPtr  scriptObj, char* stateCharPtr) { return JIM_NAMESPACE_NAME::Jim_ScriptIsComplete((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )scriptObj, stateCharPtr); }

/**
 * Find a matching name in the array of the given length.
 *
 * NULL entries are ignored.
 *
 * Returns the matching index if found, or -1 if not.
 */
JIM_CEXPORT int Jim_FindByName(const char* name, const char* const array[], size_t len) { return JIM_NAMESPACE_NAME::Jim_FindByName(name, array, len); }

/* package utilities */
JIM_CEXPORT void* Jim_GetAssocData(Jim_InterpPtr interp, const char* key) { return JIM_NAMESPACE_NAME::Jim_GetAssocData((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, key); }
JIM_CEXPORT Retval Jim_SetAssocData(Jim_InterpPtr interp, const char* key,
                                    Jim_InterpDeleteProc* delProc, void* data) { return (Retval) JIM_NAMESPACE_NAME::Jim_SetAssocData((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, key, (JIM_NAMESPACE_NAME::Jim_InterpDeleteProc*)delProc, data); }
JIM_CEXPORT Retval Jim_DeleteAssocData(Jim_InterpPtr interp, const char* key) { return (Retval) JIM_NAMESPACE_NAME::Jim_DeleteAssocData((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, key); }

/* Packages C API */
/* jim-package.c */
JIM_CEXPORT Retval Jim_PackageProvide(Jim_InterpPtr interp,
                                      const char* name, const char* ver, int flags) { return (Retval) JIM_NAMESPACE_NAME::Jim_PackageProvide((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, ver, flags); }
JIM_CEXPORT Retval Jim_PackageRequire(Jim_InterpPtr interp,
                                      const char* name, int flags) { return (Retval) JIM_NAMESPACE_NAME::Jim_PackageRequire((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, name, flags); }

/* error messages */
JIM_CEXPORT void Jim_MakeErrorMessage(Jim_InterpPtr interp) { JIM_NAMESPACE_NAME::Jim_MakeErrorMessage((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }

/* interactive mode */
JIM_CEXPORT Retval Jim_InteractivePrompt(Jim_InterpPtr interp) { return (Retval) JIM_NAMESPACE_NAME::Jim_InteractivePrompt((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT void Jim_HistoryLoad(const char* filename) { JIM_NAMESPACE_NAME::Jim_HistoryLoad(filename); }
JIM_CEXPORT void Jim_HistorySave(const char* filename) { JIM_NAMESPACE_NAME::Jim_HistorySave(filename); }
JIM_CEXPORT char* Jim_HistoryGetline(Jim_InterpPtr interp, const char* prompt) { return JIM_NAMESPACE_NAME::Jim_HistoryGetline((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, prompt); }
JIM_CEXPORT void Jim_HistorySetCompletion(Jim_InterpPtr interp, Jim_ObjPtr  commandObj) { JIM_NAMESPACE_NAME::Jim_HistorySetCompletion((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )commandObj); }
JIM_CEXPORT void Jim_HistoryAdd(const char* line) { JIM_NAMESPACE_NAME::Jim_HistoryAdd(line); }
JIM_CEXPORT void Jim_HistoryShow(void) { JIM_NAMESPACE_NAME::Jim_HistoryShow(); }

/* Misc */
JIM_CEXPORT Retval Jim_InitStaticExtensions(Jim_InterpPtr interp) { return (Retval) JIM_NAMESPACE_NAME::Jim_InitStaticExtensions((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }
JIM_CEXPORT Retval Jim_StringToWide(const char* str, jim_wide* widePtr, int base) { return (Retval) JIM_NAMESPACE_NAME::Jim_StringToWide(str, widePtr, base); }
JIM_CEXPORT int Jim_IsBigEndian(void) { return JIM_NAMESPACE_NAME::Jim_IsBigEndian(); }

/**
 * Returns 1 if a signal has been received while
 * in a catch -signal {} clause.
 */
JIM_CAPI_INLINE long_long Jim_CheckSignal(Jim_InterpPtr  i) { return JIM_NAMESPACE_NAME::Jim_CheckSignal((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }

/* jim-load.c */
JIM_CEXPORT Retval Jim_LoadLibrary(Jim_InterpPtr interp, const char* pathName) { return (Retval) JIM_NAMESPACE_NAME::Jim_LoadLibrary((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, pathName); }
//JIM_CEXPORT void Jim_FreeLoadHandles(Jim_InterpPtr interp) { JIM_NAMESPACE_NAME::Jim_FreeLoadHandles((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp); }

/* jim-aio.c */
JIM_CEXPORT FILE* Jim_AioFilehandle(Jim_InterpPtr interp, Jim_ObjPtr  command) { return JIM_NAMESPACE_NAME::Jim_AioFilehandle((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, (JIM_NAMESPACE_NAME::Jim_ObjPtr )command); }

/* type inspection - avoid where possible */
JIM_CEXPORT int Jim_IsDict(Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_IsDict((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }
JIM_CEXPORT int Jim_IsList(Jim_ObjPtr  objPtr) { return JIM_NAMESPACE_NAME::Jim_IsList((JIM_NAMESPACE_NAME::Jim_ObjPtr )objPtr); }

JIM_CAPI_INLINE void Jim_SetResult(Jim_InterpPtr  i, Jim_ObjPtr  o) { JIM_NAMESPACE_NAME::Jim_SetResult((JIM_NAMESPACE_NAME::Jim_InterpPtr)i, (JIM_NAMESPACE_NAME::Jim_ObjPtr )o); }
JIM_CAPI_INLINE void Jim_InterpIncrProcEpoch(Jim_InterpPtr  i) { JIM_NAMESPACE_NAME::Jim_InterpIncrProcEpoch((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }
JIM_CAPI_INLINE void Jim_SetResultString(Jim_InterpPtr  i, const char* s, int l /* -1 means strlen(s) */) { JIM_NAMESPACE_NAME::Jim_SetResultString((JIM_NAMESPACE_NAME::Jim_InterpPtr)i, s, l); }
JIM_CAPI_INLINE void Jim_SetResultInt(Jim_InterpPtr  i, long_long intval) { JIM_NAMESPACE_NAME::Jim_SetResultInt((JIM_NAMESPACE_NAME::Jim_InterpPtr)i, intval); }
JIM_CAPI_INLINE void Jim_SetResultBool(Jim_InterpPtr  i, long_long b) { JIM_NAMESPACE_NAME::Jim_SetResultBool((JIM_NAMESPACE_NAME::Jim_InterpPtr)i, b); }
JIM_CAPI_INLINE void Jim_SetEmptyResult(Jim_InterpPtr  i) { JIM_NAMESPACE_NAME::Jim_SetEmptyResult((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }
JIM_CAPI_INLINE Jim_ObjPtr  Jim_GetResult(Jim_InterpPtr  i) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_GetResult((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }
JIM_CAPI_INLINE void* Jim_CmdPrivData(Jim_InterpPtr  i) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_CmdPrivData((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }

JIM_CAPI_INLINE Jim_ObjPtr  Jim_NewEmptyStringObj(Jim_InterpPtr  i) { return (Jim_ObjPtr ) JIM_NAMESPACE_NAME::Jim_NewEmptyStringObj((JIM_NAMESPACE_NAME::Jim_InterpPtr)i); }
JIM_CAPI_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter) { JIM_NAMESPACE_NAME::Jim_FreeHashTableIterator((JIM_NAMESPACE_NAME::Jim_HashTableIterator*)iter); }

/* from jimiocompat.cpp */
/**
 * Set an error result based on errno and the given message.
 */
void Jim_SetResultErrno(Jim_InterpPtr interp, const char* msg) { JIM_NAMESPACE_NAME::Jim_SetResultErrno((JIM_NAMESPACE_NAME::Jim_InterpPtr)interp, msg); }

/**
 * Opens the file for writing (and appending if append is true).
 * Returns the file descriptor, or -1 on failure.
 */
int Jim_OpenForWrite(const char* filename, int append) { return JIM_NAMESPACE_NAME::Jim_OpenForWrite(filename, append); }

/**
 * Opens the file for reading.
 * Returns the file descriptor, or -1 on failure.
 */
int Jim_OpenForRead(const char* filename) { return JIM_NAMESPACE_NAME::Jim_OpenForRead(filename); }

/**
 * Unix-compatible errno
 */
int Jim_Errno(void) { return JIM_NAMESPACE_NAME::Jim_Errno(); }