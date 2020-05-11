#pragma once

/* Jim - A small embeddable Tcl interpreter
 *
 * Copyright 2005 Salvatore Sanfilippo <antirez@invece.org>
 * Copyright 2005 Clemens Hintze <c.hintze@gmx.net>
 * Copyright 2005 patthoyts - Pat Thoyts <patthoyts@users.sf.net>
 * Copyright 2008 oharboe - Øyvind Harboe - oyvind.harboe@zylin.com
 * Copyright 2008 Andrew Lunn <andrew@lunn.ch>
 * Copyright 2008 Duane Ellis <openocd@duaneellis.com>
 * Copyright 2008 Uwe Klein <uklein@klein-messgeraete.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE JIM TCL PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * JIM TCL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the Jim Tcl Project.
 *
 *
 **/

#include <time.h>
#include <limits.h>
#include <stdio.h>  /* for the FILE typedef definition */
#include <stdlib.h> /* In order to export the Jim_Free() macro */
#include <stdarg.h> /* In order to get type va_list */
#include <stdint.h>

#include <jim-base.h>
#include <prj_trace.h>


BEGIN_JIM_NAMESPACE


#define JIM_EXPORT
#define STATIC 

 /* -----------------------------------------------------------------------------
  * Compiler specific fixes.
  * ---------------------------------------------------------------------------*/

  /* Long Long type and related issues */
typedef int64_t jim_wide;
#ifndef JIM_WIDE_MODIFIER
#  define JIM_WIDE_MODIFIER "lld"
#  define JIM_WIDE_8BYTE
#endif

#define UCHAR(c) ((unsigned_char)(c))

 /* -----------------------------------------------------------------------------
  * Exported defines
  * ---------------------------------------------------------------------------*/
//enum JIM_RETURNS; // Defined in jim-api.h


enum {
    JIM_MAX_CALLFRAME_DEPTH = 1000, /* default max nesting depth for procs */
    JIM_MAX_EVAL_DEPTH = 2000 /* default max nesting depth for eval */
};

/* Some function get an integer argument with flags to change
 * the behavior. */

/* Starting from 1 << 20 flags are reserved for private uses of
 * different calls. This way the same 'flags' argument may be used
 * to pass both global flags and private flags. */
enum {
    JIM_PRIV_FLAG_SHIFT = 20
};

//enum JIM_INTERP_FLAG_FLAGS; // Defined in jim-api.h

/* Flags for Jim_SubstObj() */
enum JIM_SUBSTOBJ_FLAGS {
    JIM_SUBST_NOVAR = 1, /* don't perform variables substitutions */
    JIM_SUBST_NOCMD = 2, /* don't perform command substitutions */
    JIM_SUBST_NOESC = 4, /* don't perform escapes substitutions */
    JIM_SUBST_FLAG = 128 /* flag to indicate that this is a real substitution object */
};

/* Flags used by API calls getting a 'nocase' argument. */
enum JIM_CASE_FLAGS {
    JIM_CASESENS = 0,   /* case sensitive */
    JIM_NOCASE   = 1   /* no case */
};

#if NO_JIM_API // #optionalCode
/* Filesystem related */
enum {
    JIM_PATH_LEN = 1024
};
#endif

/* Unused arguments generate annoying warnings... */
#define JIM_NOTUSED(V) ((void) V)

#define JIM_LIBPATH "auto_path"
#define JIM_INTERACTIVE "tcl_interactive"

#ifndef JIM_API_INLINE // #optionalCode
#  ifdef JIM_INLINE_API_SMALLFUNCS
#    define JIM_API_INLINE JIM_EXPORT inline
#  else
#    define JIM_API_INLINE JIM_EXPORT
#  endif
#endif

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
struct Jim_PrngState;
struct Jim_ExprOperator;
struct jim_subcmd_type;
struct Jim_ListIter;
// Private to jim.cpp
struct ParseTokenList;
struct ParseToken;
struct JimExprNode;
struct ScriptToken;
struct ScriptObj;
struct JimParseMissing;
struct JimParserCtx;
struct lsort_info;
struct AssocDataValue;
struct ExprTree;
struct ExprBuilder;
struct ScanFmtPartDescr;
struct ScanFmtStringObj;
// Private elsewhere
struct regexp;

typedef unsigned long long      unsigned_long_long;
typedef long long               long_long;
typedef unsigned short          unsigned_short;
typedef unsigned long           unsigned_long;
typedef unsigned char           unsigned_char;
typedef const unsigned char     const_unsigned_char;
typedef const unsigned long     const_unsigned_long;
typedef unsigned int            unsigned_int;
typedef unsigned                unsigned_t;
typedef uint64_t                unsigned_jim_wide;
typedef int                     Retval;
typedef Jim_HashEntry*          Jim_HashEntryArray;
typedef Jim_HashEntry*          Jim_HashEntryPtr;
typedef void*                   VoidPtrArray;
typedef Jim_Obj*                Jim_ObjArray;
typedef const char*             constCharArray;
typedef Jim_Obj* const*         Jim_ObjConstArray;
typedef Jim_Stack*              Jim_StackPtr;
typedef Jim_HashTable*          Jim_HashTablePtr;
typedef Jim_Interp*             Jim_InterpPtr;
// Private to jim.cpp
typedef ScriptToken*            ScriptTokenPtr;
typedef ParseTokenList*         ParseTokenListPtr;
typedef ParseToken*             ParseTokenPtr;
typedef JimExprNode*            JimExprNodePtr;
typedef Jim_ExprOperator*       Jim_ExprOperatorPtr;
typedef Jim_ExprOperator*       const_Jim_ExprOperatorPtr;
typedef ExprTree*               ExprTreePtr;
typedef ExprBuilder*            ExprBuilderPtr;
typedef ScanFmtPartDescr*       ScanFmtPartDescrPtr;
typedef ScanFmtStringObj*       ScanFmtStringObjPtr;
typedef Jim_ListIter*           Jim_ListIterPtr;
typedef Jim_Obj*                Jim_ObjPtr;

