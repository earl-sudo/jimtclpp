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
#include <string.h>
#include <stdint.h>

#include <jim-base.h>
#include <prj_trace.h>

BEGIN_JIM_NAMESPACE


#define JIM_EXPORT

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
    JIM_MAX_CALLFRAME_DEPTH = 1000, /* default max nesting depth for procs #MagicNum */
    JIM_MAX_EVAL_DEPTH = 2000 /* default max nesting depth for eval #MagicNum */
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
    JIM_PATH_LEN = 1024 // #MagicNum
};
#endif

/* Unused arguments generate annoying warnings... */
#define JIM_NOTUSED(V) ((void) V)

#define JIM_LIBPATH "auto_path" // #MagicStr
#define JIM_INTERACTIVE "tcl_interactive" // #MagicStr

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
#include <jim-forwards.h>
#include <jim-alloc.h>

/* -----------------------------------------------------------------------------
 * Stack
 * ---------------------------------------------------------------------------*/

struct Jim_Stack {
private:
    int len_ = 0;
    int maxlen_ = 0;
    VoidPtrArray* vector_ = NULL;
public:

    // len_
    inline int len() const { return len_;  }
    inline void setLen(int v) { len_ = v; }
    inline void lenIncr() { ++len_; }
    inline void lenDecr() { --len_; }
    // maxLen_
    inline int maxlen() const { return maxlen_;  }
    inline void setMaxLen(int v) { maxlen_ = v; }
    // vector_
    inline void setVector(VoidPtrArray* v) { vector_ = v; }
    inline void freeVector() { Jim_TFree<VoidPtrArray>(vector_, "VoidPtrArray"); } // #FreeF
    inline void allocVector() {
        vector_ = Jim_TRealloc<VoidPtrArray>(vector_, maxlen(), "VoidPtrArray"); // AllocF
    }
    inline void* vector(int i) { return vector_[i]; }
    inline void setVector(int i, void* o) { vector_[i] = o; }
};

END_JIM_NAMESPACE

#include <jim-hashtable.h>

BEGIN_JIM_NAMESPACE

// The types of hashtables we use.
const Jim_HashTableType& JimPackageHashTableType();
const Jim_HashTableType& JimAssocDataHashTableType();
const Jim_HashTableType& JimVariablesHashTableType();
const Jim_HashTableType& JimCommandsHashTableType();
const Jim_HashTableType& JimReferencesHashTableType();
const Jim_HashTableType& JimRefMarkHashTableType();


/* ------------------------------- Hashtable funcs -------------------*/
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

// Basic Jim_Obj Types
const Jim_ObjType& dictSubstType();
const Jim_ObjType& interpolatedType();
const Jim_ObjType& stringType();
const Jim_ObjType& comparedStringType();
const Jim_ObjType& sourceType();
const Jim_ObjType& scriptLineType();
const Jim_ObjType& scriptType();
const Jim_ObjType& commandType();
const Jim_ObjType& variableType();
const Jim_ObjType& referenceType();
const Jim_ObjType& intType();
const Jim_ObjType& coercedDoubleType();
const Jim_ObjType& doubleType();
const Jim_ObjType& listType();
const Jim_ObjType& dictType();
const Jim_ObjType& indexType();
const Jim_ObjType& returnCodeType();
const Jim_ObjType& exprType();
const Jim_ObjType& scanFmtStringType();
const Jim_ObjType& getEnumType();
#if JIM_REGEXP
const Jim_ObjType& regexpType();
#endif
// jim-subcmd
const Jim_ObjType& subcmdLookupType();


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
    char* bytes_; /* string representation buffer. NULL = no string repr. */
