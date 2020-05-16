#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include <prj_trace.h>
#include <jim-hashtable.h>
#include <jim.h>

BEGIN_JIM_NAMESPACE

extern int g_JIM_RANDOMISE_HASH_VAL;

JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size);
static unsigned_int Jim_GenHashFunction(const_unsigned_char* buf, int len);
//static unsigned_int Jim_IntHashFunction(unsigned_int key);
void JimInitHashTableIterator(Jim_HashTablePtr ht, Jim_HashTableIterator* iter);
JIM_EXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht, const Jim_HashTableType* type, void* privdata);
JIM_EXPORT void Jim_ResizeHashTable(Jim_HashTablePtr ht);
JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size);
JIM_EXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void* key, void* val);
JIM_EXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht, const void* key, void* val);
JIM_EXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht, const void* key);
JIM_EXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht);
JIM_EXPORT Jim_HashEntryPtr Jim_FindHashEntry(Jim_HashTablePtr ht, const void* key);
JIM_EXPORT Jim_HashTableIterator* Jim_GetHashTableIterator(Jim_HashTablePtr ht);
JIM_EXPORT Jim_HashEntryPtr Jim_NextHashEntry(Jim_HashTableIterator* iter);
static void JimExpandHashTableIfNeeded(Jim_HashTablePtr ht);
static unsigned_int JimHashTableNextPower(unsigned_int size);
static Jim_HashEntryPtr JimInsertHashEntry(Jim_HashTablePtr ht, const void* key, int replace);
static unsigned_int JimStringCopyHTHashFunction(const void* key);
static void* JimStringCopyHTDup(void* privdata, const void* key);
static int JimStringCopyHTKeyCompare(void* privdata, const void* key1, const void* key2);
static void JimStringCopyHTKeyDestructor(void* privdata, void* key);

inline void Jim_HashTable::freeTable() { Jim_TFree<Jim_HashEntryArray>(table_, "Jim_HashEntryArray"); } // #FreeF 

/* -------------------------- hash functions -------------------------------- */

/* Thomas Wang's 32 bit Mix Function */
#if 0
static unsigned_int Jim_IntHashFunction(unsigned_int key) { // #UNUSED #REMOVE
    PRJ_TRACE;
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}
#endif

/* Generic hash function (we are using to multiply by 9 and add the byte
 * as Tcl) */
static unsigned_int Jim_GenHashFunction(const_unsigned_char* buf, int len) {
    PRJ_TRACE;
    unsigned_int h = 0;

    while (len--)
        h += (h << 3) + *buf++;
    return h;
}

/* reset a hashtable already initialized */
static void JimResetHashTable(Jim_HashTablePtr ht) {
    PRJ_TRACE;
    ht->setTable(NULL);
    ht->setSize(0);
    ht->setSizemask(0);
    ht->setUsed(0);
    ht->setCollisions(0);
    if (g_JIM_RANDOMISE_HASH_VAL) {
        /* This is initialized to a random value to avoid a hash collision attack.
         * See: n.runs-SA-2011.004
         */
        ht->setUniq((rand() ^ static_cast<unsigned_int>(time(NULL)) ^ clock())); // #NonPortFunc
    } else {
        ht->setUniq(0);
    }
}

void JimInitHashTableIterator(Jim_HashTablePtr ht, Jim_HashTableIterator* iter) {
    PRJ_TRACE;
    iter->setup(ht, NULL, NULL, -1);
}

/* Initialize the hash table */
JIM_EXPORT Retval Jim_InitHashTable(Jim_HashTablePtr ht, const Jim_HashTableType* type, void* privdata) {
    PRJ_TRACE;
    JimResetHashTable(ht);
    ht->setType(type);
    ht->setPrivdata(privdata);
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_CREATE, __FUNCTION__, ht);
    return JIM_RETURNS::JIM_OK;
}