/* -----------------------------------------------------------------------------
 * Stack
 * ---------------------------------------------------------------------------*/

struct Jim_Stack {
private:
    int len_ = 0;
    int maxlen_ = 0;
public:
    VoidPtrArray* vector_ = NULL;

    int len() const { return len_;  }
    int maxlen() const { return maxlen_;  }
    void lenIncr() { ++len_; }
    void lenDecr() { --len_;  }

    friend void Jim_InitStack(Jim_StackPtr stack);
    friend void Jim_FreeStack(Jim_StackPtr stack);
    friend int Jim_StackLen(Jim_StackPtr stack);
    friend void Jim_StackPush(Jim_StackPtr stack, void *element);
    friend void *Jim_StackPop(Jim_StackPtr stack);
    friend void *Jim_StackPeek(Jim_StackPtr stack);
    friend void Jim_FreeStackElements(Jim_StackPtr stack, void(*freeFunc) (void *ptr));
};

/* -----------------------------------------------------------------------------
 * Hash table
 * ---------------------------------------------------------------------------*/

struct Jim_HashEntry {
private:
    void *key_;
    Jim_HashEntryPtr next_;
    union {
        void *val_;
        int intval_;
    } u;
public:
    void* voidValue() const { return u.val_;  }
    Jim_HashEntryPtr  next() const { return next_;  }
    const char* keyAsStr() const { return (const char*)key_;  }
    void* keyAsVoid() const { return key_;  }
    Jim_ObjPtr  keyAsObj() const { return (Jim_ObjPtr ) key_; }

    friend int Jim_ReplaceHashEntry(Jim_HashTablePtr ht, const void *key, void *val);
    friend void Jim_SetHashVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, void* _val_);
    friend void Jim_SetHashKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, const void* _key_);
    friend STATIC Jim_HashEntryPtr JimInsertHashEntry(Jim_HashTablePtr ht, const void *key, int replace);
    friend int Jim_DeleteHashEntry(Jim_HashTablePtr ht, const void *key);
    friend void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size);
};

struct Jim_HashTableType {
    unsigned_int (*hashFunction)(const void *key) = NULL;
    void *(*keyDup)(void *privdata, const void *key) = NULL;
    void *(*valDup)(void *privdata, const void *obj) = NULL;
    int (*keyCompare)(void *privdata, const void *key1, const void *key2) = NULL;
    void (*keyDestructor)(void *privdata, void *key) = NULL;
    void (*valDestructor)(void *privdata, void *obj) = NULL;
};

struct Jim_HashTable {
private:
    const char* typeName_ = "unknown"; // Assume static string
        // typeName_ current-possible: staticVars, variables, refMark, commands, references, assocData, packages, dict
    const Jim_HashTableType *type_ = NULL; /* not used */
    void *privdata_ = NULL;

    unsigned_int size_ = 0;
    unsigned_int sizemask_ = 0;
    unsigned_int collisions_ = 0;
    unsigned_int uniq_ = 0;
    unsigned_int used_ = 0;

    friend STATIC void JimResetHashTable(Jim_HashTablePtr ht);
    friend STATIC Jim_HashEntryPtr JimInsertHashEntry(Jim_HashTablePtr ht, const void *key, int replace);
    friend STATIC void JimFreeCallFrame(Jim_InterpPtr interp, Jim_CallFrame *cf, int action);
public:
    Jim_HashEntryArray* table_ = NULL;
    unsigned_int uniq() const { return uniq_;  }
    unsigned_int collisions() const { return collisions_; }
    unsigned_int used() const { return used_; }
    unsigned_int sizemask() const { return sizemask_;  }
    unsigned_int size() const { return size_; }
    void* privdata() { return privdata_; }
    const Jim_HashTableType *type() const { return type_;  }
    void setTypeName(const char* n) { typeName_ = n; }
    const char* getTypeName() const { return typeName_; }

    friend void Jim_ExpandHashTable(Jim_HashTablePtr ht, unsigned_int size);
    friend int Jim_DeleteHashEntry(Jim_HashTablePtr ht, const void *key);
    friend int Jim_FreeHashTable(Jim_HashTablePtr ht);
    friend int Jim_InitHashTable(Jim_HashTablePtr ht, const Jim_HashTableType *type, void *privDataPtr);
};

struct Jim_HashTableIterator {
private:
    Jim_HashTablePtr ht = NULL;
    Jim_HashEntryPtr entry_ = NULL; 
    Jim_HashEntryPtr nextEntry_ = NULL;
    int index_ = 0;
public:

    int index() const { return index_;  }
    Jim_HashEntryPtr  entry() const { return entry_; }
    Jim_HashEntryPtr   nextEntry() const { return nextEntry_;  }

    friend STATIC void JimInitHashTableIterator(Jim_HashTablePtr ht, Jim_HashTableIterator *iter);
    friend Jim_HashEntryPtr Jim_NextHashEntry(Jim_HashTableIterator *iter);
};

/* This is the initial size of every hash table */
enum {
    JIM_HT_INITIAL_SIZE = 16
};


/* ------------------------------- Macros ------------------------------------*/
JIM_API_INLINE void Jim_FreeEntryVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr entry);

JIM_API_INLINE void Jim_SetHashVal(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, void* _val_);

