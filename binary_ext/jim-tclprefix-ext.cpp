/*
 * Implements the tcl::prefix command_ for Jim Tcl
 *
 * (c) 2011 Steve Bennett <steveb@workware.net.au>
 *
 * See LICENSE for license details.
 */

#include <jim-api.h>

#include "utf8.h"

#ifdef jim_ext_tclprefix

BEGIN_JIM_NAMESPACE

/**
 * Returns the common initial length of the two strings.
 */
static int JimStringCommonLength(const char *str1, int charlen1, const char *str2, int charlen2)
{
    int maxlen = 0;
    while (charlen1-- && charlen2--) {
        int c1;
        int c2;
        str1 += utf8_tounicode(str1, &c1);
        str2 += utf8_tounicode(str2, &c2);
        if (c1 != c2) {
            break;
        }
        maxlen++;
    }
    return maxlen;
}

/* [tcl::prefix]
 */
static Retval Jim_TclPrefixCoreCommand(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_ObjPtr objPtr;
    Jim_ObjPtr stringObj;
    int option;
    static const char * const options[] = { "match", "all", "longest", NULL };
    enum { OPT_MATCH, OPT_ALL, OPT_LONGEST };

    if (argc < 2) {
        Jim_WrongNumArgs(interp, 1, argv, "subcommand ?arg ...?");
        return JIM_ERR;
    }
    if (Jim_GetEnum(interp, argv[1], options, &option, NULL, JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK)
        return Jim_CheckShowCommands(interp, argv[1], options);

    switch (option) {
        case OPT_MATCH:{
            int i;
            int ret;
            int tablesize;
            const char **table;
            Jim_ObjPtr tableObj;
            Jim_ObjPtr errorObj = NULL;
            const char *message = "option";
            static const char * const matchoptions[] = { "-error", "-exact", "-message", NULL };
            enum { OPT_MATCH_ERROR, OPT_MATCH_EXACT, OPT_MATCH_MESSAGE };
            int flags = JIM_ERRMSG | JIM_ENUM_ABBREV;

            if (argc < 4) {
                Jim_WrongNumArgs(interp, 2, argv, "?options? table string");
                return JIM_ERR;
            }
            tableObj = argv[argc - 2];
            stringObj = argv[argc - 1];
            argc -= 2;
            for (i = 2; i < argc; i++) {
                int matchoption;
                if (Jim_GetEnum(interp, argv[i], matchoptions, &matchoption, "option", JIM_ERRMSG | JIM_ENUM_ABBREV) != JIM_OK)
                    return JIM_ERR;
                switch (matchoption) {
                    case OPT_MATCH_EXACT:
                        flags &= ~JIM_ENUM_ABBREV;
                        break;

                    case OPT_MATCH_ERROR:
                        if (++i == argc) {
                            Jim_SetResultString(interp, "missing error options", -1);
                            return JIM_ERR;
                        }
                        errorObj = argv[i];
                        if (Jim_Length(errorObj) % 2) {
                            Jim_SetResultString(interp, "error options must have an even number of elements", -1);
                            return JIM_ERR;
                        }
                        break;

                    case OPT_MATCH_MESSAGE:
                        if (++i == argc) {
                            Jim_SetResultString(interp, "missing message", -1);
                            return JIM_ERR;
                        }
                        message = Jim_String(argv[i]);
                        break;
                }
            }
            /* Do the match */
            tablesize = Jim_ListLength(interp, tableObj);
            table = (const char**)Jim_Alloc((tablesize + 1) * sizeof(*table));
            for (i = 0; i < tablesize; i++) {
                Jim_ListIndex(interp, tableObj, i, &objPtr, JIM_NONE);
                table[i] = Jim_String(objPtr);
            }
            table[i] = NULL;

            ret = Jim_GetEnum(interp, stringObj, table, &i, message, flags);
            Jim_TFree<constCharArray>(table,"constCharArray"); // #FreeF 
            if (ret == JIM_OK) {
                Jim_ListIndex(interp, tableObj, i, &objPtr, JIM_NONE);
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }
            if (tablesize == 0) {
                Jim_SetResultFormatted(interp, "bad %s \"%#s\": no valid options", message, stringObj);
                return JIM_ERR;
            }
            if (errorObj) {
                if (Jim_Length(errorObj) == 0) {
                    Jim_SetEmptyResult(interp);
                    return JIM_OK;
                }
                /* Do this the easy way. Build a list to evaluate */
                objPtr = Jim_NewStringObj(interp, "return -level 0 -code error", -1);
                Jim_ListAppendList(interp, objPtr, errorObj);
                Jim_ListAppendElement(interp, objPtr, Jim_GetResult(interp));
                return Jim_EvalObjList(interp, objPtr);
            }
            return JIM_ERR;
        }

        case OPT_ALL:
            if (argc != 4) {
                Jim_WrongNumArgs(interp, 2, argv, "table string");
                return JIM_ERR;
            }
            else {
                int i;
                int listlen = Jim_ListLength(interp, argv[2]);
                objPtr = Jim_NewListObj(interp, NULL, 0);
                for (i = 0; i < listlen; i++) {
                    Jim_ObjPtr valObj = Jim_ListGetIndex(interp, argv[2], i);
                    if (Jim_StringCompareLenObj(interp, argv[3], valObj, 0) == 0) {
                        Jim_ListAppendElement(interp, objPtr, valObj);
                    }
                }
                Jim_SetResult(interp, objPtr);
                return JIM_OK;
            }

        case OPT_LONGEST:
            if (argc != 4) {
                Jim_WrongNumArgs(interp, 2, argv, "table string");
                return JIM_ERR;
            }
            else if (Jim_ListLength(interp, argv[2])) {
                const char *longeststr = NULL;
                int longestlen = 0;
                int i;
                int listlen = Jim_ListLength(interp, argv[2]);

                stringObj = argv[3];

                for (i = 0; i < listlen; i++) {
                    Jim_ObjPtr valObj = Jim_ListGetIndex(interp, argv[2], i);

                    if (Jim_StringCompareLenObj(interp, stringObj, valObj, 0)) {
                        /* Does not begin with 'string' */
                        continue;
                    }

                    if (longeststr == NULL) {
                        longestlen = Jim_Utf8Length(interp, valObj);
                        longeststr = Jim_String(valObj);
                    }
                    else {
                        longestlen = JimStringCommonLength(longeststr, longestlen, Jim_String(valObj), Jim_Utf8Length(interp, valObj));
                    }
                }
                if (longeststr) {
                    Jim_SetResultString(interp, longeststr, longestlen);
                }
                return JIM_OK;
            }
    }
    return JIM_ERR; /* Cannot ever get here */
}

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-tclprefix-version.h>

Retval Jim_tclprefixInit(Jim_InterpPtr interp)
{
    if (Jim_PackageProvide(interp, "tclprefix", version, JIM_ERRMSG)) {
        return JIM_ERR;
    }

    Jim_CreateCommand(interp, "tcl::prefix", Jim_TclPrefixCoreCommand, NULL, NULL);
    return JIM_OK;
}

END_JIM_NAMESPACE

#endif // #ifdef jim_ext_tclprefix
