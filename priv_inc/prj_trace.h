#pragma once

#include <stddef.h>
#include <stdint.h>

#include <exception>

struct prj_trace {
    // TODO Subject: Stack, Tcl_Obj, References, types, call/return, var-sizes/num, numCalls/timeCalls
    enum ACTIONS {
        ENTER_FUNC, EXIT_FUNC, EXCEPTION_EXIT_FUNC,
        ALLOC_MEM, FREE_MEM, REALLOC_MEM,
        ACTION_SETTYPE,
        ACTION_HT_CREATE, ACTION_HT_DELETE, ACTION_HT_RESIZE_PRE, ACTION_HT_RESIZE_POST, ACTION_HT_STATS,
        ACTION_CMD_CREATE, ACTION_CMD_INVOKE, ACTION_CMD_DELETE,
        ACTION_PROC_CREATE, ACTION_PROC_INVOKE, ACTION_PROC_DELETE,
        ACTION_CALLFRAME_CREATE, ACTION_CALLFRAME_DELETE,
        ACTION_INTERP_CREATE, ACTION_INTERP_DELETE, ACTION_INTERP_STATS,
        ACTION_EXPR_PRE, ACTION_EXPR_POST,
        ACTION_REFERNCE_CREATE,
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
    typedef void (*prj_traceMemCb)(int action, const char* type, int sz, void* ptr, void* ptr2);
    typedef void (*prj_traceCb)(int action, const char* funcName, int stackDepth,
                                const char* obj1Name, void* obj1,
                                const char* obj2Name, void* obj2);
    typedef void (*prj_traceActCb)(int action, const char* str, void* ptr, void* ptr2);

    static prj_traceCb logFunc_;
    static prj_traceMemCb memFunc_;
    static prj_traceActCb actionFunc_;

    static int stackDepth_;

    const char* funcName_;

    inline prj_trace(const char* funcName) : funcName_(funcName) {
        stackDepth_++;
        if (logFunc_) logFunc_(ENTER_FUNC, funcName_, stackDepth_, "", NULL, "", NULL);
    }
    inline ~prj_trace(void) {
        if (std::uncaught_exceptions()) {
            // If we left a function because of an exception MAYBE show that.
            if (logFunc_) logFunc_(EXCEPTION_EXIT_FUNC, funcName_, stackDepth_, "", NULL, "", NULL);
        } else {
            if (logFunc_) logFunc_(EXIT_FUNC, funcName_, stackDepth_, "", NULL, "", NULL);
        }
        stackDepth_--;
    }
};


//#define USE_PRJ_TRACE 1 // #Debug

#ifdef USE_PRJ_TRACE
#    define PRJ_TRACEMEM_ALLOC(NAME,SZ,PTR) do { if (::prj_trace::memFunc_) { ::prj_trace::memFunc_(::prj_trace::ALLOC_MEM, NAME, SZ, PTR, NULL); } } while(0)
#    define PRJ_TRACEMEM_FREE(NAME,PTR)     do { if (::prj_trace::memFunc_) { ::prj_trace::memFunc_(::prj_trace::FREE_MEM, NAME, 0, PTR, NULL); } } while(0)
#    define PRJ_TRACEMEM_REALLOC(NAME,SZ,OLDPTR,NEWPTR) do { if (::prj_trace::memFunc_) { ::prj_trace::memFunc_(::prj_trace::REALLOC_MEM, NAME, SZ, OLDPTR, NEWPTR); } } while(0)
#  ifdef __GNUC__
#    define PRJ_TRACE prj_trace   prj_traceObj(__PRETTY_FUNCTION__)
#  else // GGC/G++, Mingw(GCC for Windows) and Clang
#    define PRJ_TRACE prj_trace   prj_traceObj(__FUNCSIG__)
#  endif
#  define PRJ_TRACE_SETTYPE(jim_objPtr, TYPENAME) do { if (::prj_trace::actionFunc_) { ::prj_trace::actionFunc_(::prj_trace::ACTION_SETTYPE, TYPENAME, (void*)jim_objPtr, NULL);} } while(0)

// HashTable tracing
#  define PRJ_TRACE_HT(ACTION, NAME, HTPTR) do { if (::prj_trace::actionFunc_) {    \
    ::prj_trace::actionFunc_(ACTION, NAME, HTPTR,                                   \
    NULL);                             \
} } while(0)
#  define PRJ_TRACE_GEN(ACTION, NAME, PTR1, PTR2) do { if (::prj_trace::actionFunc_) { ::prj_trace::actionFunc_(ACTION, NAME, (void*)PTR1, (void*)PTR2);} } while(0)
#else
#  define PRJ_TRACE
#  define PRJ_TRACEMEM_ALLOC(NAME,SZ,PTR) 
#  define PRJ_TRACEMEM_FREE(NAME,PTR) 
#  define PRJ_TRACEMEM_REALLOC(NAME,SZ,OLDPTR,NEWPTR) 
#  define PRJ_TRACE_SETTYPE(jim_objPtr, TYPENAME)
#  define PRJ_TRACE_HT(ACTION, NAME, HTPTR) 
#  define PRJ_TRACE_GEN(ACTION, NAME, PTR1, PTR2)
#endif