JIM_API_INLINE void Jim_FreeEntryKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry);

JIM_API_INLINE void Jim_SetHashKey(Jim_HashTablePtr  ht, Jim_HashEntryPtr  entry, const void* _key_);

JIM_API_INLINE int Jim_CompareHashKeys(Jim_HashTablePtr  ht, const void* key1, const void* key2);

JIM_API_INLINE unsigned_int Jim_HashKey(Jim_HashTablePtr  ht, const void* key);

JIM_API_INLINE void* Jim_GetHashEntryKey(Jim_HashEntryPtr  he);
JIM_API_INLINE void* Jim_GetHashEntryVal(Jim_HashEntryPtr  he);
JIM_API_INLINE unsigned_int Jim_GetHashTableCollisions(Jim_HashTablePtr  ht);
JIM_API_INLINE unsigned_int Jim_GetHashTableSize(Jim_HashTablePtr  ht);
JIM_API_INLINE unsigned_int Jim_GetHashTableUsed(Jim_HashTablePtr  ht);
/* -----------------------------------------------------------------------------
 * Jim_Obj structure
 * ---------------------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 * Jim object. This is mostly the same as Tcl_Obj itself,
 * with the addition of the 'prev' and 'next' pointers.
 * In Jim all the objects are stored into a linked list for GC purposes,
 * so that it's possible to access every object living in a given interpreter
 * sequentially. When an object is freed, it's moved into a different
 * linked list, used as object pool.
 *
 * The refcount of a freed object is always -1.
 * ---------------------------------------------------------------------------*/
typedef struct Jim_Obj {
private:
    int refCount_ = 0; /* reference count */
    int length_ = 0; /* number of bytes in 'bytes', not including the null term. */
    const Jim_ObjType* typePtr_; /* object type. */

    // not public
    friend STATIC void JimSetStringBytes(Jim_ObjPtr objPtr, const char *str);
    friend STATIC void StringAppendString(Jim_ObjPtr objPtr, const char *str, int len);
    friend STATIC Jim_ObjPtr JimStringTrimRight(Jim_InterpPtr interp, Jim_ObjPtr strObjPtr, Jim_ObjPtr trimcharsObjPtr);
    friend STATIC Jim_ObjPtr JimInterpolateTokens(Jim_InterpPtr interp, const ScriptTokenPtr  token, int tokens, int flags);
    friend STATIC void JimMakeListStringRep(Jim_ObjPtr objPtr, Jim_ObjArray* objv, int objc);
public:
    char *bytes_; /* string representation buffer. NULL = no string repr. */

    friend Jim_ObjPtr Jim_DuplicateObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend Jim_ObjPtr Jim_NewStringObj(Jim_InterpPtr interp, const char *s, int len);
    friend Jim_ObjPtr Jim_NewStringObjNoAlloc(Jim_InterpPtr interp, char *s, int len);
    friend Jim_ObjPtr Jim_NewObj(Jim_InterpPtr interp);
    friend void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend void Jim_IncrRefCount(Jim_ObjPtr  objPtr);
    friend void Jim_DecrRefCount(Jim_InterpPtr  interp, Jim_ObjPtr  objPtr);

    int length() const { return length_;  }
    void lengthDecr(int count = 1) { length_ -= count;  }
    int refCount() const { return refCount_; }
    const Jim_ObjType* typePtr() const { return typePtr_;  }
    inline void setTypePtr(const Jim_ObjType* typeD); // forward declared
    char* bytes() const { return bytes_;  }

    /* Internal representation union */
    union {
        /* integer number type */
        jim_wide wideValue_;
        /* generic integer value (e.g. index, return code) */
        int intValue_;
        /* double number type */
        double doubleValue_;
        /* Generic pointer */
        void *ptr_;
        /* Generic two pointers value */
        struct {
            void *ptr1;
            void *ptr2;
        } twoPtrValue_; /* UNUSED */
        /* Generic pointer, int, int value */
        struct {
        private:
            void *ptr;
            int int1;
            int int2;

            friend void FreeRegexpInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend regexp *SetRegexpFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr, unsigned_t flags);
            friend const jim_subcmd_type *Jim_ParseSubCmd(Jim_InterpPtr interp, const jim_subcmd_type * command_table,
                                                          int argc, Jim_ObjConstArray argv);
            friend int Jim_GetEnum(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                   const char *const *tablePtr, int *indexPtr, const char *name, int flags);
        public:
        } ptrIntValue_;
        /* Variable object */
        struct {
         private:
             Jim_Var *varPtr;
             unsigned_long callFrameId; /* for caching */
             int global; /* If the variable name is globally scoped with :: */
            
             friend int Jim_SetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, Jim_ObjPtr valObjPtr);
             friend STATIC int SetVariableFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
             friend STATIC Jim_Var *JimCreateVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, Jim_ObjPtr valObjPtr);
             friend int Jim_UnsetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags);
             friend int Jim_SetVariableLink(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr,
                                            Jim_ObjPtr targetNameObjPtr, Jim_CallFrame *targetCallFrame);
             friend Jim_ObjPtr Jim_GetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags);
        } varValue_;
        /* Command object */
        struct {
            Jim_ObjPtr nsObj;
            Jim_Cmd *cmdPtr;
            unsigned_long procEpoch; /* for caching */
        private:
            friend STATIC void FreeCommandInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC void DupCommandInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
            friend Jim_Cmd *Jim_GetCommand(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);
            friend int Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr);
        } cmdValue_;
        /* List object */
        struct {
        private:
            Jim_ObjArray* ele;    /* Elements vector */
            int len;        /* Length */
            int maxLen;        /* Allocated 'ele' length */

            friend STATIC void FreeListInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC void DupListInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
            friend STATIC void UpdateStringOfListCB(Jim_ObjPtr objPtr);
            friend STATIC int SetListFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend Jim_ObjPtr Jim_NewListObj(Jim_InterpPtr interp, Jim_ObjConstArray elements, int len);
            friend STATIC void JimListGetElements(Jim_InterpPtr interp, Jim_ObjPtr listObj, int *listLen,
                                           Jim_ObjArray* *listVec);
            friend STATIC void ListRemoveDuplicates(Jim_ObjPtr listObjPtr, int(*comp)(Jim_ObjArray* lhs, Jim_ObjArray* rhs));
            friend STATIC int ListSortElements(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr, lsort_info *info);
            friend STATIC void ListInsertElements(Jim_ObjPtr listPtr, int idx, int elemc, Jim_ObjConstArray elemVec);
            friend STATIC void ListAppendList(Jim_ObjPtr listPtr, Jim_ObjPtr appendListPtr);
            friend int Jim_ListLength(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend void Jim_ListInsertElements(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int idx,
                                               int objc, Jim_ObjConstArray objVec);
            friend Jim_ObjPtr Jim_ListGetIndex(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int idx);
            friend STATIC int ListSetIndex(Jim_InterpPtr interp, Jim_ObjPtr listPtr, int idx,
                                    Jim_ObjPtr newObjPtr, int flags);
            friend Jim_ObjPtr Jim_ListRange(Jim_InterpPtr interp, Jim_ObjPtr listObjPtr, Jim_ObjPtr firstObjPtr,
                                          Jim_ObjPtr lastObjPtr);
            friend STATIC int JimEvalObjList(Jim_InterpPtr interp, Jim_ObjPtr listPtr);
            friend int Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr);
            friend STATIC Jim_ObjPtr JimListIterNext(Jim_InterpPtr interp, Jim_ListIterPtr iter);
            friend STATIC int Jim_LreplaceCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
        } listValue_;
        /* String type */
        struct {
        private:
            int maxLength;
            int charLength;     /* utf-8 char length. -1 if unknown */

            friend STATIC void DupStringInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
            friend STATIC int SetStringFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend int Jim_Utf8Length(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend Jim_ObjPtr Jim_NewStringObjUtf8(Jim_InterpPtr interp, const char *s, int charlen);
            friend STATIC void StringAppendString(Jim_ObjPtr objPtr, const char *str, int len);
         } strValue_;
        /* Reference type */
        struct {
        private:
            unsigned_long id;
            Jim_Reference *refPtr;

            friend STATIC void UpdateStringOfReferenceCB(Jim_ObjPtr objPtr);
            friend STATIC int SetReferenceFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend Jim_ObjPtr Jim_NewReference(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr tagPtr, Jim_ObjPtr cmdNamePtr);
            friend Jim_Reference *Jim_GetReference(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend int Jim_Collect(Jim_InterpPtr interp);

        } refValue_;
        /* Source type */
        struct {
        private:
            Jim_ObjPtr fileNameObj;
            int lineNumber;

            friend STATIC void FreeSourceInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC void DupSourceInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
            friend STATIC void JimSetSourceInfo(Jim_InterpPtr interp, Jim_ObjPtr objPtr,
                                         Jim_ObjPtr fileNameObj, int lineNumber);
            friend STATIC void JimSetScriptFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend void Jim_FreeInterp(Jim_InterpPtr i);
            friend STATIC int SetListFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC int SetExprFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC Jim_ObjPtr JimInterpolateTokens(Jim_InterpPtr interp, const ScriptTokenPtr  token, int tokens, int flags);
            friend STATIC int Jim_InfoCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
        } sourceValue_;
        /* Dict substitution type */
        struct {
        private:
            Jim_ObjPtr varNameObjPtr;
            Jim_ObjPtr indexObjPtr;

            friend STATIC Jim_ObjPtr JimInterpolateTokens(Jim_InterpPtr interp, const ScriptTokenPtr  token, int tokens, int flags);
            friend STATIC void FreeInterpolatedInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC int JimDictSugarSet(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr valObjPtr);
            friend STATIC void DupInterpolatedInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
            friend STATIC Jim_ObjPtr JimDictSugarGet(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);
            friend STATIC void FreeDictSubstInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC void DupDictSubstInternalRepCB(Jim_InterpPtr interp, Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
            friend STATIC void SetDictSubstFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
            friend STATIC Jim_ObjPtr JimExpandDictSugar(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
        } dictSubstValue_;
        struct {
        private:
            int line;
            int argc;

            friend int Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr);
            friend STATIC Jim_ObjPtr JimNewScriptLineObj(Jim_InterpPtr interp, int argc, int line);
        } scriptLineValue_;
    } internalRep;

    jim_wide getWideValue() const { return internalRep.wideValue_;  }
    int getIntValue() const { return internalRep.intValue_; }
    double getDoubleValue() const { return internalRep.doubleValue_; }
    void* getVoidPtr() const { return internalRep.ptr_; }

    /* These fields add 8 or 16 bytes more for every object
     * but this is required for efficient garbage collection
     * of Jim references. */
    Jim_ObjPtr prevObjPtr_ = NULL; /* pointer to the prev object. */
    Jim_ObjPtr nextObjPtr_ = NULL; /* pointer to the next object. */

    Jim_ObjPtr  nextObjPtr() const { return nextObjPtr_; }
    Jim_ObjPtr  prevObjPtr() const { return prevObjPtr_;  }
} Jim_Obj;

/* Jim_Obj related macros */

void Jim_FreeObj (Jim_InterpPtr interp, Jim_ObjPtr objPtr); /* EJ HACK */
JIM_API_INLINE void Jim_IncrRefCount(Jim_ObjPtr  objPtr);
JIM_API_INLINE void Jim_DecrRefCount(Jim_InterpPtr  interp, Jim_ObjPtr  objPtr);
JIM_EXPORT int  Jim_RefCount(Jim_ObjPtr  objPtr);
JIM_API_INLINE int Jim_IsShared(Jim_ObjPtr  objPtr);

/* This macro is used when we allocate a new object using
 * Jim_New...Obj(), but for some error we need to destroy it.
 * Instead to use Jim_IncrRefCount() + Jim_DecrRefCount() we
 * can just call Jim_FreeNewObj. To call Jim_Free directly
 * seems too raw, the object handling may change and we want
 * that Jim_FreeNewObj() can be called only against objects
 * that are believed to have refcount == 0. */
#define Jim_FreeNewObj Jim_FreeObj


/* Get the internal representation pointer */
/* EJ #define Jim_GetIntRepPtr(o) (o)->internalRep.ptr */
JIM_API_INLINE void* Jim_GetIntRepPtr(Jim_ObjPtr  o);

/* Set the internal representation pointer */
/* EJ #define Jim_SetIntRepPtr(o, p) \
    (o)->internalRep.ptr = (p) */
JIM_API_INLINE void Jim_SetIntRepPtr(Jim_ObjPtr  o, void* p);

/* The object type structure.
 * There are three methods.
 *
 * - freeIntRepProc is used to free the internal representation of the object.
 *   Can be NULL if there is nothing to free.
 *
 * - dupIntRepProc is used to duplicate the internal representation of the object.
 *   If NULL, when an object is duplicated, the internalRep union is
 *   directly copied from an object to another.
 *   Note that it's up to the caller to free the old internal repr of the
 *   object before to call the Dup method.
 *
 * - updateStringProc is used to create the string from the internal repr.
 */

struct Jim_Interp;

typedef void (Jim_FreeInternalRepProc)(Jim_InterpPtr interp,
        Jim_ObjPtr objPtr);
typedef void (Jim_DupInternalRepProc)(Jim_InterpPtr interp,
        Jim_ObjPtr srcPtr, Jim_ObjPtr dupPtr);
typedef void (Jim_UpdateStringProc)(Jim_ObjPtr objPtr);

struct Jim_ObjType {
    const char *name = NULL; /* The name of the type. */
    Jim_FreeInternalRepProc *freeIntRepProc = NULL;
    Jim_DupInternalRepProc *dupIntRepProc = NULL;
    Jim_UpdateStringProc *updateStringProc = NULL;
    int flags = 0;
};

/* Jim_ObjType flags */
enum {
    JIM_TYPE_NONE = 0,        /* No flags */
    JIM_TYPE_REFERENCES = 1    /* The object may contain references. */
};

/* -----------------------------------------------------------------------------
 * Call frame, vars, commands structures
 * ---------------------------------------------------------------------------*/

/* Call frame */
typedef struct Jim_CallFrame {
private:
    unsigned_long id_ = 0; /* Call Frame ID. Used for caching. */
    int level_ = 0; /* Level of this call frame. 0 = global */
    Jim_HashTable vars_; /* Where local vars are stored */
    Jim_HashTablePtr staticVars_ = NULL; /* pointer to procedure static vars */
    Jim_CallFrame *parent_ = NULL; /* The parent callframe */
    Jim_ObjConstArray argv_ = NULL; /* object vector of the current procedure call. */
    int argc_ = 0; /* number of args of the current procedure call. */
public:
    Jim_ObjPtr procArgsObjPtr_ = NULL; /* arglist object of the running procedure */
    Jim_ObjPtr procBodyObjPtr_ = NULL; /* body object of the running procedure */
    Jim_CallFrame *next = NULL; /* Callframes are in a linked list */
    Jim_ObjPtr nsObj_ = NULL;             /* Namespace for this proc call frame */
    Jim_ObjPtr fileNameObj_ = NULL;       /* file and line of caller of this proc (if available) */
    int line;
    Jim_StackPtr localCommands_ = NULL; /* commands to be destroyed when the call frame is destroyed */
    Jim_ObjPtr tailcallObj_ = NULL;  /* Pending tailcall invocation */
    Jim_Cmd *tailcallCmd_ = NULL;  /* Resolved command for pending tailcall invocation */

    unsigned_long id() const { return id_; }
    int level() const { return level_;  }
    Jim_HashTable& vars() { return vars_; }
    Jim_HashTablePtr  staticVars() { return staticVars_; }
    Jim_CallFrame* parent() { return parent_; }
    int argc() const { return argc_; }
    Jim_ObjPtr  procArgsObjPtr() const { return procArgsObjPtr_; }
    Jim_ObjPtr  procBodyObjPtr() const { return procBodyObjPtr_;  }
    Jim_ObjPtr  nsObj() const { return nsObj_;  }
    Jim_StackPtr  localCommands() const { return localCommands_; }

    friend int Jim_UnsetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags);
    friend STATIC Jim_CallFrame *JimCreateCallFrame(Jim_InterpPtr interp, Jim_CallFrame *parent, Jim_ObjPtr nsObj);
    friend int Jim_EvalNamespace(Jim_InterpPtr interp, Jim_ObjPtr scriptObj, Jim_ObjPtr nsObj);
    friend STATIC int JimCallProcedure(Jim_InterpPtr interp, Jim_Cmd *cmd, int argc, Jim_ObjConstArray argv);
    friend STATIC int JimInfoLevel(Jim_InterpPtr interp, Jim_ObjPtr levelObjPtr,
                            Jim_ObjArray* objPtrPtr, int info_level_cmd);
} Jim_CallFrame;