public:
    inline void copyInterpRep(Jim_ObjPtr srcObj) { 
        // GCC doesn't like simple assignment with internalRep.
        //internalRep = srcObj->internalRep;  
        memcpy(&internalRep, &srcObj->internalRep, sizeof(internalRep));
    }
    inline int length() const { return length_;  }
    inline void lengthDecr(int count = 1) { length_ -= count; }
    inline void lengthIncr(int count = 1) { length_ += count; }
    inline void setLength(int val){ length_ = val;  }

    inline int refCount() const { return refCount_; }
    inline int decrRefCount() { refCount_--; return refCount_; }
    inline int incrRefCount() { refCount_++; return refCount_; }
    inline void setRefCount0() { refCount_ = 0; }

    inline const Jim_ObjType* typePtr() const { return typePtr_;  }
    inline void setTypePtr(const Jim_ObjType* typeD); // forward declared

    inline char* bytes() const { return bytes_;  }
    inline void bytes_setNULL()  { bytes_ = NULL; }
    inline void bytes_NULLterminate() { bytes_[length()] = '\0'; }
    inline char* setBytes(char* str) { bytes_ = str; return bytes_; }
    inline char* setBytes(int index, char ch) { bytes_[index] = ch; ; return bytes_; }
    inline void freeBytes() { free_CharArray(bytes_);  } // #FreeF
    inline void copyBytes(Jim_ObjPtr srcObj) { memcpy(bytes_, srcObj->bytes_, srcObj->length() + 1); }
    inline void copyBytesAt(int pos, const char* str, int len) { memcpy(bytes_ + pos, str, len);  }

    // internalRep.wideValue_.  See g_intObjType.
    jim_wide getWideValue() const { return internalRep.wideValue_; }
    inline bool testTypeRightWide(jim_wide val) const { /* dummy for now */ return true; }
    inline void setWideValue(jim_wide val) { testTypeRightWide(val);  internalRep.wideValue_ = val; }
    inline void incrWideValue() { internalRep.wideValue_++; }
    inline jim_wide wideValue(void) const { testTypeRightWide((jim_wide) 0);  return internalRep.wideValue_; }

    // internalRep.doubleValue_.  See g_coercedDoubleObjType.
    double getDoubleValue() const { return internalRep.doubleValue_; }
    void* getVoidPtr() const { return internalRep.ptr_; }
    inline bool testTypeRightDouble(double v) const { /* dummy for now*/ return true; }
    inline double doubleValue() const { testTypeRightDouble((double) 0);  return internalRep.doubleValue_; }
    inline void setDoubleValue(double val) { testTypeRightDouble(val);  internalRep.doubleValue_ = val; }

    // internalRep.intValue_.
    inline bool testTypeRightInt(int val) const { /* dummy for now */ return true; }
    inline int intValue() const { testTypeRightInt((int)0);  return internalRep.intValue_; }
    inline void setIntValue(int val) { testTypeRightInt(val); internalRep.intValue_ = val; }
    int getIntValue() const { return internalRep.intValue_; }

    // internalRep.ptr_.
    template<typename T>
    inline void setPtr(T val) { internalRep.ptr_ = (void*) val; }

    // internalRep.ptrIntValue_
    //      See getEnumType(), regexpType(), subcmdLookupType().
    template<typename T>
    inline void setPtrInt(T val, int ival) { 
        internalRep.ptrIntValue_.ptr_ = (void*) val; 
        internalRep.ptrIntValue_.int1_ = ival;
    }
    template<typename T>
    inline void setPtrInt2(T val, int ival, int ival2) {
        internalRep.ptrIntValue_.ptr_ = (void*) val;
        internalRep.ptrIntValue_.int1_ = ival;
        internalRep.ptrIntValue_.int2_ = ival2;
    }
    inline void* get_ptrInt_ptr() { return internalRep.ptrIntValue_.ptr_; }
    inline int get_ptrInt_int1() { return internalRep.ptrIntValue_.int1_; }
    inline int get_ptrInt_int2() { return internalRep.ptrIntValue_.int2_; }
    inline void free_ptrInt_ptr() { 
        Jim_TFree<void>(internalRep.ptrIntValue_.ptr_, "void"); // #FreeF
    }

    // internalRep.varValue_.  See variableType().
    inline void setVarValue(unsigned_long callFrameIdD, Jim_Var* varD, int globalD) {
        internalRep.varValue_.callFrameId_ = callFrameIdD;
        internalRep.varValue_.varPtr_ = varD;
        internalRep.varValue_.global_ = globalD;
    }
    inline Jim_Var* get_varValue_ptr() { return internalRep.varValue_.varPtr_;  }
    inline unsigned_long get_varValue_callFrameId() { return internalRep.varValue_.callFrameId_; }
    inline int get_varValue_global() { return internalRep.varValue_.global_; }

    // internalRep.cmdValue_.  See commandType()
    inline void setCmdValue(Jim_ObjPtr nsObjD, Jim_Cmd* cmdD, unsigned_long procEpochD) {
        internalRep.cmdValue_.nsObj_ = nsObjD;
        internalRep.cmdValue_.cmdPtr_ = cmdD;
        internalRep.cmdValue_.procEpoch_ = procEpochD;
    }
    inline void setCmdValueCopy(Jim_ObjPtr srcPtr) {
        // GCC doesn't like simple assignment with internalRep.
        //internalRep.cmdValue_ = srcPtr->internalRep.cmdValue_;
        memcpy(&internalRep.cmdValue_, 
            &srcPtr->internalRep.cmdValue_, 
            sizeof(internalRep.cmdValue_));
    }

    inline Jim_ObjPtr get_cmdValue_nsObj() { return internalRep.cmdValue_.nsObj_; }
    inline Jim_CmdPtr get_cmdValue_cmd() { return internalRep.cmdValue_.cmdPtr_; }
    inline unsigned_long get_procEpoch_cmd() const { return internalRep.cmdValue_.procEpoch_; }

    // internalRep.listValue_.  See listType().
    inline void setListValue(int lenD, int maxLenD, Jim_ObjArray* listObjPtrPtr) {
        internalRep.listValue_.len_ = lenD;
        internalRep.listValue_.maxLen_ = maxLenD;
        internalRep.listValue_.ele_ = listObjPtrPtr;
    }
    inline int get_listValue_len() const { return internalRep.listValue_.len_; }
    inline Jim_ObjPtr get_listValue_objArray(int i) { return internalRep.listValue_.ele_[i]; }
    inline void set_listValue_objArray(int i, Jim_ObjPtr o) { internalRep.listValue_.ele_[i] = o; }
    inline int get_listValue_maxLen() const { return internalRep.listValue_.maxLen_; }
    inline void setListValueLen(int val) { internalRep.listValue_.len_ = val;  }
    inline void setListValueMaxLen(int val) { internalRep.listValue_.maxLen_ = val; }
    inline void incrListValueLen(int val) { internalRep.listValue_.len_ += val; }
    inline void free_listValue_ele() { free_Jim_ObjArray(internalRep.listValue_.ele_);  }  // #FreeF
    inline void copy_listValue_ele(Jim_ObjPtr srcObj) {
        memcpy(internalRep.listValue_.ele_, srcObj->internalRep.listValue_.ele_,
               sizeof(Jim_ObjPtr) * srcObj->get_listValue_len());
    }
    inline void resize_listValue_ele(int len) {
        internalRep.listValue_.ele_ =
            realloc_Jim_ObjArray(get_listValue_ele(), len);
    }
    inline Jim_ObjArray* get_listValue_ele() { return internalRep.listValue_.ele_; }

    // internalRep.strValue_.  See stringType().
    inline void setStrValue(int maxLenD, int charLenD) {
        internalRep.strValue_.maxLength_ = maxLenD;
        internalRep.strValue_.charLength_ = charLenD;
    }
    inline int get_strValue_charLen() const { return internalRep.strValue_.charLength_;  }
    inline void setStrValue_charLen(int len) { internalRep.strValue_.charLength_ = len; }
    inline void incrStrValue_charLen(int len) { internalRep.strValue_.charLength_ += len; }
    inline int get_strValue_maxLen() const { return internalRep.strValue_.maxLength_; }
    inline void setStrValue_maxLen(int len) { internalRep.strValue_.maxLength_ = len; }

    // internalRep.refValue_.  See referenceType()
    inline void setRefValue(unsigned_long idD, Jim_Reference* refD) {
        internalRep.refValue_.id_ = idD;
        internalRep.refValue_.refPtr_ = refD;
    }
    inline unsigned_long get_refValue_id() const { return internalRep.refValue_.id_;  }
    inline unsigned_long* get_refValue_idPtr() { return &internalRep.refValue_.id_; }
    inline Jim_ReferencePtr get_refValue_ref() { return internalRep.refValue_.refPtr_; }

    // internalRep.sourceValue_.  See sourceType().
    inline void set_sourceValue(Jim_ObjPtr fileNameD, int lineNumD) {
        internalRep.sourceValue_.fileNameObj_ = fileNameD;
        internalRep.sourceValue_.lineNumber_ = lineNumD;
    }
    inline Jim_ObjPtr get_sourceValue_fileName() { return internalRep.sourceValue_.fileNameObj_; }
    inline int get_sourceValue_lineNum() const { return internalRep.sourceValue_.lineNumber_; }
    inline void copy_sourceValue(Jim_ObjPtr from) { 
        // GCC doesn't like simple assignment with internalRep.
        //internalRep.sourceValue_ = from->internalRep.sourceValue_;  
        memcpy(&internalRep.sourceValue_, 
            &from->internalRep.sourceValue_, 
            sizeof(internalRep.sourceValue_));
    }

    // internalRep.dictSubstValue_.  See dictSubstType().
    inline void setDictSubstValue(Jim_ObjPtr varName, Jim_ObjPtr indexObj) {
        internalRep.dictSubstValue_.varNameObjPtr_ = varName;
        internalRep.dictSubstValue_.indexObjPtr_ = indexObj;
    }
    inline Jim_ObjPtr get_dictSubstValue_varName() { return internalRep.dictSubstValue_.varNameObjPtr_;  }
    inline Jim_ObjPtr get_dictSubstValue_index() { return internalRep.dictSubstValue_.indexObjPtr_; }
    inline Jim_ObjPtr& get_dictSubstValue_indexRef() { return internalRep.dictSubstValue_.indexObjPtr_; }

    // internalRep.scriptLineValue_.  See scriptLineType().
    inline void setScriptLineValue(int argcD, int lineD) {
        internalRep.scriptLineValue_.line = lineD;
        internalRep.scriptLineValue_.argc = argcD;
    }
    inline int get_scriptLineValue_argc() const { return internalRep.scriptLineValue_.argc; }
    inline int get_scriptLineValue_line() const { return internalRep.scriptLineValue_.line; }

  private:
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
            void *ptr1_; 
            void *ptr2_;
        } twoPtrValue_; /* #UNUSED */
        /* Generic pointer, int, int value */
        struct {
            // Used by the "regular expression" and "jim enum" code.  
            //      See getEnumType(), regexpType(), subcmdLookupType().
            void *ptr_;
            int int1_;
            int int2_;
        } ptrIntValue_;
        /* Variable object */
        struct {
            // Used by Jim_Var code. See variableType().
             Jim_Var *varPtr_;
             unsigned_long callFrameId_; /* for caching */
             int global_; /* If the variable name is globally scoped with :: */
        } varValue_;
        /* Command object */
        struct {
            // Used by command code. See commandType().
            Jim_ObjPtr nsObj_;
            Jim_Cmd *cmdPtr_;
            unsigned_long procEpoch_; /* for caching */
        } cmdValue_;
        /* List object */
        struct {
            // Used by list code.  See listType().
            Jim_ObjArray* ele_;    /* Elements vector */
            int len_;        /* Length */
            int maxLen_;        /* Allocated 'ele' length */
        } listValue_;
        /* String type */
        struct {
            // Used by string code. See stringType().
            int maxLength_;
            int charLength_;     /* utf-8 char length. -1 if unknown */
         } strValue_;
        /* Reference type */
        struct {
            // Use by reference code.  See referenceType().
            unsigned_long id_;
            Jim_Reference *refPtr_;
        } refValue_;
        /* Source type */
        struct {
            // Used by "source" code.  See sourceType().
            Jim_ObjPtr fileNameObj_;
            int lineNumber_;
        } sourceValue_;
        /* Dict substitution type */
        struct {
            // Use by "dictionary subst" code. see dictSubstType().
            Jim_ObjPtr varNameObjPtr_;
            Jim_ObjPtr indexObjPtr_;
        } dictSubstValue_;
        struct {
            // Used by scriptLine code. See scriptLineType().
            int line;
            int argc;
        } scriptLineValue_;
    } internalRep;

    /* These fields add 8 or 16 bytes more for every object
     * but this is required for efficient garbage collection
     * of Jim references. */
     // #TODO prevObjPtr_ and nextObjPtr_ still has naked references. See hashtag JO_access.
    Jim_ObjPtr prevObjPtr_ = NULL; /* pointer to the prev object. */
    Jim_ObjPtr nextObjPtr_ = NULL; /* pointer to the next object. */
  public:

    inline Jim_ObjPtr  nextObjPtr() const { return nextObjPtr_; }
    inline Jim_ObjPtr  prevObjPtr() const { return prevObjPtr_;  }

    inline void setNextObjPtr(Jim_ObjPtr o) { nextObjPtr_ = o;  }
    inline void setPrevObjPtr(Jim_ObjPtr o) { prevObjPtr_ = o; }

} Jim_Obj;

