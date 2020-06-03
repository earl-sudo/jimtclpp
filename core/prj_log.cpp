#define _CRT_SECURE_NO_WARNINGS 1
/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <vector>
#include <algorithm>

#include <prj_log.h>


namespace PrjLogger {

    using namespace std;

    logFunc g_logger = log; // Default on
    bool g_LOG_USE_COLOR = false;
    bool g_LOG_USE_TIME = true;
    bool g_FULLFUNCNAME = false;
    bool g_SHOWTOPICS = false;
    bool g_DO_FLUSHES = true;

    struct PrjLogData {
        void* udata;
        log_LockFn lock;
        FILE* fp;
        int level;
        bool quiet;
    };

    static PrjLogData g_logData; // #threadIssue

    static const char* level_names[] = {
      "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
    };

    static const char* level_colors[] = {
      "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
    };

    static void lock() {
        if (g_logData.lock) {
            g_logData.lock(g_logData.udata, 1);
        }
    }

    static void unlock() {
        if (g_logData.lock) {
            g_logData.lock(g_logData.udata, 0);
        }
    }

    void log_toggle(logFunc logFunc) { g_logger = logFunc; }
    void log_set_udata(void* udata) { g_logData.udata = udata; }
    void log_set_lock(log_LockFn fn) { g_logData.lock = fn; }
    void log_set_fp(FILE* fp) { g_logData.fp = fp; }
    void log_set_level(int level) { g_logData.level = level; }
    void log_set_quiet(int enable) { g_logData.quiet = enable ? true : false; }
    void log_set_usecolor(bool enable) { g_LOG_USE_COLOR = enable; }
    void log_set_usefullpath(bool enable) { g_FULLFUNCNAME = enable; }
    void log_use_time(bool enable) { g_LOG_USE_TIME = enable; }
    void log_show_topics(bool enable) { g_SHOWTOPICS = enable; }

    vector< logfilerFunc> g_logFilters; // #threadIssue

    void log_add_filter(logfilerFunc func) {
        g_logFilters.push_back(func);
    }
    void log_remove_filter(logfilerFunc func) {
        g_logFilters.erase(std::remove(g_logFilters.begin(), g_logFilters.end(), func), g_logFilters.end());
    }

    void log(LEVELS level, const char* topics, const char* fileD, const char* func, int line, const char* fmt, ...) {

        if (level < g_logData.level) {
            return;
        }
        for (auto& filter : g_logFilters) {
            if (filter(level, topics, fileD, func, line)) return;
        }
        /* Acquire lock */
        lock();

        const char* file = fileD;

        if (!g_FULLFUNCNAME) {
            auto winfile = strrchr(fileD, '\\');
            auto unixfile = strrchr(fileD, '/');
            if (winfile) {
                winfile++;
                file = winfile;
            } else if (unixfile) {
                unixfile++;
                file = unixfile;
            } else {
                file = fileD;
            }
        }
        /* Log to stderr */
        if (!g_logData.quiet) {
            char buf[32] = { 0 };
            if (g_LOG_USE_TIME) {
                /* Get current time */
                time_t t = time(nullptr);
                struct tm* lt = localtime(&t);

                buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
            }
            if (g_LOG_USE_COLOR) {
                fprintf(
                    stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:%s\x1b[0m ",
                    buf, (g_SHOWTOPICS) ? (topics) : (level_colors[level]), level_names[level], file, line, func);
            } else {
                fprintf(stderr, "%s %-5s %s:%d:%s ", buf, (g_SHOWTOPICS) ? (topics) : (level_colors[level]), file, line, func);
            }
            va_list args;
            va_start(args, fmt);
            vfprintf(stderr, fmt, args);
            va_end(args);
            fprintf(stderr, "\n");
            if (g_DO_FLUSHES) fflush(stderr);
        }

        /* Log to file */
        if (g_logData.fp) {
            char buf[32] = { 0 };

            if (g_LOG_USE_TIME) {

                /* Get current time */
                time_t t = time(nullptr);
                struct tm* lt = localtime(&t);

                buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
            }
            va_list args;
            fprintf(g_logData.fp, "%s %-5s %s:%d:%s ", buf, level_names[level], file, line, func);
            va_start(args, fmt);
            vfprintf(g_logData.fp, fmt, args);
            va_end(args);
            fprintf(g_logData.fp, "\n");
            if (g_DO_FLUSHES) fflush(g_logData.fp);
        }

        /* Release lock */
        unlock();
    }

}; // namespace PrjLogger 

#if 0
#include <jim.h>


namespace PrjLogger {
#define PRJTRACEOBJ(LEVEL, TOPICS, FMT, OBJ)  log(level, topics, __FILE__, __FUNCTION__, __LINE__, OBJ)

