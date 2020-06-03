#include <stdio.h>
#include <stdint.h>

#include <jim-base.h>
#include <prj_trace.h>

#include <jim.h>

#include <map>

using namespace std;

static int64_t g_numCalls = 0;  // #threadIssue
static int g_maxStackDepth = 0; // #threadIssue
int prj_trace::stackDepth_ = 0; // #threadIssue

#ifdef __GNUC__
#  pragma GCC diagnostic ignored  "-Wunused-function"
#endif

typedef map<prj_trace::ACTIONS, string> ActionStringsMap;

#define ENUMVAL(X) { prj_trace:: X, #X }
ActionStringsMap    g_ActionStringsMap =  { // #threadIssue
    // Function trace actions
    ENUMVAL(ENTER_FUNC), ENUMVAL(EXIT_FUNC), ENUMVAL(EXCEPTION_EXIT_FUNC),
    // Heap change actions
    ENUMVAL(ALLOC_MEM), ENUMVAL(FREE_MEM), ENUMVAL(REALLOC_MEM),
    // Jim_Obj actions
    ENUMVAL(ACTION_SETTYPE), // Setting a type of a Jim_Obj
    // Hash table actions
    ENUMVAL(ACTION_HT_CREATE), ENUMVAL(ACTION_HT_DELETE), ENUMVAL(ACTION_HT_RESIZE_PRE), ENUMVAL(ACTION_HT_RESIZE_POST), ENUMVAL(ACTION_HT_STATS),
    // Command actions
    ENUMVAL(ACTION_CMD_CREATE), ENUMVAL(ACTION_CMD_INVOKE), ENUMVAL(ACTION_CMD_DELETE),
    // tcl proc actions
    ENUMVAL(ACTION_PROC_CREATE), ENUMVAL(ACTION_PROC_INVOKE), ENUMVAL(ACTION_PROC_DELETE),
    // tcl callframe actions
    ENUMVAL(ACTION_CALLFRAME_CREATE), ENUMVAL(ACTION_CALLFRAME_DELETE),
    // tcl interpreter actions
    ENUMVAL(ACTION_INTERP_CREATE), ENUMVAL(ACTION_INTERP_DELETE), ENUMVAL(ACTION_INTERP_STATS),
    // tcl expression actions
    ENUMVAL(ACTION_EXPR_PRE), ENUMVAL(ACTION_EXPR_POST),
    // jim reference actions
    ENUMVAL(ACTION_REFERNCE_CREATE),
    // jim garbage collection actions
    ENUMVAL(ACTION_COLLECT_PRE), ENUMVAL(ACTION_COLLECT_POST),
};

const string& getVal(prj_trace::ACTIONS act) {
    auto ret = g_ActionStringsMap.find(act);
    if (ret == g_ActionStringsMap.end()) return "unkown-action";
    return g_ActionStringsMap.find(act)->second;
}

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
    ::printf("TRACE: %s %d %s %p %p\n", getVal((prj_trace::ACTIONS)action).c_str(), action, str, ptr, ptr2);
    bool traceHT = true;
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
            if (traceHT) {
                auto ht = CAST(::Jim::Jim_HashTablePtr)ptr;
            }
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

prj_trace::prj_traceCb prj_trace::logFunc_ = (prj_trace::prj_traceCb)prj_traceShowAll;
prj_trace::prj_traceMemCb prj_trace::memFunc_ = (prj_trace::prj_traceMemCb)nullptr;
prj_trace::prj_traceActCb prj_trace::actionFunc_ = (prj_trace::prj_traceActCb)prj_traceActCbShowAll;