/* Jim_Obj related macros */

void Jim_FreeObj (Jim_InterpPtr interp, Jim_ObjPtr objPtr); /* #Review */
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
    const char *name_ = NULL; /* The name of the type. */
    Jim_FreeInternalRepProc *freeIntRepProc = NULL;
    Jim_DupInternalRepProc *dupIntRepProc = NULL;
    Jim_UpdateStringProc *updateStringProc = NULL;
    int flags_ = 0;

    inline const char* getName() const { return name_; }
    inline int getFlags() const { return flags_; }
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
    Jim_CallFramePtr parent_ = NULL; /* The parent callframe */
    Jim_ObjConstArray argv_ = NULL; /* object vector of the current procedure call. */
    int argc_ = 0; /* number of args of the current procedure call. */
    Jim_ObjPtr procArgsObjPtr_ = NULL; /* arglist_ object of the running procedure */
    Jim_ObjPtr procBodyObjPtr_ = NULL; /* body object of the running procedure */
    Jim_CallFramePtr next_ = NULL; /* Callframes are in a linked list */
    Jim_ObjPtr nsObj_ = NULL;             /* Namespace for this proc call frame */
    Jim_ObjPtr fileNameObj_ = NULL;       /* file and line of caller of this proc (if available) */
    int line_;
    Jim_StackPtr localCommands_ = NULL; /* commands to be destroyed when the call frame is destroyed */
    Jim_ObjPtr tailcallObj_ = NULL;  /* Pending tailcall invocation */
    Jim_Cmd* tailcallCmd_ = NULL;  /* Resolved command for pending tailcall invocation */