/* The var structure. It just holds the pointer of the referenced
 * object. If linkFramePtr is not NULL the variable is a link
 * to a variable of name stored in objPtr living in the given callframe
 * (this happens when the [global] or [upvar] command is used).
 * The interp in order to always know how to free the Jim_Obj associated
 * with a given variable because in Jim objects memory management is
 * bound to interpreters. */
struct Jim_Var {
    Jim_ObjPtr objPtr = NULL; /* UNUSED */
    Jim_CallFrame *linkFramePtr = NULL; /* UNUSED */
};

/* The cmd structure. */
typedef int Jim_CmdProc(Jim_InterpPtr interp, int argc,
    Jim_ObjConstArray argv);
typedef void Jim_DelCmdProc(Jim_InterpPtr interp, void *privData);



/* A command is implemented in C if isproc is 0, otherwise
 * it is a Tcl procedure with the arglist and body represented by the
 * two objects referenced by arglistObjPtr and bodyObjPtr. */

struct Jim_ProcArg {
    Jim_ObjPtr nameObjPtr = NULL;    /* Name of this arg */
    Jim_ObjPtr defaultObjPtr = NULL; /* Default value, (or rename for $args) */
};

struct Jim_Cmd {
    int isproc_ = 0;          /* Is this a procedure? */
    Jim_Cmd *prevCmd_ = NULL;    /* Previous command defn if cmd created 'local' */
    int inUse_ = 0;           /* Reference count */
private:
public:
    union {
        struct {
            /* native (C) command */
            Jim_CmdProc *cmdProc = NULL; /* The command implementation */
            Jim_DelCmdProc *delProc = NULL; /* Called when the command is deleted if != NULL */
            void *privData = NULL; /* command-private data available via Jim_CmdPrivData() */
        } native_;
        struct {
            /* Tcl procedure */
            Jim_ObjPtr argListObjPtr = NULL;
            Jim_ObjPtr bodyObjPtr = NULL;
            Jim_HashTablePtr staticVars = NULL;  /* Static vars hash table. NULL if no statics. */
            int argListLen = 0;             /* Length of argListObjPtr */
            int reqArity = 0;               /* Number of required parameters */
            int optArity = 0;               /* Number of optional parameters */
            int argsPos = 0;                /* Position of 'args', if specified, or -1 */
            int upcall = 0;                 /* True if proc is currently in upcall */
	        Jim_ProcArg *arglist = NULL;
            Jim_ObjPtr nsObj = NULL;             /* Namespace for this proc */
        } proc_;
    } u;

