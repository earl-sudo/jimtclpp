#pragma once
// NOTE: this is designed to be include in the "Jim" namespace

#include <stddef.h>

#ifndef JIM_EXPORT
#  define JIM_EXPORT 
#endif

#if defined(__GNUC__) && !defined(PRJ_OS_MACOS)
// On MacOS this is not defined and causes warnings. #BuildSpecific
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored  "-Wclass-memaccess"
#endif

/* Memory allocation */
JIM_EXPORT void* Jim_Alloc(int sizeInBytes);
JIM_EXPORT void* Jim_Realloc(void* ptr, int sizeInBytes);
JIM_EXPORT void Jim_Free(void* ptr);
JIM_EXPORT char* Jim_StrDup(const char* s);
JIM_EXPORT char* Jim_StrDupLen(const char* s, int l /* num 1 byte characters */);

/* Type specific allocators. */

template<typename T>
T* Jim_TAllocZ(int N = 1, const char* typeName MAYBE_USED = nullptr) {
    auto v = (T*) Jim_Alloc(N * sizeof(T)); memset(v, 0, sizeof(T) * N);
    PRJ_TRACEMEM_ALLOC(typeName, (N * sizeof(T)), (void*) v);
    return v;
}

template<typename T>
T* Jim_TAlloc(int N = 1, const char* typeName MAYBE_USED = nullptr) {
    auto v = (T*) Jim_Alloc(N * sizeof(T));
    PRJ_TRACEMEM_ALLOC(typeName, (N * sizeof(T)), (void*) v);
    return v;
}

template<typename T>
void Jim_TFree(T*& p, const char* typeName MAYBE_USED = nullptr) {
    PRJ_TRACEMEM_FREE(typeName, (void*) p);
    Jim_Free(p); p = nullptr;
}

template<typename T>
void Jim_TFreeNR(T* p, const char* typeName MAYBE_USED = nullptr) {
    PRJ_TRACEMEM_FREE(typeName, (void*) p);
    Jim_Free(p);
}

template<typename T>
T* Jim_TRealloc(T* ptr, int N, const char* typeName MAYBE_USED = nullptr) {
    auto ret = (T*) Jim_Realloc(ptr, N * sizeof(T));
    PRJ_TRACEMEM_REALLOC(typeName, N * sizeof(T), (void*) ptr, (void*) ret);
    return ret;
}

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_Jim_Interp          Jim_TAllocZ<Jim_Interp>(1,"Jim_Interp")
#define free_Jim_Interp(ptr)    Jim_TFree<Jim_Interp>(ptr,"Jim_Interp")
#define new_Jim_Stack           Jim_TAlloc<Jim_Stack>(1,"Jim_Interp")
#define free_Jim_Stack(ptr)     Jim_TFree<Jim_Stack>(ptr,"Jim_Stack")
#define new_Jim_CallFrame       Jim_TAllocZ<Jim_CallFrame>(1,"Jim_Stack")
#define free_Jim_CallFrame(ptr) Jim_TFree<Jim_CallFrame>(ptr,"Jim_CallFrame")
#define new_Jim_HashTable       Jim_TAlloc<Jim_HashTable>(1,"Jim_HashTable")
#define free_Jim_HashTable(ptr) Jim_TFree<Jim_HashTable>(ptr,"Jim_HashTable")
#define new_Jim_HashTableIterator Jim_TAlloc<Jim_HashTableIterator>(1,"Jim_HashTableIterator")
#define free_Jim_HashTableIterator(ptr) Jim_TFree<Jim_HashTableIterator>(ptr,"Jim_HashTableIterator") // #Review never called
#define new_Jim_Var             Jim_TAlloc<Jim_Var>(1,"Jim_Var")
#define free_Jim_Var(ptr)       Jim_TFree<Jim_Var>(ptr,"Jim_Var")
#define new_Jim_HashEntry       Jim_TAlloc<Jim_HashEntry>(1,"Jim_HashEntry")
#define free_Jim_HashEntry(ptr) Jim_TFree<Jim_HashEntry>(ptr, "Jim_HashEntry")
#define new_Jim_ObjArray(sz)    Jim_TAlloc<Jim_ObjArray>(sz,"Jim_ObjArray")
#define new_Jim_ObjArrayZ(sz)   Jim_TAllocZ<Jim_ObjArray>(sz,"Jim_ObjArray")
#define free_Jim_ObjArray(ptr)  Jim_TFree<Jim_ObjArray>(ptr,"Jim_ObjArray")
#define realloc_Jim_ObjArray(orgPtr, newSz) Jim_TRealloc<Jim_ObjArray>(orgPtr, newSz, "Jim_ObjArray")
#define new_Jim_Obj             Jim_TAllocZ<Jim_Obj>(1,"Jim_Obj")
#define free_Jim_Obj(ptr)       Jim_TFree<Jim_Obj>(ptr, "Jim_Obj")
#define new_CharArray(sz)       Jim_TAlloc<char>(sz, "CharArray")
#define new_CharArrayZ(sz)      Jim_TAllocZ<char>(sz, "CharArray")
#define free_CharArray(ptr)     Jim_TFree<char>(ptr, "CharArray")
#define realloc_CharArray(orgPtr, newSz) Jim_TRealloc<char>(orgPtr, newSz, "CharArray")
#define new_Jim_Cmd             Jim_TAllocZ<Jim_Cmd>(1,"Jim_Cmd")
#define free_Jim_Cmd(ptr)       Jim_TFree<Jim_Cmd>(ptr,"Jim_Cmd")

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif