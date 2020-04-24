#pragma once

#include "jim.h"

BEGIN_JIM_NAMESPACE


JIM_API_INLINE void Jim_FreeEntryVal(Jim_HashTable * ht, Jim_HashEntry *entry) {
    if ((ht)->type()->valDestructor)
        (ht)->type()->valDestructor((ht)->privdata(), (entry)->voidValue());
}

JIM_API_INLINE void Jim_SetHashVal(Jim_HashTable* ht, Jim_HashEntry* entry, void* _val_) {
    if ((ht)->type()->valDup)
        (entry)->u.val_ = (ht)->type()->valDup((ht)->privdata(), (_val_));
    else
        (entry)->u.val_ = (_val_);

}

JIM_API_INLINE void Jim_FreeEntryKey(Jim_HashTable* ht, Jim_HashEntry* entry) {
    if ((ht)->type()->keyDestructor)
        (ht)->type()->keyDestructor((ht)->privdata(), (entry)->keyAsVoid());
}

JIM_API_INLINE void Jim_SetHashKey(Jim_HashTable* ht, Jim_HashEntry* entry, const void* _key_) {
    if ((ht)->type()->keyDup)
        (entry)->key_ = (ht)->type()->keyDup((ht)->privdata(), (_key_));
    else
        (entry)->key_ = (void *) (_key_);
}

JIM_API_INLINE int Jim_CompareHashKeys(Jim_HashTable* ht, const void* key1, const void* key2) {
    return (((ht)->type()->keyCompare) ? \
        (ht)->type()->keyCompare((ht)->privdata(), (key1), (key2)) : \
            (key1) == (key2));
}

JIM_API_INLINE unsigned_int Jim_HashKey(Jim_HashTable* ht, const void* key) {
    return ((ht)->type()->hashFunction(key) + (ht)->uniq());
}

JIM_API_INLINE void* Jim_GetHashEntryKey(Jim_HashEntry* he) { return ((he)->keyAsVoid()); }
JIM_API_INLINE void* Jim_GetHashEntryVal(Jim_HashEntry* he) { return ((he)->voidValue()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableCollisions(Jim_HashTable* ht) { return ((ht)->collisions()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableSize(Jim_HashTable* ht) { return ((ht)->size()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableUsed(Jim_HashTable* ht) { return ((ht)->used()); }

JIM_API_INLINE void Jim_IncrRefCount(Jim_Obj* objPtr) { ++(objPtr)->refCount_; }
JIM_API_INLINE void Jim_DecrRefCount(Jim_Interp* interp, Jim_Obj* objPtr) { if (--(objPtr)->refCount_ <= 0) Jim_FreeObj(interp, objPtr); }
JIM_API_INLINE int Jim_IsShared(Jim_Obj* objPtr) { return ((objPtr)->refCount() > 1); }

/* Get the internal representation pointer */
/* EJ #define Jim_GetIntRepPtr(o) (o)->internalRep.ptr */
JIM_API_INLINE void* Jim_GetIntRepPtr(Jim_Obj* o) { return (o)->getVoidPtr(); }

/* Set the internal representation pointer */
/* EJ #define Jim_SetIntRepPtr(o, p) \
    (o)->internalRep.ptr = (p) */
JIM_API_INLINE void Jim_SetIntRepPtr(Jim_Obj* o, void* p) {
    (o)->internalRep.ptr_ = (p);
}

JIM_API_INLINE void Jim_FreeIntRep(Jim_Interp* i, Jim_Obj* o) {
    if ((o)->typePtr() && (o)->typePtr()->freeIntRepProc)
        (o)->typePtr()->freeIntRepProc(i, o);
}

JIM_API_INLINE long Jim_GetId(Jim_Interp* i) { return (++(i)->id_); }

JIM_API_INLINE long_long Jim_CheckSignal(Jim_Interp* i) { return ((i)->signal_level_ && (i)->sigmask_); }

JIM_API_INLINE void Jim_SetResult(Jim_Interp* i, Jim_Obj* o) {
    Jim_Obj *_resultObjPtr_ = (o);
    Jim_IncrRefCount(_resultObjPtr_);
    Jim_DecrRefCount(i, (i)->result());
    (i)->result_ = _resultObjPtr_;
}
JIM_API_INLINE void Jim_InterpIncrProcEpoch(Jim_Interp* i) { (i)->procEpoch((i)->procEpoch() + 1); }
JIM_API_INLINE void Jim_SetResultString(Jim_Interp* i, const char* s, int l) { Jim_SetResult(i, Jim_NewStringObj(i, s, l)); }
JIM_API_INLINE void Jim_SetResultInt(Jim_Interp* i, long_long intval) { Jim_SetResult(i, Jim_NewIntObj(i, intval)); }
JIM_API_INLINE void Jim_SetResultBool(Jim_Interp* i, long_long b) { Jim_SetResultInt(i, b); }
JIM_API_INLINE void Jim_SetEmptyResult(Jim_Interp* i) { Jim_SetResult(i, (i)->emptyObj()); }
JIM_API_INLINE Jim_Obj* Jim_GetResult(Jim_Interp* i) { return (i)->result(); }
JIM_API_INLINE void* Jim_CmdPrivData(Jim_Interp* i) { return ((i)->cmdPrivData_); }

JIM_API_INLINE Jim_Obj* Jim_NewEmptyStringObj(Jim_Interp* i) { return Jim_NewStringObj(i, "", 0); }
JIM_API_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter) { Jim_Free(iter); }

END_JIM_NAMESPACE