    template<typename TN>
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, TN* obj) {
        log(level, topics, file, func, line, "%s %p\n", "unknown", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_StackPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashEntryPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableType* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTablePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableIterator* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_VarPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CmdPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ReferencePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjType* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_InterpPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CallFramePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_PrngState* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ExprOperatorPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::jim_subcmd_type* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ListIterPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenListPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimExprNodePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptTokenPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptObj* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParseMissing* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParserCtx* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::lsort_info* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::AssocDataValue* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprTreePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprBuilderPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtPartDescrPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtStringObjPtr obj);

    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjPtr obj) {
        //obj->refCount(); obj->typePtr()->getName(); 
        log(level, topics, file, func, line, "Jim_Obj %p type %s refCount %d\n", obj, obj->typePtr()->getName(), obj->refCount());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_StackPtr obj) {
        // obj->len(); 
        log(level, topics, file, func, line, "Jim_Stack %p len %d\n", obj, obj->len());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashEntryPtr obj) {
        //obj->getVal(); obj->keyAsStr();
        log(level, topics, file, func, line, "Jim_HashEntry %p key %s\n", obj, obj->keyAsStr());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableType* obj) {
        log(level, topics, file, func, line, "Jim_HashTableType %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTablePtr obj) {
        // obj->size(); obj->type(); obj->getTypeName(); obj->used(); 
        log(level, topics, file, func, line, "Jim_HashTable %p type %s size %d used %d\n", obj, obj->getTypeName(), obj->size(), obj->used());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableIterator* obj) {
        //obj->index(); obj->ht()->size(); obj->ht()->used(); obj->ht()->getTypeName(); obj->ht()->getTypeName();
        log(level, topics, file, func, line, "Jim_HashTableIterator %p type %s index %d\n", obj, obj->ht()->getTypeName(), obj->index());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_VarPtr obj) {
        //obj->objPtr(); obj->linkFramePtr();
        log(level, topics, file, func, line, "Jim_Var %p objPtr %p linkFramePtr %p\n", obj, obj->objPtr(), obj->linkFramePtr());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CmdPtr obj) {
        //obj->inUse(); obj->isproc(); obj->cmdProc(); obj->proc_nsObj(); obj->proc_arglist(); obj->proc_argListLen(); obj->proc_optArity();
        //obj->proc_regArity();
        log(level, topics, file, func, line, "Jim_Cmd %p isproc %d cmdProc %p\n", obj, obj->isproc(), obj->cmdProc());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ReferencePtr obj) {
        //obj->tag(); obj->objPtr(); 
        log(level, topics, file, func, line, "Jim_Reference %p tag %s objPtr %p\n", obj, obj->tag(), obj->objPtr());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjType* obj) {
        //obj->getName(); obj->getFlags(); 
        log(level, topics, file, func, line, "Jim_ObjType %p name %s flags %d\n", obj, obj->getName(), obj->getFlags());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_InterpPtr obj) {
        //obj->assocDataPtr(); obj->getPackagesPtr(); obj->freeFramesList(); obj->cmdPrivData(); obj->errorFlag(); obj->unknown();
        //obj->unknown_called(); obj->errorProc(); obj->stackTrace(); obj->lastCollectId(); obj->references(); obj->liveList(); 
        //obj->freeList(); obj->commands(); obj->returnLevel(); obj->errorLine(); obj->errorFileNameObj(); obj->maxCallFrameDepth();
        //obj->maxEvalDepth(); obj->evalDepth(); obj->returnCode(); obj->exitCode(); obj->id(); obj->signal_level(); 
        //obj->getSigmask(); obj->framePtr(); obj->topFramePtr(); obj->procEpoch(); obj->callFrameEpoch(); obj->local();
        //obj->currentScriptObj(); obj->nullScriptObj(); obj->referenceNextId(); obj->lastCollectTime();
        log(level, topics, file, func, line, "Jim_Interp %p id %d\n", obj, obj->id());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CallFramePtr obj) {
        //obj->level(); obj->id(); obj->parent(); obj->argc(); obj->argv(); obj->nsObj(); obj->procArgsObjPtr(); obj->vars();
        log(level, topics, file, func, line, "Jim_CallFrame %p id %d level %d argc %d\n", obj, obj->id(), obj->level(), obj->argc());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_PrngState* obj) {
        log(level, topics, file, func, line, "Jim_PrngState %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ExprOperatorPtr obj) {
        //obj->name(); obj->attr(); obj->arity(); obj->precedence();
        log(level, topics, file, func, line, "Jim_ExprOperator %p name %s\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::jim_subcmd_type* obj) {
        //obj->cmd_; obj->args_; obj->flags_; obj->function_; obj->maxargs_; obj->minargs_;
        log(level, topics, file, func, line, "jim_subcmd_type %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ListIterPtr obj) {
        //obj->idx_; obj->objPtr_;
        log(level, topics, file, func, line, "Jim_ListIter %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenListPtr obj) {
        log(level, topics, file, func, line, "ParseTokenList %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenPtr obj) {
        //obj->tokenType(); obj->tokenLen_
        log(level, topics, file, func, line, "ParseToken %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimExprNodePtr obj) {
        //obj->left_; obj->right_; obj->objPtr_; obj->tokenType(); 
        log(level, topics, file, func, line, "JimExprNode %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptTokenPtr obj) {
        //obj->objPtr_; obj->tokenType_
        log(level, topics, file, func, line, "ScriptTokenPtr %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptObj* obj) {
        //obj->firstLineNum(); obj->errorLineNum(); obj->Num_tokenArray(); obj->inUse_(); obj->substFlags_; obj->fileNameObj_; obj->tokenArray_
        log(level, topics, file, func, line, "ScriptObj %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParseMissing* obj) {
        //obj->lineNum();  obj->ch_;
        log(level, topics, file, func, line, "JimParseMissing %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParserCtx* obj) {
        // obj->comment_; obj->currLineNum_; obj->eof_; obj->inquote_; obj->len_; obj->missing_; obj->p_; 
        // obj->retTokenLineNum_; obj->tend_; obj->tokenType_; obj->tstart_
        log(level, topics, file, func, line, "JimParserCtx %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::lsort_info* obj) {
        // obj->command_; obj->index_; obj->indexed_; obj->interp_; obj->lsortOrder_; obj->lsortType_; obj->sortingFuncPtr_(); obj->unique_;
        log(level, topics, file, func, line, "lsort_info %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::AssocDataValue* obj) {
        // obj->data_; obj->delProc_
        log(level, topics, file, func, line, "AssocDataValue %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprTreePtr obj) {
        //obj->inUse(); obj->len(); obj->expr_; obj->nodes_;
        log(level, topics, file, func, line, "ExprTree %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprBuilderPtr obj) {
        // obj->parencount(); obj->level(); obj->token_; obj->first_token_; obj->nodes_; obj->next_; obj->stack_; obj->token_;
        log(level, topics, file, func, line, "ExprBuilder %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtPartDescrPtr obj) {
        // obj->pos; obj->arg_; obj->maxWidth_; obj->prefix_; obj->typeModifier_; obj->typeOfConv_;
        log(level, topics, file, func, line, "ScanFmtPartDescr %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtStringObjPtr obj) {
        //obj->size(); obj->errorText(); obj->maxPos(); obj->scratchPad(); obj->size(); obj->orgStringRep_; obj->num_descr_; obj->descr_;
        log(level, topics, file, func, line, "ScanFmtStringObj %p size %d\n", obj);
    }
}
#endif

#include <jim.h>


namespace PrjLogger {
#define PRJTRACEOBJ(LEVEL, TOPICS, FMT, OBJ)  log(level, topics, __FILE__, __FUNCTION__, __LINE__, OBJ)

    template<typename TN>
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, TN* obj) {
       log(level, topics, file, func, line, "%s %p\n", "unknown", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_StackPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashEntryPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableType * obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTablePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableIterator * obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_VarPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CmdPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ReferencePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjType* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_InterpPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CallFramePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_PrngState* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ExprOperatorPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::jim_subcmd_type* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ListIterPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenListPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimExprNodePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptTokenPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptObj* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParseMissing* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParserCtx* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::lsort_info* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::AssocDataValue* obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprTreePtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprBuilderPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtPartDescrPtr obj);
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtStringObjPtr obj);

    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjPtr obj) {
        //obj->refCount(); obj->typePtr()->getName(); 
        log(level, topics, file, func, line, "Jim_Obj %p type %s refCount %d\n", obj, obj->typePtr()->getName(), obj->refCount());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_StackPtr obj) {
        // obj->len(); 
        log(level, topics, file, func, line, "Jim_Stack %p len %d\n", obj, obj->len());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashEntryPtr obj) {
        //obj->getVal(); obj->keyAsStr();
        log(level, topics, file, func, line, "Jim_HashEntry %p key %s\n", obj, obj->keyAsStr());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableType* obj) {
        log(level, topics, file, func, line, "Jim_HashTableType %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTablePtr obj) {
        // obj->size(); obj->type(); obj->getTypeName(); obj->used(); 
        log(level, topics, file, func, line, "Jim_HashTable %p type %s size %d used %d\n", obj, obj->getTypeName(), obj->size(), obj->used());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_HashTableIterator* obj) {
        //obj->index(); obj->ht()->size(); obj->ht()->used(); obj->ht()->getTypeName(); obj->ht()->getTypeName();
        log(level, topics, file, func, line, "Jim_HashTableIterator %p type %s index %d\n", obj, obj->ht()->getTypeName(), obj->index());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_VarPtr obj) {
        //obj->objPtr(); obj->linkFramePtr();
        log(level, topics, file, func, line, "Jim_Var %p objPtr %p linkFramePtr %p\n", obj, obj->objPtr(), obj->linkFramePtr());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CmdPtr obj) {
        //obj->inUse(); obj->isproc(); obj->cmdProc(); obj->proc_nsObj(); obj->proc_arglist(); obj->proc_argListLen(); obj->proc_optArity();
        //obj->proc_regArity();
        log(level, topics, file, func, line, "Jim_Cmd %p isproc %d cmdProc %p\n", obj, obj->isproc(), obj->cmdProc());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ReferencePtr obj) {
        //obj->tag(); obj->objPtr(); 
        log(level, topics, file, func, line, "Jim_Reference %p tag %s objPtr %p\n", obj, obj->tag(), obj->objPtr());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ObjType* obj) {
        //obj->getName(); obj->getFlags(); 
        log(level, topics, file, func, line, "Jim_ObjType %p name %s flags %d\n", obj, obj->getName(), obj->getFlags());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_InterpPtr obj) {
        //obj->assocDataPtr(); obj->getPackagesPtr(); obj->freeFramesList(); obj->cmdPrivData(); obj->errorFlag(); obj->unknown();
        //obj->unknown_called(); obj->errorProc(); obj->stackTrace(); obj->lastCollectId(); obj->references(); obj->liveList(); 
        //obj->freeList(); obj->commands(); obj->returnLevel(); obj->errorLine(); obj->errorFileNameObj(); obj->maxCallFrameDepth();
        //obj->maxEvalDepth(); obj->evalDepth(); obj->returnCode(); obj->exitCode(); obj->id(); obj->signal_level(); 
        //obj->getSigmask(); obj->framePtr(); obj->topFramePtr(); obj->procEpoch(); obj->callFrameEpoch(); obj->local();
        //obj->currentScriptObj(); obj->nullScriptObj(); obj->referenceNextId(); obj->lastCollectTime();
        log(level, topics, file, func, line, "Jim_Interp %p id %d\n", obj, obj->id());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_CallFramePtr obj) {
        //obj->level(); obj->id(); obj->parent(); obj->argc(); obj->argv(); obj->nsObj(); obj->procArgsObjPtr(); obj->vars();
        log(level, topics, file, func, line, "Jim_CallFrame %p id %d level %d argc %d\n", obj, obj->id(), obj->level(), obj->argc());
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_PrngState* obj) {
        log(level, topics, file, func, line, "Jim_PrngState %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ExprOperatorPtr obj) {
        //obj->name(); obj->attr(); obj->arity(); obj->precedence();
        log(level, topics, file, func, line, "Jim_ExprOperator %p name %s\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::jim_subcmd_type* obj) {
        //obj->cmd_; obj->args_; obj->flags_; obj->function_; obj->maxargs_; obj->minargs_;
        log(level, topics, file, func, line, "jim_subcmd_type %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::Jim_ListIterPtr obj) {
        //obj->idx_; obj->objPtr_;
        log(level, topics, file, func, line, "Jim_ListIter %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenListPtr obj) {
        log(level, topics, file, func, line, "ParseTokenList %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ParseTokenPtr obj) {
        //obj->tokenType(); obj->tokenLen_
        log(level, topics, file, func, line, "ParseToken %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimExprNodePtr obj) {
        //obj->left_; obj->right_; obj->objPtr_; obj->tokenType(); 
        log(level, topics, file, func, line, "JimExprNode %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptTokenPtr obj) {
        //obj->objPtr_; obj->tokenType_
        log(level, topics, file, func, line, "ScriptTokenPtr %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScriptObj* obj) {
        //obj->firstLineNum(); obj->errorLineNum(); obj->Num_tokenArray(); obj->inUse_(); obj->substFlags_; obj->fileNameObj_; obj->tokenArray_
        log(level, topics, file, func, line, "ScriptObj %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParseMissing* obj) {
        //obj->lineNum();  obj->ch_;
        log(level, topics, file, func, line, "JimParseMissing %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::JimParserCtx* obj) {
        // obj->comment_; obj->currLineNum_; obj->eof_; obj->inquote_; obj->len_; obj->missing_; obj->p_; 
        // obj->retTokenLineNum_; obj->tend_; obj->tokenType_; obj->tstart_
        log(level, topics, file, func, line, "JimParserCtx %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::lsort_info* obj) {
        // obj->command_; obj->index_; obj->indexed_; obj->interp_; obj->lsortOrder_; obj->lsortType_; obj->sortingFuncPtr_(); obj->unique_;
        log(level, topics, file, func, line, "lsort_info %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::AssocDataValue* obj) {
        // obj->data_; obj->delProc_
        log(level, topics, file, func, line, "AssocDataValue %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprTreePtr obj) {
        //obj->inUse(); obj->len(); obj->expr_; obj->nodes_;
        log(level, topics, file, func, line, "ExprTree %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ExprBuilderPtr obj) {
        // obj->parencount(); obj->level(); obj->token_; obj->first_token_; obj->nodes_; obj->next_; obj->stack_; obj->token_;
        log(level, topics, file, func, line, "ExprBuilder %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtPartDescrPtr obj) {
        // obj->pos; obj->arg_; obj->maxWidth_; obj->prefix_; obj->typeModifier_; obj->typeOfConv_;
        log(level, topics, file, func, line, "ScanFmtPartDescr %p\n", obj);
    }
    void log(LEVELS level, const char* topics, const char* file, const char* func, int line, JIM_NAMESPACE_NAME::ScanFmtStringObjPtr obj) {
        //obj->size(); obj->errorText(); obj->maxPos(); obj->scratchPad(); obj->size(); obj->orgStringRep_; obj->num_descr_; obj->descr_;
        log(level, topics, file, func, line, "ScanFmtStringObj %p size %d\n", obj);
    }
}