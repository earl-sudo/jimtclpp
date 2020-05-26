#include <stdio.h>
#include <stdint.h>

#include <jim-base.h>
#include <prj_trace.h>

static int64_t g_numCalls = 0;
static int g_maxStackDepth = 0;

#ifdef __GNUC__
#  pragma GCC diagnostic ignored  "-Wunused-function"
#endif

static void prj_traceShowAll(int action, const char* funcName, int stackDepth,
                             const char* obj1Name MAYBE_USED, void* obj1 MAYBE_USED, const char* obj2Name MAYBE_USED, void* obj2 MAYBE_USED) {
    g_numCalls++;
    if (stackDepth > g_maxStackDepth) g_maxStackDepth = stackDepth;

    switch (action) {
        case prj_trace::ENTER_FUNC:
            ::printf("ENTER: %03d %s\n", stackDepth, funcName);
            break;
        case prj_trace::EXIT_FUNC:
            ::printf("EXIT:  %03d %s\n", stackDepth, funcName);
            break;
    };
}

static void prj_traceMemCbShowAll(int action, const char* type, int sz MAYBE_USED, void* ptr, void* ptr2) {
    switch (action) {
        case prj_trace::ALLOC_MEM:
        case prj_trace::FREE_MEM:
        case prj_trace::REALLOC_MEM:
            ::printf("MEM: %d, %s, %p, %p\n", action, type, ptr, ptr2);
            break;
    };
}

static void prj_traceActCbShowAll(int action, const char* str, void* ptr, void* ptr2) {
    //::printf("TRACE: %d %s %p %p\n", action, getStr, ptr, ptr2);
    bool traceHT = false;
    bool traceCmd = false;
    bool tracePrj = false;
    bool traceCallFrame = false;
    bool traceExpr = false;
    bool traceRef = false;
    bool traceCollect = true;

    switch (action) {
        case prj_trace::ACTION_HT_CREATE:
        case prj_trace::ACTION_HT_DELETE:
        case prj_trace::ACTION_HT_RESIZE_PRE:
        case prj_trace::ACTION_HT_RESIZE_POST:
        case prj_trace::ACTION_HT_STATS:
        {
            prj_trace::HT_Stats* ht_stats = (prj_trace::HT_Stats*) ptr2;
            if (traceHT)
                ::printf("HT: %d %s %p sz %d used %d collisions %d uniq %d\n", action, str, ptr,
                         ht_stats->size_, ht_stats->used_, ht_stats->collisions_, ht_stats->uniq_);
        }
        break;
        case prj_trace::ACTION_CMD_CREATE:
        case prj_trace::ACTION_CMD_INVOKE:
        case prj_trace::ACTION_CMD_DELETE:
            if (traceCmd)
                ::printf("CMD: %d %s %p %p\n", action, str, ptr, ptr2);
            break;
        case prj_trace::ACTION_PROC_CREATE:
        case prj_trace::ACTION_PROC_INVOKE:
        case prj_trace::ACTION_PROC_DELETE:
            if (tracePrj)
                ::printf("PROC: %d %s %p %p\n", action, str, ptr, ptr2);
            break;
        case prj_trace::ACTION_CALLFRAME_CREATE:
            if (traceCallFrame)
                ::printf("CALLFRAME: %d %s %p %p\n", action, str, ptr, ptr2);
            break;
        case prj_trace::ACTION_EXPR_PRE:
        case prj_trace::ACTION_EXPR_POST:
            if (traceExpr)
                ::printf("EXPR: %d %s %p %p\n", action, str, ptr, ptr2);
            break;
        case prj_trace::ACTION_REFERNCE_CREATE:
            if (traceRef) 
                ::printf("REF: %d %s %p %p\n", action, str, ptr, ptr2);
            break;
        case prj_trace::ACTION_COLLECT_PRE:
        case prj_trace::ACTION_COLLECT_POST:
            if (traceCollect) 
                ::printf("COLLECT: %d %s %p %p\n", action, str, ptr, ptr2);
            break;
    };
}

int prj_trace::stackDepth_ = 0;
prj_trace::prj_traceCb prj_trace::logFunc_ = (prj_trace::prj_traceCb)NULL;
prj_trace::prj_traceMemCb prj_trace::memFunc_ = (prj_trace::prj_traceMemCb)NULL;
prj_trace::prj_traceActCb prj_trace::actionFunc_ = (prj_trace::prj_traceActCb)prj_traceActCbShowAll;
