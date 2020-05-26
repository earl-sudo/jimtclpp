/*
 * Implements the array command_ for jim
 *
 * (c) 2008 Steve Bennett <steveb@workware.net.au>
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
 * Based on code originally from Tcl 6.7:
 *
 * Copyright 1987-1991 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#include <jim-api.h>
#include <assert.h>

#if jim_ext_array 

BEGIN_JIM_NAMESPACE 

static Retval array_cmd_exists(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    /* Just a regular [info exists] */
    Jim_ObjPtr dictObj = Jim_GetVariable(interp, argv[0], JIM_UNSHARED);
    Jim_SetResultInt(interp, dictObj && Jim_DictSize(interp, dictObj) != -1);
    return JRET(JIM_OK);
}

static Retval array_cmd_get(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_ObjPtr objPtr = Jim_GetVariable(interp, argv[0], JIM_NONE);
    Jim_ObjPtr patternObj;

    if (!objPtr) {
        return JRET(JIM_OK);
    }

    patternObj = (argc == 1) ? NULL : argv[1];

    /* Optimise the "all" case */
    if (patternObj == NULL || Jim_CompareStringImmediate(interp, patternObj, "*")) {
        if (Jim_IsList(objPtr) && Jim_ListLength(interp, objPtr) % 2 == 0) {
            /* A list with an even number of elements */
            Jim_SetResult(interp, objPtr);
            return JRET(JIM_OK);
        }
    }

    return Jim_DictMatchTypes(interp, objPtr, patternObj, JIM_DICTMATCH_KEYS, JIM_DICTMATCH_KEYS | JIM_DICTMATCH_VALUES);
}

static Retval array_cmd_names(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    Jim_ObjPtr objPtr = Jim_GetVariable(interp, argv[0], JIM_NONE);

    if (!objPtr) {
        return JRET(JIM_OK);
    }

    return Jim_DictMatchTypes(interp, objPtr, argc == 1 ? NULL : argv[1], JIM_DICTMATCH_KEYS, JIM_DICTMATCH_KEYS);
}

static Retval array_cmd_unset(Jim_InterpPtr interp, int argc, Jim_ObjConstArray argv) // #JimCmd
{
    int i;
    int len;
    Jim_ObjPtr resultObj;
    Jim_ObjPtr objPtr;
    Jim_ObjArray *dictValuesObj;

    if (argc == 1 || Jim_CompareStringImmediate(interp, argv[1], "*")) {
        /* Unset the whole array */
        Jim_UnsetVariableIgnoreErr(interp, argv[0], JIM_NONE);
        return JRET(JIM_OK);
    }

    objPtr = Jim_GetVariable(interp, argv[0], JIM_NONE);

    if (objPtr == NULL) {
        /* Doesn't exist, so nothing to do */
        return JRET(JIM_OK);
    }

    if (Jim_DictPairs(interp, objPtr, &dictValuesObj, &len) != JRET(JIM_OK)) {
        /* Variable is not an array - tclsh ignores this and returns nothing - be compatible */
        Jim_SetResultString(interp, "", -1);
        return JRET(JIM_OK);
    }

    /* Create a new object with the values which don't match */
    resultObj = Jim_NewDictObj(interp, NULL, 0);

    for (i = 0; i < len; i += 2) {
        if (!Jim_StringMatchObj(interp, argv[1], dictValuesObj[i], 0)) {
            IGNOREJIMRET Jim_DictAddElement(interp, resultObj, dictValuesObj[i], dictValuesObj[i + 1]);
        }
    }
    free_Jim_ObjArray(dictValuesObj); // #FreeF 

    IGNOREJIMRET Jim_SetVariable(interp, argv[0], resultObj);
    return JRET(JIM_OK);
}

static Retval array_cmd_size(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    Jim_ObjPtr objPtr;
    int len = 0;

    /* Not found means zero length */
    objPtr = Jim_GetVariable(interp, argv[0], JIM_NONE);
    if (objPtr) {
        len = Jim_DictSize(interp, objPtr);
        if (len < 0) {
            /* Variable is not an array - tclsh ignores this and returns 0 - be compatible */
            Jim_SetResultInt(interp, 0);
            return JRET(JIM_OK);
        }
    }

    Jim_SetResultInt(interp, len);

    return JRET(JIM_OK);
}