public:
    // argv_
    inline Jim_ObjConstArray argv() { return argv_; }
    inline void setArgv(Jim_ObjConstArray o) { argv_ = o; }
    inline Jim_ObjPtr argv(int i) { return argv_[i]; }
    // id_
    inline unsigned_long id() const { return id_; }
    inline void setId(unsigned_long v) { id_ = v; }
    // level_
    inline int level() const { return level_;  }
    inline void setLevel(int v) { level_ = v; }
    // vars_
    inline Jim_HashTable& vars() { return vars_; }
    // staticVars_
    inline Jim_HashTablePtr  staticVars() { return staticVars_; }
    inline void setStaticVars(Jim_HashTablePtr o) { staticVars_ = o; }
    // parent_
    inline Jim_CallFramePtr  parent() { return parent_; }
    inline void setParent(Jim_CallFramePtr o) { parent_ = o; }
    // argc_
    inline int argc() const { return argc_; }
    inline void setArgc(int v) { argc_ = v; }
    // procArgsObjPtr_
    inline Jim_ObjPtr  procArgsObjPtr() const { return procArgsObjPtr_; }
    inline void setProcArgsObjPtr(Jim_ObjPtr o) { procArgsObjPtr_ = o; }
    // procBodyObjPtr_
    inline Jim_ObjPtr  procBodyObjPtr() const { return procBodyObjPtr_;  }
    inline void setProcBodyObjPtr(Jim_ObjPtr o) { procBodyObjPtr_ = o; }
    // nsObj_
    inline Jim_ObjPtr  nsObj() const { return nsObj_;  }
    inline void setNsObj(Jim_ObjPtr o) { nsObj_ = o; }
    // localCommands_
    inline Jim_StackPtr  localCommands() const { return localCommands_; }
    inline void setLocalCommands(Jim_StackPtr o) { localCommands_ = o; }
    // next_
    inline Jim_CallFramePtr next() { return next_; }
    inline void setNext(Jim_CallFramePtr o) { next_ = o; }
    // fileNameObj_
    inline Jim_ObjPtr fileNameObj() { return fileNameObj_; }
    inline void setFileNameObj(Jim_ObjPtr o) { fileNameObj_ = o; }
    // line
    inline int line() const { return line_; }
    inline void setLine(int v) { line_ = v; }
    // tailcallObj_
    inline Jim_ObjPtr tailcallObj() { return tailcallObj_;  }
    inline void setTailcallObj(Jim_ObjPtr o) { tailcallObj_ = o; }
    // tailcallCmd_
    inline Jim_CmdPtr tailcallCmd() { return tailcallCmd_; }
    inline void setTailcallCmd(Jim_CmdPtr o) { tailcallCmd_ = o; }
} Jim_CallFrame;