/* Resize the table to the minimal size_ that contains all the elements,
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
JIM_EXPORT void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size) {
    PRJ_TRACE;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_RESIZE_PRE, __FUNCTION__, ht);

    Jim_HashTable n;            /* the new hashtable */
    unsigned_int realsize = JimHashTableNextPower(size), i;

    /* the size_ is invalid if it is smaller than the number of
     * elements already inside the hashtable */
    if (size <= ht->used())
        return;

    Jim_InitHashTable(&n, ht->type(), ht->privdata());
    n.setSize(realsize);
    n.setSizemask(realsize - 1);
    n.setTable(Jim_TAllocZ<Jim_HashEntryArray>(realsize, "Jim_HashEntryArray")); // #AllocF 
    /* Keep the same 'uniq' as the original */
    n.setUniq(ht->uniq());
    n.setTypeName(ht->getTypeName());

    /* Initialize all the pointers to NULL */
    //memset(n.table_, 0, realsize * sizeof(Jim_HashEntryArray));

    /* Copy all the elements from the old to the new table:
     * note that if the old hash table is empty ht->used is zero,
     * so Jim_ExpandHashTable just creates an empty hash table. */
    n.setUsed(ht->used());
    for (i = 0; ht->used() > 0; i++) {
        Jim_HashEntryPtr he; Jim_HashEntryPtr nextHe;

        if (ht->getEntry(i) == NULL)
            continue;

        /* For each hash entry on this slot... */
        he = ht->getEntry(i);
        while (he) {
            unsigned_int h;

            nextHe = he->next();
            /* Get the new element index */
            h = Jim_HashKey(ht, he->keyAsVoid()) & n.sizemask();
            he->setNext(n.getEntry(h));
            n.setEntry(h, he);
            ht->decrUsed();
            /* Pass to the next_ element */
            he = nextHe;
        }
    }
    assert(ht->used() == 0);
    ht->freeTable();

    /* Remap the new hashtable in the old */
    *ht = n;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_RESIZE_POST, __FUNCTION__, ht);
}

/* Add an element to the target hash table */
JIM_EXPORT Retval Jim_AddHashEntry(Jim_HashTablePtr ht, const void* key, void* val) {
    PRJ_TRACE;
    Jim_HashEntryPtr entry;

    /* Get the index of the new element, or -1 if
     * the element already exists. */
    entry = JimInsertHashEntry(ht, key, 0);
    if (entry == NULL)
        return JIM_RETURNS::JIM_ERR;

    /* Set the hash entry fields. */
    Jim_SetHashKey(ht, entry, key);
    Jim_SetHashVal(ht, entry, val);
    return JIM_RETURNS::JIM_OK;
}

/* Add an element, discarding the old if the key already exists */
JIM_EXPORT int Jim_ReplaceHashEntry(Jim_HashTablePtr ht, const void* key, void* val) {
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
            void* newval = ht->type()->valDup(ht->privdata(), val);
            ht->type()->valDestructor(ht->privdata(), entry->getVal());
            entry->setVal(newval);
        } else {
            Jim_FreeEntryVal(ht, entry);
            Jim_SetHashVal(ht, entry, val);
        }
        existed = 1;
    } else {
        /* Doesn't exist, so set the key */
        Jim_SetHashKey(ht, entry, key);
        Jim_SetHashVal(ht, entry, val);
        existed = 0;
    }

    return existed;
}

/* Search and remove an element */
JIM_EXPORT Retval Jim_DeleteHashEntry(Jim_HashTablePtr ht, const void* key) {
    PRJ_TRACE;
    unsigned_int h;
    Jim_HashEntryPtr  he; Jim_HashEntryPtr prevHe;

    if (ht->used() == 0)
        return JIM_RETURNS::JIM_ERR;
    h = Jim_HashKey(ht, key) & ht->sizemask();
    he = ht->getEntry(h);

    prevHe = NULL;
    while (he) {
        if (Jim_CompareHashKeys(ht, key, he->keyAsVoid())) {
            /* Unlink the element from the list */
            if (prevHe)
                prevHe->setNext(he->next());
            else
                ht->setEntry(h, he->next());
            Jim_FreeEntryKey(ht, he);
            Jim_FreeEntryVal(ht, he);
            free_Jim_HashEntry(he); // #FreeF 
            ht->decrUsed();
            return JIM_RETURNS::JIM_OK;
        }
        prevHe = he;
        he = he->next();
    }
    return JIM_RETURNS::JIM_ERR;             /* not found */
}