    int inUse() const { return inUse_; }
    int isproc() const { return isproc_; }
    Jim_Cmd* prevCmd() const { return prevCmd_; }
};

/* Pseudo Random Number Generator State structure */
struct Jim_PrngState {
    unsigned_char sbox[256];
    unsigned_int i = 0, j = 0;
};

#define free_Jim_PrngState(ptr)             Jim_TFree<Jim_PrngState>(ptr,"Jim_PrngState")

/* -----------------------------------------------------------------------------
 * Jim interpreter structure.
 * Fields similar to the real Tcl interpreter structure have the same names.
 * ---------------------------------------------------------------------------*/
struct Jim_Interp {

    Jim_HashTable assocData_; /* per-interp storage for use by packages */
private:
    Jim_ObjPtr result_ = NULL;        /* object returned by the last command called. */
    int errorLine_ = 0;             /* Error line where an error occurred. UNUSED */
    Jim_ObjPtr errorFileNameObj_ = NULL; /* Error file where an error occurred. */
    int addStackTrace_ = 0;         /* > 0 if a level should be added to the stack trace */
    int maxCallFrameDepth_ = 0;     /* Used for infinite loop detection. */
    int maxEvalDepth_ = 0;          /* Used for infinite loop detection. */
    int evalDepth_ = 0;             /* Current eval depth */
    int returnCode_ = 0;            /* Completion code to return on JIM_RETURN. */
    int returnLevel_ = 0;           /* Current level of 'return -level' */
    int exitCode_ = 0;              /* Code to return to the OS on JIM_EXIT. */
    long id_ = 0;                   /* Hold unique id for various purposes UNUSED */
    int signal_level_ = 0;          /* A nesting level of catch -signal */
    jim_wide sigmask_ = 0;          /* Bit mask of caught signals, or 0 if none */
    int (*signal_set_result_)(Jim_InterpPtr interp, jim_wide sigmaskD) = NULL; /* Set a result for the sigmask */
    Jim_CallFrame* framePtr_ = NULL;    /* Pointer to the current call frame */
    Jim_CallFrame* topFramePtr_ = NULL; /* toplevel/global frame pointer. */
    Jim_HashTable commands_; /* Commands hash table */
    unsigned_long procEpoch_ = 0; /* Incremented every time the result
                of procedures names lookup caching
                may no longer be valid. */
    unsigned_long callFrameEpoch_ = 0; /* Incremented every time a new
                callframe is created. This id is used for the
                'ID' field contained in the Jim_CallFrame
                structure. */
    int local_ = 0; /* If 'local' is in effect, newly defined procs keep a reference to the old defn */
    Jim_ObjPtr liveList_ = NULL; /* Linked list of all the live objects. */
    Jim_ObjPtr freeList_ = NULL; /* Linked list of all the unused objects. */
    Jim_ObjPtr currentScriptObj_ = NULL; /* Script currently in execution. */
    Jim_ObjPtr nullScriptObj_ = NULL; /* script representation of an empty string */
    Jim_ObjPtr emptyObj_ = NULL; /* Shared empty string object. */
    Jim_ObjPtr trueObj_ = NULL; /* Shared true int object. */
    Jim_ObjPtr falseObj_ = NULL; /* Shared false int object. */
    unsigned_long referenceNextId_ = 0; /* Next id for reference. */
    Jim_HashTable references_; /* References hash table. */
    unsigned_long lastCollectId_ = 0; /* reference max Id of the last GC
                execution. It's set to ~0 while the collection
                is running as sentinel to avoid to recursive
                calls via the [collect] command inside
                finalizers. */
    time_t lastCollectTime_ = 0; /* Unix time of the last GC execution */
    Jim_ObjPtr stackTrace_ = NULL; /* Stack trace object. */
    Jim_ObjPtr errorProc_ = NULL; /* Name of last procedure which returned an error */
    Jim_ObjPtr unknown_ = NULL; /* Unknown command cache */
    int unknown_called_ = 0; /* The unknown command has been invoked */
    int errorFlag_ = 0; /* Set if an error occurred during execution. */
    void* cmdPrivData_ = NULL; /* Used to pass the private data pointer to
                  a command. It is set to what the user specified
                  via Jim_CreateCommand(). */
    Jim_CallFrame* freeFramesList_ = NULL; /* list of CallFrame structures. */
    Jim_PrngState* prngState_; /* per interpreter Random Number Gen. state. */
    Jim_HashTable packages_; /* Provided packages hash table */
    Jim_StackPtr loadHandles_; /* handles of loaded modules [load] UNUSED */