/* The var structure. It just holds the pointer of the referenced
 * object. If linkFramePtr_ is not NULL the variable is a link
 * to a variable of name stored in objPtr living in the given callframe
 * (this happens when the [global] or [upvar] command is used).
 * The interp in order to always know how to free the Jim_Obj associated
 * with a given variable because in Jim objects memory management is
 * bound to interpreters. */
struct Jim_Var {
private:
    Jim_ObjPtr objPtr_ = NULL; 
    Jim_CallFramePtr linkFramePtr_ = NULL; 
public:
    inline Jim_CallFramePtr linkFramePtr() { return linkFramePtr_; }
    inline void setLinkFramePtr(Jim_CallFramePtr o) { linkFramePtr_ = o; }
    inline Jim_ObjPtr objPtr() { return objPtr_; }
    inline void setObjPtr(Jim_ObjPtr o) { objPtr_ = o; }
};

/* The cmd structure. */
typedef int Jim_CmdProc(Jim_InterpPtr interp, int argc,
    Jim_ObjConstArray argv);
typedef void Jim_DelCmdProc(Jim_InterpPtr interp, void *privData);



/* A command is implemented in C if isproc is 0, otherwise
 * it is a Tcl procedure with the arglist_ and body represented by the
 * two objects referenced by arglistObjPtr and bodyObjPtr_. */

struct Jim_ProcArg {
private:
    Jim_ObjPtr nameObjPtr_ = NULL;    /* Name of this arg */
    Jim_ObjPtr defaultObjPtr_ = NULL; /* Default value, (or rename for $args) */
public:
    inline Jim_ObjPtr nameObjPtr() { return nameObjPtr_; }
    inline void setNamedObjPtr(Jim_ObjPtr o) { nameObjPtr_ = o; }
    inline Jim_ObjPtr defaultObjPtr() { return defaultObjPtr_; }
    inline void setDefaultObjPtr(Jim_ObjPtr o) { defaultObjPtr_ = o; }
};

struct Jim_Cmd {
private:
    int isproc_ = 0;          /* Is this a procedure? */
    Jim_Cmd *prevCmd_ = NULL;    /* Previous command defn if cmd created 'local' */
    int inUse_ = 0;           /* Reference count */

    union {
        struct {
            /* native (C) command */
            Jim_CmdProc *cmdProc_ = NULL; /* The command implementation */
            Jim_DelCmdProc *delProc_ = NULL; /* Called when the command is deleted if != NULL */
            void *privData_ = NULL; /* command-private data available via Jim_CmdPrivData() */
        } native_;
        struct {
            /* Tcl procedure */
            Jim_ObjPtr argListObjPtr_ = NULL;
            Jim_ObjPtr bodyObjPtr_ = NULL;
            Jim_HashTablePtr staticVars_ = NULL;  /* Static vars hash table. NULL if no statics. */
            int argListLen_ = 0;             /* Length of argListObjPtr_ */
            int reqArity_ = 0;               /* Number of required parameters */
            int optArity_ = 0;               /* Number of optional parameters */
            int argsPos_ = 0;                /* Position of 'args', if specified, or -1 */
            int upcall_ = 0;                 /* True if proc is currently in upcall_ */
	        Jim_ProcArg *arglist_ = NULL;
            Jim_ObjPtr nsObj_ = NULL;             /* Namespace for this proc */
        } proc_;
    } u;
public:

    // inUse_
    inline int inUse() const { return inUse_; }
    inline int incrInUse() { inUse_++; return inUse_; }
    inline int decrInUse() { inUse_--; return inUse_; }
    inline void setInUse(int v) { inUse_ = v; }
    // isproc_
    inline int isproc() const { return isproc_; }
    inline void setIsProc(int v) { isproc_ = v; }
    // prevCmd_
    inline Jim_CmdPtr prevCmd() const { return prevCmd_; }
    inline void setPrevCmd(Jim_CmdPtr o) { prevCmd_ = o; }