/* Destroy an entire hash table and leave it ready for reuse */
JIM_EXPORT Retval Jim_FreeHashTable(Jim_HashTablePtr ht) {
    PRJ_TRACE;
    PRJ_TRACE_HT(::prj_trace::ACTION_HT_DELETE, __FUNCTION__, ht);
    unsigned_int i;

    /* Free all the elements */
    for (i = 0; ht->used() > 0; i++) {
        Jim_HashEntryPtr  he; Jim_HashEntryPtr nextHe;

        if ((he = ht->getEntry(i)) == NULL)
            continue;
        while (he) {
            nextHe = he->next();
            Jim_FreeEntryKey(ht, he);
            Jim_FreeEntryVal(ht, he);
            free_Jim_HashEntry(he); // #FreeF 
            ht->decrUsed();
            he = nextHe;
        }
    }
    /* Free the table and the allocated cache structure */
    ht->freeTable();

    /* Re-initialize the table */
    JimResetHashTable(ht);
    return JIM_RETURNS::JIM_OK;              /* never fails */
}

JIM_EXPORT Jim_HashEntryPtr Jim_FindHashEntry(Jim_HashTablePtr ht, const void* key) {
    PRJ_TRACE;
    Jim_HashEntryPtr he;
    unsigned_int h;

    if (ht->used() == 0)
        return NULL;
    h = Jim_HashKey(ht, key) & ht->sizemask();
    he = ht->getEntry(h);
    while (he) {
        if (Jim_CompareHashKeys(ht, key, he->keyAsVoid()))
            return he;
        he = he->next();
    }
    return NULL;
}

JIM_EXPORT Jim_HashTableIterator* Jim_GetHashTableIterator(Jim_HashTablePtr ht) // #MissInCoverage
{
    PRJ_TRACE;
    Jim_HashTableIterator* iter = new_Jim_HashTableIterator; // #AllocF 
    JimInitHashTableIterator(ht, iter);
    return iter;
}

JIM_EXPORT Jim_HashEntryPtr Jim_NextHashEntry(Jim_HashTableIterator* iter) {
    PRJ_TRACE;
    while (1) {
        if (iter->entry() == NULL) {
            iter->indexIncr();
            if (iter->index() >= (signed) iter->ht()->size())
                break;
            iter->setEntry(iter->ht()->getEntry(iter->index()));
        } else {
            iter->setEntry(iter->nextEntry());
        }
        if (iter->entry()) {
            /* We need to save the 'next_' here, the iterator user
             * may delete the entry we are returning. */
            iter->setNextEntry(iter->entry()->next());
            return iter->entry();
        }
    }
    return NULL;
}

/* ------------------------- private functions ------------------------------ */

/* Expand the hash table if needed */
static void JimExpandHashTableIfNeeded(Jim_HashTablePtr ht) {
    PRJ_TRACE;
    /* If the hash table is empty expand it to the initial size_,
     * if the table is "full" double its size_. */
    if (ht->size() == 0)
        Jim_ExpandHashTable(ht, JIM_HT_INITIAL_SIZE);
    if (ht->size() == ht->used())
        Jim_ExpandHashTable(ht, ht->size() * 2);
}

