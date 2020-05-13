#pragma once

#include <jim.h>

BEGIN_JIM_NAMESPACE


JIM_API_INLINE void Jim_FreeEntryVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr entry) {
    if ((ht)->type()->valDestructor)
        (ht)->type()->valDestructor((ht)->privdata(), (entry)->voidValue());
}

JIM_API_INLINE void Jim_SetHashVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, void* _val_) {
    if ((ht)->type()->valDup)
        (entry)->u.val_ = (ht)->type()->valDup((ht)->privdata(), (_val_));
    else
        (entry)->u.val_ = (_val_);

}

JIM_API_INLINE void Jim_FreeEntryKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry) {
    if ((ht)->type()->keyDestructor)
        (ht)->type()->keyDestructor((ht)->privdata(), (entry)->keyAsVoid());
}

JIM_API_INLINE void Jim_SetHashKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, const void* _key_) {
    if ((ht)->type()->keyDup)
        (entry)->key_ = (ht)->type()->keyDup((ht)->privdata(), (_key_));
    else
        (entry)->key_ = (void *) (_key_);
}

JIM_API_INLINE int Jim_CompareHashKeys(Jim_HashTablePtr  ht, const void* key1, const void* key2) {
    return (((ht)->type()->keyCompare) ? \
        (ht)->type()->keyCompare((ht)->privdata(), (key1), (key2)) : \
            (key1) == (key2)); 
}

JIM_API_INLINE unsigned_int Jim_HashKey(Jim_HashTablePtr  ht, const void* key) {
    return ((ht)->type()->hashFunction(key) + (ht)->uniq());
}

JIM_API_INLINE void* Jim_GetHashEntryKey(Jim_HashEntryPtr  he) { return ((he)->keyAsVoid()); }
JIM_API_INLINE void* Jim_GetHashEntryVal(Jim_HashEntryPtr  he) { return ((he)->voidValue()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableCollisions(Jim_HashTablePtr  ht) { return ((ht)->collisions()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableSize(Jim_HashTablePtr  ht) { return ((ht)->size()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableUsed(Jim_HashTablePtr  ht) { return ((ht)->used()); }

JIM_API_INLINE void Jim_IncrRefCount(Jim_ObjPtr  objPtr) { objPtr->incrRefCount(); } 
JIM_API_INLINE void Jim_DecrRefCount(Jim_InterpPtr  interp, Jim_ObjPtr  objPtr) { 
    if (objPtr->decrRefCount() <= 0) 
        Jim_FreeObj(interp, objPtr); 
}
JIM_API_INLINE int Jim_IsShared(Jim_ObjPtr  objPtr) { return ((objPtr)->refCount() > 1); }

/* Get the internal representation pointer */
/* EJ #define Jim_GetIntRepPtr(o) (o)->internalRep.ptr */
JIM_API_INLINE void* Jim_GetIntRepPtr(Jim_ObjPtr  o) { return (o)->getVoidPtr(); }

JIM_API_INLINE void Jim_FreeIntRep(Jim_InterpPtr  i, Jim_ObjPtr  o) {
    if ((o)->typePtr() && (o)->typePtr()->freeIntRepProc)
        (o)->typePtr()->freeIntRepProc(i, o);
}

JIM_API_INLINE long Jim_GetId(Jim_InterpPtr  i) { return i->incrId(); }

JIM_API_INLINE long_long Jim_CheckSignal(Jim_InterpPtr  i) { return ((i)->signal_level() && (i)->getSigmask()); }

JIM_API_INLINE void Jim_SetResult(Jim_InterpPtr  i, Jim_ObjPtr  o) {
    Jim_ObjPtr _resultObjPtr_ = (o);
    Jim_IncrRefCount(_resultObjPtr_);
    Jim_DecrRefCount(i, (i)->result());
    (i)->setResult(_resultObjPtr_);
}
JIM_API_INLINE void Jim_InterpIncrProcEpoch(Jim_InterpPtr  i) { (i)->procEpoch((i)->procEpoch() + 1); }
JIM_API_INLINE void Jim_SetResultString(Jim_InterpPtr  i, const char* s, int l) { Jim_SetResult(i, Jim_NewStringObj(i, s, l)); }
JIM_API_INLINE void Jim_SetResultInt(Jim_InterpPtr  i, long_long intval) { Jim_SetResult(i, Jim_NewIntObj(i, intval)); }
JIM_API_INLINE void Jim_SetResultBool(Jim_InterpPtr  i, long_long b) { Jim_SetResultInt(i, b); }
JIM_API_INLINE void Jim_SetEmptyResult(Jim_InterpPtr  i) { Jim_SetResult(i, (i)->emptyObj()); }
JIM_API_INLINE Jim_ObjPtr  Jim_GetResult(Jim_InterpPtr  i) { return (i)->result(); }
JIM_API_INLINE void* Jim_CmdPrivData(Jim_InterpPtr  i) { return ((i)->cmdPrivData()); }

JIM_API_INLINE Jim_ObjPtr  Jim_NewEmptyStringObj(Jim_InterpPtr  i) { return Jim_NewStringObj(i, "", 0); }
JIM_API_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter) { Jim_Free(iter); }

END_JIM_NAMESPACE
