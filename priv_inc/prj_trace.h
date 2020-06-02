#pragma once
// This is designed to be a totally optional illumination facility.  It is much like debugging code but left
// out-of-line so not to dirty up the code.  It can be used to help understand how code works or to debug 
// issues.  You could think of it as saved break points.
#include <stddef.h>
#include <stdint.h>

#include <exception>

struct prj_trace {
    // TODO Subject: Stack, Tcl_Obj, References, types, call/return, var-sizes/num, numCalls/timeCalls
    enum ACTIONS {
        // Function trace actions
        ENTER_FUNC, EXIT_FUNC, EXCEPTION_EXIT_FUNC,
        // Heap change actions
        ALLOC_MEM, FREE_MEM, REALLOC_MEM,
        // Jim_Obj actions
        ACTION_SETTYPE, // Setting a type of a Jim_Obj
        // Hash table actions
        ACTION_HT_CREATE, ACTION_HT_DELETE, ACTION_HT_RESIZE_PRE, ACTION_HT_RESIZE_POST, ACTION_HT_STATS,
        // Command actions
        ACTION_CMD_CREATE, ACTION_CMD_INVOKE, ACTION_CMD_DELETE,
        // tcl proc actions
        ACTION_PROC_CREATE, ACTION_PROC_INVOKE, ACTION_PROC_DELETE,
        // tcl callframe actions
        ACTION_CALLFRAME_CREATE, ACTION_CALLFRAME_DELETE,
        // tcl interpreter actions
        ACTION_INTERP_CREATE, ACTION_INTERP_DELETE, ACTION_INTERP_STATS,
        // tcl expression actions
        ACTION_EXPR_PRE, ACTION_EXPR_POST,
        // jim reference actions
        ACTION_REFERNCE_CREATE,
        // jim garbage collection actions
        ACTION_COLLECT_PRE, ACTION_COLLECT_POST,
    };
    struct HT_Stats {
        int32_t uniq_;
        int32_t size_;
        int32_t used_;
        int32_t collisions_;
        HT_Stats(int uniqD, int32_t sizeD, int32_t usedD, int32_t collisionsD) 
            : uniq_(uniqD), size_(sizeD), used_(usedD), collisions_(collisionsD) { }
    };
    struct Interp_Stats {
        int maxCallFrameDepth_ = 0;     /* Used for infinite loop detection. */
        int maxEvalDepth_ = 0;          /* Used for infinite loop detection. */
        int evalDepth_ = 0;             /* Current eval depth */
        int returnLevel_ = 0;           /* Current level_ of 'return -level_' */
    };

    // Callback used with heap actions.
    typedef void (*prj_traceMemCb)(int action, const char* type, int sz, void* ptr, void* ptr2);
    // Callback use with function trace.
    typedef void (*prj_traceCb)(int action, const char* funcName, int stackDepth,
                                const char* obj1Name, void* obj1,
                                const char* obj2Name, void* obj2);
    typedef void (*prj_traceActCb)(int action, const char* str, void* ptr, void* ptr2);

    static prj_traceCb logFunc_;
    static prj_traceMemCb memFunc_;
    static prj_traceActCb actionFunc_;

    // If trace is on the current stack depth.
    static int stackDepth_;
    // If trace is on the current function name.
    const char* funcName_;

    // Called on function entry
    inline prj_trace(const char* funcName) : funcName_(funcName) {
        stackDepth_++;
        if (logFunc_) logFunc_(ENTER_FUNC, funcName_, stackDepth_, "", nullptr, "", nullptr);
    }
    // Called on function exit
    inline ~prj_trace(void) {
        if (std::uncaught_exceptions()) {
            // If we left a function because of an exception MAYBE show that.
            if (logFunc_) logFunc_(EXCEPTION_EXIT_FUNC, funcName_, stackDepth_, "", nullptr, "", nullptr);
        } else {
            if (logFunc_) logFunc_(EXIT_FUNC, funcName_, stackDepth_, "", nullptr, "", nullptr);
        }
        stackDepth_--;
    }
};


//#define USE_PRJ_TRACE 1 // #Debug

#ifdef USE_PRJ_TRACE
// When trace is on 
#    define PRJ_TRACEMEM_ALLOC(NAME,SZ,PTR) do { if (::prj_trace::memFunc_) { ::prj_trace::memFunc_(::prj_trace::ALLOC_MEM, NAME, SZ, PTR, nullptr); } } while(0)
#    define PRJ_TRACEMEM_FREE(NAME,PTR)     do { if (::prj_trace::memFunc_) { ::prj_trace::memFunc_(::prj_trace::FREE_MEM, NAME, 0, PTR, nullptr); } } while(0)
#    define PRJ_TRACEMEM_REALLOC(NAME,SZ,OLDPTR,NEWPTR) do { if (::prj_trace::memFunc_) { ::prj_trace::memFunc_(::prj_trace::REALLOC_MEM, NAME, SZ, OLDPTR, NEWPTR); } } while(0)
#  ifdef __GNUC__
#    define PRJ_TRACE prj_trace   prj_traceObj(__PRETTY_FUNCTION__)
#  else // GGC/G++, Mingw(GCC for Windows) and Clang
#    define PRJ_TRACE prj_trace   prj_traceObj(__FUNCSIG__)
#  endif
#  define PRJ_TRACE_SETTYPE(jim_objPtr, TYPENAME) do { if (::prj_trace::actionFunc_) { ::prj_trace::actionFunc_(::prj_trace::ACTION_SETTYPE, TYPENAME, (void*)jim_objPtr, nullptr);} } while(0)

// HashTable tracing
#  define PRJ_TRACE_HT(ACTION, NAME, HTPTR) do { if (::prj_trace::actionFunc_) {    \
    ::prj_trace::actionFunc_(ACTION, NAME, HTPTR,                                   \
    nullptr);                             \
} } while(0)
#  define PRJ_TRACE_GEN(ACTION, NAME, PTR1, PTR2) do { if (::prj_trace::actionFunc_) { ::prj_trace::actionFunc_(ACTION, NAME, (void*)PTR1, (void*)PTR2);} } while(0)
#else
// When trace is off.
#  define PRJ_TRACE
#  define PRJ_TRACEMEM_ALLOC(NAME,SZ,PTR) 
#  define PRJ_TRACEMEM_FREE(NAME,PTR) 
#  define PRJ_TRACEMEM_REALLOC(NAME,SZ,OLDPTR,NEWPTR) 
#  define PRJ_TRACE_SETTYPE(jim_objPtr, TYPENAME)
#  define PRJ_TRACE_HT(ACTION, NAME, HTPTR) 
#  define PRJ_TRACE_GEN(ACTION, NAME, PTR1, PTR2)
#endif