/* Our hash table capability is a power of two */
static unsigned_int JimHashTableNextPower(unsigned_int size) {
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
static Jim_HashEntryPtr JimInsertHashEntry(Jim_HashTablePtr ht, const void* key, int replace) {
    PRJ_TRACE;
    unsigned_int h;
    Jim_HashEntryPtr he;

    /* Expand the hashtable if needed */
    JimExpandHashTableIfNeeded(ht);

    /* Compute the key hash value */
    h = Jim_HashKey(ht, key) & ht->sizemask();
    /* Search if this slot does not already contain the given key */
    he = ht->getEntry(h);
    while (he) {
        if (Jim_CompareHashKeys(ht, key, he->keyAsVoid()))
            return replace ? he : NULL;
        he = he->next();
    }

    /* Allocates the memory and stores key */
    he = new_Jim_HashEntry; // #AllocF 
    he->setNext(ht->getEntry(h));
    ht->setEntry(h, he);
    ht->incrUsed();
    he->setKey(NULL);

    return he;
}

/* ----------------------- StringCopy Hash Table Type ------------------------*/

static unsigned_int JimStringCopyHTHashFunction(const void* key) {
    PRJ_TRACE;
    return Jim_GenHashFunction((const_unsigned_char*) key, (int) strlen((const char*) key));
}

static void* JimStringCopyHTDup(void* privdata MAYBE_USED, const void* key) {
    PRJ_TRACE;
    return Jim_StrDup((const char*) key);
}

static int JimStringCopyHTKeyCompare(void* privdata MAYBE_USED, const void* key1, const void* key2) {
    PRJ_TRACE;
    return strcmp((const char*) key1, (const char*) key2) == 0;
}

static void JimStringCopyHTKeyDestructor(void* privdata MAYBE_USED, void* key) {
    PRJ_TRACE;
    Jim_TFree<void>(key, "void"); // #FreeF 
}

static const Jim_HashTableType g_JimPackageHashTableType = {
    JimStringCopyHTHashFunction,     /* hash function */
    JimStringCopyHTDup,              /* key dup */
    NULL,                            /* val dup */
    JimStringCopyHTKeyCompare,       /* key compare */
    JimStringCopyHTKeyDestructor,    /* key destructor */
    NULL                             /* val destructor */
};

JIM_EXPORT const char* Jim_KeyAsStr(Jim_HashEntryPtr  he) { return he->keyAsStr(); }
JIM_EXPORT const void* Jim_KeyAsVoid(Jim_HashEntryPtr  he) { return he->keyAsVoid(); }
JIM_API_INLINE void Jim_FreeEntryVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr entry) {
    if ((ht)->type()->valDestructor)
        (ht)->type()->valDestructor((ht)->privdata(), (entry)->getVal());
}

JIM_API_INLINE void Jim_SetHashVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, void* _val_) {
    if ((ht)->type()->valDup)
        (entry)->setVal((ht)->type()->valDup((ht)->privdata(), (_val_)));
    else
        (entry)->setVal((_val_));

}
JIM_API_INLINE void Jim_FreeEntryKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry) {
    if ((ht)->type()->keyDestructor)
        (ht)->type()->keyDestructor((ht)->privdata(), (entry)->keyAsVoid());
}
JIM_API_INLINE void Jim_SetHashKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, const void* _key_) {
    if ((ht)->type()->keyDup)
        (entry)->setKey((ht)->type()->keyDup((ht)->privdata(), (_key_)));
    else
        (entry)->setKey((void*) (_key_));
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
JIM_API_INLINE void* Jim_GetHashEntryVal(Jim_HashEntryPtr  he) { return ((he)->getVal()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableCollisions(Jim_HashTablePtr  ht) { return ((ht)->collisions()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableSize(Jim_HashTablePtr  ht) { return ((ht)->size()); }
JIM_API_INLINE unsigned_int Jim_GetHashTableUsed(Jim_HashTablePtr  ht) { return ((ht)->used()); }
JIM_API_INLINE void Jim_FreeHashTableIterator(Jim_HashTableIterator* iter) { Jim_Free(iter); }

#if 0
// g_JimAssocDataHashTableType
static void stupid_example_hashtable() { // #UNUSED #REMOVE
    using JIM_NAMESPACE_NAME::JimAssocDataHashTableType;

    Jim_HashTable ht;
    Jim_HashTableType htt;
    Jim_HashEntryPtr he;
    Jim_InitHashTable(&ht, &htt, NULL);
    Jim_AddHashEntry(&ht, "key", (void*)"value");
    Jim_ReplaceHashEntry(&ht, "key", (void*)"secondValue");
    Jim_HashKey(&ht, "key");
    Jim_GetHashTableSize(&ht);
    Jim_GetHashTableUsed(&ht);
    Jim_GetHashTableCollisions(&ht);
    Jim_FindHashEntry(&ht, "key");
    Jim_ExpandHashTable(&ht, 128);
    auto it = Jim_GetHashTableIterator(&ht);
    while ((he = Jim_NextHashEntry(it)) != NULL) {
        Jim_GetHashEntryKey(he);
        Jim_GetHashEntryVal(he);
        Jim_KeyAsStr(he);
        Jim_KeyAsVoid(he);
    }
    Jim_FreeHashTableIterator(it);
    Jim_DeleteHashEntry(&ht, "key");
    Jim_FreeHashTable(&ht);
}
#endif

END_JIM_NAMESPACE