    friend Jim_HashTablePtr  Jim_PackagesHT(Jim_InterpPtr  interp);
    friend Jim_InterpPtr Jim_CreateInterp(void);
    friend void Jim_FreeInterp(Jim_InterpPtr i);
    friend void JimPrngInit(Jim_InterpPtr interp);
    friend void JimRandomBytes(Jim_InterpPtr interp, void* dest, unsigned_int len);
    friend void JimPrngSeed(Jim_InterpPtr interp, unsigned_char* seed, int seedLen);
    friend Jim_CallFrame* JimCreateCallFrame(Jim_InterpPtr interp, Jim_CallFrame* parent, Jim_ObjPtr nsObj);
    friend void JimFreeCallFrame(Jim_InterpPtr interp, Jim_CallFrame* cf, int action);
    friend void* Jim_CmdPrivData(Jim_InterpPtr  i);
    friend Retval JimInvokeCommand(Jim_InterpPtr interp, int objc, Jim_ObjConstArray objv);
    friend void JimSetStackTrace(Jim_InterpPtr interp, Jim_ObjPtr stackTraceObj);
    friend void JimAddErrorToStack(Jim_InterpPtr interp, ScriptObj* script);
    friend Retval Jim_EvalObj(Jim_InterpPtr interp, Jim_ObjPtr scriptObjPtr);
    friend Retval Jim_CatchCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend Retval JimUnknown(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend Retval JimCallProcedure(Jim_InterpPtr interp, Jim_Cmd* cmd, int argc, Jim_ObjConstArray argv);
    friend void JimResetStackTrace(Jim_InterpPtr interp);
    friend void JimAppendStackTrace(Jim_InterpPtr interp, const char* procname,
                                    Jim_ObjPtr fileNameObj, int linenr);
    friend Retval Jim_InfoCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend int Jim_Collect(Jim_InterpPtr interp);
    friend void Jim_CollectIfNeeded(Jim_InterpPtr interp);
    friend int SetReferenceFromAny(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend Jim_ObjPtr Jim_NewReference(Jim_InterpPtr interp, Jim_ObjPtr objPtr, Jim_ObjPtr tagPtr, Jim_ObjPtr cmdNamePtr);
    friend Retval JimInfoReferences(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend Retval Jim_EvalNamespace(Jim_InterpPtr interp, Jim_ObjPtr scriptObj, Jim_ObjPtr nsObj);
    friend Jim_ObjPtr Jim_NewObj(Jim_InterpPtr interp);
    friend void Jim_FreeObj(Jim_InterpPtr interp, Jim_ObjPtr objPtr);
    friend Retval Jim_DebugCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend Retval Jim_CollectCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend Retval Jim_UnsetVariable(Jim_InterpPtr interp, Jim_ObjPtr nameObjPtr, int flags);
    friend Retval JimCreateCommand(Jim_InterpPtr interp, const char* name, Jim_Cmd* cmd);
    friend void JimUpdateProcNamespace(Jim_InterpPtr interp, Jim_Cmd* cmdPtr, const char* cmdname);
    friend Retval Jim_DeleteCommand(Jim_InterpPtr interp, const char* cmdName);
    friend Retval Jim_RenameCommand(Jim_InterpPtr interp, const char* oldName, const char* newName);
    friend Jim_Cmd* Jim_GetCommand(Jim_InterpPtr interp, Jim_ObjPtr objPtr, int flags);
    friend Retval JimDeleteLocalProcs(Jim_InterpPtr interp, Jim_StackPtr localCommands);
    friend Jim_ObjPtr JimCommandsList(Jim_InterpPtr interp, Jim_ObjPtr patternObjPtr, int type);
    friend long_long Jim_CheckSignal(Jim_InterpPtr  i);
    friend long Jim_GetId(Jim_InterpPtr  i);
    friend Retval Jim_ExitCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend Retval Jim_EvalFile(Jim_InterpPtr interp, const char* filename);
    friend Retval Jim_ReturnCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
    friend void Jim_IncrStackTrace(Jim_InterpPtr  interp);
    friend void Jim_SetResult(Jim_InterpPtr  i, Jim_ObjPtr  o);
    friend Retval Jim_signalInit(Jim_InterpPtr interp);
    friend Retval signal_cmd_throw(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv);
public:
    Jim_ObjPtr  result() const { return result_;  }
    int errorLine() const { return errorLine_;  }
    Jim_ObjPtr  errorFileNameObj() const { return errorFileNameObj_; }
    int addStackTrace() const { return addStackTrace_; }
    void incrAddStackTrace() { addStackTrace_++;  }
    int maxCallFrameDepth() const { return maxCallFrameDepth_;  }
    int maxEvalDepth() const { return maxEvalDepth_;  }
    int evalDepth() const { return evalDepth_; }
    void incrEvalDepth() { evalDepth_++; }
    void decrEvalDepth() { evalDepth_--; }
    int returnCode() const { return returnCode_; }
    int exitCode() const { return exitCode_; }
    long id() const { return id_;  }
    void incrSignalLevel(int sig) { signal_level_ += sig;  }
    void decrSignalLevel(int sig) { signal_level_ -= sig;  }
    jim_wide getSigmask() { return sigmask_; }
    Jim_CallFrame * framePtr() const { return framePtr_; }
    void framePtr(Jim::Jim_CallFrame * val) { framePtr_ = val; }
    Jim_CallFrame * topFramePtr() const { return topFramePtr_; }
    void topFramePtr(Jim::Jim_CallFrame * val) { topFramePtr_ = val; }
    unsigned_long procEpoch() const { return procEpoch_; }
    void procEpoch(unsigned_long val) { procEpoch_ = val; }
    unsigned_long callFrameEpoch() const { return callFrameEpoch_; }
    void callFrameEpoch(unsigned_long val) { callFrameEpoch_ = val; }
    int local() const { return local_; }
    void incrLocal() { local_++; }
    void decrLocal() { local_--; }
    Jim_ObjPtr  currentScriptObj() const { return currentScriptObj_; }
    void currentScriptObj(Jim::Jim_ObjPtr  val) { currentScriptObj_ = val; }
    Jim_ObjPtr  nullScriptObj() const { return nullScriptObj_; }
    void nullScriptObj(Jim::Jim_ObjPtr  val) { nullScriptObj_ = val; }
    Jim_ObjPtr  emptyObj() const { return emptyObj_; }
    void emptyObj(Jim::Jim_ObjPtr  val) { emptyObj_ = val; }
    Jim_ObjPtr  trueObj() const { return trueObj_; }
    void trueObj(Jim::Jim_ObjPtr  val) { trueObj_ = val; }
    Jim_ObjPtr  falseObj() const { return falseObj_; }
    void falseObj(Jim::Jim_ObjPtr  val) { falseObj_ = val; }
    unsigned_long referenceNextId() const { return referenceNextId_;  }
    unsigned_long incrReferenceNextId() { return (referenceNextId_++); }
    time_t lastCollectTime() const { return lastCollectTime_; }
    void lastCollectTime(time_t val) { lastCollectTime_ = val; }
};

/* Free the internal representation of the object. */
JIM_API_INLINE void Jim_FreeIntRep(Jim_InterpPtr  i, Jim_ObjPtr  o);

/* Currently provided as macro that performs the increment.
 * At some point may be a real function doing more work.
 * The proc epoch is used in order to know when a command lookup
 * cached can no longer considered valid. */

/* Note: Using trueObj and falseObj here makes some things slower...*/

/* Use this for file handles, etc. which need a unique id */
/* EJ #define Jim_GetId(i) (++(i)->id)  */
JIM_API_INLINE long Jim_GetId(Jim_InterpPtr  i);

/* Reference structure. The interpreter pointer is held within privdata member in HashTable */
enum {
     JIM_REFERENCE_TAGLEN = 7 /* The tag is fixed-length, because the reference
                                  string representation must be fixed length. */
};

struct Jim_Reference {
    Jim_ObjPtr objPtr = NULL;
    Jim_ObjPtr finalizerCmdNamePtr = NULL;
    char tag[JIM_REFERENCE_TAGLEN+1];
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_Jim_Reference           Jim_TAlloc<Jim_Reference>(1,"Jim_Reference") // 

/* -----------------------------------------------------------------------------
 * Exported API prototypes.
 * ---------------------------------------------------------------------------*/

inline void Jim_Obj::setTypePtr(const Jim_ObjType* typeD) { PRJ_TRACE_SETTYPE(this, (typeD)?(typeD->name):(NULL));  typePtr_ = typeD; }

/*
 * Local Variables: ***
 * c-basic-offset: 4 ***
 * tab-width: 4 ***
 * End: ***
 */
END_JIM_NAMESPACE

#ifndef NO_JIM_API // #optionalCode
#  include <jim-api.h>
#endif

#ifdef JIM_INLINE_API_SMALLFUNCS // #optionalCode
#  include "jim-inc.h"
#endif

/* Aborting is not always an option. */
#define JIM_ABORT  abort


// Some "sensors" which could be set and do statics or timing studies.
// Topics Jim_EvalFile(), Jim_AllocStack(), Jim_FreeStack(), 
// Jim_InitHashTable(), Jim_FreeHashTable(), Jim_ExpandHashTable(), numCollisions
// Jim_NewObj(), Jim_FreeObj()
// Jim_CreateInterp(), Jim_FreeInterp()
// Jim_LoadLibrary()
// Jim_Collect()
// CallCmd(), CreateProc(), 
// All alloc/free/realloc numObj of each type