    // native_.cmdProc_
    inline Jim_CmdProc* cmdProc() { return u.native_.cmdProc_; }
    inline void  setCmdProc(Jim_CmdProc* cmdD) { u.native_.cmdProc_ = cmdD; }
    // native_.delProc_
    inline Jim_DelCmdProc* delProc() { return u.native_.delProc_; }
    inline void setDelProc(Jim_DelCmdProc* delProc) { u.native_.delProc_ = delProc; }
    // native_.privData_
    template<typename T> 
    inline T getPrivData() { return (T) u.native_.privData_; }
    template<typename T>
    inline void setPrivData(T v) { u.native_.privData_ = (void*) v; }
    // u.proc_.upcall_
    inline void proc_incrUpcall() { u.proc_.upcall_++;  }
    inline void proc_decrUpcall() { u.proc_.upcall_--; }
    inline int proc_upcall() const { return u.proc_.upcall_; }
    // u.proc_.argsPos_
    inline int proc_argsPos() const { return u.proc_.argsPos_; }
    inline void proc_setArgsPos(int val) { u.proc_.argsPos_ = val; }
    // u.proc_.optArity_
    inline int proc_optArity() const { return u.proc_.optArity_; }
    inline void proc_incrOptArity() { u.proc_.optArity_++;  }
    // u.proc_.reqArity_
    inline int proc_regArity() const { return u.proc_.reqArity_;  }
    inline void proc_incrRegArity() { u.proc_.reqArity_++;  }
    // u.proc_.argListLen_
    inline int proc_argListLen() const { return u.proc_.argListLen_; }
    inline void proc_setArgListLen(int val) { u.proc_.argListLen_ = val; }
    // u.proc_.nsObj_
    inline Jim_ObjPtr proc_nsObj() { return u.proc_.nsObj_; }
    inline void proc_setNsObj(Jim_ObjPtr o) { u.proc_.nsObj_ = o; }
    // u.proc_.argListObjPtr_
    inline Jim_ObjPtr proc_argListObjPtr() { return u.proc_.argListObjPtr_; }
    inline void proc_setArgListObjPtr(Jim_ObjPtr o) { u.proc_.argListObjPtr_ = o; }
    // u.proc_.bodyObjPtr_
    inline Jim_ObjPtr proc_bodyObjPtr() { return u.proc_.bodyObjPtr_; }
    inline void proc_setBodyObjPtr(Jim_ObjPtr o) { u.proc_.bodyObjPtr_ = o;  }
    // u.proc_.staticVars_
    inline Jim_HashTablePtr proc_staticVars() { return u.proc_.staticVars_; }
    inline void proc_setStaticVars(Jim_HashTablePtr o) { u.proc_.staticVars_ = o; }
    inline void proc_freeStaticVars() { free_Jim_HashTable(u.proc_.staticVars_); }
    // u.proc_arglist
    inline Jim_ProcArg* proc_arglist() { return u.proc_.arglist_; }
    inline void proc_setArglist(Jim_ProcArg* o) { u.proc_.arglist_ = o; }
    inline Jim_ProcArg& proc_arglist(int i) { return u.proc_.arglist_[i]; }
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
    int (*signal_set_result_)(Jim_InterpPtr interp, jim_wide sigmaskD) = NULL; /* Set a result for the sigmask */
    unsigned_long lastCollectId_ = 0; /* reference max Id of the last GC
                execution. It's set to ~0 while the collection
                is running as sentinel to avoid to recursive
                calls via the [collect] command inside
                finalizers. */
    Jim_ObjPtr emptyObj_ = NULL; /* Shared empty string object. */
    Jim_CallFramePtr  topFramePtr_ = NULL; /* toplevel/global frame pointer. */
private:
    Jim_HashTable assocData_; /* per-interp storage for use by packages */
    Jim_ObjPtr freeList_ = NULL; /* Linked list of all the unused objects. */
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
    Jim_CallFramePtr  framePtr_ = NULL;    /* Pointer to the current call frame */
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
    Jim_ObjPtr currentScriptObj_ = NULL; /* Script currently in execution. */
    Jim_ObjPtr nullScriptObj_ = NULL; /* script representation of an empty string */
    Jim_ObjPtr trueObj_ = NULL; /* Shared true int object. */
    Jim_ObjPtr falseObj_ = NULL; /* Shared false int object. */
    unsigned_long referenceNextId_ = 0; /* Next id for reference. */
    Jim_HashTable references_; /* References hash table. */
    time_t lastCollectTime_ = 0; /* Unix time of the last GC execution */
    Jim_ObjPtr stackTrace_ = NULL; /* Stack trace object. */
    Jim_ObjPtr errorProc_ = NULL; /* Name of last procedure which returned an error */
    Jim_ObjPtr unknown_ = NULL; /* Unknown command cache */
    int unknown_called_ = 0; /* The unknown command has been invoked */
    int errorFlag_ = 0; /* Set if an error occurred during execution. */
    void* cmdPrivData_ = NULL; /* Used to pass the private data pointer to
                  a command. It is set to what the user specified
                  via Jim_CreateCommand(). */
    Jim_CallFramePtr  freeFramesList_ = NULL; /* list of CallFrame structures. */
    Jim_PrngState* prngState_; /* per interpreter Random Number Gen. state. */
    Jim_HashTable packages_; /* Provided packages hash table */
    Jim_StackPtr loadHandles_; /* handles of loaded modules [load] UNUSED */

public:
    // assocData_
    Jim_HashTablePtr assocDataPtr() { return &assocData_; }
    // packages_
    inline Jim_HashTablePtr getPackagesPtr() { return &packages_; }
    // signal_set_result_
    inline void* get_signal_set_result() { return (void*)signal_set_result_; }
    // prngState_
    inline Jim_PrngState* prngState() { return prngState_; }
    inline void prngStateAlloc() { prngState_ = Jim_TAlloc<Jim_PrngState>(1, "Jim_PrngState"); } // #AllocF
    inline void prngStateFree() { free_Jim_PrngState(prngState_); } // #FreeF
    // freeFramesList_
    inline Jim_CallFramePtr freeFramesList() { return freeFramesList_; }
    inline void setFreeFramesList(Jim_CallFramePtr o) { freeFramesList_ = o; }
    // cmdPrivData_
    inline void* cmdPrivData() { return cmdPrivData_; }
    inline void setCmdPrivData(void* ptr) { cmdPrivData_ = ptr; }
    // errorFlag_
    inline int errorFlag() const { return errorFlag_; }
    inline void setErrorFlag(int val) { errorFlag_ = val; }
    // unknown_
    inline Jim_ObjPtr unknown() { return unknown_; }
    inline void setUnknown(Jim_ObjPtr o) { unknown_ = o; }
    // unknown_called_
    inline int unknown_called() const { return unknown_called_;  }
    inline void incrUnknown_called() { unknown_called_++;  }
    inline void decrUnknown_called() { unknown_called_--; }
    // errorProc_
    inline Jim_ObjPtr& errorProc() { return errorProc_;  }
    inline void setErrorProc(Jim_ObjPtr o) { errorProc_ = o; }
    // stackTrace_
    inline Jim_ObjPtr& stackTrace() { return stackTrace_; }
    inline void setStackTrace(Jim_ObjPtr o) { stackTrace_ = o; }
    // lastCollectId_
    const unsigned_long lastCollectId_resetVal = ~0;
    inline unsigned_long lastCollectId() const { return lastCollectId_; }
    inline void setLastCollectedId(unsigned_long val) { lastCollectId_ = val; }
    inline void resetLastCollectedId() { lastCollectId_ = lastCollectId_resetVal; }
    // references_
    inline Jim_HashTable& references() { return references_;  }
    inline Jim_HashTablePtr referencesPtr() { return &references_;  }
    // liveList_
    inline Jim_ObjPtr liveList() { return liveList_; }
    inline void setLiveList(Jim_ObjPtr o) { liveList_ = o; }
    // freeList_
    inline Jim_ObjPtr freeList() { return freeList_; }
    inline void setFreeList(Jim_ObjPtr o) { freeList_ = o; }
    inline void freeFreeList() { free_Jim_Obj(freeList_); }
    // commands_
    inline Jim_HashTable& commands() { return commands_; }
    inline Jim_HashTablePtr commandsPtr() { return &commands_; }
    // returnLevel_
    inline int returnLevel() const { return returnLevel_; }
    inline int decrReturnLevel() { return (--returnLevel_); }
    inline int incrReturnLevel() { return (++returnLevel_); }
    inline void setReturnLevel(int val) { returnLevel_ = val;  }
    // result_
    inline void setResult(Jim_ObjPtr obj) { result_ = obj;  }
    inline Jim_ObjPtr  result() const { return result_; }
    // errorLine_
    inline int errorLine() const { return errorLine_;  }
    inline void setErrorLine(int val) { errorLine_ = val; }
    // errorFileNameObj_
    inline Jim_ObjPtr  errorFileNameObj() const { return errorFileNameObj_; }
    inline void setErrorFileNameObj(Jim_ObjPtr o) { errorFileNameObj_ = o; }
    // addStackTrace_
    inline int addStackTrace() const { return addStackTrace_; }
    inline void incrAddStackTrace() { addStackTrace_++;  }
    inline void setAddStackTrace(int val) { addStackTrace_ = val; }
    // maxCallFrameDepth_
    inline int maxCallFrameDepth() const { return maxCallFrameDepth_;  }
    inline void setMaxCallFrameDepth(int val) { maxCallFrameDepth_ = val; }
    // maxEvalDepth_
    inline int maxEvalDepth() const { return maxEvalDepth_;  }
    inline void setMaxEvalDepth(int val) { maxEvalDepth_ = val; }
    // evalDepth_
    inline int evalDepth() const { return evalDepth_; }
    inline void incrEvalDepth() { evalDepth_++; }
    inline void decrEvalDepth() { evalDepth_--; }
    // returnCode_
    inline int returnCode() const { return returnCode_; }
    inline void setReturnCode(int val) { returnCode_ = val; }
    // exitCode_
    inline int exitCode() const { return exitCode_; }
    inline void setExitCode(int val) { exitCode_ = val; }
    // id_
    inline long id() const { return id_;  }
    inline long incrId() { return (++id_); }
    // signal_level_
    inline void incrSignalLevel(int sig) { signal_level_ += sig;  }
    inline void decrSignalLevel(int sig) { signal_level_ -= sig;  }
    inline int signal_level() const { return signal_level_; }
    // sigmask_
    inline jim_wide getSigmask() { return sigmask_; }
    inline jim_wide* getSigmaskPtr() { return &sigmask_; }
    inline void setSigmask(jim_wide val) { sigmask_ = val; }
    inline void orSigmask(jim_wide val) { sigmask_ |= val; }
    // framePtr_
    inline Jim_CallFramePtr  framePtr() const { return framePtr_; }
    inline void framePtr(Jim_CallFramePtr  val) { framePtr_ = val; }
    // topFramePtr_
    inline Jim_CallFramePtr  topFramePtr() const { return topFramePtr_; }
    inline void topFramePtr(Jim::Jim_CallFramePtr  val) { topFramePtr_ = val; }
    // procEpoch_
    inline unsigned_long procEpoch() const { return procEpoch_; }
    inline void procEpoch(unsigned_long val) { procEpoch_ = val; }
    // callFrameEpoch_
    inline unsigned_long callFrameEpoch() const { return callFrameEpoch_; }
    inline void callFrameEpoch(unsigned_long val) { callFrameEpoch_ = val; }
    inline void incrCallFrameEpoch() { callFrameEpoch_++; }
    // local_
    inline int local() const { return local_; }
    inline void incrLocal() { local_++; }
    inline void decrLocal() { local_--; }
    // currentScriptObj_
    inline Jim_ObjPtr  currentScriptObj() const { return currentScriptObj_; }
    inline void currentScriptObj(Jim::Jim_ObjPtr  val) { currentScriptObj_ = val; }
    // nullScriptObj_
    inline Jim_ObjPtr  nullScriptObj() const { return nullScriptObj_; }
    inline void nullScriptObj(Jim::Jim_ObjPtr  val) { nullScriptObj_ = val; }
    // emptyObj_
    inline Jim_ObjPtr  emptyObj() { return emptyObj_; }
    inline void emptyObj(Jim::Jim_ObjPtr  val) { emptyObj_ = val; }
    // trueObj_
    inline Jim_ObjPtr  trueObj() const { return trueObj_; }
    inline void trueObj(Jim::Jim_ObjPtr  val) { trueObj_ = val; }
    // falseObj_
    inline Jim_ObjPtr  falseObj() const { return falseObj_; }
    inline void falseObj(Jim::Jim_ObjPtr  val) { falseObj_ = val; }
    // referenceNextId_
    inline unsigned_long referenceNextId() const { return referenceNextId_;  }
    inline unsigned_long incrReferenceNextId() { return (referenceNextId_++); }
    // lastCollectTime_
    inline time_t lastCollectTime() const { return lastCollectTime_; }
    inline void lastCollectTime(time_t val) { lastCollectTime_ = val; }
};

