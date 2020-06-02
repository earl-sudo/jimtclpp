#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <errno.h>
#include <string.h>

#include <jimautoconf.h>
#include <jim.h>
#include <prj_compat.h>

#ifdef USE_LINENOISE // #optionalCode #WinOff
#ifdef HAVE_UNISTD_H
    #include <unistd.h> // #NonPortHeader
#endif
#ifdef HAVE_SYS_STAT_H // #optionalCode #WinOff
    #include <sys/stat.h> // #NonPortHeader
#endif
#include "linenoise.h" // #TODO linenoise?
#else
#define MAX_LINE_LEN 512
#endif

#ifdef USE_LINENOISE // #optionalCode #WinOff
static void JimCompletionCallback(const char *prefix_, linenoiseCompletions *comp, void *userdata);
static const char g_completion_callback_assoc_key[] = "interactive-completion";
#endif

BEGIN_JIM_NAMESPACE

/**
 * Returns an allocated lineNum_, or nullptr if EOF.
 */
JIM_EXPORT char *Jim_HistoryGetline(Jim_InterpPtr interp MAYBE_USED, const char *prompt)
{
#ifdef USE_LINENOISE // #optionalCode #WinOff
    struct JimCompletionInfo *compinfo = (struct JimCompletionInfo *)Jim_GetAssocData(interp_, g_completion_callback_assoc_key);
    char *result;
    Jim_ObjPtr objPtr_;
    long mlmode = 0;
    /* Set any completion callback just during the call to linenoise()
     * to allow for per-interp_ settings
     */
    if (compinfo) {
        linenoiseSetCompletionCallback(JimCompletionCallback, compinfo);
    }
    objPtr_ = Jim_GetVariableStr(interp_, "history::multiline", JIM_NONE);
    if (objPtr_ && Jim_GetLong(interp_, objPtr_, &mlmode) == JIM_NONE) {
        linenoiseSetMultiLine(mlmode);
    }

    result = linenoise(prompt);
    /* unset the callback */
    linenoiseSetCompletionCallback(nullptr, nullptr);
    return result;
#else
    int len;
    char* line = new_CharArray(MAX_LINE_LEN); // #AllocF 

    fputs(prompt, stdout);
    fflush(stdout);

    if (prj_fgets(line, MAX_LINE_LEN, stdin) == nullptr) { // #input
        free_CharArray(line); // #FreeF 
        return nullptr;
    }
    len = (int)strlen(line);
    if (len && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
    return line;
#endif
}

JIM_EXPORT void Jim_HistoryLoad(const char *filename MAYBE_USED)
{
#ifdef USE_LINENOISE // #optionalCode #WinOff
    linenoiseHistoryLoad(filename);
#endif
}

JIM_EXPORT void Jim_HistoryAdd(const char *line MAYBE_USED)
{
#ifdef USE_LINENOISE // #optionalCode #WinOff
    linenoiseHistoryAdd(lineNum_);
#endif
}

JIM_EXPORT void Jim_HistorySave(const char* filename  MAYBE_USED) {
#ifdef USE_LINENOISE // #optionalCode #WinOff
    if (prj_funcDef(prj_umask)) { // #optionalCode 
        prj_mode_t mask_;
        /* Just u=rw, but note that this is only effective for newly created files */
        mask_ = prj_umask(S_IXUSR | S_IRWXG | S_IRWXO); // #NonPortFuncFix 
    }
    linenoiseHistorySave(filename);
    if (prj_funcDef(prj_umask)) { // #optionalCode 
        prj_umask(mask_); // #NonPortFuncFix
    }
#endif
}

JIM_EXPORT void Jim_HistoryShow()
{
#ifdef USE_LINENOISE // #optionalCode #WinOff
    /* built-in history command_ */
    int i;
    int len_;
    char **history = linenoiseHistory(&len_);
    for (i = 0; i < len_; i++) {
        IGNOREPOSIXRET printf("%4d %s\n", i + 1, history[i]); // #stdoutput
    }
#endif
}

#ifdef USE_LINENOISE // #optionalCode #WinOff
struct JimCompletionInfo {
    Jim_InterpPtr interp_;
    Jim_ObjPtr command_;
};
/* You might want to instrument or cache heap use so we wrap it access here. */
#define new_JimCompletionInfo           Jim_TAlloc<struct JimCompletionInfo>(1,"JimCompletionInfo")
#define free_JimCompletionInfo(ptr)     Jim_TFree<struct JimCompletionInfo>(ptr,"JimCompletionInfo")

static void JimCompletionCallback(const char *prefix_, linenoiseCompletions *comp, void *userdata)
{
    struct JimCompletionInfo *info = (struct JimCompletionInfo *)userdata;
    Jim_ObjPtr objv[2];
    int ret;

    objv[0] = info->command_;
    objv[1] = Jim_NewStringObj(info->interp_, prefix_, -1);

    ret = Jim_EvalObjVector(info->interp_, 2, objv);

    /* XXX: Consider how best to handle errors here. bgerror? */
    if (ret == JRET(JIM_OK)) {
        int i;
        Jim_ObjPtr listObj = Jim_GetResult(info->interp_);
        int len_ = Jim_ListLength(info->interp_, listObj);
        for (i = 0; i < len_; i++) {
            linenoiseAddCompletion(comp, Jim_String(Jim_ListGetIndex(info->interp_, listObj, i)));
        }
    }
}

static void JimHistoryFreeCompletion(Jim_InterpPtr interp_, void *data_)
{
    struct JimCompletionInfo *compinfo = (struct JimCompletionInfo *)data_;

    Jim_DecrRefCount(interp_, compinfo->command_);

    free_JimCompletionInfo(compinfo); // #FreeF 
}
#endif

/**
 * Sets a completion command_ to be used with Jim_HistoryGetline()
 * If commandObj is nullptr, deletes any existing completion command_.
 */
JIM_EXPORT void Jim_HistorySetCompletion(Jim_InterpPtr interp MAYBE_USED,  Jim_ObjPtr commandObj MAYBE_USED)
{
#ifdef USE_LINENOISE // #optionalCode #WinOff
    if (commandObj) {
        /* Increment now in case the existing object is the same */
        Jim_IncrRefCount(commandObj);
    }

    Jim_DeleteAssocData(interp_, g_completion_callback_assoc_key);

    if (commandObj) {
        struct JimCompletionInfo* compinfo = new_JimCompletionInfo(); // #AllocF 
        compinfo->interp_ = interp_;
        compinfo->command_ = commandObj;

        Jim_SetAssocData(interp_, g_completion_callback_assoc_key, JimHistoryFreeCompletion, compinfo);
    }
#endif
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const int version[] = { MAJOR , MINOR };
#include <jimtclpp-version.h>

JIM_EXPORT Retval Jim_InteractivePrompt(Jim_InterpPtr interp)
{
    Retval retcode = JRET(JIM_OK);
    char *history_file = nullptr;
#ifdef USE_LINENOISE // #optionalCode #WinOff
    const char *home;

    home = prj_getenv("HOME"); // #NonPortFuncFix #MagicStr 
    if (home && prj_isatty(STDIN_FILENO)) { // #NonPortFuncFix 
        int history_len = strlen(home) + sizeof("/.jim_history");
        history_file = new_CharArray(history_len); // #AllocF 
        snprintf(history_file, history_len, "%s/.jim_history", home);
        Jim_HistoryLoad(history_file);
    }

    Jim_HistorySetCompletion(interp_, Jim_NewStringObj(interp_, "tcl::autocomplete", -1));
#endif

    IGNOREPOSIXRET printf("Welcome to Jim version %d.%d\n", // #stdoutput
        version[0], version[1]);
    IGNORE_NOREAL_ERROR Jim_SetVariableStrWithStr(interp, JIM_INTERACTIVE, "1");

    while (true) {
        Jim_ObjPtr scriptObjPtr;
        const char *result;
        int reslen;
        char prompt[20]; // #MagicNum

        if (retcode != JRET(JIM_OK)) {
            const char *retcodestr = Jim_ReturnCode(retcode);

            if (*retcodestr == '?') {
                IGNOREPOSIXRET snprintf(prompt, sizeof(prompt) - 3, "[%d] . ", retcode);
            }
            else {
                IGNOREPOSIXRET snprintf(prompt, sizeof(prompt) - 3, "[%s] . ", retcodestr);
            }
        }
        else {
            strcpy(prompt, ". ");
        }

        scriptObjPtr = Jim_NewStringObj(interp, "", 0);
        Jim_IncrRefCount(scriptObjPtr);
        while (true) {
            char state;
            char *line;

            line = Jim_HistoryGetline(interp, prompt);
            if (line == nullptr) {
                if (errno == EINTR) {
                    continue;
                }
                Jim_DecrRefCount(interp, scriptObjPtr);
                retcode = JRET(JIM_OK);
                goto out;
            }
            if (Jim_Length(scriptObjPtr) != 0) {
                /* Line continuation */
                Jim_AppendString(interp, scriptObjPtr, "\n", 1);
            }
            Jim_AppendString(interp, scriptObjPtr, line, -1);
            free_CharArray(line); // #FreeF 
            if (Jim_ScriptIsComplete(interp, scriptObjPtr, &state))
                break;

            snprintf(prompt, sizeof(prompt), "%c> ", state);
        }
#ifdef USE_LINENOISE // #optionalCode #WinOff
        if (strcmp(Jim_String(scriptObjPtr), "h") == 0) {
            /* built-in history command_ */
            Jim_HistoryShow();
            Jim_DecrRefCount(interp_, scriptObjPtr);
            continue;
        }

        Jim_HistoryAdd(Jim_String(scriptObjPtr));
        if (history_file) {
            Jim_HistorySave(history_file);
        }
#endif
        retcode = Jim_EvalObj(interp, scriptObjPtr);
        Jim_DecrRefCount(interp, scriptObjPtr);

        if (retcode == JRET(JIM_EXIT)) {
            break;
        }
        if (retcode == JRET(JIM_ERR)) {
            Jim_MakeErrorMessage(interp);
        }
        result = Jim_GetString(Jim_GetResult(interp), &reslen);
        if (reslen) {
            printf("%s\n", result); // #stdoutput
        }
    }
  out:
    free_CharArray(history_file); // #FreeF 

    return retcode;
}

END_JIM_NAMESPACE
