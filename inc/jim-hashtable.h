#pragma once

#include <stddef.h>

#include <jim-base.h>

BEGIN_JIM_NAMESPACE
#include <jim-forwards.h>
#include <jim-alloc.h>
END_JIM_NAMESPACE


BEGIN_JIM_NAMESPACE

struct Jim_HashEntry;
struct Jim_HashTableType;
struct Jim_HashTable;

typedef Jim_HashEntry* Jim_HashEntryPtr;
typedef Jim_HashEntry* Jim_HashEntryArray;
typedef Jim_HashTable* Jim_HashTablePtr;

struct Jim_HashEntry {
private:
    void* key_;
    // Null terminated single link list.
    Jim_HashEntryPtr next_;
    union {
        void* val_;
        int intval_; // #UNUSED
    } u;
public:
    // val_
    inline void* getVal() const { return u.val_; }
    inline void setVal(void* v) { u.val_ = v; }
    // next_
    inline Jim_HashEntryPtr  next() const { return next_; }
    inline void setNext(Jim_HashEntryPtr o) { next_ = o; }
    // key_
    inline const char* keyAsStr() const { return (const char*) key_; }
    inline void* keyAsVoid() const { return key_; }
    inline Jim_ObjPtr  keyAsObj() const { return (Jim_ObjPtr) key_; }
    inline void setKey(void* keyD) { key_ = keyD; }
};

struct Jim_HashTableType {
    unsigned_int(*hashFunction)(const void* key) = NULL;
    void* (*keyDup)(void* privdata, const void* key) = NULL;
    void* (*valDup)(void* privdata, const void* obj) = NULL;
    int (*keyCompare)(void* privdata, const void* key1, const void* key2) = NULL;
    void (*keyDestructor)(void* privdata, void* key) = NULL;
    void (*valDestructor)(void* privdata, void* obj) = NULL;
};

struct Jim_HashTable {
private:
    const char* typeName_ = "unknown"; // Assume static string
        // typeName_ current-possible: staticVars_, variables, refMark, commands, references, assocData, packages, dict
    const Jim_HashTableType* type_ = NULL; /* not used */
    void* privdata_ = NULL;

    unsigned_int size_ = 0;
    unsigned_int sizemask_ = 0;
    unsigned_int collisions_ = 0; // #TODO collisions_ used?
    unsigned_int uniq_ = 0;
    unsigned_int used_ = 0;
    Jim_HashEntryArray* table_ = NULL;

public:
    // uniq_
    inline unsigned_int uniq() const { return uniq_; }
    inline void setUniq(unsigned_int v) { uniq_ = v; }
    // collisions_
    inline unsigned_int collisions() const { return collisions_; }
    inline void setCollisions(unsigned_int v) { collisions_ = v; }
    // used_
    inline unsigned_int used() const { return used_; }
    inline void incrUsed() { used_++; }
    inline void decrUsed() { used_--; }
    inline void setUsed(unsigned_int v) { used_ = v; }
    // sizemask_
    inline unsigned_int sizemask() const { return sizemask_; }
    inline void setSizemask(unsigned_int v) { sizemask_ = v; }
    // size_
    inline unsigned_int size() const { return size_; }
    inline void setSize(unsigned_int v) { size_ = v; }
    // privdata_
    inline void* privdata() { return privdata_; }
    inline void setPrivdata(void* o) { privdata_ = o; }
    // tokenType_
    inline const Jim_HashTableType* type() const { return type_; }
    inline void setType(const Jim_HashTableType* o) { type_ = o; }
    // typeName_
    inline void setTypeName(const char* n) { typeName_ = n; }
    inline const char* getTypeName() const { return typeName_; }
    // table_
    inline Jim_HashEntryPtr getEntry(unsigned_int i) { return table_[i]; }
    inline void setEntry(unsigned_int i, Jim_HashEntryPtr o) { table_[i] = o; }
    inline void setTable(Jim_HashEntryArray* tableD) { table_ = tableD; }
    inline Jim_HashEntryArray* table() { return table_; }
    inline bool tableAllocated() const { return table_ != NULL; }
    void freeTable(); // #FreeF 
};

struct Jim_HashTableIterator {
private:
    Jim_HashTablePtr ht_ = NULL;
    Jim_HashEntryPtr entry_ = NULL;
    Jim_HashEntryPtr nextEntry_ = NULL;
    int index_ = 0;
public:
    inline void setup(Jim_HashTablePtr htD, Jim_HashEntryPtr entryD, Jim_HashEntryPtr nextEntryD, int indexD) {
        ht_ = htD; entry_ = entryD; nextEntry_ = nextEntryD; index_ = indexD;
    }
    inline int index() const { return index_; }
    inline void indexIncr() { index_++; }
    inline Jim_HashEntryPtr  entry() const { return entry_; }
    inline void setEntry(Jim_HashEntryPtr o) { entry_ = o; }
    inline Jim_HashEntryPtr   nextEntry() const { return nextEntry_; }
    inline void setNextEntry(Jim_HashEntryPtr o) { nextEntry_ = o; }
    inline Jim_HashTablePtr ht() { return ht_; }
};

/* This is the initial size_ of every hash table */
enum {
    JIM_HT_INITIAL_SIZE = 16 // #MagicNum
};

CHKRET JIM_EXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht, const Jim_HashTableType* type, void* privdata); // #ctor_like Jim_HashTable
JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size);
CHKRET JIM_EXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void* key, void* val);
CHKRET JIM_EXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht, const void* key, void* val);
CHKRET JIM_EXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht, const void* key); // #dtor_like Jim_HashTable
CHKRET JIM_EXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht); // #dtor_like Jim_HashTable
CHKRET JIM_EXPORT Jim_HashEntryPtr  Jim_FindHashEntry(Jim_HashTablePtr ht, const void* key);
JIM_EXPORT void Jim_ResizeHashTable(Jim_HashTablePtr ht);
CHKRET JIM_EXPORT Jim_HashTableIterator* Jim_GetHashTableIterator(Jim_HashTablePtr ht);
CHKRET JIM_EXPORT Jim_HashEntryPtr  Jim_NextHashEntry(Jim_HashTableIterator* iter);
CHKRET JIM_EXPORT const char* Jim_KeyAsStr(Jim_HashEntryPtr  he);
CHKRET JIM_EXPORT const void* Jim_KeyAsVoid(Jim_HashEntryPtr  he);

void JimInitHashTableIterator(Jim_HashTablePtr ht, Jim_HashTableIterator* iter); // #ctor_like

//END_JIM_HT_NAMESPACE
END_JIM_NAMESPACE