/* Free the internal representation of the object. */
JIM_API_INLINE void Jim_FreeIntRep(Jim_InterpPtr  i, Jim_ObjPtr  o);

/* Currently provided as macro that performs the increment.
 * At some point may be a real function doing more work.
 * The proc epoch is used in order to know when a command lookup
 * cached can no longer considered valid. */

/* Note: Using trueObj and falseObj here makes some things slower...*/

/* Use this for file handles, etc. which need a unique id */
JIM_API_INLINE long Jim_GetId(Jim_InterpPtr  i);

/* Reference structure. The interpreter pointer is held within privdata member in HashTable */
enum {
     JIM_REFERENCE_TAGLEN = 7 /* The tag is fixed-length, because the reference
                                  string representation must be fixed length. */
};

struct Jim_Reference {
private:
    Jim_ObjPtr objPtr_ = NULL;
    Jim_ObjPtr finalizerCmdNamePtr_ = NULL;
    char tag_[JIM_REFERENCE_TAGLEN+1];
public:
    inline Jim_ObjPtr finalizerCmdNamePtr() { return finalizerCmdNamePtr_; }
    inline void setfinalizerCmdNamePtr(Jim_ObjPtr o) { finalizerCmdNamePtr_ = o; }
    inline Jim_ObjPtr objPtr() { return objPtr_; }
    inline void setObjPtr(Jim_ObjPtr o) { objPtr_ = o; }
    inline const char* tag() const { return tag_; }
    inline void setTagChar(int i, char c) { tag_[i] = c; }
};

/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_Jim_Reference           Jim_TAlloc<Jim_Reference>(1,"Jim_Reference") // 

/* -----------------------------------------------------------------------------
 * Exported API prototypes.
 * ---------------------------------------------------------------------------*/

inline void Jim_Obj::setTypePtr(const Jim_ObjType* typeD) { PRJ_TRACE_SETTYPE(this, (typeD)?(typeD->getName()):(NULL));  typePtr_ = typeD; }

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

// Mark ignore return
#define IR (void)