static Retval array_cmd_stat(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    Jim_ObjPtr objPtr = Jim_GetVariable(interp, argv[0], JIM_NONE);
    if (objPtr) {
        return Jim_DictInfo(interp, objPtr);
    }
    Jim_SetResultFormatted(interp, "\"%#s\" isn't an array", argv[0], NULL);
    return JRET(JIM_ERR);
}

static Retval array_cmd_set(Jim_InterpPtr interp, int argc MAYBE_USED, Jim_ObjConstArray argv MAYBE_USED) // #JimCmd
{
    int i;
    int len;
    Jim_ObjPtr listObj = argv[1];
    Jim_ObjPtr dictObj;

    len = Jim_ListLength(interp, listObj);
    if (len % 2) {
        Jim_SetResultString(interp, "list must have an even number of elements", -1); // #ErrStr
        return JRET(JIM_ERR);
    }

    dictObj = Jim_GetVariable(interp, argv[0], JIM_UNSHARED);
    if (!dictObj) {
        /* Doesn't exist, so just set the list directly */
        return Jim_SetVariable(interp, argv[0], listObj);
    }
    else if (Jim_DictSize(interp, dictObj) < 0) {
        return JRET(JIM_ERR);
    }

    if (Jim_IsShared(dictObj)) {
        dictObj = Jim_DuplicateObj(interp, dictObj);
    }

    for (i = 0; i < len; i += 2) {
        Jim_ObjPtr nameObj;
        Jim_ObjPtr valueObj;

        // Can't have a bad index error.
        assert(i < Jim_Length(listObj));
        IGNORE_IMPOS_ERROR Jim_ListIndex(interp, listObj, i, &nameObj, JIM_NONE);
        IGNORE_IMPOS_ERROR Jim_ListIndex(interp, listObj, i + 1, &valueObj, JIM_NONE);

        IGNOREJIMRET Jim_DictAddElement(interp, dictObj, nameObj, valueObj);
    }
    return Jim_SetVariable(interp, argv[0], dictObj);
}

static const jim_subcmd_type g_array_command_table[] = { // #JimSubCmdDef
        {       "exists",
                "arrayName",
                array_cmd_exists,
                1,
                1,
                /* Description: Does array exist? */
        },
        {       "get",
                "arrayName ?pattern?",
                array_cmd_get,
                1,
                2,
                /* Description: Array contents as name_ value list */
        },
        {       "names",
                "arrayName ?pattern?",
                array_cmd_names,
                1,
                2,
                /* Description: Array keys as a list */
        },
        {       "set",
                "arrayName list",
                array_cmd_set,
                2,
                2,
                /* Description: Set array from list */
        },
        {       "size",
                "arrayName",
                array_cmd_size,
                1,
                1,
                /* Description: Number of elements in array */
        },
        {       "stat",
                "arrayName",
                array_cmd_stat,
                1,
                1,
                /* Description: Print statistics about an array */
        },
        {       "unset",
                "arrayName ?pattern?",
                array_cmd_unset,
                1,
                2,
                /* Description: Unset elements of an array */
        },
        {       
        }
};

#undef JIM_VERSION
#define JIM_VERSION(MAJOR, MINOR) static const char* version = #MAJOR "." #MINOR ;
#include <jim-array-version.h>

JIM_EXPORT Retval Jim_arrayInit(Jim_InterpPtr interp) // #JimCmdInit
{
    if (Jim_PackageProvide(interp, "array", version, JIM_ERRMSG)) 
        return JRET(JIM_ERR);

    Retval ret = Jim_CreateCommand(interp, "array", Jim_SubCmdProc, (void *)g_array_command_table, NULL);
    return ret;
}

END_JIM_NAMESPACE

#endif // #if jim_ext_array 
